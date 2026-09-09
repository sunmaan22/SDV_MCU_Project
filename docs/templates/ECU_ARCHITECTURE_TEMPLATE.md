# [ECU NAME] Architecture

> 담당자: A / B / C / D / E / F  
> Board:  
> Revision: v0.1  
> Date:  

---

## 1. 이 ECU의 역할

이 ECU가 차량에서 담당하는 기능을 2~3문장으로 작성한다.

예시:

> Parking ECU는 차량 주변 거리센서를 직접 읽고 유효성을 검사한다. 측정된 거리로 Parking Warning Level을 생성하며, Stage 2부터 CAN을 통해 VCU/IVI/HPC에 제공한다.

---

## 2. Hardware Block Diagram

```text
[Sensor / Input]
      ↓
[This ECU / Board]
      ↓
[Actuator / Local Output]

Stage 2 이후:
      ↕
   CAN / CAN FD
```

---

## 3. Inputs

| Input | Source | Electrical / Bus Interface | Unit / Range | Update Period | Note |
|---|---|---|---|---|---|
| | | | | | |
| | | | | | |

---

## 4. Outputs

| Output | Destination | Interface | Unit / Range | Update Method | Note |
|---|---|---|---|---|---|
| | | | | | |
| | | | | | |

---

## 5. Pin Map

> 실제 보드 datasheet / schematic / CubeMX에서 확인한 핀만 적는다.

| Function | MCU Pin | Peripheral | Direction | Voltage | Note |
|---|---|---|---|---|---|
| | | | | | |
| | | | | | |

---

## 6. Wiring

| Device | Device Pin | MCU / Board Pin | Power | Note |
|---|---|---|---|---|
| | | | | |
| | | | | |

---

## 7. Software Flow

```text
Initialization
      ↓
Input Read
      ↓
Validation
      ↓
Conversion / Filtering
      ↓
Control / State Decision
      ↓
Local Output / Data Update
```

내 ECU에 맞게 위 흐름을 수정한다.

---

## 8. 주요 Local Data

| Variable | Type | Meaning | Unit | Valid Range | Invalid Condition |
|---|---|---|---|---|---|
| | | | | | |
| | | | | | |

Raw 값과 변환된 Physical 값을 구분한다.

예:

```text
adc_raw       = ADC 원시값
motor_temp_c  = 변환된 °C
```

---

## 9. State / Mode

해당되는 ECU만 작성한다.

```text
INIT
 ↓
READY
 ↓
ACTIVE
 ↓
FAULT
```

| State | Entry Condition | Action | Exit Condition |
|---|---|---|---|
| | | | |

---

## 10. Fault Cases

| Fault | Detection Method | Local Action | Recovery |
|---|---|---|---|
| Sensor Timeout | | | |
| Sensor Invalid | | | |
| Communication Timeout | Stage 2 | | |
| | | | |

---

## 11. Future CAN Interface — TX

> Stage 1에서는 실제 송신을 구현하지 않아도 되며, **어떤 데이터의 owner인지 먼저 정의**한다.

| Signal | Meaning | Unit | Proposed Cycle | Receiver | Valid Condition |
|---|---|---|---|---|---|
| | | | | | |

---

## 12. Future CAN Interface — RX

| Signal | Meaning | Unit | Proposed Timeout | Sender | Action |
|---|---|---|---|---|---|
| | | | | | |

---

## 13. Stage 1 Test

| No. | Test | Expected | Measured / Observed | Result |
|---:|---|---|---|---|
| 1 | Board Boot | 정상 부팅 | | PASS / FAIL |
| 2 | Peripheral Init | Init 성공 | | PASS / FAIL |
| 3 | Sensor Raw Read | 입력에 따라 값 변화 | | PASS / FAIL |
| 4 | Physical Conversion | 정상 범위 값 | | PASS / FAIL |
| 5 | Min / Max Test | 범위 확인 | | PASS / FAIL |
| 6 | Disconnect / Invalid | 오류 검출 | | PASS / FAIL |
| 7 | Recovery | 재연결 후 복구 | | PASS / FAIL |

---

## 14. Evidence

- UART Log:
- Debug Screenshot:
- Wiring Photo:
- Test Video:
- Logic Analyzer / Scope Capture:

---

## 15. Stage 1 완료 체크

- [ ] Architecture를 다른 팀원에게 3분 안에 설명할 수 있다.
- [ ] 실제 Pin Map을 작성했다.
- [ ] 센서/입력 Raw 값을 확인했다.
- [ ] Physical 값 변환을 확인했다.
- [ ] Invalid/Disconnect 조건을 시험했다.
- [ ] 출력 장치가 있다면 안전한 범위에서 단독 동작을 확인했다.
- [ ] Stage 2에서 필요한 CAN TX/RX Signal을 적었다.
- [ ] 테스트 증거를 남겼다.
