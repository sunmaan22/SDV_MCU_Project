# Software 전공자를 위한 전자공학 / 임베디드 선행학습

> 목표: 회로 설계 전문가가 되는 것이 아니라 **센서·MCU·모터·카메라·CAN/LIN을 연결하고 문제를 추적할 수 있는 수준**까지 도달한다.

---

# 1. 전원이 가장 먼저다

모든 팀원이 알아야 한다.

- Voltage / Current / Resistance
- `V = I × R`
- 3.3V Logic vs 5V Logic
- Common GND
- GPIO 최대 전압/전류
- Sensor Supply Voltage
- Regulator / Buck / UBEC
- Motor Stall Current

```text
Battery
├ Motor Power → Driver → Motor
└ Regulator → 5V/3.3V Logic → STM32 / Pi
```

모터를 MCU GPIO에 직접 연결하지 않는다.

---

# 2. MCU Peripheral

| Peripheral | 사용 예 |
|---|---|
| GPIO | Gear/Button, LED, Enable |
| ADC | Accelerator, Brake, Ambient, Battery voltage |
| Timer | Ultrasonic Echo, Encoder |
| PWM | Motor Driver, RC Servo, LED |
| UART | Debug, LIN 기반 peripheral |
| I2C | AS5600, 센서 |
| SPI | 외장 CAN FD Controller 등 |
| CAN/FDCAN | ECU Backbone |
| Interrupt | Echo, Encoder, CAN RX |

---

# 3. ADC

12-bit ADC 예:

```text
0    → 0V
2048 → 약 VREF/2
4095 → 약 VREF
```

```text
Voltage = ADC_RAW / ADC_MAX × VREF
```

항상 다음 순서로 본다.

```text
Raw → Voltage → Calibration → Physical Value → Validity
```

F의 Accelerator/Brake, D의 Ambient 같은 곳에서 사용한다.

---

# 4. Timer / Pulse Measurement

## Ultrasonic

```text
Trigger 출력
→ Echo pulse width 측정
→ 시간 → 거리
```

필요 개념:

- microsecond
- Timer counter
- Input Capture
- Timeout
- 측정 간 간섭

## Encoder / Hall

```text
Motor 회전
→ Pulse
→ Timer/Counter
→ RPM
```

필요 개념:

- PPR/CPR
- Frequency
- Direction
- Sampling period

---

# 5. PWM

PWM은 MCU가 전력을 공급하는 것이 아니라 **Driver에 명령을 주는 신호**다.

```text
STM32 PWM
→ TB6612FNG 후보
→ Brushed DC Motor
```

RC Servo도 Timer PWM/pulse로 명령한다.

필요 개념:

- Frequency
- Duty
- Pulse width
- Timer period/prescaler
- Safe output range

---

# 6. I2C / UART / SPI

## I2C

필요 개념:

- SDA/SCL
- Address
- Pull-up
- 3.3V/5V
- ACK/NACK

F의 Steering Wheel AS5600 후보 등에 사용할 수 있다.

## UART

Stage 1 디버깅의 기본이다.

```text
TX → USB UART RX
RX ← USB UART TX
GND 공유
```

## SPI

외장 CAN FD controller를 사용할 경우 필요할 수 있다.

---

# 7. CAN / CAN FD

```text
STM32 FDCAN
→ CAN Transceiver
→ CANH / CANL
→ Shared Bus
```

필수 개념:

- Controller vs Transceiver
- CANH/CANL differential
- 120Ω termination
- CAN ID
- DLC / Payload
- Arbitration
- Periodic/Event
- Timeout
- Bus-off
- CAN FD와 Classic CAN 차이

Pi에는 내장 CAN FD가 없으므로 외장 CAN FD 인터페이스가 필요하다.

F 담당은 CAN Matrix를 특히 깊게 알아야 하고, 모든 담당자는 자기 TX/RX signal은 이해해야 한다.

---

# 8. LIN과 CAN↔LIN Gateway

LIN은 Body의 단순·저속 장치를 연결하는 데 사용한다.

```text
LIN Master
   ↓ schedule
Slave Response
```

우리 프로젝트:

```text
CAN FD
 ↕
D: STM32 #3 Body Gateway
   ├ CAN FD
   └ LIN Master
        ↕ LIN Transceiver
D: STM32 #4 LIN Slave
   ├ Ambient Sensor
   └ Lighting
```

필수 개념:

- LIN Master / Slave
- Header / Response
- Schedule Table
- LIN Transceiver
- Frame ID / Data
- Checksum 개념
- Timeout

Gateway란 단순히 두 선을 꽂는 것이 아니다.

```text
CAN Signal 해석
→ Mapping
→ LIN Signal 생성
```

반대 방향도 동일하다.

---

# 9. Motor / Steering Control

C가 알아야 할 핵심:

```text
Target Speed
→ Controller
→ PWM
→ Driver
→ Motor
→ Encoder
→ Actual RPM
```

Stage 1은 Open-loop부터 시작한다.

PID는 Encoder/RPM이 신뢰 가능한 뒤에 한다.

RC Servo는 자체 위치제어가 있으므로 첫 버전은:

```text
Target Steering
→ PWM mapping
→ Servo
```

으로 시작할 수 있다.

---

# 10. Camera / Vision

E는 MCU 영상처리보다 Pi 기반 pipeline을 이해해야 한다.

필수:

- CSI / USB Camera 차이
- Resolution
- FPS
- Frame
- Exposure
- Color space
- ROI
- OpenCV
- Object Detection
- Lane Detection 기초
- Latency
- CPU/Memory/Temperature

전체 지연 개념:

```text
T_total = Capture + Preprocess + Inference + Decision + CAN
```

개발 중 Pi 두 대를 사용하더라도 최종 통합을 위해 Camera input, Vision logic, Result interface를 모듈로 분리한다.

---

# 11. VCU / State / Safety

F가 특히 알아야 한다.

```text
Driver Input
ADAS Request
Ultrasonic Warning
Fault
    ↓
Validate
    ↓
Vehicle Mode
    ↓
Arbitration
    ↓
Final Command
```

필수 개념:

- State Machine
- P/R/N/D
- Priority
- Plausibility
- Heartbeat
- Timeout
- Watchdog
- Fail-safe

---

# 12. DTC

DTC는 `printf("error")`가 아니다.

```text
Fault Detect
→ DTC Code
→ Status
→ CAN Event
→ Central Store
→ UI
```

상태 후보:

```text
PENDING
ACTIVE
HISTORY
CLEARED
```

프로젝트 역할:

- 각 담당: 자기 Node Fault 검출
- F: DTC 코드 규칙/통합/중요 fault action
- E의 Pi: DTC Manager service host/저장
- B: H735 Warning/Diagnostics UI

---

# 13. 역할별 선행학습

| 담당 | 먼저 배울 것 | 다음 단계 |
|---|---|---|
| A Ultrasonic | GPIO, Timer, Input Capture, timeout | filtering, multi-sensor scheduling, CAN |
| B H735 UI | H735, TouchGFX, basic CAN data model | warning priority, DTC UI |
| C Drive/Steer | PWM, Driver, Encoder, power | RPM, PID, fail-safe |
| D LIN-CAN Body | GPIO/PWM/ADC, UART/LIN, transceiver | LIN schedule, gateway mapping, DTC |
| E HPC Vision | Linux, Camera, OpenCV, Python/C++ | AI inference, latency, SocketCAN/service |
| F VCU/DTC/CAN | GPIO/ADC/I2C, CAN/FDCAN, state machine | arbitration, heartbeat, DTC, integration |

---

# 14. 추천 학습 순서

```text
Day 1: Voltage / GND / Multimeter
Day 2: GPIO / ADC / PWM / UART
Day 3: Timer / I2C / 담당 센서
Day 4: 자기 Node Stage 1 Bring-up
Day 5: Interrupt / Filtering / Fault
Day 6: CAN / Transceiver / Termination
Day 7: Architecture / State / DTC / LIN 해당 담당
```

목표는 코드를 많이 아는 사람이 아니라 **Hardware → Signal → Firmware → Network → Application 경로를 추적할 수 있는 사람**이 되는 것이다.
