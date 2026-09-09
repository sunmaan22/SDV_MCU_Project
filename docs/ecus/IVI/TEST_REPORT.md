# Cluster + IVI Cockpit Test Report

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md` 요구사항을 실제 시험으로 검증한다.  
> 현재는 실행 전 계획 상태이므로 실제 측정값은 임의로 채우지 않는다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Cluster + IVI Cockpit |
| Owner | B |
| Board / Platform | STM32H735 + TouchGFX |
| Execution Model | FreeRTOS + CMSIS-RTOS2 |
| Firmware / SW Commit | TBD |
| Test Date | TBD |
| Specification Revision | v0.2 |
| Architecture Revision | v0.2 |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial planned test |
| v0.2 | 2026-09-09 | Team | RTOS timing/stack/queue/watchdog tests added |

---

# 1. Test Objective

H735 Cockpit이 Dummy Data와 실제 CAN 데이터를 이용해 Cluster/ADAS/Parking/Diagnostics/Settings 화면을 정상 표시하는지 검증한다. 동시에 FreeRTOS 기반 `CanRxTask`, `VehicleModelTask`, `GuiTask`, `CommandTxTask`, `HealthTask`가 의도한 구조로 실행되고, CAN burst나 UI load에서도 queue overflow, stack overflow, starvation 없이 주요 Timing 요구사항을 만족하는지 확인한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board | STM32H735 board |
| RTOS | FreeRTOS, version TBD |
| API | CMSIS-RTOS2 |
| UI | TouchGFX |
| Interface | LCD / Touch / FDCAN |
| CAN bitrate | TBD |
| Debug | STM32CubeIDE / ST-Link / runtime stats 후보 |
| Watchdog | IWDG policy TBD |

---

# 3. Requirement Verification Matrix

| Test ID | Requirement | Expected | Result |
|---|---|---|---|
| T-HMI-001 | REQ-HMI-001 | Speed/RPM/Gear 표시 | NOT RUN |
| T-HMI-002 | REQ-HMI-002 | READY/Warning 표시 | NOT RUN |
| T-HMI-003 | REQ-HMI-003 | ADAS 상태 표시 | NOT RUN |
| T-HMI-004 | REQ-HMI-004 | Parking 거리/Warning 표시 | NOT RUN |
| T-HMI-005 | REQ-HMI-005 | DTC list/detail | NOT RUN |
| T-HMI-006 | REQ-HMI-006 | Touch 화면 전환 | NOT RUN |
| T-HMI-007 | REQ-HMI-007 | timeout data invalid 표시 | NOT RUN |
| T-HMI-008 | REQ-HMI-008 | Body_Command CAN TX | NOT RUN |
| T-HMI-009 | REQ-HMI-009 | raw camera CAN path 없음 | NOT RUN |
| T-HMI-010 | REQ-HMI-010 | critical warning 우선 표시 | NOT RUN |
| T-HMI-011 | REQ-HMI-011 | CAN→Model ≤100 ms 목표 | NOT RUN |
| T-HMI-012 | REQ-HMI-012 | Touch≤150 ms 목표 | NOT RUN |
| T-HMI-013 | REQ-HMI-013 | CAN/GUI task 분리 | NOT RUN |
| T-HMI-014 | REQ-HMI-014 | FDCAN ISR 최소 처리 | NOT RUN |
| T-HMI-015 | REQ-HMI-015 | Queue/Repository 전달 | NOT RUN |
| T-HMI-016 | REQ-HMI-016 | load 중 critical warning block 없음 | NOT RUN |
| T-HMI-017 | REQ-HMI-017 | stack/queue overflow 검증 | NOT RUN |
| T-HMI-018 | REQ-HMI-018 | HealthTask/watchdog-ready 구조 | NOT RUN |

---

# 4. Stage 1 Dummy UI Test

| Test | Input | Expected | Actual | Result |
|---|---|---|---|---|
| Cluster | speed=24, rpm=1250, gear=D | 값 표시 | NOT RUN | TBD |
| Parking | RR=180 mm, CRITICAL | right critical UI | NOT RUN | TBD |
| DTC | 2 entries | list/detail | NOT RUN | TBD |
| Screen Flow | 5개 화면 이동 | hang 없이 전환 | NOT RUN | TBD |
| Warning Overlay | Settings + CRITICAL injection | warning 우선 표시 | NOT RUN | TBD |

Stage 1에서도 가능하면 DummyDataProvider가 직접 GUI를 건드리지 않고 Model update 경로를 통과하게 한다.

---

# 5. CAN Integration Test

| Message | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Vehicle_State` | RX | gear/mode update | NOT RUN | planned | TBD |
| `Drive_Status` | RX | speed/rpm update | NOT RUN | planned | TBD |
| `Ultrasonic_Status` | RX | distance/warning | NOT RUN | planned | TBD |
| Vision status | RX | ADAS/Parking update | NOT RUN | planned | TBD |
| `Body_Status` | RX | lamp/ambient | NOT RUN | planned | TBD |
| `DTC_Event` | RX | DTC model update | NOT RUN | event | TBD |
| `Body_Command` | TX | UI request transmitted | NOT RUN | N/A | TBD |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected | Actual | Result |
|---|---|---|---|---|
| F-HMI-001 | Drive timeout | speed/rpm invalid + warning | NOT RUN | TBD |
| F-HMI-002 | Ultrasonic valid=false | Sensor Invalid | NOT RUN | TBD |
| F-HMI-003 | Vision timeout | Vision Unavailable | NOT RUN | TBD |
| F-HMI-004 | Unknown DTC | raw code/source 표시 | NOT RUN | TBD |
| F-HMI-005 | Touch 연타 | GUI freeze 없음 | NOT RUN | TBD |
| F-HMI-006 | CAN unavailable | Comm Fault | NOT RUN | TBD |
| F-HMI-007 | CanRxQueue overflow injection | counter/health policy | NOT RUN | TBD |
| F-HMI-008 | GuiTask artificial load | CAN model ingestion 유지 | NOT RUN | TBD |

---

# 7. RTOS Task Test

## 7.1 Task Inventory

| Task | Expected Trigger / Period | Priority Direction | Observed | Result |
|---|---|---|---|---|
| `CanRxTask` | event | High | NOT RUN | TBD |
| `VehicleModelTask` | event / 10~20 ms 후보 | Normal~High | NOT RUN | TBD |
| `GuiTask` | TouchGFX tick | Normal | NOT RUN | TBD |
| `CommandTxTask` | event | Normal | NOT RUN | TBD |
| `HealthTask` | 100 ms 후보 | Low | NOT RUN | TBD |

## 7.2 Period / Jitter

| Task | Target | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|
| VehicleModelTask periodic mode, 사용 시 | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| GuiTask effective update | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| HealthTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |

## 7.3 Stack / Memory

| Task / Item | Configured | High-Water / Minimum Free | Result |
|---|---:|---:|---|
| CanRxTask stack | TBD | NOT RUN | TBD |
| VehicleModelTask stack | TBD | NOT RUN | TBD |
| GuiTask stack | generated/TBD | NOT RUN | TBD |
| CommandTxTask stack | TBD | NOT RUN | TBD |
| HealthTask stack | TBD | NOT RUN | TBD |
| Heap free | TBD | NOT RUN | TBD |

## 7.4 Queue / Event

| Object | Depth | Max Occupancy | Overflow Test | Result |
|---|---:|---:|---|---|
| `CanRxQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `ModelUpdateQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `UiCommandQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `SystemEvents` | flags | N/A | NOT RUN | TBD |

## 7.5 ISR → Task

| Interrupt | Expected ISR Action | Expected Task | Actual | Result |
|---|---|---|---|---|
| FDCAN RX | enqueue/notify only | CanRxTask | NOT RUN | TBD |
| Touch/BSP IRQ | framework event only | GuiTask | NOT RUN | TBD |

Code Review에서 ISR 내부 decode/render/printf가 없는지 확인한다.

---

# 8. Load / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| CAN burst + Cluster rendering | CanRxQueue overflow 0, GUI freeze 0 | NOT RUN | TBD |
| DTC list update + critical warning | warning path 지연 최소 | NOT RUN | TBD |
| Touch 연속 입력 + CAN RX | both continue | NOT RUN | TBD |
| Debug log enabled | timing target 유지 또는 영향 기록 | NOT RUN | TBD |

---

# 9. Timing Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| CAN RX → Vehicle Model | ≤100 ms 목표 | NOT RUN | timestamps | TBD |
| Critical warning → UI | ≤200 ms 목표 | NOT RUN | injection/display timestamp | TBD |
| Touch → UI response | ≤150 ms 목표 | NOT RUN | touch/render timestamp | TBD |
| Queue backlog recovery | TBD | NOT RUN | burst test | TBD |

---

# 10. Health / Watchdog Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all task heartbeat healthy | HealthTask healthy | NOT RUN | TBD |
| CanRxTask heartbeat missing | fault state, watchdog policy 적용 후보 | NOT RUN | TBD |
| GuiTask heartbeat missing | HMI task fault | NOT RUN | TBD |
| Queue overflow | health counter 증가 | NOT RUN | TBD |
| Stack low watermark | warning/diagnostic candidate | NOT RUN | TBD |

실제 IWDG reset 시험은 bench 상태에서만 수행하고, 구현 전에는 논리/health flag 검증부터 한다.

---

# 11. DTC / Diagnostics Test

| Fault | Expected Status | Pi Stored? | H735 Displayed? | Result |
|---|---|---|---|---|
| Ultrasonic timeout | sensor DTC | NOT RUN | NOT RUN | TBD |
| Body LIN fault | Body DTC | NOT RUN | NOT RUN | TBD |
| Vision fault | HPC DTC | NOT RUN | NOT RUN | TBD |
| HMI queue/task fault 후보 | HMI local health/DTC | NOT RUN | NOT RUN | TBD |

---

# 12. Evidence

- UART log: TBD
- TouchGFX screenshot/video: TBD
- CAN log: TBD
- Runtime stats: TBD
- stack high-water log: TBD
- queue occupancy log: TBD
- trace/scope: TBD

예시 로그:

```text
[RTOS][TASK] CanRx alive
[RTOS][QUEUE] CanRxQueue high=4/16
[RTOS][STACK] GuiTask watermark=TBD
[CAN][RX] Drive_Status
[MODEL] speed=24 rpm=1250
[HMI][WARN] PARKING_CRITICAL
```

---

# 13. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] 주요 UI 기능 정상
- [ ] CAN RX/TX 정상
- [ ] timeout/invalid 정상
- [ ] CanRx/Model/Gui/Command/Health Task 정상
- [ ] ISR 최소 처리 확인
- [ ] 예상 부하에서 Queue overflow 0
- [ ] Stack 여유 측정
- [ ] critical warning load test 통과
- [ ] Timing 목표 측정
- [ ] Health/Watchdog 정책 검증
- [ ] 증거 저장

## Remaining Issues

- CAN signal layout
- FDCAN transceiver/pin
- task numeric priority
- task stack size
- queue depth
- IWDG policy
- DTC Clear protocol
