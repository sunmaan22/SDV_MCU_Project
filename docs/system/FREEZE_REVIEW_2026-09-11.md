# Freeze Review — 2026-09-11

[최상위 명세](FINAL_IMPLEMENTATION_SPEC.md) · [문서 정책](DOCUMENTATION_POLICY.md)

## 판정

**부분 동결. Implementation Baseline v1.0은 아직 아니다.** Owner가 H735 이외 STM 보드를 G431KB로 구매 완료했다고 알리고, 문서·test 근거가 있으면 동결하라고 지시한 범위에서 적용했다. 새로운 성능값이나 안전 임계값을 만들어 확정하지 않았다.

검토 기준 소스: `39e500fc5e58c0f2bd6290b7dfe2b2e18d4db3bb` (`main`).
검토 범위: `docs/system/`, `docs/getting_started/WEEKLY_PLAN.md`, `docs/ecus/`의 6개 역할 specification/architecture/test report, IVI pin map 및 현재 `.ioc`. `docs/archive/`는 결정 근거에서 제외했다.

## 이번 동결 기록

| Decision | 확정값 / 범위 | 근거 | 남은 검증 |
|---|---|---|---|
| DEC-HW-001~005 | A/C/D Gateway/D Slave/F: STM32G431KB 기반 구매 보드 | 2026-09-11 Owner 구매 확정 및 동결 지시 | 제조사/revision/전체 품번, 실물 핀맵, 각 역할 bring-up와 자원 적합성 |
| DEC-HW-021 | B: STM32H735G-DK | Owner의 H735 유지 + [IVI 시험 §0.1~0.4](../ecus/IVI/TEST_REPORT.md) | 전체 IVI 기능/부하 검증 |
| DEC-HW-022 | B: FDCAN2, PB5 RX/PB6 TX 설계 예약 | [PIN_MAP §9](../ecus/IVI/PIN_MAP.md), 현재 [SDV_IVI_H735.ioc](../../firmware/IVI/SDV_IVI_H735/SDV_IVI_H735.ioc), [IVI 시험 §0.4~0.5](../ecus/IVI/TEST_REPORT.md) | 실제 외부 핀/트랜시버/2nd node 통신; internal loopback은 이 경로를 검증하지 않음 |
| DEC-HW-023 | B: LTDC display/BSP I2C4 touch, OCTOSPI1 NOR asset 0x90000000, OCTOSPI2 HyperRAM framebuffer 0x70000000 | [IVI 시험 §0.1~0.6](../ecus/IVI/TEST_REPORT.md)와 현재 `.ioc` | 전체 MPU/cache/clock/GUI budget/RTOS 통합 검증; D4/D5 런타임 디버거 시험은 NOT RUN 유지 |

결정일은 모두 2026-09-11이며 승인 근거는 위 Owner 지시다. 기존 §1/§2의 역할·publisher·safety 우선순위·실행 모델은 이미 고정된 계약을 유지한다.

## 테스트가 증명하는 범위

| 역할 / 근거 | 기존 기록 | 동결 판단 |
|---|---|---|
| [A Ultrasonic](../ecus/Ultrasonic_Perception/TEST_REPORT.md) | RESULT: NOT RUN | 센서 수/위치/filter/threshold/scan 수치 OPEN |
| [C Drive](../ecus/Motor_Steering_Control/TEST_REPORT.md) | RESULT: NOT RUN | motor/driver/encoder/servo, PID, calibration, timeout/safe action OPEN |
| [D Body](../ecus/Lighting_Ambient_LIN_CAN/TEST_REPORT.md) | RESULT: NOT RUN | LIN bitrate/checksum/schedule/mapping 및 ambient/lamp 수치 OPEN |
| [E Vision](../ecus/HPC_Camera_Vision/TEST_REPORT.md) | RESULT: NOT RUN | camera/model/FPS/freshness/switch latency OPEN |
| [F VCU](../ecus/VCU_DTC_CAN_Integration/TEST_REPORT.md) | RESULT: NOT RUN | state machine/enable/recovery/DTC/heartbeat 수치 OPEN |
| [B IVI](../ecus/IVI/TEST_REPORT.md) §0.5 | loopback 100/100 PASS, commit `835e48d` (PR #2 merge `22d6e4f`) | MAC/ISR→queue→bench task 성공만 인정; 500 kbit/s, 0x123, Classic/internal loopback은 bench 설정 유지 |
| B IVI §0.6 | 외부 NOR/HyperRAM, map/flash verify/육안 및 5분 관찰 PASS | display/touch/memory 기반 동결 가능. 이 메모리 시험에 대한 독립적인 tested source SHA와 원본 증거 경로는 후속 보강하며 loopback SHA를 대신 붙이지 않음 |
| B IVI §15 | FULL IVI INTEGRATION: NOT RUN | HMI frame budget/최종 task priority/stack/queue/watchdog OPEN |

기존 시험 보고서의 PASS를 재실행한 것으로 표현하지 않는다. Benchmark stack 2048 B와 queue depth 8은 one-shot loopback 조건의 값이며 최종 IVI나 G431KB 공통값이 아니다. 빈 `Error_Handler` 문제도 기존 미해결 항목으로 유지한다.

## G431KB에서 먼저 확인할 것

[ST MCU 자료](https://www.st.com/en/microcontrollers-microprocessors/stm32g431kb.html)에 따르면 G431KB는 Flash 128 KB, SRAM 총 32 KB(22 KB SRAM + 10 KB CCM SRAM), FDCAN 1개를 제공한다. 실제 linker 배치와 사용 가능한 각 RAM 영역을 확인해 자원 예산을 잡는다.

- A: 전체 TRIG/ECHO 핀, timer input capture/EXTI 경로와 순차 측정 자원.
- C: motor PWM/DIR, servo PWM, encoder timer와 FDCAN 핀의 동시 배정.
- D Gateway: FDCAN과 LIN용 UART/USART의 동시 배정; D Slave: LIN과 sensor/lamp 핀 배정.
- F: accelerator/brake/steering ADC, E-Stop 입력, FDCAN 및 RTOS task/queue/heap 예산.
- 각 구매 보드: SWD/debug/clock/온보드 기능과의 충돌, CAN/LIN transceiver와 전압/전원/종단 구성을 확인.

[NUCLEO-G431KB 공식 보드](https://www.st.com/en/evaluation-tools/nucleo-g431kb.html)와 구매 보드가 같은지는 아직 단정하지 않는다. MCU 기능 지원은 핀 노출/배선/실기 통신 성공과 별개다.

## 다음 동결 순서

정식 조건은 [최상위 명세 §7.1](FINAL_IMPLEMENTATION_SPEC.md#71-단계별-동결-시점)을 따른다.

1. 지금: 모델 선택과 검증된 B 기반 동결 — 완료.
2. Week 1: 실제 부품/BOM·pin/peripheral·자원 배정 확인 후 해당 Hardware Layer 동결.
3. Week 1 말~Week 2 첫 pair 통합 전: CAN/LIN matrix, 의미·단위·byte layout·주기·timeout, heartbeat/DTC 계약 동결. 물리 bitrate는 H735↔G431 및 LIN pair 검증으로 결정한다.
4. VCU/Drive 실제 출력 제어 확정 전: state/enable/arbitration/timeout/safe state/recovery 동결. 후보 정책은 mock/출력 비활성 bench에서 먼저 검증한다.
5. Week 2 계측 후~Week 3 baseline 전: 부하 시험을 근거로 RTOS/Linux 실행 계약 동결. Perception/Vision/HMI/Body 값도 관련 통합 전에 닫는다.
6. 모든 registry와 message/LIN/execution 계약 및 Gate A~D가 닫힌 커밋을 Implementation Baseline v1.0으로 지정한다. 차량 전체 PASS/release는 이후 시험으로 별도 판정한다.

`Driver_Input`의 §4 상세 계약 누락과 §6 실제 task 숫자 표 부재도 각각 Gate B/D 종료 전에 해결해야 한다. 주차가 지났다는 이유로 OPEN을 FROZEN으로 바꾸지 않는다.

## 변경 관리

동결 후 변경은 해당 Decision ID, 변경 이유, 영향 ECU/호환성, 재시험 목록, Owner 승인을 기록하고 최상위 명세→역할 문서→코드 순서로 반영한다. 하드웨어 구매 동결을 실기 PASS로 표시하거나 계획된 Expected/Target을 측정값으로 승격하지 않는다.
