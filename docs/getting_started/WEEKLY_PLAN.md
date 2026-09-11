# 4주 개발 계획 — RTOS 반영 6역할 기준

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> 원칙: **단독 Bring-up → RTOS Task 구조 → 작은 통신 통합 → 전체 Backbone → 차량 통합** 순서로 간다.

> 2026-09-11 부분 동결: B는 H735G-DK, A/C/D Gateway/D Slave/F는 STM32G431KB 기반 구매 보드로 확정했다.
> Week 1의 Hardware Layer 확정 전에 Gate A, Week 2 첫 ECU pair 통합 전에 Gate B,
> 실제 VCU/Drive 출력 제어 확정 전에 Gate C, Week 2 부하 계측 후 Week 3 통합 baseline 전에 Gate D를 닫는다.
> 주차 경과는 동결 근거가 아니다. 계측용 bench/skeleton은 OPEN 값으로 가능하며 최종 상수와 구분한다.
> 상세 조건: [최상위 명세 §7.1](../system/FINAL_IMPLEMENTATION_SPEC.md#71-단계별-동결-시점).

# 역할

| 담당 | 역할 | 실행 환경 |
|---|---|---|
| A | Ultrasonic / 인지 | STM32 + FreeRTOS |
| B | H735 Cluster + IVI / UI | STM32H735 + FreeRTOS + TouchGFX |
| C | Motor + Steering / 제어 | STM32 + FreeRTOS |
| D | Lighting + Ambient / LIN-CAN | STM32 Gateway + STM32 LIN Slave + FreeRTOS |
| E | HPC + Front/Rear Camera Vision | Raspberry Pi Linux |
| F | VCU + DTC + CAN Integration | STM32 + FreeRTOS |

---

# Week 1 — Specification + Architecture + Bring-up + RTOS Skeleton

모든 담당자는 자기 폴더에:

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

를 만든다.

MCU 담당은 `ARCHITECTURE.md`에 최소 다음을 작성한다.

```text
Task Table
ISR Map
Queue / Notification / Event
Priority 방향
Watchdog / Health 구조
```

## 역할별 목표

| 담당 | Week 1 목표 |
|---|---|
| A | Ultrasonic 1개 측정 → Timer ISR + UltrasonicTask 구조 |
| B | TouchGFX Dummy UI → GuiTask + VehicleModelTask + CanRxTask skeleton |
| C | PWM/DIR/Encoder/Servo → ControlTask + FeedbackTask skeleton |
| D | Ambient/Lighting 단독 + Gateway/Slave FreeRTOS skeleton + LIN 기본 통신 |
| E | Pi #1 Front / Pi #2 Rear camera capture, Linux service 구조 초안 |
| F | Driver Input + VCU state dummy → SafetyTask/VcuControlTask/CanRxTask skeleton |

## Week 1 PASS

- [ ] 모든 MCU Build/Flash/Run
- [ ] FreeRTOS Scheduler 정상 시작, MCU Node
- [ ] 최소 2개 이상 Task가 의도대로 실행됨
- [ ] ISR에서 긴 연산 없이 Task로 넘기는 구조 확인
- [ ] 실제 Wiring/Pin 기록
- [ ] 정상 Input/Output 시험
- [ ] Task Stack overflow 없음
- [ ] CAN/LIN 후보 Signal 정의

---

# Week 2 — Local 기능 완성 + RTOS 안정화 + 작은 통신 통합

## CAN Pair

```text
F VCU ↔ C Drive/Steering
F VCU ↔ A Ultrasonic
F VCU ↔ B H735
```

순차적으로 붙인다.

## LIN

```text
D Gateway LIN Master ↔ D LIN Slave
```

## 역할별

| 담당 | Week 2 목표 |
|---|---|
| A | 여러 Ultrasonic, filtering, CAN status, queue/notification 검증 |
| B | CAN RX → Queue → Vehicle Model → TouchGFX 연결 |
| C | CAN command + timeout, ControlTask period 측정, RPM feedback |
| D | LinScheduleTask + CAN↔LIN MappingTask + Slave LightingTask |
| E | Lane/Object baseline, Rear Parking baseline, result queue/interface |
| F | CAN Matrix v0.1, Heartbeat, SafetyTask, VCU CAN arbitration, DTC table v0.1 |

## RTOS 공통 시험

- [ ] Task period 측정
- [ ] event task wake-up 확인
- [ ] Queue overflow 없음
- [ ] Stack high-water mark 기록
- [ ] 중요 Task에서 blocking printf 제거
- [ ] Task starvation 여부 확인

---

# Week 3 — 전체 CAN Backbone + End-to-End + Health Monitoring

## Drive Path

```text
DriverInputTask
→ VcuControlTask
→ CAN
→ Drive CanRxTask
→ ControlTask
→ Motor/Servo
→ StatusTask
→ H735
```

## Front ADAS

```text
Front Camera
→ Pi Vision
→ ADAS Request
→ VCU CanRxTask
→ Safety/VcuControlTask
→ Drive ControlTask
```

## Parking

```text
A UltrasonicTask ──────────┐
                           ├→ F VCU Safety/Control → Final Stop/Speed
E Rear Parking Vision ─────┘

A/E Status → B VehicleModelTask → H735 Parking Screen
```

## Body

```text
B/F Body Request
→ CAN
→ D CanRxTask
→ GatewayMappingTask
→ LinScheduleTask
→ LIN Slave LightingTask
```

## DTC / Health

```text
Local Fault
→ DiagnosticTask / HealthTask
→ DTC Event
→ Pi DTC Manager
→ H735 Diagnostics
```

MCU Node는 가능한 경우 다음까지 구현한다.

```text
Critical Task heartbeat
→ HealthTask
→ all healthy
→ IWDG refresh
```

Week 3에서는 Task를 일부러 멈추게 하는 시험을 바로 강제하지 않고, 먼저 Health flag/Watchdog 구조가 논리적으로 맞는지 검토한다.

---

# Week 4 — 최종 Pi 통합 + RTOS/차량 검증

## Vision

개발용 Pi 두 대의 코드를 최종 Pi 한 대로 통합한다.

```text
Gear D → Front Vision active
Gear R → Rear Parking Vision active
```

## 차량 기능 시험

- [ ] Gear P/R/N/D
- [ ] Accelerator / Brake
- [ ] Steering input → Servo
- [ ] Encoder RPM
- [ ] Ultrasonic warning/stop
- [ ] Front Vision request
- [ ] Rear Vision + Ultrasonic parking
- [ ] H735 Cluster/IVI
- [ ] Ambient → LIN → CAN
- [ ] CAN → LIN → Lighting
- [ ] Heartbeat timeout
- [ ] Sensor disconnect
- [ ] Camera service failure
- [ ] DTC Active/History/Clear

## RTOS 측정

MCU별로 가능한 항목을 기록한다.

- ControlTask period / jitter
- SafetyTask period / response latency
- CAN RX → Application Task latency
- LIN Schedule period / jitter
- GUI/CAN 동시 부하에서 H735 응답성
- Stack high-water mark
- Queue maximum occupancy
- CPU load, 측정 가능 시
- Task overrun count
- Watchdog/Health 상태

정확한 목표값은 각 Specification에 정의한 뒤 측정 결과와 비교한다.

---

# 최종 Demo 흐름

```text
Power ON
→ FreeRTOS Scheduler / Linux services start
→ MCU Health / Heartbeat
→ H735 Cluster READY
→ Gear D
→ Front Vision
→ ADAS Request
→ VCU Safety / Arbitration Task
→ Drive ControlTask
→ Motor/Steering
→ Gear R
→ Rear Vision + Ultrasonic
→ Parking Warning / Stop
→ Ambient 변화
→ LIN Slave → Gateway → CAN → H735
→ Lamp Request
→ CAN → Gateway → LIN → Lamp
→ Sensor/Camera/Communication Fault
→ DTC → Pi → H735
```

최종 목표는 단순히 RTOS를 사용했다는 것이 아니라 **각 Task의 책임, 우선순위, 데이터 전달, Fault 대응을 설명하고 실제 Timing을 측정할 수 있는 시스템**을 만드는 것이다.
