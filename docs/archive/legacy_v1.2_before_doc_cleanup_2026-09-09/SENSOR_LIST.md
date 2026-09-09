# Sensor / Input / Feedback List

> Architecture v1.2 기준. 부품 모델은 일부 후보 상태이며 실제 구매/보유 부품 확인 후 확정한다.

---

# 1. 전체 목록

| 영역 | 장치 | 목적 | Interface | Owner | 상태 |
|---|---|---|---|---|---|
| Parking | Ultrasonic Sensor x N | 장애물 거리 | Trigger/Echo Timer/GPIO | A | 핵심 |
| Driver | P/R/N/D Buttons | Gear 선택 | GPIO | F | 핵심 |
| Driver | Accelerator Position | 가속 입력 | ADC | F | 후보: 10k Pot / Hall |
| Driver | Brake Position | 브레이크 입력 | ADC | F | 후보: 10k Pot |
| Driver | Steering Wheel Angle | 운전자 조향 입력 | I2C/Analog | F | 후보: AS5600 |
| Safety | E-Stop Button | 즉시 정지 요청 | GPIO | F | 권장 |
| Drive | Motor Encoder/Hall | RPM/속도 Feedback | Timer/GPIO | C | 권장 |
| Drive | Motor Temperature | 모터 온도 | ADC/I2C | C | 선택 확장 |
| Vision | Front Camera | Lane/Object/ADAS | CSI | E | 핵심 |
| Vision | Rear Camera | Parking Vision | USB 최종안 | E | 핵심 |
| Body | Ambient Light Sensor | Auto-light 판단용 조도 | ADC/I2C | D LIN Slave | 핵심 후보 |
| Battery | Battery Voltage | 전압/저전압 경고 | ADC + divider | F | 권장 후보 |
| Battery | Battery Temperature | 배터리 온도 | ADC/I2C | F | 선택 |
| Battery | Current Sensor | 소비전류/전력 | ADC/I2C | F | 선택 |

B의 STM32H735는 기본적으로 센서 Owner가 아니다. 다른 Node가 만든 값을 CAN으로 받아 표시한다.

---

# 2. A — Ultrasonic

기본 구조:

```text
Ultrasonic Sensor
├ Trigger ← STM32 GPIO/Timer
└ Echo    → STM32 Timer/Input Capture
```

출력 후보:

```text
distance_mm
valid
warning_level
sensor_timeout
```

초기에는 센서 1개부터 시작하고 이후 Rear Left/Rear Right 또는 차량 배치에 맞게 확장한다.

여러 초음파센서를 동시에 발사하면 간섭이 생길 수 있으므로 순차 측정을 고려한다.

---

# 3. F — Driver Input

## Gear

```text
[P] [R] [N] [D]
      ↓ GPIO
      VCU
```

버튼 4개 또는 로터리/스위치 구조를 사용할 수 있다.

## Accelerator

초기 후보:

```text
10k Linear Potentiometer
또는 Hall Position Sensor
→ ADC
→ 0~100%
```

## Brake

초기 버전:

```text
10k Linear Potentiometer
→ ADC
→ 0~100%
```

## Steering Wheel

후보:

```text
Steering shaft
→ Magnet
→ AS5600 계열
→ I2C
→ relative angle / -100~100%
```

Center calibration이 필요하다.

---

# 4. C — Drive Feedback

## Motor Encoder / Hall

```text
Brushed DC Motor
→ Encoder/Hall
→ STM32 Timer
→ RPM
```

사용 목적:

- RPM 표시
- 속도 추정
- Speed PID Feedback
- Encoder Fault 검출

RC Servo 조향은 첫 버전에서 별도 실제 Wheel Angle Sensor 없이 시작할 수 있다. 필요하면 추후 AS5600/Potentiometer를 조향 링크에 추가한다.

---

# 5. E — Cameras

## Front

```text
Front Pi Camera
→ CSI
→ Raspberry Pi
→ Front ADAS Vision
```

## Rear

최종 기본안:

```text
Rear USB Camera
→ Raspberry Pi
→ Parking Vision
```

개발 중에는 Pi를 두 대 사용할 수 있다.

```text
Pi #1: Front
Pi #2: Rear
```

최종은 Pi 하나로 합친다.

Raw Frame은 CAN으로 보내지 않고 다음과 같은 결과만 보낸다.

```text
lane_offset
object_detected
object_position
collision_level
parking_vision_warning
speed_request
steering_request
```

---

# 6. D — Ambient / Lighting Input

Ambient Sensor는 LIN Slave가 직접 읽는다.

간단한 첫 버전:

```text
CDS + resistor divider
→ STM32 #4 ADC
→ ambient_raw
→ LIN Ambient_Status
```

또는 Digital Ambient Light Sensor를 사용할 수 있다.

```text
Ambient
→ LIN Slave
→ LIN
→ Body Gateway
→ CAN FD
→ H735 / VCU / HPC
```

---

# 7. Battery Monitoring

VCU 쪽 선택 확장 기능으로 둔다.

Battery 전압이 MCU ADC 범위를 넘으면 직접 연결하지 않는다.

```text
Battery
→ Voltage Divider / Measurement Circuit
→ ADC
```

SOC는 전압 하나만으로 정확히 계산하기 어렵기 때문에 프로젝트에서는 `estimated_soc` 또는 단순 Battery Gauge로 명확히 표현한다.

---

# 8. Stage 1 우선순위

## 반드시 먼저

- [ ] A: Ultrasonic 1개 거리/timeout
- [ ] F: Gear GPIO
- [ ] F: Accelerator ADC
- [ ] F: Brake ADC
- [ ] F: Steering Wheel angle
- [ ] C: Motor PWM + Encoder/Hall
- [ ] C: RC Servo
- [ ] E: Front Camera capture
- [ ] E: Rear Camera capture
- [ ] D: Ambient sensor
- [ ] D: Lamp GPIO/PWM

## 나중에

- [ ] Battery monitoring
- [ ] Motor temperature
- [ ] Steering actual feedback
- [ ] Dual-channel accelerator plausibility
- [ ] Current sensor

---

# 9. 구매 전 확인

```text
Operating Voltage
Logic Level
Pinout
Interface
Current Consumption
Measurement Range
Update Rate
Required Pull-up/Pull-down
MCU Pin Compatibility
```

센서를 코드보다 먼저 태워버리면 펌웨어 버그가 아니어서 디버거도 도와주지 않는다.
