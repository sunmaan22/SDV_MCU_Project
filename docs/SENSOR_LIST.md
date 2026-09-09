# Sensor List

> 본 문서는 현재 `SDV_MCU_Project`에서 사용을 상정한 센서와 운전자 입력 장치를 정리한다.  
> 아직 최종 부품 선정 전이므로 **확정 / 후보 / 선택 확장** 상태를 구분한다.

---

# 1. 전체 센서 구성 요약

| 영역 | 센서 / 입력 장치 | 목적 | 인터페이스 | 연결 대상 | 상태 |
|---|---|---|---|---|---|
| Driver Input | P/R/N/D 버튼 | 기어 선택 | GPIO | VCU STM32 | 확정 방향 |
| Driver Input | Accelerator Position Sensor | 엑셀 입력 0~100% | ADC | VCU STM32 | 후보 |
| Driver Input | Brake Position Sensor | 브레이크 입력 0~100% | ADC | VCU STM32 | 후보 |
| Driver Input | Steering Wheel Angle Sensor | 운전자 조향 입력 | I2C / Analog / PWM | VCU STM32 | 후보 |
| Drive | Motor Encoder / Hall Sensor | DC 모터 RPM / 속도 측정 | Timer Input / GPIO | Drive ECU STM32 | 권장 |
| Steering | Steering Feedback Sensor | 실제 조향각 피드백 | I2C / ADC | Drive/Steering ECU | 선택 확장 |
| ADAS | Front Camera | 차선 / 객체 / 전방 위험 인식 | CSI | Raspberry Pi 4 HPC | 확정 방향 |
| Parking | Rear Camera | 후방 영상 / 주차 보조 | USB | Raspberry Pi 4 HPC | 확정 방향 |
| Parking | ToF / Ultrasonic Sensors | 전후좌우 거리 측정 | I2C / GPIO | Parking ECU STM32 | 확정 방향 |
| Body | Ambient Light Sensor | Auto Light 판단 | ADC / I2C | Body ECU STM32 | 후보 |
| Thermal | Motor Temperature Sensor | 구동 모터 온도 감시 | ADC / I2C | Drive ECU STM32 | 선택 확장 |
| Thermal | Battery Temperature Sensor | 배터리 온도 감시 | ADC / I2C | VCU 또는 별도 측정 노드 | 후보 |
| Battery | Battery Voltage Measurement | 배터리 전압 / SOC 보조 정보 | ADC | VCU 또는 별도 측정 노드 | 후보 |
| Battery | Battery Current Sensor | 소비 전류 / 전력 추정 | ADC / I2C | VCU 또는 별도 측정 노드 | 선택 확장 |

---

# 2. Driver Input Sensors

## 2.1 Gear Input

초기 구현은 별도 위치 센서 대신 버튼으로 구성한다.

```text
[P] [R] [N] [D]
      ↓ GPIO
    VCU STM32
```

VCU 내부 표현 예시:

```text
GEAR_P
GEAR_R
GEAR_N
GEAR_D
```

Stage 1에서는 각 버튼 입력이 정확히 인식되는지만 확인한다.

---

## 2.2 Accelerator Position Sensor

### 1차 후보

- 10 kΩ Linear Potentiometer
- 또는 Analog Hall Position Sensor

```text
Accelerator Pedal
      ↓
Potentiometer / Hall Sensor
      ↓ 0 ~ 3.3 V
STM32 ADC
      ↓
Accelerator = 0 ~ 100 %
```

예시 변환식:

```text
Pedal[%] = (ADC - ADC_MIN) / (ADC_MAX - ADC_MIN) × 100
```

### Stage 1 목표

- 페달 최소 / 최대 ADC 값 측정
- 0~100% 정규화
- Noise 확인
- Sensor disconnect / out-of-range 상태 정의

### 선택 확장

실차의 센서 중복성 개념을 모사하려면 Accelerator Position Sensor를 2채널로 구성할 수 있다.

```text
APP1 = 43 %
APP2 = 42 %
→ VALID

APP1 = 81 %
APP2 = 15 %
→ PLAUSIBILITY FAULT
```

---

## 2.3 Brake Position Sensor

초기 구현은 엑셀과 동일하게 Potentiometer 기반으로 구성한다.

```text
Brake Pedal
   ↓
10 kΩ Linear Potentiometer
   ↓ ADC
VCU STM32
   ↓
Brake = 0 ~ 100 %
```

현재 프로젝트는 별도 유압 브레이크 시스템이 아니므로 브레이크 입력은 VCU에서 구동 모터의 목표 속도 / 토크를 낮추는 용도로 사용한다.

### 선택 확장

- Load Cell
- Force Sensitive Sensor

다만 Load Cell은 증폭 / Calibration이 추가되어 초기 구현에서는 후순위로 둔다.

---

## 2.4 Steering Wheel Angle Sensor

### 추천 후보

- AS5600 계열 Magnetic Angle Sensor + Magnet

```text
Steering Wheel
      ↓ Shaft
    Magnet
      ↓
AS5600
      ↓ I2C
VCU STM32
```

출력 예시:

```text
Steering Input = -100 ~ +100 %
```

또는

```text
Steering Wheel Angle = -180° ~ +180°
```

### Stage 1 목표

- Center 위치 Calibration
- 좌 / 우 최대각 확인
- Raw Angle 출력
- Center 기준 상대각 변환

---

# 3. Drive / Steering Sensors

## 3.1 Motor RPM Sensor

브러시드 DC Motor + TB6612FNG 구동부의 실제 회전속도 측정을 위해 Encoder 또는 Hall Sensor를 권장한다.

```text
Brushed DC Motor
      ↓
Encoder / Hall Sensor
      ↓
Drive ECU STM32 Timer
      ↓
RPM / Vehicle Speed
```

사용 목적:

- Motor RPM 표시
- Instrument Cluster RPM 표시
- Vehicle Speed 추정
- Speed PID의 Feedback

Stage 1에서는 다음까지만 성공하면 된다.

```text
Motor Rotation
→ Pulse Count
→ RPM 계산
→ UART 출력
```

---

## 3.2 Steering Feedback Sensor

현재 조향 액추에이터는 RC Servo를 사용할 예정이므로 **초기에는 별도 Steering Feedback Sensor를 생략할 수 있다.**

RC Servo 내부에서 자체 위치제어가 수행되므로:

```text
Steering ECU
   ↓ PWM
RC Servo
```

구조로 시작한다.

향후 실제 바퀴의 조향각을 측정하려면 다음을 추가한다.

- AS5600 / Magnetic Angle Sensor
- Potentiometer

```text
Target Steering Angle = +15°
Actual Steering Angle = +12°
```

처럼 Command와 Feedback을 분리할 수 있다.

---

# 4. ADAS Sensors

## 4.1 Front ADAS Camera

### 기본 구성

- Raspberry Pi Camera
- Raspberry Pi 4 CSI 연결

```text
Front Camera
    ↓ CSI
Raspberry Pi 4 HPC
    ↓
OpenCV / AI
```

주요 추출 정보 후보:

- Lane Detection
- Lane Offset
- Lane Angle
- Object Class
- Object Bounding Box
- Forward Collision Warning
- Steering Request
- Speed / Stop Request

중요:

```text
Camera Raw Frame
→ Pi 내부에서 처리
→ CAN/CAN FD에는 결과만 송신
```

예시:

```text
LANE_OFFSET      = -35 mm
LANE_ANGLE       = +2.8 deg
OBJECT_TYPE      = PERSON
COLLISION_LEVEL  = WARNING
```

---

# 5. Parking Sensors

## 5.1 Rear Parking Camera

### 기본 구성

- USB Camera
- Raspberry Pi 4 연결

```text
Gear R
  ↓
Rear USB Camera Active
  ↓
Raspberry Pi Parking Vision
```

주행 중에는 Front Camera / ADAS를 사용하고, R단에서는 Rear Camera / Parking Vision으로 전환한다.

---

## 5.2 ToF / Ultrasonic Distance Sensors

Parking ECU의 핵심 센서다.

구성 예시:

```text
Front Left  Sensor ─┐
Front Right Sensor ─┤
Rear Left   Sensor ─┤→ Parking ECU STM32
Rear Right  Sensor ─┘
```

최소 권장 구성:

- Rear Left
- Rear Right

확장 구성:

- Front Left
- Front Right
- Rear Left
- Rear Right

출력 예시:

```text
FL = 820 mm
FR = 760 mm
RL = 310 mm
RR = 280 mm
```

상태값 예시:

```text
> 500 mm       = SAFE
200 ~ 500 mm   = WARNING
< 200 mm       = CRITICAL
```

실제 임계값은 RC카 크기와 센서 특성에 맞춰 시험 후 확정한다.

---

# 6. Body / Lighting Sensors

## 6.1 Ambient Light Sensor

Auto Light 기능용.

후보:

- CDS Photoresistor
- Digital Ambient Light Sensor

```text
Ambient Light
    ↓
Body ECU STM32
    ↓
AUTO_LIGHT 판단
    ↓
Head Lamp ON / OFF
```

초기 구현은 CDS + ADC 방식이 가장 단순하다.

---

# 7. Temperature / Battery Sensors

현재 Cluster에서 Battery / Temperature 정보를 표시할 계획이므로 최소 하나 이상의 실제 온도 데이터를 만드는 것을 권장한다.

## 7.1 Motor Temperature

후보:

- NTC Thermistor
- Digital Temperature Sensor

```text
Motor Surface
    ↓
Temperature Sensor
    ↓
Drive ECU
    ↓ CAN
Cluster / IVI
```

---

## 7.2 Battery Temperature

후보:

- NTC Thermistor
- Digital Temperature Sensor

사용 목적:

- Cluster Battery Temperature
- DTC Overtemperature

---

## 7.3 Battery Voltage

배터리 전압이 STM32 ADC 최대 입력전압보다 높으면 직접 연결하지 않는다.

```text
Battery Voltage
      ↓
Voltage Divider / Measurement Circuit
      ↓
STM32 ADC
```

사용 목적:

- Battery 상태 표시
- 저전압 Warning
- 간단한 Battery % 추정 보조

정확한 SOC 계산은 전압 하나만으로 하기 어렵기 때문에 프로젝트에서는 단순 Gauge 또는 추정값으로 구분한다.

---

## 7.4 Battery Current Sensor — 선택 확장

후보 방식:

- Hall Current Sensor
- Shunt + Current Monitor IC

사용 목적:

```text
Current
→ Power 계산
→ 소비전력
→ Battery Monitoring
```

초기 필수 기능은 아니다.

---

# 8. Sensor → ECU Mapping

```text
                    [ Driver Input ]

Gear Buttons ───────────────┐
Accelerator Position ───────┤
Brake Position ─────────────┤→ VCU STM32
Steering Wheel Angle ───────┘


                    [ Drive ]

Motor Encoder / Hall ─────────→ Drive ECU STM32
Motor Temperature ─────────────→ Drive ECU STM32
Optional Steering Feedback ────→ Drive / Steering ECU


                    [ ADAS / Parking Vision ]

Front Camera ─ CSI ─────────────→ Raspberry Pi 4 HPC
Rear Camera ─ USB ──────────────→ Raspberry Pi 4 HPC


                    [ Parking ]

ToF / Ultrasonic Sensors ───────→ Parking ECU STM32


                    [ Body ]

Ambient Light Sensor ───────────→ Body ECU STM32


                    [ Battery ]

Battery Temperature ────────────→ VCU / Battery Measurement Node
Battery Voltage ────────────────→ VCU / Battery Measurement Node
Optional Current Sensor ────────→ VCU / Battery Measurement Node
```

---

# 9. Stage 1 우선순위

## 반드시 먼저 시험할 것

- [ ] Gear Button GPIO
- [ ] Accelerator ADC
- [ ] Brake ADC
- [ ] Steering Wheel Angle Sensor
- [ ] Motor Encoder / Hall Sensor
- [ ] Front Camera Capture
- [ ] Rear Camera Capture
- [ ] Parking Distance Sensor 최소 2개
- [ ] Ambient Light Sensor

## 이후 추가

- [ ] Motor Temperature Sensor
- [ ] Battery Temperature Sensor
- [ ] Battery Voltage Measurement
- [ ] Steering Actual Angle Feedback
- [ ] Battery Current Sensor
- [ ] Accelerator Dual Channel Plausibility

---

# 10. 아직 확정해야 할 부품

| 항목 | 현재 후보 | 확정 필요 사항 |
|---|---|---|
| Accelerator | 10k Linear Pot / Hall Sensor | 기구 장착 방식, ADC 범위 |
| Brake | 10k Linear Pot | 기구 장착 방식 |
| Steering Wheel | AS5600 | 핸들 축 / 자석 장착 |
| Motor RPM | Encoder / Hall Sensor | 모터축 장착 가능 여부, PPR |
| Parking Distance | ToF / Ultrasonic | 센서 모델, 개수, 배치 |
| Ambient Light | CDS / Digital ALS | ADC 방식 또는 I2C 방식 |
| Motor Temp | NTC / Digital Sensor | 장착 위치 |
| Battery Temp | NTC / Digital Sensor | 배터리 표면 장착 방식 |
| Battery Voltage | Divider + ADC | 배터리 최대전압 기준 저항값 |
| Battery Current | Hall / Shunt Monitor | 필요성 및 정격 |

---

# 11. 주의

센서를 구매하거나 연결하기 전에 반드시 다음을 확인한다.

```text
Operating Voltage
Logic Level
Maximum Input Voltage
Interface
Pinout
Current Consumption
Required Pull-up / Pull-down
Measurement Range
Update Rate
```

특히 STM32 GPIO / ADC에 5V 센서 출력을 바로 연결하기 전에 해당 핀의 5V tolerant 여부와 ADC 최대 입력전압을 반드시 확인한다.
