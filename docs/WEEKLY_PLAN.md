# 4주 개발 계획 — Stage 1 단독 Bring-up부터 차량 통합까지

[Main README](../README.md)

> **기간 / 인원:** 4주 / 6명  
> **기준 Architecture:** Raspberry Pi 4 HPC + STM32 #1~#5 + STM32H735 Cockpit + CAN FD Backbone + LIN Subnetwork  
> **핵심 원칙:** 1주차에 모든 ECU를 한꺼번에 연결하지 않는다. 먼저 각자 자기 Node를 단독 검증한다.

---

# 0. 역할

| 담당 | Node |
|---|---|
| A | STM32 #1 VCU / Driver Input / Safety / CAN Integration |
| B | STM32 #2 Drive + Steering |
| C | Raspberry Pi 4 HPC / Front ADAS / DTC Manager |
| D | STM32 #3 Parking + Rear Camera Service |
| E | STM32H735 Cluster + IVI Cockpit |
| F | STM32 #4 Body Gateway + STM32 #5 LIN Body Slave |

---

# Week 1 — Specification, Architecture, Stage 1 Bring-up

## 공통

모든 담당자는 코딩 전에 다음 3개 문서를 만든다.

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

필수 작업:

- [ ] Board / MCU 정확한 모델 확인
- [ ] Logic voltage / power input 확인
- [ ] CAN/FDCAN 지원 여부 확인
- [ ] Sensor / actuator datasheet 확인
- [ ] Pin Map 작성
- [ ] Wiring Table 작성
- [ ] Build / Flash / Debug 확인
- [ ] UART 또는 Debug 출력 확보

## A — VCU

- Gear P/R/N/D 버튼
- Accelerator ADC
- Brake ADC
- Steering AS5600 후보
- E-Stop
- `INIT / READY / DRIVE / REVERSE / FAULT` State 초안

완료 예:

```text
GEAR=D ACCEL=35 BRAKE=0 STEER=-11.2 ESTOP=0
```

## B — Drive + Steering

- TB6612FNG 후보 데이터시트/정격 확인
- Motor PWM / Direction
- 낮은 출력 Motor test
- Encoder/Hall pulse
- RPM 계산
- RC Servo center/left/right calibration

## C — HPC / ADAS

- Pi 4 boot / power 안정화
- Front CSI Camera capture
- Resolution / FPS 측정
- OpenCV frame read
- ROI / grayscale 등 최소 pipeline
- DTC Manager 저장구조 초안

## D — Parking

- ToF/Ultrasonic 1개 bring-up
- Near/Mid/Far 측정
- 최소 2개 센서 확장 방향 확인
- Rear USB Camera capture
- C와 Camera 실행 규칙 합의

## E — H735 Cockpit

- TouchGFX build
- LCD / Touch
- Cluster main 화면
- Dummy Speed/RPM/Battery/Gear
- ADAS/Parking/DTC/Settings 화면 전환

## F — Body Network

### STM32 #4 Gateway

- LIN peripheral/UART 확인
- LIN Transceiver 확인
- Master Header/Frame 기초
- LIN Schedule v0.1

### STM32 #5 Slave

- Ambient ADC
- Head/Tail/Brake/Turn/Hazard LED output
- Turn blink
- Slave response 구조

## Week 1 완료 조건

- [ ] 모든 Node 단독 Build/Flash/Run 가능
- [ ] 각 담당이 Spec + Architecture 초안 작성
- [ ] 핵심 Sensor/Actuator Raw 값 확인
- [ ] Disconnect / Invalid 시험 최소 1개
- [ ] Front/Rear Camera 각각 frame 확보
- [ ] H735 Cluster/IVI dummy UI 동작
- [ ] LIN Master/Slave 기본 연결 준비

> Week 1에서 CAN 전체 통합 성공은 완료조건이 아니다.

---

# Week 2 — Local Function 완성 + 2-Node Network Integration

## 공통

- CAN Signal Matrix v0.1 확정
- CAN bitrate / FD 설정 확정
- ECU Heartbeat 규칙
- Timeout 규칙
- DTC Event 형식
- LIN Schedule / Mapping Table v0.1 확정

## CAN 연결 순서

```text
1. VCU ↔ Drive
2. VCU ↔ Parking
3. VCU ↔ H735
4. Gateway ↔ CAN
5. Pi ↔ CAN
```

전부 동시에 연결하지 않는다.

## A

- `Driver_Input`
- `Vehicle_State`
- VCU Arbitration
- Brake > Accelerator 우선 규칙
- Gear R / D mode
- Heartbeat monitor skeleton

## B

- VCU Target 수신
- Motor command 적용
- RPM feedback 송신
- Servo steering command
- Speed PID는 가능하면 이 단계에서 시작
- Command timeout 시 Motor stop

## C

- Lane 또는 Object Detection 최소 1개
- ADAS Result 구조
- Pi CAN/SocketCAN 준비
- Dummy DTC Event 저장
- Active / History 구분

## D

- Parking Sensor 2~4채널
- SAFE/WARNING/CRITICAL
- `Parking_Status`
- R-mode Rear camera service

## E

- CAN RX → Vehicle Data Model
- Cluster에 실제 Speed/RPM/Gear 표시 시작
- ADAS/Parking dummy → 실제 signal 교체
- DTC dummy list 유지

## F

### LIN

```text
Gateway Master ↔ LIN Slave
```

- Ambient_Status
- Lamp_Command
- Lamp_Status
- Lamp_Diagnostic

### Gateway

```text
CAN Body_Command → LIN Lamp_Command
LIN Ambient_Status → CAN Body_Status
```

## Week 2 완료 조건

- [ ] VCU ↔ Drive CAN 통신
- [ ] VCU ↔ Parking CAN 통신
- [ ] H735가 실제 CAN signal 최소 2개 표시
- [ ] Gateway ↔ LIN Slave 양방향 통신
- [ ] CAN↔LIN Mapping 최소 1개 End-to-End
- [ ] 각 ECU Heartbeat 또는 상태 프레임 구현 시작

---

# Week 3 — Full Backbone & End-to-End Integration

## 3.1 Driver Path

```text
Accel / Brake / Steering / Gear
→ VCU
→ CAN
→ Drive + Steering
→ Motor / Servo
→ Status
→ H735
```

## 3.2 ADAS Path

```text
Front Camera
→ Pi ADAS
→ ADAS_Request
→ VCU Arbitration
→ Drive/Steering
```

## 3.3 Parking Path

```text
Gear R
→ VCU Vehicle_State
→ Pi Front ADAS Pause / Rear Camera Active
→ Parking Sensor Status
→ H735 Parking Screen
```

## 3.4 Body Gateway Path

```text
Ambient
→ LIN Slave
→ LIN
→ Gateway
→ CAN Body_Status
→ H735 / VCU / HPC
```

```text
H735/VCU Body_Command
→ CAN
→ Gateway
→ LIN
→ Slave
→ Lamp
```

## 3.5 DTC Path

```text
Sensor Disconnect
→ Local ECU Fault
→ DTC_Event
→ Pi DTC Manager
→ H735 Warning + Diagnostic Detail
```

## Week 3 중점

새 기능보다 다음을 우선한다.

- Timeout
- Invalid data
- Mode conflict
- Message ownership
- Recovery
- Network reconnect
- Camera service switching

## 완료 조건

- [ ] 전체 CAN Backbone 통신
- [ ] LIN subnetwork 정상
- [ ] CAN↔LIN Gateway 양방향
- [ ] Driver input → Motor/Steering
- [ ] ADAS request → VCU
- [ ] Gear R → Rear camera
- [ ] Parking → H735
- [ ] Local DTC → Pi → H735

---

# Week 4 — Vehicle Mounting, Validation, Fault Injection

## 차량 장착

- [ ] Wiring 고정
- [ ] Motor/logic power rail 확인
- [ ] Ground / voltage drop 확인
- [ ] Camera 위치 고정
- [ ] Parking sensor 위치 확정
- [ ] H735 Cockpit 장착
- [ ] Lighting / LIN node 장착

## 기능 시험

- [ ] P/R/N/D
- [ ] Accelerator / Brake
- [ ] Steering Wheel → Servo
- [ ] Motor RPM
- [ ] Front ADAS
- [ ] R-mode camera switching
- [ ] Parking warning
- [ ] Ambient → Auto light
- [ ] Turn/Brake/Hazard
- [ ] Cluster + IVI 화면

## Fault Injection

- [ ] Parking sensor disconnect
- [ ] Encoder signal loss
- [ ] LIN Slave power off
- [ ] CAN ECU unplug
- [ ] Camera service error
- [ ] Heartbeat timeout
- [ ] DTC clear / recovery

## 측정 항목

| 항목 | 측정 |
|---|---|
| Front ADAS FPS | avg / min |
| Camera → ADAS Request | ms |
| CAN Request → Actuator | ms |
| Gear R → Rear first frame | ms |
| Parking update period | ms |
| LIN Schedule / Response | ms |
| CAN↔LIN Gateway latency | ms |
| Heartbeat timeout | ms |
| Fault → DTC → H735 warning | ms |

---

# 최종 Demo Scenario

```text
1. Power ON
2. ECU Heartbeat / H735 READY
3. Gear D
4. Accelerator + Steering input
5. Motor / Servo response
6. Front Camera ADAS
7. ADAS steering/stop request → VCU
8. Gear R
9. Front ADAS pause / Rear Camera active
10. Parking sensor warning → H735
11. Ambient light change
12. LIN Slave → Gateway → CAN → H735
13. H735/VCU Lamp command → CAN → Gateway → LIN → Lamp
14. Sensor/ECU fault injection
15. DTC → Pi History → H735 Diagnostic screen
16. Critical fault → VCU safe state
```

---

# 진행 기록 양식

```markdown
### Week N
- 담당:
- 이번 주 요구사항:
- 구현:
- Commit:
- Test condition:
- Expected:
- Actual:
- Measured:
- PASS/FAIL:
- 문제/원인:
- 다음 작업:
```

완료 여부는 "코드가 있음"이 아니라 **시험과 증거가 있음**을 기준으로 판단한다.
