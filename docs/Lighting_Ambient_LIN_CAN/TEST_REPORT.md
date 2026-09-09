# Lighting + Ambient / LIN-CAN Test Report

> 목적: `SPECIFICATION.md`의 Requirement를 실제 시험으로 검증한다. 현재는 실행 전 계획 상태이므로 측정값은 임의로 채우지 않고 `NOT RUN` / `TBD`로 둔다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Lighting + Ambient / LIN-CAN |
| Owner | D |
| Board / Platform | Gateway STM32 + LIN Slave STM32 |
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

Gateway의 CAN↔LIN 변환, LIN Master↔Slave 통신, Ambient sensing, Lighting output, Fault/Timeout 처리와 FreeRTOS timing/queue/stack/watchdog 상태를 검증한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Gateway MCU | TBD |
| LIN Slave MCU | TBD |
| RTOS | FreeRTOS version TBD |
| CMSIS-RTOS API | v2 예정 |
| CAN FD Transceiver | TBD |
| LIN Transceiver | TBD |
| Ambient Sensor | TBD |
| Lamp / LED | TBD |
| CAN bitrate | TBD |
| LIN bitrate | TBD |
| Tool | STM32CubeIDE / ST-Link / Logic Analyzer 후보 |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|
| Gateway FDCAN | TBD | CAN FD Transceiver | schematic 확인 |
| Gateway LIN UART | TBD | LIN Transceiver | direct LIN 연결 금지 |
| Slave LIN UART | TBD | LIN Transceiver | direct LIN 연결 금지 |
| Ambient Sensor | TBD | Slave ADC/I2C | voltage 확인 |
| Lamp Output | TBD | Driver/LED | load current 확인 |

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-BODY-001 | REQ-BODY-001 | CAN command injection | Gateway receives `Body_Command` | NOT RUN | TBD |
| T-BODY-002 | REQ-BODY-002 | end-to-end signal test | CAN request maps to LIN signal | NOT RUN | TBD |
| T-BODY-003 | REQ-BODY-003 | logic analyzer/timestamp | LIN schedule period/order correct | NOT RUN | TBD |
| T-BODY-004 | REQ-BODY-004 | ambient source variation | valid ambient value generated | NOT RUN | TBD |
| T-BODY-005 | REQ-BODY-005 | LIN Lamp_Command | lamp output changes | NOT RUN | TBD |
| T-BODY-006 | REQ-BODY-006 | output state readback | Lamp_Status matches state | NOT RUN | TBD |
| T-BODY-007 | REQ-BODY-007 | CAN monitor | Body_Status TX correct | NOT RUN | TBD |
| T-BODY-008 | REQ-BODY-008 | slave disconnect | timeout/fault detected | NOT RUN | TBD |
| T-BODY-009 | REQ-BODY-009 | code review | ISR minimal | NOT RUN | TBD |
| T-BODY-010 | REQ-BODY-010 | architecture/runtime | tasks separated | NOT RUN | TBD |
| T-BODY-011 | REQ-BODY-011 | RTOS runtime stats | stack/queue health measurable | NOT RUN | TBD |
| T-BODY-012 | REQ-BODY-012 | health fault injection | watchdog policy reacts | NOT RUN | TBD |
| T-BODY-013 | REQ-BODY-013 | CAN fault injection | LIN tasks remain alive | NOT RUN | TBD |
| T-BODY-014 | REQ-BODY-014 | LIN fault injection | CAN node remains alive | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual | Evidence | Result |
|---|---|---|---|---|---|
| T-BODY-001 | Headlamp ON request over CAN | Gateway logical request updated | NOT RUN | TBD | TBD |
| T-BODY-002 | Headlamp ON request | LIN `LAMP_HEAD_CMD=ON` | NOT RUN | TBD | TBD |
| T-BODY-004 | Ambient low/mid/high reference | ambient logical value changes | NOT RUN | TBD | TBD |
| T-BODY-005 | LIN Lamp_Command ON/OFF | physical lamp toggles | NOT RUN | TBD | TBD |
| T-BODY-007 | valid ambient/lamp state | Body_Status CAN reflects state | NOT RUN | TBD | TBD |

---

# 5. Boundary / Calibration Test

| Condition | Input | Expected | Actual | Note | Result |
|---|---|---|---|---|---|
| Ambient minimum candidate | sensor minimum | valid or defined low state | NOT RUN | actual sensor range TBD | TBD |
| Ambient maximum candidate | sensor maximum | valid or defined high state | NOT RUN | actual sensor range TBD | TBD |
| Lamp command rapid toggle | repeated events | no corrupted state | NOT RUN | load limit 고려 | TBD |
| LIN schedule high load | all frames enabled | no missed deadline target | NOT RUN | period TBD | TBD |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Action | Actual | Result |
|---|---|---|---|---|---|
| F-BODY-001 | LIN Slave disconnect | response timeout | slave invalid + Body fault | NOT RUN | TBD |
| F-BODY-002 | Ambient sensor disconnect | sensor invalid | ambient invalid report | NOT RUN | TBD |
| F-BODY-003 | CAN bus unavailable | controller fault | CAN degraded, LIN task alive | NOT RUN | TBD |
| F-BODY-004 | LIN communication failure | timeout/error | CAN node alive + fault report | NOT RUN | TBD |
| F-BODY-005 | Queue full | send fail/overflow counter | bounded drop/fault policy | NOT RUN | TBD |
| F-BODY-006 | LinScheduleTask delayed | overrun/jitter detect | health degraded/log | NOT RUN | TBD |
| F-BODY-007 | Invalid lamp command | validation fail | output unchanged/safe policy | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| LIN slot period | TBD | NOT RUN | logic analyzer | TBD |
| LIN schedule jitter | TBD | NOT RUN | timestamp/trace | TBD |
| CAN RX → LIN command latency | TBD | NOT RUN | timestamp | TBD |
| LIN ambient → CAN Body_Status latency | TBD | NOT RUN | timestamp | TBD |
| Lamp command → GPIO/PWM response | TBD | NOT RUN | logic analyzer | TBD |
| Health timeout detection | TBD | NOT RUN | fault injection | TBD |

---

# 8. RTOS Test

## 8.1 Gateway Task Inventory

| Task | Expected Trigger / Period | Priority Direction | Observed | Result |
|---|---|---|---|---|
| CanRxTask | Event | High | NOT RUN | TBD |
| LinScheduleTask | slot TBD | High | NOT RUN | TBD |
| GatewayMappingTask | Event | Normal/High | NOT RUN | TBD |
| CanTxTask | Event/Periodic | Normal | NOT RUN | TBD |
| HealthTask | 100 ms candidate | Low/Normal | NOT RUN | TBD |

## 8.2 Slave Task Inventory

| Task | Expected Trigger / Period | Priority Direction | Observed | Result |
|---|---|---|---|---|
| LinRxTask | Event | High | NOT RUN | TBD |
| AmbientTask | 50~100 ms candidate | Normal | NOT RUN | TBD |
| LightingTask | Event/10~20 ms candidate | Normal/High | NOT RUN | TBD |
| StatusTask | Event/Schedule | Normal | NOT RUN | TBD |
| HealthTask | 100 ms candidate | Low | NOT RUN | TBD |

## 8.3 Stack / Memory

| Task / Item | Configured | High-Water / Minimum Free | Target | Result |
|---|---:|---:|---:|---|
| Gateway tasks | TBD | NOT RUN | margin TBD | TBD |
| Slave tasks | TBD | NOT RUN | margin TBD | TBD |
| Heap free | TBD | NOT RUN | TBD | TBD |

## 8.4 Queue / IPC

| Object | Depth | Max Occupancy | Overflow Test | Result |
|---|---|---|---|---|
| CanRxQueue | TBD | NOT RUN | Planned | TBD |
| BodyCommandQueue | TBD | NOT RUN | Planned | TBD |
| LinStatusQueue | TBD | NOT RUN | Planned | TBD |
| CanTxQueue | TBD | NOT RUN | Planned | TBD |

## 8.5 ISR → Task

| Interrupt | Expected ISR Action | Wake Task | Actual | Result |
|---|---|---|---|---|
| FDCAN RX | frame copy/notify | CanRxTask | NOT RUN | TBD |
| LIN RX/TX | state/notify | LinRxTask/LinScheduleTask | NOT RUN | TBD |
| CAN/LIN Error | state/notify | HealthTask | NOT RUN | TBD |

## 8.6 Watchdog / Health

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| All critical tasks alive | watchdog refresh | NOT RUN | TBD |
| LinScheduleTask health missing | fault/no-refresh policy | NOT RUN | TBD |
| queue overflow | health counter/fault | NOT RUN | TBD |
| stack low watermark | health warning | NOT RUN | TBD |

---

# 9. Communication Test

## CAN FD

| Message | Direction | Expected | Actual | Timeout/Fault | Result |
|---|---|---|---|---|---|
| `Body_Command` | RX | logical body request | NOT RUN | Planned | TBD |
| `Body_Status` | TX | ambient/lamp/LIN health | NOT RUN | N/A | TBD |
| `DTC_Event` | TX | Body fault event | NOT RUN | N/A | TBD |
| `ECU_Heartbeat` | TX | gateway alive | NOT RUN | N/A | TBD |

## LIN

| Frame | Publisher | Expected | Actual | Fault Test | Result |
|---|---|---|---|---|---|
| `Lamp_Command` | Gateway | lamp command signals | NOT RUN | Planned | TBD |
| `Ambient_Status` | Slave | valid ambient | NOT RUN | Planned | TBD |
| `Lamp_Status` | Slave | lamp state | NOT RUN | Planned | TBD |
| `Lamp_Diagnostic` | Slave | local fault | NOT RUN | Planned | TBD |

---

# 10. End-to-End Gateway Test

| Flow | Expected | Actual | Result |
|---|---|---|---|
| H735/VCU → CAN → Gateway → LIN → Lamp | requested lamp action | NOT RUN | TBD |
| Ambient → Slave → LIN → Gateway → CAN → H735 | ambient status visible | NOT RUN | TBD |
| Slave Fault → LIN → Gateway → CAN DTC | fault reaches diagnostics | NOT RUN | TBD |

---

# 11. Soak / Load Test

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Normal LIN schedule soak | TBD | no reset/deadlock | NOT RUN | TBD |
| CAN burst + LIN schedule | TBD | LIN deadline maintained | NOT RUN | TBD |
| Repeated lamp commands | TBD | state consistent | NOT RUN | TBD |
| Repeated slave disconnect/reconnect | TBD | recovery repeatable | NOT RUN | TBD |

---

# 12. Logs / Evidence

- UART log: TBD
- CAN log: TBD
- LIN logic analyzer capture: TBD
- Wiring photo: TBD
- Lamp test video: TBD
- RTOS runtime stats: TBD
- Stack high-water log: TBD

예시 로그 형식:

```text
[BODY][CAN_RX] Body_Command head=1 left=0 right=0
[BODY][MAP] HEADLAMP_REQ -> LAMP_HEAD_CMD
[LIN][TX] Lamp_Command
[LIN][RX] Lamp_Status head=1
[BODY][CAN_TX] Body_Status
[BODY][FAULT] LIN slave timeout
```

---

# 13. Problems and Fixes

| Problem | Root Cause | Fix | Retest | Prevention |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

---

# 14. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Gateway/Slave 각각 build/flash/run
- [ ] CAN→LIN command path 정상
- [ ] LIN→CAN status path 정상
- [ ] Ambient sensing 정상
- [ ] Lamp output/status 정상
- [ ] Slave timeout 검출
- [ ] LIN schedule period/jitter 측정
- [ ] ISR→Task 구조 확인
- [ ] Stack/Queue health 확인
- [ ] CAN fault와 LIN fault isolation 확인
- [ ] Watchdog/Health 정책 확인
- [ ] 로그/logic analyzer/영상 증거 확보

## Remaining Issues

- 실제 MCU/Transceiver 선정
- LIN bitrate/frame ID/schedule 확정
- Lamp driver/load 정격 확정
- CAN Matrix 확정
- RTOS priority/stack/queue depth 실측 후 확정
