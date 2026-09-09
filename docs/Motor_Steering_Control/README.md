# Motor + Steering Control Documentation

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../FINAL_IMPLEMENTATION_SPEC.md)  
> Motor/Driver/Servo/Command/Timeout/RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **C 담당: Motor + Steering / 제어**의 하위 구현 문서다.

## 고정 역할

```text
VCU Final_Drive_Command
→ Drive + Steering ECU
→ Motor / Steering Control
→ DC Motor + RC Servo
```

이 ECU는 VCU가 승인한 최종 명령을 실제 actuator output으로 바꾼다. Driver/ADAS/Parking arbitration은 하지 않는다.

## 이미 고정된 규칙

- VCU→Drive 논리 인터페이스는 `Final_Drive_Command`를 사용한다.
- `Final_Drive_Command` Publisher는 VCU다.
- command timeout 검출 책임은 Drive ECU에 있다.
- timeout 시 오래된 Motor PWM을 유지하지 않는다.
- TB6612FNG는 Motor 전압/정격/Stall Current 검증 전까지 후보일 뿐이다.
- D↔R 즉시 반전 금지.
- Encoder ISR은 count/timestamp만 처리한다.
- 실제 final command를 직접 만드는 것은 C가 아니라 F다.

## FreeRTOS 구조

```text
FDCAN ISR
→ CanRxTask
→ ControlTask
→ Motor PWM / DIR / Servo

Encoder ISR
→ FeedbackTask
→ RPM / Feedback

StatusTask
HealthTask
```

## 구현해야 할 것

- 안전한 bench 상태에서 low-output PWM/DIR
- Servo Left/Center/Right 및 기구 한계 측정
- Encoder/Hall 입력과 RPM 계산
- Dummy Final_Drive_Command → actuator output
- command freshness/timeout injection
- Stage 1 open-loop
- Feedback 안정화 후 closed-loop 확장
- ControlTask period/jitter/stack/queue 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-002
DEC-HW-010 ~ DEC-HW-013
DEC-NET-004 ~ DEC-NET-007
DEC-CTRL-004 ~ DEC-CTRL-005
DEC-CTRL-013 ~ DEC-CTRL-018
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Motor/Driver/Encoder/Servo, PWM frequency, request unit/range, Servo safe limit, command timeout, steering timeout action, D↔R 조건, encoder fallback, PID, Drive_Status contract, RTOS 수치는 독자적으로 최종 확정하지 않는다.

## Coding Gate

Motor Driver rating과 `Final_Drive_Command`/`Drive_Status` 계약, timeout/safe state가 `FROZEN`되기 전에는 통합 actuator 제어 상수를 확정하지 않는다.

## Stage 1 PASS

```text
Motor low-output PWM/DIR
+ Servo Left/Center/Right
+ Encoder RPM
+ FreeRTOS
+ Dummy Final_Drive_Command
+ timeout → safe state
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
