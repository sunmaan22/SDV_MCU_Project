# Stage 1 Bring-up Test Report — [ECU / Function Name]

> 담당자: A / B / C / D / E / F  
> Board:  
> Sensor / Actuator:  
> Firmware Commit:  
> Test Date:  

---

## 1. 시험 목적

이번 시험에서 확인하려는 내용을 2~3문장으로 작성한다.

예시:

> Parking ECU에 연결한 거리센서가 정상적으로 초기화되고, 실제 거리 변화에 따라 측정값이 변하는지 확인한다. 센서 분리 시 MCU가 멈추지 않고 Invalid 상태를 만들 수 있는지도 확인한다.

---

## 2. 시험 구성

```text
[Input / Sensor]
      ↓
[MCU / Board]
      ↓
[UART / Debug / Output]
```

---

## 3. Hardware Information

| 항목 | 내용 |
|---|---|
| Board | |
| MCU | |
| Sensor / Actuator | |
| Interface | ADC / GPIO / I2C / SPI / TIM / USB / CSI |
| Supply Voltage | |
| Debug Interface | ST-Link / UART / SSH / 기타 |

---

## 4. Wiring

| Device | Device Pin | MCU / Board Pin | Function | Voltage |
|---|---|---|---|---|
| | | | | |
| | | | | |

배선 사진 또는 회로 그림이 있으면 아래에 첨부한다.

---

## 5. Firmware Configuration

| 항목 | 설정 |
|---|---|
| Clock | |
| Peripheral | |
| Sampling / Update Period | |
| Timer / PWM Frequency | 해당 시 |
| Baudrate | UART 사용 시 |
| Sensor Address | I2C/SPI 해당 시 |

---

## 6. Raw Data Test

먼저 가공 전 값이 입력 변화에 따라 정상적으로 변하는지 확인한다.

| Condition | Raw Value 1 | Raw Value 2 | Raw Value 3 | Result |
|---|---:|---:|---:|---|
| Minimum / Near | | | | PASS / FAIL |
| Middle | | | | PASS / FAIL |
| Maximum / Far | | | | PASS / FAIL |

---

## 7. Physical Value Test

| Reference / Condition | Converted Value | Unit | Error / Note | Result |
|---|---:|---|---|---|
| | | | | PASS / FAIL |
| | | | | PASS / FAIL |
| | | | | PASS / FAIL |

---

## 8. Actuator Test

액추에이터가 없는 노드는 `N/A`로 표시한다.

| Command | Expected | Observed | Result |
|---|---|---|---|
| | | | PASS / FAIL |
| | | | PASS / FAIL |
| | | | PASS / FAIL |

모터/조향 등 움직이는 장치는 저속·저출력의 고정된 테스트 환경에서만 단독 시험하고, 차량 통합 주행은 Stage 1 범위에 포함하지 않는다.

---

## 9. Fault / Invalid Test

| Fault Injection | Expected Software State | Actual | Result |
|---|---|---|---|
| Sensor Disconnect | Invalid / Timeout | | PASS / FAIL |
| Out-of-range Input | Invalid / Clamp | | PASS / FAIL |
| Reconnect | 정상 복구 | | PASS / FAIL |
| 기타 | | | PASS / FAIL |

---

## 10. Serial / Debug Log

```text
여기에 핵심 로그를 짧게 첨부
```

예:

```text
[INIT] SENSOR OK
RAW=502 PHYSICAL=501mm VALID=1
RAW=498 PHYSICAL=497mm VALID=1
[WARN] SENSOR TIMEOUT
VALID=0
[RECOVERY] SENSOR OK
```

---

## 11. 문제와 해결 내용

| 문제 | 원인 | 해결 | 재발 방지 |
|---|---|---|---|
| | | | |

`안 됐는데 다시 하니 됨`으로 끝내지 말고 원인을 가능한 범위에서 기록한다.

---

## 12. Stage 2 CAN 준비 데이터

Stage 1에서 정상적으로 확보한 데이터 중 CAN으로 보낼 후보를 적는다.

| Signal | Current Local Variable | Unit | Valid Range | Sender | Receiver |
|---|---|---|---|---|---|
| | | | | | |

---

## 13. Evidence

- [ ] Wiring Photo
- [ ] UART / Debug Log
- [ ] Screenshot
- [ ] Short Test Video
- [ ] 필요한 경우 Scope / Logic Analyzer Capture

파일명 규칙 예시:

```text
A_VCU_STAGE1_gear_test.jpg
B_DRIVE_STAGE1_encoder_log.txt
C_ADAS_STAGE1_front_camera.png
D_PARK_STAGE1_distance_test.csv
E_IVI_STAGE1_touchgfx.mp4
F_BODY_STAGE1_light_test.jpg
```

---

## 14. 최종 판정

### PASS 조건

- [ ] Board Bring-up 완료
- [ ] 입력 Raw 값 정상
- [ ] 필요한 Physical 값 변환 정상
- [ ] 출력/액추에이터 단독 시험 정상 또는 N/A
- [ ] Invalid/Disconnect 처리 확인
- [ ] 재연결 또는 Reset 후 복구 확인
- [ ] Architecture 문서와 실제 Pin Map이 일치
- [ ] 다음 단계에 필요한 CAN Signal 후보 작성

### Result

```text
STAGE 1 : PASS / FAIL / PARTIAL
```

### 남은 문제

- 
- 
- 
