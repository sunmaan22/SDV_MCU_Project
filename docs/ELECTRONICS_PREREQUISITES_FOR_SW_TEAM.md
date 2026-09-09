# Software 전공자를 위한 전자공학 선행학습 가이드

> 대상: STM32 / Raspberry Pi / CAN FD / LIN 기반 모빌리티 프로젝트를 처음 진행하는 Software 전공 팀원  
> 목표: 회로 설계 전문가가 되는 것이 아니라, **센서·MCU·모터·통신을 안전하게 연결하고 Hardware → Signal → Firmware → Network 문제를 스스로 추적할 수 있는 수준**까지 도달하는 것

---

# 0. 현재 프로젝트에서 필요한 기술 계층

```text
전압 / 전류 / GND
      ↓
GPIO / ADC / PWM / Timer
      ↓
I2C / SPI / UART
      ↓
Sensor / Motor / Servo / Encoder
      ↓
CAN Controller / Transceiver / CAN FD
      ↓
LIN Master / Slave / Transceiver
      ↓
Gateway / State Machine / DTC / Fail-safe
      ↓
ADAS / Cockpit / Vehicle Integration
```

코드보다 먼저 전압, GND, 핀, 인터페이스를 확인한다. 잘못된 전압은 디버깅 로그 대신 연기를 출력한다.

---

# 1. 공통 필수 전자공학

## 1.1 전압·전류·저항

최소한 다음 관계를 이해한다.

```text
V = I × R
```

반드시 구분할 것:

- 3.3 V Logic / 5 V Logic
- Sensor VCC와 Signal Voltage
- MCU GPIO 최대 허용전압
- GPIO가 공급할 수 있는 전류는 작다는 점
- Logic Power와 Motor Power

## 1.2 Common Ground

센서, STM32, Motor Driver, CAN/LIN Transceiver가 신호를 주고받을 때 기준 GND가 맞아야 한다.

```text
MCU GND ─ Sensor GND
MCU GND ─ Driver Logic GND
MCU GND ─ CAN/LIN Transceiver GND
```

## 1.3 Multimeter

모든 팀원이 최소한 다음을 할 수 있어야 한다.

- DC Voltage 측정
- Continuity 확인
- GND 확인
- 전원 polarity 확인
- 예상전압과 실제전압 비교

센서가 안 되면 코드보다 먼저 `VCC-GND = ? V`를 측정한다.

---

# 2. STM32 Peripheral 기초

| Peripheral | 프로젝트 예 |
|---|---|
| GPIO | Gear button, E-stop, LED, Enable |
| ADC | Accelerator, Brake, Ambient, Battery Voltage/Temp |
| Timer | Encoder count, periodic task |
| PWM | TB6612FNG command, RC Servo, Lighting |
| UART | Debug, LIN peripheral 기반 구현 |
| I2C | AS5600, ToF |
| SPI | 외장 CAN FD controller 후보 |
| CAN/FDCAN | ECU 간 Backbone |
| Interrupt | Encoder, CAN RX, event |

## GPIO

알아야 할 것:

- Input / Output
- Pull-up / Pull-down
- Floating
- Active High/Low
- Debounce
- Push-pull / Open-drain

## ADC

12-bit ADC 예:

```text
0 ~ 4095
```

```text
Voltage = ADC_RAW / ADC_MAX × VREF
```

ADC Raw가 물리값은 아니다.

```text
ADC Raw → Voltage → Calibration / Transfer Function → Physical Value
```

## PWM

```text
STM32 PWM
   ↓
Driver / Servo / LED
```

MCU GPIO에 Motor를 직접 연결하지 않는다.

---

# 3. Sensor Interface

## I2C

- SDA / SCL
- Address
- Pull-up
- Clock speed
- Register / library

프로젝트 예:

- AS5600 Steering Wheel Angle
- ToF Parking Sensor

## SPI

- MOSI / MISO / SCK / CS
- 외장 CAN FD controller 후보

## UART

Stage 1 Debug의 기본 도구다.

```text
GEAR=D ACCEL=35 BRAKE=0 STEER=-12.3
RPM=145 PARK_RR=320 VALID=1
```

---

# 4. Datasheet 읽는 최소 순서

센서:

1. Supply Voltage
2. Logic Voltage
3. Pinout
4. Interface
5. Address / Register
6. Range / Accuracy
7. Timing
8. Absolute Maximum Rating

Motor Driver:

1. Motor Supply
2. Logic Supply
3. Continuous / Peak Current
4. PWM / Direction Input
5. Protection
6. Thermal Limit

MCU:

1. GPIO Voltage
2. Alternate Function
3. ADC pin
4. Timer/PWM
5. I2C/SPI/UART
6. CAN/FDCAN 지원

---

# 5. Motor / Servo / Encoder

## Brushed DC Motor + TB6612FNG 후보

```text
STM32
 ↓ PWM + Direction
TB6612FNG
 ↓ Power
Brushed DC Motor
```

확인할 것:

- Motor voltage
- Stall current
- Driver continuous/peak current
- Motor supply와 logic supply
- Enable / standby pin

## Encoder / Hall

```text
Motor Rotation
 ↓
Pulse
 ↓
STM32 Timer
 ↓
RPM
```

알아야 할 것:

- PPR / CPR
- Count
- Frequency
- Direction
- Sampling period

## RC Servo

서보는 내부 위치제어가 있으므로 초기에는 별도 Steering Feedback 없이 다음처럼 사용할 수 있다.

```text
Target Steering
 ↓
STM32 PWM
 ↓
RC Servo
```

Pulse width 범위는 실제 Servo datasheet/시험으로 Calibration한다.

---

# 6. Closed-loop / PID 기초

```text
Error = Target - Actual
```

```text
Target RPM
   ↓
PID
   ↓
PWM
   ↓
Motor
   ↓
Encoder RPM
   └── Feedback
```

Stage 1에서는 PID보다 **센서와 액추에이터가 신뢰 가능한지** 먼저 확인한다.

---

# 7. CAN / CAN FD

구조:

```text
STM32 FDCAN Controller
        ↓
CAN Transceiver
        ↓
CANH / CANL
        ↓
Shared Bus
```

반드시 알아야 할 개념:

- Controller vs Transceiver
- Differential signal
- CANH / CANL
- 120 Ω termination
- CAN ID
- Arbitration
- DLC
- Classic CAN vs CAN FD
- Periodic / Event message
- Timeout
- Bus-off

Raspberry Pi에는 CAN FD가 내장되어 있지 않으므로 `MCP2518FD`, `TCAN4550`, USB-CAN FD 등 실제 선택한 인터페이스를 별도로 이해해야 한다. `MCP2515`는 Classic CAN용이다.

---

# 8. LIN

이번 프로젝트에서 LIN은 **Body Local Subnetwork**로 구현한다.

```text
CAN FD Backbone
      ↓
Body Gateway STM32 #4
      ↓ LIN Master
LIN Transceiver
      ↓
LIN Bus
      ↓
Body LIN Slave STM32 #5
```

## LIN에서 알아야 할 것

- LIN은 Master가 Schedule을 주도한다.
- Slave는 Master Header에 맞춰 Response를 제공한다.
- UART signal을 Bus에 직접 연결하지 않고 LIN Transceiver가 필요하다.
- CAN보다 저속이고 단순 Body 장치에 적합하다.

### 기본 개념

- Master / Slave
- Header / Response
- Frame ID / PID 개념
- Schedule Table
- Checksum
- Timeout
- LIN Transceiver

예시 Schedule:

| Slot | Frame | 역할 |
|---:|---|---|
| 1 | Ambient_Status | Slave 값을 Master가 읽음 |
| 2 | Lamp_Command | Master가 Slave에 명령 |
| 3 | Lamp_Status | Slave 상태 읽음 |
| 4 | Lamp_Diagnostic | Fault 상태 읽음 |

---

# 9. Gateway 개념

Gateway는 CAN선과 LIN선을 둘 다 꽂은 장치가 아니다.

```text
Protocol A Message
      ↓
Decode / Validate
      ↓
Signal Mapping
      ↓
Protocol B Message
```

본 프로젝트:

```text
CAN Body_Command
→ Body Gateway
→ LIN Lamp_Command
```

반대 방향:

```text
LIN Ambient_Status
→ Body Gateway
→ CAN Body_Status
```

필수 문서:

- CAN↔LIN Mapping Table
- LIN Schedule
- Timeout 정책
- Gateway DTC

---

# 10. State Machine / Heartbeat / DTC

## State Machine

예:

```text
INIT → READY → DRIVE / REVERSE → FAULT
```

Gear R:

```text
Rear Camera Active
Front ADAS Pause
Parking Mode
```

## Heartbeat

```text
ECU Heartbeat Lost
→ VCU Timeout
→ Fault 판단
→ 필요한 경우 Motor Stop
```

## DTC

```text
Fault Detection
 ↓
DTC Code / Status
 ↓ CAN FD
Pi DTC Manager
 ↓
Active / History / Count / Timestamp
 ↓
H735 Diagnostic UI
```

---

# 11. Power 기초

```text
Battery
 ├→ Motor Power → TB6612FNG → Motor
 └→ Regulator / UBEC → Logic 5V/3.3V → Pi / STM32
```

확인할 것:

- Battery maximum voltage
- Motor stall current
- Regulator current capacity
- Logic/Motor rail
- Common GND
- Voltage drop
- Reverse polarity
- 필요 시 fuse 개념

---

# 12. 계측

## Multimeter

모든 역할 필수.

## Oscilloscope / Logic Analyzer

가능하면 다음을 확인한다.

- PWM
- Encoder pulse
- UART
- I2C
- CANH/CANL
- LIN waveform

특히 B와 F는 기본 파형을 볼 수 있으면 디버깅 속도가 크게 올라간다.

---

# 13. 역할별 선행학습

| 담당 | Node | 필수 기술 |
|---|---|---|
| **A** | VCU | GPIO, ADC, I2C, State Machine, CAN FD, Heartbeat, Safety |
| **B** | Drive + Steering | PWM, Motor Driver, Encoder, Timer, Servo, Power, PID |
| **C** | Pi HPC / ADAS | Pi power, CSI Camera, OpenCV, latency, SocketCAN, Linux service, DTC DB |
| **D** | Parking | I2C/GPIO, ToF/Ultrasonic, multi-sensor timing, Rear USB Camera |
| **E** | H735 Cockpit | STM32H7, TouchGFX, FDCAN, HMI data model, DTC UI |
| **F** | Body Gateway + LIN Slave | CAN/FDCAN, UART/LIN, LIN Transceiver, Master/Slave, Gateway Mapping, GPIO/PWM/ADC |

---

# 14. ADAS 담당 예시

ADAS는 YOLO 실행으로 끝나지 않는다.

```text
Camera
 ↓
Frame Capture
 ↓
Preprocess
 ↓
CV / AI
 ↓
Risk / ADAS Result
 ↓
CAN Request
 ↓
VCU
```

Stage 1:

1. Pi 부팅
2. Camera 인식
3. Frame Capture
4. Resolution / FPS
5. OpenCV frame 접근
6. ROI / grayscale 등 최소 처리
7. Camera error 처리
8. CPU temperature/FPS 기록

그 다음:

```text
Lane / Object
→ ADAS Result
→ CAN FD
```

전체 Latency도 측정한다.

```text
T_total = Capture + Preprocess + Inference + Decision + CAN
```

---

# 15. 최소 학습 순서

```text
Day 1  Voltage / Current / GND / Multimeter
Day 2  GPIO / ADC / PWM / UART
Day 3  I2C / SPI + 담당 Datasheet
Day 4  담당 Sensor/Actuator Bring-up
Day 5  Timer / Encoder / Filtering
Day 6  CAN / Transceiver / Termination
Day 7  LIN / Gateway / Architecture / State / DTC
```

각 담당은 이후 자기 역할 심화로 들어간다.

---

# 16. CAN 통합 전에 답할 수 있어야 하는 질문

- [ ] 내 MCU Logic Voltage는?
- [ ] 센서 VCC/Signal Voltage는?
- [ ] GND는 어떻게 연결하는가?
- [ ] ADC Raw와 Physical Value의 차이는?
- [ ] Motor를 GPIO에 직접 연결하면 왜 안 되는가?
- [ ] CAN Controller와 Transceiver 차이는?
- [ ] CAN bus termination은 왜 필요한가?
- [ ] LIN Master와 Slave는 무엇이 다른가?
- [ ] Gateway는 어떤 Signal을 Mapping하는가?
- [ ] 내 Node가 소유하는 데이터는 무엇인가?
- [ ] 센서가 끊기면 어떤 상태가 되는가?

---

## 관련 문서

- [Beginner Architecture & Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
- [Sensor List](SENSOR_LIST.md)
- [Node Spec / Architecture Examples](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [ECU Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)
- [Main README](../README.md)
