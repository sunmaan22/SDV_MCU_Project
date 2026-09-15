# Lighting / LIN-CAN Documentation

> **2026-09-15 범위 변경:** Ambient Sensor 기능을 삭제했다 (`DEC-HW-014`/`DEC-BODY-003` REMOVED). 헤드램프는 밝기값으로 제어하고, `brake_lamp`는 F가 감속 감지로 자동 생성한 값을 D가 그대로 중계한다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)
> CAN/LIN/Lamp/RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **D 담당: Body Gateway + Body LIN Slave + Lighting**의 하위 구현 문서다.

## 고정 역할

```text
VCU Body_Command (headlamp_brightness/turn/brake_lamp)
→ Body Gateway
→ CAN↔LIN Mapping
→ LIN Master
→ Body LIN Slave
→ Lighting
```

## 이미 고정된 규칙

- `Body_Command` Publisher는 VCU 하나다.
- H735는 턴시그널 요청과 헤드램프 밝기 요청만 `Body_User_Request`로 만든다 (`DEC-BODY-002` FROZEN).
- `brake_lamp`는 사용자 요청이 아니라 F가 감속을 감지해 자동 생성한다 (`DEC-CTRL-020`). D는 이 값을 그대로 LIN으로 중계할 뿐 판단하지 않는다.
- Gateway는 CAN↔LIN mapping과 LIN schedule owner다.
- LIN Slave는 Lamp actual state owner다 (Ambient는 삭제됨).
- LIN 장애와 CAN 장애가 서로의 task 전체를 hang시키지 않도록 분리한다.
- Lamp 부하가 MCU GPIO 정격을 넘으면 driver stage를 사용한다.

## FreeRTOS 구조

### Gateway

```text
FDCAN ISR → CanRxTask
→ GatewayMappingTask
→ LinScheduleTask

LIN status
→ GatewayMappingTask
→ CanTxTask

HealthTask
```

### Slave

```text
LIN ISR → LinRxTask
LightingTask
StatusTask
HealthTask
```

## 구현해야 할 것

- Lamp/LED GPIO/PWM output (헤드램프는 밝기 PWM)
- Gateway LIN Master + Slave first frame
- Lamp_Command / Lamp_Status / Lamp_Diagnostic local structure
- LIN timeout → slave invalid
- Body_Command → LIN → Lamp
- Lamp state → LIN → Body_Status
- RTOS timing/queue/stack/watchdog 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-003 ~ DEC-HW-007
DEC-NET-001 ~ DEC-NET-012
DEC-BODY-001, DEC-BODY-002
DEC-CTRL-020
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Gateway/Slave MCU, CAN/LIN transceiver, LIN bitrate/frame/checksum/schedule, Lamp 범위/전기조건, Body_User_Request/Body_Command, mapping table, timeout, DTC, RTOS 수치는 독자적으로 최종 확정하지 않는다.

## Coding Gate

LIN frame set/schedule과 `Body_Command`/`Body_Status` 계약이 `FROZEN`되기 전에는 최종 Frame ID, CAN ID, bit layout을 코드에 고정하지 않는다.

## Stage 1 PASS

```text
Slave Lamp output (헤드램프 밝기 포함)
+ Gateway LIN Master
+ Master↔Slave
+ FreeRTOS
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
