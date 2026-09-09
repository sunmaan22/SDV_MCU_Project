# Beginner Guide — ECU Architecture 작성법 & Stage 1 단독 Bring-up

> 대상: STM32 / Raspberry Pi / CAN / LIN 기반 모빌리티 프로젝트를 처음 진행하는 팀원  
> 목표: **각자 자기 Node의 역할·입출력·핀·고장조건을 설명하고, 센서/액추에이터를 단독 시험한 뒤 네트워크 통합으로 넘어가는 것**

---

# 0. 현재 프로젝트 Node부터 이해하기

현재 구조는 다음과 같다.

```text
Raspberry Pi 4 HPC
├ Front Camera → ADAS
├ Rear USB Camera → Parking Vision
├ DTC Manager
└ Logger
       │
     CAN FD
       │
 ┌─────┼───────────────────────────────┐
 │     │            │          │       │
VCU  Drive/Steer  Parking  Body GW   H735 Cockpit
 │                            │
 │                           LIN
 │                            │
Driver Input             Body LIN Slave
                         ├ Ambient Sensor
                         └ Lighting
```

소형 STM32 5개는 다음처럼 사용한다.

| MCU | 역할 |
|---|---|
| STM32 #1 | VCU / Driver Input / Safety |
| STM32 #2 | Drive + Steering ECU |
| STM32 #3 | Parking ECU |
| STM32 #4 | Body Gateway / CAN FD ↔ LIN Master |
| STM32 #5 | Body LIN Slave / Ambient + Lighting |
| STM32H735 | Cluster + IVI 통합 Cockpit ECU |

H735는 Cluster와 IVI를 따로 만드는 것이 아니다. **기본 화면은 Digital Cluster, 메뉴 화면은 ADAS/Parking/DTC/Settings IVI**로 구성한다.

---

# 1. 초심자가 개발하는 순서

처음부터 CAN과 LIN을 전부 연결하지 않는다.

```text
1. 내 Node 역할 정의
2. 명세서 작성
3. Architecture 작성
4. Datasheet 확인
5. Pin Map / Wiring 작성
6. Board Bring-up
7. Sensor / Actuator 단독 시험
8. Raw Data 확인
9. Physical Value 변환
10. Disconnect / Invalid 시험
11. Stage 1 PASS
12. 그 다음 CAN / LIN 통합
```

Stage 1의 목표는 차량 완성이 아니다.

> **내 보드가 자기 입력을 정확히 읽고, 자기 출력을 정확히 만들고, 오류를 검출할 수 있는 상태**가 Stage 1 완료다.

---

# 2. 명세서와 Architecture의 차이

## 명세서 Specification

"무엇을 해야 하는가"를 적는다.

예: Parking ECU

```text
REQ-PARK-001
Parking ECU는 Rear Left/Right 거리센서를 50 ms 이하 주기로 갱신해야 한다.

REQ-PARK-002
센서 응답이 지정 시간 동안 없으면 Sensor Invalid 상태를 생성해야 한다.

REQ-PARK-003
거리값으로 SAFE / WARNING / CRITICAL 상태를 생성해야 한다.
```

좋은 명세는 나중에 PASS/FAIL 시험이 가능해야 한다.

## Architecture

"그 요구사항을 어떤 구조로 구현하는가"를 적는다.

```text
ToF Sensor
   ↓ I2C
Parking STM32
   ↓
Read → Validate → Filter → Warning Level
   ↓
Parking_Status
   ↓ Stage 2
CAN FD
```

즉:

> **Specification = What**  
> **Architecture = How**

---

# 3. Architecture는 5개 항목부터 작성한다

## 3.1 Block Diagram

```text
[INPUT]
   ↓
[MY NODE]
   ↓
[OUTPUT]
```

예: Drive + Steering

```text
Encoder ─────────────┐
                     ▼
VCU Target ──CAN──→ Drive + Steering STM32
                     ├→ TB6612FNG → Brushed Motor
                     └→ PWM → RC Servo
```

## 3.2 I/O Table

| Signal | Direction | Peripheral | Unit | Range | Owner |
|---|---|---|---|---|---|
| Encoder Pulse | Input | Timer | pulse | device 기준 | Drive ECU |
| Motor PWM | Output | PWM | % | 0~100 | Drive ECU |
| Steering PWM | Output | Timer PWM | us | Servo calibration | Drive ECU |

## 3.3 Pin Map

| Function | MCU Pin | Peripheral | Voltage | 비고 |
|---|---|---|---|---|
| | | | | |

예시 핀을 그대로 복사하지 않는다. 실제 보드 schematic / datasheet / CubeMX에서 확인한다.

## 3.4 Software Flow

```text
Init
 ↓
Read
 ↓
Validate
 ↓
Convert / Filter
 ↓
State / Control
 ↓
Output
```

## 3.5 Interface Contract

Stage 1에서는 CAN/LIN을 실제로 붙이지 않아도 무엇을 주고받을지 적는다.

예:

| Signal | Owner | Consumer | Unit | 의미 |
|---|---|---|---|---|
| Motor_RPM | Drive ECU | VCU/H735/HPC | rpm | 실제 모터 회전속도 |
| Gear | VCU | All | enum | P/R/N/D |
| Ambient_Value | Body LIN Slave | Gateway→H735/VCU | raw/% | 조도 |

---

# 4. 데이터 Owner 원칙

같은 센서를 여러 ECU가 직접 읽지 않는다.

예:

```text
Encoder
  ↓
Drive ECU
  ↓ Motor_RPM
CAN FD
  ↓
VCU / H735 / HPC
```

H735가 Encoder를 다시 읽지 않는다.

현재 주요 Owner는 다음과 같다.

| 데이터 | Owner |
|---|---|
| Gear / Accelerator / Brake / Steering Wheel Input | VCU |
| Motor RPM / Vehicle Speed | Drive ECU |
| Parking Distance | Parking ECU |
| Ambient Light / Lamp Local State | Body LIN Slave |
| CAN↔LIN 변환 상태 | Body Gateway |
| ADAS Result | Raspberry Pi HPC |
| DTC History DB | Raspberry Pi HPC |
| 화면 표시값 | H735가 생성하지 않고 각 Owner에서 수신 |

---

# 5. Stage 1 공통 Bring-up

## Step 1 — Board

- [ ] Build
- [ ] Flash
- [ ] Debugger
- [ ] LED Blink 또는 UART Hello

## Step 2 — Datasheet

연결 전에 최소 다음을 확인한다.

- Supply Voltage
- Logic Voltage
- Pinout
- Interface
- Measurement Range
- Timing
- Absolute Maximum Rating

## Step 3 — Wiring Table

```text
Device VCC → ?
Device GND → GND
Signal     → MCU Pin
```

## Step 4 — Peripheral Only Test

- GPIO: 버튼/LED
- ADC: Raw ADC
- I2C: Address ACK
- Timer: Counter
- PWM: 파형 또는 안전한 부하
- CSI/USB: Camera frame
- UART: Debug log

## Step 5 — Raw 값

```text
ADC_RAW = 2048
ENC_CNT = 1250
DIST_RAW = 437
ANGLE_RAW = 1987
```

먼저 Raw가 믿을 만한지 본다.

## Step 6 — Physical Value

```text
Raw → Calibration → Physical Value
```

예:

```text
Accelerator ADC → 0~100 %
Encoder Pulse → RPM
ToF Raw → mm
AS5600 Raw → Steering Angle
```

## Step 7 — Fault

```text
Sensor Disconnect
→ timeout?
→ MCU hang?
→ valid=false?
→ reconnect recovery?
```

---

# 6. 역할별 Stage 1

## A — STM32 #1 VCU / Driver Input / Safety

### Hardware Input

- P/R/N/D 버튼 또는 스위치
- Accelerator: 10k Linear Pot 또는 Hall 후보
- Brake: 10k Linear Pot 후보
- Steering Wheel: AS5600 + Magnet 후보
- E-Stop 버튼
- Battery Voltage / Temperature는 선택적으로 VCU가 소유

### Stage 1

```text
Gear Button → GPIO
Accel/Brake → ADC
Steering → I2C
       ↓
      VCU
       ↓
UART Debug
```

완료 기준:

```text
GEAR = D
ACCEL = 37 %
BRAKE = 0 %
STEERING = -12.5 deg
ESTOP = 0
```

- [ ] 각 입력 Raw 확인
- [ ] 0~100% / 각도 Calibration
- [ ] Gear debounce
- [ ] Accel + Brake 동시 입력 정책 정의
- [ ] E-Stop 상태 정의
- [ ] VCU State Machine 초안

---

## B — STM32 #2 Drive + Steering

### Hardware

```text
STM32
 ├ PWM/DIR → TB6612FNG 후보 → Brushed DC Motor
 ├ Encoder/Hall ← Motor RPM
 └ PWM → RC Servo
```

RC Servo를 쓰므로 초기에는 외부 조향각 센서 없이 시작할 수 있다.

### Stage 1

- [ ] TB6612FNG 입력/전원 정격 확인
- [ ] 낮은 PWM에서 Motor 단독 회전
- [ ] Direction 확인
- [ ] Encoder/Hall pulse 확인
- [ ] RPM 계산
- [ ] RC Servo center/left/right calibration
- [ ] Motor stop 조건 확인

PID는 센서값이 정상임을 확인한 후 진행한다.

---

## C — Raspberry Pi 4 HPC / ADAS / DTC

### Stage 1

```text
Front Pi Camera
 ↓ CSI
Pi 4
 ↓
Frame Capture
 ↓
OpenCV
```

- [ ] Camera 인식
- [ ] 목표 Resolution / FPS 측정
- [ ] 1분 이상 안정 Capture
- [ ] ROI / grayscale / edge 등 최소 CV
- [ ] Camera error 처리
- [ ] CPU Temperature 기록
- [ ] 향후 ADAS CAN Signal 정의

아직 Motor 제어와 CAN 제어는 하지 않는다.

DTC Manager는 Dummy DTC를 DB/JSON에 저장하고 Active/History 구조를 시험해도 좋다.

---

## D — STM32 #3 Parking + Rear Camera Service

### Parking ECU

```text
ToF / Ultrasonic
    ↓
Parking STM32
    ↓
Distance / Valid
```

- [ ] 센서 1개부터 시작
- [ ] Near/Mid/Far 3점 측정
- [ ] 최소 Rear Left/Right로 확장
- [ ] 다중 I2C Address 문제 확인
- [ ] Disconnect / Timeout

### Rear Camera

```text
USB Camera → Raspberry Pi 4
```

C와 협업해 Front/Rear Camera 프로세스 충돌을 막는다.

---

## E — STM32H735 Cluster + IVI Cockpit

H735는 센서를 직접 읽지 않는다.

### Stage 1

Dummy Data로 완성한다.

```text
speed = 23
rpm = 1480
battery = 78
gear = D
adas_warning = false
parking_rr = 320
active_dtc = 1
```

기본 Cluster 화면:

- Speed
- RPM
- Battery
- Temperature
- Gear
- READY
- Lamp / Turn
- Warning

IVI 메뉴:

- ADAS
- Parking
- Diagnostics
- Settings

완료 기준:

- [ ] TouchGFX Build
- [ ] LCD/Touch
- [ ] Cluster Gauge update
- [ ] 화면 전환
- [ ] Warning 표시
- [ ] DTC Dummy list
- [ ] 필요한 CAN Signal 목록 작성

---

## F — STM32 #4 Body Gateway + STM32 #5 Body LIN Slave

F는 두 MCU를 담당한다.

### STM32 #4 Body Gateway

Stage 1에서는 CAN이 없어도 LIN Master부터 검증한다.

```text
Gateway STM32
  ↓ UART/LIN peripheral
LIN Transceiver
  ↓
LIN Bus
```

- [ ] UART/LIN peripheral 확인
- [ ] LIN Transceiver 전압 확인
- [ ] Header / Response 기본 프레임
- [ ] Master Schedule 초안
- [ ] LIN timeout 검출

### STM32 #5 Body LIN Slave

```text
Ambient Sensor → ADC
Buttons(optional)
        ↓
    LIN Slave STM32
        ↓
Head/Tail/Brake/Turn/Hazard LED
```

- [ ] Ambient Raw ADC
- [ ] Lighting GPIO/PWM
- [ ] Turn blink
- [ ] Brake Lamp
- [ ] Lamp local status
- [ ] LIN response 준비

Stage 2에서 Gateway가 다음 Mapping을 수행한다.

```text
CAN Body_Command → LIN Lamp_Command
LIN Ambient_Status → CAN Body_Status
LIN Lamp_Diagnostic → CAN DTC_Event
```

---

# 7. Stage 2 네트워크 통합 순서

Stage 1이 끝난 뒤에도 전부 한꺼번에 연결하지 않는다.

```text
1. VCU ↔ Drive CAN
2. VCU ↔ Parking CAN
3. Gateway ↔ LIN Slave
4. Gateway CAN ↔ VCU/H735
5. H735 CAN RX
6. Pi CAN interface
7. 전체 CAN Backbone
```

네트워크 문제와 센서 문제를 동시에 디버깅하지 않는 것이 목적이다.

---

# 8. 개인 문서 작성 순서

각 담당자는 최소 3개 문서를 남긴다.

```text
1. SPECIFICATION.md
2. ARCHITECTURE.md
3. STAGE1_TEST_REPORT.md
```

템플릿:

- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [ECU Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

작성 예시는 [NODE_SPEC_ARCHITECTURE_EXAMPLES.md](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)에 정리한다.

---

# 9. Stage 1 종료 조건

- [ ] 담당 Node Specification 작성
- [ ] Architecture 작성
- [ ] 실제 Pin/Wiring 작성
- [ ] Board Bring-up
- [ ] Input Raw 확인
- [ ] Physical Conversion
- [ ] Output 단독 시험 또는 N/A
- [ ] Disconnect/Invalid 시험
- [ ] Local Fault 변수 정의
- [ ] Stage 2 Network Signal 후보 정의
- [ ] Log/사진/영상 증거 남김

팀원이 자기 Node를 **3분 안에 Input → Process → Output → Fault → Network 관점으로 설명할 수 있으면** Stage 1 문서가 제대로 작성된 것이다.

---

# 10. 질문할 때 필요한 정보

```text
Board:
MCU:
Sensor/Actuator:
Supply Voltage:
Interface:
MCU Pin:
Expected:
Actual:
UART Log:
Tried:
```

`안 돼요`만 던지면 다른 팀원은 텔레파시 기능이 없어서 해결하기 어렵다.
