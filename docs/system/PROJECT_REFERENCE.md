# Project Reference

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> 현재 프로젝트에서 **어떤 보드가 무엇을 맡고, 어떤 센서/데이터를 소유하고, 어떤 RTOS Task 구조를 기본으로 하는지** 한 곳에서 확인하는 문서다.  
> 핀 번호, CAN ID, 주기, 임계값, FreeRTOS numeric priority는 실제 보드/시험 후 확정하며 미정값은 `TBD`로 둔다.

> **2026-09-15 범위 변경 (1차):** Rear Camera/Rear Vision/주차 Vision, Ambient Sensor, Encoder/Hall 실측 Feedback을 삭제했다. 충돌주의는 Ultrasonic 4방향(FL/FR/RL/RR) 전용, Driver 입력(RF/가변저항)은 C가 읽어 `Driver_Input`으로 발행한다.
>
> **2026-09-15 범위 변경 (2차):** Gear/E-Stop 물리 입력도 F에서 C로 이전했다. F는 Driver/Gear/E-Stop 입력용 GPIO를 갖지 않는다. Pi DTC Manager(History DB)는 삭제했다 — `DTC_Event`는 B(IVI)가 각 ECU로부터 직접 CAN 구독해 실시간(Active만) 표시한다. 근거: [`FINAL_IMPLEMENTATION_SPEC.md`](FINAL_IMPLEMENTATION_SPEC.md).

# 1. 현재 전체 구조

```text
Front Camera → Raspberry Pi Vision/HPC ────────────────┐
                                                        │ CAN FD
Ultrasonic(4방향) → STM32 #1 ───────────────────────────┤
                                                        ├→ STM32 #5 VCU (물리 GPIO 없음)
STM32H735 Cockpit ←──────────────────────────────────────┤       │
                                                        │       ↓
STM32 #3 Body Gateway ←──────────────────────────────────┘  STM32 #2 Drive/Steer
        ↕ LIN                                                    ↑
STM32 #4 Body LIN Slave                                RF 수신기/가변저항 + Gear + E-Stop
        └ Lighting (헤드램프 밝기/턴/브레이크)
```

## Board Mapping / Execution Model

| Node | Hardware | 역할 | 실행 환경 |
|---|---|---|---|
| A | STM32 #1 + Ultrasonic | Collision distance perception (4방향 FL/FR/RL/RR) | FreeRTOS |
| B | STM32H735 + TouchGFX | Cluster + IVI + DTC 실시간 표시 | FreeRTOS + TouchGFX |
| C | STM32 #2 + Motor Driver + Motor + Servo + RF 수신기/가변저항 + Gear/E-Stop | Drive + Steering control, Driver Input(gear/estop_status 포함), E-Stop 로컬 즉시 차단 | FreeRTOS |
| D-Gateway | STM32 #3 + CAN/LIN Transceiver | CAN FD ↔ LIN Gateway, LIN Master | FreeRTOS |
| D-Slave | STM32 #4 + LIN Transceiver | Lighting LIN Slave | FreeRTOS 기본 |
| E | Raspberry Pi | Front Camera Vision(COCO), HPC services | Linux |
| F | STM32 #5 | VCU, Safety, CAN Integration (물리 GPIO 없음) | FreeRTOS |

Pi DTC Manager(History DB)는 삭제됐다. Diagnostics는 별도 Node가 아니며, `DTC_Event`는 각 ECU가 직접 발행하고 B가 실시간 구독·표시한다.

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
| Driver | P/R/N/D (Gear) | Drive ECU(C) | GPIO/ADC | `DEC-HW-026`/`028` 확정 전 |
| Driver | E-Stop | Drive ECU(C) | GPIO/EXTI | `DEC-HW-020`/`027` 확정 전, 로컬 즉시 차단 |
| Driver | Accelerator/Brake/Steering (RF 리모컨 또는 가변저항) | Drive ECU(C) | PWM capture / ADC | `DEC-HW-024` 확정 전 |
| Steering | Actual Steering Feedback | Drive ECU | ADC/I2C | 선택 확장 |
| Collision Warning | Ultrasonic Sensors (FL/FR/RL/RR 4방향 고정) | Ultrasonic ECU | GPIO/Timer | `FROZEN` |
| Vision | Front Camera | Raspberry Pi | CSI | 계획 |
| Thermal | Motor Temperature | Drive ECU | ADC/I2C | 선택 확장 |
| Battery | Battery Voltage | VCU 또는 지정 Node | ADC measurement circuit | 후보 |
| Battery | Battery Temperature | VCU 또는 지정 Node | ADC/I2C | 후보 |
| Battery | Current Sensor | 지정 Node | ADC/I2C | 선택 확장 |

---

# 4. Data Owner 원칙

| 데이터 | Owner | 주요 Consumer |
|---|---|---|
| Gear / E-Stop / Accelerator / Brake / Steering Input (`Driver_Input`) | Drive ECU(C) | VCU, HPC, H735 |
| Final Speed / Steering Request | VCU | Drive + Steering ECU |
| Motor RPM / Vehicle Speed (estimated, 명령값 기반) | Drive ECU | VCU, H735, HPC |
| Ultrasonic Distance / Warning (FL/FR/RL/RR) | Ultrasonic ECU | VCU, H735, HPC |
| Front Vision Result (detected_class/direction) | Raspberry Pi HPC | VCU, H735 |
| Local Lamp Status | Body LIN Slave | Gateway → VCU/H735/HPC |
| CAN↔LIN Gateway Status | Body Gateway | VCU/H735/HPC |
| `DTC_Event` (실시간, 지속 저장 없음) | 각 로컬 Node | H735(구독), VCU(안전 판단) |
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
Input   : FL/FR/RL/RR 4방향 Trigger/Echo
Process : 거리 계산 → 유효성 → 필터 → Warning Level → 초음파 충돌 위험도 판단(A 단독)
Output  : zone_id, distance_mm, valid, warning_level
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
Input   : CAN Vehicle / ADAS / Collision Warning / DTC data, Touch
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
| `CommandTxTask` | UI event | Normal | Body_User_Request CAN TX |
| `HealthTask` | 100 ms 후보 | Low | task/queue/stack health |

상세 예시는 [`IVI/ARCHITECTURE.md`](../ecus/IVI/ARCHITECTURE.md)를 기준으로 한다.

---

## C. Motor + Steering

```text
Input   : RF 수신기/가변저항(Driver Input), Final Drive Command
Process : Driver Input 읽기 → Driver_Input 발행 / Command validation → control → Motor/Servo mapping → 명령값 기반 speed/rpm 추정
Output  : Driver_Input, Motor PWM/DIR, Servo PWM, RPM(estimated)/Drive Status
Fault   : command timeout, driver input invalid, control output fault
```

Encoder/Hall 실측 Feedback은 사용하지 않는다 (`DEC-HW-012` REMOVED) — Motor_RPM/Vehicle_Speed는 명령값(PWM) 기반 추정 함수 결과다.

### RTOS 구조 후보

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| **E-Stop EXTI** | GPIO event | **ISR (최고 우선)** | Motor Driver Enable/STBY 로컬 즉시 차단 (CAN 비의존) |
| Driver Input capture ISR (accel/brake/steering/gear) | PWM capture/ADC event | ISR | raw sample + notify |
| `CanRxTask` | event | High | latest VCU command update |
| `ControlTask` | 5~10 ms 후보 | Highest application | speed/steering control, PWM update, speed/rpm 추정 |
| `DriverInputTask` | 5~10 ms 후보/event | High | RF/가변저항/Gear read → `Driver_Input`(gear/estop_status 포함) 산출 |
| `CanTxTask` | 20~50 ms 후보 + event | Normal | `Driver_Input`/`Drive_Status` 송신 |
| `HealthTask` | 50~100 ms 후보 | Low/Normal | command timeout / task health |

ControlTask는 UART printf, blocking CAN TX, 느린 진단 처리에 의존하지 않는다. E-Stop만 예외적으로 ISR이 안전 액션을 직접 수행한다 (2026-09-15부터 Gear/E-Stop이 F에서 C로 이전).

---

## D. Lighting / LIN-CAN

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
| `LightingTask` | event / 10~20 ms 후보 | Normal/High | lamp state/output (헤드램프 밝기 포함) |
| `StatusTask` | schedule event | Normal | slave status 준비 |
| `HealthTask` | 100 ms 후보 | Low | sensor/output/task health |

---

## E. HPC + Camera Vision

Pi는 RTOS가 아니라 Linux다. Front Camera 1대로 COCO 기반 객체인식만 수행한다 (Rear Camera/Rear Vision/주차 Vision 삭제).

```text
Front Camera → front_vision (COCO Object Detection)
```

최종:
```text
front_vision service ─┐
can_service ───────────┤→ vehicle_manager
logger ──────────────────┘
```

DTC manager(Pi) 서비스는 삭제됐다 — `DTC_Event`는 각 ECU가 직접 CAN 발행하고 B(IVI)가 실시간 구독·표시한다.

Architecture에는 Process/Thread, Queue, service dependency, restart 정책을 기록한다.

Raw Camera frame은 Pi 내부에서 처리하고 CAN에는 semantic/control result만 보낸다.

---

## F. VCU + DTC + CAN Integration

```text
Driver_Input (CAN, C 발행 — accel/brake/steering/gear/estop_status)
ADAS_Request
Ultrasonic_Status (Collision Critical 포함)
ECU Heartbeat/Fault
        ↓
Vehicle State / Gear / Mode
        ↓
Safety & Arbitration (Collision Critical > ADAS_Request)
        ↓
Final Speed / Steering Request
```

F는 Driver/Gear/E-Stop 입력용 GPIO를 갖지 않는다 (2026-09-15부터 Gear/E-Stop도 C 소유, `Driver_Input`으로만 CAN 수신).

### RTOS 구조 후보

| Task / ISR | Trigger / Period 후보 | Priority 방향 | 역할 |
|---|---|---|---|
| `CanRxTask` | event | High | Driver_Input(gear/estop_status 포함)/ADAS/US/Drive/Body status 수신 |
| `SafetyTask` | 5~10 ms/event 후보 | Highest application | estop_status(CAN)/critical fault override |
| `VcuControlTask` | 10 ms 후보 | High | mode/state/arbitration |
| `CanTxTask` | 20 ms/event 후보 | Normal/High | final command/state TX |
| `DiagnosticTask` | 100 ms/event | Low | DTC 실시간 발행 (history 없음) |
| `HealthTask` | 100 ms 후보 | Low | task health + watchdog coordination |

우선순위 기본 방향:
```text
Critical Fault / E-Stop
> Ultrasonic Collision Critical
> ADAS_Request
> Normal Driver / Mode Request
```

Ultrasonic Collision Critical은 `ADAS_Request`보다 항상 우선하며, `ADAS_Request`가 이를 해제/override할 수 없다.

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
| `Driver_Input` | Drive ECU(C) | VCU | TBD | TBD | TBD |
| `Final_Speed_Request` | VCU | Drive | % or m/s TBD | TBD | TBD |
| `Final_Steering_Request` | VCU | Drive | deg/% TBD | TBD | TBD |
| `Motor_RPM` (estimated) | Drive | VCU/H735/HPC | rpm | TBD | TBD |
| `Ultrasonic_Status` (FL/FR/RL/RR) | Ultrasonic | VCU/H735/HPC | mm/flags | TBD | TBD |
| `ADAS_Request` | HPC(E) | VCU | enum | TBD | TBD |
| `Body_Command.headlamp_brightness` | VCU | Gateway | 0-100% | TBD | TBD |

LIN 후보:

| Frame | Publisher | 역할 |
|---|---|---|
| `Lamp_Command` | Gateway Master | 조명 명령 (헤드램프 밝기 포함) |
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
