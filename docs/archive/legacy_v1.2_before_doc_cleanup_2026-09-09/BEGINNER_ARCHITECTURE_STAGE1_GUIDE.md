# Beginner Guide — Specification, Architecture & Stage 1 Bring-up

> 대상: STM32 / Raspberry Pi / CAN / LIN 프로젝트가 처음인 팀원  
> 목표: **자기 역할을 설명하고, 자기 보드/센서/액추에이터를 단독으로 검증한 뒤 통합 단계로 넘어가는 것**

---

# 1. 개발 순서

```text
1. 내 역할 이해
2. Specification 작성
3. Architecture 작성
4. Pin / Wiring 작성
5. Board Bring-up
6. Sensor / Actuator 단독 시험
7. Fault / Invalid 시험
8. Stage 1 Report 작성
9. 그 다음 CAN / LIN 통합
```

Stage 1의 목표는 차량 전체를 움직이는 것이 아니다.

> **내 기능만 떼어 놓았을 때 입력 → 처리 → 출력이 정상인지 증명하는 것**이 Stage 1이다.

---

# 2. Specification과 Architecture 차이

## Specification

**무엇을 해야 하는지** 적는다.

예:

```text
REQ-US-001
Ultrasonic ECU는 거리값을 mm로 계산해야 한다.

REQ-DRV-001
Drive ECU는 VCU 명령을 Motor PWM으로 변환해야 한다.
```

## Architecture

**그 기능을 어떤 구성으로 만들지** 적는다.

예:

```text
Ultrasonic
→ Trigger/Echo Driver
→ Distance Calculation
→ Filter
→ Warning
→ CAN
```

---

# 3. 모든 담당자가 먼저 쓰는 5개 질문

```text
Input  : 나는 무엇을 받는가?
Process: 무엇을 계산/판단하는가?
Output : 무엇을 만드는가?
Target : 누구에게 보내는가?
Fault  : 무엇이 잘못됐는지 어떻게 아는가?
```

이 다섯 줄이 Architecture의 뼈대다.

---

# 4. 현재 역할 기준 Stage 1

## A — Ultrasonic / 인지

```text
Ultrasonic 1개
→ Trigger
→ Echo 측정
→ Distance mm
→ UART 출력
```

### 해야 할 것

- [ ] 센서 전압/핀 확인
- [ ] Trigger/Echo 동작
- [ ] 3개 기준거리 측정
- [ ] Timeout 처리
- [ ] 여러 센서 확장 순서 작성
- [ ] Warning Level 후보 정의

Stage 1 출력 예:

```text
SENSOR=RR RAW_US=986 DIST=169mm VALID=1 LEVEL=CRITICAL
```

---

## B — H735 Cluster + IVI / UI

센서를 억지로 연결하지 않는다. Stage 1은 Dummy Data로 UI부터 검증한다.

```text
Dummy Data
→ H735 Data Model
→ TouchGFX
→ Cluster / IVI
```

### 해야 할 것

- [ ] Build/Flash
- [ ] LCD
- [ ] Touch
- [ ] Cluster main
- [ ] ADAS/Parking/DTC 화면
- [ ] 숫자/Gauge/Warning 변경

예:

```text
speed=24
rpm=1250
gear=D
parking_warning=CRITICAL
dtc_count=2
```

---

## C — Motor + Steering / 제어

```text
STM32
├ PWM/DIR → TB6612FNG 후보 → Brushed DC Motor
├ Encoder/Hall ← Motor
└ Servo PWM → RC Servo
```

### 순서

1. Motor 없이 PWM 확인
2. Driver 전원/Enable 확인
3. 낮은 출력에서 Motor 회전
4. Direction 확인
5. Encoder count
6. RPM 계산
7. Servo center
8. 좌/우 안전 범위
9. Command 0 → 정지

Stage 1에서는 PID보다 **센서와 출력이 믿을 만한지** 먼저 확인한다.

---

## D — Lighting + Ambient / LIN-CAN

D는 두 STM32를 사용한다.

```text
STM32 #3 Gateway
CAN FD + LIN Master
      ↕ LIN
STM32 #4 LIN Slave
Ambient + Lighting
```

Stage 1에서는 CAN Gateway까지 한 번에 하지 않는다.

### Part A: LIN Slave 단독

- [ ] Ambient sensor raw
- [ ] Head/Tail/Brake/Turn LED
- [ ] Lamp state machine

### Part B: LIN 통신

- [ ] LIN transceiver 전원
- [ ] Master header
- [ ] Slave response
- [ ] `Ambient_Status`
- [ ] `Lamp_Command`

CAN↔LIN Mapping은 Stage 2에서 붙인다.

---

## E — HPC + Camera Vision / 인지·판단

개발 중에는 Pi 두 대로 병렬 개발 가능하다.

```text
Pi #1 → Front Vision
Pi #2 → Rear Vision
```

Stage 1은 AI 성능 경쟁이 아니다.

### Front

- [ ] Camera 인식
- [ ] Frame Capture
- [ ] 해상도/FPS
- [ ] OpenCV frame 접근
- [ ] ROI/gray/edge 등 기본 pipeline

### Rear

- [ ] USB Camera 인식
- [ ] Frame Capture
- [ ] 해상도/FPS
- [ ] Object/ROI baseline

두 프로젝트 모두 **결과 데이터 구조**를 먼저 정한다.

```text
VisionResult
├ valid
├ object_detected
├ object_position
├ warning_level
└ timestamp
```

최종 Pi 1대로 합칠 수 있도록 Camera 입력부와 Vision 알고리즘을 분리해 작성한다.

---

## F — VCU + DTC + CAN Integration / 최종 판단

Stage 1에서는 전체 CAN을 기다리지 않고 Driver Input과 VCU State를 단독 시험한다.

### 입력 후보

- Gear P/R/N/D: GPIO
- Accelerator: ADC Pot/Hall
- Brake: ADC Pot
- Steering Wheel: AS5600 후보 I2C
- E-Stop: GPIO

```text
Driver Inputs
→ Read
→ Validate
→ Normalize
→ Vehicle State
→ Arbitration Dummy Input
→ Final Request
→ UART
```

### 해야 할 것

- [ ] Gear state
- [ ] Accel 0~100%
- [ ] Brake 0~100%
- [ ] Steering center/left/right
- [ ] E-Stop
- [ ] `INIT / READY / DRIVE / REVERSE / FAULT`
- [ ] Dummy ADAS/Ultrasonic 요청을 넣어 Arbitration 시험

DTC는 Stage 1에서 각 Fault 변수를 정의한다.

```text
input_valid
sensor_timeout
comm_timeout
fault_active
```

---

# 5. Stage 1 공통 Hardware 순서

## Step 1 — 전원 확인

- Supply Voltage
- Logic Voltage
- GND
- Pinout
- Maximum Input Voltage

## Step 2 — Wiring Table

| Device | Pin | Board Pin | Interface | Voltage |
|---|---|---|---|---|
| | | | | |

## Step 3 — Raw부터 확인

```text
ADC_RAW
TIMER_CNT
ECHO_US
ENCODER_CNT
CAMERA_FRAME_COUNT
```

그 다음 물리값으로 바꾼다.

## Step 4 — 정상/오류 둘 다 시험

```text
정상 입력
최소/중간/최대
센서 분리
Timeout
복구
```

---

# 6. Architecture 공통 원칙

### 센서 Owner는 하나

```text
Encoder → C Drive ECU → RPM → CAN → B/F/E
```

H735가 Encoder를 또 읽지 않는다.

### UI는 Motor를 직접 제어하지 않음

```text
B H735 Request
→ F VCU / 해당 ECU 판단
→ C Control
```

### Vision은 PWM을 직접 만들지 않음

```text
E Vision → Request → F VCU → C Drive/Steer
```

### Raw 영상은 CAN에 보내지 않음

```text
Camera Frame → Pi 내부 처리 → 의미 있는 결과만 CAN
```

### Ultrasonic과 Vision은 역할이 다름

```text
A Ultrasonic = 거리
E Vision     = 물체/공간 해석
F VCU        = 최종 판단
```

---

# 7. Stage 2 시작 조건

- [ ] 자기 Specification 완료
- [ ] Architecture 완료
- [ ] Pin/Wiring 실제값 반영
- [ ] Stage 1 단독 시험 PASS
- [ ] Fault/Invalid 시험
- [ ] CAN/LIN으로 보낼 데이터 후보 정의
- [ ] 테스트 로그/사진/영상 저장

그 다음 처음부터 전체 버스를 연결하지 말고 **두 노드부터** 시작한다.

예:

```text
F VCU ↔ C Drive
F VCU ↔ A Ultrasonic
D LIN Master ↔ D LIN Slave
```

---

# 8. 막혔을 때 질문 형식

```text
Role:
Board:
Sensor/Actuator:
Supply Voltage:
Interface:
Pin:
Expected:
Actual:
UART/Log:
Tried:
```

`안 돼요`는 증상이 아니라 팀 채팅에 발생한 새로운 고장 코드에 가깝다. 재현 가능한 정보를 남긴다.
