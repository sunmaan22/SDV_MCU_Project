# Software 전공자를 위한 전자공학 선행학습 가이드

> 대상: STM32 / Raspberry Pi / CAN 기반 모빌리티 프로젝트를 처음 진행하는 Software 전공 팀원  
> 목표: 회로를 깊게 설계하는 전자공학 전공자 수준이 아니라, **센서·MCU·모터·통신을 안전하게 연결하고 문제를 스스로 디버깅할 수 있는 수준**까지 도달하는 것  
> 프로젝트 기준: Raspberry Pi 4 HPC + STM32 분산 ECU + CAN/CAN FD + ADAS + Parking + IVI + Cluster + DTC

---

# 0. 먼저 기억할 것

이 프로젝트에서 Software 전공자가 가장 많이 실수하는 지점은 코드가 아니다.

```text
잘못된 전압 인가
→ 센서/MCU 손상

GND 미공유
→ 통신 안 됨

Pull-up / Pull-down 누락
→ 입력값이 랜덤

모터를 MCU GPIO에 직접 연결
→ MCU 손상

CAN Transceiver 없이 CANH/CANL 연결 시도
→ 통신 안 됨

ADC Raw 값만 보고 실제 물리량이라고 생각
→ 제어값 오류
```

따라서 이 프로젝트에서 필요한 능력은 다음 순서로 잡는다.

```text
전압 / 전류 / GND 이해
        ↓
GPIO / ADC / PWM 이해
        ↓
I2C / SPI / UART 이해
        ↓
센서 Datasheet 읽기
        ↓
모터 / Driver / Encoder 이해
        ↓
CAN / CAN FD 이해
        ↓
실시간 제어 / State Machine
        ↓
차량 아키텍처 / DTC / Fail-safe
```

**Oscilloscope를 잘 쓰는 사람보다 먼저 Multimeter를 제대로 쓰는 사람이 되는 것이 우선이다.**

---

# 1. 공통 필수 전자공학 지식

## 1.1 전압, 전류, 저항

최소한 아래 관계는 바로 이해해야 한다.

```text
V = I × R
```

- Voltage: 두 점 사이의 전위차
- Current: 회로에 흐르는 전류
- Resistance: 전류 흐름을 제한하는 요소

예시:

```text
3.3 V GPIO
  ↓
220 Ω resistor
  ↓
LED
  ↓
GND
```

LED에 저항 없이 GPIO를 바로 연결하지 않는다.

### 반드시 알아야 할 것

- 3.3 V Logic와 5 V Logic의 차이
- MCU GPIO의 최대 허용 전압
- 센서 전원전압 `VCC`
- GND 기준
- 입력핀과 출력핀의 차이
- GPIO가 공급 가능한 전류는 매우 작다는 점

---

## 1.2 Common Ground

두 보드가 신호를 주고받으려면 대부분 기준 전압이 같아야 한다.

```text
STM32 GND ───────── Sensor GND
STM32 3.3V ──────── Sensor VCC
STM32 SDA ───────── Sensor SDA
STM32 SCL ───────── Sensor SCL
```

GND를 공유하지 않으면 신호가 있어도 기준이 없어 정상적으로 해석하지 못할 수 있다.

### 프로젝트에서 특히 중요한 구간

- STM32 ↔ Sensor
- STM32 ↔ Motor Driver
- STM32 ↔ CAN Transceiver
- Raspberry Pi ↔ CAN Interface
- Battery / UBEC / Logic Power

단, 실제 고전력 시스템은 Ground Loop, Isolation 같은 더 복잡한 문제가 있지만 Stage 1에서는 **공통 GND와 전원 분리 개념**을 먼저 이해한다.

---

# 2. MCU 기본 구조

STM32를 단순히 "작은 컴퓨터"라고만 보면 부족하다.

```text
               STM32 MCU

        ┌────────────────────┐
GPIO ───┤                    ├── PWM
ADC  ───┤        CPU         ├── UART
I2C  ───┤                    ├── SPI
CAN  ───┤ Peripheral / Timer ├── Interrupt
        └────────────────────┘
```

Software 전공자가 꼭 알아야 할 Peripheral은 다음과 같다.

| Peripheral | 프로젝트 사용 예 |
|---|---|
| GPIO | Switch, LED, Enable pin |
| ADC | 온도센서, 가변저항, 아날로그 센서 |
| Timer | 주기 제어, Encoder 측정 |
| PWM | Motor speed, Servo, LED brightness |
| Input Capture | Encoder / pulse width 측정 |
| UART | Debug log |
| I2C | ToF, IMU, display sensor |
| SPI | CAN controller, display, sensor |
| CAN / FDCAN | ECU 간 차량 통신 |
| Interrupt | Encoder, sensor event, CAN RX |

---

# 3. GPIO를 제대로 이해하기

GPIO는 단순히 HIGH / LOW가 아니다.

## 3.1 Input

예:

```text
Button
  ↓
GPIO Input
```

입력이 아무것에도 연결되지 않으면 Floating 상태가 될 수 있다.

```text
3.3 V
 │
10 kΩ
 │
GPIO ─── Button ─── GND
```

이런 구조가 Pull-up이다.

반대로 GND 방향으로 저항을 두는 것은 Pull-down이다.

### 알아야 할 개념

- Input / Output
- Push-pull
- Open-drain
- Pull-up
- Pull-down
- Floating
- Active High / Active Low
- Debouncing

---

# 4. ADC와 센서값

ADC는 전압을 Digital Number로 바꾼다.

예를 들어 12-bit ADC라면:

```text
0 ~ 4095
```

3.3 V 기준이라면 대략:

```text
0     → 0 V
2048  → 약 1.65 V
4095  → 약 3.3 V
```

기본 관계:

```text
Voltage = ADC_RAW / ADC_MAX × VREF
```

하지만 ADC 값이 바로 온도나 압력은 아니다.

```text
ADC Raw
  ↓
Voltage
  ↓
Sensor Transfer Function
  ↓
Physical Value
```

예:

```text
ADC = 2480
↓
Voltage = 2.00 V
↓
Sensor equation
↓
Temperature = 41.2 °C
```

### 반드시 알아야 할 것

- ADC Resolution
- VREF
- Sampling
- Noise
- Moving Average / Low Pass Filter
- Calibration
- Sensor output range

---

# 5. PWM

PWM은 디지털 신호를 빠르게 ON/OFF하여 평균 출력 효과를 만든다.

```text
Duty 25%

HIGH ──┐      ┌─
       │      │
LOW    └──────┘
```

```text
Duty 75%

HIGH ──────┐  ┌────
           │  │
LOW        └──┘
```

프로젝트 사용 예:

- Motor Driver speed command
- Servo pulse
- LED brightness

### 꼭 구분할 것

`PWM GPIO → Motor`가 아니다.

올바른 구조:

```text
STM32 PWM
   ↓
Motor Driver
   ↓
Motor
```

MCU는 제어 신호를 만들고, 실제 전력은 Driver가 공급한다.

---

# 6. 센서 통신 인터페이스

## 6.1 I2C

기본 구조:

```text
MCU
 │
 ├── SDA
 └── SCL
      │
   Sensor
```

특징:

- 두 선으로 여러 Device 연결 가능
- 각 Device가 Address를 가짐
- SDA / SCL에 Pull-up이 필요

확인해야 할 것:

- Device address
- 3.3 V / 5 V
- Pull-up 유무
- Clock speed
- Register map

Parking용 ToF 센서가 대표적인 예다.

---

## 6.2 SPI

```text
MCU
 ├ MOSI
 ├ MISO
 ├ SCK
 └ CS
```

특징:

- 빠름
- Slave마다 보통 CS 필요
- 배선이 I2C보다 많음

프로젝트에서는 외장 CAN FD controller, display 등에 사용할 수 있다.

---

## 6.3 UART

```text
MCU TX → USB-UART RX
MCU RX ← USB-UART TX
GND    ───────── GND
```

Stage 1에서 가장 중요한 디버깅 수단 중 하나다.

예:

```text
[Parking ECU]
FL = 523 mm
FR = 498 mm
RL = 810 mm
RR = 793 mm
```

이런 로그를 UART로 출력할 수 있어야 한다.

---

# 7. Datasheet 읽는 법

Datasheet를 처음부터 끝까지 읽을 필요는 없다.

센서라면 먼저 다음을 찾는다.

```text
1. Supply Voltage
2. Logic Voltage
3. Pinout
4. Interface
5. Address / Register
6. Measurement Range
7. Accuracy
8. Timing
9. Recommended Circuit
10. Absolute Maximum Rating
```

Motor Driver라면:

```text
1. Motor Supply Voltage
2. Logic Supply Voltage
3. Continuous Current
4. Peak Current
5. PWM input
6. Direction input
7. Protection function
8. Thermal limit
```

MCU 보드라면:

```text
1. GPIO Voltage
2. Pin alternate function
3. ADC capable pins
4. Timer/PWM channels
5. I2C/SPI/UART pins
6. CAN/FDCAN support
7. Board power input
```

---

# 8. 계측 장비 사용

## 8.1 Multimeter

Stage 1에서 모든 팀원이 최소한 다음은 할 수 있어야 한다.

- DC Voltage 측정
- Continuity 확인
- GND 확인
- 전원 polarity 확인
- 예상 전압과 실제 전압 비교

센서가 안 되면 코드부터 보지 말고 먼저:

```text
VCC-GND = ? V
```

부터 측정한다.

---

## 8.2 Oscilloscope

가능하면 다음 단계에서 배운다.

- PWM waveform
- UART waveform
- I2C SCL/SDA
- Encoder pulse
- CANH/CANL

처음부터 모든 팀원이 Oscilloscope 전문가일 필요는 없지만, A/B 역할은 기본 사용법을 익히는 것이 좋다.

---

# 9. 모터 / Encoder / 조향 제어 기초

## 9.1 Open-loop와 Closed-loop

Open-loop:

```text
PWM 50%
 ↓
Motor
```

실제 속도가 얼마인지 모른다.

Closed-loop:

```text
Target Speed
    ↓
Controller
    ↓
PWM
    ↓
Motor
    ↓
Encoder
    └──────── Feedback
```

프로젝트에서는 가능하면 Closed-loop를 구현한다.

---

## 9.2 Encoder

Encoder는 회전을 pulse로 알려준다.

```text
Motor rotation
   ↓
Encoder Pulse
   ↓
STM32 Timer / Interrupt
   ↓
RPM 계산
```

예:

```text
PPR = Pulse Per Revolution
```

일정 시간 동안 pulse 수를 세면 RPM을 계산할 수 있다.

### 알아야 할 것

- Pulse
- Frequency
- PPR / CPR
- Timer Counter
- Input Capture
- Direction

---

# 10. 제어 기초: PID

모터 속도나 조향각 제어에서 다음 개념이 나온다.

```text
Error = Target - Actual
```

PID:

```text
Output = P + I + D
```

각 항의 의미를 수학적으로 완벽히 증명하는 것보다 먼저 다음을 이해한다.

- P: 현재 오차에 반응
- I: 누적 오차 보정
- D: 변화 속도에 반응

Stage 1에서는 먼저 센서/모터가 정상 동작하는지 확인하고, PID tuning은 그 다음이다.

---

# 11. CAN / CAN FD 선행지식

CAN은 UART처럼 `TX → RX` 단순 연결이 아니다.

```text
STM32 FDCAN Peripheral
        ↓
CAN Transceiver
        ↓
CANH / CANL
        ↓
Shared CAN Bus
```

필수 개념:

- CAN Controller와 CAN Transceiver 차이
- CANH / CANL
- Differential Signal
- 120 Ω termination
- CAN ID
- Arbitration
- DLC
- Payload
- Bitrate
- Periodic / Event message
- Timeout
- Bus-off

### Controller와 Transceiver

MCU 내부:

```text
FDCAN Controller
```

실제 Bus 전기신호 변환:

```text
CAN Transceiver
```

둘은 다른 것이다.

```text
STM32
FDCAN_TX/RX
    ↓
Transceiver
    ↓
CANH/CANL
```

이 개념을 모르면 CAN 핀을 바로 CANH/CANL에 꽂는 꽤 창의적인 사고가 발생한다.

---

# 12. 전원 설계 기초

차량 프로젝트는 Logic과 Motor가 같이 있기 때문에 전원이 중요하다.

예:

```text
Battery
   │
   ├── Motor Power
   │      ↓
   │   Motor Driver
   │
   └── Regulator / UBEC
          ↓
       5 V Logic
          ↓
       STM32 / Pi
```

### 알아야 할 것

- Battery voltage
- Regulator
- Buck converter
- Current capacity
- Motor stall current
- Logic rail과 motor rail
- Common GND
- Reverse polarity
- Fuse 개념

특히 Motor stall current는 반드시 확인한다.

평상시 500 mA라고 해서 500 mA Driver를 고르면 안 된다.

---

# 13. RTOS / Embedded Software에서 필요한 전자공학 관점

Software 전공자에게 익숙한 Thread 개념과 비슷하지만 MCU에서는 Timing이 더 중요하다.

예:

```text
1 ms   Motor Control Task
10 ms  Sensor Task
20 ms  CAN Tx Task
100 ms DTC Monitor Task
```

알아야 할 개념:

- Task period
- Interrupt
- Priority
- Shared variable
- Mutex / Queue
- Blocking
- Deadline
- Watchdog

제어 시스템에서는 "결과가 맞다"뿐 아니라 **제때 나오는가**도 중요하다.

---

# 14. 차량 시스템에서 추가로 알아야 할 개념

## 14.1 State Machine

차량 모드는 단순 boolean 여러 개보다 State Machine으로 관리한다.

```text
IDLE
 ↓
DRIVE
 ↓
REVERSE
 ↓
FAULT
```

예:

```text
Gear = R
→ PARKING mode
→ Rear Camera ON
→ Front ADAS pause
```

---

## 14.2 Heartbeat

ECU가 살아 있는지 확인한다.

```text
Drive ECU
  ↓ every 100 ms
HEARTBEAT
```

VCU에서 timeout:

```text
Heartbeat 없음
→ COMM FAULT
→ Motor Stop
```

---

## 14.3 DTC

DTC는 단순 `printf("error")`가 아니다.

```text
Fault detection
 ↓
Fault code
 ↓
Status
 ↓
Timestamp / occurrence
 ↓
Central diagnostics
```

예:

```text
PARK_001
Rear ToF Timeout
ACTIVE
```

---

# 15. 역할별 필요한 선행학습

| 역할 | 필수 전자공학 / 임베디드 지식 | 추가 학습 |
|---|---|---|
| **A - VCU / CAN Lead** | GPIO, CAN/FDCAN, Transceiver, State Machine, Interrupt, Watchdog | Arbitration, Heartbeat, DTC, fail-safe |
| **B - Drive + Steering** | PWM, Timer, Encoder, Motor Driver, Power, ADC | PID, closed-loop, current/temperature monitoring |
| **C - ADAS / HPC** | Pi power, Camera interface, CAN interface, basic signal timing | OpenCV, CV, inference, latency, camera calibration |
| **D - Parking Assist** | I2C, GPIO, ToF/Ultrasonic, sensor timing | sensor filtering, multi-sensor scheduling, parking vision |
| **E - IVI / H735** | STM32H7 basics, display interface, FDCAN, TouchGFX | GUI task timing, diagnostics UI, event handling |
| **F - Body + Cluster** | GPIO, PWM, TFT/SPI, ADC, lighting output | warning logic, cluster update rate, HMI prioritization |

---

# 16. 최소 선행학습 순서

## Level 0 — 회로를 태우지 않는 단계

- [ ] Voltage / Current / Resistance
- [ ] 3.3 V vs 5 V
- [ ] GND
- [ ] Multimeter
- [ ] Datasheet의 Supply Voltage / Pinout 읽기

## Level 1 — MCU Peripheral

- [ ] GPIO Input / Output
- [ ] Pull-up / Pull-down
- [ ] ADC
- [ ] PWM
- [ ] Timer
- [ ] UART debug

## Level 2 — Sensor / Actuator

- [ ] I2C
- [ ] SPI
- [ ] Encoder
- [ ] Motor Driver
- [ ] ToF / Ultrasonic
- [ ] Sensor filtering

## Level 3 — Vehicle Network

- [ ] CAN Controller
- [ ] CAN Transceiver
- [ ] CANH / CANL
- [ ] Termination
- [ ] CAN ID / Signal
- [ ] CAN FD 차이
- [ ] Heartbeat / Timeout

## Level 4 — System Integration

- [ ] State Machine
- [ ] PID basics
- [ ] RTOS scheduling
- [ ] DTC
- [ ] Fail-safe
- [ ] Latency measurement

---

# 17. ADAS 담당자를 예시로 보면 무엇을 공부해야 하는가

ADAS 담당 C를 예시로 전체 과정을 보자.

ADAS는 단순히 YOLO를 실행하는 역할이 아니다.

프로젝트에서의 실제 데이터 흐름은 다음과 같다.

```text
Front Camera
    ↓ CSI
Raspberry Pi 4
    ↓
Camera Driver / Frame Capture
    ↓
Image Preprocessing
    ↓
Computer Vision / AI
    ↓
ADAS Result
    ↓
CAN Interface
    ↓
VCU
    ↓
Drive / Steering ECU
```

따라서 ADAS 담당자는 Software뿐 아니라 **Camera hardware, timing, network interface까지 이해**해야 한다.

---

## 17.1 ADAS Level 1 — Camera가 왜 동작하는지 이해

먼저 알아야 할 것:

- Raspberry Pi Camera power
- CSI camera interface 개념
- Resolution
- FPS
- Exposure
- Frame
- Pixel
- RGB / YUV / grayscale

최초 목표:

```text
Camera 연결
→ Frame Capture 성공
→ 화면 출력
→ FPS 측정
```

이 단계에서 AI를 먼저 붙이지 않는다.

### Stage 1 결과 예

```text
Camera      : Raspberry Pi Camera
Resolution  : 640 x 480
Target FPS  : 30
Measured FPS: 28.7
Capture     : PASS
```

---

## 17.2 ADAS Level 2 — 영상처리 기초

알아야 할 것:

- Image coordinate
- ROI
- Resize
- Color conversion
- Threshold
- Edge
- Contour
- Perspective

차선 인식의 가장 단순한 개념:

```text
Camera Frame
   ↓
ROI
   ↓
Gray / Edge
   ↓
Line Detection
   ↓
Left Lane / Right Lane
   ↓
Lane Center
```

차량 중앙과 Lane 중앙의 차이를 구하면:

```text
Lane Offset
```

을 만들 수 있다.

예:

```text
Image Center = 320 px
Lane Center  = 345 px

Offset = +25 px
```

이 값을 실제 steering request로 바로 사용하지 말고 이후 VCU와 제어 규칙을 거친다.

---

# 18. ADAS Level 3 — Object Detection

AI 모델을 사용한다면 이해해야 할 것:

- Input resolution
- Model inference
- Bounding Box
- Class
- Confidence
- FPS
- Latency

출력 예:

```text
class      = person
confidence = 0.91
bbox       = [x1, y1, x2, y2]
```

중요한 점은 `person detected`가 곧바로 `motor stop`은 아니라는 것이다.

ADAS Application에서 의미를 판단한다.

```text
Detection
  ↓
ROI / Distance / Risk 판단
  ↓
COLLISION_LEVEL
  ↓
Speed / Stop Request
```

---

# 19. ADAS Level 4 — 카메라 Calibration

카메라는 단순히 사진을 찍는 장치가 아니다.

렌즈 때문에 왜곡이 발생한다.

알아야 할 개념:

- Intrinsic Parameter
- Focal Length
- Principal Point
- Lens Distortion
- Camera height
- Camera angle

프로젝트 초기에 모든 Calibration을 깊게 할 필요는 없지만, 다음 사실은 이해해야 한다.

```text
Pixel Distance ≠ Real Distance
```

영상에서 100 px 떨어져 있다고 실제 100 cm가 아니다.

거리 추정이 필요하면 Calibration 또는 별도 거리센서와 결합해야 한다.

이 때문에 Parking에서는 Camera만 믿지 않고 ToF/Ultrasonic을 함께 사용한다.

---

# 20. ADAS Level 5 — Latency

ADAS에서 정확도만 보는 것은 부족하다.

```text
Camera
 ↓
Frame Capture
 ↓
Inference
 ↓
ADAS Logic
 ↓
CAN Tx
 ↓
VCU
```

각 단계에 시간이 걸린다.

전체 지연:

```text
T_total
= T_capture
+ T_preprocess
+ T_inference
+ T_decision
+ T_CAN
```

예:

```text
Capture      20 ms
Preprocess    5 ms
Inference    45 ms
Decision      2 ms
CAN           3 ms
----------------
Total        75 ms
```

따라서 ADAS 담당자는 FPS와 latency를 모두 기록해야 한다.

---

# 21. ADAS Level 6 — CAN으로 무엇을 보낼 것인가

절대 Camera frame 전체를 CAN으로 보내지 않는다.

잘못된 생각:

```text
Camera Image
→ CAN FD
→ VCU
```

올바른 방식:

```text
Camera Image
→ Pi 내부 CV 처리
→ 의미 있는 결과만 CAN
```

예:

```text
ADAS_STATE        ACTIVE
LANE_OFFSET       -35 mm
LANE_ANGLE        +2.4 deg
OBJECT_DETECTED   1
OBJECT_TYPE       PERSON
COLLISION_LEVEL   WARNING
SPEED_REQUEST     0.25 m/s
STEERING_REQUEST  +3.0 deg
```

이 값도 VCU가 최종 중재한다.

```text
ADAS Request
    ↓
VCU Safety / Arbitration
    ↓
Final Command
    ↓
Drive / Steering ECU
```

---

# 22. ADAS 담당자의 Stage 1 실제 해야 할 일

Stage 1에서는 다음까지만 완료하면 된다.

```text
1. Raspberry Pi 4 부팅
2. Camera 인식
3. Frame capture
4. 화면 출력
5. FPS 측정
6. OpenCV에서 frame read
7. 간단한 ROI / grayscale 확인
8. Camera disconnect 오류 확인
9. CPU / temperature / FPS 기록
10. 결과 문서화
```

### Stage 1 PASS 기준 예

- [ ] Camera가 재부팅 후에도 인식된다.
- [ ] 640×480 또는 목표 해상도에서 안정적으로 frame이 들어온다.
- [ ] 최소 1분 이상 frame drop 없이 동작한다.
- [ ] FPS를 측정했다.
- [ ] Camera disconnect 시 프로그램이 죽지 않고 오류를 기록한다.
- [ ] Raw 영상은 Pi 내부에만 존재한다.
- [ ] 향후 CAN으로 보낼 ADAS signal 후보를 정리했다.

Stage 1에서 아직 필요 없는 것:

- 완성형 YOLO
- 자동조향
- VCU 연동
- CAN 송신
- Collision control

먼저 Camera → Frame까지 확실히 만드는 것이 우선이다.

---

# 23. ADAS 담당 Architecture 예시

```text
[Hardware]

Raspberry Pi Camera
        ↓ CSI
Raspberry Pi 4
        ↓
CAN FD Interface


[Software]

Camera Service
      ↓
Preprocessing
      ↓
Lane / Object Detection
      ↓
ADAS Decision
      ↓
CAN Service
      ↓
VCU
```

### Input / Process / Output

| 구분 | 내용 |
|---|---|
| Input | Front Camera Frame, Vehicle Mode |
| Process | Lane / Object Detection, Risk 판단 |
| Output | Lane Offset, Object, Warning, Speed/Steering Request |
| Failure | Camera Timeout, Frame Drop, Inference Failure |
| Network | CAN/CAN FD to VCU / IVI |

### 필요한 기술스택

```text
Linux
Python / C++
OpenCV
libcamera / Picamera2
Image Processing
AI Inference
Camera Calibration basics
CAN / SocketCAN
Systemd / Process management
Logging
Latency measurement
```

전자공학 관점에서는:

```text
Camera interface
Power stability
Ground
CAN transceiver/interface
Timing
Signal flow
```

을 이해해야 한다.

---

# 24. 역할별 Stage 1에서 전자공학적으로 확인할 항목

## A — VCU

- [ ] GPIO switch 입력
- [ ] Pull-up / Pull-down
- [ ] Gear input mock
- [ ] Emergency stop input
- [ ] UART debug
- [ ] FDCAN peripheral 지원 여부 확인

## B — Drive / Steering

- [ ] Motor Driver 전원
- [ ] PWM
- [ ] Motor direction
- [ ] Encoder pulse
- [ ] Steering feedback
- [ ] Motor current / temperature 확인

## C — ADAS

- [ ] Pi power
- [ ] CSI Camera
- [ ] Camera FPS
- [ ] USB / CAN interface 후보
- [ ] CPU temperature

## D — Parking

- [ ] ToF / Ultrasonic voltage
- [ ] I2C / GPIO
- [ ] Sensor address
- [ ] multi-sensor interference
- [ ] Rear USB Camera

## E — IVI

- [ ] H735 power
- [ ] LCD / Touch interface
- [ ] TouchGFX frame update
- [ ] FDCAN interface
- [ ] dummy vehicle data display

## F — Body / Cluster

- [ ] LED driver / resistor
- [ ] GPIO / PWM
- [ ] TFT interface
- [ ] warning lamp logic
- [ ] dummy speed / RPM display

---

# 25. 팀 공통 학습 체크리스트

CAN 통합을 시작하기 전에 팀원 전원이 최소한 다음 질문에 답할 수 있어야 한다.

- [ ] 내가 쓰는 보드는 3.3 V Logic인가?
- [ ] 센서 VCC는 몇 V인가?
- [ ] 센서 출력이 MCU 입력 허용범위를 넘지 않는가?
- [ ] GND는 어디에 연결하는가?
- [ ] GPIO에 Motor를 직접 연결하면 왜 안 되는가?
- [ ] ADC Raw와 실제 물리량의 차이는 무엇인가?
- [ ] I2C SDA/SCL 역할은 무엇인가?
- [ ] Pull-up은 왜 필요한가?
- [ ] PWM Duty가 무엇인가?
- [ ] Encoder가 무엇을 측정하는가?
- [ ] CAN Controller와 Transceiver 차이는 무엇인가?
- [ ] CAN bus 양 끝에 termination이 왜 필요한가?
- [ ] ECU가 통신 두절되면 어떤 상태로 가야 하는가?
- [ ] 내 ECU가 Input / Process / Output 중 무엇을 담당하는가?

---

# 26. 추천 학습 순서

한 번에 차량 전체를 공부하지 않는다.

```text
Day 1
전압 / 전류 / GND / Multimeter

Day 2
GPIO / ADC / PWM / UART

Day 3
I2C / SPI + 담당 센서 Datasheet

Day 4
담당 ECU 센서 / 액추에이터 Bring-up

Day 5
Timer / Interrupt / Encoder / Filtering

Day 6
CAN / Transceiver / Termination

Day 7
Architecture / CAN Signal / State Machine
```

이후 역할별 심화로 들어간다.

```text
ADAS → OpenCV / Camera / AI / Latency
Drive → Motor / Encoder / PID
Parking → ToF / Sensor scheduling
IVI → TouchGFX / HMI / CAN model
Body → GPIO / PWM / Lighting
VCU → CAN / State / Safety / DTC
```

---

# 27. 최종 목표

이 프로젝트에서 Software 전공 팀원이 전자공학을 배운다는 의미는 회로이론 시험 문제를 푸는 것이 아니다.

최종적으로 다음 상황에서 원인을 추적할 수 있으면 된다.

```text
센서 값이 안 들어온다
→ 전원 / GND / Pin / Protocol / Code 확인

모터가 안 돈다
→ Battery / Driver / PWM / Enable / Motor 확인

CAN이 안 된다
→ Controller / Transceiver / CANH/L / Termination / Bitrate 확인

카메라 FPS가 낮다
→ Resolution / Exposure / CPU / Inference / Pipeline 확인

차량이 이상하게 제어된다
→ Sensor → ECU → CAN → VCU → Actuator 데이터 흐름 확인
```

즉 목표는 다음이다.

> **코드만 보는 개발자에서, Hardware → Signal → Firmware → Network → Application 전체 경로를 추적할 수 있는 Embedded / Mobility Software 개발자로 올라가는 것.**

---

## 관련 문서

- [Beginner Architecture & Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
- [ECU Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)
- [Main Project README](../README.md)
