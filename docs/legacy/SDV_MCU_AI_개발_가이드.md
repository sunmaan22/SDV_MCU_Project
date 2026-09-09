# SDV MCU 프로젝트 문서 작성 및 AI 개발 가이드

이 프로젝트는 각 담당자가 자기 기능을 개발하지만, 최종적으로는 하나의 차량 시스템으로 통합한다.

핵심은 단순하다.

> **각 담당자는 최종 명세서를 기준으로 자기 영역을 개발하고, 자기 MCU 내부 판단은 직접 수행하되 차량 전체의 최종 판단은 VCU가 담당한다.**

AI를 사용할 때도 이 구조를 깨지 않는 것이 가장 중요하다. AI한테 그냥 “코드 짜줘”라고 던지면 각자 다른 나라의 교통법으로 자동차를 만들 수 있다.

---

# 1. 개발 전에 반드시 봐야 하는 문서

각 담당자는 아래 순서로 문서를 확인한다.

```text
1. docs/FINAL_IMPLEMENTATION_SPEC.md
2. 자기 담당 폴더 README.md
3. 자기 담당 SPECIFICATION.md
4. 자기 담당 ARCHITECTURE.md
5. 실제 부품 Datasheet
6. Board Schematic / Pin Map
7. 구현 후 TEST_REPORT.md
```

가장 중요한 문서는 다음이다.

```text
docs/FINAL_IMPLEMENTATION_SPEC.md
```

이 문서는 프로젝트 전체의 **최상위 구현 기준**이다.

역할별 문서와 내용이 충돌하면 다음 우선순위를 따른다.

```text
FINAL_IMPLEMENTATION_SPEC.md
>
SPECIFICATION.md
>
ARCHITECTURE.md
>
README.md
```

GitHub:

https://github.com/sunmaan22/SDV_MCU_Project/blob/main/docs/FINAL_IMPLEMENTATION_SPEC.md

전체 문서:

https://github.com/sunmaan22/SDV_MCU_Project/tree/main/docs

---

# 2. 각 담당자가 작성해야 하는 문서

각 담당자는 자기 폴더에서 기본적으로 다음 문서를 관리한다.

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

의미는 다음과 같다.

```text
SPECIFICATION
= 무엇을 구현할 것인가

ARCHITECTURE
= 어떻게 구현할 것인가

CODE
= 실제 구현

TEST_REPORT
= 실제로 동작했는지 검증
```

---

# 3. SPECIFICATION.md에는 무엇을 작성하는가

SPECIFICATION은 기능 요구사항을 작성한다.

예를 들어 Ultrasonic 담당이라면:

```text
- 초음파 센서로 거리를 측정한다.
- 센서 timeout을 검출한다.
- 측정값의 valid 여부를 판단한다.
- SAFE / WARNING / CRITICAL을 판단한다.
- Ultrasonic_Status를 생성한다.
- CAN FD를 통해 상태를 전달한다.
```

중요한 점은 **근거 없는 숫자를 임의로 확정하지 않는 것**이다.

잘못된 예:

```text
CRITICAL = 20 cm
Task Period = 10 ms
Timeout = 50 ms
```

아직 실제 측정을 하지 않았다면 다음처럼 작성한다.

```text
CRITICAL threshold
→ Stage 1 측정 후 확정

Task Period
→ 실행시간/Jitter 측정 후 확정

Timeout
→ Message Cycle 확정 후 결정
```

---

# 4. ARCHITECTURE.md에는 무엇을 작성하는가

ARCHITECTURE는 실제 코드 구조를 설명한다.

예:

```text
Timer Input Capture ISR
        ↓
UltrasonicTask
        ↓
PerceptionTask
        ↓
CanTxTask
```

다음 내용을 포함한다.

```text
- Task 구조
- Task별 역할
- ISR 역할
- Queue / Notification / Event
- Sensor Driver
- CAN 송수신 구조
- Data Flow
- Health Monitoring
- Watchdog
- Module 구조
```

기본 원칙:

```text
ISR
→ 최소 처리

계산
→ Task

CAN
→ Control과 분리

UI
→ Control과 분리

공유 전역변수
→ 최소화
```

---

# 5. TEST_REPORT.md에는 무엇을 작성하는가

TEST_REPORT에는 예상값이 아니라 **실제 측정 결과**를 기록한다.

예:

```text
실제 거리 : 100 mm
측정 거리 : 103 mm

실제 거리 : 300 mm
측정 거리 : 295 mm

Sensor Disconnect
→ valid = false 확인

Task Period
→ 10.02 ms

Max Jitter
→ 0.15 ms
```

즉:

```text
Datasheet
= 이론 / 보장 범위

TEST_REPORT
= 우리 시스템에서 실제로 나온 값
```

이다.

---

# 6. 각 MCU는 어디까지 판단하는가

각 ECU는 자기 담당 영역의 **로컬 판단**을 수행한다.

전체 차량 행동에 대한 최종 판단은 VCU가 한다.

```text
A Ultrasonic
→ 거리 계산
→ valid 판단
→ SAFE / WARNING / CRITICAL 판단

B HMI
→ 무엇을 화면에 표시할지 판단
→ Warning UI 우선순위 판단

C Motor + Steering
→ Motor PWM 계산
→ Servo 출력 계산
→ PID
→ Command timeout 판단

D Body
→ Ambient 상태
→ Lamp 상태
→ LIN timeout / fault 판단

E Vision
→ Lane
→ Object
→ Parking 상황
→ ADAS_Request 생성

F VCU
→ 모든 입력을 종합
→ 최종 차량 행동 판단
```

---

# 7. 판단 구조

전체 구조는 다음과 같다.

```text
Sensor / Camera / Input
        ↓
각 ECU
Local Processing
        ↓
Local Decision
        ↓
Status / Warning / Request
        ↓ CAN FD
VCU
        ↓
Final Arbitration
        ↓
Final_Drive_Command
        ↓
Drive ECU
```

예를 들어 Ultrasonic은:

```text
Echo
↓
거리 계산
↓
Filtering
↓
valid 판단
↓
SAFE / WARNING / CRITICAL
↓
Ultrasonic_Status
↓
VCU
```

여기까지 A가 한다.

하지만 다음은 하면 안 된다.

```text
CRITICAL
→ Motor PWM = 0
```

이 최종 행동은 VCU가 결정한다.

---

# 8. VCU가 최종 판단하는 이유

각 MCU가 자기 마음대로 차량 명령까지 만들면 충돌한다.

예:

```text
Ultrasonic ECU
→ STOP

Vision ECU
→ GO

Driver
→ Accelerator 70%

Drive ECU
→ 누구 말을 따라야 하지?
```

그래서 최종 판단은 VCU 하나에 둔다.

현재 우선순위 개념은 다음과 같다.

```text
E-Stop / Critical Fault
        >
Ultrasonic Critical
        >
ADAS Safety Request
        >
Normal Driver Input
```

최종적으로 VCU가 다음을 생성한다.

```text
Final Speed
Final Steering
Drive Enable
Gear / Direction
Body Command
```

---

# 9. AI로 문서 작성할 때 무엇을 보여줘야 하는가

AI에게 단순히:

```text
내 ECU 문서 작성해줘
```

라고 하지 않는다.

반드시 다음을 같이 기준으로 제공한다.

```text
FINAL_IMPLEMENTATION_SPEC.md
+
본인 README.md
+
본인 SPECIFICATION.md
+
본인 ARCHITECTURE.md
+
실제 Datasheet
+
Pin Map / Schematic
```

AI에게 다음처럼 요청한다.

```text
이 프로젝트의 최상위 기준은
FINAL_IMPLEMENTATION_SPEC.md이다.

공통 Message 이름,
Publisher / Consumer,
역할 경계,
Safety Rule은 변경하지 마라.

내 담당은 [A/B/C/D/E/F]이다.

첨부한 Datasheet와 현재 문서를 기준으로
SPECIFICATION과 ARCHITECTURE를 검토하고 수정해라.

실측이 필요한 값은 임의로 확정하지 말고
MEASUREMENT REQUIRED로 표시해라.

프로젝트 Owner가 결정해야 하는 공통값은
OWNER DECISION REQUIRED로 표시해라.
```

---

# 10. AI로 코드 개발할 때 기본 순서

처음부터 전체 코드를 한 번에 만들지 않는다.

다음 순서로 개발한다.

```text
1. Hardware Driver
2. Sensor / Actuator 단독 동작
3. Raw Data 확인
4. Local Processing
5. Timeout / Fault
6. FreeRTOS Task 구조
7. Status / Request 구조
8. CAN 송수신
9. ECU 간 통합
10. VCU Arbitration
11. 전체 시스템 통합
```

예를 들어 Ultrasonic은:

```text
GPIO Trigger
↓
Timer Input Capture
↓
Echo 측정
↓
거리 계산
↓
Timeout
↓
Filtering
↓
Warning 판단
↓
FreeRTOS Task
↓
Ultrasonic_Status
↓
CAN
```

순서로 진행한다.

---

# 11. AI에게 코드 작성 요청 예시

예를 들어 Ultrasonic 담당이면:

```text
STM32 + FreeRTOS 프로젝트다.

최상위 기준:
FINAL_IMPLEMENTATION_SPEC.md

담당:
Ultrasonic Perception ECU

Hardware:
[STM32 모델]
[Ultrasonic Sensor 모델]

Datasheet:
첨부

Architecture:
Timer Input Capture ISR
→ UltrasonicTask
→ PerceptionTask
→ CanTxTask

이번 단계에서는 CAN을 구현하지 말고
Sensor 1개 단독 동작 코드만 작성해라.

요구사항:
- Trigger
- Echo Input Capture
- Timeout
- distance_mm 계산
- valid flag
- UART debug
- ISR에서는 timestamp만 처리
- HAL + CMSIS-RTOS2 사용

임의의 Pin이나 Timer를 생성하지 말고
첨부한 Pin Map을 기준으로 작성해라.
```

기능이 성공한 뒤 다음 기능을 요청한다.

---

# 12. AI 코드 생성 후 반드시 확인해야 하는 것

AI 코드가 생성됐다고 바로 믿지 않는다.

반드시 다음을 확인한다.

```text
- 실제 Pin과 일치하는가
- Timer Channel이 맞는가
- ADC Channel이 맞는가
- MCU가 FDCAN을 지원하는가
- Voltage가 맞는가
- Datasheet 범위를 넘지 않는가
- ISR 안에서 blocking하지 않는가
- Task가 무한정 멈추지 않는가
- Timeout이 존재하는가
- valid 처리가 있는가
- Queue overflow 처리가 있는가
- Motor/Servo safe state가 있는가
```

AI는 존재하지 않는 Peripheral도 상당히 당당하게 만들어낼 수 있으므로 Datasheet 확인은 필수다.

---

# 13. 지금 바로 결정해야 하는 것

코드 시작 전에 다음 Hardware는 결정해야 한다.

```text
- STM32 모델
- Ultrasonic Sensor
- Motor
- Motor Driver 후보
- Servo
- Encoder / Hall Sensor
- Ambient Sensor
- Camera
- CAN FD Transceiver
- LIN Transceiver
- Raspberry Pi CAN FD Interface
```

Hardware가 확정된 다음:

```text
GPIO
Timer
ADC
UART
I2C
SPI
FDCAN
```

을 결정한다.

---

# 14. Datasheet를 보고 결정해야 하는 것

다음은 Datasheet와 Schematic을 보고 결정한다.

```text
Pin
Peripheral
Voltage
Current
Timer Channel
ADC Channel
FDCAN 지원
PWM 가능 여부
Memory
Interface
```

예:

```text
MCU 선정
↓
Datasheet
↓
Board Schematic
↓
CubeMX
↓
Pin 확정
```

---

# 15. Stage 1 실측 후 결정하는 것

다음 값들은 실제로 동작시킨 뒤 결정한다.

```text
Sensor Threshold
Filtering
Calibration
Motor PWM Limit
Servo Limit
Vision Resolution
Vision FPS
Task Period
Task Priority
Stack Size
Queue Depth
```

예를 들어 Ultrasonic:

```text
10 cm
20 cm
30 cm
50 cm
```

에서 실제 측정값을 기록한 후:

```text
SAFE
WARNING
CRITICAL
```

Threshold를 결정한다.

---

# 16. Motor Driver 결정 기준

Motor Driver를 먼저 확정하지 않는다.

순서:

```text
Motor 선정
↓
Motor Voltage
Rated Current
Stall Current
확인
↓
Motor Driver Datasheet 확인
↓
Driver 최종 선정
```

현재 TB6612FNG는 후보로만 사용한다.

---

# 17. Servo Limit 결정 기준

Servo Datasheet의 최대 각도를 그대로 사용하지 않는다.

실제 기구를 연결한다.

```text
Servo
↓
Linkage
↓
Wheel
```

그 후:

```text
Left Mechanical Limit
Center
Right Mechanical Limit
```

을 측정한다.

실제 한계보다 약간 안쪽을 Software Safe Limit으로 사용한다.

---

# 18. Vision 결정 기준

Vision은 실제 Pi 성능을 보고 결정한다.

측정할 것:

```text
FPS
Inference Latency
CPU Usage
RAM Usage
Temperature
Dropped Frames
Queue Backlog
```

이 결과를 보고:

```text
Resolution
FPS
Model
Processing 방식
```

을 확정한다.

---

# 19. FreeRTOS에서 먼저 결정할 것과 나중에 결정할 것

Task 구조는 먼저 결정한다.

예:

```text
CanRxTask
ControlTask
FeedbackTask
HealthTask
```

하지만 다음 값은 실측 후 확정한다.

```text
Task Period
Priority
Stack
Queue Depth
```

측정:

```text
Execution Time
Jitter
Stack High-Water
Queue Occupancy
CPU Load
```

---

# 20. CAN 관련 값은 언제 결정하는가

논리 Message와 Publisher는 지금 정할 수 있다.

예:

```text
Ultrasonic_Status
Vision_Status
ADAS_Request
Final_Drive_Command
Drive_Status
Body_Command
Body_Status
Vehicle_State
```

하지만 다음 값은 Stage 1 이후 확정한다.

```text
CAN ID
DLC
Bit Position
Scale
Offset
Message Cycle
Timeout
```

---

# 21. CAN Cycle과 Timeout 결정 순서

순서는 다음이다.

```text
Sensor / Control Update Period 확인
↓
Message Cycle 결정
↓
실제 Jitter 측정
↓
허용 가능한 Message Miss 횟수 결정
↓
Timeout 결정
```

Timeout부터 먼저 정하지 않는다.

---

# 22. LIN 관련 결정

Stage 2 전까지 다음을 확정한다.

```text
LIN Frame ID
Publisher
Subscriber
Checksum
Schedule Table
Slot Period
Timeout
CAN ↔ LIN Mapping
```

Gateway가 LIN Master Schedule을 관리한다.

---

# 23. DTC 결정

먼저 Fault를 정의한다.

예:

```text
Sensor Timeout
CAN Timeout
LIN Node Timeout
Camera Failure
Encoder Invalid
Motor Feedback Fault
Queue Overflow
Heartbeat Timeout
```

그리고 각 Fault가 시스템에 미치는 영향을 보고:

```text
INFO
WARNING
CRITICAL
```

등의 Severity를 결정한다.

마지막으로:

```text
DTC Code
Confirmation Rule
Clear Rule
Safe Action
```

을 결정한다.

---

# 24. VCU에서 결정해야 하는 것

VCU는 다음을 최종적으로 결정한다.

```text
Vehicle State Machine

INIT
READY
P
R
N
D
FAULT
```

그리고:

```text
READY 조건
Drive Enable 조건
Brake vs Accelerator
D ↔ R 전환 조건
E-Stop Recovery
Ultrasonic Action
ADAS Arbitration
Communication Timeout Action
Critical DTC Safe Action
```

을 결정한다.

---

# 25. 각 값은 무엇을 보고 결정하는가

```text
Pin
→ Datasheet / Schematic / CubeMX

Voltage / Current
→ Datasheet

Motor Driver
→ Motor Stall Current

Sensor Threshold
→ 실제 측정

Calibration
→ 실제 측정

Servo Limit
→ 실제 기구

Vision FPS
→ Pi CPU / RAM / Temperature / Latency

Task Period
→ Execution Time / Deadline

Task Priority
→ Deadline / Blocking 관계

Stack
→ Stack High-Water

CAN Cycle
→ Sensor / Control Update Period

Timeout
→ Cycle + Jitter

DTC Severity
→ 시스템에 미치는 영향

VCU Arbitration
→ 각 ECU 출력과 Safety Requirement
```

---

# 26. 전체 개발 순서

```text
Architecture 이해
↓
Hardware 선정
↓
Datasheet 확인
↓
Pin / Peripheral 결정
↓
SPECIFICATION 작성
↓
ARCHITECTURE 작성
↓
Stage 1 단독 개발
↓
실제 측정
↓
Threshold / Calibration / Timing 결정
↓
CAN / LIN Interface Freeze
↓
Stage 2 ECU 간 통합
↓
VCU Arbitration
↓
DTC / Heartbeat
↓
전체 시스템 통합
↓
TEST_REPORT
↓
최종 튜닝
```

---

# 27. 담당별 한 줄 정리

```text
A
센서를 읽고 거리 상태까지 판단해서 보낸다.

B
차량 상태를 보여주고 사용자 요청을 만든다.

C
VCU의 최종 명령을 Motor / Servo 출력으로 바꾼다.

D
Body 상태를 관리하고 CAN ↔ LIN을 연결한다.

E
Camera를 분석하고 Vision 상태와 ADAS 요청을 만든다.

F
모든 정보를 종합해서 차량의 최종 행동을 결정한다.
```

---

# 28. 가장 중요한 규칙

```text
내 ECU 내부 구현
→ 담당자가 결정

ECU와 ECU 사이 Interface
→ FINAL_IMPLEMENTATION_SPEC 기준

실제 성능과 관련된 값
→ 실측 후 결정

최종 차량 행동
→ VCU가 결정
```

---

# 29. AI 사용 핵심 규칙

> **AI에게는 최종 명세서, 자기 역할 문서, Datasheet, Pin Map을 함께 제공한다. AI는 자기 담당 내부 구현을 제안하고 코드를 작성할 수 있지만, 공통 Message 이름, Publisher/Consumer, 역할 경계, Safety Rule을 임의로 변경하면 안 된다. 실측이 필요한 값은 추측으로 확정하지 않고 측정 후 반영한다.**

GitHub 최종 명세서:

https://github.com/sunmaan22/SDV_MCU_Project/blob/main/docs/FINAL_IMPLEMENTATION_SPEC.md

GitHub 전체 문서:

https://github.com/sunmaan22/SDV_MCU_Project/tree/main/docs
