# Project Reference

> 현재 프로젝트에서 **어떤 보드가 무엇을 맡고, 어떤 센서/데이터를 소유하는지** 한 곳에서 확인하는 문서다.  
> 핀 번호, CAN ID, 주기, 임계값은 실제 보드/시험 후 확정하며 미정값은 `TBD`로 둔다.

# 1. 현재 전체 구조

```text
Front Camera ─┐
              ├→ Raspberry Pi Vision/HPC ─────────────┐
Rear Camera ──┘                                      │
                                                     │ CAN FD
Ultrasonic → STM32 #1 ───────────────────────────────┤
                                                     ├→ STM32 #5 VCU
STM32H735 Cockpit ←───────────────────────────────────┤       │
                                                     │       ↓
STM32 #3 Body Gateway ←───────────────────────────────┘  STM32 #2 Drive/Steer
        ↕ LIN
STM32 #4 Body LIN Slave
        ├ Ambient Sensor
        └ Lighting
```

## Board Mapping

| Node | Hardware | 역할 |
|---|---|---|
| A | STM32 #1 + Ultrasonic | Parking distance perception |
| B | STM32H735 + TouchGFX | Cluster + IVI |
| C | STM32 #2 + Motor Driver + Motor + Servo | Drive + Steering control |
| D-Gateway | STM32 #3 + CAN/LIN Transceiver | CAN FD ↔ LIN Gateway, LIN Master |
| D-Slave | STM32 #4 + LIN Transceiver | Ambient + Lighting LIN Slave |
| E | Raspberry Pi | Front/Rear Camera Vision, HPC services |
| F | STM32 #5 | VCU, Driver Input, Safety, CAN Integration |
| Diagnostics | Raspberry Pi service | DTC History / Logger |

> 소형 STM32의 실제 FDCAN 지원 여부는 사용 보드가 확정되면 확인한다. 지원하지 않을 경우 Classic CAN 또는 외장 CAN FD Controller 사용 여부를 별도 결정한다.

---

# 2. Sensor / Input List

| 영역 | 입력 / 센서 | Owner | Interface 후보 | 상태 |
|---|---|---|---|---|
| Driver | P/R/N/D | VCU | GPIO | 계획 |
| Driver | Accelerator Position | VCU | ADC, Pot/Hall | 후보 |
| Driver | Brake Position | VCU | ADC, Pot/Hall | 후보 |
| Driver | Steering Wheel Angle | VCU | I2C/Analog, AS5600 후보 | 후보 |
| Drive | Motor Encoder / Hall | Drive ECU | Timer/GPIO | 권장 |
| Steering | Actual Steering Feedback | Drive ECU | ADC/I2C | 선택 확장 |
| Parking | Ultrasonic Sensors | Ultrasonic ECU | GPIO/Timer | 계획 |
| Vision | Front Camera | Raspberry Pi | CSI | 계획 |
| Vision | Rear Camera | Raspberry Pi | USB | 계획 |
| Body | Ambient Light Sensor | LIN Slave | ADC/I2C | 계획 |
| Thermal | Motor Temperature | Drive ECU | ADC/I2C | 선택 확장 |
| Battery | Battery Voltage | VCU 또는 지정 Node | ADC measurement circuit | 후보 |
| Battery | Battery Temperature | VCU 또는 지정 Node | ADC/I2C | 후보 |
| Battery | Current Sensor | 지정 Node | ADC/I2C | 선택 확장 |

---

# 3. Data Owner 원칙

같은 데이터를 여러 Node가 따로 만들어서는 안 된다.

| 데이터 | Owner | 주요 Consumer |
|---|---|---|
| Gear / Accelerator / Brake / Steering Input | VCU | Drive, HPC, H735 |
| Final Speed / Steering Request | VCU | Drive + Steering ECU |
| Motor RPM / Vehicle Speed | Drive ECU | VCU, H735, HPC |
| Ultrasonic Distance / Warning | Ultrasonic ECU | VCU, H735, HPC |
| Front/Rear Vision Result | Raspberry Pi HPC | VCU, H735 |
| Ambient Light / Local Lamp Status | Body LIN Slave | Gateway → VCU/H735/HPC |
| CAN↔LIN Gateway Status | Body Gateway | VCU/H735/HPC |
| DTC History DB | Raspberry Pi DTC Manager | H735 / Debug tools |
| 화면 값 | 원본 Node | H735는 Subscriber |

---

# 4. 역할별 Input → Process → Output → Fault

## A. Ultrasonic

```text
Input   : Trigger/Echo 또는 Sensor response
Process : 거리 계산 → 유효성 → 필터 → Warning Level
Output  : distance_mm, valid, warning_level
Target  : VCU / H735 / HPC
Fault   : timeout, invalid range, sensor unavailable
```

명세 Requirement 예:
- `REQ-US-001`: 거리값을 정의된 단위로 제공해야 한다.
- `REQ-US-002`: 유효하지 않은 측정은 `valid=false`로 구분해야 한다.
- `REQ-US-003`: Warning Level을 생성해야 한다.

Architecture 핵심:
```text
Ultrasonic Driver
→ Distance Calculation
→ Validation/Filtering
→ Parking State
→ CAN Service
```

---

## B. H735 Cluster + IVI

```text
Input   : CAN Vehicle / ADAS / Parking / DTC data, Touch
Process : Data Model → UI State → Screen Rendering
Output  : Cluster/IVI 화면, User Request
Target  : Driver / 필요한 Request는 CAN
Fault   : CAN timeout, invalid display data, UI state error
```

화면:
- Cluster Main: Speed, RPM, Gear, Battery, Warning
- ADAS
- Parking
- Diagnostics / DTC
- Settings

H735는 Motor PWM이나 Sensor physical value를 직접 만들지 않는다.

---

## C. Motor + Steering

```text
Input   : Final Speed/Steering Request, Encoder/Hall
Process : Command validation → Motor/Servo mapping → feedback control
Output  : Motor PWM/DIR, Servo PWM, RPM/Drive Status
Target  : Actuator + VCU/H735/HPC
Fault   : command timeout, encoder invalid, output fault candidate
```

하드웨어 후보:
```text
STM32 → TB6612FNG 후보 → Brushed DC Motor
STM32 → PWM → RC Servo
Motor → Encoder/Hall → STM32
```

초기에는 Motor/Servo를 낮은 출력의 고정된 시험환경에서 단독 검증하고, 전체 차량 통합은 이후 단계에서 진행한다.

---

## D. Lighting + Ambient / LIN-CAN

```text
CAN FD
↕
Gateway STM32
├ CAN Service
├ CAN↔LIN Mapping
├ LIN Schedule
└ Gateway Fault Monitor
↕ LIN
Body LIN Slave
├ Ambient Sensor
├ Lighting State
└ Lamp Output
```

CAN → LIN 예:
```text
BODY_COMMAND.HeadLamp = ON
→ Gateway Mapping
→ LIN Lamp_Command
→ LIN Slave
→ Head Lamp ON
```

LIN → CAN 예:
```text
Ambient Sensor
→ LIN Ambient_Status
→ Gateway
→ CAN BODY_STATUS.Ambient
```

Gateway MCU는 별도의 세 번째 MCU가 필요한 것이 아니다. **STM32 #3이 CAN FD Node이면서 LIN Master/Gateway 역할을 함께 맡고, STM32 #4가 LIN Slave가 된다.**

---

## E. HPC + Camera Vision

개발:
```text
Pi #1 + Front Camera → Front Vision
Pi #2 + Rear Camera  → Rear Vision
```

최종 목표:
```text
Front CSI Camera ─┐
                  ├→ Raspberry Pi HPC
Rear USB Camera ──┘
```

Front 결과 후보:
- lane_offset
- lane_angle
- object_detected
- collision_level
- speed_request
- steering_request

Rear 결과 후보:
- rear_object_detected
- object_position
- vision_warning

Raw frame은 Pi 내부에서만 처리하고 CAN에는 의미 있는 결과만 전달한다.

---

## F. VCU + DTC + CAN Integration

```text
Driver Input
ADAS Request
Ultrasonic Warning
ECU Heartbeat/Fault
        ↓
Input Validation
        ↓
Vehicle State / Gear / Mode
        ↓
Safety & Arbitration
        ↓
Final Speed / Steering Request
        ↓
CAN FD
```

우선순위 기본 방향:
```text
Critical Fault / E-Stop
> Critical Obstacle Stop
> ADAS Safety Request
> Normal Driver / Mode Request
```

실제 정책과 임계값은 시험 후 확정한다.

DTC:
```text
Local Node detects fault
→ DTC Event over CAN
→ Pi DTC Manager
→ Active / History / First Seen / Last Seen / Count
→ H735 Diagnostics UI
```

---

# 5. CAN / LIN Interface 방향

CAN ID와 bit layout은 아직 확정 전이면 `TBD`로 둔다. 먼저 **Signal의 의미, Owner, Consumer, Unit, Timeout**을 확정한다.

예시 Signal Contract:

| Signal | Owner | Consumer | Unit | Cycle | Timeout |
|---|---|---|---|---|---|
| `Vehicle_Gear` | VCU | All | enum | TBD | TBD |
| `Final_Speed_Request` | VCU | Drive | % or m/s TBD | TBD | TBD |
| `Final_Steering_Request` | VCU | Drive | deg/% TBD | TBD | TBD |
| `Motor_RPM` | Drive | VCU/H735/HPC | rpm | TBD | TBD |
| `Rear_Distance` | Ultrasonic | VCU/H735/HPC | mm | TBD | TBD |
| `ADAS_Warning` | HPC | VCU/H735 | enum | TBD | TBD |
| `Body_Ambient` | Gateway | VCU/H735/HPC | raw/% TBD | TBD | TBD |

LIN 후보:

| Frame | Publisher | 역할 |
|---|---|---|
| `Ambient_Status` | LIN Slave | 조도 상태 |
| `Lamp_Command` | Gateway Master | 조명 명령 |
| `Lamp_Status` | LIN Slave | 실제 조명 상태 |
| `Lamp_Diagnostic` | LIN Slave | Body fault 상태 |

---

# 6. Stage 1 최소 산출물

각 담당자는 자기 기능에서:

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

를 만든다.

작성 형식은 `docs/templates/`를 사용한다.

Stage 1에서 가장 중요한 것은 기능 개수를 늘리는 것이 아니라:

```text
Input이 믿을 만한가?
→ Process가 설명 가능한가?
→ Output이 재현되는가?
→ Fault를 구분할 수 있는가?
```

를 증명하는 것이다.
