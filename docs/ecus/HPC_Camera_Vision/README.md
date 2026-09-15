# HPC + Camera Vision / 인지·판단 Documentation

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> **2026-09-15 범위 변경:** Rear Camera / Rear Vision / 주차 Vision 기능을 삭제했다. Front Camera 1대의 COCO 기반 객체인식만 담당한다. 주차 판단은 A(Ultrasonic)가 전담한다.

> **최상위 구현 기준:** [`../FINAL_IMPLEMENTATION_SPEC.md`](../../system/FINAL_IMPLEMENTATION_SPEC.md)
> Camera/Vision/IPC/CAN/Timing 최종값은 Project Owner가 `FROZEN`한 값만 사용한다.

이 폴더는 **E 담당: Raspberry Pi HPC + Front Camera Vision(COCO 객체인식)**의 하위 구현 문서다.

## 고정 역할

```text
Front Camera (1대)
→ Raspberry Pi Vision (COCO Object Detection)
→ Vision_Status + ADAS_Request
→ VCU / H735
```

E는 고수준 인지와 회피 요청 생성까지 담당한다. 최종 차량 명령과 Motor/Servo PWM은 만들지 않으며, 주차 판단도 만들지 않는다 (A 전담).

## 이미 고정된 규칙

- `Vision_Status`와 `ADAS_Request`를 분리한다.
- `Vision_Status`는 상태/semantic data(`detected_class`, `direction`/`zone`), `ADAS_Request`는 VCU용 고수준 회피 요청이다.
- `ADAS_Request`는 Ultrasonic `Parking Critical`을 해제/override하지 않는다 (항상 Ultrasonic이 우선).
- Raw Camera frame은 CAN으로 보내지 않는다.
- semantic result는 valid/freshness 정보를 가진다.
- frame queue는 bounded 구조이며 최신성을 우선한다.
- `can_service`가 CAN interface single owner다.
- Front Vision은 Gear/Mode와 무관하게 항상 동작한다 (Rear 전환 로직 없음).

## Linux 구조

```text
front_vision ─────┐
vehicle_manager ──┤
health_monitor ───┤→ IPC / latest result
logger ───────────┤
                  ↓
              can_service
                  ↓
                CAN FD
```

## 구현해야 할 것

- Front Camera capture + FPS 측정
- Front COCO semantic output (detected_class + direction/zone) 최소 1개
- bounded frame/result queue
- Vision_Status / ADAS_Request 분리
- camera timeout/disconnect → valid=false
- process supervisor/restart prototype
- mock can_service
- CPU/RAM/temperature/FPS/latency 측정

## Owner가 최종 명세에서 결정할 항목

관련 Decision ID:

```text
DEC-HW-008
DEC-HW-015
DEC-NET-004 ~ DEC-NET-008
DEC-VIS-001, DEC-VIS-003 ~ DEC-VIS-005, DEC-VIS-008
DEC-DTC-001 ~ DEC-DTC-005
DEC-HLT-001
```

Pi/OS, Camera, resolution/FPS, COCO 모델, freshness, IPC/queue policy, Pi CAN FD interface, Vision_Status/ADAS_Request 계약은 독자적으로 최종 확정하지 않는다.

## Coding Gate

`Vision_Status`, `ADAS_Request`, Camera/Model/Resolution/FPS 및 freshness 정책이 `FROZEN`되기 전에는 통합 CAN payload와 최종 inference parameter를 기준값으로 고정하지 않는다.

## Stage 1 PASS

```text
Front capture + FPS
+ COCO semantic result
+ valid/timeout
+ Vision_Status / ADAS_Request 분리
+ bounded queue / metrics
```

상세 기능은 [SPECIFICATION.md](SPECIFICATION.md), 구현 구조는 [ARCHITECTURE.md](ARCHITECTURE.md), 실측 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
