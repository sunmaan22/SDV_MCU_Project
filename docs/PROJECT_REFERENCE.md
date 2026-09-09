# Project Reference

> 현재 프로젝트에서 **어떤 보드가 무엇을 맡고, 어떤 센서/데이터를 소유하고, 어떤 RTOS Task 구조를 기본으로 하는지** 한 곳에서 확인하는 문서다.  
> 핀 번호, CAN ID, 주기, 임계값, FreeRTOS numeric priority는 실제 보드/시험 후 확정하며 미정값은 `TBD`로 둔다.

# 1. 현재 전체 구조

```text
Front Camera ─┐
              ├→ Raspberry Pi Vision/HPC ─────────────┐
Rear Camera ──┘                                      │
                                                     │ CAN FD
Ultrasonic → STM32 #1 ───────────────────────────────┤
                                                     ├→ STM32 #5 VCU
STM32H735 Cockpit ←───────────────────────────────────┤       │
                                                     │       ↓
STM32 #3 Body Gateway ←───────────────────────────────┘  STM32 #2 Drive/Steer
        ↕ LIN
STM32 #4 Body LIN Slave
        ├ Ambient Sensor
        └ Lighting
```

## Board Mapping / Execution Model

| Node | Hardware | 역할 | 실행 환경 |
|---|---|---|---|
| A | STM32 #1 + Ultrasonic | Parking distance perception | FreeRTOS |
| B | STM32H735 + TouchGFX | Cluster + IVI | FreeRTOS + TouchGFX |
| C | STM32 #2 + Motor Driver + Motor + Servo | Drive + Steering control | FreeRTOS |
| D-Gateway | STM32 #3 + CAN/LIN Transceiver | CAN FD ↔ LIN Gateway, LIN Master | FreeRTOS |
| D-Slave | STM32 #4 + LIN Transceiver | Ambient + Lighting LIN Slave | FreeRTOS 기본 |
| E | Raspberry Pi | Front/Rear Camera Vision, HPC services | Linux |
| F | STM32 #5 | VCU, Driver Input, Safety, CAN Integration | FreeRTOS |
| Diagnostics | Raspberry Pi service | DTC History / Logger | Linux service |

> STM32는 FreeRTOS + CMSIS-RTOS2 API를 기본안으로 한다. 실제 MCU 자원이 너무 작은 경우만 Architecture Decision을 남기고 예외를 검토한다.

---

# 2. RTOS 공통 설계 기준

```text
ISR
→ 최소 처리
→ Task Notification / Queue
→ Task에서 실제 계산
```

기본 원칙:
- periodic Task: `osDelayUntil()` / `vTaskDelayUntil()` 계열 사용 권장
- event Task: Queue / Task Notification으로 wake-up
- Task 간 전역변수 직접 공유 최소화
- Mutex보다 single-owner + Queue 구조 우선
- Control/Safety Task에서 blocking log 금지
- Static allocation 또는 startup 이후 heap 사용 최소화 권장
- Stack watermark / Queue high-water / overrun을 시험
- IWDG + HealthTask 구조 권장

## 우선순위 방향

```text
Safety / Control
> Critical Communication RX
> Sensor / State / Gateway
> UI / Periodic Status
> Diagnostics / Logging
```

정확한 숫자는 Node별 Timing Test 후 결정한다.

---

# 3. Sensor / Input List

| 영역 | 입력 / 센서 | Owner | Interface 후보 | 상태 |
|---|---|---|---|---|
| Driver | P/R/N/D | VCU | GPIO | 계획 |
| Driver | Accelerator Position | VCU | ADC, Pot/Hall | 후보 |
| Driver | Brake Position | VCU | ADC, Pot/Hall | 후보 |
| Driver | Steering Wheel Angle | VCU | I2C/Analog, AS5600 후보 | 후보 |
| Drive | Motor Encoder / Hall | Drive ECU | Timer/GPIO | 권장 |
| Steering | Actual Steering Feedback | Drive ECU | ADC/I2C | 선택 확장 |
| Parking | Ultrasonic Sensors | Ultrasonic ECU | GPIO/Timer | 계획 |
| Vision | Front Camera | Raspberry Pi | CSI | 계획 |
| Vision | Rear Camera | Raspberry Pi | USB | 계획 |
| Body | Ambient Light Sensor | LIN Slave | ADC/I2C | 계획 |
| Thermal | Motor Temperature | Drive ECU | ADC/I2C | 선택 확장 |
| Battery | Battery Voltage | VCU 또는 지정 Node | ADC measurement circuit | 후보 |
| Battery | Battery Temperature | VCU 또는 지정 Node | ADC/I2C | 후보 |
| Battery | Current Sensor | 지정 Node | ADC/I2C | 선택 확장 |

---

# 4. Data Owner 원칙

| 데이터 | Owner | 주요 Consumer |
|---|---|---|
| Gear / Accelerator / Brake / Steering Input | VCU | Drive, HPC, H735 |
| Final Speed / Steering Request | VCU | Drive + Steering ECU |
| Motor RPM / Vehicle Speed | Drive ECU | VCU, H735, HPC |
| Ultrasonic Distance / Warning | Ultrasonic ECU | VCU, H735, HPC |
| Front/Rear Vision Result | Raspberry Pi HPC | VCU, H735 |
| Ambient Light / Local Lamp Status | Body LIN Slave | Gateway → VCU/H735/HPC |
| CAN↔LIN Gateway Status | Body Gateway | VCU/H735/HPC |
| DTC History DB | Raspberry Pi DTC Manager | H735 / Debug tools |
| 화면 값 | 원본 Node | H735는 Subscriber |

Node 내부에서도 한 데이터의 writer를 가능하면 하나로 둔다.

예:
```text
CanRxTask
→ decode
→ VehicleRepository single writer
→ GuiTask read snapshot
```

---

# 5. 역할별 Input → Process → Output → RTOS

## A. Ultrasonic

```text
Input   : Trigger/Echo 또는 Sensor response
Process : 거리 계산 → 유효성 → 필터 → Warning Level
Output  : distance_mm, valid, warning_level
Target  : VCU / H735 / HPC
Fault   : timeout, invalid range, sensor unavailable
```

### RTOS 구조 후보

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| Timer Capture ISR | Echo edge | ISR | timestamp 저장 후 Task notify |
| `UltrasonicTask` | 20~50 ms 후보 | Normal/High | trigger, distance 계산, validation |
| `CanTxTask` | event / status period | Normal | status CAN 송신 |
| `HealthTask` | 100 ms 후보 | Low | sensor/task health |

```text
Echo ISR
→ Notification
→ UltrasonicTask
→ Measurement Queue / latest state
→ CanTxTask
```

---

## B. H735 Cluster + IVI

```text
Input   : CAN Vehicle / ADAS / Parking / DTC data, Touch
Process : Data Model → Warning/Validity → UI State → Rendering
Output  : Cluster/IVI 화면, User Request
Target  : Driver / CAN Request
Fault   : CAN timeout, invalid data, UI task fault
```

### RTOS 구조 후보

| Task / ISR | Trigger / Period | Priority 방향 | 역할 |
|---|---|---|---|
| FDCAN ISR | frame arrival | ISR | frame enqueue / task notify |
| `CanRxTask` | event | High | CAN decode / model update request |
| `VehicleModelTask` | event / 10~20 ms 후보 | Normal/High | repository, validity, warning state |
| `GuiTask` | TouchGFX tick | Normal | TouchGFX rendering |
| `CommandTxTask` | UI event | Normal | Body/DTC request CAN TX |
| `HealthTask` | 100 ms 후보 | Low | task/queue/stack health |

상세 예시는 [`IVI/ARCHITECTURE.md`](IVI/ARCHITECTURE.md)를 기준으로 한다.

---

## C. Motor + Steering

```text
Input   : Final Speed/Steering Request, Encoder/Hall
Process : Command validation → feedback/control → Motor/Servo mapping
Output  : Motor PWM/DIR, Servo PWM, RPM/Drive Status
Fault   : command timeout, encoder invalid, control output fault
```

### RTOS 구조 후보

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| Encoder ISR | edge/input capture | ISR | count/timestamp + notify |
| `CanRxTask` | event | High | latest VCU command update |
| `ControlTask` | 5~10 ms 후보 | Highest application | speed/steering control, PWM update |
| `FeedbackTask` | 5~10 ms 후보/event | High | RPM/feedback 계산 |
| `StatusTask` | 20~50 ms 후보 | Normal | Drive_Status 송신 |
| `HealthTask` | 50~100 ms 후보 | Low/Normal | command timeout / task health |

ControlTask는 UART printf, blocking CAN TX, 느린 진단 처리에 의존하지 않는다.

---

## D. Lighting + Ambient / LIN-CAN

### Gateway

```text
CAN FD
↕
Gateway STM32
├ CAN Service
├ CAN↔LIN Mapping
├ LIN Schedule
└ Gateway Fault Monitor
↕ LIN
Body LIN Slave
```

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| CAN/LIN ISR | bus event | ISR | notification/queue |
| `CanRxTask` | event | High | CAN command RX |
| `LinScheduleTask` | slot period TBD | High | LIN master schedule |
| `GatewayMappingTask` | event | Normal/High | CAN↔LIN signal mapping |
| `CanTxTask` | event / periodic | Normal | Body_Status TX |
| `HealthTask` | 100 ms 후보 | Low | LIN node timeout / gateway health |

### LIN Slave

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| LIN ISR | frame event | ISR | wake LinRxTask |
| `LinRxTask` | event | High | LIN command/status handling |
| `AmbientTask` | 50~100 ms 후보 | Normal | ambient sampling/filter |
| `LightingTask` | event / 10~20 ms 후보 | Normal/High | lamp state/output |
| `StatusTask` | schedule event | Normal | slave status 준비 |
| `HealthTask` | 100 ms 후보 | Low | sensor/output/task health |

---

## E. HPC + Camera Vision

Pi는 RTOS가 아니라 Linux다.

```text
Pi #1 + Front Camera → Front Vision
Pi #2 + Rear Camera  → Rear Vision
```

최종:
```text
front_vision service ─┐
rear_vision service ──┤
can_service ──────────┤→ vehicle_manager
DTC manager ──────────┤
logger ────────────────┘
```

Architecture에는 Process/Thread, Queue, service dependency, restart 정책을 기록한다.

Raw Camera frame은 Pi 내부에서 처리하고 CAN에는 semantic/control result만 보낸다.

---

## F. VCU + DTC + CAN Integration

```text
Driver Input
ADAS Request
Ultrasonic Warning
ECU Heartbeat/Fault
        ↓
Vehicle State / Gear / Mode
        ↓
Safety & Arbitration
        ↓
Final Speed / Steering Request
```

### RTOS 구조 후보

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| GPIO/ADC/Peripheral ISR | event | ISR | 최소 capture |
| `CanRxTask` | event | High | ADAS/US/Drive/Body status 수신 |
| `DriverInputTask` | 10 ms 후보 | High/Normal | Gear/Pedal/Steering input |
| `SafetyTask` | 5~10 ms/event 후보 | Highest application | E-Stop, heartbeat, critical fault |
| `VcuControlTask` | 10 ms 후보 | High | mode/state/arbitration |
| `CanTxTask` | 20 ms/event 후보 | Normal/High | final command/state TX |
| `DiagnosticTask` | 100 ms/event | Low | DTC/status integration |
| `HealthTask` | 100 ms 후보 | Low | task health + watchdog coordination |

우선순위 기본 방향:
```text
Critical Fault / E-Stop
> Critical Obstacle Stop
> ADAS Safety Request
> Normal Driver / Mode Request
```

RTOS Priority와 차량 Arbitration Priority는 다른 개념이다. 둘을 문서에서 혼동하지 않는다.

---

# 6. Task 간 통신 기본 예

## ISR → Task

```text
Hardware Interrupt
→ xTaskNotifyFromISR / CMSIS equivalent
→ Task wakes
```

## Producer → Consumer

```text
CanRxTask
→ Queue
→ Application Task
```

## Latest-value 데이터

속도 명령처럼 최신값만 중요하면 Queue 깊이를 무한히 늘리는 대신 single latest-value 구조 또는 overwrite queue를 검토한다.

## Event Flags

예:
```text
BIT_CAN_READY
BIT_SENSOR_VALID
BIT_ESTOP
BIT_FAULT
```

상태 동기화에 사용할 수 있다.

---

# 7. Watchdog / Health Monitoring

MCU Node 권장 구조:

```text
Critical Task A ─ health flag ┐
Critical Task B ─ health flag ├→ HealthTask
CanRxTask       ─ health flag ┘      ↓
                              all healthy?
                              ├ Yes → IWDG refresh
                              └ No  → fault / no refresh
```

HealthTask가 단순히 무조건 watchdog을 갱신하면 Task deadlock을 놓칠 수 있다.

검토할 항목:
- task heartbeat
- queue overflow
- stack watermark
- control loop overrun
- communication timeout
- scheduler alive

---

# 8. CAN / LIN Interface 방향

CAN ID와 bit layout은 확정 전이면 `TBD`로 둔다.

| Signal | Owner | Consumer | Unit | Cycle | Timeout |
|---|---|---|---|---|---|
| `Vehicle_Gear` | VCU | All | enum | TBD | TBD |
| `Final_Speed_Request` | VCU | Drive | % or m/s TBD | TBD | TBD |
| `Final_Steering_Request` | VCU | Drive | deg/% TBD | TBD | TBD |
| `Motor_RPM` | Drive | VCU/H735/HPC | rpm | TBD | TBD |
| `Rear_Distance` | Ultrasonic | VCU/H735/HPC | mm | TBD | TBD |
| `ADAS_Warning` | HPC | VCU/H735 | enum | TBD | TBD |
| `Body_Ambient` | Gateway | VCU/H735/HPC | raw/% TBD | TBD | TBD |

LIN 후보:

| Frame | Publisher | 역할 |
|---|---|---|
| `Ambient_Status` | LIN Slave | 조도 상태 |
| `Lamp_Command` | Gateway Master | 조명 명령 |
| `Lamp_Status` | LIN Slave | 실제 조명 상태 |
| `Lamp_Diagnostic` | LIN Slave | Body fault 상태 |

---

# 9. Stage 1 최소 산출물

각 담당자는:

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

를 만든다.

MCU Node의 Stage 1 완료 질문:

```text
Peripheral이 단독으로 동작하는가?
→ Scheduler가 정상 시작하는가?
→ Task가 의도한 주기/이벤트로 실행되는가?
→ Queue/Notification 흐름이 정상인가?
→ 중요한 Task가 deadline을 지키는가?
→ Stack/Queue overflow가 없는가?
→ Fault 시 Watchdog/health 정책이 동작하는가?
```

Pi Node는 RTOS 항목 대신 Linux Process/Thread/Service 동작을 기록한다.
