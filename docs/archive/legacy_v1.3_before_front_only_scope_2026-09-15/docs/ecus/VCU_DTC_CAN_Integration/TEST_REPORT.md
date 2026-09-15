# VCU + DTC + CAN Integration Test Report

> 2026-09-11: STM32G431KB 구매 모델 부분 동결. [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-001~005를 따른다. 제조사/revision/핀 배정과 실기 시험은 별도이며, 아래 시험 결과/측정값을 PASS로 변경한 것은 아니다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: VCU 기능 요구사항과 FreeRTOS 실행 구조를 실제 시험으로 검증한다. 현재는 실행 전 계획 상태이므로 결과는 `NOT RUN / TBD`로 둔다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | VCU + DTC + CAN Integration |
| Owner | F |
| Board / Platform | STM32G431KB (STM32 #5) |
| Execution Model | FreeRTOS + CMSIS-RTOS2 |
| Firmware Commit | TBD |
| Test Date | TBD |
| Specification Revision | v0.1 |
| Architecture Revision | v0.1 |

# 1. Test Objective

Driver Input, CAN Request, Safety/Fault를 조합했을 때 VCU가 예상한 최종 명령을 만들고, timeout/critical fault/RTOS load 조건에서도 stale 또는 위험한 명령을 유지하지 않는지 검증한다.

# 2. Test Environment

| Item | Value |
|---|---|
| STM32 | STM32G431KB, 구매 모델 확정 / 실기 NOT RUN |
| RTOS | FreeRTOS version TBD |
| CMSIS-RTOS | v2 |
| CAN FD Transceiver | TBD |
| Driver Input | Gear/Accel/Brake/Steering/E-Stop 후보 |
| Debug | STM32CubeIDE / ST-Link / UART / CAN logger 후보 |
| CAN bitrate | TBD |

# 3. Requirement Verification Matrix

| Test ID | Requirement | Test | Expected | Result |
|---|---|---|---|---|
| T-VCU-001 | REQ-VCU-001 | driver input sweep | normalized valid input | NOT RUN |
| T-VCU-002 | REQ-VCU-002 | CAN request injection | request repository update | NOT RUN |
| T-VCU-003 | REQ-VCU-003 | stop periodic request | stale request invalid | NOT RUN |
| T-VCU-004 | REQ-VCU-004 | E-Stop during normal request | safety override wins | NOT RUN |
| T-VCU-005 | REQ-VCU-005 | parking critical + driver accel | safety rule wins | NOT RUN |
| T-VCU-006 | REQ-VCU-006 | inspect CAN TX | only final command to Drive | NOT RUN |
| T-VCU-007 | REQ-VCU-007 | peer heartbeat timeout | fault detected | NOT RUN |
| T-VCU-008 | REQ-VCU-008 | periodic state/heartbeat | messages observed | NOT RUN |
| T-VCU-009 | REQ-VCU-009 | DTC event injection | common DTC state handled | NOT RUN |
| T-VCU-010 | REQ-VCU-010 | critical DTC injection | safe action applied | NOT RUN |
| T-VCU-011 | REQ-VCU-011 | heavy logging/load | control timing maintained | NOT RUN |
| T-VCU-012 | REQ-VCU-012 | code review/trace | ISR minimal | NOT RUN |
| T-VCU-013 | REQ-VCU-013 | stack/queue health | observable health | NOT RUN |
| T-VCU-014 | REQ-VCU-014 | watchdog health test | expected policy | NOT RUN |

# 4. Driver Input Test

| Input | Condition | Expected | Actual | Result |
|---|---|---|---|---|
| Gear | P/R/N/D | defined enum | NOT RUN | TBD |
| Accelerator | min/mid/max | calibrated normalized value | NOT RUN | TBD |
| Brake | min/mid/max | calibrated normalized value | NOT RUN | TBD |
| Steering | left/center/right | calibrated angle/value | NOT RUN | TBD |
| E-Stop | inactive/active | safety flag follows | NOT RUN | TBD |
| Invalid ADC | out-of-range | invalid/safe state | NOT RUN | TBD |

# 5. Arbitration Test

| Scenario | Driver | ADAS | Parking | Fault | Expected Final | Result |
|---|---|---|---|---|---|---|
| Normal drive | accel | none | safe | none | driver-based command | TBD |
| ADAS request | accel | valid request | safe | none | policy result | TBD |
| Parking critical | accel | none | critical | none | stop/limit policy | TBD |
| E-Stop | accel | request | critical/none | E-Stop | safe/disable | TBD |
| HPC timeout | accel | stale | safe | comm fault | ADAS ignored | TBD |
| Drive ECU offline | any | any | any | drive timeout | safe state candidate | TBD |

세부 expected 값은 Arbitration policy 확정 후 수치화한다.

# 6. Fault / Edge Case Test

| Fault | Expected Detection | Expected Action | Actual | Result |
|---|---|---|---|---|
| Vision request timeout | freshness timeout | request invalid | NOT RUN | TBD |
| Ultrasonic timeout | status timeout | parking info invalid + DTC 후보 | NOT RUN | TBD |
| Drive heartbeat timeout | heartbeat timeout | safe command/disable 후보 | NOT RUN | TBD |
| CAN bus-off | controller status | degraded + DTC | NOT RUN | TBD |
| DTC unknown code | format valid/code unknown | raw source/code 유지 | NOT RUN | TBD |
| CanRxQueue overflow | queue health | overflow recorded | NOT RUN | TBD |
| FinalCommandQueue pressure | queue high-water | latest command policy 유지 | NOT RUN | TBD |
| task delayed | health/overrun | fault visible | NOT RUN | TBD |

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| VcuControlTask period | 5~10 ms 후보 | NOT RUN | timestamp/trace | TBD |
| VcuControlTask jitter | TBD | NOT RUN | trace | TBD |
| Safety event → override | TBD | NOT RUN | EXTI/task timestamp | TBD |
| CAN RX → arbitration | TBD | NOT RUN | timestamp | TBD |
| arbitration → CAN TX | TBD | NOT RUN | timestamp | TBD |
| heartbeat timeout detect | TBD | NOT RUN | frame stop injection | TBD |

# 8. RTOS Test

## Task Inventory

| Task | Expected Trigger/Period | Priority Direction | Observed | Result |
|---|---|---|---|---|
| SafetyTask | event + fast periodic | Highest | NOT RUN | TBD |
| VcuControlTask | 5~10 ms 후보 | High | NOT RUN | TBD |
| CanRxTask | event | High | NOT RUN | TBD |
| DriverInputTask | 10~20 ms 후보 | High/Normal | NOT RUN | TBD |
| CanTxTask | event/periodic | Normal/High | NOT RUN | TBD |
| DiagnosticTask | event/periodic | Normal/Low | NOT RUN | TBD |
| HealthTask | 50~100 ms 후보 | Low/Normal | NOT RUN | TBD |

## Stack / Queue

| Item | Configured | High-Water / Max Occupancy | Result |
|---|---:|---:|---|
| SafetyTask stack | TBD | NOT RUN | TBD |
| VcuControlTask stack | TBD | NOT RUN | TBD |
| CanRxQueue | TBD | NOT RUN | TBD |
| DtcEventQueue | TBD | NOT RUN | TBD |
| FinalCommandQueue | TBD | NOT RUN | TBD |

## Load / Starvation

| Scenario | Expected | Result |
|---|---|---|
| UART logging heavy | Safety/Control deadline 유지 | TBD |
| CAN burst | critical task timing 유지 | TBD |
| many DTC events | control path 영향 제한 | TBD |
| low-priority diagnostics busy | no safety starvation | TBD |

# 9. CAN Communication Test

| Message | Direction | Expected | Timeout Test | Result |
|---|---|---|---|---|
| Vision_Request | RX | repository update | yes | TBD |
| Ultrasonic_Status | RX | warning update | yes | TBD |
| Drive_Status | RX | state update | yes | TBD |
| Body_Status | RX | body update | yes | TBD |
| ECU_Heartbeat | RX/TX | alive tracking | yes | TBD |
| DTC_Event | RX/TX | diagnostic flow | event | TBD |
| Vehicle_State | TX | gear/mode/safety | N/A | TBD |
| Final_Drive_Command | TX | final speed/steer/enable | N/A | TBD |

# 10. DTC / Diagnostics Test

| Fault | Expected DTC/Status | Pi Stored? | H735 Displayed? | Safe Action? | Result |
|---|---|---|---|---|---|
| Driver input invalid | VCU input DTC candidate | NOT RUN | NOT RUN | policy | TBD |
| Drive heartbeat lost | communication DTC | NOT RUN | NOT RUN | candidate yes | TBD |
| HPC timeout | communication DTC | NOT RUN | NOT RUN | ADAS disabled | TBD |
| CAN bus-off | VCU CAN DTC | NOT RUN | NOT RUN | degraded | TBD |
| RTOS health fault | VCU SW DTC candidate | NOT RUN | NOT RUN | policy | TBD |

# 11. Watchdog / Health Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all critical tasks healthy | watchdog refresh | NOT RUN | TBD |
| SafetyTask health missing | fault / no-refresh policy | NOT RUN | TBD |
| VcuControlTask health missing | fault / no-refresh policy | NOT RUN | TBD |
| queue overflow | health flag set | NOT RUN | TBD |
| stack low watermark | health warning | NOT RUN | TBD |

Watchdog reset 계열 시험은 안전한 bench 상태에서 수행한다.

# 12. Soak Test

| Test | Duration | Expected | Result |
|---|---|---|---|
| Normal VCU loop | TBD | reset/deadlock/overflow 없음 | TBD |
| CAN burst + driver input | TBD | timing 유지 | TBD |
| repeated D/R + vision request | TBD | stale state 없음 | TBD |
| repeated fault/recovery | TBD | state recovery 일관 | TBD |

# 13. Evidence

- UART log: TBD
- CAN trace: TBD
- RTOS trace/runtime stats: TBD
- Stack watermark: TBD
- Logic analyzer/scope: TBD
- Wiring/photo: TBD

# 14. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Driver Input 유효성 확인
- [ ] Arbitration 우선순위 재현
- [ ] E-Stop safety override 확인
- [ ] stale CAN request 제거 확인
- [ ] final command TX 확인
- [ ] heartbeat/DTC 흐름 확인
- [ ] RTOS timing/stack/queue 확인
- [ ] Watchdog/Health 정책 확인
- [ ] load/soak에서 deadlock/starvation 없음
