# Team Guide

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> 처음 보는 팀원이 이 문서 하나로 **내 역할, 필요한 전자기초, RTOS가 왜 필요한지, 개발 순서**를 이해하는 것을 목표로 한다.

# 1. 프로젝트를 아주 쉽게 보면

```text
센서/카메라가 본다          = 인지
        ↓
상황을 해석하고 결정한다     = 판단
        ↓
모터/서보/조명을 움직인다   = 제어
        ↓
화면에 보여준다              = UI
        ↓
노드끼리 데이터를 주고받는다 = CAN / LIN
        ↓
고장을 찾고 기록한다         = DTC
```

우리 프로젝트는 이 기능을 여러 Node로 나눠 구현한다.

STM32 Node는 가능한 경우 **FreeRTOS**를 사용한다.

```text
센서 읽기 Task
제어 Task
CAN/LIN Task
UI Task
진단 Task
```

처럼 서로 다른 일을 나누고, 중요한 일이 덜 중요한 일 때문에 늦어지지 않게 만드는 것이 목적이다.

Raspberry Pi는 FreeRTOS가 아니라 Linux에서 Service/Process/Thread 구조로 개발한다.

---

# 2. RTOS를 아주 쉽게 이해하기

Bare-metal에서는 보통 하나의 큰 `while(1)` 안에 모든 일을 넣기 쉽다.

```text
while(1)
 ├ 센서 읽기
 ├ CAN 확인
 ├ 모터 제어
 ├ 화면 갱신
 └ DTC 확인
```

기능이 늘어나면 한 기능이 오래 걸릴 때 다른 기능도 같이 늦어진다.

FreeRTOS에서는 일을 Task로 나눈다.

```text
ControlTask      : 10 ms마다 제어
CanRxTask        : CAN frame 도착 시 처리
SensorTask       : 센서 측정
GuiTask          : 화면 갱신
DiagnosticTask   : 낮은 우선순위로 상태 확인
```

중요한 개념은 다섯 개다.

| 개념 | 쉽게 말하면 |
|---|---|
| Task | 독립적으로 실행되는 일 |
| Priority | 무엇을 먼저 실행할지 |
| Queue | Task끼리 데이터를 안전하게 전달하는 통로 |
| Semaphore / Notification | 어떤 일이 생겼다고 Task를 깨우는 신호 |
| Mutex | 한 자원을 동시에 두 Task가 쓰지 못하게 잠그는 것 |

ISR은 센서 Edge나 CAN interrupt가 왔다는 사실만 빠르게 처리하고, 긴 계산은 Task에서 한다.

```text
Interrupt
→ timestamp / flag
→ Task Notification
→ Task에서 실제 처리
```

---

# 3. 6명은 무엇을 하는가

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

### RTOS에서 나누는 예

```text
Timer Input Capture ISR
        ↓ Task Notification
UltrasonicTask
        ↓ Queue
CanTxTask

HealthTask
→ sensor timeout / task health
```

초기 후보:
- `UltrasonicTask`: 20~50 ms 후보
- `CanTxTask`: event 또는 status period
- `HealthTask`: 100 ms 후보

정확한 주기는 실제 센서 응답시간과 차량 요구사항을 보고 확정한다.

---

## B — Cluster + IVI / UI

**한마디:** 다른 Node가 만든 값을 운전자에게 보여준다.

```text
CAN FD
→ H735 Vehicle Data Model
→ TouchGFX
→ Cluster Main / ADAS / Parking / DTC / Settings
```

### RTOS에서 나누는 예

```text
FDCAN ISR
  ↓
CanRxTask
  ↓ Queue
VehicleModelTask
  ↓
GuiTask / TouchGFX

Touch Event
  ↓
CommandTxTask
  ↓
CAN TX

HealthTask
```

GUI가 바쁘다고 CAN timeout 검출이 멈추거나, CAN frame을 많이 받는다고 TouchGFX가 멈추면 안 된다.

세부 예시는 [`IVI/ARCHITECTURE.md`](../ecus/IVI/ARCHITECTURE.md)를 참고한다.

---

## C — Motor + Steering / 제어

**한마디:** VCU가 정한 명령을 실제 움직임으로 바꾼다.

```text
VCU Final Command
→ STM32
├ Motor PWM / Direction → Motor Driver → Brushed DC Motor
└ Steering PWM → RC Servo

Motor Encoder/Hall
→ RPM Feedback
```

### RTOS에서 중요한 부분

```text
CanRxTask
   ↓ latest command
ControlTask  ← Encoder/Feedback
   ↓
PWM / Servo Output

StatusTask → CAN
HealthTask → command timeout / sensor fault
```

`ControlTask`는 이 Node에서 가장 중요한 Task 중 하나다.

초기 후보:
- `ControlTask`: 5~10 ms
- `FeedbackTask`: 5~10 ms 또는 ISR+Task
- `StatusTask`: 20~50 ms
- `HealthTask`: 50~100 ms

중요:
- ControlTask 안에서 느린 `printf` 금지
- CAN 송신 때문에 ControlTask가 오래 block되지 않게 함
- Encoder edge ISR에서 PID 계산하지 않음

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

### Gateway RTOS 예

```text
CanRxTask
   ↓
GatewayMappingTask
   ↓
LinScheduleTask
   ↓
LIN

LinRx Event
→ GatewayMappingTask
→ CanTxTask

HealthTask
```

`LinScheduleTask`는 LIN Schedule을 일정하게 실행하는 역할이다.

### LIN Slave RTOS 예

```text
LinRxTask
AmbientTask
LightingTask
StatusTask
HealthTask
```

소형 Slave MCU의 RAM/Flash가 너무 작은 경우만 측정 근거를 가지고 Bare-metal 예외를 검토한다.

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

Pi는 Linux이므로 FreeRTOS Task를 만들지 않는다.

대신 Software Architecture에는 다음을 적는다.

```text
front_vision service
rear_vision service
can_service
dtc_manager
vehicle_manager
logger
```

필요하면 Process/Thread/Queue로 분리한다.

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

### RTOS에서 나누는 예

```text
DriverInputTask ───────┐
CanRxTask ─────────────┤
                       ↓
                  VcuControlTask
                       ↓
                   CanTxTask

SafetyTask
→ E-Stop / heartbeat / critical fault

DiagnosticTask
→ DTC / health
```

초기 후보:
- `SafetyTask`: 5~10 ms 또는 event, 가장 높은 Application priority 후보
- `VcuControlTask`: 10 ms 후보
- `DriverInputTask`: 10 ms 후보
- `CanRxTask`: event driven, 높은 priority
- `CanTxTask`: 20 ms/event 후보
- `DiagnosticTask`: 100 ms 후보

SafetyTask와 VcuControlTask는 logging/UI 같은 부가 기능 때문에 늦어지지 않게 한다.

---

# 4. 최소 전자기초

## 4.1 전압 / GND

- 센서 Supply Voltage 확인
- MCU 입력 허용전압 확인
- Logic Level 확인
- 공통 GND 확인

## 4.2 GPIO

사용 예:
- Gear Button
- E-Stop
- Direction
- LED
- Ultrasonic Trigger/Echo

## 4.3 ADC

```text
Sensor Voltage
→ ADC Raw
→ Calibration
→ Physical Value
```

## 4.4 PWM / Timer

사용 예:
- DC Motor PWM
- RC Servo
- Encoder/Hall
- LED brightness
- Ultrasonic Echo timing

## 4.5 UART / I2C / SPI

- UART: Debug
- I2C: AS5600 / sensor
- SPI: 외장 CAN Controller 후보

## 4.6 CAN / CAN FD

```text
MCU CAN/FDCAN
→ CAN Transceiver
→ CANH / CANL
```

알아야 할 것:
- CAN ID
- Payload
- Periodic / Event
- Timeout
- Heartbeat
- Termination

## 4.7 LIN

```text
MCU UART/LIN
→ LIN Transceiver
→ LIN Bus
```

---

# 5. RTOS 공통 규칙

## Task

Task를 기능 이름만으로 만들지 말고 역할과 실행조건을 분명히 한다.

좋은 예:
```text
ControlTask
Trigger: every 10 ms
Input: latest speed command, RPM
Output: PWM
```

애매한 예:
```text
Task1
Task2
```

## Queue

```text
Producer Task
→ Queue
→ Consumer Task
```

CAN RX frame, sensor result, UI command처럼 ownership이 바뀌는 데이터에 적합하다.

## ISR

ISR에서 하지 않는 것:
- 긴 계산
- blocking call
- printf
- 화면 처리
- PID 전체 계산

## Mutex

공유 자원이 정말 필요한 경우에만 사용한다.

예:
- 공용 SPI bus
- 여러 Task가 같은 CAN TX API를 직접 호출하는 구조

가능하면 한 Task가 자원의 Owner가 되고 Queue로 요청을 받는 구조가 더 단순하다.

## Watchdog

추천 구조:

```text
Critical Tasks
→ health flag / heartbeat
→ HealthTask
→ all healthy?
   ├ Yes → IWDG refresh
   └ No  → watchdog reset 허용 / fault log
```

---

# 6. 개발 순서

```text
1. 내 역할 이해
2. SPECIFICATION 작성
3. ARCHITECTURE 작성
4. Datasheet / Pin / Wiring 확인
5. Board Bring-up
6. Peripheral 하나 단독 시험
7. FreeRTOS Scheduler 실행
8. Task / Queue / ISR 구조로 기능 분리
9. 정상 기능 시험
10. Fault / Timeout 시험
11. RTOS Timing / Stack / Queue 확인
12. TEST_REPORT 작성
13. CAN / LIN 통합
```

처음부터 RTOS Task 10개를 만들지 않는다. Hardware가 하나씩 동작하는지 확인한 후 필요한 Task만 추가한다.

---

# 7. Stage 1 공통 PASS 기준

- [ ] Build / Flash / Debug 가능
- [ ] 실제 Pin/Wiring 기록
- [ ] FreeRTOS Scheduler 정상 시작, MCU Node 해당 시
- [ ] 각 Task가 정상 실행되는지 확인
- [ ] Input Raw 또는 Dummy Data 확인
- [ ] Physical/Logical 값 생성
- [ ] Output이 있으면 안전한 범위에서 단독 시험
- [ ] Invalid / Timeout / Disconnect 시험
- [ ] Task period 또는 event flow 확인
- [ ] Stack overflow / Queue overflow가 없는지 확인
- [ ] 다음 단계 CAN/LIN Input/Output 정의
- [ ] TEST_REPORT에 증거 저장

---

# 8. 팀원끼리 데이터를 넘길 때

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
| Timeout Action | VCU policy |
| RTOS Delivery | Queue / latest-value model / event |

CAN Signal과 RTOS Queue는 같은 것이 아니다.

```text
CAN frame
→ CanRxTask
→ Decode
→ Queue / Repository
→ Application Task
```

처럼 Node 내부와 차량 네트워크를 구분한다.

---

# 9. 막혔을 때 질문 양식

```text
Board:
RTOS / Bare-metal:
Task:
Priority:
Period / Trigger:
Sensor / Actuator:
Interface:
Expected:
Actual:
Log:
Already Tried:
```

---

# 10. 자기 역할을 이해했는지 확인

1. 나는 무엇을 입력받는가?
2. 무엇을 계산/판단하는가?
3. 어떤 Task가 이 일을 하는가?
4. 결과를 어디로 넘기는가?
5. 어떤 Task가 가장 중요하고 왜 그런가?
6. Interrupt가 오면 ISR과 Task 중 누가 무엇을 하는가?
7. 내 기능이 멈췄는지 어떻게 감지하는가?

상세 Task 후보와 데이터 Owner는 [PROJECT_REFERENCE.md](../system/PROJECT_REFERENCE.md)를 참고한다.
