# [NODE NAME] Architecture

> 담당자: A / B / C / D / E / F  
> Node Type: STM32 ECU / Raspberry Pi HPC / H735 Cockpit / LIN Slave  
> Board / MCU:  
> Revision: v0.1  
> Date:  
> Related Specification: `SPECIFICATION.md`

---

## 1. Role

이 Node가 차량에서 담당하는 역할과 **담당하지 않는 역할**을 2~5문장으로 작성한다.

예:

> Parking ECU는 ToF/Ultrasonic 거리센서의 owner이며 거리 측정과 유효성 판단을 담당한다. Camera 영상처리는 담당하지 않으며 Rear Camera는 Raspberry Pi가 처리한다.

---

## 2. Requirement Traceability

명세서 Requirement와 Architecture 요소를 연결한다.

| Requirement ID | Architecture Element | 구현 위치 |
|---|---|---|
| REQ-XXX-001 | Sensor Read Task | `sensor.c` |
| | | |

---

## 3. Hardware Block Diagram

```text
[Sensor / Input]
      ↓
[This Node]
      ↓
[Actuator / Local Output]
      ↕
[CAN FD / LIN / CSI / USB]
```

현재 Node에 맞게 수정한다.

---

## 4. Inputs

| Input | Source | Interface | Unit / Range | Period | Owner | Invalid Condition |
|---|---|---|---|---|---|---|
| | | | | | | |

---

## 5. Outputs

| Output | Destination | Interface | Unit / Range | Period / Event | 비고 |
|---|---|---|---|---|---|
| | | | | | |

---

## 6. Pin Map

> 실제 board schematic / datasheet / CubeMX에서 확인한 핀만 적는다.

| Function | MCU Pin | Peripheral | Direction | Voltage | Note |
|---|---|---|---|---|---|
| | | | | | |

---

## 7. Wiring / External Circuit

| Device | Device Pin | Board Pin | Power | Additional Circuit | Note |
|---|---|---|---|---|---|
| | | | | | |

예: Motor는 MCU GPIO에 직접 연결하지 않고 Driver를 거친다. Battery Voltage는 ADC에 직접 넣지 않고 Divider/Measurement Circuit을 사용한다.

---

## 8. Software Component Diagram

예:

```text
Driver / HAL
    ↓
Sensor Service
    ↓
Validation / Filtering
    ↓
Application / State / Control
    ↓
Network Service / Local Output
```

실제 모듈명을 적는다.

---

## 9. Software Flow

```text
Init
 ↓
Read / Receive
 ↓
Validate
 ↓
Convert / Filter
 ↓
State / Control
 ↓
Publish / Actuate
```

---

## 10. Local Data

| Variable | Type | Meaning | Unit | Valid Range | Owner | Invalid 표현 |
|---|---|---|---|---|---|---|
| | | | | | | |

Raw와 Physical 값을 분리한다.

---

## 11. State / Mode

해당되는 Node만 작성한다.

| State | Entry | Action | Exit | Fault Behavior |
|---|---|---|---|---|
| INIT | | | | |
| READY | | | | |
| ACTIVE | | | | |
| FAULT | | | | |

---

## 12. CAN / CAN FD Interface

### TX

| Message / Signal | Meaning | Unit | Cycle | Receiver | Valid Condition |
|---|---|---|---|---|---|
| | | | | | |

### RX

| Message / Signal | Meaning | Unit | Timeout | Sender | Timeout Action |
|---|---|---|---|---|---|
| | | | | | |

Stage 1에서는 실제 CAN 구현 전이라도 데이터 owner와 의미는 작성한다.

---

## 13. LIN Interface — 해당 Node만

### 역할

- [ ] LIN Master
- [ ] LIN Slave
- [ ] N/A

### Frame / Schedule

| Frame | Publisher | Subscriber | Period / Slot | Payload |
|---|---|---|---|---|
| | | | | |

### CAN ↔ LIN Mapping — Gateway만

| CAN Signal | Direction | LIN Signal | Conversion / Rule |
|---|---|---|---|
| | CAN→LIN / LIN→CAN | | |

---

## 14. Fault / DTC

| Fault | Detection | Debounce / Timeout | Local Action | DTC | Recovery |
|---|---|---|---|---|---|
| | | | | | |

---

## 15. Timing

| Function | Target Period / Deadline | Measurement Method |
|---|---:|---|
| Sensor Update | | |
| Control Loop | | |
| CAN Tx | | |
| LIN Frame | | |
| UI Update | | |

N/A 항목은 삭제한다.

---

## 16. Stage 1 Test Plan

| No. | Test | Expected | Evidence |
|---:|---|---|---|
| 1 | Board Boot | 정상 | log |
| 2 | Peripheral Init | 정상 | log |
| 3 | Raw Input | 입력에 따라 변화 | log/table |
| 4 | Conversion | 정상 범위 | table |
| 5 | Local Output | 정상 또는 N/A | video/scope |
| 6 | Disconnect / Invalid | 오류 검출 | log |
| 7 | Recovery | 정상 복구 | log |

---

## 17. Stage 2 Integration Plan

처음 연결할 상대 Node를 적는다.

```text
My Node ↔ First Integration Node
```

예:

- VCU ↔ Drive CAN
- Gateway ↔ LIN Slave
- VCU ↔ H735 CAN

---

## 18. Open Issues

| Issue | 영향 | 담당 | 결정 필요일 |
|---|---|---|---|
| | | | |

---

## 19. 완료 체크

- [ ] Specification의 요구사항을 Architecture와 연결했다.
- [ ] Input / Output owner를 정의했다.
- [ ] Pin/Wiring이 실제 보드와 일치한다.
- [ ] Raw / Physical data를 구분했다.
- [ ] Fault와 Recovery를 적었다.
- [ ] CAN/LIN interface를 적었다.
- [ ] Stage 1 시험 계획이 있다.
- [ ] 다른 팀원이 3분 안에 구조를 이해할 수 있다.
