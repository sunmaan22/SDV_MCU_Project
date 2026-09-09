# Team Guide

> 처음 보는 팀원이 이 문서 하나로 **내 역할, 필요한 전자기초, 개발 순서**를 이해하는 것을 목표로 한다.

# 1. 프로젝트를 아주 쉽게 보면

```text
센서/카메라가 본다        = 인지
        ↓
상황을 해석하고 결정한다   = 판단
        ↓
모터/서보/조명을 움직인다 = 제어
        ↓
화면에 보여준다            = UI
        ↓
노드끼리 데이터를 주고받는다 = CAN / LIN
        ↓
고장을 찾고 기록한다       = DTC
```

우리 프로젝트는 이 기능을 한 보드에 몰아넣지 않고 여러 Node로 나눠 구현한다.

---

# 2. 6명은 무엇을 하는가

## A — Ultrasonic / 인지

**한마디:** 장애물이 얼마나 가까운지 잰다.

```text
Ultrasonic Sensor
→ Trigger / Echo
→ Distance 계산
→ Filter / Validity
→ SAFE / WARNING / CRITICAL
→ CAN FD
```

해야 할 일:
- 센서 1개부터 거리 측정
- 여러 센서로 확장
- 잘못된 값/Timeout 검출
- 거리와 Warning Level을 CAN으로 전달

넘겨주는 값 예:
```text
rear_left_mm = 320
rear_right_mm = 180
warning = CRITICAL
valid = true
```

---

## B — Cluster + IVI / UI

**한마디:** 다른 Node가 만든 값을 운전자에게 보여준다.

```text
CAN FD
→ H735 Vehicle Data Model
→ TouchGFX
→ Cluster Main / ADAS / Parking / DTC / Settings
```

해야 할 일:
- 기본 Cluster: Speed, RPM, Gear, Battery, Warning
- IVI 메뉴: ADAS, Parking, Diagnostics, Settings
- Touch 입력과 화면 전환
- CAN이 아직 없어도 Dummy Data로 먼저 UI 검증

중요:
> H735가 센서 값을 새로 만드는 것이 아니다. 값의 Owner에게 CAN으로 받는다.

---

## C — Motor + Steering / 제어

**한마디:** VCU가 정한 명령을 실제 움직임으로 바꾼다.

```text
VCU Final Command
→ STM32
├ Motor PWM / Direction → TB6612FNG 후보 → Brushed DC Motor
└ Steering PWM → RC Servo

Motor Encoder/Hall
→ RPM Feedback
```

해야 할 일:
- Motor PWM / Direction
- Encoder/Hall 기반 RPM
- Servo Center / Left / Right Calibration
- Command Timeout 시 안전 정지
- 센서/제어가 안정된 뒤 Speed PID 확장

---

## D — Lighting + Ambient / LIN-CAN

**한마디:** 차체 조명 기능을 LIN으로 만들고 CAN FD 차량망과 연결한다.

```text
CAN FD
↕
STM32 Gateway
CAN ↔ LIN Mapping / LIN Master
↕ LIN
STM32 LIN Slave
├ Ambient Sensor
└ Head / Tail / Brake / Turn / Hazard
```

해야 할 일:
- LIN Slave에서 조도센서/LED 먼저 단독 동작
- Gateway와 LIN 통신
- CAN 명령을 LIN 명령으로 변환
- LIN 상태를 CAN 상태로 변환
- LIN Node Timeout / Fault 검출

Gateway는 별도 장치가 아니라 **D가 사용하는 Gateway STM32가 CAN과 LIN 양쪽을 가진다.** LIN 통신을 보여주려면 반대편 LIN Slave MCU는 필요하다.

---

## E — HPC + Camera Vision / 인지·판단

**한마디:** 카메라 영상을 보고 무엇이 보이는지 판단한다.

개발 단계:
```text
Pi #1 + Front Camera → Front ADAS Vision
Pi #2 + Rear Camera  → Rear Parking Vision
```

최종 단계:
```text
Front Camera ─┐
              ├→ Raspberry Pi HPC
Rear Camera ──┘
```

Front Vision 후보:
- Lane Detection
- Object Detection
- Collision Warning
- Speed / Steering Request

Rear Vision 후보:
- Rear Object Detection
- Object Position
- Parking Warning

중요:
> Camera Raw Frame은 CAN FD로 보내지 않는다. Pi에서 처리한 결과만 CAN으로 보낸다.

---

## F — VCU + DTC + CAN Integration / 최종 판단

**한마디:** 여러 요청 중 차량이 실제로 무엇을 할지 최종 결정하고 통신 규칙을 맞춘다.

```text
Driver Input ───────┐
ADAS Request ───────┤
Ultrasonic Status ──┤
ECU Fault ──────────┤
                    ↓
                   VCU
              Safety / Mode
              Arbitration
                    ↓
        Final Speed / Steering
                    ↓ CAN FD
              Drive + Steering
```

해야 할 일:
- P/R/N/D
- Accelerator / Brake / Steering Wheel 입력
- E-Stop / Safety State
- Driver / ADAS / Parking 요청 중재
- CAN Signal Matrix와 Heartbeat/Timeout 통합
- DTC Code 규칙과 중요 Fault 대응 통합

DTC는 F 혼자 만드는 것이 아니다.

```text
각 Node가 자기 Fault 검출
→ DTC Event
→ Pi DTC Manager가 저장
→ H735에서 표시
```

---

# 3. 최소 전자기초

전자전공 수준의 회로이론 전체가 필요한 것은 아니다. 아래를 모르면 배선과 디버깅이 힘들어진다.

## 3.1 전압 / GND

- MCU Logic Level: 실제 보드 확인
- 센서 Supply Voltage 확인
- MCU 입력 허용전압 확인
- 신호를 주고받는 장치의 GND 기준 확인

전원 확인 없이 센서를 연결하지 않는다.

## 3.2 GPIO

사용 예:
- Gear Button
- E-Stop
- Direction
- LED
- Ultrasonic Trigger/Echo

필수 개념:
- Input / Output
- Pull-up / Pull-down
- Active High / Low
- Debounce

## 3.3 ADC

```text
Sensor Voltage
→ ADC Raw
→ Calibration
→ Physical Value
```

예:
```text
Accelerator Pot
→ ADC 0~4095
→ Calibrated 0~100 %
```

Raw 값과 실제 단위를 분리한다.

## 3.4 PWM / Timer

사용 예:
- DC Motor PWM
- RC Servo
- Encoder / Hall 측정
- LED brightness

```text
STM32 PWM → Motor Driver → Motor
```

MCU GPIO에서 Motor를 직접 구동하지 않는다.

## 3.5 UART / I2C / SPI

- UART: Debug Log
- I2C: AS5600, 일부 Sensor
- SPI: 외장 CAN Controller 등 후보

## 3.6 CAN / CAN FD

```text
MCU CAN/FDCAN
→ CAN Transceiver
→ CANH / CANL
```

알아야 할 것:
- CAN ID
- DLC / Payload
- Periodic / Event
- Timeout
- Heartbeat
- Termination

## 3.7 LIN

```text
MCU UART/LIN
→ LIN Transceiver
→ LIN Bus
```

기본 구조:
```text
LIN Master
├ Slave A
└ Slave B
```

현재 프로젝트에서는 Body Gateway가 LIN Master 역할을 한다.

---

# 4. 개발은 이 순서로 한다

```text
1. 내 역할 이해
2. SPECIFICATION 작성
3. ARCHITECTURE 작성
4. Datasheet / Pin / Wiring 확인
5. Board Bring-up
6. 입력 또는 출력 하나만 단독 시험
7. Raw 값 확인
8. Physical 값 / State로 변환
9. Fault / Timeout 시험
10. TEST_REPORT 작성
11. 그 다음 CAN / LIN 통합
```

처음부터 전체 보드를 다 연결하지 않는다.

---

# 5. Stage 1 공통 PASS 기준

- [ ] Build / Flash / Debug 가능
- [ ] Pin/Wiring 실제 구성 기록
- [ ] Input Raw 값 또는 Dummy Data 확인
- [ ] 필요한 Physical 값/State 생성
- [ ] Output이 있으면 안전한 범위에서 단독 시험
- [ ] Invalid / Timeout / Disconnect 중 해당 항목 시험
- [ ] UART Log / Screenshot / 사진 / 영상 등 증거 확보
- [ ] 다음 단계 CAN/LIN에 필요한 Input/Output 정의

---

# 6. 팀원끼리 데이터를 넘길 때

`값 하나 보내면 되겠지`로 끝내지 않는다.

최소 다음을 합의한다.

| 항목 | 예 |
|---|---|
| 이름 | `rear_right_mm` |
| Owner | Ultrasonic ECU |
| Consumer | VCU / H735 / HPC |
| 단위 | mm |
| 범위 | TBD after test |
| 주기 | TBD |
| Invalid | `valid=false` |
| Timeout Action | VCU policy에 따라 Warning/Stop |

---

# 7. 막혔을 때 질문 양식

```text
Board:
Sensor / Actuator:
Power:
Interface:
Pin:
Expected:
Actual:
Log:
Already Tried:
```

`안 됩니다`만 적으면 다른 사람이 디버깅하기 어렵다.

---

# 8. 자기 역할을 이해했는지 확인

아래 다섯 질문에 자기 말로 답할 수 있으면 된다.

1. 나는 무엇을 입력받는가?
2. 그 입력으로 무엇을 계산/판단하는가?
3. 내가 만드는 출력은 무엇인가?
4. 그 출력을 누구에게 보내는가?
5. 내 기능이 고장났는지 어떻게 알 수 있는가?

상세 부품, 센서, 데이터 Owner는 [PROJECT_REFERENCE.md](PROJECT_REFERENCE.md)를 참고한다.
