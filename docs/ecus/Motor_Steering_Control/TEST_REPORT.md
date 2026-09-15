# Motor + Steering Control Test Report

> 2026-09-11: STM32G431KB 구매 모델 부분 동결. [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-001~005를 따른다. 제조사/revision/핀 배정과 실기 시험은 별도이며, 아래 시험 결과/측정값을 PASS로 변경한 것은 아니다.

> **2026-09-15 범위 변경 (1차):** Encoder/Hall 관련 테스트 항목을 전부 삭제했다. RF/가변저항 `Driver_Input` 읽기 시험과 명령값 기반 speed/rpm 추정 검증 항목으로 대체했다.
>
> **2026-09-15 범위 변경 (2차):** E-Stop/Gear 물리 입력 시험 항목을 F에서 C로 이전했다. E-Stop은 CAN 비의존 로컬 즉시 차단 시험을 추가했다. 근거: [`FINAL_IMPLEMENTATION_SPEC.md` §1, §1.2, §3.1, §4.8.1, §8 C Drive](../../system/FINAL_IMPLEMENTATION_SPEC.md).

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md`의 요구사항을 실제 시험으로 검증하고, FreeRTOS 기반 제어 Node의 기능뿐 아니라 **ControlTask timing, ISR→Task, Stack, Queue, Watchdog/Health**까지 확인한다.
> **현재는 시험 전 계획 상태이므로 실제 측정값을 임의로 채우지 않는다.** 시험 후 `NOT RUN`과 `TBD`를 실제 결과로 교체한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Motor + Steering Control ECU |
| Owner | C |
| Board / Platform | STM32G431KB (STM32 #2) + Motor Driver + Brushed DC Motor + RC Servo + RF 수신기/가변저항 |
| Execution Model | FreeRTOS + CMSIS-RTOS2 기본 |
| Firmware / SW Commit | TBD |
| Test Date | TBD |
| Specification Revision | v0.2 |
| Architecture Revision | v0.2 |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial planned RTOS control test example |
| v0.2 | 2026-09-15 | Team | Encoder/Hall 시험 항목 삭제, Driver_Input 읽기 및 speed/rpm 추정 시험 항목으로 대체 |

---

## 추가 E-Stop 검증 계획

| 조건 | 기대 동작 | 실행 상태 |
|---|---|---|
| 부팅 전부터 E-Stop active | Motor Enable/STBY 비활성 유지 | NOT RUN |
| E-Stop active + 반복 enable command | ControlTask가 로컬 차단을 덮어쓰지 않음 | NOT RUN |
| E-Stop active + CAN 단절/RTOS 부하 | CAN 수신/Task 실행을 기다리지 않고 로컬 차단 | NOT RUN |
| E-Stop 해제 | 단순 해제만으로 재구동하지 않음; DEC-CTRL-006 확정 조건 적용 | NOT RUN |

# 1. Test Objective

Drive + Steering ECU가 RF/가변저항 Driver 입력을 읽어 `Driver_Input`으로 발행하고, VCU의 최종 Command를 받아 Motor/Servo 출력으로 변환하며, 모터 명령값 기반 추정 함수로 Speed/RPM 표시값을 산출하는지 확인한다. 또한 Command Timeout, Invalid Input, CAN burst 등의 조건에서도 ControlTask가 정의된 주기 안에서 동작하고 안전한 상태 전환 및 Health 정보를 제공하는지 검증한다.

초기 Motor/Servo 시험은 낮은 출력의 bench 조건에서 수행하며, 전체 차량 주행 시험보다 먼저 단독 기능과 timeout 동작을 검증한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / MCU | STM32G431KB (STM32 #2), 구매 모델 확정 / 실기 NOT RUN |
| RTOS | FreeRTOS version TBD |
| CMSIS-RTOS API | CMSIS-RTOS2 |
| Motor | TBD |
| Motor Driver | TB6612FNG 후보, 최종 확정 전 |
| Driver Input 장치 | RF 수신기 또는 가변저항, `DEC-HW-024` 확정 전 |
| Steering Servo | TBD |
| Power | 실제 시험 시 기록 |
| CAN Interface | FDCAN 또는 실제 보드 지원 구조 TBD |
| CAN Bitrate | TBD |
| Debug | STM32CubeIDE / ST-Link / UART 후보 |
| Measurement | Logic Analyzer / Oscilloscope / CAN logger 후보 |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|
| Motor Driver PWM | TBD | STM32 TIM PWM | actual pin TBD |
| Motor Driver DIR | TBD | STM32 GPIO | actual pin TBD |
| Motor Driver STBY/Enable | TBD | STM32 GPIO | safe init 확인 |
| Driver Input (accel/brake/steer) | TBD | TIM Input Capture / ADC | 채택 장치에 따라 확정 |
| Servo PWM | TBD | STM32 TIM PWM | actual servo spec 기준 |
| CAN FD Transceiver | TBD | STM32 FDCAN | actual board support 확인 |

사진/회로/핀맵 링크: TBD

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-DRV-001 | REQ-DRV-001 | RF/가변저항/Gear 신호 입력 | `Driver_Input` CAN 발행 | NOT RUN | TBD |
| T-DRV-001a | REQ-DRV-001a | E-Stop 활성화 (CAN 연결 끊은 상태) | CAN 없이도 Motor Driver 즉시 disable | NOT RUN | TBD |
| T-DRV-001b | REQ-DRV-001b | E-Stop 활성화 | `Driver_Input.estop_status=true` CAN 발행 | NOT RUN | TBD |
| T-DRV-002 | REQ-DRV-002 | Dummy/real CAN command | Drive/Steering command 수신 | NOT RUN | TBD |
| T-DRV-003 | REQ-DRV-003 | invalid/out-of-range command | actuator에 직접 적용되지 않음 | NOT RUN | TBD |
| T-DRV-004 | REQ-DRV-004 | `Drive_Enable=false` | Motor safe state | NOT RUN | TBD |
| T-DRV-005 | REQ-DRV-005 | speed command step | PWM/DIR mapping | NOT RUN | TBD |
| T-DRV-006 | REQ-DRV-006 | steering min/center/max | calibrated servo command | NOT RUN | TBD |
| T-DRV-007 | REQ-DRV-007 | 명령값 단계 변화 | speed/rpm 추정값 변화, estimated 표기 확인 | NOT RUN | TBD |
| T-DRV-008 | REQ-DRV-008 | VCU command stop | timeout 후 stale command 미유지 | NOT RUN | TBD |
| T-DRV-009 | REQ-DRV-009 | CAN status monitor | Drive_Status/Fault TX | NOT RUN | TBD |
| T-DRV-010 | REQ-DRV-010 | power/reset | unintended motor movement 없음 | NOT RUN | TBD |
| T-DRV-011 | REQ-DRV-011 | architecture/code inspect | Encoder/Hall 하드웨어 의존성 없음 | NOT RUN | TBD |
| T-DRV-012 | REQ-DRV-012 | RTOS inspect | CAN/Control/DriverInput/Status 분리 | NOT RUN | TBD |
| T-DRV-013 | REQ-DRV-013 | load/logging test | ControlTask timing 유지 | NOT RUN | TBD |
| T-DRV-014 | REQ-DRV-014 | health injection | task/queue/timeout health 검출 | NOT RUN | TBD |
| T-DRV-015 | REQ-DRV-015 | watchdog health test | policy대로 refresh/withhold | NOT RUN | TBD |
| T-DRV-016 | REQ-DRV-016 | datasheet/spec review | Motor/Driver 적합성 확인 | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|
| T-DRV-001-A | RF/가변저항 accel 입력 변화 | `Driver_Input.accel` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-001-B | RF/가변저항 brake 입력 변화 | `Driver_Input.brake` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-001-C | RF/가변저항 steer 입력 변화 | `Driver_Input.steer` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-005-A | low speed request | 낮은 Motor PWM | NOT RUN | scope/log TBD | TBD |
| T-DRV-005-B | speed request increase | PWM mapping 증가 | NOT RUN | scope/log TBD | TBD |
| T-DRV-005-C | stop request | Motor output safe/zero policy | NOT RUN | scope/video TBD | TBD |
| T-DRV-006-A | steering left candidate | left calibrated output | NOT RUN | video/scope TBD | TBD |
| T-DRV-006-B | steering center | center output | NOT RUN | video/scope TBD | TBD |
| T-DRV-006-C | steering right candidate | right calibrated output | NOT RUN | video/scope TBD | TBD |
| T-DRV-007-A | PWM 명령값 증가 | 추정 rpm/speed 증가, estimated flag 유지 | NOT RUN | UART/log TBD | TBD |
| T-DRV-009-A | normal active state | Drive_Status periodic TX | NOT RUN | CAN log TBD | TBD |

---

# 5. Boundary / Calibration Test

## 5.1 Motor PWM / 추정 RPM

| Condition | Command | PWM | Estimated RPM | Note | Result |
|---|---:|---:|---:|---|---|
| Stop | TBD | TBD | NOT RUN | | TBD |
| Low | TBD | TBD | NOT RUN | | TBD |
| Mid | TBD | TBD | NOT RUN | | TBD |
| High test limit | TBD | TBD | NOT RUN | bench 범위만 | TBD |

Motor/Driver의 실제 전기적 한계를 확인하기 전 무리하게 최대 출력 시험을 하지 않는다. 여기서 기록하는 RPM은 실측이 아니라 추정 함수 출력값이다.

## 5.2 Steering Calibration

| Position | Command | Servo Output | Mechanical Angle / Position | Note | Result |
|---|---:|---:|---:|---|---|
| Left safe limit | TBD | TBD | NOT RUN | 기구 한계 확인 | TBD |
| Center | TBD | TBD | NOT RUN | 기준점 | TBD |
| Right safe limit | TBD | TBD | NOT RUN | 기구 한계 확인 | TBD |

실제 Servo 사양과 차량 steering linkage를 기준으로 limit을 정한다.

## 5.3 Driver Input Calibration

| Item | Value |
|---|---|
| Driver Input 장치 (RF/가변저항) | TBD (`DEC-HW-024`) |
| Accel 입력 → speed 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| Brake 입력 → speed 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| Brake 감속 감지 → brake_lamp threshold | TBD (`DEC-CTRL-020`) |
| Steer 입력 → steering 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| 매핑 검증됨? | NOT RUN |

## 5.4 Speed/RPM 추정 함수 Calibration

| Item | Value |
|---|---|
| PWM → RPM 추정 함수 형태 | TBD (`DEC-CTRL-021`) |
| PWM → Vehicle Speed 추정 함수 형태 | TBD (`DEC-CTRL-021`) |
| 실측 대비 참고 오차(있는 경우) | N/A — 실측 비교 대상이 아님, 참고용 bench 비교만 가능 |
| 추정 함수 검증됨? | NOT RUN |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-DRV-000 | E-Stop active + CAN cable 분리 | GPIO EXTI (CAN 무관) | Motor Driver Enable/STBY 즉시 disable | NOT RUN | TBD |
| F-DRV-001 | VCU command timeout | last_rx timeout | Motor safe state + fault | NOT RUN | TBD |
| F-DRV-002 | `Drive_Enable=false` while command exists | enable check | output disable | NOT RUN | TBD |
| F-DRV-003 | speed request out-of-range | range validation | reject/clamp policy | NOT RUN | TBD |
| F-DRV-004 | steering request beyond calibrated limit | range validation | mechanical limit 밖 command 금지 | NOT RUN | TBD |
| F-DRV-005 | Driver Input 신호 disconnected | signal timeout | `Driver_Input` invalid, 안전 기본값 | NOT RUN | TBD |
| F-DRV-006 | Driver Input 신호 implausible | plausibility | invalid flag | NOT RUN | TBD |
| F-DRV-007 | CAN burst | queue occupancy | ControlTask deadline 유지 | NOT RUN | TBD |
| F-DRV-008 | CanRxQueue full | RTOS API/counter | overflow health flag | NOT RUN | TBD |
| F-DRV-009 | ControlTask artificial delay | overrun detector | health fault / evidence | NOT RUN | TBD |
| F-DRV-010 | CanTxTask delayed | period monitor | ControlTask 영향 제한 | NOT RUN | TBD |
| F-DRV-011 | debug logging load | timing trace | control jitter 영향 제한 | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| `ControlTask` period | 5~10 ms 후보 | NOT RUN | GPIO toggle / trace / timestamp | TBD |
| `ControlTask` execution time | TBD | NOT RUN | runtime timestamp | TBD |
| `ControlTask` jitter | TBD | NOT RUN | trace/statistics | TBD |
| `DriverInputTask` update | 5~10 ms 후보/event | NOT RUN | timestamp | TBD |
| CAN RX → valid command | TBD | NOT RUN | RX/task timestamp | TBD |
| command → PWM update | TBD | NOT RUN | CAN timestamp + scope | TBD |
| Status CAN period | 20~50 ms 후보 | NOT RUN | CAN log | TBD |
| Command timeout detection | TBD | NOT RUN | CAN stop + timestamp | TBD |

후보 값은 초기 Architecture 가정이며 실제 결과를 보고 Specification/Architecture를 함께 수정한다.

---

# 8. RTOS Test

## 8.1 Task Inventory

| Task | Expected Period / Trigger | Relative Priority | Observed | Result |
|---|---|---|---|---|
| `CanRxTask` | CAN event | High | NOT RUN | TBD |
| `ControlTask` | 5~10 ms 후보 | Highest application | NOT RUN | TBD |
| `DriverInputTask` | event / 5~10 ms 후보 | High | NOT RUN | TBD |
| `CanTxTask` | 20~50 ms 후보 + event | Normal | NOT RUN | TBD |
| `HealthTask` | 50~100 ms 후보 | Low/Normal | NOT RUN | TBD |

## 8.2 Period / Jitter

| Task | Target Period | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|
| ControlTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| DriverInputTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| CanTxTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| HealthTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |

## 8.3 Stack / Memory

| Task / Item | Configured | Minimum Free / High-Water | Target | Result |
|---|---:|---:|---:|---|
| ControlTask stack | TBD | NOT RUN | TBD | TBD |
| CanRxTask stack | TBD | NOT RUN | TBD | TBD |
| DriverInputTask stack | TBD | NOT RUN | TBD | TBD |
| CanTxTask stack | TBD | NOT RUN | TBD | TBD |
| HealthTask stack | TBD | NOT RUN | TBD | TBD |
| Free heap | TBD | NOT RUN | TBD | TBD |

## 8.4 Queue / Event / Notification

| Object | Depth / Config | Max Occupancy / Result | Overflow Test | Result |
|---|---|---|---|---|
| CanRxQueue | TBD | NOT RUN | Planned | TBD |
| CommandQueue/latest object | TBD | NOT RUN | Planned | TBD |
| DriverInputNotify | notification | NOT RUN | Planned | TBD |
| DriverInputQueue | TBD | NOT RUN | Planned | TBD |
| StatusQueue | TBD | NOT RUN | Planned | TBD |

## 8.5 ISR → Task Test

| Interrupt | Expected ISR Action | Expected Task Wake-up | Actual | Result |
|---|---|---|---|---|
| FDCAN RX | enqueue/notify only | CanRxTask | NOT RUN | TBD |
| Driver Input capture (PWM/ADC), 필요 시 | raw sample/timestamp only | DriverInputTask | NOT RUN | TBD |

Code Review 항목:
- [ ] ISR에서 제어 연산 없음
- [ ] ISR에서 `printf` 없음
- [ ] ISR에서 blocking API 없음
- [ ] ISR에서 Servo/Motor 전체 control logic 수행하지 않음

## 8.6 Priority / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| CAN burst + normal control | ControlTask deadline 유지 | NOT RUN | TBD |
| heavy UART logging | control timing 영향 제한 | NOT RUN | TBD |
| CanTxTask delayed | motor/steering output timing 유지 | NOT RUN | TBD |
| Driver Input event burst | deadlock/starvation 없음 | NOT RUN | TBD |

## 8.7 Watchdog / Health Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all critical tasks healthy | watchdog refresh 후보 | NOT RUN | TBD |
| ControlTask health missing | health fault / refresh 중단 정책 | NOT RUN | TBD |
| command timeout | safe output + health fault | NOT RUN | TBD |
| queue overflow | counter/fault 반영 | NOT RUN | TBD |
| stack low watermark | health warning 후보 | NOT RUN | TBD |

Watchdog reset 시험은 bench 상태에서 수행하고, Motor/Servo가 안전 상태인지 먼저 확인한다.

---

# 9. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Driver_Input` | TX | accel/brake/steer 발행 | NOT RUN | N/A | TBD |
| `Final_Speed_Request` | RX | command update | NOT RUN | Planned | TBD |
| `Final_Steering_Request` | RX | steering update | NOT RUN | Planned | TBD |
| `Drive_Enable` | RX | output enable/disable | NOT RUN | Planned | TBD |
| `Vehicle_Gear` | RX | drive context | NOT RUN | Planned | TBD |
| `Motor_RPM` (estimated) | TX | 추정 rpm | NOT RUN | N/A | TBD |
| `Drive_Status` | TX | state/health | NOT RUN | N/A | TBD |
| `Steering_Status` | TX | target/valid | NOT RUN | N/A | TBD |
| `ECU_Heartbeat` | TX | node alive | NOT RUN | N/A | TBD |
| `DTC_Event` | TX | local fault event | NOT RUN | N/A | TBD |

## LIN

N/A.

---

# 10. DTC / Diagnostics Test

| Fault | Expected DTC / Status | H735 Active 표시? | 해소 시 제거? | Result |
|---|---|---|---|---|
| VCU command timeout | `DRV_COMM_TIMEOUT` 후보 | NOT RUN | NOT RUN | TBD |
| Driver Input timeout | `DRV_INPUT_TIMEOUT` 후보 | NOT RUN | NOT RUN | TBD |
| ControlTask overrun | `DRV_TASK_OVERRUN` 후보 | NOT RUN | NOT RUN | TBD |
| CAN fault | `DRV_CAN_FAULT` 후보 | NOT RUN | NOT RUN | TBD |

실제 DTC code와 status lifecycle은 F/DTC 통합 규격을 따른다.

---

# 11. Soak / Load Test

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Normal low-output bench soak | TBD | reset/deadlock/overflow 없음 | NOT RUN | TBD |
| CAN burst + periodic control | TBD | ControlTask timing 유지 | NOT RUN | TBD |
| Driver Input activity + CAN + status | TBD | queue/stack 정상 | NOT RUN | TBD |
| Repeated enable/disable | TBD | stale/unsafe output 없음 | NOT RUN | TBD |
| Repeated steering command | TBD | calibrated range 유지 | NOT RUN | TBD |

---

# 12. Logs / Evidence

- UART / Console Log: TBD
- Wiring Photo: TBD
- Motor/Servo Test Video: TBD
- CAN Log: TBD
- PWM Scope Capture: TBD
- Driver Input Capture Trace: TBD
- FreeRTOS Runtime Stats: TBD
- Stack High-Water Log: TBD
- Queue Occupancy/Overflow Log: TBD

예시 로그 형식:

```text
[DRV][INIT] output=safe
[DRV][INPUT] accel=... brake=... steer=... valid=1
[DRV][CAN] speed_req=... steer_req=... enable=1
[DRV][CTRL] pwm=... dir=... servo=...
[DRV][EST] rpm_estimated=... speed_estimated=...
[DRV][TIMEOUT] VCU command stale
[DRV][SAFE] motor_output=0
[DRV][HEALTH] control_alive=1 queue_overflow=0
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

- [ ] Motor output이 안전한 초기 상태에서 시작한다.
- [ ] 낮은 출력 bench test에서 PWM/DIR 동작을 재현한다.
- [ ] Servo center/left/right calibration을 기록한다.
- [ ] RF/가변저항 입력을 읽어 `Driver_Input`을 CAN으로 발행한다.
- [ ] 명령값 기반 speed/rpm 추정값을 산출하고 estimated임을 확인한다.
- [ ] VCU CAN command를 받아 output에 반영한다.
- [ ] Command timeout에서 stale command를 유지하지 않는다.
- [ ] Drive_Status / Motor_RPM(estimated) / Steering_Status CAN TX를 확인한다.
- [ ] ControlTask/DriverInputTask period/jitter를 측정한다.
- [ ] Stack high-water를 확인한다.
- [ ] Queue occupancy/overflow 정책을 확인한다.
- [ ] ISR→Task 구조를 확인한다.
- [ ] logging/CAN burst에서 ControlTask starvation이 없다.
- [ ] Watchdog/Health 정책을 확인한다, 해당 시.
- [ ] Motor/Driver 정격 적합성 검토를 완료한다.
- [ ] 로그/사진/영상/trace 증거를 남긴다.

## Remaining Issues

- 실제 Motor/Driver 정격 확정 필요
- RF 수신기 vs 가변저항 최종 선택 필요 (`DEC-HW-024`)
- 입력값→speed/steering 선형 매핑 계수 확정 필요 (`DEC-CTRL-019`)
- Motor 명령값→speed/rpm 추정 함수 형태 확정 필요 (`DEC-CTRL-021`)
- Brake 감속 감지→brake_lamp threshold 확정 필요 (`DEC-CTRL-020`)
- Servo calibration 필요
- STM32/FDCAN pin map 확정 필요
- CAN Matrix 확정 필요
- ControlTask/DriverInputTask 실제 주기 확정 필요
- Command timeout 및 steering timeout policy 확정 필요
