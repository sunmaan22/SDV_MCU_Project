# Node별 명세서 / Architecture 작성 예시

> 이 문서는 팀원 A~F가 자기 `SPECIFICATION.md`와 `ARCHITECTURE.md`를 작성할 때 참고하는 **예시**다.  
> 핀 번호, 주기, 임계값, CAN ID, 센서 모델은 실제 부품 확인 후 확정한다. 그대로 복붙해서 확정값처럼 사용하지 않는다.

---

# 0. 현재 Node 구성

| 담당 | Node | Hardware |
|---|---|---|
| A | VCU / Driver Input / Safety | STM32 #1 |
| B | Drive + Steering ECU | STM32 #2 + TB6612FNG 후보 + Brushed Motor + RC Servo |
| C | Central HPC / ADAS / DTC | Raspberry Pi 4 + Front Pi Camera |
| D | Parking ECU + Rear Vision 협업 | STM32 #3 + ToF/Ultrasonic + Rear USB Camera |
| E | Cluster + IVI Cockpit | STM32H735 + TouchGFX |
| F | Body Gateway | STM32 #4 + CAN FD/LIN Transceiver |
| F | Body LIN Slave | STM32 #5 + Ambient Sensor + Lighting |

각 담당자는 최소 다음 문서를 만든다.

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

---

# 1. A — STM32 #1 VCU / Driver Input / Safety

## 1.1 명세서 예시

### Purpose

VCU는 운전자 입력과 ADAS/Parking 요청을 모아 차량의 최종 Speed/Steering 요청을 결정한다. Drive ECU에 직접 PWM 값을 주는 것이 아니라 **안전 검증이 끝난 최종 Request**를 제공한다.

### Scope

하는 일:

- P/R/N/D Gear 관리
- Accelerator / Brake / Steering Wheel 입력 소유
- E-Stop
- Vehicle Mode
- ADAS/Parking/Driver Request Arbitration
- Critical Heartbeat 감시
- Battery Voltage/Temperature 후보 입력

하지 않는 일:

- Motor PWM 생성
- RC Servo PWM 생성
- Camera Vision
- Lighting 직접 구동

### Functional Requirement 예

| ID | Requirement | Priority |
|---|---|---|
| REQ-VCU-001 | VCU는 Gear 입력을 P/R/N/D 중 하나의 유효 상태로 관리해야 한다. | MUST |
| REQ-VCU-002 | Accelerator와 Brake를 0~100% logical value로 변환해야 한다. | MUST |
| REQ-VCU-003 | Brake가 활성화된 경우 Accelerator보다 Brake/Safe policy를 우선해야 한다. | MUST |
| REQ-VCU-004 | E-Stop 입력 시 Final Speed Request를 0으로 만들어야 한다. | MUST |
| REQ-VCU-005 | Gear R에서 Reverse/Parking state를 생성해야 한다. | MUST |
| REQ-VCU-006 | Critical ECU heartbeat timeout을 검출해야 한다. | SHOULD |

### Input 예

| Input | Source | Interface | Unit |
|---|---|---|---|
| Gear | Buttons | GPIO | enum |
| Accelerator | Pot/Hall | ADC | % |
| Brake | Pot | ADC | % |
| Steering Wheel | AS5600 후보 | I2C | deg / % |
| E-Stop | Button | GPIO | bool |
| ADAS_Request | Pi | CAN FD | speed/angle |
| Parking_Status | Parking ECU | CAN FD | enum/distance |

### Output 예

| Output | Destination | Meaning |
|---|---|---|
| Vehicle_State | All | Gear / Mode / Safety |
| Final_Speed_Request | Drive ECU | 최종 속도 요청 |
| Final_Steering_Request | Drive ECU | 최종 조향 요청 |
| VCU_DTC | HPC | VCU fault |

### DTC 예

| DTC | 조건 |
|---|---|
| `VCU_001` | Gear input invalid |
| `VCU_002` | Accelerator range invalid |
| `VCU_003` | Steering sensor invalid |
| `VCU_010` | Drive ECU heartbeat timeout |

## 1.2 Architecture 예시

```text
Gear GPIO ───────────┐
Accel ADC ───────────┤
Brake ADC ───────────┤
Steering I2C ────────┤
E-Stop GPIO ─────────┤
ADAS CAN ────────────┤
Parking CAN ─────────┤
                     ▼
               ┌──────────┐
               │   VCU    │
               │          │
               │ Input    │
               │ Validate │
               │ State    │
               │ Safety   │
               │ Arbitrate│
               └────┬─────┘
                    │
          Final Speed / Steering
                    │ CAN FD
                    ▼
             Drive + Steering ECU
```

### Software Module 예

```text
vcu_input
├ gear_input
├ pedal_input
└ steering_input

vcu_state
├ vehicle_mode
└ gear_state

vcu_safety
├ estop
├ plausibility
└ heartbeat_monitor

vcu_arbitration
└ final_command

can_service
```

### Stage 1 해야 할 일

- [ ] Gear 버튼 4개 또는 switch bring-up
- [ ] Accelerator ADC min/max 측정
- [ ] Brake ADC min/max 측정
- [ ] Steering raw/center calibration
- [ ] E-Stop
- [ ] UART 상태 출력
- [ ] `INIT/READY/DRIVE/REVERSE/FAULT` state 초안
- [ ] 센서 분리/범위오류 변수 정의

---

# 2. B — STM32 #2 Drive + Steering ECU

## 2.1 명세서 예시

### Purpose

Drive + Steering ECU는 VCU가 보낸 최종 요청을 실제 Motor/Servo 출력으로 변환하는 Hard Real-Time Node다.

### Functional Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-DRV-001 | ECU는 Motor PWM과 Direction을 생성해야 한다. | MUST |
| REQ-DRV-002 | Encoder/Hall로 Motor RPM을 계산해야 한다. | MUST |
| REQ-DRV-003 | VCU command timeout 시 Motor를 안전 상태로 전환해야 한다. | MUST |
| REQ-STR-001 | ECU는 Steering Request를 RC Servo command로 변환해야 한다. | MUST |
| REQ-STR-002 | Steering center/left/right calibration 값을 제한해야 한다. | MUST |
| REQ-DRV-010 | Closed-loop Speed PID를 지원한다. | SHOULD |

### Hardware

- Brushed DC Motor
- TB6612FNG 후보
- Encoder / Hall
- RC Servo

### Input

| Input | Source | Interface |
|---|---|---|
| Final Speed Request | VCU | CAN FD |
| Final Steering Request | VCU | CAN FD |
| Encoder/Hall | Motor | Timer/GPIO |

### Output

| Output | Destination | Interface |
|---|---|---|
| Motor PWM | TB6612FNG | PWM |
| Motor Direction | TB6612FNG | GPIO |
| Steering Pulse | RC Servo | PWM |
| Drive_Status | CAN | CAN FD |

### DTC 예

- `DRIVE_001`: Encoder timeout
- `DRIVE_002`: Command timeout
- `DRIVE_003`: Motor temperature high, if implemented
- `STEER_001`: Servo command invalid

## 2.2 Architecture 예시

```text
VCU CAN
 ↓
Target Speed / Steering
 ↓
┌────────────────────────────┐
│ Drive + Steering STM32     │
│                            │
│ Target → Speed Controller ─┼→ PWM/DIR → TB6612FNG → Motor
│               ↑            │
│          Encoder RPM       │
│                            │
│ Steering Mapping ──────────┼→ PWM → RC Servo
└────────────────────────────┘
```

### Stage 1 해야 할 일

1. TB6612FNG 후보와 Motor 정격 확인
2. Motor 없이 PWM 확인
3. 낮은 PWM부터 Motor 회전
4. Direction
5. Encoder count
6. RPM 계산
7. Servo center
8. Left/right safe limit
9. Stop command
10. Encoder disconnect fault

### Stage 1 결과 예

```text
PWM=20% RPM=95
PWM=30% RPM=142
PWM=40% RPM=188
SERVO=CENTER pulse=xxxx us
```

---

# 3. C — Raspberry Pi 4 Central HPC / ADAS / DTC

Raspberry Pi는 MCU는 아니지만 차량 전체에서 하나의 Compute Node이므로 동일하게 Spec/Architecture를 작성한다.

## 3.1 명세서 예시

### Purpose

Pi는 고대역폭 Camera 데이터를 처리하고 ADAS 결과를 생성한다. 또한 각 ECU의 DTC를 중앙 저장하고 차량 로그를 기록한다.

### Functional Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-HPC-001 | Front CSI Camera frame을 안정적으로 획득해야 한다. | MUST |
| REQ-HPC-002 | Gear D/ADAS mode에서 Front ADAS service를 실행해야 한다. | MUST |
| REQ-HPC-003 | Gear R에서 Front ADAS를 pause하고 Rear Camera service를 활성화해야 한다. | MUST |
| REQ-HPC-004 | Raw Camera Frame을 CAN FD에 전송하지 않아야 한다. | MUST |
| REQ-HPC-005 | ADAS semantic result만 CAN으로 제공해야 한다. | MUST |
| REQ-DTC-001 | DTC Event를 Active/History 형태로 저장해야 한다. | MUST |
| REQ-DTC-002 | First Seen/Last Seen/Count를 기록해야 한다. | SHOULD |

### ADAS Output 후보

- Lane Offset
- Lane Angle
- Object Type
- Collision Level
- Speed Request
- Steering Request

### DTC DB 예

```text
code
source_ecu
status
severity
first_seen
last_seen
count
```

## 3.2 Architecture 예시

```text
Front CSI Camera ─→ front_camera_service ─→ ADAS pipeline ─┐
                                                          │
Rear USB Camera ──→ rear_camera_service ─→ parking vision ┤
                                                          ▼
                                                   vehicle_service
                                                          │
                                                     SocketCAN
                                                          │
                                                        CAN FD

CAN DTC_Event ─→ dtc_manager ─→ SQLite/JSON ─→ H735 response
```

### Process 구성 예

```text
adas_service
parking_vision_service
gateway_can_service
dtc_manager
logger
vehicle_mode_manager
```

### Stage 1 해야 할 일

- [ ] Front Camera capture
- [ ] Resolution/FPS
- [ ] OpenCV frame
- [ ] 1분 이상 안정 동작
- [ ] CPU temperature
- [ ] Camera error 처리
- [ ] Dummy DTC DB 생성
- [ ] Service 구조 문서화

---

# 4. D — STM32 #3 Parking ECU

D는 Rear Camera service도 C와 협업하지만 **STM32 #3의 Sensor Owner는 거리센서**다.

## 4.1 명세서 예시

### Purpose

Parking ECU는 ToF/Ultrasonic 센서를 주기적으로 읽어 거리값의 유효성을 검사하고 Parking Warning Level을 생성한다.

### Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-PARK-001 | 활성 거리센서를 주기적으로 읽어야 한다. | MUST |
| REQ-PARK-002 | 거리값에 Valid/Invalid 상태를 제공해야 한다. | MUST |
| REQ-PARK-003 | SAFE/WARNING/CRITICAL level을 제공해야 한다. | MUST |
| REQ-PARK-004 | Sensor timeout을 검출해야 한다. | MUST |
| REQ-PARK-005 | R mode에서 필요한 Parking status를 제공해야 한다. | MUST |

### 센서 구성

1차:

- Rear Left
- Rear Right

확장:

- Front Left / Front Right

### DTC

- `PARK_001`: RL sensor timeout
- `PARK_002`: RR sensor timeout
- `PARK_010`: multi-sensor invalid

## 4.2 Architecture 예시

```text
RL ToF ─┐
RR ToF ─┤
        ▼
 Parking STM32
 ├ Sensor Driver
 ├ Validity
 ├ Filtering
 └ Warning Logic
        │
 Parking_Status
        │ CAN FD
        ▼
 VCU / H735 / Pi
```

### Stage 1 해야 할 일

- [ ] 센서 1개 I2C/GPIO bring-up
- [ ] Near/Mid/Far 측정
- [ ] 최소 2개 확장
- [ ] Address collision 해결
- [ ] Sensor disconnect
- [ ] `distance_valid`
- [ ] UART output
- [ ] Rear USB Camera capture는 C와 협업

---

# 5. E — STM32H735 Cluster + IVI Cockpit

## 5.1 명세서 예시

### Purpose

H735는 Driver HMI 단일 Node다. 기본 화면은 Digital Cluster이고, 터치 메뉴로 ADAS/Parking/Diagnostics/Vehicle Settings를 제공한다.

### 중요한 경계

H735가 하는 일:

- CAN data 수신
- Driver information 표시
- Warning 표시
- Diagnostic Detail 표시
- User Request 생성

H735가 하지 않는 일:

- Speed/RPM sensor 직접 읽기
- Motor PWM 생성
- ADAS 판단
- DTC 원인 직접 진단

### Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-HMI-001 | Power on 후 Cluster main screen을 제공해야 한다. | MUST |
| REQ-HMI-002 | Speed/RPM/Gear/Battery/Warning을 표시해야 한다. | MUST |
| REQ-HMI-003 | ADAS/Parking/DTC 화면으로 전환 가능해야 한다. | MUST |
| REQ-HMI-004 | Critical warning을 Cluster main에서도 표시해야 한다. | MUST |
| REQ-HMI-005 | 상세 DTC를 Diagnostic 화면에 표시해야 한다. | MUST |
| REQ-HMI-006 | UI command는 Request로 보내고 직접 actuator를 제어하지 않아야 한다. | MUST |

### Cluster Signal

- Speed
- RPM
- Gear
- Battery Voltage / Estimated SOC
- Battery / Motor Temp
- READY
- Turn/Lamp
- ADAS active
- General warning

### IVI Screen

- ADAS
- Parking
- Diagnostics
- Settings

## 5.2 Architecture 예시

```text
CAN FD
 ↓
CAN Rx Service
 ↓
Vehicle Data Model
 ├ speed/rpm/gear
 ├ battery/temp
 ├ adas
 ├ parking
 ├ body
 └ dtc
 ↓
TouchGFX Presenter / View
 ↓
Cluster Main + IVI Menu
```

### Stage 1 해야 할 일

CAN 없이 Dummy Model로 먼저 한다.

- [ ] TouchGFX build
- [ ] Cluster main
- [ ] Speed/RPM gauge
- [ ] Gear
- [ ] Warning
- [ ] Screen transition
- [ ] Parking dummy
- [ ] ADAS dummy
- [ ] DTC list dummy
- [ ] 각 화면 CAN Signal 요구 목록

---

# 6. F — STM32 #4 Body Gateway / LIN Master

## 6.1 명세서 예시

### Purpose

Body Gateway는 CAN FD Backbone과 LIN Body Network 사이에서 Signal을 Mapping한다. 실제 SDV의 Zone/Gateway 개념을 축소해서 보여주는 Node다.

### Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-GW-001 | Gateway는 LIN Master 역할을 수행해야 한다. | MUST |
| REQ-GW-002 | LIN Schedule에 따라 Slave status를 읽어야 한다. | MUST |
| REQ-GW-003 | CAN Body_Command를 LIN Lamp_Command로 변환해야 한다. | MUST |
| REQ-GW-004 | LIN Ambient_Status를 CAN Body_Status로 변환해야 한다. | MUST |
| REQ-GW-005 | LIN timeout을 검출하고 Gateway/Body DTC를 생성해야 한다. | MUST |

### Mapping 예

| CAN | 방향 | LIN |
|---|---|---|
| `HEADLAMP_REQ` | CAN→LIN | `LAMP_HEAD_CMD` |
| `TURN_LEFT_REQ` | CAN→LIN | `LAMP_LEFT_CMD` |
| `BRAKE_LAMP_REQ` | CAN→LIN | `LAMP_BRAKE_CMD` |
| `AMBIENT_VALUE` | LIN→CAN | `BODY_AMBIENT` |
| `LAMP_STATUS` | LIN→CAN | `BODY_LAMP_STATUS` |
| `LAMP_FAULT` | LIN→CAN | `DTC_Event` |

### LIN Schedule 예

| Slot | Frame | Period |
|---:|---|---:|
| 1 | Ambient_Status | 100 ms |
| 2 | Lamp_Command | 50 ms |
| 3 | Lamp_Status | 100 ms |
| 4 | Lamp_Diagnostic | 500 ms |

## 6.2 Architecture 예시

```text
CAN FD
 ↓
CAN Service
 ↓
Gateway Mapping
 ↕
LIN Schedule / Master
 ↓
LIN Transceiver
 ↓
LIN Bus
```

### Stage 1 해야 할 일

- [ ] LIN Transceiver power/pin
- [ ] Master peripheral
- [ ] Header/basic frame
- [ ] Slave response 수신
- [ ] Schedule Table v0.1
- [ ] Timeout
- [ ] CAN↔LIN Mapping Table 작성, 실제 CAN은 Stage 2 가능

---

# 7. F — STM32 #5 Body LIN Slave / Lighting

## 7.1 명세서 예시

### Purpose

Body LIN Slave는 저속 Body Sensor/Actuator Node다. Ambient Light를 읽고 Gateway의 Lamp Command에 따라 Lighting을 제어한다.

### Requirements

| ID | Requirement | Priority |
|---|---|---|
| REQ-BODY-001 | Ambient sensor 값을 읽어야 한다. | MUST |
| REQ-BODY-002 | LIN Master 요청에 Ambient_Status를 제공해야 한다. | MUST |
| REQ-BODY-003 | Lamp_Command를 받아 Lighting을 제어해야 한다. | MUST |
| REQ-BODY-004 | Lamp local state를 Lamp_Status로 제공해야 한다. | MUST |
| REQ-BODY-005 | Local fault를 diagnostic status로 제공해야 한다. | SHOULD |

### Local I/O

Input:

- Ambient Sensor

Output:

- Head Lamp
- Tail Lamp
- Brake Lamp
- Left Turn
- Right Turn
- Hazard

## 7.2 Architecture 예시

```text
Ambient Sensor
     ↓ ADC
┌─────────────────┐
│ STM32 #5 Slave  │
│                 │
│ Ambient Service │
│ Lamp Logic      │
│ LIN Slave       │
└──────┬──────────┘
       │
       ├→ Head/Tail/Brake LED
       └→ Turn/Hazard LED
       ↑
      LIN
       ↑
Body Gateway Master
```

### Stage 1 해야 할 일

- [ ] Ambient ADC raw min/max
- [ ] Head/Tail/Brake LED
- [ ] Turn blink period
- [ ] Hazard
- [ ] Lamp status variable
- [ ] LIN Slave response
- [ ] LIN communication loss 후 local behavior 정의

---

# 8. Node 사이의 첫 통합 순서

각 Node가 Stage 1 PASS 후 아래처럼 작은 단위로 연결한다.

```text
A VCU ↔ B Drive

A VCU ↔ D Parking

F Gateway ↔ F LIN Slave

A/F ↔ E H735

C Pi ↔ CAN Backbone
```

그 다음 전체를 연결한다.

---

# 9. 공통 문서에서 반드시 맞춰야 할 것

팀원 문서가 따로 놀지 않으려면 다음 이름은 공통으로 맞춘다.

## Data Name

예:

```text
Gear
Accelerator_Pct
Brake_Pct
Steering_Input
Final_Speed_Request
Final_Steering_Request
Motor_RPM
Vehicle_Speed
Parking_RR
Parking_Level
Ambient_Value
Lamp_Status
ADAS_State
DTC_Status
```

## Unit

```text
Speed      → m/s 또는 프로젝트 합의 단위 하나
Distance   → mm
Angle      → deg
Pedal      → %
RPM        → rpm
Temp       → °C
Voltage    → V 또는 mV, CAN에서는 scaling 명시
```

## Validity

값과 Valid를 구분한다.

```text
distance_mm = 0
```

만으로는 실제 0인지 센서 오류인지 모른다.

```text
distance_mm
distance_valid
```

형태가 더 낫다.

---

# 10. 팀원별 최종 Stage 1 제출물

| 담당 | 제출물 |
|---|---|
| A | VCU Spec, Architecture, Gear/Pedal/Steering/E-stop Test |
| B | Drive Spec, Architecture, Motor/Encoder/Servo Test |
| C | HPC Spec, Architecture, Front Camera/FPS + DTC DB Test |
| D | Parking Spec, Architecture, Distance + Rear Camera Test |
| E | Cockpit Spec, Architecture, Cluster/IVI Dummy UI Test |
| F | Gateway Spec + Slave Spec, Architecture 2개, LIN/Lighting Test |

템플릿:

- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [ECU Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

예시는 요구사항 작성 방향을 보여주기 위한 것이며, 실제 수치와 Pin은 반드시 실제 Hardware 기준으로 수정한다.
