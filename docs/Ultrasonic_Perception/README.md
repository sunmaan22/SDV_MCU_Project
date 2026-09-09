# Ultrasonic Perception Documentation

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../FINAL_IMPLEMENTATION_SPEC.md)  
> Hardware / CAN / threshold / timing / RTOS 최종값은 Project Owner가 위 문서에서 `FROZEN`한 값만 사용한다.

이 폴더는 **A 담당: Ultrasonic / 인지**의 하위 구현 문서다.

## 고정 역할

```text
Ultrasonic Sensor
→ Trigger / Echo
→ Distance / Validity / Filter
→ SAFE / WARNING / CRITICAL
→ Ultrasonic_Status
→ VCU / H735 / HPC
```

A는 거리 인지와 결과 신뢰성까지 담당한다. Motor 정지 판단이나 PWM은 만들지 않는다.

## 이미 고정된 규칙

- `valid`와 `warning_level`을 분리한다.
- `valid=false`이면 해당 측정값을 정상 판단에 사용하지 않는다.
- warning은 정상 데이터에 대해 `SAFE / WARNING / CRITICAL`을 사용한다.
- `Ultrasonic_Status` Publisher는 A다.
- Ultrasonic `CRITICAL`은 VCU에 직접 전달하며 Rear Vision이 이를 해제하지 않는다.
- Local fault는 `fault_flags`, confirmed fault는 공통 `DTC_Event`를 사용한다.
- 여러 센서는 순차 Scan을 기본으로 한다.
- ISR에서는 timestamp/capture만 처리하고 계산/filter/CAN은 Task에서 수행한다.

## FreeRTOS 구조

```text
Timer Capture ISR
   ↓
UltrasonicTask
   ↓
PerceptionTask
   ↓
CanTxTask

HealthTask
```

## 구현해야 할 것

- Sensor 1개 Trigger/Echo Input Capture
- 3-point distance 검증
- timeout / out-of-range / disconnect → `valid=false`
- raw / filtered distance 비교
- Sensor scan 구조
- `Ultrasonic_Status` 생성
- FreeRTOS task/queue/health 구조
- period/jitter/stack/queue 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-001
DEC-HW-009
DEC-PER-001 ~ DEC-PER-004
DEC-NET-004 ~ DEC-NET-007
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001 ~ DEC-HLT-003
```

센서 모델, 센서 개수/Zone, Pin/Timer, Echo voltage, filter, threshold, hysteresis, scan period, CAN ID/DLC/cycle/timeout, DTC, RTOS 숫자는 이 폴더에서 독자적으로 최종 확정하지 않는다.

## Coding Gate

`Ultrasonic_Status` 계약과 센서 Hardware/threshold가 `FROZEN`되기 전에는 CAN payload 상수와 최종 threshold를 코드에 고정하지 않는다.

## Stage 1 PASS

```text
Sensor 1개 측정
+ 3-point 검증
+ timeout/invalid
+ raw/filtered/valid/warning
+ FreeRTOS 실행
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
