# Stage 1 Test Report — [NODE / FUNCTION]

> 담당: A / B / C / D / E / F  
> Board:  
> Sensor / Actuator / Camera:  
> Firmware/Software Commit:  
> Date:

---

## 1. 시험 목적

이번 시험에서 **무엇을 입력하고 무엇이 나오면 PASS인지** 2~3문장으로 작성한다.

## 2. 시험 구성

```text
[Input]
  ↓
[Board / Software]
  ↓
[Output / Log]
```

## 3. Hardware / Environment

| 항목 | 내용 |
|---|---|
| Board | |
| MCU / Pi | |
| Sensor / Actuator / Camera | |
| Interface | GPIO / ADC / TIM / I2C / CAN / LIN / USB / CSI |
| Supply Voltage | |
| OS / Toolchain | 해당 시 |
| Debug | ST-Link / UART / SSH |

## 4. Wiring

| Device | Pin | Board Pin | Function | Voltage |
|---|---|---|---|---|

## 5. Configuration

| 항목 | 값 |
|---|---|
| Clock | |
| Sampling / Update | |
| PWM / Timer | |
| UART | |
| CAN bitrate | 해당 시 |
| LIN schedule | 해당 시 |
| Camera Resolution/FPS | 해당 시 |

## 6. Normal Test

| No | Condition | Expected | Measured / Observed | Result |
|---:|---|---|---|---|
| 1 | | | | PASS/FAIL |

## 7. Min / Middle / Max 또는 대표 3점

| Condition | Raw | Physical/Result | Error/Note | Result |
|---|---:|---:|---|---|

Camera/UI처럼 3점 측정이 의미 없으면 해상도/FPS/화면 전환 등 역할에 맞는 대표 조건으로 바꾼다.

## 8. Fault / Invalid Test

| Fault Injection | Expected State | Actual | Result |
|---|---|---|---|
| Disconnect / Service Stop | Invalid/Timeout | | |
| Out of Range | Clamp/Fault | | |
| Recovery | 정상 복구 | | |

## 9. Output / Actuator Test

액추에이터가 없으면 N/A.

| Command | Expected | Observed | Result |
|---|---|---|---|

움직이는 장치는 고정된 저속/저출력 테스트 환경에서 시험한다.

## 10. Log

```text
핵심 로그를 짧게 첨부
```

## 11. 문제 / 해결

| Problem | Cause | Fix | Prevention |
|---|---|---|---|

## 12. Stage 2 Interface Candidate

| Signal/Frame | Local Variable | Unit | Sender | Receiver | Timeout/Period |
|---|---|---|---|---|---|

## 13. Evidence

- [ ] Wiring photo
- [ ] UART / Debug log
- [ ] Screenshot
- [ ] Short video
- [ ] Scope / Logic Analyzer, 필요 시

## 14. Final Result

```text
STAGE 1: PASS / PARTIAL / FAIL
```

### Remaining Issues

- 
- 
