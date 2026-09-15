# VCU + DTC + CAN Integration Documentation

> **2026-09-15 범위 변경:** `Driver_Input`(가속/브레이크/조향) publisher가 F에서 C로 이전됐다. F는 더 이상 GPIO/ADC로 Driver Input을 직접 읽지 않고 CAN RX로 수신한다 (Gear/E-Stop은 VCU 자체 물리 입력 유지). `Vision_Request`는 `ADAS_Request`로 명칭을 통일했고, Ultrasonic Parking Critical이 `ADAS_Request`보다 항상 우선함을 재확인했다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)  
> Vehicle State / Arbitration / CAN / DTC / Heartbeat / RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **F 담당: VCU + DTC + CAN Integration**의 하위 구현 문서다.

## 고정 역할

```text
Gear/E-Stop (VCU 자체 입력)
+ Driver_Input (CAN, C 발행)
+ ADAS_Request
+ Vision_Status
+ Ultrasonic_Status (Parking Critical 포함)
+ Drive/Body Status
+ Heartbeat / DTC
+ Body_User_Request
        ↓
       VCU
Safety + Arbitration (Parking Critical > ADAS_Request)
        ↓
Final_Drive_Command / Body_Command
```

VCU는 차량 전체의 최종 판단과 명령 통합을 담당한다.

## 이미 고정된 규칙

- `Final_Drive_Command`와 `Body_Command` Publisher는 F다.
- `Driver_Input` Publisher는 C다 (기존 F에서 이전, 2026-09-15). F는 CAN RX로만 소비한다.
- `VcuControlTask`만 final command를 작성한다.
- `SafetyTask`는 override state를 갱신하고 VcuControlTask를 깨운다.
- `Vision_Status`와 `ADAS_Request`를 분리한다.
- Ultrasonic Parking `CRITICAL`은 `ADAS_Request`보다 항상 우선하며, `ADAS_Request`는 이를 해제/override할 수 없다.
- H735의 조명 입력은 `Body_User_Request`로 받는다.
- Drive command timeout은 C가 감지하고, VCU는 peer status/heartbeat timeout을 감지한다.
- DTC History DB는 Pi가 소유한다.
- Brake와 Accelerator 동시 유효입력은 Brake 우선이 기본이다 (`Driver_Input` 값 기준).
- D↔R 즉시 반전 금지.

## FreeRTOS 구조

```text
E-Stop / Critical Event
→ SafetyTask
→ safety override + notify
→ VcuControlTask
→ final command single writer
→ CanTxTask

LocalInputTask (Gear/E-Stop) ─┐
CanRxTask (Driver_Input 등) ──┤→ VcuControlTask
DiagnosticTask ────────────────┤
HealthTask ────────────────────┘
```

## 구현해야 할 것

- Gear / E-Stop input (VCU 자체)
- `Driver_Input` CAN 수신/validation (C가 발행)
- FreeRTOS task skeleton
- Dummy Driver_Input + ADAS + Ultrasonic arbitration
- Final_Drive_Command structure
- Body_User_Request → Body_Command path
- peer freshness/timeout repository
- DTC runtime processing
- E-Stop → VcuControlTask wake-up latency 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-005 ~ DEC-HW-006
DEC-HW-017 ~ DEC-HW-020
DEC-NET-001 ~ DEC-NET-008
DEC-CTRL-001 ~ DEC-CTRL-018
DEC-BODY-002
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

VCU Hardware, Driver Input calibration, Vehicle State, READY/Enable, D↔R, E-Stop recovery, Ultrasonic/ADAS action, Final_Drive_Command, Body_Command, Heartbeat, DTC, Watchdog, RTOS 수치는 Project Owner가 중앙 명세에서 결정한다.

## Coding Gate

Vehicle State Machine, Arbitration, `Final_Drive_Command`, `Body_User_Request`, `Body_Command`, Heartbeat, DTC Event, timeout/safe state가 `FROZEN`되기 전에는 통합 제어 코드를 Baseline으로 확정하지 않는다.

## Stage 1 PASS

```text
Gear/E-Stop + Driver_Input(CAN)
+ FreeRTOS
+ Dummy arbitration
+ E-Stop priority
+ Final_Drive_Command 생성
+ stale/timeout
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
