# IVI / Cluster Cockpit Documentation

이 폴더는 **B 담당: STM32H735 + TouchGFX Cluster/IVI Cockpit**의 기준 문서다.

## 역할

```text
CAN FD
  ↓
CanRxTask
  ↓
VehicleModelTask
  ↓
GuiTask / TouchGFX
  ↓
Cluster + IVI
```

H735는 차량 상태를 **표시하고 사용자 요청을 생성**한다. 최종 차량 제어 판단이나 Lamp GPIO 제어를 직접 하지 않는다.

## 통합 검토 후 확정된 규칙

1. Vision 데이터는 표시용 `Vision_Status`와 제어요청용 `ADAS_Request`를 구분한다.
   - H735는 기본적으로 `Vision_Status`를 표시한다.
   - `ADAS_Request`의 최종 승인 여부는 VCU 책임이다.
2. H735가 Body Gateway에 최종 `Body_Command`를 직접 발행하지 않는다.
   - Touch 설정 → `Body_User_Request` → VCU
   - VCU가 검증/통합 후 `Body_Command` → Body Gateway
   - 이렇게 해서 조명 명령의 최종 Publisher를 하나로 유지한다.
3. H735의 WarningManager는 **표시 우선순위**만 결정한다. 센서 threshold나 차량 안전 판단을 다시 계산하지 않는다.
4. Battery SOC / Temperature는 데이터 Owner가 정해질 때까지 핵심 MUST 표시항목으로 취급하지 않는다. Source가 정해진 뒤 추가한다.
5. DTC live event는 표시할 수 있지만, History의 canonical source는 Raspberry Pi DTC Manager다.
6. CAN frame decode와 TouchGFX rendering은 같은 실행경로에 직접 묶지 않는다.

## FreeRTOS 기본 구조

```text
FDCAN ISR
  ↓ CanRxQueue
CanRxTask
  ↓ ModelUpdateQueue
VehicleModelTask
  ↓ Repository
GuiTask / TouchGFX

GuiTask
  ↓ UiCommandQueue
CommandTxTask
  ↓ Body_User_Request / Diagnostic Request

HealthTask
```

## 지금 개발해야 할 것

- TouchGFX로 `Cluster`, `ADAS`, `Parking`, `Diagnostics`, `Settings` 5개 화면을 만든다.
- DummyDataProvider로 CAN 없이 Speed/RPM/Gear/Warning/DTC를 화면에 표시한다.
- `VehicleDataRepository`를 만들고 View가 CAN frame을 직접 decode하지 않게 한다.
- `valid=false`, timeout, unknown DTC 상태의 표시 방식을 만든다.
- Critical Warning overlay가 현재 화면과 무관하게 뜨도록 구현한다.
- FreeRTOS `CanRxTask`, `VehicleModelTask`, `GuiTask`, `CommandTxTask`, `HealthTask` skeleton을 만든다.
- `Body_User_Request`를 dummy queue로 생성하고 송신 계층과 UI를 분리한다.
- Touch 응답시간과 GUI update 부하를 측정할 수 있는 timestamp/log 구조를 넣는다.

## 반드시 결정해야 할 것

- H735 실제 FDCAN pin과 CAN FD Transceiver
- Cluster 기본 화면의 최종 필수 항목
- Battery Voltage/SOC/Temperature의 데이터 Owner와 message
- 화면별 signal 목록과 invalid 표시 방식
- Warning 표시 우선순위
- Gear R 시 Parking 화면 자동진입 여부
- `Body_User_Request`에 포함할 사용자 기능 범위
- DTC clear를 이번 프로젝트에서 구현할지, 구현한다면 request/response 규칙
- TouchGFX update rate / frame budget
- CAN→UI 목표 latency
- Task priority, stack, queue depth
- IWDG/HealthTask 적용 범위

## Stage 1 PASS

```text
5개 화면 전환
+ Dummy Data 표시
+ invalid/critical warning 표시
+ FreeRTOS task 분리
+ UI Request가 Queue까지 전달
```

실제 수치와 interface가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
