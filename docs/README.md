# Documentation Guide

> 기준: **Architecture v1.2 + RTOS/Linux Execution Policy + Integration Review 2026-09-09**

이 문서는 `docs/`의 **공통 통합 기준**이다. 각 역할 폴더의 `README.md`에는 최신 개발/결정사항이 있고, `SPECIFICATION.md`, `ARCHITECTURE.md`, `TEST_REPORT.md`는 그 기준을 실제 수치와 구현 결과로 채워가는 문서다.

# 1. 현재 역할 문서

```text
docs/
├ Ultrasonic_Perception/          # A
├ IVI/                            # B
├ Motor_Steering_Control/         # C
├ Lighting_Ambient_LIN_CAN/       # D
├ HPC_Camera_Vision/              # E
├ VCU_DTC_CAN_Integration/        # F
└ templates/
```

각 역할은 먼저 자기 폴더의 `README.md`를 읽고, 거기에 적힌 **지금 개발해야 할 것 / 반드시 결정해야 할 것**을 기준으로 `SPECIFICATION.md`, `ARCHITECTURE.md`, `TEST_REPORT.md`를 갱신한다.

# 2. 실행 환경

```text
STM32 Node
→ FreeRTOS + CMSIS-RTOS2 기본

Raspberry Pi HPC
→ Linux Service / Process / Thread
```

공통 원칙:
- ISR은 timestamp/flag/notification 등 최소 처리만 한다.
- 주기/제어/통신/UI/진단을 서로 blocking시키지 않는다.
- Queue/Notification/Event를 사용하고 무분별한 공유 전역변수를 피한다.
- Control/Safety 경로에서 blocking log를 하지 않는다.
- Task period/jitter, stack high-water, queue overflow, watchdog 조건을 실제 측정한다.
- Pi는 FPS/latency/CPU/RAM/temperature/queue backlog를 측정한다.

# 3. Integration Review에서 정리한 공통 Interface

아래 이름은 **논리적 메시지 이름**이다. 실제 CAN ID, DLC, bit position은 F 담당이 CAN Matrix에서 확정한다.

| Message | Publisher | Consumer | 의미 |
|---|---|---|---|
| `Ultrasonic_Status` | A | F/B/E | distance, valid, warning, fault flags |
| `Vision_Status` | E | F/B | lane/object/parking semantic result, valid/health |
| `ADAS_Request` | E | F | speed/steering 등 고수준 요청 |
| `Final_Drive_Command` | F | C | final speed/steering/drive enable/gear 관련 최종 명령 |
| `Drive_Status` | C | F/B/E | RPM, speed, steering/control health |
| `Body_User_Request` | B | F | HMI에서 발생한 조명/차량설정 사용자 요청 |
| `Body_Command` | F | D | VCU가 검증/통합한 최종 Body 명령 |
| `Body_Status` | D | F/B/E | ambient, lamp, LIN/gateway health |
| `Vehicle_State` | F | All | gear, mode, safety/ready state |
| `Driver_Input` | F | B/E | normalized accel/brake/steering 등 |
| `DTC_Event` | 각 Node | F/Pi/B 필요 시 | 공통 고장 이벤트 |
| `ECU_Heartbeat` | 각 Node | F/Pi | node alive/health |

# 4. Single Owner 규칙

통합에서 가장 중요한 규칙이다.

```text
Ultrasonic distance/warning → A
UI representation           → B
Motor/Servo actuator output → C
LIN schedule / CAN↔LIN      → D Gateway
Ambient/Lamp actual state   → D Slave
Vision semantic result      → E
ADAS high-level request     → E
Final vehicle command       → F
Body final command          → F
DTC history database        → Pi DTC Manager
```

같은 최종 데이터를 여러 Node가 동시에 publish하지 않는다.

# 5. 이번 검토에서 고친 핵심 문제

1. **Vision 상태와 제어요청이 섞여 있던 문제**
   - `Vision_Status`와 `ADAS_Request`로 분리한다.

2. **H735와 VCU가 둘 다 Body_Command를 publish할 수 있던 문제**
   - H735는 `Body_User_Request`만 생성한다.
   - VCU가 최종 `Body_Command`의 single publisher다.

3. **Ultrasonic invalid와 warning enum이 중복될 수 있던 문제**
   - `valid`와 `warning_level`을 분리한다.
   - `valid=false`이면 warning을 정상 판단에 사용하지 않는다.

4. **VCU/Drive timeout 책임이 뒤집혀 있던 문제**
   - Drive ECU가 `Final_Drive_Command` timeout을 검출한다.
   - VCU는 `Drive_Status`/Heartbeat/peer timeout을 검출한다.

5. **DTC History ownership이 겹칠 수 있던 문제**
   - 각 ECU는 local fault를 검출한다.
   - F는 DTC 규칙/severity/safety action을 통합한다.
   - Pi가 DTC History DB를 소유한다.
   - B는 표시한다.

6. **SafetyTask와 VcuControlTask가 final command를 동시에 쓸 위험**
   - `VcuControlTask`만 final command를 쓴다.
   - `SafetyTask`는 override state를 갱신하고 VcuControlTask를 즉시 깨운다.

# 6. Parking 데이터 규칙

```text
A Ultrasonic
→ 거리 / valid / SAFE-WARNING-CRITICAL
→ VCU에 직접 전달

E Rear Vision
→ object / position / vision warning
→ Vision_Status

F VCU
→ 두 정보를 각각 유지
→ Ultrasonic CRITICAL을 Rear Vision이 해제하지 못함
→ 최종 action 결정
```

센서 fusion을 한다고 해서 한 센서의 critical fault를 다른 센서가 지워버리면 안 된다.

# 7. Body 제어 규칙

```text
H735 Touch
→ Body_User_Request
→ VCU
→ validation / vehicle state rule
→ Body_Command
→ Gateway
→ LIN Lamp_Command
→ LIN Slave
→ Lamp Output
```

Brake lamp, direction/vehicle-state-dependent output처럼 차량 상태와 연관된 명령은 VCU가 최종 통합한다.

# 8. DTC 규칙

```text
Local Node
→ fault detect
→ local fault_flags
→ confirmed DTC_Event

VCU
→ severity / safety relevance
→ safe action if needed

Pi
→ history / timestamp / count / storage

H735
→ active/history display
```

공통 `DTC_Event` 최소 필드 후보:
- source/node
- code
- status
- severity
- timestamp 또는 sequence

최종 field/bit layout은 F가 팀과 합의해 CAN Matrix/DTC table에 확정한다.

# 9. Stage 2 전에 반드시 Freeze할 공통 결정

아래가 안 정해지면 2-node 통합을 시작하지 않는다.

- 각 STM32 실제 MCU와 FDCAN 지원 여부
- CAN FD Transceiver / Pi CAN FD interface
- logical message별 Publisher/Consumer
- signal name / unit / range / valid condition
- message cycle / timeout
- `Final_Drive_Command` 구조
- `Vision_Status` / `ADAS_Request` 구조
- `Body_User_Request` / `Body_Command` 구조
- `Ultrasonic_Status` 구조
- `DTC_Event` 공통 field
- `ECU_Heartbeat` node 식별 방식
- VCU arbitration 기본 rule

# 10. 문서 작성 순서

```text
1. README의 개발/결정사항 확인
2. 부품/센서/보드 Datasheet 확인
3. SPECIFICATION의 TBD 결정
4. ARCHITECTURE의 Task/Service/Interface 갱신
5. Stage 1 구현
6. TEST_REPORT에 실제 측정값 기록
7. Stage 2 전 CAN/LIN contract freeze
```

`docs/archive/`는 과거 참고용이며 현재 개발 기준으로 사용하지 않는다.

[Main README](../README.md)
