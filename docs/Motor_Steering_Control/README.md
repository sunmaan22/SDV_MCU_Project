# Motor + Steering Control Documentation

이 폴더는 **C 담당: Motor + Steering / 제어**의 작성 예시다.

현재 공통 Template을 실제 프로젝트 역할에 맞춰 채운 상태이며, 실제 Motor/Servo/Encoder/STM32 보드가 확정되고 시험이 진행되면 `TBD`, `후보`, `NOT RUN` 항목을 실제 값으로 교체한다.

## 역할 한 줄 요약

```text
VCU가 최종으로 정한 속도 / 조향 명령
        ↓ CAN FD
Drive + Steering STM32
        ↓ FreeRTOS
Motor / Steering Control
        ↓
DC Motor + RC Servo
```

이 Node는 **차량을 실제로 움직이는 제어 ECU**다.

```text
Driver Input / ADAS / Parking
        ↓
       VCU
  최종 안전 판단
        ↓
Final Speed / Steering Request
        ↓
Drive + Steering ECU
        ↓
PWM / Direction / Servo Command
```

중요한 역할 경계:

- 이 ECU가 Accelerator나 Steering Wheel 센서를 직접 소유하지 않는다. Driver Input Owner는 VCU다.
- ADAS Vision 결과를 직접 Motor PWM으로 바꾸지 않는다. 최종 명령은 VCU를 거친다.
- Motor RPM/Vehicle Speed는 이 ECU가 Encoder/Hall을 이용해 생성하는 데이터다.
- Steering actuator는 RC Servo를 기본안으로 하며 실제 Servo의 사양과 기구 한계를 기준으로 보정한다.
- Motor Driver는 현재 **TB6612FNG 후보**다. 최종 Motor의 전압, 정격전류, 특히 Stall Current와 Driver 정격이 맞는지 확인한 뒤 확정한다.

---

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - 제어 기능이 무엇을 해야 하는지
   - VCU Command / Encoder Input
   - Motor / Servo Output
   - Command Timeout / Safe State
   - Open-loop → Closed-loop 확장
   - FreeRTOS 실행 요구사항

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - Driver / Feedback / Control / CAN 구조
   - FreeRTOS Task와 ISR
   - Queue / Notification / Repository
   - Motor Control과 Steering Control
   - Runtime / Fault Flow
   - Watchdog / Stack / Timing

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Motor PWM / Direction
   - Encoder RPM
   - Servo Left / Center / Right
   - Command Timeout
   - CAN RX/TX
   - ControlTask period / jitter
   - Stack / Queue / Watchdog
   - 장시간 Soak Test

---

## 기본 RTOS 구조

```text
CAN RX ISR
   ↓
CanRxTask
   ↓ CommandQueue
ControlTask
├ Motor Command
└ Steering Command
   ↓
PWM / DIR / Servo PWM

Encoder ISR
   ↓ notification
FeedbackTask
   ↓ FeedbackQueue
ControlTask / StatusTask

StatusTask
   ↓
Drive_Status CAN TX

HealthTask
├ command timeout
├ task alive
├ queue overflow
├ stack health
└ watchdog condition
```

정확한 주기와 FreeRTOS numeric priority는 실제 하드웨어에서 Timing Test 후 확정한다.

---

## 개발 단계

### Stage 1

```text
Motor Driver / Servo / Encoder 단독 시험
→ FreeRTOS Task 구조 적용
→ UART Log로 값 확인
```

### Stage 2

```text
VCU ↔ Drive/Steering ECU
CAN 2-Node 통신
→ Final Command 수신
→ Output
→ Drive Status 송신
```

### Stage 3

```text
VCU + Drive + H735 + HPC 통합
→ Driver / ADAS Request
→ VCU Arbitration
→ Drive / Steering
→ Status / DTC
```

---

## 현재 주요 TBD

- 실제 STM32 보드 / FDCAN 지원 여부
- Motor 모델과 전압/정격/ Stall Current
- TB6612FNG 최종 사용 가능 여부
- Encoder/Hall 종류와 CPR/PPR
- Motor PWM Frequency
- Speed 단위와 Final Command Scale
- Servo 모델 / Center / Left / Right Calibration
- Steering 기구 한계
- ControlTask / FeedbackTask 실제 주기
- CAN ID / DLC / Signal Layout
- Speed PID 사용 시 Kp/Ki/Kd
- Command Timeout 값
- Watchdog timeout

추측으로 숫자를 확정하지 않고 실제 Hardware/Datasheet/Test 결과로 채운다.
