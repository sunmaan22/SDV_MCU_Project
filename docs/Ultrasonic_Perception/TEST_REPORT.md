# Ultrasonic Perception ECU Test Report

> 목적: `SPECIFICATION.md`의 Requirement를 실제 시험으로 검증하고, FreeRTOS 기반 Sensor Node의 거리 측정뿐 아니라 Task/Timing/Stack/Queue/Watchdog 상태까지 확인한다.  
> **현재는 실행 전 계획 상태이므로 실제 측정값은 임의로 채우지 않는다.** 시험 후 `NOT RUN` / `TBD`를 실제 결과로 교체한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Ultrasonic Perception ECU |
| Owner | A |
| Board / Platform | STM32 #1 + Ultrasonic Sensor Array |
| Execution Model | FreeRTOS + CMSIS-RTOS2 기본 |
| Firmware / SW Commit | TBD |
| Test Date | TBD |
| Specification Revision | v0.1 |
| Architecture Revision | v0.1 |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial planned test example |

---

# 1. Test Objective

Ultrasonic Sensor의 Trigger/Echo 측정이 반복 가능하게 동작하고, Echo 시간에서 계산한 거리값이 실제 기준거리와 비교 가능한 수준인지 확인한다. 또한 Sensor timeout, invalid range, multi-sensor interference, CAN 송신 실패 같은 비정상 조건에서 ECU가 hang하지 않고 `valid=false` / fault 상태를 올바르게 만들며, FreeRTOS Task/Queue/Stack/Watchdog 구조가 안정적으로 동작하는지 검증한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / MCU | STM32 #1, actual model TBD |
| RTOS / OS | FreeRTOS version TBD |
| CMSIS-RTOS API | v2 목표 |
| Sensor | Ultrasonic Sensor model TBD |
| Sensor Count | Stage 1: 1개, 이후 실제 구성 TBD |
| Power | Sensor/board datasheet 기준, 실제 시험 시 기록 |
| Interface | Trigger GPIO / Echo Timer Input Capture / FDCAN |
| CAN Bitrate | TBD |
| Tool / Debug | STM32CubeIDE / ST-Link / UART log / Logic Analyzer 후보 |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|
| Sensor Trigger | TBD | STM32 GPIO | actual sensor datasheet 기준 |
| Sensor Echo | TBD | STM32 Timer Input Capture | Echo logic level 확인 필요 |
| CAN FD Transceiver | TBD | STM32 FDCAN | Stage 2 |
| GND | common reference | STM32/Sensor | 실제 전원구성 기록 |

사진/배선도: TBD

센서 모델이 5V Echo를 출력하는 경우 MCU 입력 허용전압을 먼저 확인하고 필요한 전압 변환 회로를 사용한다.

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-US-001 | REQ-US-001 | Trigger/Echo waveform 확인 | measurement cycle 정상 | NOT RUN | TBD |
| T-US-002 | REQ-US-002 | 기준거리 비교 | distance_mm 계산 | NOT RUN | TBD |
| T-US-003 | REQ-US-003 | 정상/비정상 입력 | valid 상태 구분 | NOT RUN | TBD |
| T-US-004 | REQ-US-004 | 반복 측정 noise 비교 | filter 효과 확인 | NOT RUN | TBD |
| T-US-005 | REQ-US-005 | threshold 주변 거리 입력 | Warning state 전환 | NOT RUN | TBD |
| T-US-006 | REQ-US-006 | Echo 미응답 | timeout + invalid | NOT RUN | TBD |
| T-US-007 | REQ-US-007 | out-of-range 조건 | 정상 거리로 사용하지 않음 | NOT RUN | TBD |
| T-US-008 | REQ-US-008 | multi-sensor test | 순차 scan / 간섭 감소 | NOT RUN | TBD |
| T-US-009 | REQ-US-009 | CAN monitor | status TX 확인 | NOT RUN | TBD |
| T-US-010 | REQ-US-010 | Sensor fault injection | fault/DTC candidate | NOT RUN | TBD |
| T-US-011 | REQ-US-011 | ISR code/trace inspection | 긴 계산/blocking 없음 | NOT RUN | TBD |
| T-US-012 | REQ-US-012 | RTOS task inspection/runtime | 책임 분리 동작 | NOT RUN | TBD |
| T-US-013 | REQ-US-013 | queue/stack/overrun injection | health state 반영 | NOT RUN | TBD |
| T-US-014 | REQ-US-014 | health/watchdog test | unhealthy 시 무조건 refresh 금지 | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|
| T-US-001 | Sensor 1개, 정상 반사체 | Trigger 후 Echo capture | NOT RUN | logic analyzer/TIM log | TBD |
| T-US-002-A | 기준거리 Near | 실제값과 비교 가능한 distance | NOT RUN | photo/log | TBD |
| T-US-002-B | 기준거리 Mid | 실제값과 비교 가능한 distance | NOT RUN | photo/log | TBD |
| T-US-002-C | 기준거리 Far | 실제값과 비교 가능한 distance | NOT RUN | photo/log | TBD |
| T-US-003 | 정상 Echo | `valid=true` | NOT RUN | UART/log | TBD |
| T-US-005 | configured warning zones | SAFE/WARNING/CRITICAL 전환 | NOT RUN | log/video | TBD |
| T-US-009 | valid status generated | CAN `Ultrasonic_Status` TX | NOT RUN | CAN log | TBD |

3-point 거리 자체는 센서의 실제 usable range 안에서 선정하고 시험 시 숫자를 기록한다.

---

# 5. Boundary / Calibration Test

## 5.1 거리 오차 측정

| Reference Distance | Raw Pulse | Raw Distance | Filtered Distance | Error | Result |
|---:|---:|---:|---:|---:|---|
| Near TBD | NOT RUN | NOT RUN | NOT RUN | TBD | TBD |
| Mid TBD | NOT RUN | NOT RUN | NOT RUN | TBD | TBD |
| Far TBD | NOT RUN | NOT RUN | NOT RUN | TBD | TBD |

측정 방법:

```text
실제 기준거리 고정
→ N회 반복 측정
→ Raw / Filtered 기록
→ 평균 / min / max / 표준편차 후보 계산
→ 필요 시 conversion/filter calibration
```

## 5.2 Warning Threshold Boundary

| Condition | Expected State | Actual | Result |
|---|---|---|---|
| SAFE↔WARNING 경계 직전/직후 | 정의된 transition | NOT RUN | TBD |
| WARNING↔CRITICAL 경계 직전/직후 | 정의된 transition | NOT RUN | TBD |
| invalid input | INVALID | NOT RUN | TBD |

Warning threshold는 실제 차량 크기와 실험 결과가 확정된 뒤 기록한다.

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-US-001 | Echo 없음 | measurement timeout | `valid=false`, timeout counter | NOT RUN | TBD |
| F-US-002 | Sensor disconnect | 반복 timeout | unavailable/fault status | NOT RUN | TBD |
| F-US-003 | out-of-range pulse | range check | reject measurement | NOT RUN | TBD |
| F-US-004 | sudden spike | filter/plausibility | 정책에 따른 reject/smooth | NOT RUN | TBD |
| F-US-005 | multi-sensor crosstalk | inconsistent jump/noise | sequential gap/filter 조정 | NOT RUN | TBD |
| F-US-006 | `MeasurementQueue` full | RTOS queue error | overflow flag/drop policy | NOT RUN | TBD |
| F-US-007 | `StatusQueue` full | RTOS queue error | overflow flag/latest-state policy | NOT RUN | TBD |
| F-US-008 | UltrasonicTask delayed | timing monitor | overrun health flag | NOT RUN | TBD |
| F-US-009 | CAN unavailable | controller/error status | local perception 유지 + comm fault | NOT RUN | TBD |
| F-US-010 | critical task health missing | HealthTask | watchdog policy 적용 | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| Sensor measurement duration | datasheet/test 후 TBD | NOT RUN | timestamp/scope | TBD |
| Full scan period | TBD | NOT RUN | RTOS timestamp | TBD |
| Measurement → Perception latency | TBD | NOT RUN | queue timestamps | TBD |
| Perception → CAN enqueue latency | TBD | NOT RUN | timestamp | TBD |
| CAN status cycle | TBD | NOT RUN | CAN analyzer | TBD |
| UltrasonicTask jitter | TBD | NOT RUN | runtime trace | TBD |

Sensor 개수와 inter-sensor gap이 확정되기 전에는 전체 scan period를 임의로 확정하지 않는다.

---

# 8. RTOS Test

## 8.1 Task Inventory

| Task | Expected Period / Trigger | Priority Direction | Observed | Result |
|---|---|---|---|---|
| `UltrasonicTask` | periodic + capture notification | High | NOT RUN | TBD |
| `PerceptionTask` | MeasurementQueue event | High/Normal | NOT RUN | TBD |
| `CanTxTask` | StatusQueue + periodic heartbeat | Normal | NOT RUN | TBD |
| `HealthTask` | periodic | Low/Normal | NOT RUN | TBD |

## 8.2 Period / Jitter

| Task | Target Period | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|
| UltrasonicTask scan | TBD | NOT RUN | NOT RUN | NOT RUN | TBD | TBD |
| HealthTask | TBD | NOT RUN | NOT RUN | NOT RUN | TBD | TBD |

Event-driven Task는 notification→run latency를 별도로 기록한다.

## 8.3 Stack / Memory

| Task / Item | Configured | Minimum Free / High-Water | Target | Result |
|---|---:|---:|---:|---|
| UltrasonicTask stack | TBD | NOT RUN | margin TBD | TBD |
| PerceptionTask stack | TBD | NOT RUN | margin TBD | TBD |
| CanTxTask stack | TBD | NOT RUN | margin TBD | TBD |
| HealthTask stack | TBD | NOT RUN | margin TBD | TBD |
| Heap free | TBD | NOT RUN | no runtime exhaustion | TBD |

## 8.4 Queue / Event / Notification

| Object | Depth / Config | Max Occupancy / Result | Overflow Test | Result |
|---|---|---|---|---|
| EchoNotify | notification | NOT RUN | missed/duplicate event test | TBD |
| MeasurementQueue | TBD | NOT RUN | Planned | TBD |
| StatusQueue | TBD | NOT RUN | Planned | TBD |
| HealthFlags | event flags | NOT RUN | missing bit test | TBD |

## 8.5 ISR → Task Test

| Interrupt | Expected ISR Action | Expected Task Wake-up | Actual | Result |
|---|---|---|---|---|
| Echo rising/falling | timestamp/capture state | UltrasonicTask notify | NOT RUN | TBD |
| Timeout event | flag/notify | UltrasonicTask timeout path | NOT RUN | TBD |

Code Review:
- [ ] ISR에서 printf 없음
- [ ] ISR에서 filter/distance 전체 계산 없음
- [ ] ISR에서 blocking API 없음
- [ ] FromISR 계열 API가 필요한 위치에 사용됨

## 8.6 Priority / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| CAN/log load 증가 | sensing deadline 유지 | NOT RUN | TBD |
| HealthTask 실행 중 sensing event | sensing 우선 처리 | NOT RUN | TBD |
| repeated sensor timeout | scheduler deadlock 없음 | NOT RUN | TBD |
| shared CAN/log resource contention | deadlock/priority inversion 없음 | NOT RUN | TBD |

## 8.7 Watchdog / Health Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all critical tasks healthy | watchdog refresh condition true | NOT RUN | TBD |
| UltrasonicTask health missing | unhealthy flag / no unconditional refresh | NOT RUN | TBD |
| queue overflow | health 상태 반영 | NOT RUN | TBD |
| task overrun | health 상태 반영 | NOT RUN | TBD |

실제 Watchdog reset 시험은 차량을 움직이지 않는 안전한 bench 상태에서 수행한다.

---

# 9. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Ultrasonic_Status` | TX | distance/valid/warning | NOT RUN | Consumer side planned | TBD |
| `Ultrasonic_Fault` 후보 | TX | fault status | NOT RUN | N/A/Event | TBD |
| `ECU_Heartbeat` | TX | alive/health | NOT RUN | Consumer side planned | TBD |
| `Vehicle_Mode` 후보 | RX | optional enable/mode | NOT RUN | Planned | TBD |

## LIN

N/A.

---

# 10. DTC / Diagnostics Test

| Fault | Expected DTC / Status | Pi Manager Stored? | H735 Displayed? | Result |
|---|---|---|---|---|
| Sensor timeout | `US_SENSOR_TIMEOUT` 후보 | NOT RUN | NOT RUN | TBD |
| Multiple sensor unavailable | `US_MULTI_SENSOR_FAULT` 후보 | NOT RUN | NOT RUN | TBD |
| CAN fault | `US_CAN_FAULT` 후보 | NOT RUN | NOT RUN | TBD |
| RTOS queue overflow | `US_RTOS_QUEUE` 후보/local health | NOT RUN | NOT RUN | TBD |

실제 DTC code와 lifecycle은 F 담당의 Diagnostics 규격 확정 후 갱신한다.

---

# 11. Soak / Load Test

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Sensor 1개 continuous measurement | TBD | reset/deadlock/overflow 없음 | NOT RUN | TBD |
| Multi-sensor continuous scan | TBD | crosstalk/queue 상태 안정 | NOT RUN | TBD |
| CAN traffic + sensing | TBD | sensing timing 유지 | NOT RUN | TBD |
| Repeated timeout/recovery | TBD | memory leak/deadlock 없음 | NOT RUN | TBD |

---

# 12. Logs / Evidence

- UART / Console Log: TBD
- Wiring Photo: TBD
- Reference distance Photo: TBD
- Logic Analyzer / Scope: TBD
- CAN Log: TBD
- RTOS Trace / Runtime stats: TBD
- Stack high-water log: TBD
- Test Video: TBD

Stage 1 예시 로그 형식:

```text
[US][INIT] sensor_count=1
[US][TRIG] id=0
[US][ECHO] id=0 pulse_us=...
[US][DATA] id=0 dist_mm=... valid=1 level=SAFE
[US][TIMEOUT] id=0
[US][HEALTH] queue_overflow=0 task_overrun=0
```

---

# 13. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

---

# 14. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Trigger/Echo waveform이 확인된다.
- [ ] 3개 이상의 기준거리 측정값을 기록한다.
- [ ] distance raw/filtered 값과 오차를 비교한다.
- [ ] valid/invalid/timeout을 구분한다.
- [ ] Warning state boundary를 시험한다.
- [ ] multi-sensor 사용 시 crosstalk/scan을 시험한다.
- [ ] CAN status 송신을 확인한다.
- [ ] Task period/trigger를 확인한다.
- [ ] ISR → Task Notification 흐름을 확인한다.
- [ ] 중요 Task stack high-water를 측정한다.
- [ ] Queue overflow 정책을 시험한다.
- [ ] starvation/deadlock 징후가 없다.
- [ ] Health/Watchdog 정책을 확인한다.
- [ ] 로그/사진/trace 증거가 남아 있다.

## Remaining Issues

- 실제 Sensor model / count / placement 확정 필요
- Echo voltage / level shifting 확인 필요
- 실제 distance calibration 필요
- Warning threshold / filter parameter 확정 필요
- CAN Matrix 확정 필요
- RTOS priority / stack / queue depth profiling 필요
