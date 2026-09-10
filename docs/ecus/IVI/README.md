# IVI / Cluster Cockpit Documentation

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md)

> **최상위 구현 기준:** [`FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)  
> CAN Signal / Warning 정책 / HMI 입력 범위 / RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **B 담당: STM32H735 + TouchGFX Cluster/IVI**의 하위 구현 문서다.

## 현재 진행 상태

2026-09-10 기준 STM32H735G-DK에서 MaJerle TouchGFX reference firmware의 기본 board bring-up을 완료했다.

```text
[x] Build 0 errors / 0 warnings
[x] ST-LINK flash / run
[x] LCD backlight 정상
[x] TouchGFX 화면 정상 표시
[x] Touch 입력 시 UI 정상 반응

NEXT
[ ] 우리 IVI project에서 board setting 재현
[ ] FDCAN2 PB5/PB6 추가
[ ] FDCAN2 internal loopback
```

Reference board bring-up의 상세 기록은 [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md), 실기 시험 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.

## 구현 시작점

처음 보드를 bring-up할 때는 아래 순서로 확인한다.

1. [STM32H735G-DK Hardware Bring-up](HARDWARE_BRINGUP.md)
2. [STM32H735G-DK IVI Pin Map](PIN_MAP.md)
3. [TouchGFX Reference Project 기준](REFERENCE_PROJECT.md)
4. [기능 명세](SPECIFICATION.md)
5. [소프트웨어 아키텍처](ARCHITECTURE.md)
6. [시험 기록](TEST_REPORT.md)

외부 TouchGFX 예제 프로젝트를 최종 코드로 복사하지 않고, H735G-DK의 LCD / Touch / OCTOSPI / HyperRAM / MPU / TouchGFX 설정을 검증하는 reference로 사용한다. Reference GUI가 실제 보드에서 정상 동작하는 것을 확인했으므로 다음 단계부터 우리 IVI project와 FDCAN2 bring-up을 진행한다.

## 고정 역할

```text
CAN FD
→ CanRxTask
→ VehicleModelTask
→ GuiTask / TouchGFX
→ Cluster + IVI

Touch
→ Body_User_Request / Diagnostic Request
```

H735는 상태를 표시하고 사용자 요청을 만든다. 최종 차량 제어와 Lamp GPIO는 직접 수행하지 않는다.

## 이미 고정된 규칙

- `Vision_Status`는 표시용, `ADAS_Request`는 VCU 제어판단용이다.
- H735는 `Body_Command`를 직접 publish하지 않는다.
- H735 → `Body_User_Request` → VCU → `Body_Command` 순서다.
- WarningManager는 표시 우선순위만 정하고 센서 threshold나 차량 safety를 다시 계산하지 않는다.
- DTC History canonical source는 Pi DTC Manager다.
- CAN decode와 TouchGFX rendering은 분리한다.

## Hardware / Peripheral 기준

```text
STM32H735G-DK
├ LTDC RGB888        → LCD
├ DMA2D              → TouchGFX accelerator
├ OCTOSPI1           → external Flash / GUI assets
├ OCTOSPI2 HyperBus  → HyperRAM / framebuffer
├ I2C4 + BSP         → Touch
├ FDCAN2 PB5/PB6     → SDV CAN backbone 후보
├ SWD                → Debug
└ FreeRTOS CMSIS-V2  → application tasks
```

H735에 Ultrasonic TRIG/ECHO, accelerator/brake ADC, motor PWM, servo PWM, LIN, lamp output을 직접 연결하지 않는다. 해당 기능은 각 ECU가 처리하고 IVI에는 CAN logical message로 전달한다.

## FreeRTOS 구조

```text
FDCAN ISR
→ CanRxTask
→ VehicleModelTask
→ Repository
→ GuiTask

GuiTask
→ CommandTxTask

HealthTask
```

## 구현해야 할 것

- [x] STM32H735G-DK Reference LCD / Touch / TouchGFX bring-up
- [ ] 우리 IVI project에서 LCD / Touch / HyperRAM / external Flash 설정 재현
- [ ] FDCAN2 loopback / physical CAN test
- [ ] Cluster / ADAS / Parking / Diagnostics / Settings 5개 화면
- [ ] DummyDataProvider
- [ ] VehicleDataRepository
- [ ] invalid / timeout / unknown DTC 표시
- [ ] Critical Warning overlay
- [ ] Body_User_Request queue
- [ ] CAN→UI / Touch latency 측정
- [ ] RTOS task/queue/stack/health 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-006
DEC-NET-004 ~ DEC-NET-007
DEC-HMI-001 ~ DEC-HMI-005
DEC-BODY-002
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Cluster 필수 항목, signal 목록, Warning 표시 정책, Gear R 화면정책, DTC Clear, Body_User_Request 기능범위, CAN ID/DLC/cycle/timeout, RTOS 수치는 이 폴더에서 독자적으로 최종 확정하지 않는다.

## Coding Gate

`Vehicle_State`, `Drive_Status`, `Vision_Status`, `Ultrasonic_Status`, `Body_Status`, `Body_User_Request`, `DTC_Event` 계약이 `FROZEN`되기 전에는 실제 CAN decode/encode bit layout을 확정하지 않는다.

## Stage 1 PASS 목표

```text
[x] H735G-DK Reference GUI Bring-up
[x] Reference LCD / Touch 정상
[ ] 우리 IVI project board bring-up 재현
[ ] 5개 화면
[ ] Dummy Data
[ ] invalid / critical warning
[ ] FreeRTOS task 분리
[ ] UI Request queue
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
