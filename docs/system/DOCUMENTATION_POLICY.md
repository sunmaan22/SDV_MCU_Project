# Documentation Guide

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> 기준: **Architecture v1.2 + Final Implementation Freeze Policy / 2026-09-09**

# 1. 최상위 구현 기준

코드 작성 직전의 최상위 명세서는 아래 파일 하나다.

- [`FINAL_IMPLEMENTATION_SPEC.md`](FINAL_IMPLEMENTATION_SPEC.md)

```text
FINAL_IMPLEMENTATION_SPEC.md
        ↓
각 역할 SPECIFICATION.md
        ↓
각 역할 ARCHITECTURE.md
        ↓
Code
        ↓
TEST_REPORT.md
```

문서 간 충돌이 있으면 우선순위는 다음과 같다.

```text
1. FINAL_IMPLEMENTATION_SPEC.md
2. 역할별 SPECIFICATION.md
3. 역할별 ARCHITECTURE.md
4. README / 예시 문서
5. archive
```

Project Owner가 Hardware / CAN / LIN / Vehicle State / Arbitration / DTC / RTOS 공통값을 직접 확정한다. 담당자나 AI는 `OWNER INPUT`, `OPEN` 상태를 임의로 `FROZEN`으로 바꾸지 않는다.

# 2. 역할 문서

문서 위치는 [전체 문서 안내](../README.md)와 [ECU별 안내](../ecus/README.md)를 참고한다.

각 역할 폴더에는:

```text
README.md
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

가 있다.

역할 README는 현재 해야 할 개발 항목을 설명하고, 실제 최종 수치와 Interface는 `FINAL_IMPLEMENTATION_SPEC.md`를 따른다.

# 3. 이미 고정된 역할 경계

| Message / Data | Owner / Publisher |
|---|---|
| `Ultrasonic_Status` | A |
| UI representation | B |
| `Body_User_Request` | B |
| Motor / Servo actual output | C |
| `Drive_Status` | C |
| LIN schedule / CAN↔LIN mapping | D Gateway |
| Ambient / Lamp actual state | D Slave |
| `Body_Status` | D Gateway |
| `Vision_Status` | E |
| `ADAS_Request` | E |
| `Final_Drive_Command` | F |
| `Body_Command` | F |
| `Vehicle_State` | F |
| `Driver_Input` | F |
| DTC History DB | Raspberry Pi DTC Manager |

같은 최종 데이터를 여러 Node가 동시에 publish하지 않는다.

# 4. 고정 Safety / Integration 규칙

```text
E-Stop / Critical Fault
> Ultrasonic Parking Critical
> ADAS Safety Request
> Normal Driver Request
```

- Ultrasonic `CRITICAL`을 Rear Vision이 해제하지 않는다.
- `valid=false`인 센서값은 정상 판단에 사용하지 않는다.
- H735는 `Body_Command`를 직접 publish하지 않고 `Body_User_Request`만 보낸다.
- `Final_Drive_Command`의 Publisher는 VCU 하나다.
- `VcuControlTask`만 final command를 작성한다.
- Drive ECU가 `Final_Drive_Command` timeout을 감지한다.
- VCU는 peer status/heartbeat timeout을 감지한다.
- DTC local detection은 각 Node, safety/severity integration은 VCU, History DB는 Pi, 표시는 H735가 담당한다.

# 5. 실행 환경

```text
STM32 Node
→ FreeRTOS + CMSIS-RTOS2 기본

Raspberry Pi HPC
→ Linux Service / Process / Thread
```

공통 원칙:
- ISR 최소 처리
- Control/Safety path에서 blocking log 금지
- Queue / Notification / Event / single-owner 구조 우선
- Task period/jitter, stack high-water, queue overflow, watchdog 실제 측정
- Pi는 FPS/latency/CPU/RAM/temperature/backlog 실제 측정

# 6. Owner Freeze 순서

코드 작성 전 Project Owner가 아래 순서로 `FINAL_IMPLEMENTATION_SPEC.md`를 채운다.

```text
1. Hardware Freeze
2. Network Freeze
3. Vehicle / Control Freeze
4. Perception / Vision Freeze
5. HMI / Body Freeze
6. DTC / Health Freeze
7. CAN / LIN Message Contract Freeze
8. RTOS / Linux Execution Contract Freeze
```

각 항목은:

```text
OPEN
→ Owner 결정
→ FROZEN
```

상태로 관리한다.

# 7. Coding Gate

다음 항목이 FROZEN되기 전에는 통합 코드에 영구 상수로 박지 않는다.

- CAN ID / DLC / bit position
- Message cycle / timeout
- Unit / scale / offset / range
- LIN frame / schedule
- Final_Drive_Command
- Vision_Status / ADAS_Request
- Ultrasonic_Status
- Body_User_Request / Body_Command
- DTC_Event
- ECU_Heartbeat
- Vehicle State Machine
- Arbitration / Safe State
- RTOS numeric priority / stack / queue depth

# 8. AI에게 역할 문서를 줄 때

다음 규칙을 같이 전달한다.

```text
FINAL_IMPLEMENTATION_SPEC.md가 최상위 규칙이다.
고정된 Publisher/Owner/역할 경계를 변경하지 마라.
OWNER INPUT 또는 OPEN 상태를 임의로 최종 확정하지 마라.
Project Owner가 승인한 값만 FROZEN으로 바꿔라.
내부 구현은 제안할 수 있지만 ECU 간 Interface는 최종 명세를 따른다.
```

# 9. 구현 Baseline

모든 핵심 결정이 FROZEN되면 해당 커밋을 **Implementation Baseline v1.0**으로 사용한다.

`docs/archive/`는 과거 참고용이며 현재 구현 기준으로 사용하지 않는다.

[Main README](../../README.md)
