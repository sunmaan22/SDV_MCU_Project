# Node별 명세서 / Architecture 작성 예시

> 이 문서는 현재 Architecture v1.2 기준이다.  
> 각 담당자는 이 예시를 참고해 자기 폴더에 `SPECIFICATION.md`, `ARCHITECTURE.md`, `STAGE1_TEST_REPORT.md`를 작성한다.  
> 핀, CAN ID, 주기, 임계값은 실제 부품/시험 후 확정한다.

---

# 0. 현재 역할

| 담당 | 역할 | Hardware |
|---|---|---|
| A | Ultrasonic / 인지 | STM32 #1 + Ultrasonic |
| B | Cluster + IVI / UI | STM32H735 + TouchGFX |
| C | Motor + Steering / 제어 | STM32 #2 + TB6612FNG 후보 + Motor + Servo |
| D | Lighting + Ambient / LIN-CAN | STM32 #3 Gateway + STM32 #4 LIN Slave |
| E | HPC + Camera Vision / 인지·판단 | Raspberry Pi + Front/Rear Camera |
| F | VCU + DTC + CAN Integration | STM32 #5 + Pi Diagnostics 협업 |

---

# 1. A — Ultrasonic Perception ECU

## Specification 예시

### Purpose

차량 주변 초음파센서의 거리를 측정하고 유효성을 검사한다. 거리 결과와 Warning Level을 CAN으로 제공한다.

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-US-001 | 센서 Echo를 이용해 거리값을 mm 단위로 계산해야 한다. |
| REQ-US-002 | 측정값이 유효하지 않으면 `valid=false`를 제공해야 한다. |
| REQ-US-003 | 거리 기준에 따라 SAFE/WARNING/CRITICAL을 생성해야 한다. |
| REQ-US-004 | Sensor timeout을 검출해야 한다. |

### Input / Output

| 구분 | 내용 |
|---|---|
| Input | Echo pulse / Sensor response |
| Output | `distance_mm`, `valid`, `warning_level` |
| Network | CAN FD status |
| Fault | Sensor Timeout / Invalid |

## Architecture 예시

```text
Ultrasonic Sensors
       ↓
Trigger / Echo Driver
       ↓
Distance Calculation
       ↓
Validity Check
       ↓
Filtering
       ↓
Warning Level
       ↓
CAN Status
```

### Stage 1 PASS

- 센서 1개 거리 측정
- 3개 기준거리 비교
- timeout 검출
- 여러 센서 확장 방법 기록

---

# 2. B — STM32H735 Cluster + IVI

## Specification 예시

### Purpose

CAN으로 받은 차량 데이터를 운전자에게 보여준다. 기본 화면은 Cluster이고 메뉴에서 ADAS/Parking/DTC/Settings 화면으로 이동한다.

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-HMI-001 | Speed/RPM/Gear를 Cluster에 표시해야 한다. |
| REQ-HMI-002 | ADAS/Parking warning을 표시해야 한다. |
| REQ-HMI-003 | DTC 상세 화면을 제공해야 한다. |
| REQ-HMI-004 | Touch로 화면을 전환할 수 있어야 한다. |

## Architecture 예시

```text
CAN RX
  ↓
Vehicle Data Model
  ↓
TouchGFX Presenter / Model
  ↓
Cluster Main
├ ADAS
├ Parking
├ Diagnostics
└ Settings
```

### Stage 1

CAN 없이 Dummy Data를 사용한다.

```text
speed = 24
rpm = 1250
gear = D
warning = true
```

### Stage 1 PASS

- LCD/Touch 동작
- 숫자/gauge 갱신
- 화면전환
- Warning on/off
- Dummy DTC list

---

# 3. C — Drive + Steering Control ECU

## Specification 예시

### Purpose

VCU의 Final Speed / Steering Request를 받아 Motor와 RC Servo를 제어하고 실제 RPM/상태를 반환한다.

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-DRV-001 | PWM으로 TB6612FNG 후보 Motor Driver를 제어해야 한다. |
| REQ-DRV-002 | Encoder/Hall을 이용해 RPM을 계산해야 한다. |
| REQ-DRV-003 | Steering Request를 Servo pulse로 변환해야 한다. |
| REQ-DRV-004 | Command timeout 시 안전한 정지 상태로 가야 한다. |

## Architecture 예시

```text
Final Speed Request ──────────┐
Final Steering Request ───────┤
                              ▼
                     Drive + Steering ECU
                       │              │
                       │              └→ Servo PWM → RC Servo
                       │
                       └→ Speed Control → PWM → TB6612FNG → DC Motor
                                                ↑
                                          Encoder / Hall
```

### Stage 1 PASS

- Motor PWM 저출력 시험
- RPM 계산
- Servo Left/Center/Right
- Motor/Encoder fault 후보 정의

---

# 4. D — Body Gateway + LIN Slave

D는 두 MCU를 사용한다.

```text
STM32 #3 = CAN FD ↔ LIN Gateway / LIN Master
STM32 #4 = LIN Slave / Ambient / Lighting
```

## 4.1 Gateway Specification 예시

### Purpose

CAN FD 차량망과 LIN Body망 사이에서 필요한 신호를 변환한다.

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-GW-001 | CAN `Body_Command`를 LIN `Lamp_Command`로 변환해야 한다. |
| REQ-GW-002 | LIN `Ambient_Status`를 CAN `Body_Status`로 변환해야 한다. |
| REQ-GW-003 | LIN Slave timeout을 검출해야 한다. |
| REQ-GW-004 | LIN Schedule을 주기적으로 실행해야 한다. |

## 4.2 LIN Slave Specification 예시

| ID | Requirement |
|---|---|
| REQ-BODY-001 | Ambient Sensor 값을 읽어 LIN status로 제공해야 한다. |
| REQ-BODY-002 | LIN Lamp Command에 따라 Lamp를 제어해야 한다. |
| REQ-BODY-003 | Lamp/Ambient local fault를 status로 제공해야 한다. |

## Architecture 예시

```text
CAN FD
  ↕
STM32 #3 Gateway
  ├ CAN RX/TX
  ├ CAN↔LIN Mapping
  └ LIN Master / Schedule
        ↕ LIN
STM32 #4 LIN Slave
  ├ Ambient Sensor
  └ Lighting Outputs
```

### Mapping Table 예

| CAN Signal | 방향 | LIN Signal |
|---|---|---|
| `HEADLAMP_REQ` | CAN → LIN | `LAMP_HEAD_CMD` |
| `TURN_REQ` | CAN → LIN | `LAMP_TURN_CMD` |
| `AMBIENT_VALUE` | LIN → CAN | `BODY_AMBIENT` |
| `LIN_FAULT` | LIN/Gateway → CAN | `BODY_DTC` |

### Stage 1 PASS

- Slave Ambient 읽기
- Slave LED 제어
- LIN Master↔Slave 통신
- CAN↔LIN Mapping 문서 초안

---

# 5. E — Raspberry Pi HPC + Camera Vision

## Specification 예시

### Purpose

Front/Rear Camera 영상을 처리해 차량 주변 상황을 인지하고 ADAS/Parking용 상위 Request를 생성한다.

### 개발 방법

```text
Pi #1 + Front Camera → Front Vision 개발
Pi #2 + Rear Camera  → Rear Vision 개발
```

최종적으로 한 Pi에 통합한다.

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-VIS-001 | Front Camera frame을 안정적으로 획득해야 한다. |
| REQ-VIS-002 | Front Vision은 Lane/Object result 중 최소 하나를 생성해야 한다. |
| REQ-VIS-003 | Rear Vision은 Parking object/status를 생성해야 한다. |
| REQ-VIS-004 | Raw Frame은 CAN으로 보내지 않아야 한다. |
| REQ-VIS-005 | Gear D/R에 따라 Front/Rear service를 전환할 수 있어야 한다. |

## Architecture 예시

```text
Front Camera ─→ Front Vision ─┐
                              ├→ Vision Result / Request → CAN Service
Rear Camera ──→ Rear Vision ──┘

Gear / Vehicle State
        ↓
Vehicle Manager
        ↓
Front / Rear Service Enable
```

### Output 예

```text
LANE_OFFSET
OBJECT_DETECTED
OBJECT_TYPE
VISION_WARNING
SPEED_REQUEST
STEERING_REQUEST
```

### Stage 1 PASS

- Front/Rear 각각 Frame
- FPS 기록
- 간단한 OpenCV 처리
- disconnect/error 처리
- GitHub 모듈 분리

---

# 6. F — VCU + DTC + CAN Integration

## Specification 예시

### Purpose

Driver Input, Vision Request, Ultrasonic Status, Fault를 보고 최종 Speed/Steering Request를 결정한다. CAN Signal과 DTC 체계를 통합한다.

### 직접 입력 후보

- Gear P/R/N/D
- Accelerator Position
- Brake Position
- Steering Wheel Angle
- E-Stop

### Requirement 예

| ID | Requirement |
|---|---|
| REQ-VCU-001 | Gear를 P/R/N/D 중 유효한 상태로 관리해야 한다. |
| REQ-VCU-002 | Accelerator/Brake를 logical value로 변환해야 한다. |
| REQ-VCU-003 | Critical Ultrasonic/Fault 시 안전 정책을 적용해야 한다. |
| REQ-VCU-004 | ADAS Request를 최종 명령으로 바로 전달하지 않고 검증해야 한다. |
| REQ-VCU-005 | Critical ECU heartbeat timeout을 검출해야 한다. |
| REQ-DTC-001 | 프로젝트 공통 DTC 코드/상태 규칙을 정의해야 한다. |

## Architecture 예시

```text
Driver Input ────────────┐
Vision Request ──────────┤
Ultrasonic Status ───────┤
Heartbeat / Fault ───────┤
                         ▼
                       VCU
                 ┌───────┼────────┐
                 │       │        │
              State   Safety   Arbitration
                 │       │        │
                 └───────┴────────┘
                         ↓
             Final Speed / Steering
                         ↓ CAN
                 Drive + Steering
```

## DTC 구조

```text
각 Node Local Fault
      ↓
DTC Event
      ↓ CAN
Pi DTC Manager / DB
      ↓
H735 Diagnostic UI
```

F가 모든 ECU 고장을 직접 코딩하는 것이 아니라 **DTC ID/Status/Severity/통합 규칙을 관리**한다.

### Stage 1 PASS

- Gear / Accel / Brake / Steering 입력
- 기본 State Machine
- E-Stop
- CAN Signal Owner 표
- DTC Naming Rule v0.1

---

# 7. 공통 문서 작성 방법

각자 `SPECIFICATION.md`에서는 다음을 적는다.

```text
이 기능은 왜 필요한가?
무엇을 해야 하는가?
무엇은 하지 않는가?
입력은 무엇인가?
출력은 무엇인가?
고장은 어떻게 알 것인가?
Stage 1 PASS 조건은 무엇인가?
```

`ARCHITECTURE.md`에서는 다음을 적는다.

```text
센서/입력
   ↓
Driver / Software Component
   ↓
Validation / Processing / Control
   ↓
Local Output / CAN / LIN
```

---

# 8. 팀 공통 Data Owner 예시

| Data | Owner |
|---|---|
| Ultrasonic Distance | A |
| Parking Warning from Ultrasonic | A |
| Speed / RPM | C |
| Steering Actuator Status | C |
| Ambient / Lamp Status | D |
| Vision Object / Lane | E |
| ADAS/Parking Vision Request | E |
| Gear / Driver Input | F |
| Final Speed / Steering Request | F |
| DTC Code Rule | F |
| UI Representation | B |

같은 값을 여러 Node가 제각각 계산하지 않는다. **누가 그 데이터의 원본을 만드는지(owner)를 먼저 정한다.**

---

# 9. 다음 문서

- [팀 역할 쉬운 설명](TEAM_ROLE_EASY_GUIDE.md)
- [Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)
