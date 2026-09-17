# VCU + DTC + CAN Integration Test Report

> 2026-09-11: STM32G431KB 구매 모델 부분 동결. [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-001~005를 따른다. 제조사/revision/핀 배정과 실기 시험은 별도이며, 아래 시험 결과/측정값을 PASS로 변경한 것은 아니다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **2026-09-15 범위 변경 (1차):** `Driver_Input`(가속/브레이크/조향) 입력을 VCU가 직접 GPIO/ADC로 읽던 시험 항목을 삭제하고, C가 발행한 `Driver_Input` CAN 수신 시험으로 대체했다. `Vision_Request`는 `ADAS_Request`로, Collision Critical 우선순위 시험을 강화했다.
>
> **2026-09-15 범위 변경 (2차):** Gear를 VCU 자체 GPIO로 시험하던 항목을 삭제했다. F는 Driver/Gear 입력용 GPIO가 없으며, `Driver_Input.gear` CAN 수신 시험으로 대체한다. DTC는 Pi 저장 시험 없이 실시간 표시만 확인한다.
>
> **2026-09-17: E-Stop 기능 전체 제거.** 데모 보드 특성상 E-Stop(소프트웨어/하드웨어 전부)을 프로젝트 전역에서 제거했다. E-Stop 관련 시험 항목(T-VCU-004의 E-Stop 부분, Arbitration/DTC의 E-Stop 행)을 삭제했다. 근거: [`FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-020/DEC-HW-027/DEC-CTRL-006(REMOVED).

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
| Specification Revision | v0.2 |
| Architecture Revision | v0.2 |

# 1. Test Objective

`Driver_Input`(CAN, C 발행 — gear 포함), `ADAS_Request`, `Ultrasonic_Status`, Safety/Fault를 조합했을 때 VCU가 예상한 최종 명령을 만들고, timeout/critical fault/RTOS load 조건에서도 stale 또는 위험한 명령을 유지하지 않는지 검증한다. Ultrasonic Collision Critical이 `ADAS_Request`보다 항상 우선함을 별도로 검증한다.

# 2. Test Environment

| Item | Value |
|---|---|
| STM32 | STM32G431KB, 구매 모델 확정 / 실기 NOT RUN |
| RTOS | FreeRTOS version TBD |
| CMSIS-RTOS | v2 |
| CAN FD Transceiver | TBD |
| VCU 자체 물리 입력 | 없음 (전부 CAN 수신) |
| `Driver_Input` 소스 | Drive ECU(C) CAN 발행 (accel/brake/steering/gear), dummy/real |
| Debug | STM32CubeIDE / ST-Link / UART / CAN logger 후보 |
| CAN bitrate | TBD |

# 3. Requirement Verification Matrix

| Test ID | Requirement | Test | Expected | Result |
|---|---|---|---|---|
| T-VCU-001 | REQ-VCU-001 | driver input sweep | normalized valid input | NOT RUN |
| T-VCU-002 | REQ-VCU-002 | CAN request injection | request repository update | NOT RUN |
| T-VCU-003 | REQ-VCU-003 | stop periodic request | stale request invalid | NOT RUN |
| T-VCU-004 | REQ-VCU-004 | critical fault during normal request | safety override wins | NOT RUN |
| T-VCU-005 | REQ-VCU-005 | collision_warning critical + driver accel | safety rule wins | NOT RUN |
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
| `Driver_Input.gear` (CAN, C 발행) | P/R/N/D | defined enum | NOT RUN | TBD |
| `Driver_Input.accel` (CAN, C 발행) | min/mid/max | 값 반영 | NOT RUN | TBD |
| `Driver_Input.brake` (CAN, C 발행) | min/mid/max | 값 반영 | NOT RUN | TBD |
| `Driver_Input.steer` (CAN, C 발행) | left/center/right | 값 반영 | NOT RUN | TBD |
| `Driver_Input` invalid/timeout | out-of-range 또는 미수신 | invalid/safe state | NOT RUN | TBD |

# 5. Arbitration Test

| Scenario | Driver | ADAS | Collision Warning | Fault | Expected Final | Result |
|---|---|---|---|---|---|---|
| Normal drive | accel | none | safe | none | driver-based command | TBD |
| ADAS request | accel | valid request | safe | none | policy result | TBD |
| Collision Warning critical | accel | none | critical | none | stop/limit policy, ADAS_Request와 무관 | TBD |
| Collision Warning critical + ADAS request 동시 | accel | valid request | critical | none | Collision Critical이 ADAS_Request override, stop/limit policy 유지 | TBD |
| HPC timeout | accel | stale | safe | comm fault | ADAS ignored | TBD |
| Drive ECU offline | any | any | any | drive timeout | safe state candidate | TBD |

세부 expected 값은 Arbitration policy 확정 후 수치화한다.

# 6. Fault / Edge Case Test

| Fault | Expected Detection | Expected Action | Actual | Result |
|---|---|---|---|---|
| Vision request timeout | freshness timeout | request invalid | NOT RUN | TBD |
| Ultrasonic timeout | status timeout | collision_warning info invalid + DTC 후보 | NOT RUN | TBD |
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
| `Driver_Input` | RX | repository update | yes | TBD |
| `ADAS_Request` | RX | repository update | yes | TBD |
| `Ultrasonic_Status` | RX | warning/Collision Critical update | yes | TBD |
| `Drive_Status` | RX | state(estimated speed/rpm) update | yes | TBD |
| `Body_Status` | RX | body update | yes | TBD |
| `ECU_Heartbeat` | RX/TX | alive tracking | yes | TBD |
| `DTC_Event` | RX/TX | diagnostic flow | event | TBD |
| `Vehicle_State` | TX | gear/mode/safety | N/A | TBD |
| Final_Drive_Command | TX | final speed/steer/enable | N/A | TBD |

# 10. DTC / Diagnostics Test

> Pi DTC Manager/History DB는 삭제됐다. 저장 여부 시험이 아니라 **B(IVI) 실시간 표시**와 fault 해소 시 화면에서 사라지는지를 확인한다.

| Fault | Expected DTC/Status | H735 Displayed (Active)? | 해소 시 화면에서 사라짐? | Safe Action? | Result |
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
- [ ] stale CAN request 제거 확인
- [ ] final command TX 확인
- [ ] heartbeat/DTC 흐름 확인
- [ ] RTOS timing/stack/queue 확인
- [ ] Watchdog/Health 정책 확인
- [ ] load/soak에서 deadlock/starvation 없음

## 2026-09-15 변경 회귀 시험 계획

추가 계획이며 기존 실기 PASS의 범위를 확대하지 않는다. 수치 기준은 최상위 명세 동결 후 적용한다.

| ID | 입력/조건 | 기대 결과 | 결과 |
|---|---|---|---|
| CW-VCU-01 | 유효 초음파 CRITICAL + ADAS + Driver 가속 | 초음파 안전 개입이 ADAS/Driver보다 우선 | NOT RUN |
| CW-VCU-02 | 초음파 CRITICAL + Critical Fault | Critical Fault 최우선 유지 | NOT RUN |
| CW-VCU-03 | 전/후방 위험과 D/R/P/N 조합 | DEC-CTRL-011/012에서 확정한 대상 zone·정지/복구 정책 적용; 동결 전 판정 보류 | NOT RUN |
