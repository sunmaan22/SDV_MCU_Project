# VCU + DTC + CAN Integration Documentation

이 폴더는 **F 담당: VCU + DTC + CAN Integration** 역할의 작성 예시다.

VCU는 차량 전체에서 **최종 판단**을 담당한다. Driver Input, Vision Request, Ultrasonic Warning, 각 ECU Fault를 받아 실제 Drive/Steering ECU에 전달할 최종 명령을 만든다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - Driver Input
   - Vehicle State / Mode
   - Arbitration / Safety Priority
   - Final Speed / Steering Request
   - Heartbeat / Timeout
   - DTC 규칙과 통합
   - FreeRTOS 요구사항

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - FreeRTOS Task 구조
   - Driver Input → Vehicle State → Arbitration
   - CAN RX/TX
   - Safety / E-Stop
   - Diagnostics / DTC integration
   - Health / Watchdog

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Gear / Accelerator / Brake / Steering 입력
   - Arbitration
   - Command Timeout
   - E-Stop
   - CAN 통신
   - DTC / Heartbeat
   - RTOS Timing / Stack / Queue / Watchdog

## 역할을 쉽게 보면

```text
Driver Input ───────────────┐
Front/Rear Vision Request ──┤
Ultrasonic Warning ─────────┤
ECU Fault / Heartbeat ──────┤
                            ↓
                        [ VCU ]
                            ↓
               Final Speed / Steering
                            ↓ CAN FD
                  Drive + Steering ECU
```

VCU가 하는 일:

```text
"엑셀 40%"를 읽는다
"ADAS가 감속 요청"을 받는다
"초음파가 CRITICAL"인지 본다
"E-Stop이 눌렸는지" 본다
"Drive ECU가 살아있는지" 본다
        ↓
우선순위와 안전조건 적용
        ↓
최종 명령 생성
```

기본 우선순위 개념:

```text
E-Stop / Critical Fault
> Parking Critical
> ADAS Safety Request
> Normal Driver Request
```

정확한 정책은 시험과 팀 합의 후 확정한다.

## DTC 역할

DTC는 VCU 혼자 만드는 기능이 아니다.

```text
각 ECU
→ 자기 Fault 검출
→ DTC Event
→ Pi DTC Manager / History
→ H735 Diagnostics

VCU
→ DTC 규칙 통합
→ Critical Fault 시 차량 Safe Action
```

## 실행 환경

```text
STM32 #5 VCU
→ FreeRTOS + CMSIS-RTOS2 기본
```

중요 Task 후보:

- `SafetyTask`
- `VcuControlTask`
- `DriverInputTask`
- `CanRxTask`
- `CanTxTask`
- `DiagnosticTask`
- `HealthTask`

## 현재 주요 TBD

- VCU 실제 STM32 모델
- FDCAN 지원 여부 / Transceiver
- Driver Input 실제 센서/회로/핀
- Accelerator/Brake scaling
- Steering calibration
- Arbitration 세부 규칙
- Vehicle state machine 세부 상태
- CAN ID / DLC / cycle / timeout
- DTC code numbering / severity
- Task numeric priority / period / stack / queue depth

미정값은 실제 부품/통합 결과 전까지 `TBD`로 둔다.
