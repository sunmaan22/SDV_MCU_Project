# VCU + DTC + CAN Integration Software Architecture

> 2026-09-11: STM32G431KB 구매 모델 부분 동결. [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-001~005를 따른다. 제조사/revision/핀 배정과 실기 시험은 별도이며, 아래 시험 결과/측정값을 PASS로 변경한 것은 아니다.

> **2026-09-15 범위 변경:** `Driver_Input`(가속/브레이크/조향) publisher가 F에서 C로 이전됐다 (`FINAL_IMPLEMENTATION_SPEC.md` §1.1, §3.3). F는 더 이상 GPIO/ADC로 Driver Input을 직접 읽지 않으며, `DriverInputTask`/`DriverInputAdapter`는 제거하고 C가 발행한 `Driver_Input`을 CAN RX로 수신해 arbitration에 사용한다. Gear/E-Stop 등 VCU 자체 물리 입력은 유지된다. `Vision_Request`는 `ADAS_Request`로 명칭을 통일했다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 문서 목적: VCU가 Driver_Input(CAN), CAN Request, Fault를 어떤 FreeRTOS 구조로 받아 최종 차량 명령으로 만드는지 설명한다.

## Document Information

| Item | Value |
|---|---|
| Node / System | VCU + DTC + CAN Integration |
| Owner | F |
| Board / Platform | STM32G431KB (STM32 #5) |
| Execution Model | FreeRTOS + CMSIS-RTOS2 |
| Revision | v0.2 |
| Status | Draft |
| Related Specification | `SPECIFICATION.md` |
| Related Test | `TEST_REPORT.md` |

# 1. Introduction & Goals

## Purpose

VCU는 프로젝트에서 **최종 차량 판단과 안전 우선순위 적용**을 담당한다.

```text
Driver_Input (CAN, C 발행)
ADAS_Request
Ultrasonic_Status (Parking Critical 포함)
Peer ECU Status/Fault
        ↓
       VCU
        ↓
Final Speed / Steering / Enable
```

## Scope

포함:
- Driver_Input(CAN) 수신/validation (accel/brake/steering 직접 GPIO/ADC 획득은 하지 않음)
- Gear/E-Stop 등 VCU 자체 물리 입력 수집
- vehicle mode/state
- arbitration
- safety override
- CAN RX/TX
- heartbeat
- DTC integration
- RTOS health/watchdog

제외:
- Motor/Servo PWM
- Vision processing
- Ultrasonic distance calculation
- Lamp GPIO
- HMI rendering

# 2. Quality Goals

| Priority | Goal | Scenario |
|---:|---|---|
| 1 | Safety | E-Stop/critical fault 발생 시 normal request보다 먼저 safe command 적용 |
| 2 | Timing | Control/Safety task가 deadline을 지키고 logging에 막히지 않음 |
| 3 | Reliability | CAN timeout/stale request가 최종 명령에 계속 쓰이지 않음 |
| 4 | Testability | Dummy Driver/CAN input으로 arbitration 단독 시험 가능 |
| 5 | Maintainability | Input, Safety, Arbitration, CAN, Diagnostics 분리 |

# 3. Constraints

- STM32 + FreeRTOS 사용
- CAN FD backbone 목표
- 최종 actuator PWM은 Drive ECU 소유
- Camera raw frame 미수신
- DTC History DB는 Raspberry Pi service 소유
- CAN ID/timeout 일부 TBD

# 4. Context View

```mermaid
flowchart LR
    GEAR[Gear/E-Stop 등 VCU 자체 물리 입력] --> VCU[VCU]
    DRIVE[Drive ECU] -->|Driver_Input| VCU
    HPC[HPC Vision] -->|ADAS_Request| VCU
    US[Ultrasonic ECU] -->|Ultrasonic_Status| VCU
    DRIVE -->|Drive_Status| VCU
    BODY[Body Gateway] -->|Body Status| VCU
    ALL[All ECUs] -->|Heartbeat / DTC| VCU
    VCU -->|Final Drive Command| DRIVE
    VCU -->|Vehicle State| HMI[H735]
    VCU -->|DTC/State| PI[Pi DTC Manager]
```

# 5. Solution Strategy

| Strategy | Reason |
|---|---|
| SafetyTask를 일반 ControlTask와 분리 | critical condition latency 분리 |
| `Driver_Input`은 CAN RX로 수신, freshness/validation을 CanRxTask에서 처리 | GPIO/ADC 직접 획득 대신 C가 발행한 CAN 신호를 신뢰 경계로 검증 |
| CAN RX는 event-driven task | ISR 최소화 |
| VcuControlTask가 final command single owner | 여러 task가 최종 명령을 동시에 수정하지 않게 함 |
| CanTxTask가 CAN TX single owner | bus access 충돌과 blocking 감소 |
| Diagnostics를 낮은 우선순위로 분리 | control timing 보호 |
| HealthTask가 watchdog refresh 조건 관리 | task hang/queue 문제 감지 |

# 6. Component View

```text
Gear/E-Stop GPIO         FDCAN (Driver_Input/ADAS_Request/Ultrasonic_Status 등)
        ↓                        ↓
LocalInputAdapter        CanRxAdapter
        ↓                        ↓
InputValidator / SignalFreshness
        ↓
VehicleStateManager
        ↓
SafetyManager
        ↓
ArbitrationManager
        ↓
FinalCommandRepository
        ↓
CanTxService
```

Diagnostics path:

```text
Local/Peer Fault
→ DtcManager
→ severity/status
→ SafetyManager + CanTxService
```

| Component | Responsibility |
|---|---|
| `LocalInputAdapter` | gear/E-Stop 등 VCU 자체 물리 입력 읽기 |
| `InputValidator` | `Driver_Input`(CAN) range/freshness, local input calibration/invalid 판단 |
| `CanRxAdapter` | CAN frame 수신/queue (`Driver_Input` 포함) |
| `SignalFreshnessManager` | timeout/timestamp 관리 |
| `VehicleStateManager` | P/R/N/D, ready/mode/state 관리 |
| `SafetyManager` | E-Stop, critical fault, override |
| `ArbitrationManager` | Driver/ADAS/Ultrasonic Parking 우선순위 적용 (Parking Critical이 ADAS_Request보다 항상 우선) |
| `FinalCommandRepository` | final speed/steering/enable single writer data |
| `DtcManager` | local/peer fault code/status/severity 통합 |
| `CanTxService` | final command/state/heartbeat/dtc TX |
| `HealthManager` | RTOS health/watchdog 조건 |

# 7. RTOS / Concurrency View

## 7.1 Task Model

| Task | Responsibility | Trigger / Period 후보 | Relative Priority | Blocking Policy |
|---|---|---|---|---|
| `SafetyTask` | E-Stop/critical fault/safety override | event + fast periodic | Highest | blocking 금지 |
| `VcuControlTask` | vehicle state + arbitration + final command | 5~10 ms 후보 | High | logging/CAN blocking 금지 |
| `CanRxTask` | RX decode(`Driver_Input` 포함), freshness repository update | event | High | short processing |
| `LocalInputTask` | Gear/E-Stop 등 VCU 자체 GPIO 획득 | 10~20 ms 후보 | High/Normal | long I/O 금지 |
| `CanTxTask` | command/state/heartbeat/DTC 송신 | event/periodic | Normal/High | bounded queue |
| `DiagnosticTask` | DTC state/table/event handling | event/periodic | Normal/Low | control path block 금지 |
| `HealthTask` | task alive/queue/stack/watchdog | 50~100 ms 후보 | Low/Normal | bounded |

## 7.2 Priority Rationale

```text
SafetyTask
> VcuControlTask / critical CanRx
> LocalInputTask
> CanTxTask
> DiagnosticTask / HealthTask
```

정확한 numeric priority는 측정 후 확정한다.

## 7.3 ISR Map

| Interrupt | ISR Responsibility | Wake-up Target |
|---|---|---|
| FDCAN RX | frame metadata 저장 / queue notify | `CanRxTask` |
| GPIO EXTI E-Stop 후보 | state/timestamp capture | `SafetyTask` |
| Gear GPIO change 후보 | state capture | `LocalInputTask` |

ISR에서는 arbitration, printf, DTC table lookup, CAN application decode 전체를 수행하지 않는다.

## 7.4 RTOS Objects

| Object | Type | Producer | Consumer | Policy |
|---|---|---|---|---|
| `CanRxQueue` | Queue | FDCAN ISR | CanRxTask | bounded, overflow health 기록 |
| `SafetyEvent` | Notification/Event Flag | ISR/Health | SafetyTask | critical event 우선 |
| `FinalCommandQueue` | Queue/latest mailbox | VcuControlTask | CanTxTask | 최신 command 우선 |
| `DtcEventQueue` | Queue | Local/CanRx | DiagnosticTask | overflow 기록 |
| `HealthFlags` | Event/bitset | critical tasks | HealthTask | alive window 확인 |

# 8. Runtime Views

## 8.1 Normal Drive

```mermaid
sequenceDiagram
    participant DR0 as Drive ECU (C)
    participant RX as CanRxTask
    participant LI as LocalInputTask
    participant VC as VcuControlTask
    participant TX as CanTxTask
    participant DR as Drive ECU
    DR0->>RX: Driver_Input (accel/brake/steering)
    LI->>VC: gear/E-Stop local state
    RX->>VC: validated Driver_Input
    VC->>VC: state + arbitration
    VC->>TX: final command
    TX->>DR: CAN Final_Drive_Command
```

## 8.2 ADAS Request

```mermaid
sequenceDiagram
    participant HPC as HPC (E)
    participant RX as CanRxTask
    participant VC as VcuControlTask
    participant TX as CanTxTask
    HPC->>RX: ADAS_Request
    RX->>VC: valid/fresh request
    VC->>VC: driver + Ultrasonic Parking Critical + safety + mode arbitration
    VC->>TX: final command
```

## 8.3 E-Stop

```mermaid
sequenceDiagram
    participant ISR as E-Stop EXTI
    participant S as SafetyTask
    participant VC as VcuControlTask
    participant TX as CanTxTask
    ISR->>S: safety event
    S->>VC: override active
    VC->>TX: drive disable / safe command
```

## 8.4 Peer Timeout

```text
Heartbeat missing
→ CanRx/Freshness timeout
→ DTC/Health
→ Safety evaluation
→ request invalid / safe action 후보
```

# 9. Deployment View

```text
Gear buttons / E-Stop
        ↓ GPIO
     STM32 #5 VCU
     + FreeRTOS
        ↕ FDCAN (Driver_Input 포함 수신)
 CAN FD Transceiver
        ↕
 CAN FD Backbone
```

Accelerator/Brake/Steering 입력은 더 이상 VCU에 직접 연결하지 않는다 (C의 `DriverInputTask`가 읽어 `Driver_Input`으로 발행). 실제 Gear/E-Stop pin, transceiver는 TBD.

# 10. Interfaces & Contracts

## RX

| Signal | Sender | Use |
|---|---|---|
| `Driver_Input` | C | 가속/브레이크/조향 driver 입력 (publisher가 C로 이전) |
| `ADAS_Request` | E(HPC) | 전방 객체 회피 요청 (주차 사유 없음) |
| `Ultrasonic_Status` | A | distance/warning/Parking Critical |
| `Drive_Status` | C | RPM(estimated)/speed(estimated)/health |
| `Body_Status` | D | body/LIN status |
| `ECU_Heartbeat` | All | peer health |
| `DTC_Event` | All | diagnostic integration |

## TX

| Signal | Receiver | Use |
|---|---|---|
| `Final_Drive_Command` 후보 | Drive ECU | final speed/steering/enable |
| `Vehicle_State` | H735/HPC/All | gear/mode/safety |
| `ECU_Heartbeat` | All | VCU alive |
| `DTC_Event` | Pi/H735 | VCU/local fault |

# 11. Data & State Model

## Main Data

| Data | Owner | Meaning |
|---|---|---|
| `driver_input` | CanRxTask repository | C가 발행한 `Driver_Input`의 최신 유효값 |
| `local_input` | LocalInputTask | gear/E-Stop 등 VCU 자체 입력 |
| `vehicle_state` | VcuControlTask | mode/gear/readiness |
| `adas_request` | CanRxTask repository | latest valid `ADAS_Request` |
| `parking_status` | CanRxTask repository | latest `Ultrasonic_Status` (Critical 포함) |
| `safety_override` | SafetyTask | critical override |
| `final_command` | VcuControlTask | final speed/steer/enable |
| `dtc_state` | DiagnosticTask | active/pending/history candidate |

## State 후보

```mermaid
stateDiagram-v2
    [*] --> INIT
    INIT --> READY
    READY --> DRIVE: Gear D/R + enable conditions
    DRIVE --> READY: stop/neutral/park
    READY --> FAULT
    DRIVE --> FAULT
    FAULT --> READY: recovery conditions satisfied
```

세부 state는 통합 전 확정한다.

# 12. Cross-cutting Concepts

## Timeout/Freshness

모든 periodic CAN request/status는 timestamp를 가진다. stale data는 정상값으로 계속 사용하지 않는다.

## DTC

각 ECU가 자기 fault를 검출하고 VCU/Pi가 공통 규칙으로 통합한다.

```text
Local Fault Detection
→ DTC Event
→ severity/status
→ Pi history/logger
→ H735 display
→ VCU safe action if critical
```

## Watchdog

```text
SafetyTask heartbeat
VcuControlTask heartbeat
CanRxTask heartbeat
LocalInputTask heartbeat
        ↓
HealthTask
        ↓
all required healthy?
├ yes → IWDG refresh
└ no  → fault / refresh policy
```

# 13. Architecture Decisions

| ADR | Decision | Reason |
|---|---|---|
| ADR-VCU-001 | final command는 VcuControlTask 단일 owner | race/책임 중복 방지 |
| ADR-VCU-002 | SafetyTask 별도 분리 | critical event latency 보호 |
| ADR-VCU-003 | actuator PWM은 Drive ECU 소유 | 판단과 actuator control 분리 |
| ADR-VCU-004 | DTC History DB는 Pi에 둠 | STM32는 runtime fault/safety에 집중 |
| ADR-VCU-005 | CAN TX single owner task | 통신 자원 충돌 감소 |
| ADR-VCU-006 | `Driver_Input` publisher를 F에서 C로 이전, F의 `DriverInputTask` 제거 | accel/brake/steering 입력 하드웨어가 실제로 C에 붙기 때문 (2026-09-15, `FINAL_IMPLEMENTATION_SPEC.md` §1.1) |

# 14. Risks

- Arbitration rule 미확정
- sensor calibration 미확정
- command timeout 너무 짧거나 길 가능성
- CAN matrix 변경에 따른 재작업
- SafetyTask priority/latency 미검증
- queue depth/stack size 미검증

# 15. Requirement Traceability

| Requirement | Component | Task | Test |
|---|---|---|---|
| REQ-VCU-001 | InputValidator (Driver_Input CAN + local Gear/E-Stop) | CanRxTask/LocalInputTask | T-VCU-001 |
| REQ-VCU-004 | SafetyManager | SafetyTask | T-VCU-004 |
| REQ-VCU-005 | ArbitrationManager | VcuControlTask | T-VCU-005 |
| REQ-VCU-006 | FinalCommandRepository/CanTx | VcuControlTask/CanTxTask | T-VCU-006 |
| REQ-VCU-009 | DtcManager | DiagnosticTask | T-VCU-009 |
| REQ-VCU-013 | HealthManager | HealthTask | T-VCU-013 |

# 16. Review Checklist

- [ ] Driver/ADAS/Parking/Fault priority가 설명 가능하다.
- [ ] Final command owner가 하나다.
- [ ] E-Stop 경로가 일반 logging/UI 경로와 독립적이다.
- [ ] stale CAN data 정책이 있다.
- [ ] ISR과 Task 책임이 분리됐다.
- [ ] DTC와 safety action의 관계가 정의됐다.
- [ ] Stack/Queue/Watchdog 시험 계획이 있다.
