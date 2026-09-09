# Ultrasonic Perception Documentation

이 폴더는 **A 담당: Ultrasonic / 인지** 기능의 기준 문서다.

## 역할

```text
Ultrasonic Sensor
        ↓
Trigger / Echo
        ↓
거리 계산
        ↓
Validity / Filtering
        ↓
SAFE / WARNING / CRITICAL
        ↓ CAN FD
VCU / H735 / HPC
```

A 담당은 **거리 인지와 그 결과의 신뢰성**까지 책임진다. Motor 정지 여부를 직접 결정하거나 PWM을 만들지 않는다.

## 통합 검토 후 확정된 규칙

1. `valid`와 `warning_level`을 분리한다.
   - `valid=false`이면 해당 거리값과 warning을 정상 판단에 사용하지 않는다.
   - `warning_level`은 정상 측정에 대해 `SAFE / WARNING / CRITICAL`만 사용한다.
   - `INVALID`를 warning enum에 중복으로 넣지 않는다.
2. 정상 상태는 `Ultrasonic_Status`로 송신한다.
3. Local fault는 status 내부 `fault_flags`로 제공하고, 확정된 고장 이벤트는 공통 `DTC_Event` 형식을 사용한다.
4. Ultrasonic `CRITICAL`은 VCU로 직접 전달한다. Rear Vision이 이 상태를 덮어쓰거나 해제하지 않는다.
5. 여러 센서를 사용할 경우 동시 Trigger보다 순차 Scan을 기본안으로 한다.
6. ISR에서는 Echo timestamp/capture 상태만 처리하고 거리 계산, filtering, warning, CAN 송신은 Task에서 한다.

## FreeRTOS 기본 구조

```text
Timer Input Capture ISR
        ↓ Notification
UltrasonicTask
        ↓ MeasurementQueue
PerceptionTask
        ↓ StatusQueue
CanTxTask

HealthTask
└ Sensor / Task / Queue / Stack / Watchdog health
```

## 지금 개발해야 할 것

- 실제 Ultrasonic Sensor 1개를 STM32에 연결해 Trigger/Echo를 Timer Input Capture로 측정한다.
- 최소 3개 실제 거리 지점에서 raw echo와 계산 거리값을 기록한다.
- timeout, out-of-range, sensor disconnect에서 `valid=false`가 되는지 확인한다.
- raw distance와 filtered distance를 둘 다 출력해 filtering 효과를 비교한다.
- FreeRTOS `UltrasonicTask`, `PerceptionTask`, `CanTxTask`, `HealthTask` skeleton을 만든다.
- 여러 센서 확장 전에 1개 센서에서 측정 주기와 jitter를 확인한다.
- CAN 없이도 `distance_mm`, `valid`, `warning_level`, `fault_flags`를 UART/debugger로 확인 가능하게 한다.

## 반드시 결정해야 할 것

- 실제 Sensor 모델과 동작전압
- Echo logic voltage와 level shifting 필요 여부
- 센서 개수와 장착 위치/zone
- Trigger/Echo Pin 및 Timer Channel
- 센서별 유효 측정 범위
- 거리 보정식 또는 calibration 방식
- Filtering 방식과 파라미터
- SAFE/WARNING/CRITICAL threshold
- threshold hysteresis 또는 recovery 조건
- Sensor scan 순서와 sensor 간 gap
- 전체 scan/update period
- `Ultrasonic_Status`에 sensor array를 넣을지 zone summary를 넣을지
- CAN cycle/timeout
- DTC 후보와 fault confirmation count
- Task period, priority, stack, queue depth

## Stage 1 PASS

```text
1개 Sensor 측정 성공
+ 3-point 거리 검증
+ timeout/invalid 검증
+ FreeRTOS task 실행
+ raw/filtered/valid/warning 확인
```

실제 수치가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
