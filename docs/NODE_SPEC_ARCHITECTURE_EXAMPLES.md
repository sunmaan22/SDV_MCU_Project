# Node별 Specification / Architecture 예시

> 이 문서는 Architecture v1.2의 6역할 기준 예시다.  
> 실제 핀, 센서 모델, CAN ID, 주기, 임계값은 시험 후 확정한다.

---

# 0. 역할

| 담당 | Node |
|---|---|
| A | Ultrasonic Perception STM32 #1 |
| B | STM32H735 Cluster + IVI Cockpit |
| C | Drive + Steering STM32 #2 |
| D | STM32 #3 Body Gateway + STM32 #4 LIN Slave |
| E | Raspberry Pi HPC + Front/Rear Vision |
| F | STM32 #5 VCU + DTC + CAN Integration |

---

# 1. A — Ultrasonic Perception

## Specification 예

| ID | Requirement |
|---|---|
| REQ-US-001 | Echo pulse에서 거리값을 계산해야 한다. |
| REQ-US-002 | 유효하지 않은 측정은 `valid=false`로 표시해야 한다. |
| REQ-US-003 | SAFE/WARNING/CRITICAL 상태를 생성해야 한다. |
| REQ-US-004 | Sensor timeout을 검출해야 한다. |

## Architecture

```text
Ultrasonic
→ Trigger/Echo Driver
→ Distance Calculation
→ Range Check
→ Filter
→ Warning Level
→ CAN Service
```

## Stage 1

- 센서 1개
- 3개 기준거리
- timeout
- UART log

---

# 2. B — H735 Cluster + IVI

## Specification 예

| ID | Requirement |
|---|---|
| REQ-HMI-001 | Speed/RPM/Gear를 기본 Cluster에 표시해야 한다. |
| REQ-HMI-002 | ADAS/Parking Warning을 표시해야 한다. |
| REQ-HMI-003 | 상세 DTC 화면을 제공해야 한다. |
| REQ-HMI-004 | Touch로 메뉴를 전환해야 한다. |

## Architecture

```text
CAN RX
→ Vehicle Data Model
→ TouchGFX Model/Presenter
→ Cluster Main
  ├ ADAS
  ├ Parking
  ├ Diagnostics
  └ Settings
```

## Stage 1

```text
Dummy Data
→ 화면 표시/전환/Warning 확인
```

---

# 3. C — Drive + Steering

## Specification 예

| ID | Requirement |
|---|---|
| REQ-DRV-001 | Motor PWM과 Direction을 출력해야 한다. |
| REQ-DRV-002 | Encoder/Hall로 RPM을 계산해야 한다. |
| REQ-DRV-003 | Command timeout 시 Motor 출력을 안전 상태로 전환해야 한다. |
| REQ-STR-001 | Steering command를 RC Servo pulse로 변환해야 한다. |
| REQ-STR-002 | Servo command를 안전 범위로 제한해야 한다. |

## Architecture

```text
VCU Final Speed ─→ Speed Control ─→ PWM/DIR ─→ TB6612FNG 후보 ─→ Motor
                         ↑
                    Encoder RPM

VCU Final Steering → Steering Mapping → Servo PWM → RC Servo
```

## Stage 1

- PWM
- Direction
- Encoder count / RPM
- Servo center / left / right
- stop

---

# 4. D — Body Gateway + LIN Slave

## Specification 예

### Gateway

| ID | Requirement |
|---|---|
| REQ-GW-001 | CAN Body_Command를 LIN Lamp_Command로 변환해야 한다. |
| REQ-GW-002 | LIN Ambient_Status를 CAN Body_Status로 변환해야 한다. |
| REQ-GW-003 | LIN node timeout을 검출해야 한다. |

### LIN Slave

| ID | Requirement |
|---|---|
| REQ-BODY-001 | Ambient 값을 읽어 LIN으로 제공해야 한다. |
| REQ-BODY-002 | Lamp_Command에 따라 Lighting을 제어해야 한다. |
| REQ-BODY-003 | Lamp 상태를 LIN으로 제공해야 한다. |

## Architecture

```text
CAN FD
 ↕
STM32 #3 Gateway
CAN Service
Gateway Mapping
LIN Master/Schedule
 ↕ LIN
STM32 #4 Slave
├ Ambient Driver
├ Lighting Logic
└ LIN Response
```

## Mapping 예

| CAN | 방향 | LIN |
|---|---|---|
| `HEADLAMP_REQ` | CAN→LIN | `Lamp_Command.Head` |
| `TURN_LEFT_REQ` | CAN→LIN | `Lamp_Command.Left` |
| `AMBIENT_LIGHT` | LIN→CAN | `Body_Status.Ambient` |
| `LAMP_FAULT` | LIN→CAN | `Body_DTC` |

---

# 5. E — HPC + Camera Vision

## Specification 예

| ID | Requirement |
|---|---|
| REQ-VIS-001 | Front Camera frame을 안정적으로 획득해야 한다. |
| REQ-VIS-002 | Rear Camera frame을 안정적으로 획득해야 한다. |
| REQ-VIS-003 | Raw video를 CAN으로 전송하지 않아야 한다. |
| REQ-VIS-004 | Front Vision에서 Lane/Object 결과를 생성해야 한다. |
| REQ-VIS-005 | Rear Vision에서 Object/Warning 결과를 생성해야 한다. |
| REQ-VIS-006 | 최종 Pi 한 대에서 두 서비스를 실행할 수 있는 구조여야 한다. |

## Architecture

```text
Front CSI Camera → Front Camera Service → ADAS Vision ─┐
                                                       ├→ Vision Result / Request → CAN
Rear USB Camera  → Rear Camera Service  → Parking Vision┘

                     Raspberry Pi HPC
```

개발 시:

```text
Pi #1 Front / Pi #2 Rear
```

최종 시 동일한 Result Interface를 유지한 채 하나의 Pi로 합친다.

## Output 예

```text
ADAS_Result
- lane_offset
- collision_level
- speed_request
- steering_request

Parking_Vision
- object_detected
- object_position
- warning_level
```

---

# 6. F — VCU + DTC + CAN Integration

## Specification 예

| ID | Requirement |
|---|---|
| REQ-VCU-001 | Gear를 P/R/N/D 상태로 관리해야 한다. |
| REQ-VCU-002 | Accelerator/Brake를 0~100%로 정규화해야 한다. |
| REQ-VCU-003 | Steering Wheel 입력을 유효한 조향 요청으로 변환해야 한다. |
| REQ-VCU-004 | E-Stop/Critical Fault를 정상 Driver Request보다 우선해야 한다. |
| REQ-VCU-005 | Vision과 Ultrasonic 요청을 검증한 뒤 Final Command를 생성해야 한다. |
| REQ-CAN-001 | CAN Matrix와 Heartbeat/Timeout 규칙을 관리해야 한다. |
| REQ-DTC-001 | 프로젝트 DTC Code/Status 규칙을 정의해야 한다. |

## Architecture

```text
Gear/Accel/Brake/Steering ─┐
ADAS Request ───────────────┤
Ultrasonic Status ──────────┤
Heartbeat/Fault ────────────┤
                           ↓
                         VCU
                Input Validation
                Vehicle State
                Safety / Arbitration
                           ↓
                Final Speed/Steering
                           ↓ CAN
                  Drive + Steering
```

## DTC Architecture

```text
A/C/D/E/F Local Fault
      ↓ DTC Event
CAN FD
      ↓
Pi DTC Manager
Active / History / Time / Count
      ↓
B H735 Diagnostics UI
```

F는 DTC 규격의 Owner지만 각 Node 담당자가 자기 Fault Detection을 구현한다.

---

# 7. 각자 제출할 문서

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

각 문서에 반드시 포함:

- Input
- Process
- Output
- Interface
- Fault
- Stage 1 PASS 기준
- Stage 2 CAN/LIN 후보 Signal
