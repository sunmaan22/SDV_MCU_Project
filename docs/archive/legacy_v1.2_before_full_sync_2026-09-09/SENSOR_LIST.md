# Sensor List

> 현재 `SDV_MCU_Project`에서 상정한 센서·운전자 입력 장치를 정리한다.  
> 부품이 완전히 확정된 것은 아니므로 **확정 방향 / 후보 / 선택 확장**을 구분한다.

---

# 1. 전체 Sensor / Input Mapping

| 영역 | 센서 / 입력 | 목적 | Interface | Owner Node | 상태 |
|---|---|---|---|---|---|
| Driver | P/R/N/D Buttons | Gear 선택 | GPIO | STM32 #1 VCU | 확정 방향 |
| Driver | Accelerator Position | 0~100% 운전자 가속 입력 | ADC | STM32 #1 VCU | 후보 |
| Driver | Brake Position | 0~100% 제동 요청 | ADC | STM32 #1 VCU | 후보 |
| Driver | Steering Wheel Angle | 운전자 조향 입력 | I2C / Analog | STM32 #1 VCU | AS5600 후보 |
| Driver | Emergency Stop Button | 즉시 정지 입력 | GPIO | STM32 #1 VCU | 권장 |
| Drive | Motor Encoder / Hall | DC Motor RPM / Speed | Timer / GPIO | STM32 #2 Drive ECU | 권장 |
| Drive | Motor Temperature | Motor thermal monitoring | ADC / I2C | STM32 #2 Drive ECU | 선택 확장 |
| Steering | Actual Steering Feedback | 실제 바퀴 조향각 | I2C / ADC | STM32 #2 Drive/Steering | 선택 확장 |
| ADAS | Front Raspberry Pi Camera | Lane / Object / Risk | CSI | Raspberry Pi 4 HPC | 확정 방향 |
| Parking | Rear USB Camera | 후방 Parking Vision | USB | Raspberry Pi 4 HPC | 확정 방향 |
| Parking | ToF / Ultrasonic | 장애물 거리 | I2C / GPIO | STM32 #3 Parking ECU | 확정 방향 |
| Body | Ambient Light Sensor | Auto Light 판단 | ADC / I2C | STM32 #5 LIN Body Slave | 후보 |
| Battery | Battery Voltage | 전압 / 저전압 Warning / Gauge | ADC via divider | STM32 #1 VCU | 후보 |
| Battery | Battery Temperature | 배터리 온도 | ADC / I2C | STM32 #1 VCU | 후보 |
| Battery | Battery Current | 소비전류 / Power 추정 | ADC / I2C | STM32 #1 VCU | 선택 확장 |

> **STM32H735 Cockpit은 센서 Owner가 아니다.** Speed, RPM, Gear, Battery, Temperature, Parking, ADAS, Lamp, DTC 값을 CAN으로 받아 화면에 표시한다.

---

# 2. Driver Input — STM32 #1 VCU

## 2.1 Gear

초기 구현은 버튼 또는 스위치를 사용한다.

```text
[P] [R] [N] [D]
       ↓ GPIO
      VCU
```

Stage 1:

- Debounce
- 동시에 여러 기어 입력 시 Invalid 처리
- UART로 현재 Gear 출력

---

## 2.2 Accelerator

1차 후보:

- 10 kΩ Linear Potentiometer
- Analog Hall Position Sensor

```text
Pedal
 ↓
Pot / Hall
 ↓ ADC
VCU
 ↓
Accelerator = 0~100 %
```

```text
Pedal[%] = (ADC - ADC_MIN) / (ADC_MAX - ADC_MIN) × 100
```

확장:

- 2-channel Accelerator Plausibility

---

## 2.3 Brake

초기에는 10 kΩ Linear Potentiometer를 추천한다.

```text
Brake Pedal
 ↓
Pot
 ↓ ADC
VCU
 ↓
Brake = 0~100 %
```

현재 RC 플랫폼에서는 유압 브레이크가 아니라 VCU가 Drive Request를 낮추거나 0으로 만드는 용도로 사용한다.

---

## 2.4 Steering Wheel Angle

추천 후보:

- AS5600 + Magnet

```text
Steering Wheel
 ↓
Magnet
 ↓
AS5600
 ↓ I2C
VCU
```

Stage 1:

- Raw angle
- Center calibration
- Left/Right 최대범위
- 상대 Steering Input 변환

---

## 2.5 E-Stop

Push Button을 GPIO input으로 사용한다.

```text
E-Stop
 ↓
VCU
 ↓
Safety State
```

Stage 1에서는 실제 Motor를 제어하지 않아도 VCU가 `ESTOP=1`과 `FAULT/STOP` 상태를 만들 수 있으면 된다.

---

# 3. Drive / Steering — STM32 #2

## 3.1 Motor Encoder / Hall

브러시드 DC Motor + TB6612FNG 후보 구동부의 실제 속도를 측정한다.

```text
Brushed DC Motor
 ↓
Encoder / Hall
 ↓
STM32 Timer
 ↓
RPM / Vehicle Speed
```

사용 목적:

- Motor RPM
- Speed PID Feedback
- H735 Cluster Speed/RPM 표시

---

## 3.2 Steering Feedback — Optional

조향은 RC Servo를 사용할 예정이므로 초기에는 별도 실제 조향각 센서를 생략한다.

```text
STM32 PWM → RC Servo
```

확장 시 AS5600 또는 Potentiometer를 linkage/wheel angle에 추가한다.

---

## 3.3 Motor Temperature — Optional

후보:

- NTC Thermistor
- Digital Temperature Sensor

```text
Motor Surface → Sensor → Drive ECU → CAN → H735/HPC
```

---

# 4. ADAS — Raspberry Pi 4 HPC

## Front Camera

```text
Raspberry Pi Camera
 ↓ CSI
Pi 4
 ↓
OpenCV / AI
```

추출 정보 후보:

- Lane Offset
- Lane Angle
- Object Class
- Bounding Box
- Collision Level
- Speed Request
- Steering Request

Raw frame은 CAN으로 전송하지 않는다.

---

# 5. Parking

## 5.1 Rear Camera — Pi

```text
Gear R
 ↓
Rear USB Camera Active
 ↓
Raspberry Pi Parking Vision
```

D/주행 상태에서는 Front ADAS를 사용하고, R에서는 Front ADAS를 pause한 뒤 Rear Camera를 활성화한다.

## 5.2 Distance Sensors — STM32 #3

기본 권장:

- Rear Left
- Rear Right

확장:

- Front Left
- Front Right
- Rear Left
- Rear Right

```text
ToF / Ultrasonic
 ↓
Parking ECU
 ↓
Distance + Valid + Warning Level
```

Stage 1에서는 센서 하나부터 시작하고 다채널로 확장한다.

---

# 6. Body / LIN — STM32 #5 Body LIN Slave

## Ambient Light

Ambient Sensor는 이제 Body Gateway가 아니라 **LIN Slave가 직접 읽는다.**

후보:

- CDS + ADC
- Digital Ambient Light Sensor

```text
Ambient Sensor
 ↓
STM32 #5 LIN Slave
 ↓ LIN Ambient_Status
STM32 #4 Gateway
 ↓ CAN Body_Status
VCU / H735 / HPC
```

Lighting output도 같은 LIN Slave가 담당한다.

```text
Head Lamp
Tail Lamp
Brake Lamp
Turn Signal
Hazard
```

이 장치들은 센서는 아니지만 `SENSOR_LIST`의 Body Node mapping을 이해하기 위해 함께 기록한다.

---

# 7. Battery Monitoring — STM32 #1 VCU

Dedicated BMS를 별도로 두지 않으므로 프로젝트 수준의 Battery Monitoring은 VCU에서 처리하는 방향으로 잡는다.

## Battery Voltage

배터리를 ADC에 직접 연결하지 않는다.

```text
Battery
 ↓
Voltage Divider / Measurement Circuit
 ↓
VCU ADC
```

사용 목적:

- H735 Battery Voltage 표시
- Low Voltage Warning
- 단순 Gauge 보조

정확한 SOC는 전압만으로 계산하기 어렵기 때문에 프로젝트에서는 **Estimated SOC / Gauge**로 표현한다.

## Battery Temperature

후보:

- NTC
- Digital Temp Sensor

## Current — Optional

- Hall Current Sensor
- Shunt + Monitor IC

---

# 8. 전체 Owner Map

```text
STM32 #1 VCU
├ Gear Buttons
├ Accelerator
├ Brake
├ Steering Wheel Angle
├ E-Stop
├ Battery Voltage
└ Battery Temperature

STM32 #2 Drive + Steering
├ Motor Encoder / Hall
├ Optional Motor Temperature
└ Optional Steering Feedback

STM32 #3 Parking
└ ToF / Ultrasonic

Raspberry Pi 4
├ Front Camera
└ Rear USB Camera

STM32 #5 Body LIN Slave
└ Ambient Light Sensor

STM32 #4 Body Gateway
└ 센서를 직접 소유하기보다 CAN↔LIN Gateway 역할

STM32H735 Cockpit
└ 센서를 직접 소유하지 않고 CAN 데이터를 표시
```

---

# 9. Stage 1 우선순위

## Must Test

- [ ] Gear GPIO
- [ ] Accelerator ADC
- [ ] Brake ADC
- [ ] Steering Wheel Angle
- [ ] E-Stop
- [ ] Motor PWM + Encoder/Hall
- [ ] RC Servo Calibration
- [ ] Front Camera
- [ ] Rear Camera
- [ ] Parking Distance Sensor 최소 2개 목표
- [ ] Ambient Light Sensor on LIN Slave
- [ ] Lighting GPIO/PWM

## Next

- [ ] Battery Voltage
- [ ] Battery Temperature
- [ ] Motor Temperature
- [ ] Steering Actual Feedback
- [ ] Battery Current
- [ ] Dual Accelerator Plausibility

---

# 10. 아직 확정해야 할 부품

| 항목 | 후보 | 확인할 것 |
|---|---|---|
| Accelerator | 10k Pot / Hall | 기구부, ADC 범위 |
| Brake | 10k Pot | 기구부 |
| Steering Input | AS5600 | 축/자석 장착 |
| Motor RPM | Encoder / Hall | 장착, PPR |
| Parking | ToF / Ultrasonic | 모델/개수/배치 |
| Ambient | CDS / Digital ALS | LIN Slave 연결 방식 |
| Battery Temp | NTC / Digital | 장착 위치 |
| Battery Voltage | Divider + ADC | 최대전압/저항값 |
| CAN FD Interface | MCU 내장 FDCAN + Transceiver / 외장 | 실제 보드 지원 |
| LIN Transceiver | TJA102x / MCP200x 등 후보 | 전원/모듈/가용성 |

---

# 11. 연결 전 체크

```text
Operating Voltage
Logic Level
Maximum Rating
Interface
Pinout
Pull-up / Pull-down
Current Consumption
Measurement Range
Update Rate
```

특히 ADC 핀, 5V sensor output, Battery voltage, Motor power는 연결 전에 반드시 실제 보드 허용범위를 확인한다.
