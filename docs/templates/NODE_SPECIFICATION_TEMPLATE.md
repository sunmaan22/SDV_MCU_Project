# [NODE NAME] Specification

> 담당: A / B / C / D / E / F  
> 역할 분류: 인지 / 판단 / 제어 / UI / 통신 / 진단  
> Board:  
> Revision: v0.1  
> Date:

---

## 1. 한 문장 설명

> 이 Node는 __________________________________________ 한다.

## 2. 내가 받는 것

| Input | Source | Interface | Unit / Range | Update |
|---|---|---|---|---|
| | | | | |

## 3. 내가 만드는 것

| Output | Destination | Interface | Unit / Range | Update |
|---|---|---|---|---|
| | | | | |

## 4. 하지 않는 일

- 
- 

역할 경계를 명확히 적는다.

## 5. Functional Requirements

| ID | Requirement | Priority | Verification |
|---|---|---|---|
| REQ-XXX-001 | | MUST/SHOULD | Test/Inspect |

Requirement는 `~해야 한다` 형태로 쓴다.

## 6. Hardware Requirements

| Item | Requirement | Status |
|---|---|---|
| Supply Voltage | | |
| Logic Level | | |
| Peripheral | GPIO/ADC/TIM/I2C/CAN/LIN/USB/CSI | |
| Transceiver/Driver | | |

## 7. Network Requirements

### CAN TX

| Signal | Meaning | Unit | Cycle/Event | Receiver |
|---|---|---|---|---|

### CAN RX

| Signal | Meaning | Sender | Timeout | Action on Timeout |
|---|---|---|---|---|

### LIN, 해당 시

| Frame | Master/Publisher | Data | Period | Fault Condition |
|---|---|---|---|---|

## 8. State / Mode

| State | Entry | Action | Exit |
|---|---|---|---|

## 9. Fault / DTC Requirements

| Fault | Detection | Local Action | DTC Candidate | Recovery |
|---|---|---|---|---|

## 10. Timing

| Function | Target Period / Deadline | Measurement Method |
|---|---|---|

## 11. Stage 1 Acceptance Criteria

- [ ] Board bring-up
- [ ] Input 정상
- [ ] Output 정상
- [ ] Raw/Physical 값 확인
- [ ] Invalid/Timeout 확인
- [ ] Recovery 확인
- [ ] 결과 증거 저장

## 12. Stage 2 Integration Criteria

- [ ] CAN/LIN 후보 Signal 정의
- [ ] Sender/Receiver 합의
- [ ] Unit/Range 합의
- [ ] Timeout/Fail-safe 정의
