# [NODE / FEATURE NAME] Test Report

> 목적: `SPECIFICATION.md`의 Requirement를 실제 시험 결과로 확인하고 증거를 남긴다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | |
| Owner | A / B / C / D / E / F |
| Board / Platform | |
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
| Sensor / Actuator | |
| Power | |
| Interface | |
| CAN/LIN Bitrate, 해당 시 | |
| Camera Resolution/FPS, 해당 시 | |
| Tool / Debug Interface | |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|

사진/그림 링크:

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-001 | REQ-XXX-001 | | | | |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|

Raw 값이 있는 기능은 Raw → Physical/Logical 변환을 같이 기록한다.

---

# 5. Boundary / Calibration Test

| Condition | Raw | Converted | Reference | Error / Note | Result |
|---|---:|---:|---:|---|---|

예:
- Ultrasonic: Near / Mid / Far
- Accelerator: Min / Mid / Max
- Steering: Left / Center / Right
- Motor: PWM step별 RPM

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-001 | Sensor disconnect | invalid / timeout | | | |
| F-002 | Out-of-range | reject / clamp | | | |
| F-003 | CAN/LIN timeout | timeout flag | | | |

해당 없는 항목은 `N/A`.

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| Update period | | | | |
| Response latency | | | | |
| Timeout detection | | | | |
| FPS / inference latency | | | | |

해당되는 항목만 사용한다.

---

# 8. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|

## LIN

| Frame / Signal | Publisher | Expected Period/Value | Actual | Fault Test | Result |
|---|---|---|---|---|---|

---

# 9. DTC / Diagnostics Test

| Fault | Expected DTC / Status | Pi Manager Stored? | H735 Displayed? | Result |
|---|---|---|---|---|

Local DTC가 아직 없는 Stage 1 기능은 Local Fault flag까지만 확인할 수 있다.

---

# 10. Logs / Evidence

- UART / Console Log:
- Screenshot:
- Wiring Photo:
- Test Video:
- CAN/LIN Log:
- Scope / Logic Analyzer, 해당 시:

핵심 로그는 짧게 첨부한다.

```text
[INIT] ...
[DATA] ...
[FAULT] ...
[RECOVERY] ...
```

---

# 11. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|

원인을 알 수 없는 경우 `Unknown`으로 숨기지 말고, 확인한 범위와 다음 조사 항목을 적는다.

---

# 12. Final Result

```text
RESULT: PASS / FAIL / PARTIAL
```

## PASS 조건

- [ ] 주요 Requirement가 Test ID에 연결되어 있다.
- [ ] 정상 기능 결과가 재현된다.
- [ ] 경계값/Calibration이 필요한 기능은 측정값이 있다.
- [ ] Fault/Timeout이 필요한 기능은 실제 시험했다.
- [ ] Timing 목표가 있는 기능은 측정했다.
- [ ] 증거가 남아 있다.
- [ ] 남은 문제/TBD가 기록되어 있다.

## Remaining Issues

- 
- 
