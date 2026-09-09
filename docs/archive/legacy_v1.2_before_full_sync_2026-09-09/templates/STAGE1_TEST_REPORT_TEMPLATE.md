# Stage 1 Bring-up Test Report — [NODE / FUNCTION]

> 담당자: A / B / C / D / E / F  
> Node:  
> Board / MCU:  
> Sensor / Actuator:  
> Firmware / SW Commit:  
> Test Date:  
> Related Requirement ID:  

---

## 1. 시험 목적

이번 시험에서 무엇을 확인하는지 2~3문장으로 작성한다.

예:

> Accelerator Potentiometer가 VCU ADC에서 정상적으로 읽히고 최소/중간/최대 위치가 0~100%로 변환되는지 확인한다. 센서 선 분리 또는 ADC 범위 이탈 시 Invalid 상태를 검출하는지도 확인한다.

---

## 2. 시험 구성

```text
[Input / Sensor]
      ↓
[Node / MCU]
      ↓
[UART / Local Output / Display]
```

Stage 1에서는 CAN/LIN 통합이 아직 없어도 된다. 단, LIN Master↔Slave 자체 bring-up처럼 담당 기능이 네트워크 그 자체인 경우에는 해당 Local Network 시험을 포함한다.

---

## 3. Hardware Information

| 항목 | 내용 |
|---|---|
| Node | |
| Board / MCU | |
| Device | |
| Interface | GPIO / ADC / I2C / SPI / TIM / PWM / CSI / USB / LIN |
| Supply Voltage | |
| Logic Voltage | |
| Debug | ST-Link / UART / SSH / 기타 |

---

## 4. Wiring

| Device | Device Pin | MCU / Board Pin | Function | Voltage | Additional Circuit |
|---|---|---|---|---|---|
| | | | | | |

- 배선 사진 첨부
- Motor/Battery는 전원·Driver·Divider 등을 명시
- LIN/CAN은 Transceiver를 명시

---

## 5. Firmware / Software Configuration

| 항목 | 설정 |
|---|---|
| Clock | |
| Peripheral | |
| Sampling / Update Period | |
| PWM Frequency / Pulse | 해당 시 |
| UART Baudrate | 해당 시 |
| Sensor Address | 해당 시 |
| Camera Resolution / FPS | 해당 시 |
| LIN Schedule / Frame | 해당 시 |

---

## 6. Raw Data Test

| Condition | Raw 1 | Raw 2 | Raw 3 | Expected Trend | Result |
|---|---:|---:|---:|---|---|
| Minimum / Near | | | | | PASS / FAIL |
| Middle | | | | | PASS / FAIL |
| Maximum / Far | | | | | PASS / FAIL |

Raw 값이 없는 UI/HPC logic 시험은 N/A로 표시한다.

---

## 7. Physical / Logical Value Test

| Input / Reference | Converted / Logical Value | Unit / Enum | Error / Note | Result |
|---|---|---|---|---|
| | | | | PASS / FAIL |
| | | | | PASS / FAIL |
| | | | | PASS / FAIL |

예:

```text
ADC 620  → ACCEL 0%
ADC 2120 → ACCEL 50%
ADC 3620 → ACCEL 100%
```

---

## 8. Output / Actuator / UI Test

| Command / Data | Expected | Observed | Result |
|---|---|---|---|
| | | | PASS / FAIL |
| | | | PASS / FAIL |

예:

- Motor PWM
- RC Servo Center/Left/Right
- Lamp ON/OFF
- H735 Gauge update
- Camera frame

---

## 9. Fault / Invalid Test

| Fault Injection | Expected State | Actual | Recovery | Result |
|---|---|---|---|---|
| Sensor Disconnect | Invalid / Timeout | | | PASS / FAIL |
| Out-of-range | Invalid / Clamp | | | PASS / FAIL |
| Power Cycle | 정상 재초기화 | | | PASS / FAIL |
| 기타 | | | | PASS / FAIL |

Gateway/LIN 예:

- LIN Slave power off
- LIN response timeout
- 잘못된 frame / no response

HPC 예:

- Camera unavailable
- Process restart

---

## 10. Timing / Performance

해당되는 항목만 작성한다.

| Metric | Target | Measured | Result |
|---|---:|---:|---|
| Sensor update | | | |
| RPM update | | | |
| Camera FPS | | | |
| Camera first frame | | | |
| LIN response | | | |
| UI update | | | |

---

## 11. 핵심 Log

```text
[INIT] ...
[DATA] ...
[FAULT] ...
[RECOVERY] ...
```

로그 전체를 붙이지 말고 문제를 재현하는 핵심 부분을 남긴다.

---

## 12. 문제 / 원인 / 해결

| 문제 | 원인 | 해결 | 재발 방지 |
|---|---|---|---|
| | | | |

`다시 하니 됨`은 원인 분석이 아니다. 전압/배선/설정/코드 중 어느 계층의 문제였는지 가능한 범위에서 기록한다.

---

## 13. Stage 2 Network 준비

### CAN / CAN FD Candidate

| Signal | Local Variable | Unit | Valid Range | Sender | Receiver |
|---|---|---|---|---|---|
| | | | | | |

### LIN Candidate — 해당 시

| Frame / Signal | Publisher | Subscriber | Period | 비고 |
|---|---|---|---|---|
| | | | | |

### Gateway Mapping — 해당 시

| Source Signal | Destination Signal | Rule |
|---|---|---|
| CAN `Body_Command` | LIN `Lamp_Command` | bit mapping |
| | | |

---

## 14. Evidence

- [ ] Wiring Photo
- [ ] UART / Debug Log
- [ ] Screenshot
- [ ] Short Test Video
- [ ] Scope / Logic Analyzer Capture 필요 시

파일명 예:

```text
A_VCU_STAGE1_accel_test.csv
B_DRIVE_STAGE1_encoder_log.txt
C_HPC_STAGE1_front_camera.png
D_PARK_STAGE1_distance.csv
E_COCKPIT_STAGE1_cluster.mp4
F_GATEWAY_STAGE1_lin_capture.png
```

---

## 15. 최종 판정

- [ ] Requirement를 시험했다.
- [ ] Board Bring-up 완료
- [ ] Raw / Logical input 정상
- [ ] Physical conversion 정상 또는 N/A
- [ ] Local output 정상 또는 N/A
- [ ] Invalid/Disconnect 처리
- [ ] Recovery 확인
- [ ] Architecture와 실제 Pin/Wiring 일치
- [ ] Stage 2 Signal 후보 작성
- [ ] Evidence 저장

```text
STAGE 1 : PASS / FAIL / PARTIAL
```

남은 문제:

- 
- 
