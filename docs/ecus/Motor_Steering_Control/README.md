# Motor + Steering Control Documentation

> **2026-09-17 C 입력 계획 변경:** 기어·조향·속도 요청은 RF로 STM32(C)에 수신한다. E-Stop은 로컬 GPIO/EXTI 차단을 유지한다. RF 모델은 nRF24L01, STM32 연결은 SPI로 확정했다. 모듈 보드/핀/패킷/수치와 CAN 매핑은 OPEN이다. 아래 2026-09-15 기록의 가변저항·로컬 Gear GPIO 설명은 변경 이력이며 현재 입력 구성에 적용하지 않는다.

> **2026-09-15 범위 변경 (1차):** Encoder/Hall 실측 Feedback을 삭제했다 (`DEC-HW-012`, `DEC-CTRL-017` REMOVED). C는 RF/가변저항 Driver 입력을 읽어 `Driver_Input`을 CAN으로 발행하는 owner가 되었고(publisher가 F에서 C로 이전), Motor_RPM/Vehicle_Speed는 명령값 기반 추정 함수 결과로 대체한다 (`DEC-CTRL-021`, 실측 아님).
>
> **2026-09-15 범위 변경 (2차):** E-Stop/Gear 물리 입력도 F에서 C로 이전했다. E-Stop은 CAN 없이 로컬에서 즉시 Motor Driver를 차단하고, `Driver_Input`에 gear/estop_status를 포함해 CAN 발행한다. Pi DTC Manager(History DB)는 삭제됐다 — `DTC_Event`는 B(IVI)가 실시간으로만 표시한다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)
> Motor/Driver/Servo/Command/Timeout/RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **C 담당: Motor + Steering / 제어 + Driver 입력**의 하위 구현 문서다.

## 지금 시작할 작업

**[Driver_Input RF 수신 시작 가이드](DRIVER_INPUT_START.md)** 순서로 진행한다. 먼저 기존 프로젝트 Build/Download → nRF24L01 SPI 레지스터 확인 → 시험 패킷 수신 → 기어·조향·속도 요청 해석 → 신호 두절 시험을 한다. CAN/RTOS/모터 출력은 이후 통합한다. 이 초기 bench 프로젝트는 C ECU 입력부이며 별도 ECU 추가가 아니다.

속도는 우선 목표 속도/스로틀 요청이라는 작업 가정이다. 실측 속도나 모터 출력 기반 추정 속도와 구분한다. 기존 `accel/brake` CAN 필드에 대한 매핑은 최상위 명세 §4.8.1의 OPEN 결정이다.

## 고정 역할

```text
RF 수신기(기어·조향·속도 요청) → Driver_Input (C 발행)
VCU Final_Drive_Command
→ Drive + Steering ECU
→ Motor / Steering Control
→ DC Motor + RC Servo
```

이 ECU는 Driver/Gear 입력을 읽어 발행하고, VCU가 승인한 최종 명령을 실제 actuator output으로 바꾼다. E-Stop만 예외로 로컬에서 즉시 처리한다. ADAS/Collision Warning arbitration은 하지 않는다.

## 이미 고정된 규칙

- VCU→Drive 논리 인터페이스는 `Final_Drive_Command`를 사용한다.
- `Final_Drive_Command` Publisher는 VCU다.
- `Driver_Input` Publisher는 C다 (기존 F에서 이전; accel/brake/steering/gear/estop_status 포함).
- E-Stop은 C가 로컬 GPIO/EXTI로 직접 읽고 CAN과 무관하게 즉시 Motor Driver를 disable한다. Gear는 C가 읽어 CAN으로 보고만 하고 최종 방향 결정은 F가 한다.
- command timeout 검출 책임은 Drive ECU에 있다.
- timeout 시 오래된 Motor PWM을 유지하지 않는다.
- TB6612FNG는 Motor 전압/정격/Stall Current 검증 전까지 후보일 뿐이다.
- D↔R 즉시 반전 금지.
- Encoder/Hall은 사용하지 않는다 — Speed/RPM은 명령값(PWM) 기반 추정 함수 결과이며 항상 estimated로 표시한다.
- 실제 final command를 직접 만드는 것은 C가 아니라 F다.
- Pi DTC Manager(History DB)는 삭제됐다 — `DTC_Event`는 B(IVI)가 실시간으로만 표시한다.

## FreeRTOS 구조

```text
E-Stop EXTI → 로컬 즉시 Motor Enable/STBY 차단 (CAN 비의존)

FDCAN ISR
→ CanRxTask
→ ControlTask
→ Motor PWM / DIR / Servo → Speed/RPM 추정

RF 기어·조향·속도 요청 수신
→ DriverInputTask
→ Driver_Input (gear/estop_status 포함)

CanTxTask
HealthTask
```

## 구현해야 할 것

- 안전한 bench 상태에서 low-output PWM/DIR
- Servo Left/Center/Right 및 기구 한계 측정
- RF(기어·조향·속도 요청) 입력 읽기 및 `Driver_Input` 발행
- E-Stop 로컬 즉시 차단 (CAN 비의존)
- 명령값 기반 Speed/RPM 추정 함수
- Dummy Final_Drive_Command → actuator output
- command freshness/timeout injection
- ControlTask/DriverInputTask period/jitter/stack/queue 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-002
DEC-HW-010 ~ DEC-HW-011, DEC-HW-013, DEC-HW-020, DEC-HW-024, DEC-HW-026 ~ DEC-HW-028
DEC-NET-004 ~ DEC-NET-007
DEC-CTRL-004 ~ DEC-CTRL-006
DEC-CTRL-013 ~ DEC-CTRL-016, DEC-CTRL-018 ~ DEC-CTRL-021
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Motor/Driver/Servo/Driver Input 장치, PWM frequency, request unit/range, Servo safe limit, command timeout, steering timeout action, D↔R 조건, 입력 선형 매핑, speed/rpm 추정 함수, Drive_Status contract, RTOS 수치는 독자적으로 최종 확정하지 않는다.

## Coding Gate

Motor Driver rating과 `Final_Drive_Command`/`Driver_Input`/`Drive_Status` 계약, timeout/safe state가 `FROZEN`되기 전에는 통합 actuator 제어 상수를 확정하지 않는다.

## Stage 1 PASS

```text
Motor low-output PWM/DIR
+ Servo Left/Center/Right
+ Driver_Input 발행
+ 명령값 기반 speed/rpm 추정
+ FreeRTOS
+ Dummy Final_Drive_Command
+ timeout → safe state
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
