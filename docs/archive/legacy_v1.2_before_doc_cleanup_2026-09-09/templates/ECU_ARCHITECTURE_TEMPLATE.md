# [NODE NAME] Architecture

> 담당: A / B / C / D / E / F  
> Board:  
> Revision: v0.1

---

## 1. Role

### 한 문장

> 이 Node는 __________________________________ 한다.

### 데이터 흐름 5문장

```text
Input  :
Process:
Output :
Target :
Fault  :
```

---

## 2. Hardware Block Diagram

```text
[Sensor / Input / Network]
          ↓
      [This Node]
          ↓
[Actuator / Output / Network]
```

---

## 3. Hardware I/O

| Function | Device | Interface | MCU Pin | Voltage | Direction |
|---|---|---|---|---|---|

실제 보드 schematic/CubeMX로 확인한 핀만 적는다.

---

## 4. Software Component

```text
Driver Layer
    ↓
Validation
    ↓
Conversion / Filtering
    ↓
Application / State / Control
    ↓
Network / Output
```

내 Node에 맞게 수정한다.

---

## 5. Main Data

| Variable | Type | Meaning | Unit | Valid Range | Invalid Condition |
|---|---|---|---|---|---|

Raw와 Physical 값을 분리한다.

---

## 6. State Machine

```text
INIT → READY → ACTIVE → FAULT
```

필요한 Node만 수정해서 사용한다.

| State | Entry | Action | Exit |
|---|---|---|---|

---

## 7. CAN Interface

### TX

| Message/Signal | Meaning | Unit | Cycle/Event | Receiver |
|---|---|---|---|---|

### RX

| Message/Signal | Meaning | Sender | Timeout | Timeout Action |
|---|---|---|---|---|

---

## 8. LIN Interface, 해당 시

| Frame | Direction | Publisher | Subscriber | Period |
|---|---|---|---|---|

### CAN ↔ LIN Mapping, Gateway 해당 시

| CAN Signal | Direction | LIN Signal | Conversion |
|---|---|---|---|

---

## 9. Fault / DTC

| Fault | Detection | Local Action | DTC | Recovery |
|---|---|---|---|---|

---

## 10. Timing

| Task / Function | Period | Priority | Worst/Measured Time |
|---|---:|---|---:|

---

## 11. Stage 1 Test Plan

| Test | Input/Condition | Expected | Evidence |
|---|---|---|---|

---

## 12. Requirement Traceability

| Requirement ID | Component | Test ID |
|---|---|---|

---

## 13. 완료 체크

- [ ] 역할 경계 명확
- [ ] Input/Output owner 명확
- [ ] 실제 Wiring 일치
- [ ] Software flow 설명 가능
- [ ] Fault 정의
- [ ] CAN/LIN 후보 정의
- [ ] Stage 1 증거 존재
