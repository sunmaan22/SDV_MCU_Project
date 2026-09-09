# Cluster + IVI Cockpit Test Report

> 목적: `SPECIFICATION.md`의 요구사항을 실제 시험으로 검증하기 위한 예시 문서다.  
> **현재는 실행 전 계획 상태이므로 실제 측정값을 임의로 채우지 않는다.** 시험 후 `NOT RUN`을 실제 결과로 교체한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Cluster + IVI Cockpit |
| Owner | B |
| Board / Platform | STM32H735 + TouchGFX |
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

H735 Cockpit이 Dummy Data와 실제 CAN 데이터를 이용해 Cluster/ADAS/Parking/Diagnostics/Settings 화면을 정상 표시하는지 확인한다. 또한 CAN timeout, invalid signal, critical warning 같은 비정상 조건에서도 UI가 멈추지 않고 올바른 상태를 표시하는지 검증한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / MCU / Pi | STM32H735 board |
| Sensor / Actuator | N/A, Stage 1은 Dummy Data 사용 |
| Power | Board specification 기준, 실제 시험 시 기록 |
| Interface | LCD / Touch / FDCAN |
| CAN/LIN Bitrate | TBD, CAN Matrix 확정 후 기록 |
| Camera Resolution/FPS | N/A, H735는 raw camera frame 미수신 |
| Tool / Debug Interface | STM32CubeIDE / TouchGFX / ST-Link 후보 |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|
| LCD | Board integrated | STM32H735 | board config 사용 |
| Touch | Board integrated | STM32H735 | board config 사용 |
| CAN FD Transceiver | TBD | H735 FDCAN | Stage 2 통합 시 작성 |

사진/그림 링크: TBD

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-HMI-001 | REQ-HMI-001 | Dummy/real status 입력 | Speed/RPM/Gear 표시 | NOT RUN | TBD |
| T-HMI-002 | REQ-HMI-002 | READY/Warning 상태 변경 | 표시 상태 변경 | NOT RUN | TBD |
| T-HMI-003 | REQ-HMI-003 | Dummy Vision status 입력 | ADAS 화면 갱신 | NOT RUN | TBD |
| T-HMI-004 | REQ-HMI-004 | Dummy Ultrasonic status 입력 | Parking 거리/Warning 표시 | NOT RUN | TBD |
| T-HMI-005 | REQ-HMI-005 | Dummy DTC list 입력 | DTC list/detail 표시 | NOT RUN | TBD |
| T-HMI-006 | REQ-HMI-006 | Touch 화면 전환 | 5개 화면 이동 | NOT RUN | TBD |
| T-HMI-007 | REQ-HMI-007 | CAN timeout simulation | Invalid/Comm warning 표시 | NOT RUN | TBD |
| T-HMI-008 | REQ-HMI-008 | Lighting UI event | `Body_Command` TX 요청 | NOT RUN | TBD |
| T-HMI-009 | REQ-HMI-009 | Architecture/code inspection | Raw camera CAN path 없음 | NOT RUN | TBD |
| T-HMI-010 | REQ-HMI-010 | Critical warning injection | 현재 화면에서 warning 표시 | NOT RUN | TBD |
| T-HMI-011 | REQ-HMI-011 | timestamp measurement | ≤100 ms 목표 | NOT RUN | TBD |
| T-HMI-012 | REQ-HMI-012 | touch timestamp measurement | ≤150 ms 목표 | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|
| T-HMI-001 | speed=24, rpm=1250, gear=D | Cluster에 값 표시 | NOT RUN | TBD | TBD |
| T-HMI-002 | READY=true, general_warning=false | READY 표시, warning 없음 | NOT RUN | TBD | TBD |
| T-HMI-003 | ADAS active + object warning | ADAS 화면에 상태 표시 | NOT RUN | TBD | TBD |
| T-HMI-004 | RL=420 mm, RR=180 mm, warning=CRITICAL | Parking 화면에 우측 critical 표시 | NOT RUN | TBD | TBD |
| T-HMI-005 | DTC 2개 입력 | list count=2, 상세 진입 가능 | NOT RUN | TBD | TBD |
| T-HMI-006 | Cluster→ADAS→Parking→Diagnostics→Settings | 각 화면 정상 전환 | NOT RUN | TBD | TBD |

---

# 5. Boundary / Calibration Test

H735 Cockpit은 센서 calibration owner가 아니므로 센서 raw calibration 대신 UI 표시 경계를 시험한다.

| Condition | Input | Expected Display | Actual | Note | Result |
|---|---|---|---|---|---|
| Speed zero | 0 | 0 표시 | NOT RUN | | TBD |
| RPM zero | 0 | 0 표시 | NOT RUN | | TBD |
| Max display candidate | signal max | layout overflow 없음 | NOT RUN | 실제 range 확정 후 | TBD |
| Parking warning transition | SAFE→WARNING→CRITICAL | 단계별 UI 변경 | NOT RUN | | TBD |
| DTC list empty | count=0 | No Active DTC 상태 | NOT RUN | | TBD |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-HMI-001 | `Drive_Status` timeout | ValidityManager timeout | Speed/RPM invalid + Comm Warning | NOT RUN | TBD |
| F-HMI-002 | Ultrasonic valid=false | invalid flag | 거리 대신 Sensor Invalid | NOT RUN | TBD |
| F-HMI-003 | Vision timeout | source timeout | Vision Unavailable 표시 | NOT RUN | TBD |
| F-HMI-004 | Unknown DTC code | lookup miss | Raw code/source 표시 | NOT RUN | TBD |
| F-HMI-005 | Touch 연타 | event validation | UI freeze 없이 처리/무시 | NOT RUN | TBD |
| F-HMI-006 | CAN bus unavailable | CAN error state | Communication Fault 표시 | NOT RUN | TBD |
| F-HMI-007 | Critical warning while Settings screen | WarningManager priority | Settings 위에 warning 확인 가능 | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| CAN RX → Vehicle Model update | ≤100 ms 목표 | NOT RUN | RX timestamp / model timestamp | TBD |
| Critical warning → UI indication | ≤200 ms 목표 | NOT RUN | injection/display timestamp | TBD |
| Touch → screen response | ≤150 ms 목표 | NOT RUN | touch/render timestamp | TBD |
| UI freeze during 5 min dummy update | 0회 | NOT RUN | soak test | TBD |

목표값은 실측 후 조정할 수 있으며, 변경 시 Specification도 함께 갱신한다.

---

# 8. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Vehicle_State` | RX | Gear/Mode update | NOT RUN | Planned | TBD |
| `Drive_Status` | RX | Speed/RPM update | NOT RUN | Planned | TBD |
| `Ultrasonic_Status` | RX | Distance/Warning update | NOT RUN | Planned | TBD |
| `Vision_Request` | RX | ADAS/Parking semantic update | NOT RUN | Planned | TBD |
| `Body_Status` | RX | Lamp/Ambient/LIN health | NOT RUN | Planned | TBD |
| `DTC_Event` | RX | DTC list update | NOT RUN | N/A/Event | TBD |
| `Body_Command` | TX | User lighting request 전송 | NOT RUN | N/A | TBD |

## LIN

N/A. H735는 LIN을 직접 사용하지 않는다.

---

# 9. DTC / Diagnostics Test

| Fault | Expected DTC / Status | Pi Manager Stored? | H735 Displayed? | Result |
|---|---|---|---|---|
| Ultrasonic sensor timeout | Ultrasonic DTC/status | NOT RUN | NOT RUN | TBD |
| Drive communication timeout | VCU/HMI communication status 후보 | NOT RUN | NOT RUN | TBD |
| Body LIN fault | Body/LIN DTC | NOT RUN | NOT RUN | TBD |
| Camera/Vision fault | HPC Vision fault | NOT RUN | NOT RUN | TBD |
| Unknown DTC | Raw code 표시 | N/A | NOT RUN | TBD |

---

# 10. Logs / Evidence

- UART / Console Log: TBD
- Screenshot: TBD
- Wiring Photo: TBD
- Test Video: TBD
- CAN Log: TBD
- TouchGFX screenshot/video: TBD

Stage 1 예시 로그 형식:

```text
[HMI][INIT] Display OK
[HMI][INIT] Touch OK
[HMI][DUMMY] speed=24 rpm=1250 gear=D
[HMI][SCREEN] CLUSTER -> PARKING
[HMI][WARN] PARKING_CRITICAL zone=RR
```

Stage 2 예시 로그 형식:

```text
[CAN][RX] Drive_Status rpm=1250 speed=24 valid=1
[CAN][TIMEOUT] Ultrasonic_Status
[HMI][INVALID] parking_sensor=1
```

---

# 11. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|
| TBD | TBD | TBD | TBD | TBD |

---

# 12. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Cluster Main 정상 표시
- [ ] Touch 화면 전환 정상
- [ ] Dummy/실제 차량 데이터 갱신 정상
- [ ] ADAS/Parking/DTC 화면 정상
- [ ] Invalid/Timeout UI 처리 확인
- [ ] Critical Warning 우선 표시 확인
- [ ] Body Command CAN TX 확인
- [ ] Timing 목표 측정
- [ ] 로그/스크린샷/영상 증거 저장

## Remaining Issues

- 실제 CAN ID / Signal layout 확정 필요
- 실제 FDCAN Transceiver / pin map 확정 필요
- DTC Clear Request protocol 확정 필요
- 실제 H735 성능 측정 필요
