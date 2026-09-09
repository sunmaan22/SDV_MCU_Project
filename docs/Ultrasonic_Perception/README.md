# Ultrasonic Perception Documentation

이 폴더는 **A 담당: Ultrasonic / 인지** 기능의 작성 예시다.

공통 Template을 현재 프로젝트 구조와 FreeRTOS 정책에 맞춰 실제로 채운 상태다. 아직 센서 모델, 센서 개수, 정확한 핀, CAN ID, Warning 거리 기준이 확정되지 않은 항목은 임의 숫자로 채우지 않고 `TBD`로 유지한다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - 무엇을 측정해야 하는지
   - 입력/출력과 거리/Validity/Warning 정의
   - Sensor Timeout / Invalid / Crosstalk 같은 Edge Case
   - CAN FD Interface
   - FreeRTOS Task / ISR 요구사항
   - Acceptance Criteria

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - Trigger/Echo 측정 구조
   - Timer Input Capture ISR
   - `UltrasonicTask`, `PerceptionTask`, `CanTxTask`, `HealthTask`
   - Queue / Task Notification 구조
   - Filtering / Warning State
   - CAN Interface / DTC / Watchdog
   - Architecture Decision / Risk / Traceability

3. [TEST_REPORT.md](TEST_REPORT.md)
   - 3-point distance test
   - 반복 측정 / 오차 / 경계값 시험
   - Timeout / Out-of-range / Sensor disconnect
   - 여러 센서 간섭 시험
   - CAN TX 시험
   - RTOS Period / Jitter / Stack / Queue / ISR→Task / Watchdog 시험

## 역할 한 줄 요약

```text
Ultrasonic Sensor
        ↓
Trigger / Echo 측정
        ↓
거리 계산 / 유효성 / 필터
        ↓
SAFE / WARNING / CRITICAL
        ↓ CAN FD
VCU / H735 / Raspberry Pi
```

A 담당은 **주변 거리를 정확하게 인지해서 상태로 전달하는 것**까지 책임진다.

```text
Ultrasonic ECU
→ "Rear Right = 180 mm, CRITICAL"
```

까지는 A의 역할이다.

```text
"그래서 Motor를 정지시킨다"
```

라는 최종 판단과 제어는 VCU와 Drive/Steering ECU의 역할이다.

## FreeRTOS 기본 구조

```text
Timer Input Capture ISR
        ↓ Task Notification
  UltrasonicTask
        ↓ Measurement Queue
   PerceptionTask
        ↓ Status Queue
      CanTxTask

      HealthTask
   └ Task/Queue/Sensor Health
```

ISR에서는 Echo edge의 timestamp/상태만 최소 처리하고 거리 계산, 필터, 로그, CAN 송신은 Task에서 수행한다.

## 현재 주요 TBD

- 실제 Ultrasonic Sensor 모델
- 센서 개수와 차량 장착 위치
- Trigger/Echo 실제 Pin / Timer Channel
- Echo logic voltage와 level shifting 필요 여부
- 거리 변환 계수/보정 방식
- Filtering 방식과 파라미터
- SAFE / WARNING / CRITICAL threshold
- 측정 Scan Period / CAN 송신 Cycle
- CAN ID / DLC / bit layout
- DTC Code 숫자
- 실제 Task priority number / stack size / queue depth

이 값들은 Datasheet와 Stage 1 측정 결과를 근거로 확정한다.
