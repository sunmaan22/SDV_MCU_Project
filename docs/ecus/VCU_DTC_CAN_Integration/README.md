# VCU + DTC + CAN Integration Documentation

> **2026-09-15 범위 변경 (1차):** `Driver_Input`(가속/브레이크/조향) publisher가 F에서 C로 이전됐다. F는 더 이상 GPIO/ADC로 Driver Input을 직접 읽지 않고 CAN RX로 수신한다. `Vision_Request`는 `ADAS_Request`로 명칭을 통일했고, Ultrasonic Collision Critical이 `ADAS_Request`보다 항상 우선함을 재확인했다.
>
> **2026-09-15 범위 변경 (2차):** Gear/E-Stop 물리 입력도 F에서 C로 이전했다. **F는 Driver/Gear/E-Stop 입력용 GPIO를 갖지 않는다.** E-Stop은 C가 로컬에서 즉시 차단(CAN 비의존)하고 상태만 `Driver_Input.estop_status`로 CAN 보고한다. Pi DTC Manager(History DB)는 삭제됐다 — `DTC_Event`는 B(IVI)가 실시간으로만 표시한다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)
> Vehicle State / Arbitration / CAN / DTC / Heartbeat / RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **F 담당: VCU + DTC + CAN Integration**의 하위 구현 문서다.

## 고정 역할

```text
Driver_Input (CAN, C 발행 — accel/brake/steering/gear/estop_status)
+ ADAS_Request
+ Vision_Status
+ Ultrasonic_Status (Collision Critical 포함)
+ Drive/Body Status
+ Heartbeat / DTC
+ Body_User_Request
        ↓
       VCU
Safety + Arbitration (Collision Critical > ADAS_Request)
        ↓
Final_Drive_Command / Body_Command
```

VCU는 차량 전체의 최종 판단과 명령 통합을 담당한다. 물리 GPIO는 하나도 갖지 않는다 — 모든 입력이 CAN으로 들어온다.

## 이미 고정된 규칙

- `Final_Drive_Command`와 `Body_Command` Publisher는 F다.
- `Driver_Input` Publisher는 C다 (기존 F에서 이전, 2026-09-15). F는 CAN RX로만 소비하며, accel/brake/steering뿐 아니라 gear/estop_status도 여기 포함된다.
- `VcuControlTask`만 final command를 작성한다.
- `SafetyTask`는 CAN으로 수신한 `estop_status`를 보고 override state를 갱신하고 VcuControlTask를 깨운다. **모터의 실제 정지는 C가 이미 로컬로 수행한 뒤다** — F의 override는 Vehicle_State 갱신/다른 요청 무효화용이다.
- `Vision_Status`와 `ADAS_Request`를 분리한다.
- Ultrasonic Collision `CRITICAL`은 `ADAS_Request`보다 항상 우선하며, `ADAS_Request`는 이를 해제/override할 수 없다.
- H735의 조명 입력은 `Body_User_Request`로 받는다.
- Drive command timeout은 C가 감지하고, VCU는 peer status/heartbeat timeout을 감지한다.
- DTC History DB는 삭제됐다 — `DTC_Event`는 지속 저장 없이 B(IVI)가 실시간(Active만) 표시한다.
- Brake와 Accelerator 동시 유효입력은 Brake 우선이 기본이다 (`Driver_Input` 값 기준).
- D↔R 즉시 반전 금지.

## FreeRTOS 구조

```text
CanRxTask (Driver_Input의 estop_status 포함 수신)
→ SafetyTask (override 판단)
→ VcuControlTask
→ final command single writer
→ CanTxTask

DiagnosticTask (DTC 실시간 발행, history 없음)
HealthTask
```

F는 GPIO 기반 Task(`LocalInputTask`)를 갖지 않는다.

## 구현해야 할 것

- `Driver_Input` CAN 수신/validation (C가 발행, gear/estop_status 포함)
- FreeRTOS task skeleton
- Dummy Driver_Input + ADAS + Ultrasonic arbitration
- Final_Drive_Command structure
- Body_User_Request → Body_Command path
- peer freshness/timeout repository
- DTC 실시간 발행 (history/저장 없음)
- CAN 수신 estop_status → VcuControlTask 반영 latency 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-005 ~ DEC-HW-006
DEC-NET-001 ~ DEC-NET-008
DEC-CTRL-001 ~ DEC-CTRL-006, DEC-CTRL-011 ~ DEC-CTRL-018
DEC-BODY-002
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Vehicle State, READY/Enable, D↔R, E-Stop recovery, Ultrasonic/ADAS action, Final_Drive_Command, Body_Command, Heartbeat, DTC, Watchdog, RTOS 수치는 Project Owner가 중앙 명세에서 결정한다. Gear/E-Stop 물리 입력 관련 Decision(`DEC-HW-020`, `DEC-HW-026~028`)은 [Motor_Steering_Control](../Motor_Steering_Control/README.md) 문서를 참고한다 (C 소유).

## Coding Gate

Vehicle State Machine, Arbitration, `Final_Drive_Command`, `Body_User_Request`, `Body_Command`, Heartbeat, DTC Event, timeout/safe state가 `FROZEN`되기 전에는 통합 제어 코드를 Baseline으로 확정하지 않는다.

## Stage 1 PASS

```text
Driver_Input(CAN, gear/estop 포함) 수신
+ FreeRTOS
+ Dummy arbitration
+ estop_status 반영 우선순위
+ Final_Drive_Command 생성
+ stale/timeout
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
