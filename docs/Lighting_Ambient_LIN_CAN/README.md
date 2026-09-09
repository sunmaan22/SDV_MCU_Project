# Lighting + Ambient / LIN-CAN Documentation

이 폴더는 **D 담당: Body Gateway + Body LIN Slave + Ambient + Lighting**의 기준 문서다.

## 역할

```text
VCU Body_Command
      ↓ CAN FD
Body Gateway STM32
├ CAN RX/TX
├ CAN↔LIN Mapping
└ LIN Master / Schedule
      ↓ LIN
Body LIN Slave STM32
├ Ambient Sensor
└ Lighting Output
```

## 통합 검토 후 확정된 규칙

1. `Body_Command`의 최종 Publisher는 **VCU 하나**로 둔다.
2. H735의 사용자 설정은 `Body_User_Request`로 VCU에 전달한다.
   - H735 → `Body_User_Request` → VCU → `Body_Command` → Gateway.
   - H735와 VCU가 같은 `Body_Command`를 동시에 publish하지 않는다.
3. Gateway는 단순 byte relay가 아니라 **CAN signal ↔ LIN signal mapping**을 담당한다.
4. LIN Master schedule owner는 Gateway다. Slave가 임의 주기로 bus를 주도하지 않는다.
5. Ambient/Lamp 실제 데이터 Owner는 LIN Slave다.
6. Gateway는 LIN node timeout과 mapping fault를 검출하고 `Body_Status`와 필요 시 공통 `DTC_Event`로 알린다.
7. CAN 장애가 LIN task 전체를 막거나, LIN 장애가 CAN node 전체를 막지 않게 실행경로를 분리한다.
8. Lamp/LED 부하가 MCU GPIO 허용전류를 넘는 경우 적절한 driver stage를 사용하며 실제 부하 사양을 먼저 확인한다.

## FreeRTOS 기본 구조

### Gateway

```text
FDCAN ISR → CanRxTask
                ↓
        GatewayMappingTask
                ↓
        LinScheduleTask
                ↓ LIN

LIN status → GatewayMappingTask → CanTxTask → Body_Status
HealthTask → LIN timeout / queue / stack / watchdog
```

### LIN Slave

```text
LIN ISR → LinRxTask
AmbientTask → latest ambient
LightingTask → lamp output
StatusTask → LIN response data
HealthTask → sensor/output/task health
```

## 지금 개발해야 할 것

- STM32 #4에서 Ambient Sensor 1개를 읽고 raw/converted 값을 확인한다.
- STM32 #4에서 Lamp/LED output을 GPIO/PWM으로 제어한다.
- STM32 #3을 LIN Master, STM32 #4를 LIN Slave로 만들어 첫 Master↔Slave frame 교환을 성공시킨다.
- FreeRTOS Gateway task와 Slave task skeleton을 각각 만든다.
- `Lamp_Command`, `Ambient_Status`, `Lamp_Status`, `Lamp_Diagnostic`의 local data structure를 만든다.
- LIN timeout에서 Gateway가 slave invalid를 만들도록 구현한다.
- 그 다음 CAN dummy `Body_Command` → LIN `Lamp_Command` end-to-end 경로를 구현한다.
- 반대 방향으로 Ambient → LIN → Gateway → `Body_Status` 경로를 구현한다.

## 반드시 결정해야 할 것

- Gateway/Slave 실제 STM32 모델과 peripheral 지원
- CAN FD Transceiver / LIN Transceiver 모델
- LIN bitrate, frame ID, checksum 방식
- LIN schedule table, slot order, slot period
- Ambient Sensor 모델과 ADC/I2C interface
- Ambient 단위와 filtering/calibration
- Lamp 종류, 동작전압/전류, driver 회로 필요 여부
- Head/Tail/Brake/Turn/Hazard 중 실제 구현 범위
- `Body_User_Request`에 허용할 사용자 기능
- VCU가 만드는 `Body_Command` signal 목록
- CAN↔LIN mapping table
- LIN timeout/recovery 조건
- Gateway/Slave DTC 후보
- Task priority/period/stack/queue depth
- 작은 Slave MCU에서 FreeRTOS memory가 충분한지

## Stage 1 PASS

```text
Slave Ambient read
+ Slave Lamp output
+ Gateway LIN Master
+ Master↔Slave 통신
+ FreeRTOS task 구조
```

Stage 2 PASS 후보:

```text
Body_User_Request → VCU Body_Command
→ Gateway CAN→LIN
→ Slave Lamp

Ambient → Slave → LIN → Gateway → Body_Status
```

실제 수치가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
