# [NODE / FEATURE NAME] Test Report

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md`의 Requirement를 실제 시험 결과로 확인하고, RTOS Node는 기능뿐 아니라 Task/Timing/Stack/Queue/Watchdog 상태까지 검증한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | |
| Owner | A / B / C / D / E / F |
| Board / Platform | |
| Execution Model | FreeRTOS / Linux / Bare-metal exception |
| Firmware / SW Commit | |
| Test Date | |
| Specification Revision | |
| Architecture Revision | |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | | | Initial |

---

# 1. Test Objective

이번 시험에서 무엇을 검증하는지 2~3문장으로 적는다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / MCU / Pi | |
| RTOS / OS | FreeRTOS version / Linux distro |
| CMSIS-RTOS API | v2 / N/A |
| Sensor / Actuator | |
| Power | |
| Interface | |
| CAN/LIN Bitrate | |
| Camera Resolution/FPS | |
| Tool / Debug Interface | |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-001 | REQ-XXX-001 | | | | |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|

---

# 5. Boundary / Calibration Test

| Condition | Raw | Converted | Reference | Error / Note | Result |
|---|---:|---:|---:|---|---|

해당 없는 Node는 N/A.

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-001 | Sensor disconnect | invalid / timeout | | | |
| F-002 | CAN/LIN timeout | timeout flag | | | |
| F-003 | Queue full, RTOS 해당 시 | overflow detect/drop policy | | | |
| F-004 | Task delayed/blocked | health/overrun detect | | | |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| Update period | | | timestamp/trace | |
| Response latency | | | | |
| Timeout detection | | | | |
| Periodic task jitter | | | | |
| FPS / inference latency | | | | |

---

# 8. RTOS Test

Linux Node는 Process/Thread/Service test로 바꿔 작성한다.

## 8.1 Task Inventory

| Task | Expected Period / Trigger | Priority | Observed | Result |
|---|---|---|---|---|

## 8.2 Period / Jitter

| Task | Target Period | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|

## 8.3 Stack / Memory

| Task / Item | Configured | Minimum Free / High-Water | Target | Result |
|---|---:|---:|---:|---|
| Task stack | | | | |
| Heap free | | | | |

실제 수치는 FreeRTOS runtime stats / stack high-water API / debugger 등 사용 가능한 방법으로 측정한다.

## 8.4 Queue / Event / Notification

| Object | Depth / Config | Max Occupancy / Result | Overflow Test | Result |
|---|---|---|---|---|

## 8.5 ISR → Task Test

| Interrupt | Expected ISR Action | Expected Task Wake-up | Actual | Result |
|---|---|---|---|---|

ISR 안에서 긴 계산이나 blocking 호출이 없는지도 Code Review한다.

## 8.6 Priority / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| High load + critical task | critical task deadline 유지 | | |
| Logging enabled | control/safety timing 영향 제한 | | |
| Shared resource contention | deadlock/priority inversion 없음 | | |

## 8.7 Watchdog / Health Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| All critical tasks healthy | watchdog refresh | | |
| Critical task health missing | fault / refresh 중단 정책 | | |
| Queue overflow / overrun | health 상태에 반영 | | |

실제 Watchdog reset 시험은 안전한 bench 상태에서 수행한다.

---

# 9. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|

## LIN

| Frame / Signal | Publisher | Expected Period/Value | Actual | Fault Test | Result |
|---|---|---|---|---|---|

---

# 10. DTC / Diagnostics Test

| Fault | Expected DTC / Status | Pi Manager Stored? | H735 Displayed? | Result |
|---|---|---|---|---|

---

# 11. Soak / Load Test

RTOS Node는 짧은 정상 시험만으로 끝내지 않는다.

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Normal soak | TBD | reset/deadlock/overflow 없음 | | |
| CAN burst + normal function | TBD | critical task timing 유지 | | |
| UI/render + CAN, H735 | TBD | UI freeze/queue overflow 없음 | | |

---

# 12. Logs / Evidence

- UART / Console Log:
- Screenshot:
- Wiring Photo:
- Test Video:
- CAN/LIN Log:
- Trace / Runtime stats:
- Stack high-water screenshot/log:
- Scope / Logic Analyzer:

---

# 13. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|

---

# 14. Final Result

```text
RESULT: PASS / FAIL / PARTIAL / NOT RUN
```

## PASS 조건

- [ ] 주요 Requirement가 Test ID에 연결되어 있다.
- [ ] 정상 기능 결과가 재현된다.
- [ ] Fault/Timeout이 필요한 기능은 시험했다.
- [ ] Timing 목표가 있는 기능은 측정했다.
- [ ] RTOS Node는 Task period/trigger를 확인했다.
- [ ] 중요 Task의 Stack 여유를 확인했다.
- [ ] Queue overflow 정책을 확인했다.
- [ ] ISR → Task 흐름을 확인했다.
- [ ] deadlock/starvation 징후가 없다.
- [ ] Watchdog/Health 정책을 확인했다, 해당 시.
- [ ] 증거가 남아 있다.
- [ ] 남은 문제/TBD가 기록되어 있다.

## Remaining Issues

- 
- 
