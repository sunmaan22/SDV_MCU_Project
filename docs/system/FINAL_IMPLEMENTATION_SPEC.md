# Final Implementation Specification

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> **Status:** PARTIAL FREEZE — 2026-09-11 / Implementation Baseline v1.0 미도달
> **Decision Authority:** Project Owner  
> **Purpose:** 코드 작성 직전 모든 공통 Hardware / CAN / LIN / RTOS / State / DTC 값을 한 곳에서 확정하는 최종 명세서

이 문서는 프로젝트의 **최상위 구현 계약(Source of Truth)** 이다.

2026-09-11 Owner의 “H735 제외 STM 보드는 G431KB 구매 완료, 근거가 있으면 freeze” 지시에 따라
`DEC-HW-001~005`, `DEC-HW-021~023`만 이번에 동결했다. 구매 결정과 기존 H735 시험 기록을 근거로 하며,
새 실기 시험을 수행한 것은 아니다. 상세 근거와 다음 동결 조건은 [Freeze Review](FREEZE_REVIEW_2026-09-11.md)를 따른다.

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

충돌이 발생하면 우선순위는 다음과 같다.

```text
1. FINAL_IMPLEMENTATION_SPEC.md
2. 역할별 SPECIFICATION.md
3. 역할별 ARCHITECTURE.md
4. README / 예시 / 과거 문서
```

`docs/archive/`는 구현 기준으로 사용하지 않는다.

---

# 1. 이미 고정된 시스템 규칙

아래 항목은 다시 토론하지 않고 구현 기준으로 사용한다.

| 영역 | 고정 규칙 |
|---|---|
| A Ultrasonic | 거리 / valid / warning / local fault owner |
| B H735 | UI 표시 + 사용자 Request 생성 |
| C Drive | Motor / Servo 실제 actuator output owner |
| D Gateway | CAN↔LIN mapping + LIN Master schedule owner |
| D Slave | Ambient + Lamp actual state owner |
| E Vision | Vision semantic result + ADAS high-level request owner |
| F VCU | 최종 vehicle arbitration + Final Drive / Body Command owner |
| Pi DTC | DTC History DB / timestamp / count / storage owner |

## 1.1 고정 Message Publisher

| Logical Message | Publisher | Consumer |
|---|---|---|
| `Ultrasonic_Status` | A | F, B, E |
| `Vision_Status` | E | F, B |
| `ADAS_Request` | E | F |
| `Final_Drive_Command` | F | C |
| `Drive_Status` | C | F, B, E |
| `Body_User_Request` | B | F |
| `Body_Command` | F | D Gateway |
| `Body_Status` | D Gateway | F, B, E |
| `Vehicle_State` | F | All |
| `Driver_Input` | F | B, E |
| `DTC_Event` | 각 Local Node | F, Pi, B 필요 시 |
| `ECU_Heartbeat` | 각 Node | F, Pi |

같은 최종 Message를 두 Node가 동시에 publish하지 않는다.

## 1.2 고정 Safety Rule

```text
E-Stop / Critical Fault
> Ultrasonic Parking Critical
> ADAS Safety Request
> Normal Driver Request
```

- Rear Vision은 Ultrasonic `CRITICAL`을 해제하지 않는다.
- `valid=false`인 센서값은 정상 판단에 사용하지 않는다.
- `SafetyTask`는 final command를 직접 쓰지 않는다.
- `VcuControlTask`만 `final_command`의 writer다.
- Drive ECU가 `Final_Drive_Command` timeout을 감지한다.
- VCU는 `Drive_Status`, Heartbeat, Peer Message timeout을 감지한다.
- Brake와 Accelerator가 동시에 유효하게 입력되면 Brake 우선을 기본 정책으로 한다.
- D↔R은 차량이 움직이는 상태에서 즉시 반전하지 않는다.

---

# 2. 실행 환경 고정

```text
STM32 Node
→ FreeRTOS + CMSIS-RTOS2 기본

Raspberry Pi
→ Linux Service / Process / Thread
```

공통 MCU 규칙:
- ISR은 timestamp / counter / flag / notification 등 최소 처리만 한다.
- Control / Safety Task에서 blocking log 금지.
- Queue / Notification / Event / single-owner data 구조를 우선한다.
- Task period / jitter / stack high-water / queue overflow / watchdog을 측정한다.
- 실제 numeric priority / stack / queue depth는 이 문서에서 Owner가 최종 Freeze한다.

---

# 3. Owner Decision Registry

아래 값은 **Project Owner가 직접 확정**한다. 담당자나 AI가 독자적으로 최종값을 바꾸지 않는다.

Status는 `OPEN / FROZEN` 중 하나를 사용한다.

## 3.1 Hardware Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-HW-001` | A Ultrasonic STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-002` | C Drive STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-003` | D Gateway STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-004` | D LIN Slave STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-005` | F VCU STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-006` | CAN FD Transceiver 모델 | OWNER INPUT | OPEN |
| `DEC-HW-007` | LIN Transceiver 모델 | OWNER INPUT | OPEN |
| `DEC-HW-008` | Pi CAN FD Interface | OWNER INPUT | OPEN |
| `DEC-HW-009` | Ultrasonic Sensor 모델/개수 | OWNER INPUT | OPEN |
| `DEC-HW-010` | Motor 모델 | OWNER INPUT | OPEN |
| `DEC-HW-011` | Motor Driver | OWNER INPUT | OPEN |
| `DEC-HW-012` | Encoder/Hall | OWNER INPUT | OPEN |
| `DEC-HW-013` | RC Servo | OWNER INPUT | OPEN |
| `DEC-HW-014` | Ambient Sensor | OWNER INPUT | OPEN |
| `DEC-HW-015` | Front Camera | OWNER INPUT | OPEN |
| `DEC-HW-016` | Rear Camera | OWNER INPUT | OPEN |
| `DEC-HW-017` | Accelerator Sensor | OWNER INPUT | OPEN |
| `DEC-HW-018` | Brake Sensor | OWNER INPUT | OPEN |
| `DEC-HW-019` | Steering Input Sensor | OWNER INPUT | OPEN |
| `DEC-HW-020` | E-Stop 입력 방식 | OWNER INPUT | OPEN |
| `DEC-HW-021` | B IVI 보드 | STM32H735G-DK | FROZEN |
| `DEC-HW-022` | B CAN peripheral / 핀 예약 | FDCAN2, PB5 RX / PB6 TX (설계 배정; 외부 통신 검증 미완료) | FROZEN |
| `DEC-HW-023` | B Display / Touch / 외부 메모리 역할 | LTDC RGB888 / BSP I2C4 touch / OCTOSPI1 NOR asset @ 0x90000000 / OCTOSPI2 HyperRAM framebuffer @ 0x70000000 | FROZEN |

동결 범위:
- G431KB는 **MCU 모델 선택** 동결이다. NUCLEO 정품 여부, 제조사/보드 revision/실장 MCU 전체 품번은 구매 실물과 회로도로 기록한다. NUCLEO-G431KB 핀맵을 다른 G431KB 보드에 자동 적용하지 않는다.
- 모델 선택은 자원·배선 적합성 시험 PASS를 뜻하지 않는다. G431KB의 역할별 Pin/Timer/ADC/UART/FDCAN 배정, 메모리 예산과 실기 검증은 Gate A/D에 남는다.
- B의 핀 예약은 현재 `.ioc` 및 [PIN_MAP](../ecus/IVI/PIN_MAP.md) 기준이다. Internal loopback은 PB5/PB6의 외부 전기 경로를 검증하지 않는다.
- `DEC-HW-023`은 메모리 역할/주소 기반 동결이다. 전체 MPU/cache 설정, 모든 clock/timing, GUI frame budget, RTOS stack/queue 수치는 동결하지 않는다.

## 3.2 Network Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-NET-001` | CAN nominal bitrate | OWNER INPUT | OPEN |
| `DEC-NET-002` | CAN FD data bitrate | OWNER INPUT | OPEN |
| `DEC-NET-003` | CAN sample / mode parameters | OWNER INPUT | OPEN |
| `DEC-NET-004` | Message ID allocation | OWNER INPUT | OPEN |
| `DEC-NET-005` | DLC / payload layout | OWNER INPUT | OPEN |
| `DEC-NET-006` | Signal endian / signedness | OWNER INPUT | OPEN |
| `DEC-NET-007` | Message cycle / timeout | OWNER INPUT | OPEN |
| `DEC-NET-008` | Heartbeat node ID 방식 | OWNER INPUT | OPEN |
| `DEC-NET-009` | LIN bitrate | OWNER INPUT | OPEN |
| `DEC-NET-010` | LIN frame ID | OWNER INPUT | OPEN |
| `DEC-NET-011` | LIN checksum | OWNER INPUT | OPEN |
| `DEC-NET-012` | LIN schedule / slot period | OWNER INPUT | OPEN |

## 3.3 Vehicle / Control Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-CTRL-001` | Vehicle State Machine | OWNER INPUT | OPEN |
| `DEC-CTRL-002` | READY 조건 | OWNER INPUT | OPEN |
| `DEC-CTRL-003` | Drive Enable 조건 | OWNER INPUT | OPEN |
| `DEC-CTRL-004` | D↔R 전환 허용 조건 | OWNER INPUT | OPEN |
| `DEC-CTRL-005` | Stop speed threshold | OWNER INPUT | OPEN |
| `DEC-CTRL-006` | E-Stop recovery 정책 | OWNER INPUT | OPEN |
| `DEC-CTRL-007` | Accelerator calibration/deadband | OWNER INPUT | OPEN |
| `DEC-CTRL-008` | Brake calibration/deadband | OWNER INPUT | OPEN |
| `DEC-CTRL-009` | Brake-over-Accelerator threshold | OWNER INPUT | OPEN |
| `DEC-CTRL-010` | Steering input range/calibration | OWNER INPUT | OPEN |
| `DEC-CTRL-011` | Ultrasonic SAFE/WARNING/CRITICAL action | OWNER INPUT | OPEN |
| `DEC-CTRL-012` | ADAS arbitration rule | OWNER INPUT | OPEN |
| `DEC-CTRL-013` | Final speed unit/range | OWNER INPUT | OPEN |
| `DEC-CTRL-014` | Final steering unit/range | OWNER INPUT | OPEN |
| `DEC-CTRL-015` | Drive command timeout | OWNER INPUT | OPEN |
| `DEC-CTRL-016` | Steering timeout action | OWNER INPUT | OPEN |
| `DEC-CTRL-017` | Encoder invalid fallback | OWNER INPUT | OPEN |
| `DEC-CTRL-018` | PID 적용 여부 / tuning policy | OWNER INPUT | OPEN |

## 3.4 Perception / Vision Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-PER-001` | Ultrasonic zone / 장착 위치 | OWNER INPUT | OPEN |
| `DEC-PER-002` | Ultrasonic filter | OWNER INPUT | OPEN |
| `DEC-PER-003` | Ultrasonic threshold / hysteresis | OWNER INPUT | OPEN |
| `DEC-PER-004` | Ultrasonic scan period / gap | OWNER INPUT | OPEN |
| `DEC-VIS-001` | Front Vision Stage 기능 | OWNER INPUT | OPEN |
| `DEC-VIS-002` | Rear Vision Stage 기능 | OWNER INPUT | OPEN |
| `DEC-VIS-003` | Camera resolution / FPS | OWNER INPUT | OPEN |
| `DEC-VIS-004` | Vision algorithm/model | OWNER INPUT | OPEN |
| `DEC-VIS-005` | Vision freshness timeout | OWNER INPUT | OPEN |
| `DEC-VIS-006` | Gear D/R camera switching | OWNER INPUT | OPEN |
| `DEC-VIS-007` | Gear R → first valid result latency | OWNER INPUT | OPEN |

## 3.5 HMI / Body Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-HMI-001` | Cluster 필수 표시항목 | OWNER INPUT | OPEN |
| `DEC-HMI-002` | Warning 표시 우선순위 | OWNER INPUT | OPEN |
| `DEC-HMI-003` | Gear R Parking 화면 정책 | OWNER INPUT | OPEN |
| `DEC-HMI-004` | DTC Clear 구현 여부 | OWNER INPUT | OPEN |
| `DEC-HMI-005` | TouchGFX update/frame budget | OWNER INPUT | OPEN |
| `DEC-BODY-001` | 구현 Lamp 범위 | OWNER INPUT | OPEN |
| `DEC-BODY-002` | Body_User_Request 기능 범위 | OWNER INPUT | OPEN |
| `DEC-BODY-003` | Ambient 단위/filter/calibration | OWNER INPUT | OPEN |

## 3.6 DTC / Health Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-DTC-001` | DTC code numbering | OWNER INPUT | OPEN |
| `DEC-DTC-002` | DTC status enum | OWNER INPUT | OPEN |
| `DEC-DTC-003` | DTC severity enum | OWNER INPUT | OPEN |
| `DEC-DTC-004` | confirmation / clear rule | OWNER INPUT | OPEN |
| `DEC-DTC-005` | Critical DTC → Safe Action mapping | OWNER INPUT | OPEN |
| `DEC-HLT-001` | Heartbeat period / timeout | OWNER INPUT | OPEN |
| `DEC-HLT-002` | Watchdog refresh condition | OWNER INPUT | OPEN |
| `DEC-HLT-003` | RTOS task priority / period / stack / queue depth | OWNER INPUT | OPEN |

---

# 4. Final Logical Message Contracts

> 실제 CAN ID / bit position은 Owner Freeze 후 이 표에 직접 기록한다. 하위 문서는 이 표를 복제해 새 값을 만들지 않는다.

## 4.1 `Ultrasonic_Status`

| Field | Unit / Type | Range | Valid Rule | Final |
|---|---|---|---|---|
| sensor/zone id | TBD | TBD | defined ID | OWNER INPUT |
| distance | mm 후보 | TBD | `valid=true` | OWNER INPUT |
| valid | bool | 0/1 | source health | OWNER INPUT |
| warning_level | enum | SAFE/WARNING/CRITICAL | valid only | OWNER INPUT |
| fault_flags | bitfield | TBD | local fault | OWNER INPUT |
| freshness/sequence | TBD | TBD | monotonic | OWNER INPUT |

## 4.2 `Vision_Status`

| Field | Unit / Type | Final |
|---|---|---|
| front_valid | bool | OWNER INPUT |
| rear_valid | bool | OWNER INPUT |
| lane result | TBD | OWNER INPUT |
| object result | TBD | OWNER INPUT |
| rear parking result | TBD | OWNER INPUT |
| vision warning | enum | OWNER INPUT |
| freshness/sequence | TBD | OWNER INPUT |

## 4.3 `ADAS_Request`

| Field | Unit / Type | Final |
|---|---|---|
| request_valid | bool | OWNER INPUT |
| requested_speed | TBD | OWNER INPUT |
| requested_steering | TBD | OWNER INPUT |
| request_reason/type | enum | OWNER INPUT |
| freshness/sequence | TBD | OWNER INPUT |

## 4.4 `Final_Drive_Command`

| Field | Unit / Type | Final |
|---|---|---|
| drive_enable | bool | OWNER INPUT |
| speed_request | TBD | OWNER INPUT |
| steering_request | TBD | OWNER INPUT |
| gear/direction | enum | OWNER INPUT |
| command_valid | bool | OWNER INPUT |
| sequence | TBD | OWNER INPUT |

## 4.5 `Drive_Status`

| Field | Unit / Type | Final |
|---|---|---|
| motor_rpm | rpm | OWNER INPUT |
| vehicle_speed | TBD | OWNER INPUT |
| steering_target | TBD | OWNER INPUT |
| steering_actual | optional/TBD | OWNER INPUT |
| command_valid | bool | OWNER INPUT |
| fault_flags | bitfield | OWNER INPUT |

## 4.6 `Body_User_Request`

| Field | Unit / Type | Final |
|---|---|---|
| request_type | enum | OWNER INPUT |
| requested_value | TBD | OWNER INPUT |
| request_valid | bool | OWNER INPUT |

## 4.7 `Body_Command`

| Field | Unit / Type | Final |
|---|---|---|
| headlamp | bool/enum | OWNER INPUT |
| tail_lamp | bool/enum | OWNER INPUT |
| brake_lamp | bool/enum | OWNER INPUT |
| turn_left | bool | OWNER INPUT |
| turn_right | bool | OWNER INPUT |
| hazard | bool | OWNER INPUT |

## 4.8 `Body_Status`

| Field | Unit / Type | Final |
|---|---|---|
| ambient | TBD | OWNER INPUT |
| lamp_status | bitfield | OWNER INPUT |
| lin_health | enum/flags | OWNER INPUT |
| fault_flags | bitfield | OWNER INPUT |

## 4.9 `Vehicle_State`

| Field | Unit / Type | Final |
|---|---|---|
| gear | P/R/N/D enum | OWNER INPUT |
| ready | bool | OWNER INPUT |
| vehicle_mode | enum | OWNER INPUT |
| safety_state | enum | OWNER INPUT |

## 4.10 `DTC_Event`

| Field | Type | Final |
|---|---|---|
| source_node | enum/id | OWNER INPUT |
| code | integer/enum | OWNER INPUT |
| status | enum | OWNER INPUT |
| severity | enum | OWNER INPUT |
| sequence/timestamp | TBD | OWNER INPUT |

## 4.11 `ECU_Heartbeat`

| Field | Type | Final |
|---|---|---|
| node_id | enum/id | OWNER INPUT |
| alive_counter | counter | OWNER INPUT |
| health_flags | bitfield | OWNER INPUT |

---

# 5. LIN Contract

## 5.1 Frame Set

| LIN Frame | Publisher | Subscriber | Period | ID | Checksum |
|---|---|---|---|---|---|
| `Lamp_Command` | Gateway | Slave | OWNER INPUT | OWNER INPUT | OWNER INPUT |
| `Ambient_Status` | Slave | Gateway | OWNER INPUT | OWNER INPUT | OWNER INPUT |
| `Lamp_Status` | Slave | Gateway | OWNER INPUT | OWNER INPUT | OWNER INPUT |
| `Lamp_Diagnostic` | Slave | Gateway | OWNER INPUT | OWNER INPUT | OWNER INPUT |

## 5.2 CAN ↔ LIN Mapping

| CAN Signal | LIN Signal | Mapping / Scale | Final |
|---|---|---|---|
| Body_Command headlamp | LAMP_HEAD_CMD | TBD | OWNER INPUT |
| Body_Command turn left/right | LAMP_TURN_CMD | TBD | OWNER INPUT |
| Body_Command brake | LAMP_BRAKE_CMD | TBD | OWNER INPUT |
| Ambient_Status | Body_Status ambient | TBD | OWNER INPUT |
| Lamp_Status | Body_Status lamp_status | TBD | OWNER INPUT |

---

# 6. RTOS / Linux Final Execution Contract

각 MCU 역할은 최종 코드 작성 전에 아래 표의 `Final Period / Priority / Stack / Queue`를 Owner가 Freeze한다.

| Node | Critical Tasks |
|---|---|
| A | UltrasonicTask, PerceptionTask, CanTxTask, HealthTask |
| B | CanRxTask, VehicleModelTask, GuiTask, CommandTxTask, HealthTask |
| C | CanRxTask, ControlTask, FeedbackTask, StatusTask, HealthTask |
| D Gateway | CanRxTask, GatewayMappingTask, LinScheduleTask, CanTxTask, HealthTask |
| D Slave | LinRxTask, AmbientTask, LightingTask, StatusTask, HealthTask |
| F | SafetyTask, VcuControlTask, DriverInputTask, CanRxTask, CanTxTask, DiagnosticTask, HealthTask |

Linux E Node는 다음을 Freeze한다.

| Service | Owner Decision |
|---|---|
| front_vision | lifetime / mode / restart |
| rear_vision | lifetime / mode / restart |
| vehicle_manager | state ownership |
| can_service | CAN single owner |
| health_monitor | timeout / restart condition |
| logger | storage / rotation / blocking policy |

---

# 7. Coding Gate

## Gate A: Hardware Ready

코드의 Hardware Layer를 확정하기 전에:
- [x] 모든 STM32 모델 확정 (2026-09-11: A/C/D Gateway/D Slave/F G431KB, B H735G-DK)
- [ ] FDCAN 지원 확인
- [ ] CAN/LIN Transceiver 확정
- [ ] Sensor/Actuator 모델 확정
- [ ] Pin/Timer/ADC/UART/FDCAN peripheral 확정

## Gate B: Interface Freeze

ECU 간 통합 코드 작성 전에:
- [ ] 모든 Logical Message field 확정
- [ ] CAN ID/DLC/bit layout 확정
- [ ] Unit/Scale/Offset/Range 확정
- [ ] Cycle/Timeout 확정
- [ ] Heartbeat/DTC 확정
- [ ] LIN Schedule/Mapping 확정

## Gate C: Control Freeze

VCU/Drive 제어 코드 작성 전에:
- [ ] Vehicle State Machine 확정
- [ ] READY/Enable 조건 확정
- [ ] Arbitration Rule 확정
- [ ] E-Stop Recovery 확정
- [ ] D↔R 조건 확정
- [ ] Command Timeout/Safe State 확정

## Gate D: RTOS Freeze

통합 빌드 전에:
- [ ] Task period / priority 확정
- [ ] Stack / Queue depth 확정
- [ ] Watchdog refresh condition 확정
- [ ] ISR→Task path 확정
- [ ] blocking logging 제거 확인

`Gate B`가 끝나기 전에는 담당자가 임의의 CAN ID나 bit position을 코드에 영구 상수로 박지 않는다.

## 7.1 단계별 동결 시점

동결은 전 항목을 한 번에 완료하는 행사가 아니다. **의존하는 구현을 확정하기 직전**, 해당 범위의 값과 검증 근거를 동결한다.
`FROZEN`은 선택한 설계 계약이며 `TEST PASS`와 별개다. Gate A~D는 현재 모두 미완료다.

| 시점 | 동결 범위 | 완료 조건 |
|---|---|---|
| 지금 | 구매 모델 + 검증된 B 기반 | 위 Hardware 8개 결정; 기존 §1/§2 역할·실행 원칙 유지 |
| Week 1, 역할별 Hardware Layer 확정 전 | Gate A: transceiver, sensor/actuator, pin/peripheral | 실제 보드/부품 회로도, 전압/정격, 핀 중복과 timer channel/AF 검토, 최소 bring-up 기록 |
| Week 1 말~Week 2 첫 ECU pair 통합 전 | Gate B: Network + 전체 message/LIN + DTC/Heartbeat 계약 | ID 중복 없음, field/단위/범위/invalid/enum/byte layout 완비, 양쪽 encode/decode 일치; 물리 bitrate/mode는 실제 H735↔G431 및 LIN pair 검증 |
| VCU/Drive 실제 출력 제어 확정 전 | Gate C: State/Enable/Arbitration/Timeout/Safe Action | 상태·전이·복구·stale/invalid 처리를 결정하고 수치 근거 및 bench 검증 기록; 먼저 mock/출력 비활성 시험 가능 |
| Week 2 계측 후~Week 3 통합 baseline 빌드 전 | Gate D: RTOS/Linux 자원·주기·watchdog | 대표 통신/GUI/제어 부하에서 period/jitter, stack high-water, queue 최대 점유/overflow, starvation, fault 경로 측정 |
| 관련 기능 통합 전, 늦어도 전체 baseline 직전 | Perception/Vision/HMI/Body 잔여 결정 | 실제 장착·보정·baseline 계측 후 수치와 기능 범위 결정, 시험의 Target/Expected에서 TBD 제거 |

주차는 [WEEKLY_PLAN](../getting_started/WEEKLY_PLAN.md)의 목표이며 달력 경과만으로 gate를 통과하지 않는다.
Bring-up, RTOS skeleton, mock, 계측용 bench 코드는 OPEN 값으로도 작성할 수 있다.
이때 값은 `BENCH ONLY / NOT FROZEN` 설정으로 분리하고 최종 통합 상수나 PASS 근거로 승격하지 않는다.
Gate D 이전의 계측용 통합 빌드는 허용하되 최종 통합 baseline으로 취급하지 않는다.

Gate B 추가 완결성 확인:
- §1.1의 `Driver_Input`도 필수 계약이다. 현재 §4에 상세 표가 없으므로 필드·payload·주기·timeout 표를 추가한 뒤에야 Gate B를 닫는다.
- DTC/Heartbeat처럼 다중 publisher인 메시지는 노드 식별·ID 할당/충돌 회피까지 정의한다.
- §6의 Task 이름 목록만으로 Gate D를 닫지 않는다. Node별 실제 Task/Period/Priority/Stack(bytes)/Queue(depth와 item bytes)/Watchdog 조건을 기록해야 한다.

---

# 8. 역할별 코드 작성 기준

## A Ultrasonic

```text
Driver → Measurement → Perception → Repository → CAN
```

코드 전에 `DEC-HW-009`, `DEC-PER-001~004`, `Ultrasonic_Status`가 FROZEN이어야 한다.

## B IVI

```text
CAN → Repository → TouchGFX
Touch → Body_User_Request
```

View에서 CAN Driver를 직접 호출하지 않는다.

## C Drive

```text
Final_Drive_Command → Validation → Control → PWM/DIR/Servo
Encoder → Feedback → Status
```

Motor driver rating과 command timeout/safe state가 FROZEN이어야 한다.

## D Body

```text
Body_Command → CAN↔LIN Mapping → LIN → Lamp
Ambient/Lamp Status → LIN → CAN Body_Status
```

LIN schedule은 Gateway만 소유한다.

## E HPC

```text
Camera → Vision → Vision_Status / ADAS_Request → can_service
```

Raw image는 CAN으로 보내지 않는다.

## F VCU

```text
Driver + Perception + Status + Fault
→ Validation/Freshness
→ Safety/Arbitration
→ Final_Drive_Command / Body_Command
```

Final command writer는 `VcuControlTask` 하나다.

---

# 9. AI / 담당자 수정 규칙

AI에게 문서를 넘길 때 반드시 이 규칙을 같이 준다.

```text
FINAL_IMPLEMENTATION_SPEC.md가 최상위 규칙이다.
고정된 Publisher/Owner/역할 경계를 변경하지 마라.
OWNER INPUT 또는 OPEN 상태의 값을 임의로 최종 확정하지 마라.
Project Owner가 승인한 값만 FROZEN으로 바꿔라.
역할별 내부 구현은 제안할 수 있지만 ECU 간 Interface는 이 문서를 따른다.
충돌 시 FINAL_IMPLEMENTATION_SPEC.md를 우선한다.
```

---

# 10. Final Freeze Checklist

검증용 코드로 필요한 근거를 확보한 뒤, 최종 구현 baseline을 확정하기 전 Project Owner 확인:

- [ ] Hardware Decision 전부 FROZEN
- [ ] Network Decision 전부 FROZEN
- [ ] Vehicle/Control Decision 전부 FROZEN
- [ ] Perception/Vision Decision 전부 FROZEN
- [ ] HMI/Body Decision 전부 FROZEN
- [ ] DTC/Health Decision 전부 FROZEN
- [ ] CAN Message Contract 전부 FROZEN
- [ ] LIN Contract 전부 FROZEN
- [ ] RTOS/Linux Execution Contract 전부 FROZEN

모든 핵심 항목이 FROZEN된 시점을 **Implementation Baseline v1.0**으로 태그/커밋한다.

현재는 일부 Hardware 결정만 동결되었으므로 v1.0 태그를 만들지 않는다.
각 동결 기록에는 결정 ID, 날짜, Owner 지시/승인, 근거 문서·시험 대상 소스 SHA, 적용 범위와 미검증 범위를 남긴다.
동결 후 변경은 사유·영향 ECU/메시지·재시험 범위·Owner 승인을 기록하고 새 revision으로 반영한다.
Baseline v1.0은 구현 계약의 동결이며 차량 전체 시험 PASS나 최종 release를 대신하지 않는다.
