# IVI / Cluster Cockpit Documentation

이 폴더는 **B 담당: STM32H735 + TouchGFX Cluster/IVI Cockpit**의 작성 예시다.

현재 기준은:

```text
STM32H735
+ FreeRTOS
+ CMSIS-RTOS2
+ TouchGFX
+ CAN FD
```

이다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - 기능/화면 요구사항
   - CAN Input/Output
   - Timeout / Edge Case
   - RTOS 실행 요구사항
   - Acceptance Criteria

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - Context / Component / Runtime / Deployment
   - TouchGFX + Vehicle Data Model
   - FreeRTOS Task / Queue / ISR 구조
   - Warning / DTC / CAN 처리
   - Watchdog / Stack / Priority 정책

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Dummy Data Stage 1
   - CAN 통합 시험
   - Timeout / Warning 시험
   - RTOS Task period / jitter
   - Stack / Queue / Watchdog 시험

## 한 줄 역할

```text
CAN FD
  ↓
CanRxTask
  ↓ Queue
VehicleModelTask
  ↓
GuiTask / TouchGFX
  ↓
Cluster + IVI
```

사용자 요청은 Actuator를 직접 움직이지 않는다.

```text
Touch
→ GuiTask
→ Command Queue
→ CommandTxTask
→ CAN FD
→ VCU / Body Gateway
```

## 기본 RTOS Task 후보

| Task | 역할 | Priority 방향 |
|---|---|---|
| `CanRxTask` | CAN 수신/Decode | High |
| `VehicleModelTask` | Repository/Validity/Warning | Normal~High |
| `GuiTask` | TouchGFX rendering/input | Normal |
| `CommandTxTask` | 사용자 Request CAN TX | Normal |
| `HealthTask` | task/queue/stack/watchdog | Low |

정확한 numeric priority, stack size, period는 실측 후 확정한다.

## 주요 TBD

- CAN ID / DLC / signal layout
- H735 FDCAN Transceiver / Pin Map
- 실제 Task priority / stack size
- TouchGFX frame/update 성능
- DTC Clear Request protocol
- Battery SOC Owner
- IWDG/HealthTask 최종 정책

임의 숫자를 넣지 않고 실측과 통합 결과로 채운다.
