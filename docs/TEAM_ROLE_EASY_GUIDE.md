# 팀 역할 쉬운 설명 — 나는 무엇을 하는가?

> 자동차 전장과 MCU가 처음인 팀원을 위한 문서다.  
> 어려운 ECU 이름보다 먼저 **내가 무엇을 받고 → 무엇을 만들고 → 누구에게 넘기는지** 이해한다.

---

# 1. 자동차를 여섯 가지 일로 나누기

```text
① 본다 / 잰다        = 인지
② 상황을 해석한다    = 판단
③ 실제로 움직인다    = 제어
④ 운전자에게 보여준다 = UI
⑤ 서로 데이터를 보낸다 = 통신
⑥ 문제가 생겼는지 본다 = 진단
```

우리 프로젝트도 똑같다.

```text
Camera / Ultrasonic / Driver Input
                ↓
             인지
                ↓
        Vision / VCU 판단
                ↓
       Motor / Steering 제어
                ↓
            실제 차량

모든 과정의 상태 → H735 화면
모든 Node의 고장 → DTC
모든 Node 사이 데이터 → CAN FD / LIN
```

---

# 2. A — Ultrasonic / 인지

### 한 문장

> **차가 장애물과 얼마나 가까운지 숫자로 만드는 사람.**

```text
Ultrasonic Trigger
      ↓
Echo 시간 측정
      ↓
Distance 계산
      ↓
유효성 검사 / Filtering
      ↓
SAFE / WARNING / CRITICAL
      ↓ CAN
VCU / H735 / HPC
```

A가 해야 하는 일:

- 초음파센서 연결
- Trigger/Echo 동작 확인
- 거리 계산
- 여러 센서 사용 시 측정 순서 관리
- 이상값/Timeout 처리
- 거리와 Warning Level CAN 전송 준비

A가 하지 않는 일:

- 카메라 AI
- 모터 PWM
- 최종 정지 여부 결정

**내 출력 예시**

```text
rear_left_mm  = 420
rear_right_mm = 170
valid         = true
warning       = CRITICAL
```

---

# 3. B — Cluster + IVI / UI

### 한 문장

> **다른 ECU가 만든 값을 운전자가 이해할 수 있는 화면으로 보여주는 사람.**

STM32H735 + TouchGFX 한 보드에서 Cluster와 IVI를 같이 만든다.

```text
CAN Vehicle Data
      ↓
H735 Data Model
      ↓
Cluster Main Screen
├ Speed / RPM / Gear
├ Battery / Temperature
├ Warning
└ Lamp / ADAS 상태

Menu
├ ADAS
├ Parking
├ Diagnostics
└ Settings
```

B가 해야 하는 일:

- TouchGFX 화면 설계
- Dummy Data로 화면부터 완성
- CAN 값과 UI 변수 연결
- Warning 우선순위
- DTC 상세 화면

B가 하지 않는 일:

- Speed를 직접 센서로 측정
- DTC 원인을 직접 판정
- Motor를 직접 제어

---

# 4. C — Motor + Steering / 제어

### 한 문장

> **VCU가 내려준 최종 명령을 실제 모터와 조향 움직임으로 바꾸는 사람.**

```text
VCU Final Command
       ↓ CAN
Drive + Steering STM32
       ├→ Motor PWM / Direction → TB6612FNG 후보 → Brushed DC Motor
       └→ Servo PWM → RC Steering Servo

Motor Encoder/Hall
       ↓
RPM Feedback
```

C가 해야 하는 일:

- TB6612FNG 후보와 모터 구동 확인
- PWM / Direction
- Encoder/Hall로 RPM 계산
- 필요 시 Speed PID
- Servo center / left / right calibration
- Command timeout 시 안전 정지

C가 하지 않는 일:

- 카메라 판단
- 운전자 요청과 ADAS 요청 중 누가 우선인지 결정

---

# 5. D — Lighting + Ambient / LIN-CAN

### 한 문장

> **차체의 느린 장치들을 LIN으로 묶고, 그 LIN 데이터를 CAN 차량망과 이어주는 사람.**

D는 소형 STM32 두 개를 사용한다.

```text
CAN FD Backbone
      ↕
STM32 #3 Body Gateway
CAN FD + LIN Master
      ↕ LIN
STM32 #4 Body LIN Slave
      ├ Ambient Light Sensor
      └ Head / Tail / Brake / Turn / Hazard
```

Gateway는 별도 세 번째 MCU가 필요한 것이 아니다. **STM32 #3 자체가 CAN과 LIN 양쪽을 가진 Gateway**가 된다.

D가 해야 하는 일:

- LIN Master/Slave 통신
- LIN Transceiver 연결
- CAN FD ↔ LIN Signal Mapping
- Ambient 값 수신
- Lamp Command 전달
- LIN Node Timeout / Lamp Fault 처리

예:

```text
LIN Ambient_Status = 25
       ↓ Gateway
CAN Body_Status.Ambient = 25
```

반대 방향:

```text
CAN Body_Command.HeadLamp = ON
       ↓ Gateway
LIN Lamp_Command = ON
       ↓
LIN Slave → LED ON
```

---

# 6. E — HPC + Camera Vision / 인지·판단

### 한 문장

> **카메라 영상을 보고 차선·물체·주차 상황을 알아낸 뒤, 차량에 필요한 요청을 만드는 사람.**

영상처리는 STM32가 아니라 Raspberry Pi에서 한다.

개발 중:

```text
Pi #1 + Front Camera → Front ADAS Vision
Pi #2 + Rear Camera  → Rear Parking Vision
```

최종:

```text
Front CSI Camera ─┐
                  ├→ Raspberry Pi HPC
Rear USB Camera ──┘
                  ├ Front ADAS Service
                  └ Rear Parking Vision Service
```

기어에 따라 서비스 전환을 목표로 한다.

```text
D → Front ADAS active
R → Rear Parking Vision active
```

E가 만드는 값 예:

```text
lane_offset
object_detected
collision_level
adas_speed_request
adas_steering_request
rear_object_position
parking_vision_warning
```

E는 Motor PWM을 직접 보내지 않는다. **Request를 F의 VCU로 보낸다.**

---

# 7. F — VCU + DTC + CAN Integration / 최종 판단

### 한 문장

> **사람·카메라·초음파가 각자 말한 것을 모아서 차량이 최종적으로 무엇을 할지 결정하는 사람.**

F가 직접 읽는 Driver Input 후보:

```text
P/R/N/D Buttons
Accelerator Position
Brake Position
Steering Wheel Angle
E-Stop
```

그리고 CAN으로 받는다.

```text
ADAS Request
Ultrasonic Warning
Drive Status
Body Status
Heartbeat / Fault
```

VCU 흐름:

```text
Driver Request ─┐
ADAS Request ───┤
Ultrasonic ─────┤
Fault / E-Stop ─┤
                ↓
              VCU
     Validate / Mode / Safety
            Arbitration
                ↓
     Final Speed / Steering
                ↓ CAN
      Drive + Steering ECU
```

우선순위 예시:

```text
Critical Fault / E-Stop
        >
Ultrasonic Critical Stop
        >
ADAS Safety Request
        >
Normal Driver Request
```

### DTC는 이렇게 나눈다

- A: Ultrasonic Fault 검출
- B: H735 자체 Fault 검출
- C: Encoder/Command/Motor Fault 검출
- D: LIN/Lamp/Gateway Fault 검출
- E: Camera/Vision Service Fault 검출
- F: VCU/Input/CAN Timeout Fault 검출 + DTC 규격 통합
- Pi DTC Manager: Active/History/시간/횟수 저장
- B H735: Warning과 상세 DTC 표시

---

# 8. Parking Assist에서 Vision과 Ultrasonic 차이

```text
Ultrasonic
= "얼마나 가까운가?"

Vision
= "무엇이 있고 어디에 있는가?"
```

예:

```text
A Ultrasonic:
Rear Right = 150 mm
CRITICAL

E Vision:
Object Detected = true
Position = REAR_RIGHT

F VCU:
→ Final Speed Request = 0
```

서로 경쟁하는 기능이 아니라 서로 다른 정보를 준다.

---

# 9. 내 역할을 이해했는지 확인

자기 역할에 대해 아래 다섯 문장에 답할 수 있어야 한다.

1. 나는 무엇을 입력받는가?
2. 그 입력으로 무엇을 계산하거나 판단하는가?
3. 내가 만드는 최종 데이터/출력은 무엇인가?
4. 그 결과를 누구에게 넘기는가?
5. 내 기능이 고장났을 때 어떻게 검출하는가?

이 다섯 개가 답이 안 나오면 코드를 쓰기 전에 Architecture부터 다시 본다. 코드가 빨리 생기는 것과 시스템이 빨리 완성되는 것은 놀랍도록 자주 다른 일이다.
