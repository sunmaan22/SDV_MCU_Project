# HPC + Camera Vision / 인지·판단 Documentation

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../FINAL_IMPLEMENTATION_SPEC.md)  
> Camera/Vision/IPC/CAN/Timing 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **E 담당: Raspberry Pi HPC + Front/Rear Camera Vision**의 하위 구현 문서다.

## 고정 역할

```text
Front / Rear Camera
→ Raspberry Pi Vision
→ Vision_Status + ADAS_Request
→ VCU / H735
```

E는 고수준 인지와 요청 생성까지 담당한다. 최종 차량 명령과 Motor/Servo PWM은 만들지 않는다.

## 이미 고정된 규칙

- `Vision_Status`와 `ADAS_Request`를 분리한다.
- `Vision_Status`는 상태/semantic data, `ADAS_Request`는 VCU용 고수준 요청이다.
- Rear Vision이 Ultrasonic `CRITICAL`을 해제하지 않는다.
- Raw Camera frame은 CAN으로 보내지 않는다.
- semantic result는 valid/freshness 정보를 가진다.
- frame queue는 bounded 구조이며 최신성을 우선한다.
- `can_service`가 CAN interface single owner다.
- Front/Rear는 독립 개발 가능하고 최종 Pi 1대에서 통합 가능해야 한다.

## Linux 구조

```text
front_vision ─────┐
rear_vision ──────┤
vehicle_manager ──┤
health_monitor ───┤→ IPC / latest result
logger ───────────┤
                  ↓
              can_service
                  ↓
                CAN FD
```

## 구현해야 할 것

- Front/Rear Camera capture + FPS 측정
- Front semantic output 최소 1개
- Rear parking semantic output 최소 1개
- bounded frame/result queue
- Vision_Status / ADAS_Request 분리
- camera timeout/disconnect → valid=false
- process supervisor/restart prototype
- mock can_service
- 최종 Pi 통합 CPU/RAM/temperature/FPS/latency 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-008
DEC-HW-015 ~ DEC-HW-016
DEC-NET-004 ~ DEC-NET-008
DEC-VIS-001 ~ DEC-VIS-007
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001
```

Pi/OS, Camera, resolution/FPS, Vision 기능/모델, freshness, Gear switching, IPC/queue policy, Pi CAN FD interface, Vision_Status/ADAS_Request 계약은 독자적으로 최종 확정하지 않는다.

## Coding Gate

`Vision_Status`, `ADAS_Request`, Camera/Model/Resolution/FPS 및 freshness 정책이 `FROZEN`되기 전에는 통합 CAN payload와 최종 inference parameter를 기준값으로 고정하지 않는다.

## Stage 1 PASS

```text
Front capture + FPS
+ Rear capture + FPS
+ semantic result
+ valid/timeout
+ Vision_Status / ADAS_Request 분리
+ bounded queue / metrics
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
