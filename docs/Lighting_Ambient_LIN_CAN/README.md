# Lighting + Ambient / LIN-CAN Documentation

이 폴더는 **D 담당: Body Gateway + Body LIN Slave + Ambient + Lighting** 역할의 작성 예시다.

현재 프로젝트 공통 Template을 실제 역할에 맞춰 채운 상태이며, 실제 MCU 모델/핀/Transceiver/LIN bitrate/CAN ID가 확정되면 `TBD`, `후보`, `NOT RUN` 항목을 실제 값으로 교체한다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - 무엇을 해야 하는지
   - Gateway와 LIN Slave의 역할 경계
   - CAN→LIN / LIN→CAN 기능
   - Ambient / Lighting 기능
   - Fault / Timeout / DTC
   - FreeRTOS 실행 요구사항

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - Gateway STM32와 LIN Slave STM32의 SW 구조
   - CAN/LIN ISR → Task 구조
   - LIN Schedule
   - CAN↔LIN Mapping
   - Lighting/Ambient Task
   - Queue / Event / Watchdog / Health

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Ambient 측정
   - Lamp 출력
   - LIN Master↔Slave
   - CAN→LIN / LIN→CAN End-to-End
   - Timeout / Disconnect / DTC
   - RTOS Timing / Stack / Queue / Watchdog

## 역할을 아주 쉽게 보면

```text
차량 CAN FD
   ↕
[Body Gateway STM32]
CAN ↔ LIN 변환
LIN Master / Schedule
   ↕ LIN
[Body LIN Slave STM32]
├ Ambient Sensor 읽기
└ Lighting 출력
```

CAN에서 조명 요청이 오면:

```text
H735 / VCU
→ Body_Command over CAN
→ Gateway
→ LIN Lamp_Command
→ LIN Slave
→ Lamp Output
```

반대로 조도값은:

```text
Ambient Sensor
→ LIN Slave
→ Ambient_Status over LIN
→ Gateway
→ Body_Status over CAN
→ H735 / VCU / HPC
```

## 중요한 역할 경계

- Gateway STM32는 **CAN FD Node + LIN Master + CAN↔LIN Signal Mapper**다.
- LIN Slave STM32는 **Ambient Sensor와 Lamp Output의 실제 Owner**다.
- H735가 Lamp GPIO를 직접 제어하지 않는다.
- VCU/H735는 `Body_Command` 요청만 보낸다.
- Gateway는 Lamp를 직접 구동하는 대신 LIN으로 명령을 전달한다.
- LIN Slave는 실제 Lamp 상태와 Local Fault를 보고한다.

## 실행 환경

```text
Body Gateway STM32
→ FreeRTOS + CMSIS-RTOS2 기본

Body LIN Slave STM32
→ FreeRTOS 기본
→ MCU RAM/Flash가 너무 작으면 Architecture Decision을 남기고 예외 검토
```

## 현재 주요 TBD

- Gateway / Slave 실제 STM32 모델
- FDCAN 지원 여부
- CAN FD Transceiver 모델
- LIN Transceiver 모델
- LIN bitrate / frame ID / checksum mode
- LIN Schedule period
- Ambient Sensor 모델/Interface
- Lamp 회로와 Driver/Transistor 필요 여부
- Lamp current / voltage
- CAN ID / DLC / signal layout
- Task numeric priority / stack / queue depth

이 값들은 Datasheet와 실제 시험 없이 상상으로 확정하지 않는다.
