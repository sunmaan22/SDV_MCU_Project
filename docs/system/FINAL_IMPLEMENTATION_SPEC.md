# Final Implementation Specification

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> **Status:** PARTIAL FREEZE — 2026-09-11 / Implementation Baseline v1.0 미도달
> **Decision Authority:** Project Owner  
> **Purpose:** 코드 작성 직전 모든 공통 Hardware / CAN / LIN / RTOS / State / DTC 값을 한 곳에서 확정하는 최종 명세서

이 문서는 프로젝트의 **최상위 구현 계약(Source of Truth)** 이다.

2026-09-11 Owner의 “H735 제외 STM 보드는 G431KB 구매 완료, 근거가 있으면 freeze” 지시에 따라
`DEC-HW-001~005`, `DEC-HW-021~023`만 이번에 동결했다. 구매 결정과 기존 H735 시험 기록을 근거로 하며,
새 실기 시험을 수행한 것은 아니다. 상세 근거와 다음 동결 조건은 [Freeze Review](FREEZE_REVIEW_2026-09-11.md)를 따른다.

> **2026-09-15 범위 변경:** E-Stop과 Gear 물리 입력을 F에서 C로 이전했다. C가 E-Stop을 로컬에서 즉시 차단(모터 Enable/STBY 직접 차단, CAN 비의존)하고, Gear/E-Stop 상태를 `Driver_Input`에 포함해 CAN으로 F에 보고한다. Pi DTC History DB(중앙 저장/이력) 기능은 삭제했다 — 이 프로젝트에 OBD2/외부 진단 커넥터가 없어 이력 조회의 실효성이 낮으므로, 각 Node가 발행하는 `DTC_Event`를 B(IVI)가 직접 구독해 실시간(Active만) 표시한다. History/Severity 지속 저장은 없다.

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
| A Ultrasonic | 4방향(FL/FR/RL/RR) 거리 / valid / warning / local fault owner — 초음파 충돌 위험도 판단 전담 |
| B H735 | UI 표시 + 사용자 Request 생성 + `DTC_Event` 실시간 구독·표시(Active only, History 없음) |
| C Drive | Motor / Servo 실제 actuator output owner + Driver 입력(RF/가변저항) owner + Gear/E-Stop 물리 입력 owner (E-Stop 로컬 즉시 차단) |
| D Gateway | CAN↔LIN mapping + LIN Master schedule owner |
| D Slave | Lamp actual state owner |
| E Vision | 전방 카메라 객체인식(COCO) 결과 + 전방 회피 ADAS 요청 owner (주차 관여 안 함) |
| F VCU | 최종 vehicle arbitration + Final Drive / Body Command owner |

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
| `Driver_Input` | C | F |
| `DTC_Event` | 각 Local Node | F, B |
| `ECU_Heartbeat` | 각 Node | F, Pi |

같은 최종 Message를 두 Node가 동시에 publish하지 않는다. `Driver_Input`은 accel/brake/steering뿐 아니라 gear와 estop_status도 포함한다 (2026-09-15부터 C가 Gear/E-Stop 물리 입력 owner, §4.8.1 참고).

## 1.2 고정 Safety Rule

```text
E-Stop / Critical Fault
> Ultrasonic Collision Critical
> ADAS Safety Request
> Normal Driver Request
```

- Vision(전방 객체 회피 요청)은 Ultrasonic Collision `CRITICAL`을 해제/override하지 않는다.
- `valid=false`인 센서값은 정상 판단에 사용하지 않는다.
- `SafetyTask`는 final command를 직접 쓰지 않는다.
- `VcuControlTask`만 `final_command`의 writer다.
- Drive ECU가 `Final_Drive_Command` timeout을 감지한다.
- VCU는 `Drive_Status`, Heartbeat, Peer Message timeout을 감지한다.
- Brake와 Accelerator가 동시에 유효하게 입력되면 Brake 우선을 기본 정책으로 한다.
- D↔R은 차량이 움직이는 상태에서 즉시 반전하지 않는다.
- E-Stop은 C가 로컬 GPIO/EXTI로 직접 읽고, CAN 경유 없이 즉시 Motor Driver Enable/STBY를 차단한다. C는 E-Stop 상태를 `Driver_Input.estop_status`로 CAN 발행해 F가 `Vehicle_State`/arbitration에 반영하지만, 모터 차단 자체는 CAN 통신 상태와 무관하게 동작해야 한다.

---

### 1.3 충돌주의 기능 범위 (2026-09-15 사용자 결정)

- Parking/주차 보조 기능을 **충돌주의(Collision Warning)** 로 대체한다. 주차 공간 탐색, 주차 경로 생성, 자동 주차 조향은 포함하지 않는다. 기어 P는 기존 기어 상태이며 기능명 변경과 무관하다.
- A는 기어 R 진입을 전제로 하지 않고 FL/FR/RL/RR의 거리·valid·warning을 생성한다. B는 모든 기어에서 4방향 충돌주의 패널에 접근할 수 있게 한다. Gear R 전용 화면 자동 전환은 요구하지 않는다.
- B는 유효하고 최신인 zone별 위험도와 센서 invalid/통신 stale을 구분한다. CRITICAL 경고는 상세 패널을 닫아도 기본 계기판에서 보이며, 패널 닫기가 경고 해제나 VCU 안전 개입 해제가 되어서는 안 된다.
- 사용자는 **기존 안전 개입 유지**를 선택했다. F의 우선순위는 E-Stop/Critical Fault > Ultrasonic Collision Critical > ADAS Safety Request > Driver Request다. A는 위험도를 산출하고, F만 최종 감속·정지 명령을 결정하며 C가 출력한다.
- 전후진별 제어 대상 zone, 정차 시 처리, 거리 threshold/hysteresis, 감속·정지·복구 조건은 `DEC-PER-003`, `DEC-CTRL-011/012`의 OPEN 결정이다. 4방향 표시를 모든 방향의 동일 제동 규칙으로 해석하지 않는다. E의 전방 카메라 ADAS 범위는 유지한다.
- `Ultrasonic_Status` 메시지명, publisher/consumer 및 zone 필드는 유지한다. 새 `Parking_Status`/`Collision_Status` CAN 메시지를 추가하지 않는다. payload/주기/timeout 수치는 계속 OWNER INPUT이다.

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

Status는 `OPEN / FROZEN / REMOVED`를 사용한다. `REMOVED`는 삭제된 결정의 이력이며 구현 대상이 아니다.

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
| `DEC-HW-009` | Ultrasonic Sensor 모델 | OWNER INPUT (개수 4개는 DEC-HW-025에서 FROZEN) | OPEN |
| `DEC-HW-010` | Motor 모델 | OWNER INPUT | OPEN |
| `DEC-HW-011` | Motor Driver | OWNER INPUT | OPEN |
| `DEC-HW-012` | Encoder/Hall | 미사용 — Speed/RPM 표시는 명령값(PWM 등) 기반 추정으로 대체 | REMOVED |
| `DEC-HW-013` | RC Servo | OWNER INPUT | OPEN |
| `DEC-HW-014` | Ambient Sensor | 미사용 — Ambient 기능 삭제 | REMOVED |
| `DEC-HW-015` | Front Camera | OWNER INPUT (전방 전용, COCO 기반 객체인식용) | OPEN |
| `DEC-HW-016` | Rear Camera | 미사용 — 충돌주의는 초음파 4방향 전용, Rear Vision 삭제 | REMOVED |
| `DEC-HW-017` | Accelerator Sensor | OWNER INPUT | OPEN |
| `DEC-HW-018` | Brake Sensor | OWNER INPUT | OPEN |
| `DEC-HW-019` | Steering Input Sensor | OWNER INPUT | OPEN |
| `DEC-HW-020` | E-Stop owner node / 동작 방식 | C 물리 GPIO/EXTI, 로컬 즉시 차단(CAN 비의존) + `Driver_Input.estop_status`로 상태 보고 | FROZEN |
| `DEC-HW-024` | Driver 원격 입력 장치 (RF 리모컨 or 가변저항) | OWNER INPUT | OPEN |
| `DEC-HW-025` | Ultrasonic 4방향 센서 배치 | 전좌(FL) / 전우(FR) / 후좌(RL) / 후우(RR) 4개 고정 | FROZEN |
| `DEC-HW-026` | Gear owner node / 동작 방식 | C 물리 GPIO, `Driver_Input.gear`로 CAN 발행 | FROZEN |
| `DEC-HW-027` | E-Stop 회로/부품 (스위치 모델, pull-up/down, debounce) | OWNER INPUT | OPEN |
| `DEC-HW-028` | Gear 입력 회로/부품 (버튼 개수 vs 로터리 스위치 vs ADC selector) | OWNER INPUT | OPEN |
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
| `DEC-CTRL-017` | Encoder invalid fallback | 미사용 — Encoder 삭제 | REMOVED |
| `DEC-CTRL-018` | PID 적용 여부 / tuning policy | OWNER INPUT | OPEN |
| `DEC-CTRL-019` | 입력값→speed/steering 선형 매핑 (RF/가변저항, accel 비례↑ brake 비례↓) | OWNER INPUT | OPEN |
| `DEC-CTRL-020` | Brake 입력 감속 감지 → `brake_lamp` 자동 점등 threshold | OWNER INPUT | OPEN |
| `DEC-CTRL-021` | Motor 명령값→speed/rpm 표시값 추정 함수 (실측 아님을 UI에 명시) | OWNER INPUT | OPEN |

## 3.4 Perception / Vision Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-PER-001` | Ultrasonic zone / 장착 위치 | 전좌(FL) / 전우(FR) / 후좌(RL) / 후우(RR) 4개 고정 | FROZEN |
| `DEC-PER-002` | Ultrasonic filter | OWNER INPUT | OPEN |
| `DEC-PER-003` | Ultrasonic threshold / hysteresis | OWNER INPUT | OPEN |
| `DEC-PER-004` | Ultrasonic scan period / gap | OWNER INPUT | OPEN |
| `DEC-VIS-001` | Front Vision Stage 기능 | OWNER INPUT | OPEN |
| `DEC-VIS-002` | Rear Vision Stage 기능 | 미사용 — Rear Vision 삭제 | REMOVED |
| `DEC-VIS-003` | Camera resolution / FPS | OWNER INPUT | OPEN |
| `DEC-VIS-004` | Vision algorithm/model | OWNER INPUT | OPEN |
| `DEC-VIS-005` | Vision freshness timeout | OWNER INPUT | OPEN |
| `DEC-VIS-006` | Gear D/R camera switching | 미사용 — 카메라는 전방 1대 상시 동작 | REMOVED |
| `DEC-VIS-007` | Gear R → first valid result latency | 미사용 — Rear Vision 삭제 | REMOVED |
| `DEC-VIS-008` | 객체 class(COCO)/방향(zone: 좌/중/우 등) 표현 방식 | OWNER INPUT | OPEN |

## 3.5 HMI / Body Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-HMI-001` | Cluster 필수 표시항목 | OWNER INPUT | OPEN |
| `DEC-HMI-002` | Warning 표시 우선순위 | OWNER INPUT | OPEN |
| `DEC-HMI-003` | 전방 객체 알림 및 4방향 충돌주의 overlay/panel 상세 정책 | OWNER INPUT | OPEN |
| `DEC-HMI-004` | DTC Clear 구현 여부 | OWNER INPUT | OPEN |
| `DEC-HMI-005` | TouchGFX update/frame budget | OWNER INPUT | OPEN |
| `DEC-BODY-001` | 구현 Lamp 범위 | 좌/우 턴시그널 + 헤드램프(밝기 가변) + 브레이크등 | FROZEN |
| `DEC-BODY-002` | Body_User_Request 기능 범위 | 좌/우 턴시그널 요청 + 헤드램프 밝기 요청 (brake_lamp는 사용자 요청이 아니라 F가 감속 감지로 자동 생성) | FROZEN |
| `DEC-BODY-003` | Ambient 단위/filter/calibration | 미사용 — Ambient 기능 삭제 | REMOVED |

## 3.6 DTC / Health Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-DTC-000` | DTC History DB / 저장 여부 | 미사용 — Pi DTC Manager/History DB 삭제. OBD2/외부 진단 커넥터가 없어 이력 조회 실효성이 낮으므로, `DTC_Event`는 B(IVI)가 실시간(Active만) 구독·표시하고 지속 저장하지 않는다 | REMOVED |
| `DEC-DTC-001` | DTC code numbering | OWNER INPUT | OPEN |
| `DEC-DTC-002` | DTC status enum (Active/Inactive 중심, History 상태 불필요) | OWNER INPUT | OPEN |
| `DEC-DTC-003` | DTC severity enum | OWNER INPUT | OPEN |
| `DEC-DTC-004` | fault 확정 / 해소 판정 규칙 (소스 ECU) | OWNER INPUT | OPEN |
| `DEC-DTC-005` | Critical DTC → Safe Action mapping | OWNER INPUT | OPEN |
| `DEC-HLT-001` | Heartbeat period / timeout | OWNER INPUT | OPEN |
| `DEC-HLT-002` | Watchdog refresh condition | OWNER INPUT | OPEN |
| `DEC-HLT-003` | RTOS task priority / period / stack / queue depth | OWNER INPUT | OPEN |

---

# 4. Final Logical Message Contracts

> 실제 CAN ID / bit position은 Owner Freeze 후 이 표에 직접 기록한다. 하위 문서는 이 표를 복제해 새 값을 만들지 않는다.

## 4.1 `Ultrasonic_Status`

> 4방향 고정: `FL`(전좌) / `FR`(전우) / `RL`(후좌) / `RR`(후우). 각 zone은 독립된 valid+distance를 갖는다 (단일 신호가 아니라 zone별 4세트, 또는 zone id로 구분되는 반복 필드).

| Field | Unit / Type | Range | Valid Rule | Final |
|---|---|---|---|---|
| zone id | enum | FL / FR / RL / RR | defined ID | FROZEN |
| distance | mm 후보 | TBD | `valid=true` | OWNER INPUT |
| valid | bool | 0/1 | source health | OWNER INPUT |
| warning_level | enum | SAFE/WARNING/CRITICAL | valid only | OWNER INPUT |
| fault_flags | bitfield | TBD | local fault | OWNER INPUT |
| freshness/sequence | TBD | TBD | monotonic | OWNER INPUT |

## 4.2 `Vision_Status`

> Rear Vision/주차 관련 필드 삭제 (2026-09-15). 전방 카메라 1대의 COCO 기반 객체인식 결과만 전달한다. Raw image는 CAN payload로 보내지 않는다 (§8 E HPC 참고).

| Field | Unit / Type | Final |
|---|---|---|
| front_valid | bool | OWNER INPUT |
| detected_class | enum (COCO name 기반) | OWNER INPUT |
| direction/zone | enum (예: LEFT/CENTER/RIGHT) | OWNER INPUT |
| vision warning | enum | OWNER INPUT |
| freshness/sequence | TBD | OWNER INPUT |

## 4.3 `ADAS_Request`

> 범위: 전방 객체 감지에 따른 회피/감속 요청만 담당한다. 초음파 위험도는 A가 `Ultrasonic_Status`로 F에 직접 제공한다. E는 A의 위험도를 재판정하거나 해제하지 않으며 §1.2 우선순위를 따른다.

| Field | Unit / Type | Final |
|---|---|---|
| request_valid | bool | OWNER INPUT |
| requested_speed | TBD | OWNER INPUT |
| requested_steering | TBD | OWNER INPUT |
| request_reason/type | enum (전방 객체 회피 사유만; 주차 사유 없음) | OWNER INPUT |
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

> `motor_rpm`/`vehicle_speed`는 실측 센서(Encoder/Hall)가 아니라 **모터 명령값(PWM 등) 기반 추정 함수의 결과**다. IVI/Cluster 표시 시 이 필드가 추정값임을 UI에서 구분한다 (`DEC-CTRL-021`).

| Field | Unit / Type | Final |
|---|---|---|
| motor_rpm (estimated) | rpm | OWNER INPUT |
| vehicle_speed (estimated) | TBD | OWNER INPUT |
| steering_target | TBD | OWNER INPUT |
| command_valid | bool | OWNER INPUT |
| fault_flags | bitfield | OWNER INPUT |

명령값 기반 추정 speed/RPM은 실제 정지·감속의 측정값이 아니다. D↔R 허용을 추정 speed=0만으로 확정하지 않는다. 실측 피드백이 없는 구성의 정지 확인/전환 대기/복구 기준은 `DEC-CTRL-004/005`에서 bench 근거와 함께 결정한다. brake_lamp 판정도 실제 감속으로 단정하지 않고 유효한 brake 입력 및 명령 감속 기반 정책을 `DEC-CTRL-020`에서 구체화한다.

## 4.6 `Body_User_Request`

> 범위: 좌/우 턴시그널 요청 + 헤드램프 밝기 요청만 (`DEC-BODY-002`). `brake_lamp`는 사용자 요청 대상이 아니다 — F가 감속을 감지해 자동으로 `Body_Command.brake_lamp`를 생성한다 (`DEC-CTRL-020`).

| Field | Unit / Type | Final |
|---|---|---|
| request_type | enum (TURN_LEFT / TURN_RIGHT / HEADLAMP_BRIGHTNESS) | OWNER INPUT |
| requested_value | TBD (헤드램프는 밝기 레벨) | OWNER INPUT |
| request_valid | bool | OWNER INPUT |

## 4.7 `Body_Command`

> `hazard`, `tail_lamp` 필드 삭제 (요구 범위 밖). `headlamp`는 on/off가 아니라 밝기값이다. `brake_lamp`는 F가 자동 생성한다.

| Field | Unit / Type | Final |
|---|---|---|
| headlamp_brightness | 0~100% 등 (bool 아님) | OWNER INPUT |
| brake_lamp | bool (F가 감속 감지로 자동 설정) | OWNER INPUT |
| turn_left | bool | OWNER INPUT |
| turn_right | bool | OWNER INPUT |

## 4.8 `Body_Status`

> `ambient` 필드 삭제 (Ambient 기능 삭제).

| Field | Unit / Type | Final |
|---|---|---|
| lamp_status | bitfield | OWNER INPUT |
| lin_health | enum/flags | OWNER INPUT |
| fault_flags | bitfield | OWNER INPUT |

## 4.8.1 `Driver_Input`

> Publisher는 C다 (2026-09-15부터 accel/brake/steering + gear + E-Stop 상태까지 포함, §7.1에서 지적된 누락 계약을 채움). C는 RF 리모컨 또는 가변저항으로 accel/brake/steering을, 물리 GPIO로 gear와 E-Stop을 읽어 하나의 `Driver_Input` 메시지로 CAN 발행한다. E-Stop의 실제 차단은 C가 CAN과 무관하게 로컬에서 수행하며, 이 필드는 F의 `Vehicle_State`/arbitration 반영용이다.

| Field | Unit / Type | Final |
|---|---|---|
| accel | TBD (`DEC-CTRL-019` 선형 매핑) | OWNER INPUT |
| brake | TBD (`DEC-CTRL-019` 선형 매핑) | OWNER INPUT |
| steering | TBD (`DEC-CTRL-019` 선형 매핑) | OWNER INPUT |
| gear | P/R/N/D enum | OWNER INPUT |
| estop_status | bool (C가 로컬로 이미 차단한 상태를 보고) | OWNER INPUT |
| request_valid | bool | OWNER INPUT |
| freshness/sequence | TBD | OWNER INPUT |

`Driver_Input`의 invalid/stale/timeout을 F가 E-Stop 해제나 유효한 Gear로 간주해서는 안 된다. 안전 상태는 `DEC-CTRL-004~006`과 freshness 계약에 따라 처리한다. C의 로컬 E-Stop 차단은 CAN 송수신과 독립적이며, 이후 ControlTask/수신 command가 차단을 덮어쓰지 않도록 interlock을 유지한다. 부팅 시 이미 눌린 E-Stop도 확인해야 한다. 해제만으로 자동 재구동하지 않으며, 구체적인 복구 조건은 `DEC-CTRL-006`에서 확정한다. Enable/STBY 비활성 극성과 실제 핀은 선택된 드라이버/회로 기준으로 확정한다.

## 4.9 `Vehicle_State`

| Field | Unit / Type | Final |
|---|---|---|
| gear | P/R/N/D enum | OWNER INPUT |
| ready | bool | OWNER INPUT |
| vehicle_mode | enum | OWNER INPUT |
| safety_state | enum | OWNER INPUT |

## 4.10 `DTC_Event`

> Pi DTC History DB는 삭제됐다 (`DEC-DTC-000` REMOVED). `DTC_Event`는 B(IVI)가 실시간으로 구독·표시하며 지속 저장하지 않는다 — 소스 Node의 fault가 해소되면 해당 이벤트도 화면에서 사라진다(Active만 표시).

| Field | Type | Final |
|---|---|---|
| source_node | enum/id | OWNER INPUT |
| code | integer/enum | OWNER INPUT |
| status | enum (Active/Inactive) | OWNER INPUT |
| severity | enum | OWNER INPUT |
| sequence/timestamp | TBD | OWNER INPUT |

### 실시간 fault 표시 계약

- 각 ECU가 자기 fault의 Active/Inactive를 판정하고 `DTC_Event` 또는 합의된 fault flag로 직접 발행한다. B는 `(source_node, code)`별 현재 상태를 RAM에만 유지한다. severity는 현재 표시/안전 판단에 사용하며, 이력 저장 삭제와 severity 필드 삭제를 혼동하지 않는다.
- 소스의 유효한 Inactive 통보 또는 해당 fault가 해소되었음을 나타내는 최신 상태를 받으면 B는 Active 목록에서 제거한다. 이벤트 미수신만으로 정상 복귀를 추정하지 않는다. IVI 수동 DTC Clear 요청은 현재 범위에 포함하지 않는다.
- IVI 재시작/재접속, Active 또는 Inactive 프레임 누락 후에도 현재 상태를 회복할 수 있도록 소스의 현재 fault 상태 재전송/주기 snapshot 계약이 필요하다. 방식·주기·timeout·최대 항목 수·sequence 처리 규칙은 `DEC-DTC-002/004`, `DEC-NET-004~007`, `DEC-HLT-001`에서 OWNER INPUT으로 확정한다. 과거 이벤트 재생은 하지 않는다.
- 소스 heartbeat/fault 상태가 stale이면 통신 두절/상태 미확인으로 구분하고, 이전 값을 현재 Active 또는 정상으로 확정 표시하지 않는다. 재접속 후에는 최신 유효 상태로 갱신한다.
- 화면을 보지 않는 동안 발생했다가 해소된 간헐적 fault는 나중에 확인할 수 없다. 이는 사용자가 수용한 범위 제한이다. fault 확정 threshold는 별도이며, CAN 프레임 한 번 누락을 반드시 DTC로 확정한다는 뜻은 아니다.

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
| `Lamp_Status` | Slave | Gateway | OWNER INPUT | OWNER INPUT | OWNER INPUT |
| `Lamp_Diagnostic` | Slave | Gateway | OWNER INPUT | OWNER INPUT | OWNER INPUT |

> `Ambient_Status` 프레임 삭제 (Ambient 기능 삭제, 2026-09-15).

## 5.2 CAN ↔ LIN Mapping

| CAN Signal | LIN Signal | Mapping / Scale | Final |
|---|---|---|---|
| Body_Command headlamp_brightness | LAMP_HEAD_CMD | TBD | OWNER INPUT |
| Body_Command turn left/right | LAMP_TURN_CMD | TBD | OWNER INPUT |
| Body_Command brake_lamp | LAMP_BRAKE_CMD | TBD | OWNER INPUT |
| Lamp_Status | Body_Status lamp_status | TBD | OWNER INPUT |

---

# 6. RTOS / Linux Final Execution Contract

각 MCU 역할은 최종 코드 작성 전에 아래 표의 `Final Period / Priority / Stack / Queue`를 Owner가 Freeze한다.

| Node | Critical Tasks |
|---|---|
| A | UltrasonicTask, PerceptionTask, CanTxTask, HealthTask |
| B | CanRxTask, VehicleModelTask, GuiTask, CommandTxTask, HealthTask |
| C | CanRxTask, ControlTask, DriverInputTask, CanTxTask, StatusTask, HealthTask |
| D Gateway | CanRxTask, GatewayMappingTask, LinScheduleTask, CanTxTask, HealthTask |
| D Slave | LinRxTask, LightingTask, StatusTask, HealthTask |
| F | SafetyTask, VcuControlTask, CanRxTask, CanTxTask, DiagnosticTask, HealthTask |

> C에 `DriverInputTask` 추가 (RF 수신기 또는 가변저항 입력 읽기 + `Driver_Input` CAN 발행 — 입력 하드웨어가 실제로 C에 물리적으로 붙기 때문에 publisher를 F에서 C로 이전, §1.1 참고). C의 `FeedbackTask`(Encoder 기반)는 Encoder 삭제로 제거. F의 `DriverInputTask`는 publisher 이전에 따라 제거.
>
> **2026-09-15:** E-Stop/Gear GPIO도 F에서 C로 이전했다. C는 E-Stop EXTI ISR에서 Motor Driver Enable/STBY를 즉시 로컬 차단(가장 높은 우선순위, CAN/RTOS Task 경유 없이 ISR에서 직접 처리 가능)하고, `DriverInputTask`가 gear/estop_status를 `Driver_Input`에 실어 CAN 발행한다. F는 더 이상 E-Stop/Gear GPIO를 직접 읽지 않으며, `SafetyTask`는 CAN으로 수신한 `Driver_Input.estop_status`를 보고 override를 갱신한다. F의 `DiagnosticTask`는 Pi DTC Manager 없이 `DTC_Event` 발행과 현재 fault/status 처리를 담당하며, 지속 이력 저장은 하지 않는다. B는 각 ECU의 이벤트를 직접 구독해 표시한다.

Linux E Node는 다음을 Freeze한다.

| Service | Owner Decision |
|---|---|
| front_vision | lifetime / mode / restart |
| vehicle_manager | state ownership |
| can_service | CAN single owner |
| health_monitor | timeout / restart condition |
| logger | storage / rotation / blocking policy |

Pi DTC Manager 서비스는 삭제됐다 (`DEC-DTC-000` REMOVED). Diagnostics history/storage는 어떤 Node도 소유하지 않는다.

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

주차별 일정은 [WEEKLY_PLAN](../getting_started/WEEKLY_PLAN.md)의 목표이며 달력 경과만으로 gate를 통과하지 않는다.
Bring-up, RTOS skeleton, mock, 계측용 bench 코드는 OPEN 값으로도 작성할 수 있다.
이때 값은 `BENCH ONLY / NOT FROZEN` 설정으로 분리하고 최종 통합 상수나 PASS 근거로 승격하지 않는다.
Gate D 이전의 계측용 통합 빌드는 허용하되 최종 통합 baseline으로 취급하지 않는다.

Gate B 추가 완결성 확인:
- §4.8.1 `Driver_Input`은 accel/brake/steering/gear/estop_status 필드 목록까지는 채웠으나 값/단위/payload/주기/timeout은 여전히 OWNER INPUT이다. 이 값들이 FROZEN되어야 Gate B가 닫힌다.
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
DTC_Event(CAN) → 실시간 구독 → Diagnostics 화면 (Active only, History 없음)
```

View에서 CAN Driver를 직접 호출하지 않는다.

## C Drive

```text
E-Stop GPIO/EXTI → 로컬 즉시 Motor Enable/STBY 차단 (CAN 비의존)
Gear GPIO + RF 수신기/가변저항 → DriverInputTask → Driver_Input(CAN, C 발행: accel/brake/steering/gear/estop_status)
Final_Drive_Command → Validation → Control → PWM/DIR/Servo
PWM/모터 명령값 → 추정 함수 → Motor_RPM/Speed(estimated) → Status
```

Motor driver rating과 command timeout/safe state가 FROZEN이어야 한다. Encoder/Hall 실측 Feedback은 사용하지 않는다 (`DEC-HW-012` REMOVED) — speed/rpm 표시는 명령값 기반 추정 함수(`DEC-CTRL-021`)로 대체한다. Driver 입력(RF 또는 가변저항, `DEC-HW-024`)은 accel/brake 값에 선형 비례해 target speed가 오르내린다(`DEC-CTRL-019`). E-Stop 로컬 차단(`DEC-HW-020`)은 Driver_Input CAN 발행과 독립적으로 가장 먼저 처리한다.

## D Body

```text
Body_Command(headlamp_brightness/turn/brake) → CAN↔LIN Mapping → LIN → Lamp
Lamp Status → LIN → CAN Body_Status
```

LIN schedule은 Gateway만 소유한다. Ambient 관련 기능은 삭제되어 D Slave는 Lamp actual state owner만 담당한다. `brake_lamp`는 F가 감속을 감지해 자동 생성하며, D는 그 값을 그대로 LIN으로 중계할 뿐 판단하지 않는다.

## E HPC

```text
Front Camera(1대) → YOLO/COCO 객체인식 → detected_class + direction/zone
→ Vision_Status(팝업용, B로) / ADAS_Request(회피요청, F로) → can_service
```

Raw image는 CAN으로 보내지 않는다. Rear Camera/Rear Vision/주차 Vision 기능은 삭제되었다 (`DEC-HW-016`, `DEC-VIS-002/006/007` REMOVED) — 초음파 충돌 위험도 판단은 A(Ultrasonic)가 전담한다.

## F VCU

```text
Driver_Input(CAN, gear/estop 포함) + Perception + Status + Fault
→ Validation/Freshness
→ Safety/Arbitration
→ Final_Drive_Command / Body_Command
```

Final command writer는 `VcuControlTask` 하나다. F는 Gear/E-Stop 물리 GPIO를 더 이상 직접 읽지 않는다 (2026-09-15부터 C 소유) — `SafetyTask`는 CAN으로 수신한 `Driver_Input.estop_status`를 override 조건으로 사용한다.

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
