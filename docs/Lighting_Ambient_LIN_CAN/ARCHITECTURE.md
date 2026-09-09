# Lighting + Ambient / LIN-CAN Software Architecture

> 문서 목적: D 담당의 **Body Gateway STM32 + Body LIN Slave STM32**를 어떤 구조로 구현하는지 설명한다. 기능 요구사항은 `SPECIFICATION.md`, 검증은 `TEST_REPORT.md`를 기준으로 한다.

## Document Information

| Item | Value |
|---|---|
| Node / System | Body Gateway + Body LIN Slave |
| Owner | D |
| Board / Platform | STM32 Gateway + STM32 Slave |
| Execution Model | FreeRTOS + CMSIS-RTOS2 기본 |
| Revision | v0.1 |
| Status | Draft |
| Related Specification | `SPECIFICATION.md` |
| Related Test | `TEST_REPORT.md` |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial filled example |

---

# 1. Introduction & Goals

## 1.1 Purpose

> Gateway는 차량 CAN FD Backbone과 LIN Body Subnetwork를 연결하고, LIN Slave는 Ambient Sensor와 Lamp Output을 소유한다.

## 1.2 Scope

포함:
- CAN FD RX/TX
- LIN Master schedule
- CAN↔LIN signal mapping
- Ambient sensing
- Lighting command/output/status
- Fault/timeout/DTC
- FreeRTOS task/ISR/queue/watchdog

제외:
- H735 UI 내부 구현
- VCU 최종 arbitration
- motor/steering control

## 1.3 Stakeholders

| Stakeholder | 관심사 |
|---|---|
| D 담당 | LIN/CAN/RTOS/Lighting 구조 |
| F VCU/CAN 통합 | Body message contract, timeout |
| B H735 | Body status, lamp request |
| 테스트 담당 | LIN timing, end-to-end mapping, fault |

---

# 2. Quality Goals

| Priority | Quality Goal | Scenario / Measure |
|---:|---|---|
| 1 | Reliability | Slave disconnect 시 Gateway가 hang되지 않고 fault 처리 |
| 2 | Timing | LIN schedule이 정의된 period/jitter를 만족 |
| 3 | Maintainability | CAN decode, mapping, LIN, lighting 기능 분리 |
| 4 | Testability | Gateway↔Slave만으로 Stage 1/2 시험 가능 |

---

# 3. Constraints

| Constraint | Reason / Impact |
|---|---|
| Gateway와 Slave는 별도 STM32 | LIN Master/Slave 실습 구조 |
| CAN FD Backbone 목표 | 상위 차량 네트워크 |
| LIN Transceiver 필요 | UART pin 직접 LIN 연결 불가 |
| CAN FD Transceiver 필요 | FDCAN pin 직접 CANH/L 연결 불가 |
| FreeRTOS 기본 | 프로젝트 SW 정책 |
| 실제 MCU/Transceiver/bitrate 일부 TBD | HW 확정 후 갱신 |

---

# 4. Context & Scope View

```mermaid
flowchart LR
    VCU[VCU] -->|Body_Command| GW[Body Gateway STM32]
    HMI[H735] -->|Body_Command| GW
    GW -->|Body_Status / Fault| VCU
    GW -->|Body_Status| HMI
    GW <-->|LIN| SLAVE[Body LIN Slave STM32]
    SLAVE --> AMB[Ambient Sensor]
    SLAVE --> LAMP[Lamp Outputs]
```

## External Interfaces

| Entity | Direction | Data | Interface | Owner |
|---|---|---|---|---|
| VCU/H735 | RX/TX | Body_Command / Body_Status | CAN FD | F/B |
| LIN Slave | RX/TX | Lamp/Ambient/Diagnostic | LIN | D |
| Ambient Sensor | RX | raw value | ADC/I2C TBD | D |
| Lamps | TX | GPIO/PWM | local | D |

---

# 5. Solution Strategy & Rationale

| Decision | Why | Quality |
|---|---|---|
| Gateway와 Slave 역할 분리 | LIN Master/Slave 구조를 명확히 보여줌 | Testability |
| CAN↔LIN mapping을 별도 component로 분리 | protocol driver와 signal logic 결합 방지 | Maintainability |
| LIN schedule을 dedicated task로 운영 | 주기/jitter 관리 | Timing |
| Lamp physical output은 Slave가 소유 | local actuator ownership | Reliability |
| CAN TX는 single-owner task 방향 | shared FDCAN contention 감소 | Maintainability |

---

# 6. Building Block / Component View

## 6.1 Gateway

```text
FDCAN Driver
   ↓
CanRxAdapter
   ↓
BodyCommandDecoder
   ↓
GatewayMapper
   ↓
LinScheduleManager
   ↓
LIN Driver

LIN Status
   ↓
GatewayMapper
   ↓
BodyStatusBuilder
   ↓
CanTxService
```

## 6.2 LIN Slave

```text
LIN Driver
   ↓
LinCommandHandler
   ↓
LightingManager
   ↓
LampDriver

AmbientDriver
   ↓
AmbientFilter / Validity
   ↓
BodyStatusRepository
   ↓
LIN Response Builder
```

## 6.3 Component Responsibility

| Component | Responsibility |
|---|---|
| `CanRxAdapter` | CAN frame receive / enqueue |
| `BodyCommandDecoder` | CAN payload → logical request |
| `GatewayMapper` | CAN signal ↔ LIN signal mapping |
| `LinScheduleManager` | LIN frame order/period 관리 |
| `CanTxService` | Body status / DTC / heartbeat TX |
| `LinCommandHandler` | Slave command decode |
| `AmbientManager` | sample/filter/validity |
| `LightingManager` | lamp state machine / output mapping |
| `HealthManager` | timeout/task/queue health |

---

# 7. Concurrency / RTOS View

## 7.1 Gateway Task Model

| Task | Responsibility | Trigger / Period | Relative Priority | Stack | Blocking Policy |
|---|---|---|---|---|---|
| `CanRxTask` | CAN command decode | Event | High | TBD | short only |
| `LinScheduleTask` | LIN master schedule | slot TBD | High | TBD | no printf |
| `GatewayMappingTask` | CAN↔LIN logical mapping | Event | Normal/High | TBD | bounded |
| `CanTxTask` | Body status/fault TX | Event/periodic | Normal | TBD | queue wait 허용 |
| `HealthTask` | timeout/task/queue/watchdog | 100 ms 후보 | Low/Normal | TBD | bounded |

## 7.2 Slave Task Model

| Task | Responsibility | Trigger / Period | Relative Priority | Stack | Blocking Policy |
|---|---|---|---|---|---|
| `LinRxTask` | LIN command/status handling | Event | High | TBD | short only |
| `AmbientTask` | sensor sampling | 50~100 ms 후보 | Normal | TBD | timeout bounded |
| `LightingTask` | lamp output update | Event/10~20 ms 후보 | Normal/High | TBD | no logging block |
| `StatusTask` | LIN response data 준비 | Event/schedule | Normal | TBD | bounded |
| `HealthTask` | local health/watchdog | 100 ms 후보 | Low | TBD | bounded |

## 7.3 ISR Map

| Interrupt | ISR Responsibility | Wake-up Target | Mechanism |
|---|---|---|---|
| FDCAN RX | frame metadata/copy 최소 처리 | `CanRxTask` | Queue/Notification |
| FDCAN Error | state 저장 | `HealthTask` | Event/Notification |
| LIN/UART RX/TX | byte/frame/error state 최소 처리 | `LinRxTask`/`LinScheduleTask` | Notification/Queue |
| ADC/I2C complete 후보 | sample ready flag | `AmbientTask` | Notification |

ISR에서는 mapping, filtering, lamp logic, printf를 수행하지 않는다.

## 7.4 RTOS Objects / IPC

| Object | Type | Producer | Consumer | Data | Overflow Policy |
|---|---|---|---|---|---|
| `CanRxQueue` | Queue | CAN ISR | CanRxTask | raw CAN frame | counter + bounded drop policy TBD |
| `BodyCommandQueue` | Queue | CanRxTask | GatewayMappingTask | logical request | latest/event policy TBD |
| `LinCommandState` | Queue/Repository | GatewayMappingTask | LinScheduleTask | command signals | latest state |
| `LinStatusQueue` | Queue | LIN path | GatewayMappingTask | status/fault | bounded |
| `CanTxQueue` | Queue | Mapping/Health | CanTxTask | status/event | fault priority 고려 |

## 7.5 Shared Resource Ownership

| Resource | Owner | Protection |
|---|---|---|
| FDCAN TX | `CanTxTask` | single-owner queue |
| LIN Master peripheral | `LinScheduleTask` | single owner |
| Lamp GPIO/PWM | `LightingTask` | single owner |
| Ambient sensor bus | `AmbientTask` | single owner or mutex if shared |

---

# 8. Runtime View

## 8.1 CAN → LIN

```mermaid
sequenceDiagram
    participant CAN as FDCAN ISR
    participant CRX as CanRxTask
    participant MAP as GatewayMappingTask
    participant LIN as LinScheduleTask
    participant SL as LinRxTask/LightingTask
    CAN->>CRX: Body_Command frame
    CRX->>MAP: logical body request
    MAP->>LIN: LIN signal state
    LIN->>SL: Lamp_Command frame
    SL->>SL: update Lamp output
```

## 8.2 LIN → CAN

```mermaid
sequenceDiagram
    participant AMB as AmbientTask
    participant SL as StatusTask
    participant LIN as LinScheduleTask
    participant MAP as GatewayMappingTask
    participant CAN as CanTxTask
    AMB->>SL: ambient value + valid
    LIN->>SL: Ambient_Status slot
    SL-->>LIN: LIN response
    LIN->>MAP: logical LIN status
    MAP->>CAN: Body_Status
```

## 8.3 Slave Timeout

```mermaid
sequenceDiagram
    participant LIN as LinScheduleTask
    participant H as HealthTask
    participant TX as CanTxTask
    LIN--xLIN: no slave response
    LIN->>H: timeout/error counter
    H->>H: slave invalid
    H->>TX: Body fault / DTC event
```

---

# 9. Deployment / Hardware View

```text
CANH/L
  ↓
CAN FD Transceiver
  ↓
Gateway STM32 + FreeRTOS
  ↓ UART/LIN
LIN Transceiver
  ↓
LIN Bus
  ↓
LIN Transceiver
  ↓
Slave STM32 + FreeRTOS
  ├ Ambient Sensor
  └ Lamp Driver / LED
```

| HW | SW | Interface | Note |
|---|---|---|---|
| Gateway STM32 | CAN/LIN Gateway tasks | FDCAN, UART/LIN | model TBD |
| Slave STM32 | Ambient/Lighting tasks | UART/LIN, ADC/I2C, GPIO/PWM | model TBD |
| CAN FD Transceiver | physical CAN | CANH/L | TBD |
| LIN Transceivers | physical LIN | LIN | Gateway/Slave 각각 필요 |

Pin map은 실제 board schematic/CubeMX 후 작성한다.

---

# 10. Interfaces & Contracts

## 10.1 CAN

### RX
| Message | Meaning | Sender | Timeout | Action |
|---|---|---|---|---|
| `Body_Command` | lamp/body request | VCU/H735 | TBD | request invalid/hold policy TBD |

### TX
| Message | Meaning | Receiver | Cycle/Event |
|---|---|---|---|
| `Body_Status` | ambient/lamp/LIN health | VCU/H735/HPC | periodic TBD |
| `DTC_Event` | Body fault | Pi/H735 | event |
| `ECU_Heartbeat` | gateway alive | VCU/HPC | periodic TBD |

## 10.2 LIN

| Frame | Publisher | Subscriber | Period | Fault |
|---|---|---|---|---|
| `Lamp_Command` | Gateway Master | Slave | TBD | invalid/no response |
| `Ambient_Status` | Slave | Gateway | TBD | timeout/checksum |
| `Lamp_Status` | Slave | Gateway | TBD | timeout/checksum |
| `Lamp_Diagnostic` | Slave | Gateway | TBD | timeout/checksum |

## 10.3 CAN ↔ LIN Mapping

| CAN | Direction | LIN | Conversion |
|---|---|---|---|
| `HEADLAMP_REQ` | CAN→LIN | `LAMP_HEAD_CMD` | boolean/enum |
| `TURN_LEFT_REQ` | CAN→LIN | `LAMP_LEFT_CMD` | boolean |
| `TURN_RIGHT_REQ` | CAN→LIN | `LAMP_RIGHT_CMD` | boolean |
| `BRAKE_LAMP_REQ` | CAN→LIN | `LAMP_BRAKE_CMD` | boolean |
| `BODY_AMBIENT` | LIN→CAN | `AMBIENT_VALUE` | scale TBD |
| `BODY_LAMP_STATUS` | LIN→CAN | `LAMP_STATUS` | flags |
| `DTC_Event` candidate | LIN→CAN | `LAMP_FAULT` | fault map |

---

# 11. Data & State Model

## 11.1 Main Data

| Data | Owner | Meaning | Invalid Condition |
|---|---|---|---|
| `body_command` | VCU/H735 | desired lamp states | CAN timeout/invalid |
| `ambient_value` | LIN Slave | measured ambient | sensor invalid |
| `lamp_state` | LIN Slave | actual logical lamp state | output/local fault |
| `lin_slave_valid` | Gateway | node health | response timeout |
| `body_status` | Gateway | mapped CAN status | source invalid |

## 11.2 Gateway State

```text
INIT → READY → ACTIVE
            ↘ LIN_FAULT
            ↘ CAN_FAULT
```

Fault state라고 전체 gateway가 반드시 정지하는 것은 아니다. CAN과 LIN 장애를 각각 degraded mode로 관리한다.

---

# 12. Cross-cutting Concepts

## Error Handling
- LIN timeout은 slave invalid로 표시
- CAN bus-off와 LIN timeout을 서로 독립적으로 관리
- invalid ambient를 정상 최신값처럼 전달하지 않음

## Diagnostics
- LIN node timeout
- ambient sensor fault
- lamp/output mismatch 후보
- CAN bus error
- queue/task/stack software health

## Timing
| Function | Period / Trigger | Target |
|---|---|---|
| LIN schedule | slot TBD | jitter TBD |
| Ambient sample | 50~100 ms 후보 | TBD |
| Body status CAN TX | TBD | TBD |
| Lighting response | event | TBD |

## Memory / Stack
- stack high-water 측정
- queue depth 실측 후 확정
- startup 이후 불필요한 heap allocation 최소화

## Watchdog
```text
Gateway critical tasks / Slave critical tasks
→ HealthTask
→ all healthy?
→ IWDG refresh
```

---

# 13. Architecture Decisions

| ADR ID | Decision | Alternative | Reason | Consequence |
|---|---|---|---|---|
| ADR-BODY-001 | STM32 #3를 CAN↔LIN Gateway/LIN Master로 사용 | 별도 Gateway MCU 추가 | 보드 수 절감, 역할 명확 | Gateway SW 책임 증가 |
| ADR-BODY-002 | STM32 #4를 LIN Slave로 사용 | Lamp/Ambient를 Gateway에 직접 연결 | LIN 실습/Local node 구조 | MCU 2개 필요 |
| ADR-BODY-003 | Lamp physical output은 Slave가 소유 | H735/Gateway 직접 GPIO | local ownership | LIN feedback 필요 |
| ADR-BODY-004 | LIN schedule dedicated task | super-loop | timing 관리 | task/priority 관리 필요 |
| ADR-BODY-005 | CAN TX single-owner task | 여러 task direct TX | contention 감소 | TX queue 필요 |

---

# 14. Quality Scenarios & Verification

| Quality | Scenario | Target | Verification |
|---|---|---|---|
| Reliability | Slave disconnect | Gateway hang 없음, fault 표시 | disconnect test |
| Timing | LIN schedule | period/jitter TBD 충족 | logic analyzer |
| Isolation | CAN bus failure | LIN task deadlock 없음 | fault injection |
| Isolation | LIN failure | CAN node alive 유지 | fault injection |
| RTOS health | soak test | stack/queue overflow 0 | runtime stats |

---

# 15. Risks & Technical Debt

| ID | Risk | Impact | Mitigation | Owner |
|---|---|---|---|---|
| RISK-BODY-001 | 실제 STM32 FDCAN 지원 미확정 | Gateway HW 변경 가능 | 보드 datasheet 확인 | D |
| RISK-BODY-002 | LIN transceiver 미선정 | 통신 지연 | voltage/part 조기 확정 | D |
| RISK-BODY-003 | Lamp load 정격 미확정 | GPIO/driver 손상 위험 | driver 회로/부하 확인 | D |
| RISK-BODY-004 | LIN schedule 미확정 | timing 재작업 | signal/frame 먼저 freeze | D/F |
| RISK-BODY-005 | 작은 Slave MCU RAM 부족 가능 | FreeRTOS 적용 어려움 | RAM/stack 측정 후 ADR로 예외 검토 | D |

---

# 16. Requirement Traceability

| Requirement | Component | Task | Interface | Test |
|---|---|---|---|---|
| REQ-BODY-001 | CanRxAdapter | CanRxTask | CAN RX | T-BODY-001 |
| REQ-BODY-002 | GatewayMapper | GatewayMappingTask | CAN↔LIN | T-BODY-002 |
| REQ-BODY-003 | LinScheduleManager | LinScheduleTask | LIN | T-BODY-003 |
| REQ-BODY-004 | AmbientManager | AmbientTask | ADC/I2C | T-BODY-004 |
| REQ-BODY-005 | LightingManager | LightingTask | GPIO/PWM | T-BODY-005 |
| REQ-BODY-008 | HealthManager | HealthTask | LIN status | T-BODY-008 |
| REQ-BODY-011 | RTOS Health | HealthTask | RTOS | T-BODY-011 |

---

# 17. Glossary

| Term | Meaning |
|---|---|
| LIN Master | schedule/header를 시작하는 LIN node |
| LIN Slave | Master 요청에 응답하는 local node |
| Gateway | 서로 다른 network/signal을 변환/중계하는 node |
| Transceiver | MCU logic과 physical bus를 연결하는 회로 |
| DTC | Diagnostic Trouble Code |

---

# 18. Review Checklist

- [ ] Gateway와 Slave 역할이 분리되어 있다.
- [ ] CAN↔LIN mapping이 명시되어 있다.
- [ ] LIN schedule owner가 명확하다.
- [ ] ISR과 Task 책임이 분리되어 있다.
- [ ] CAN/LIN physical transceiver가 고려되어 있다.
- [ ] Lamp output owner가 Slave로 명확하다.
- [ ] timeout/fault/degraded behavior가 있다.
- [ ] RTOS queue/stack/watchdog 정책이 있다.
- [ ] Requirement→Task→Test 추적이 가능하다.
