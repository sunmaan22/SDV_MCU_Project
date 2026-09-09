# 팀 역할 쉬운 설명 — 우리 프로젝트에서 나는 무엇을 하는가?

> 대상: 자동차 전장, STM32, CAN, LIN을 처음 접하는 팀원  
> 목표: 어려운 용어를 외우기 전에 **내가 무엇을 입력받고, 무엇을 만들고, 누구에게 넘기는지** 이해한다.

---

# 0. 프로젝트를 자동차 한 대라고 생각하기

우리 프로젝트는 크게 다음 순서로 움직인다.

```text
주변을 본다
   ↓
무슨 상황인지 알아낸다
   ↓
어떻게 움직일지 정한다
   ↓
모터와 조향을 움직인다
   ↓
운전자에게 상태를 보여준다
   ↓
문제가 생기면 기록한다
```

이걸 조금 더 기술적인 말로 바꾸면 다음과 같다.

```text
인지 → 판단 → 제어
       +
UI / 통신 / 진단
```

처음에는 이 여섯 단어만 이해하면 된다.

---

# 1. 인지란 무엇인가?

**센서가 읽은 값에서 현재 상황을 알아내는 일**이다.

예를 들어 초음파센서가 있다고 하자.

```text
센서가 Echo 시간을 읽음
        ↓
STM32가 거리 계산
        ↓
Rear Right = 182 mm
        ↓
너무 가까움 = CRITICAL
```

여기까지가 인지다.

카메라도 마찬가지다.

```text
Camera Image
     ↓
사람이 보임
     ↓
오른쪽에 있음
     ↓
위험 가능성 있음
```

카메라는 Raspberry Pi가 처리하고, 초음파는 STM32가 처리한다.

---

# 2. 판단이란 무엇인가?

판단은 **인지 결과를 보고 차량이 무엇을 해야 할지 정하는 것**이다.

예:

```text
앞에 장애물이 있음
       ↓
속도를 줄여야 함
```

또는

```text
차선 중앙보다 차량이 왼쪽에 있음
       ↓
오른쪽으로 조금 조향해야 함
```

하지만 여기에도 두 단계가 있다.

### Raspberry Pi의 판단

```text
Vision 결과
→ ADAS Speed Request
→ ADAS Steering Request
```

Pi는 "이렇게 움직이는 게 좋겠다"는 **요청(Request)** 을 만든다.

### VCU의 최종 판단

VCU는 Pi의 요청을 바로 믿지 않는다.

```text
ADAS Request
Driver Input
Ultrasonic Critical
Gear
Fault State
       ↓
      VCU
       ↓
Final Speed / Steering Request
```

즉 Pi가 의견을 내고, VCU가 최종 허가를 내리는 구조다.

---

# 3. 제어란 무엇인가?

제어는 **최종 요청을 실제 Motor / Servo 움직임으로 만드는 것**이다.

예:

```text
Final Speed Request = 0.4 m/s
       ↓
Drive ECU
       ↓
PWM 계산
       ↓
TB6612FNG 후보
       ↓
Brushed DC Motor
```

조향도 같다.

```text
Final Steering Request = +10°
       ↓
Steering Control
       ↓
Servo PWM
       ↓
RC Servo
```

즉 제어 담당은 "차량이 어떻게 움직여야 하는가"보다 **실제로 그 움직임을 만들어내는 방법**에 집중한다.

---

# 4. UI는 무엇인가?

UI는 운전자에게 정보를 보여주는 부분이다.

우리 프로젝트에서는 STM32H735 + TouchGFX 한 보드가 Cluster와 IVI를 모두 담당한다.

```text
Cluster Main
├ Speed
├ RPM
├ Gear
├ Battery
└ Warning

IVI Menu
├ ADAS
├ Parking
├ DTC
└ Settings
```

중요한 점:

> H735는 속도나 거리를 직접 측정하지 않는다.

다른 ECU가 만든 값을 CAN으로 받아 화면에 보여준다.

```text
Drive ECU → RPM
Parking ECU → Distance
VCU → Gear
HPC → ADAS Status
       ↓
      CAN
       ↓
     H735
       ↓
    TouchGFX
```

---

# 5. 통신과 Gateway는 무엇인가?

우리 프로젝트에는 두 종류의 통신을 사용한다.

```text
CAN FD
LIN
```

CAN FD는 차량 전체 보드들이 정보를 공유하는 큰 통신망이다.

```text
VCU
Drive ECU
Ultrasonic ECU
HPC
H735
Body Gateway
       ↕
     CAN FD
```

LIN은 Body 영역에서 단순한 센서/조명 장치를 연결하는 작은 통신망이다.

```text
Body Gateway
    ↓ LIN
Body Slave
├ Ambient Sensor
└ Lighting
```

Gateway는 두 통신망 사이에서 데이터를 바꿔주는 역할이다.

```text
CAN 명령
  ↓
Body Gateway
  ↓ 변환
LIN 명령
```

반대도 가능하다.

```text
Ambient Sensor
  ↓ LIN
Gateway
  ↓ CAN
VCU / H735 / HPC
```

---

# 6. DTC는 무엇인가?

DTC는 쉽게 말하면 **차량 고장 기록**이다.

예:

```text
Ultrasonic Sensor 응답 없음
       ↓
PARK_SENSOR_TIMEOUT
```

```text
Motor Encoder Pulse 없음
       ↓
DRIVE_ENCODER_FAULT
```

```text
LIN Slave 응답 없음
       ↓
BODY_LIN_TIMEOUT
```

각 ECU가 자기 문제를 먼저 알아낸다.

```text
각 ECU
 ↓
Local Fault Detection
 ↓
DTC Event
 ↓ CAN FD
Raspberry Pi DTC Manager
 ↓
저장 / History
 ↓
H735 Diagnostic 화면
```

DTC 담당자는 모든 고장을 혼자 만드는 사람이 아니다.

**각자 자기 ECU의 고장 검출을 만들고, DTC 담당자가 코드 규칙과 중앙 처리를 통합한다.**

---

# 7. 우리 팀 6명의 역할

## A — Ultrasonic / 인지 담당

### 한 줄 설명

> **"차량 주변에 장애물이 얼마나 가까운지 알아내는 사람"**

### 사용하는 것

- STM32 #1
- Ultrasonic Sensor 여러 개

### 하는 일

```text
Ultrasonic
   ↓
거리 측정
   ↓
필터링
   ↓
유효한 값인지 확인
   ↓
SAFE / WARNING / CRITICAL
   ↓
CAN으로 전달
```

### 처음 해야 할 일

1. 초음파센서 1개 연결
2. 10cm / 30cm / 50cm에서 거리 확인
3. 센서가 끊겼을 때 오류 확인
4. 여러 센서로 확장
5. CAN으로 `Distance`와 `Warning` 보낼 준비

### 최종 출력 예

```text
REAR_LEFT_MM  = 320
REAR_RIGHT_MM = 180
WARNING       = CRITICAL
VALID         = 1
```

### 하지 않는 일

- Motor PWM 만들기
- Camera AI
- 최종 차량 정지 결정

CRITICAL이라고 알려주면 VCU가 최종 판단한다.

---

# 8. B — Cluster + IVI / UI 담당

### 한 줄 설명

> **"차량이 지금 어떤 상태인지 운전자에게 보여주는 사람"**

### 사용하는 것

- STM32H735
- TouchGFX
- LCD / Touch

### 하는 일

```text
CAN Data
  ↓
Vehicle Data Model
  ↓
TouchGFX
  ↓
Cluster / IVI
```

### Cluster에서 보여줄 것

- Speed
- RPM
- Gear
- Battery
- Temperature
- Turn Signal
- Warning

### IVI에서 보여줄 것

- ADAS 상태
- Parking 상태
- DTC
- Lighting 설정

### 처음 해야 할 일

1. TouchGFX 화면 띄우기
2. 숫자 값 바꾸기
3. 버튼으로 화면 전환하기
4. Dummy Data로 Speed/RPM/Gear 표시하기
5. 나중에 Dummy Data를 CAN 데이터로 바꾸기

### 하지 않는 일

- Motor를 직접 움직이기
- Ultrasonic을 직접 읽기
- Camera 영상처리

---

# 9. C — Motor + Steering / 제어 담당

### 한 줄 설명

> **"최종 명령을 받아서 실제 자동차를 움직이는 사람"**

### 사용하는 것

- STM32 #2
- Brushed DC Motor
- TB6612FNG 후보
- Encoder / Hall Sensor
- RC Servo

### Motor 흐름

```text
Target Speed
    ↓
Drive ECU
    ↓
PWM
    ↓
Motor Driver
    ↓
DC Motor
    ↓
Encoder
    ↓
RPM Feedback
```

### Steering 흐름

```text
Target Steering
    ↓
STM32
    ↓
Servo PWM
    ↓
RC Servo
```

### 처음 해야 할 일

1. Motor 없이 PWM 확인
2. Driver + Motor 저출력 시험
3. Encoder Pulse 확인
4. RPM 계산
5. Servo Center/Left/Right 확인
6. 이후 PID/속도제어

### 하지 않는 일

- Camera 판단
- Parking 거리 판단
- ADAS가 위험한지 최종 결정

---

# 10. D — Lighting + Ambient / LIN-CAN 담당

### 한 줄 설명

> **"차량 조명 영역을 만들고 LIN과 CAN을 연결하는 사람"**

### 사용하는 것

- STM32 #3 = Body Gateway / LIN Master
- STM32 #4 = Body LIN Slave
- CAN FD Transceiver
- LIN Transceiver
- Ambient Light Sensor
- Head / Tail / Brake / Turn / Hazard LED

### 핵심 구조

```text
CAN FD
  ↓
STM32 #3 Gateway
  ↓ LIN
STM32 #4 Slave
  ├ Ambient Sensor
  └ Lighting
```

### 왜 MCU가 두 개인가?

LIN 통신을 실제로 보여주려면 최소한 Master와 Slave가 필요하다.

```text
Master ↔ Slave
```

STM32 #3가 Gateway + LIN Master가 되고, STM32 #4가 LIN Slave가 된다.

### 처음 해야 할 일

1. Slave에서 조도센서 읽기
2. Slave에서 LED 켜기/끄기
3. Master ↔ Slave LIN 한 프레임 송수신
4. Ambient 값 LIN으로 읽기
5. Lamp Command LIN으로 보내기
6. 그 다음 CAN↔LIN 변환

### Gateway 예

```text
CAN:
HEADLAMP_REQUEST = ON
       ↓
Gateway
       ↓
LIN:
LAMP_COMMAND = ON
```

---

# 11. E — HPC + Camera Vision / 인지·판단 담당

### 한 줄 설명

> **"카메라 영상을 보고 도로 상황을 이해해서 차량에 행동을 요청하는 사람"**

### 개발 단계 Hardware

```text
Pi #1 + Front Camera
→ Front ADAS 개발

Pi #2 + Rear Camera
→ Rear Parking Vision 개발
```

최종 차량에서는 Pi 하나로 통합한다.

### Front Vision

```text
Front Camera
  ↓
Lane Detection
Object Detection
  ↓
ADAS Result
  ↓
Speed / Steering Request
```

### Rear Vision

```text
Rear Camera
  ↓
Rear Object Detection
  ↓
Parking Vision Status
```

### 처음 해야 할 일

1. Camera Frame 읽기
2. FPS 측정
3. OpenCV 처리
4. Front / Rear 코드를 분리해서 GitHub에 관리
5. Vision Result 구조 정의
6. 나중에 CAN으로 Result만 송신

### 중요한 원칙

```text
Camera Raw Image
→ Raspberry Pi 안에서만 처리
```

CAN에는 이런 값만 보낸다.

```text
OBJECT = PERSON
LANE_OFFSET = -30 mm
WARNING = 1
STEERING_REQUEST = +3 deg
```

STM32에서 영상처리를 억지로 하지 않는다.

---

# 12. F — VCU + DTC + CAN Integration / 최종 판단·진단 담당

### 한 줄 설명

> **"여러 사람이 만든 정보를 모아 차량이 최종적으로 무엇을 할지 정하고, 문제가 없는지 확인하는 사람"**

### 사용하는 것

- STM32 #5
- Gear Buttons
- Accelerator Sensor
- Brake Sensor
- Steering Wheel Sensor
- E-Stop
- CAN FD
- Raspberry Pi DTC Manager와 협업

### VCU 역할

```text
Driver Input ───────┐
ADAS Request ───────┤
Ultrasonic Warning ─┤
Fault State ────────┤
Gear ───────────────┘
          ↓
         VCU
          ↓
Final Speed / Steering
```

### 예

```text
Accelerator = 60%
ADAS Speed Request = 40%
Ultrasonic = CRITICAL

→ Final Speed Request = 0
```

### DTC 역할

F는 DTC 코드와 통합 규칙을 관리한다.

예:

```text
A: US_001 Sensor Timeout
C: DRIVE_001 Encoder Fault
D: BODY_001 LIN Timeout
E: HPC_001 Camera Fault
```

각 담당자가 Local Fault를 만들고, F가 코드 체계와 DTC 흐름을 정한다.

### 처음 해야 할 일

1. Gear P/R/N/D 버튼 읽기
2. Accelerator/Brake ADC 읽기
3. Steering Input 읽기
4. 간단한 State Machine
5. E-Stop 시 Speed = 0
6. CAN Matrix 초안
7. DTC Code 규칙 초안

---

# 13. 여섯 역할이 같이 동작하는 예

## 예제 1 — 주행 중 장애물

```text
E: Front Camera에서 장애물 인식
        ↓
E: 감속 Request 생성
        ↓
F: VCU가 요청과 차량 상태 확인
        ↓
F: Final Speed Request 결정
        ↓
C: Motor PWM 감소
        ↓
B: H735에 ADAS Warning 표시
```

## 예제 2 — 후진 주차

```text
F: Gear = R 확인
        ↓
E: Rear Camera Vision 활성화
        ↓
A: Ultrasonic 거리 측정
        ↓
E/F: Vision + Distance 정보 사용
        ↓
B: Parking 화면 표시
        ↓
A: CRITICAL 거리 발생
        ↓
F: Final Speed = 0
        ↓
C: Motor Stop
```

## 예제 3 — 야간 조명

```text
D: Ambient Sensor 값 읽음
        ↓ LIN
D: Gateway가 CAN 상태로 변환
        ↓
F 또는 사용자: Lighting Request
        ↓ CAN
D: Gateway
        ↓ LIN
D: Lighting Slave
        ↓
Head Lamp ON
        ↓
B: H735에 Headlamp ON 표시
```

## 예제 4 — 센서 고장

```text
A: Ultrasonic 응답 없음
        ↓
A: Local Fault 생성
        ↓ CAN
F: DTC 규칙에 따라 처리
        ↓
Pi DTC Manager 저장
        ↓
B: H735 Diagnostic 화면에 표시
```

---

# 14. 내가 맡은 기능을 설명할 때 이 5문장만 답하면 된다

각 팀원은 아래 질문에 답할 수 있어야 한다.

```text
1. 나는 무엇을 입력받는가?
2. 그 입력으로 무엇을 계산/판단하는가?
3. 내가 만드는 최종 데이터는 무엇인가?
4. 그 데이터를 누구에게 보내는가?
5. 내 기능이 고장나면 어떻게 알 수 있는가?
```

예: Ultrasonic 담당

```text
1. Echo 신호를 입력받는다.
2. 거리와 Warning Level을 계산한다.
3. Distance_mm / Valid / Warning을 만든다.
4. CAN으로 VCU/H735/HPC에 보낸다.
5. Timeout이면 Sensor Fault를 만든다.
```

이 정도를 말할 수 있으면 자기 Architecture의 뼈대를 이해한 것이다.

---

# 15. Stage 1에서는 어디까지 하면 되는가?

Stage 1에서는 다른 팀원을 기다리지 않는다.

```text
내 보드
  ↓
내 센서 / 모터 / 화면
  ↓
단독 동작
  ↓
UART / 화면 / 측정값 확인
```

각 역할의 Stage 1 목표:

| 담당 | Stage 1 PASS 예 |
|---|---|
| A | Ultrasonic 1개 이상 거리 측정 + Timeout 검출 |
| B | H735 Cluster/IVI 화면 + Dummy Data 표시 |
| C | Motor PWM/RPM + Servo Left/Center/Right |
| D | Ambient/LED 단독 + LIN Master↔Slave 통신 |
| E | Front/Rear Camera Frame + 기본 Vision/FPS |
| F | Gear/Accel/Brake/Steering 입력 + VCU 기본 State |

그 다음에 CAN/LIN 통합으로 넘어간다.

---

# 16. 가장 중요한 한 문장

> **A와 E가 상황을 보고, E와 F가 행동을 판단하고, C와 D가 실제 하드웨어를 움직이며, B가 사람에게 보여주고, F가 전체 통신과 고장 흐름을 정리한다.**

이 문장을 이해하면 프로젝트 전체 구조를 거의 이해한 것이다.

---

## 다음에 읽을 문서

1. [전자공학 선행학습](ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md)
2. [Architecture & Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
3. [Sensor List](SENSOR_LIST.md)
4. [Node별 명세/Architecture 예시](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
5. [작성 Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
