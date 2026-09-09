# 4주 개발 계획 — 인지·판단·제어 분리 개발부터 차량 통합까지

[Main README](../README.md) · [팀 역할 쉬운 설명](TEAM_ROLE_EASY_GUIDE.md)

> **기간 / 인원:** 4주 / 6명  
> **기준 Architecture:** Raspberry Pi Vision/HPC + STM32 #1~#5 + STM32H735 Cockpit + CAN FD Backbone + LIN Subnetwork  
> **핵심 원칙:** Stage 1에서는 각자 자기 기능을 단독으로 확인하고, 그 다음에 CAN/LIN으로 연결한다.

---

# 0. 현재 6인 역할

| 담당 | 역할 | 주요 Hardware |
|---|---|---|
| A | Ultrasonic / 인지 | STM32 #1 + Ultrasonic |
| B | Cluster + IVI / UI | STM32H735 + TouchGFX |
| C | Motor + Steering / 제어 | STM32 #2 + TB6612FNG 후보 + Motor + Servo |
| D | Lighting + Ambient / LIN-CAN | STM32 #3 Gateway + STM32 #4 LIN Slave |
| E | HPC + Camera Vision / 인지·판단 | Raspberry Pi + Front/Rear Camera |
| F | VCU + DTC + CAN Integration / 최종 판단·진단 | STM32 #5 + Pi Diagnostics 협업 |

---

# Week 1 — 내 기능 단독 Bring-up

## 공통

모든 팀원은 코딩 전에 다음을 작성한다.

```text
SPECIFICATION.md
ARCHITECTURE.md
```

그리고 자기 보드/센서/액추에이터를 **다른 팀원의 CAN 데이터 없이** 단독 동작시킨다.

## A — Ultrasonic

- [ ] 센서 1개 Wiring
- [ ] Trigger / Echo 확인
- [ ] 실제 거리 3점 측정
- [ ] `distance_mm`, `valid` 생성
- [ ] Sensor timeout 확인
- [ ] 2개 이상 센서 확장 방법 정리

완료 예:

```text
REAR_LEFT_MM = 301
VALID = 1
```

## B — H735 Cluster + IVI

- [ ] TouchGFX Build
- [ ] LCD / Touch 동작
- [ ] Cluster Main 화면
- [ ] Dummy Speed/RPM/Gear 표시
- [ ] ADAS/Parking/DTC 화면 전환
- [ ] Warning icon on/off

Stage 1에서는 CAN을 기다리지 않고 Dummy Data를 사용한다.

## C — Drive + Steering

- [ ] Motor Driver 전원/핀 확인
- [ ] Motor 없이 PWM 파형 확인
- [ ] 저출력 Motor 회전
- [ ] Encoder/Hall pulse 확인
- [ ] RPM 계산
- [ ] RC Servo Center / Left / Right 확인

PID는 기본 센서/구동이 확인된 뒤 시작한다.

## D — Body / LIN-CAN

### STM32 #4 Slave

- [ ] Ambient Sensor 읽기
- [ ] Head/Tail/Turn/Brake LED 단독 제어

### STM32 #3 Gateway / Master

- [ ] LIN Transceiver 연결
- [ ] LIN Master 기본 frame 송수신
- [ ] Master ↔ Slave 통신 성공

Week 1에는 CAN↔LIN Gateway까지 욕심내지 않고 LIN 자체를 먼저 성공시킨다.

## E — HPC / Vision

개발 중 Pi 두 대 사용 가능:

```text
Pi #1 → Front Vision
Pi #2 → Rear Vision
```

- [ ] Front Camera Frame Capture
- [ ] Rear Camera Frame Capture
- [ ] 해상도 / FPS 측정
- [ ] OpenCV 기본 처리
- [ ] Front/Rear 실행 코드 분리
- [ ] Camera disconnect/error 처리

## F — VCU / Diagnostics

- [ ] Gear P/R/N/D GPIO
- [ ] Accelerator ADC
- [ ] Brake ADC
- [ ] Steering Input 후보 센서 읽기
- [ ] E-Stop
- [ ] VCU 기본 State Machine
- [ ] DTC 코드 naming 초안
- [ ] CAN Signal owner 초안

## Week 1 완료 조건

- [ ] 6명 전원이 자기 기능을 3분 안에 설명 가능
- [ ] 자기 Node Specification / Architecture 작성
- [ ] Stage 1 단독 동작 증거 확보
- [ ] Pin/Wiring 실제값 기록
- [ ] Fault/Disconnect 최소 1개 시험

---

# Week 2 — 기능 완성 + 2 Node 통신

모든 보드를 한꺼번에 CAN에 연결하지 않는다.

```text
2 Node 성공
→ 다음 Node 추가
```

## A

- Ultrasonic 다채널
- Filtering
- SAFE / WARNING / CRITICAL
- `Ultrasonic_Status` CAN 송신

## B

- H735 CAN RX skeleton
- 실제 CAN 값을 Vehicle Data Model에 연결
- Speed/RPM/Gear/Warning 1개 이상 실데이터 표시

## C

- Target Speed → Motor PWM
- RPM feedback
- Servo target mapping
- CAN command/status 구조

## D

- LIN Schedule Table
- Ambient LIN status
- Lamp LIN command/status
- Body Gateway CAN RX/TX skeleton
- CAN signal ↔ LIN signal Mapping Table

## E

- Front Lane 또는 Object Detection 최소 1개
- Rear Object Detection 또는 Parking Vision 최소 1개
- Vision Result 구조체
- Pi별 결과 format 통일

## F

- CAN Matrix v0.1
- VCU `P/R/N/D`, `MANUAL/ADAS/PARK/FAULT`
- Heartbeat 규칙
- DTC Event 기본 규칙
- VCU ↔ C 또는 VCU ↔ A 2-node CAN 통신

## Week 2 완료 조건

- [ ] CAN 2-node 이상 통신 성공
- [ ] LIN Master ↔ Slave 성공
- [ ] H735 실 CAN 데이터 표시 1개 이상
- [ ] Vision Result 생성
- [ ] Motor/Servo command interface 동작

---

# Week 3 — 전체 데이터 흐름 통합

## ADAS Path

```text
Front Camera
→ E Vision
→ ADAS Request
→ F VCU
→ Final Request
→ C Drive / Steering
```

## Parking Path

```text
Gear R
→ F VCU
→ Rear Vision Active

A Ultrasonic ───────┐
E Rear Vision ──────┤
                    ↓
               F / HPC 판단
                    ↓
              Parking Status
                    ↓
                 B H735
```

## Body Path

```text
Ambient
→ D LIN Slave
→ LIN Master/Gateway
→ CAN FD
→ F / B / E
```

```text
Lighting Request
→ CAN FD
→ D Gateway
→ LIN
→ D Slave
→ Lamp
```

## DTC Path

```text
각 담당 Local Fault
→ DTC Event
→ F 규칙 / Pi DTC Manager
→ B H735 Diagnostic UI
```

Week 3에는 새 기능보다 **통신 불일치, Timeout, Mode 충돌, 복구**를 먼저 고친다.

---

# Week 4 — 차량 장착 / 검증 / 최종 Demo

- [ ] 모든 보드 차량 장착
- [ ] 전원 rail / GND / 배선 고정
- [ ] Gear D → Front Vision
- [ ] Gear R → Rear Vision
- [ ] Ultrasonic distance warning
- [ ] Motor / Steering 저속 주행
- [ ] H735 Cluster + IVI
- [ ] LIN Ambient / Lighting
- [ ] CAN↔LIN Gateway
- [ ] Heartbeat timeout
- [ ] Sensor disconnect
- [ ] Camera service fault
- [ ] DTC 저장/표시
- [ ] Critical fault 시 Safe State

## 최종 측정 항목

| 항목 | 담당 중심 |
|---|---|
| Ultrasonic update period / error | A |
| H735 UI update | B |
| Motor command → response / RPM | C |
| LIN cycle / CAN↔LIN latency | D |
| Vision FPS / inference latency | E |
| CAN latency / Heartbeat timeout / DTC latency | F |

---

# 최종 Demo 예

```text
1. Power ON
2. Heartbeat 확인
3. H735 READY
4. Gear D
5. Front Camera Vision
6. ADAS Request
7. VCU 최종 판단
8. Motor / Steering 동작
9. Ultrasonic 장애물 감지
10. Warning / Stop
11. Gear R
12. Rear Vision + Ultrasonic Parking Assist
13. Ambient 변화 → LIN → CAN → H735
14. Lighting Command → CAN → LIN → Lamp
15. 센서/통신 Fault Injection
16. DTC → Pi 저장 → H735 표시
```

---

# 주간 기록 양식

```markdown
### N주차 진행 기록
- 담당:
- 이번 주 내 Input:
- 이번 주 내 Output:
- 완료 기능:
- 테스트 조건:
- 기대 결과:
- 실제 결과:
- PASS / FAIL:
- 로그 / 사진 / 영상:
- 문제 / 원인 / 해결:
- 다음 통합 대상:
```
