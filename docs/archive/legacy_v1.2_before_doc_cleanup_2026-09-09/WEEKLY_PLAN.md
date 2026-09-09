# 4주 개발 계획 — 6역할 기준

> 원칙: **Stage 1 단독 검증 → 작은 통신 통합 → 전체 Backbone → 차량 통합** 순서로 간다.

---

# 역할

| 담당 | 역할 |
|---|---|
| A | Ultrasonic / 인지 |
| B | H735 Cluster + IVI / UI |
| C | Motor + Steering / 제어 |
| D | Lighting + Ambient / LIN-CAN |
| E | HPC + Front/Rear Camera Vision / 인지·판단 |
| F | VCU + DTC + CAN Integration / 최종 판단 |

---

# Week 1 — Specification + Architecture + Stage 1

모든 팀원:

- `SPECIFICATION.md`
- `ARCHITECTURE.md`
- `STAGE1_TEST_REPORT.md`

을 작성한다.

| 담당 | Week 1 목표 |
|---|---|
| A | Ultrasonic 1개 거리 측정, timeout, 3-point test |
| B | H735 TouchGFX Cluster/IVI Dummy Data UI |
| C | Motor PWM/DIR, Encoder/Hall RPM, RC Servo |
| D | Ambient/Lighting Slave 단독 + LIN Master↔Slave 통신 |
| E | Pi #1 Front Camera, Pi #2 Rear Camera capture/FPS, 기본 Vision pipeline |
| F | Gear/Accel/Brake/Steering/E-Stop + VCU State/Arbitration dummy test |

### Week 1 PASS

- [ ] 모든 보드 Build/Flash/Run
- [ ] 모든 담당자 자기 Input/Output 설명 가능
- [ ] 실제 Wiring/Pin 기록
- [ ] 정상 입력 시험
- [ ] Invalid/Disconnect/Timeout 시험
- [ ] CAN/LIN 후보 Signal 정의

---

# Week 2 — Local 기능 + 작은 통신 통합

처음부터 모든 Node를 한 버스에 넣지 않는다.

## CAN pair

```text
F VCU ↔ C Drive/Steering
F VCU ↔ A Ultrasonic
F VCU ↔ B H735
```

순차적으로 붙인다.

## LIN

```text
D Gateway LIN Master ↔ D LIN Slave
```

그 후:

```text
CAN Body_Command
→ Gateway
→ LIN Lamp_Command
```

과 반대 방향을 시험한다.

## 역할별

| 담당 | Week 2 |
|---|---|
| A | 여러 Ultrasonic 확장, filtering, CAN status |
| B | CAN Vehicle Data Model 연결 |
| C | CAN command + timeout, RPM feedback, Speed control baseline |
| D | LIN schedule + CAN↔LIN mapping |
| E | Lane/Object baseline, Rear Parking Vision baseline, 공통 Result interface |
| F | CAN Matrix v0.1, Heartbeat, VCU 실제 CAN arbitration, DTC code table v0.1 |

---

# Week 3 — 전체 CAN Backbone + End-to-End

## Drive path

```text
Driver Input
→ F VCU
→ C Drive/Steer
→ Motor/Servo
→ C Status
→ B H735
```

## Front ADAS

```text
Front Camera
→ E Vision
→ ADAS Request
→ F VCU
→ C Drive/Steer
```

## Parking

```text
A Ultrasonic ───────────┐
                        ├→ F VCU → Final Stop/Speed
E Rear Parking Vision ──┘

A/E Status → B H735 Parking Screen
```

## Body

```text
B/F Body Request
→ CAN
→ D Gateway
→ LIN
→ D Slave
→ Lighting
```

## DTC

```text
Local Fault
→ DTC Event
→ Pi DTC Manager
→ B H735 Diagnostics
→ F VCU safe action if critical
```

E는 Pi에서 DTC Manager service가 실행될 수 있도록 Linux/service 환경을 지원하고, F가 Diagnostic 규격을 관리한다.

---

# Week 4 — 최종 Pi 통합 + 차량 검증

### Vision

개발용 Pi 두 대의 코드를 최종 Pi 한 대로 통합한다.

```text
D gear → Front Vision active
R gear → Rear Parking Vision active
```

### 차량 시험

- [ ] Gear P/R/N/D
- [ ] Accelerator / Brake
- [ ] Steering input → Servo
- [ ] Encoder RPM
- [ ] Ultrasonic warning/stop
- [ ] Front Vision request
- [ ] Rear Vision + Ultrasonic parking
- [ ] H735 Cluster/IVI
- [ ] Ambient → LIN → CAN
- [ ] CAN → LIN → Lighting
- [ ] Heartbeat timeout
- [ ] Sensor disconnect
- [ ] Camera service failure
- [ ] DTC Active/History/Clear

### 측정할 것

- Front Vision FPS
- Rear Vision FPS
- Camera sensing → request latency
- CAN request → Motor/Servo response
- Ultrasonic update period
- LIN schedule period
- DTC detection → H735 indication
- Gear R → Rear first frame latency

---

# 최종 Demo 예

```text
Power ON
→ Heartbeat
→ H735 Cluster READY
→ Gear D
→ Front Vision
→ ADAS Request
→ VCU Arbitration
→ Motor/Steering
→ Gear R
→ Rear Vision + Ultrasonic
→ Parking Warning / Stop
→ Ambient 변화
→ LIN Slave → Gateway → CAN → H735
→ Lamp Request
→ CAN → Gateway → LIN → Lamp
→ Sensor/Camera Fault
→ DTC → Pi → H735
```

기능 하나가 멋있게 보이는 것보다 이 흐름 전체가 반복 가능하게 동작하는 것이 최종 목표다.
