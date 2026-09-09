# Motor + Steering Control Documentation

이 폴더는 **C 담당: Motor + Steering / 제어**의 기준 문서다.

## 역할

```text
VCU Final_Drive_Command
        ↓ CAN FD
Drive + Steering ECU
        ↓
Motor / Steering Control
        ↓
DC Motor + RC Servo
```

이 ECU는 **VCU가 승인한 최종 명령을 실제 Actuator 출력으로 바꾸는 Node**다. Driver Input, ADAS, Parking 우선순위를 직접 판단하지 않는다.

## 통합 검토 후 확정된 규칙

1. VCU→Drive 제어 인터페이스는 논리적으로 `Final_Drive_Command` 하나를 기준으로 한다.
   - 내부 signal 후보: `speed_request`, `steering_request`, `drive_enable`, `gear/direction`, `valid/freshness`.
   - 실제 CAN ID/DLC/bit layout은 CAN Matrix에서 확정한다.
2. `Final_Drive_Command` timeout 검출 책임은 **Drive ECU**에 있다.
   - VCU는 `Drive_Status`/Heartbeat timeout을 검출한다.
3. Command timeout 시 오래된 Motor PWM을 계속 유지하면 안 된다. Motor는 safe state로 전환한다.
4. Steering timeout 시 `hold / center / neutral` 중 어떤 정책을 사용할지는 기구 시험 전에 반드시 결정한다. 임의로 center 복귀를 넣지 않는다.
5. TB6612FNG는 확정 부품이 아니다. Motor voltage, rated current, 특히 stall current를 확인하기 전에는 최종 선정하지 않는다.
6. 방향 전환 D↔R 또는 Motor direction reversal은 즉시 반전하지 않고 VCU/Drive가 합의한 stop 조건을 거친다.
7. Encoder ISR에서는 count/timestamp만 처리하고 RPM 계산과 PID는 Task에서 수행한다.

## FreeRTOS 기본 구조

```text
FDCAN ISR
  ↓
CanRxTask
  ↓ latest command / queue
ControlTask
  ├ Motor PWM / DIR
  └ Servo target

Encoder ISR
  ↓ notification
FeedbackTask
  ↓ RPM / feedback
ControlTask / StatusTask

StatusTask → Drive_Status
HealthTask → timeout / queue / stack / watchdog
```

## 지금 개발해야 할 것

- Motor를 무부하 또는 안전한 bench 상태에서 낮은 출력으로 PWM/DIR 제어한다.
- RC Servo를 Left/Center/Right로 움직이고 실제 기구 한계를 기록한다.
- Encoder/Hall pulse를 Timer/Input Capture로 읽고 RPM을 계산한다.
- FreeRTOS `CanRxTask`, `ControlTask`, `FeedbackTask`, `StatusTask`, `HealthTask` skeleton을 만든다.
- Dummy `Final_Drive_Command`를 넣어서 ControlTask가 Motor/Servo output을 만드는 경로를 완성한다.
- command timestamp/freshness를 저장하고 timeout injection 시험을 만든다.
- Stage 1은 open-loop request→PWM부터 검증하고, encoder가 안정된 뒤 closed-loop PID를 붙인다.
- ControlTask period/jitter를 측정할 수 있는 timestamp 구조를 넣는다.

## 반드시 결정해야 할 것

- 실제 STM32 보드와 FDCAN 지원 여부
- Motor 모델, 사용 전압, rated current, stall current
- Motor Driver 최종 부품과 정격 적합성
- Encoder/Hall 종류, PPR/CPR, RPM 계산식
- Motor PWM frequency와 허용 duty 범위
- Speed command 단위/scale
- Servo 모델, center pulse, left/right safe limit
- Steering command 단위와 mapping
- command timeout 값
- command timeout 시 Steering 정책
- D↔R direction reversal 조건
- Encoder invalid 시 open-loop 유지/정지/degraded 정책
- PID 적용 여부와 적용 시 Kp/Ki/Kd tuning 절차
- `Drive_Status` signal 목록과 cycle
- Task period/priority/stack/queue depth

## Stage 1 PASS

```text
Motor low-output PWM/DIR
+ Servo Left/Center/Right
+ Encoder RPM
+ FreeRTOS task 구조
+ Dummy Final_Drive_Command
+ timeout 시 Motor safe state
```

실제 수치가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
