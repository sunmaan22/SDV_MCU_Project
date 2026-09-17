# Driver_Input Firmware Workspace

NUCLEO-G431KB (STM32G431KBT6) 기반, C ECU의 RF 입력부(기어·조향·속도 요청)를 먼저 검증하는 bench 단독 프로젝트다. 별도 ECU가 아니라 `Motor_Steering_Control` 역할의 입력 검증 단계이며, 상세 절차는 [DRIVER_INPUT_START.md](../../docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md)를 따른다.

## 현재 상태

CubeMX generated HAL/BSP 초기화 + LED/COM1 샘플만 있다. RF 수신(SPI/nRF24L01), FDCAN, RTOS 입력 처리는 아직 없다. 빌드·다운로드·실기 동작은 아직 시험하지 않았다.

## Generated code 규칙

- `USER CODE BEGIN/END` 밖의 generated source(HAL/BSP init)를 수동 수정하지 않는다.
- 수신 처리가 커지면 `Core/Src/main.c`의 USER CODE 구간에서 `driver_input.c/.h` 한 쌍으로 분리한다.
- SPI/CE/CSN/IRQ 핀 배정과 RF 패킷 레이아웃은 [DRIVER_INPUT_START.md](../../docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md)의 미정(TBD) 항목이 확정된 뒤 코드에 반영한다.

## 진행 순서

자세한 절차와 완료 기준은 `docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md`와 [TEST_REPORT.md](../../docs/ecus/Motor_Steering_Control/TEST_REPORT.md)를 따른다.
