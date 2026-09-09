# HPC + Camera Vision Test Report

> 목적: `SPECIFICATION.md`의 요구사항을 실제 시험으로 검증한다.  
> 현재는 **시험 전 계획 상태**이므로 측정하지 않은 값은 `NOT RUN` / `TBD`로 남긴다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | HPC + Front/Rear Camera Vision |
| Owner | E |
| Board / Platform | Raspberry Pi + Linux |
| Execution Model | Linux Service / Process / Thread |
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

Front/Rear Camera capture, Vision semantic result 생성, Gear D/R 기반 mode switching, CAN result transmission, camera/process 장애 처리, Pi 1대 통합 성능을 검증한다. 또한 FPS, processing latency, CPU, memory, queue backlog, thermal 상태를 측정한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / Pi | Raspberry Pi 모델 TBD |
| OS | Linux distro/version TBD |
| Front Camera | TBD |
| Rear Camera | TBD |
| Front Interface | CSI 후보 |
| Rear Interface | USB 후보 |
| CAN FD Interface | TBD |
| Vision Library / Model | TBD |
| Resolution / FPS | TBD |
| Debug / Metrics | console/log/system metrics 후보 |

## Setup

```text
Stage 1:
Pi #1 + Front Camera
Pi #2 + Rear Camera

Final Integration:
Front Camera + Rear Camera
        ↓
   Raspberry Pi 1대
        ↓
     CAN FD
```

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-VIS-001 | REQ-VIS-001 | Front capture test | 안정적 frame 획득 | NOT RUN | TBD |
| T-VIS-002 | REQ-VIS-002 | Rear capture test | 안정적 frame 획득 | NOT RUN | TBD |
| T-VIS-003 | REQ-VIS-003 | Front Vision test | lane/object semantic result | NOT RUN | TBD |
| T-VIS-004 | REQ-VIS-004 | Rear Vision test | object/position/warning result | NOT RUN | TBD |
| T-VIS-005 | REQ-VIS-005 | Gear D/R state injection | 올바른 Vision mode | NOT RUN | TBD |
| T-VIS-006 | REQ-VIS-006 | D→R switching timing | Rear first valid result 측정 | NOT RUN | TBD |
| T-VIS-007 | REQ-VIS-007 | CAN/code inspection | semantic result만 TX | NOT RUN | TBD |
| T-VIS-008 | REQ-VIS-008 | architecture/code inspection | PWM 직접 제어 없음 | NOT RUN | TBD |
| T-VIS-009 | REQ-VIS-009 | camera disconnect | valid=false/fault | NOT RUN | TBD |
| T-VIS-010 | REQ-VIS-010 | process kill/restart | fault detect/recovery | NOT RUN | TBD |
| T-VIS-011 | REQ-VIS-011 | Pi 2대 독립 실행 | Front/Rear 독립 시험 가능 | NOT RUN | TBD |
| T-VIS-012 | REQ-VIS-012 | Pi 1대 integration | Front/Rear 통합 실행 | NOT RUN | TBD |
| T-VIS-013 | REQ-VIS-013 | timestamp inspect | freshness 판단 가능 | NOT RUN | TBD |
| T-VIS-014 | REQ-VIS-014 | CAN service failure | Vision 전체 불필요 중단 없음 | NOT RUN | TBD |
| T-VIS-015 | REQ-VIS-015 | performance measurement | FPS/latency/CPU/memory 기록 | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|
| T-VIS-001 | Front Camera active | continuous frames | NOT RUN | TBD | TBD |
| T-VIS-002 | Rear Camera active | continuous frames | NOT RUN | TBD | TBD |
| T-VIS-003 | Front lane/object scene | defined lane/object result | NOT RUN | TBD | TBD |
| T-VIS-004 | Rear object scene | object/position/warning result | NOT RUN | TBD | TBD |
| T-VIS-005A | Gear D | Front ACTIVE / Rear IDLE 후보 | NOT RUN | TBD | TBD |
| T-VIS-005B | Gear R | Rear ACTIVE / Front PAUSE 후보 | NOT RUN | TBD | TBD |
| T-VIS-007 | Valid Vision result | CAN semantic result TX | NOT RUN | TBD | TBD |

---

# 5. Vision Accuracy / Boundary Test

정확도 평가는 최종 algorithm이 정해진 뒤 구체화한다.

| Scenario | Reference / Ground Truth | Expected | Actual | Result |
|---|---|---|---|---|
| Lane centered | TBD | near-center result | NOT RUN | TBD |
| Lane left/right offset | TBD | offset direction 일치 | NOT RUN | TBD |
| Front object present | annotated/reference | detected | NOT RUN | TBD |
| Front object absent | reference | no false positive 목표 | NOT RUN | TBD |
| Rear object left/center/right | reference | position 분류 | NOT RUN | TBD |
| Low light / blur 후보 | test condition | degraded/valid policy 확인 | NOT RUN | TBD |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Recovery / Behavior | Actual | Result |
|---|---|---|---|---|---|
| F-VIS-001 | Front Camera disconnect | frame timeout | front valid=false | NOT RUN | TBD |
| F-VIS-002 | Rear Camera disconnect | frame timeout | rear valid=false | NOT RUN | TBD |
| F-VIS-003 | Front Vision process kill | process health | fault + restart 후보 | NOT RUN | TBD |
| F-VIS-004 | Rear Vision process kill | process health | fault + restart 후보 | NOT RUN | TBD |
| F-VIS-005 | CAN service down | socket/service error | local Vision 유지, publish unavailable | NOT RUN | TBD |
| F-VIS-006 | IPC queue full | queue metric | defined drop policy | NOT RUN | TBD |
| F-VIS-007 | inference slowdown | latency/stale age | stale result invalid/degraded | NOT RUN | TBD |
| F-VIS-008 | rapid D↔R changes | mode state | final gear와 일치하는 active mode | NOT RUN | TBD |
| F-VIS-009 | bad config/model path | startup validation | explicit startup fault | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| Front Capture FPS | TBD | NOT RUN | frame timestamps | TBD |
| Rear Capture FPS | TBD | NOT RUN | frame timestamps | TBD |
| Front processing latency | TBD | NOT RUN | frame→result timestamp | TBD |
| Rear processing latency | TBD | NOT RUN | frame→result timestamp | TBD |
| Result publish latency | TBD | NOT RUN | result→CAN TX timestamp | TBD |
| Gear D→R mode switch | TBD | NOT RUN | gear event→mode state | TBD |
| Gear R→Rear first frame | TBD | NOT RUN | gear event→frame timestamp | TBD |
| Gear R→Rear first valid result | TBD | NOT RUN | gear event→result timestamp | TBD |
| Result age / freshness | TBD | NOT RUN | timestamp difference | TBD |

---

# 8. Linux Process / IPC Test

## 8.1 Process Inventory

| Process / Service | Expected State | Observed | Restart Policy | Result |
|---|---|---|---|---|
| `front_vision` | running/active by mode | NOT RUN | TBD | TBD |
| `rear_vision` | running/active by mode | NOT RUN | TBD | TBD |
| `vehicle_manager` | running | NOT RUN | TBD | TBD |
| `can_service` | running | NOT RUN | TBD | TBD |
| `health_monitor` | running | NOT RUN | TBD | TBD |
| `logger` | running | NOT RUN | TBD | TBD |

## 8.2 IPC / Queue

| Object | Configured Depth | Max Occupancy | Overflow Test | Result |
|---|---:|---:|---|---|
| Front Frame Queue | TBD | NOT RUN | planned | TBD |
| Rear Frame Queue | TBD | NOT RUN | planned | TBD |
| Front Result Queue | TBD | NOT RUN | planned | TBD |
| Rear Result Queue | TBD | NOT RUN | planned | TBD |
| CAN TX Queue | TBD | NOT RUN | planned | TBD |
| Log Queue | TBD | NOT RUN | planned | TBD |

확인 항목:
- 오래된 frame이 무한정 쌓이지 않는가
- queue full 시 정책대로 drop/fault 처리하는가
- logger가 Vision processing을 block하지 않는가

## 8.3 Process Failure / Recovery

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| kill front_vision | front invalid + supervisor action | NOT RUN | TBD |
| kill rear_vision | rear invalid + supervisor action | NOT RUN | TBD |
| kill can_service | Vision local pipeline 유지 가능 | NOT RUN | TBD |
| logger failure | Vision critical path 유지 | NOT RUN | TBD |

---

# 9. Resource Test

| Metric | Idle | Front Only | Rear Only | Final Integrated | Result |
|---|---:|---:|---:|---:|---|
| CPU usage | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| Memory RSS | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| Temperature | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| Dropped Frames | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| Queue High-Water | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |

Thermal throttling 여부도 장시간 시험에서 확인한다.

---

# 10. CAN / Communication Test

| Message / Signal | Direction | Expected | Actual | Timeout/Fault Test | Result |
|---|---|---|---|---|---|
| `Vehicle_State` | RX | Gear/Mode update | NOT RUN | Planned | TBD |
| `Vision_Request` | TX | semantic/request publish | NOT RUN | Planned | TBD |
| `Vision_Status` 후보 | TX | valid/health publish | NOT RUN | Planned | TBD |
| `ECU_Heartbeat` 후보 | TX | HPC alive | NOT RUN | Planned | TBD |

검증:
- raw image payload를 CAN으로 보내지 않는가
- invalid/stale result를 정상 request로 보내지 않는가
- CAN failure가 camera process 전체 crash로 이어지지 않는가

---

# 11. DTC / Diagnostics Test

| Fault | Expected Status / DTC Candidate | Pi Manager Stored? | H735 Displayed? | Result |
|---|---|---|---|---|
| Front Camera timeout | `VIS_FRONT_CAMERA_xxx` 후보 | NOT RUN | NOT RUN | TBD |
| Rear Camera timeout | `VIS_REAR_CAMERA_xxx` 후보 | NOT RUN | NOT RUN | TBD |
| Vision service crash | `VIS_*_SERVICE_xxx` 후보 | NOT RUN | NOT RUN | TBD |
| CAN interface fault | `HPC_CAN_xxx` 후보 | NOT RUN | NOT RUN | TBD |
| Excessive latency | `VIS_LATENCY_xxx` 후보 | NOT RUN | NOT RUN | TBD |

실제 DTC code는 F 담당의 공통 규격에 맞춰 확정한다.

---

# 12. Soak / Load Test

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Front Vision soak | TBD | crash/frame backlog 없음 | NOT RUN | TBD |
| Rear Vision soak | TBD | crash/frame backlog 없음 | NOT RUN | TBD |
| Pi 1대 Front/Rear integration | TBD | resource 안정 | NOT RUN | TBD |
| D↔R repeated switching | TBD cycles | camera/service state 일관성 | NOT RUN | TBD |
| CAN traffic + Vision load | TBD | processing 안정 | NOT RUN | TBD |
| Logging enabled | TBD | latency 영향 제한 | NOT RUN | TBD |

---

# 13. Logs / Evidence

- Front/Rear sample video: TBD
- Vision overlay screenshot: TBD
- FPS/latency log: TBD
- CPU/memory/temperature log: TBD
- process supervisor log: TBD
- CAN log: TBD
- queue occupancy metric: TBD

예시 로그 형식:

```text
[MODE] gear=D front=ACTIVE rear=IDLE
[CAM][FRONT] frame=1024 ts=...
[VIS][FRONT] lane_offset=... latency_ms=...
[CAN][TX] Vision_Request valid=1
[MODE] gear=R front=PAUSE rear=ACTIVE
[CAM][REAR] first_frame latency_ms=...
[HEALTH][REAR] frame_timeout
```

---

# 14. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

---

# 15. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Front Camera capture 확인
- [ ] Rear Camera capture 확인
- [ ] Front semantic result 확인
- [ ] Rear semantic result 확인
- [ ] Gear D/R switching 확인
- [ ] raw frame CAN 전송 없음 확인
- [ ] semantic result CAN TX 확인
- [ ] camera disconnect fault 확인
- [ ] process crash/recovery 확인
- [ ] FPS/latency 측정
- [ ] CPU/memory/temperature 측정
- [ ] queue backlog/overflow 정책 확인
- [ ] Pi 1대 integration load 확인
- [ ] 반복/soak test 증거 확보

## Remaining Issues

- 실제 Camera 모델 확정 필요
- Vision Algorithm/Model 확정 필요
- CAN FD adapter 확정 필요
- CAN Matrix 확정 필요
- IPC/Supervisor 구현 방식 확정 필요
- 성능 목표는 baseline 측정 후 확정 필요
