# Lighting + Ambient / LIN-CAN Documentation

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../FINAL_IMPLEMENTATION_SPEC.md)  
> CAN/LIN/Lamp/Ambient/RTOS 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **D 담당: Body Gateway + Body LIN Slave + Ambient + Lighting**의 하위 구현 문서다.

## 고정 역할

```text
VCU Body_Command
→ Body Gateway
→ CAN↔LIN Mapping
→ LIN Master
→ Body LIN Slave
→ Ambient / Lighting
```

## 이미 고정된 규칙

- `Body_Command` Publisher는 VCU 하나다.
- H735는 `Body_User_Request`만 만든다.
- Gateway는 CAN↔LIN mapping과 LIN schedule owner다.
- LIN Slave는 Ambient / Lamp actual state owner다.
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
AmbientTask
LightingTask
StatusTask
HealthTask
```

## 구현해야 할 것

- Ambient Sensor read
- Lamp/LED GPIO/PWM output
- Gateway LIN Master + Slave first frame
- Lamp_Command / Ambient_Status / Lamp_Status / Lamp_Diagnostic local structure
- LIN timeout → slave invalid
- Body_Command → LIN → Lamp
- Ambient → LIN → Body_Status
- RTOS timing/queue/stack/watchdog 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-003 ~ DEC-HW-007
DEC-HW-014
DEC-NET-001 ~ DEC-NET-012
DEC-BODY-001 ~ DEC-BODY-003
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

Gateway/Slave MCU, CAN/LIN transceiver, LIN bitrate/frame/checksum/schedule, Ambient sensor/filter, Lamp 범위/전기조건, Body_User_Request/Body_Command, mapping table, timeout, DTC, RTOS 수치는 독자적으로 최종 확정하지 않는다.

## Coding Gate

LIN frame set/schedule과 `Body_Command`/`Body_Status` 계약이 `FROZEN`되기 전에는 최종 Frame ID, CAN ID, bit layout을 코드에 고정하지 않는다.

## Stage 1 PASS

```text
Slave Ambient read
+ Slave Lamp output
+ Gateway LIN Master
+ Master↔Slave
+ FreeRTOS
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
