# VCU + DTC + CAN Integration Documentation

이 폴더는 **F 담당: VCU + DTC + CAN Integration**의 기준 문서다.

VCU는 차량 전체에서 **최종 판단과 명령 통합**을 담당한다.

## 역할

```text
Driver Input ───────────────┐
ADAS_Request ───────────────┤
Vision_Status ──────────────┤
Ultrasonic_Status ──────────┤
Drive/Body Status ──────────┤
Heartbeat / DTC ────────────┤
Body_User_Request ──────────┘
                            ↓
                           VCU
                    Safety + Arbitration
                            ↓
        ┌───────────────────┴──────────────────┐
        ↓                                      ↓
Final_Drive_Command                     Body_Command
        ↓                                      ↓
Drive + Steering ECU                  Body Gateway
```

## 통합 검토 후 확정된 규칙

1. 최종 Drive command의 single owner는 VCU다.
   - `VcuControlTask`만 `final_command`를 작성한다.
2. Vision은 두 종류로 분리해서 받는다.
   - `Vision_Status`: 인지/상태 정보
   - `ADAS_Request`: speed/steering 등 고수준 요청
3. Ultrasonic `CRITICAL`은 Rear Vision과 독립적으로 VCU에 직접 들어간다. Vision이 Ultrasonic critical을 해제하지 않는다.
4. H735가 만든 조명 설정은 `Body_User_Request`로 VCU에 온다.
   - VCU가 최종 `Body_Command`를 만들어 Gateway에 보낸다.
   - `Body_Command` publisher를 VCU 하나로 유지한다.
5. VCU가 감지할 통신 timeout은 `Drive_Status`/Heartbeat/peer message timeout이다.
   - `Final_Drive_Command` 수신 timeout은 Drive ECU가 감지한다.
6. DTC History DB는 Pi가 소유한다.
   - VCU `DiagnosticTask`는 runtime fault, severity, safety relevance와 safe action 연계를 담당한다.
   - VCU에 별도 history DB를 만들지 않는다.
7. E-Stop / Critical Fault는 일반 Driver/ADAS 요청보다 우선한다.
8. Brake와 Accelerator가 동시에 의미 있게 입력되면 **Brake 우선**을 기본 정책으로 한다. 실제 threshold는 calibration 후 결정한다.
9. D↔R 방향 변경은 차량이 움직이는 상태에서 즉시 반전시키지 않는다. stop 조건과 허용 threshold를 정한 뒤 적용한다.
10. `SafetyTask`가 직접 Final Command를 쓰지 않는다. Safety override state를 갱신하고 `VcuControlTask`를 즉시 깨워 single-writer 원칙을 유지한다.

## FreeRTOS 기본 구조

```text
E-Stop ISR / Critical Event
        ↓
    SafetyTask
        ↓ safety override + notify
  VcuControlTask
        ↓
 final command single writer
        ↓
    CanTxTask
        ↓
Final_Drive_Command

DriverInputTask ─────┐
CanRxTask ───────────┤→ VcuControlTask
DiagnosticTask ──────┤
HealthTask ──────────┘
```

## 초기 Arbitration 방향

```text
E-Stop / Critical Fault
> Ultrasonic Parking Critical
> ADAS Safety Request
> Normal Driver Request
```

세부 speed limit, recovery, steering 처리 수치는 실제 시험 후 확정한다.

## 지금 개발해야 할 것

- Gear P/R/N/D GPIO 입력을 읽고 enum으로 변환한다.
- Accelerator/Brake ADC를 읽어 raw와 normalized 값을 출력한다.
- Steering input sensor를 읽고 normalized steering 값을 만든다.
- E-Stop event를 별도 safety path로 처리한다.
- FreeRTOS `SafetyTask`, `VcuControlTask`, `DriverInputTask`, `CanRxTask`, `CanTxTask`, `DiagnosticTask`, `HealthTask` skeleton을 만든다.
- Dummy Driver Input + Dummy ADAS_Request + Dummy Ultrasonic_Status를 넣어 arbitration을 재현한다.
- `Final_Drive_Command` data structure와 latest-value 전송 구조를 만든다.
- `Body_User_Request` → `Body_Command` 경로를 dummy data로 만든다.
- peer message마다 last_rx timestamp를 저장하고 stale/timeout 처리를 구현한다.
- 공통 `DTC_Event` 최소 필드 후보를 정리한다: source/node, code, status, severity, timestamp/sequence 후보.
- E-Stop이 발생했을 때 `VcuControlTask`가 즉시 wake-up되는 latency를 측정할 수 있게 한다.

## 반드시 결정해야 할 것

- VCU 실제 STM32와 FDCAN/Transceiver
- Gear/Accel/Brake/Steering/E-Stop 실제 sensor, pin, voltage
- Accelerator/Brake calibration range와 deadband
- Brake-over-Accelerator threshold
- Steering center/range/calibration
- Vehicle state machine: INIT/READY/D/R/N/P/FAULT 등의 최종 상태
- READY/Drive Enable 조건
- D↔R 변경 허용 조건과 stop speed threshold
- E-Stop 해제 후 자동복구인지 재승인 필요인지
- Ultrasonic SAFE/WARNING/CRITICAL에 대한 VCU action
- ADAS_Request arbitration 방식
- `Final_Drive_Command` signal/단위/range/cycle/timeout
- `Body_User_Request` 및 `Body_Command` signal 범위
- Heartbeat source/node 식별 방식과 timeout
- 공통 `DTC_Event` field, status, severity, confirmation/clear rule
- Critical DTC → VCU safe action mapping
- Task period/priority/stack/queue depth
- Watchdog refresh 조건

## Stage 1 PASS

```text
Driver Input 5종 읽기
+ FreeRTOS task 구조
+ Dummy arbitration
+ E-Stop priority
+ Final_Drive_Command 생성
+ stale/timeout 처리
```

실제 수치가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
