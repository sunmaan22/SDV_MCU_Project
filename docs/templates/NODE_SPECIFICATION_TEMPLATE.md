# [NODE NAME] Specification

> 담당자: A / B / C / D / E / F  
> Board / MCU:  
> Revision: v0.1  
> Date:  
> Related Architecture: `ARCHITECTURE.md`

---

## 1. Purpose

이 Node가 왜 필요한지 2~4문장으로 작성한다.

예:

> Body Gateway는 CAN FD Backbone과 LIN Body Subnetwork 사이의 Protocol/Signal Gateway 역할을 한다. CAN Body Command를 LIN Lamp Command로 변환하고 LIN Ambient/Lamp 상태를 CAN Body Status로 변환한다.

---

## 2. Scope

### 이 Node가 하는 일

- 
- 

### 이 Node가 하지 않는 일

- 
- 

역할 경계를 적어야 데이터 owner 중복을 막을 수 있다.

---

## 3. Functional Requirements

Requirement ID는 변경하지 않고 관리한다.

| ID | Requirement | Priority | Verification |
|---|---|---|---|
| REQ-XXX-001 | Node는 ... 해야 한다. | MUST | Test |
| REQ-XXX-002 | Node는 ... 해야 한다. | MUST | Test |
| REQ-XXX-003 | Node는 ... 해야 한다. | SHOULD | Test / Inspection |

좋은 Requirement 예:

```text
REQ-PARK-001
Parking ECU는 각 활성 거리센서 값을 설정된 update period 내에 갱신해야 한다.
```

나쁜 예:

```text
센서를 잘 읽는다.
```

---

## 4. Input Requirements

| ID | Input | Source | Unit / Range | Required Rate | Invalid Condition |
|---|---|---|---|---|---|
| IN-XXX-001 | | | | | |

---

## 5. Output Requirements

| ID | Output | Destination | Unit / Range | Rate / Event | Safe Value |
|---|---|---|---|---|---|
| OUT-XXX-001 | | | | | |

---

## 6. Interface Requirements

### Hardware

| Interface | Requirement |
|---|---|
| Power | |
| GPIO | |
| ADC | |
| I2C/SPI/UART | |
| PWM/Timer | |

### CAN / CAN FD

| Message / Signal | Tx/Rx | Requirement |
|---|---|---|
| | | |

### LIN — 해당 시

| Frame | Master/Slave Role | Requirement |
|---|---|---|
| | | |

### Camera / USB / CSI — 해당 시

| Interface | Requirement |
|---|---|
| | |

---

## 7. State / Mode Requirements

| State | 의미 | Entry | Required Behavior |
|---|---|---|---|
| INIT | | | |
| READY | | | |
| ACTIVE | | | |
| FAULT | | | |

해당 없는 Node는 삭제한다.

---

## 8. Safety / Fail-safe Requirements

| ID | Condition | Required Safe Behavior |
|---|---|---|
| SAFE-XXX-001 | Sensor Invalid | |
| SAFE-XXX-002 | Communication Timeout | |
| SAFE-XXX-003 | E-Stop / Critical Fault | |

---

## 9. Diagnostic Requirements

| DTC | Fault Condition | Detection | Clear / Recovery |
|---|---|---|---|
| XXX_001 | | | |

각 Local ECU는 자기 Sensor/Actuator Fault를 먼저 검출한다.

---

## 10. Timing / Performance Requirements

| Metric | Target | Priority |
|---|---:|---|
| Sensor update | | MUST / SHOULD |
| Control period | | |
| CAN period | | |
| LIN response | | |
| Camera FPS | | |
| UI update | | |

N/A는 삭제한다.

---

## 11. Stage 1 Acceptance Criteria

- [ ] Build / Flash / Run
- [ ] Peripheral Init
- [ ] Raw input 확인 또는 N/A
- [ ] Physical/Logical value 확인
- [ ] Local output 확인 또는 N/A
- [ ] Invalid / Disconnect 검출
- [ ] Recovery 확인
- [ ] Stage 2 Network Signal 후보 작성

담당 Node 특화 조건:

- [ ] 
- [ ] 

---

## 12. Stage 2 Integration Criteria

첫 통합 상대:

```text
[My Node] ↔ [Target Node]
```

확인할 항목:

- [ ] Network communication
- [ ] Signal scale / unit
- [ ] Timeout
- [ ] Invalid data
- [ ] Recovery

---

## 13. Open Decisions

| Item | Options | Owner | Due |
|---|---|---|---|
| | | | |

---

## 14. Revision History

| Rev | Date | Change | Author |
|---|---|---|---|
| v0.1 | | Initial | |
