# HPC + Camera Vision / 인지·판단 Documentation

이 폴더는 **E 담당: Raspberry Pi HPC + Front/Rear Camera Vision**의 기준 문서다.

## 역할

```text
Front / Rear Camera
        ↓
Raspberry Pi Vision
        ↓
Semantic Result
├ Vision_Status
└ ADAS_Request
        ↓ CAN FD
VCU / H735
```

Vision/HPC는 고수준 인지와 요청 생성까지 담당한다. 최종 차량 명령은 VCU가 결정하고, Motor/Servo PWM은 Drive ECU가 만든다.

## 통합 검토 후 확정된 규칙

1. 표시용 상태와 제어요청을 분리한다.
   - `Vision_Status`: lane/object/rear parking/validity/health 등 의미 정보. VCU와 H735가 소비한다.
   - `ADAS_Request`: speed/steering 등 제어요청. VCU만 최종 판단에 사용한다.
   - 기존 `Vision_Request` 하나에 모든 의미를 섞지 않는다.
2. Rear Vision warning은 Ultrasonic `CRITICAL`을 해제하거나 덮어쓰지 않는다.
   - Ultrasonic은 VCU에 직접 들어가는 독립 안전 입력이다.
   - Rear Vision은 추가 의미 정보와 advisory/request를 제공한다.
3. Raw Camera frame은 Pi 내부에서만 사용하고 CAN FD로 보내지 않는다.
4. 모든 semantic result에는 `valid`와 freshness 판단이 가능한 timestamp/age 정보를 유지한다.
5. frame queue는 bounded 구조로 두고 backlog 시 오래된 frame보다 최신성을 우선한다.
6. `can_service`는 CAN interface single owner를 기본안으로 한다.
7. Front/Rear Vision은 개발 중 독립 실행 가능해야 하고, 최종 Pi 1대에서 함께 실행 가능해야 한다.

## Linux 실행 구조

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

## 지금 개발해야 할 것

- 개발용 Pi #1에서 Front Camera capture와 FPS 측정을 완료한다.
- 개발용 Pi #2에서 Rear Camera capture와 FPS 측정을 완료한다.
- Front pipeline에서 최소 1개 semantic output을 만든다. 예: lane offset 또는 object status.
- Rear pipeline에서 최소 1개 parking semantic output을 만든다. 예: object detected/position.
- capture와 processing 사이 bounded queue를 만들고 queue backlog/drop을 로그로 확인한다.
- `Vision_Status`와 `ADAS_Request` data structure를 분리한다.
- Camera disconnect/frame timeout 시 `valid=false`가 되는 health path를 구현한다.
- process crash/restart를 검증할 수 있는 supervisor 방식의 prototype을 만든다.
- CAN hardware가 없어도 mock `can_service`로 result publish를 시험할 수 있게 한다.
- 최종 Pi 1대에서 Front/Rear를 동시에 또는 mode switching으로 돌렸을 때 CPU/RAM/temperature를 측정한다.

## 반드시 결정해야 할 것

- 최종 Raspberry Pi 모델과 OS image
- Front Camera / Rear Camera 실제 모델과 interface
- Camera resolution / target FPS
- Front Vision 최소 기능 범위: lane/object 중 무엇을 Stage 1/2에서 구현할지
- Rear Vision 최소 object class와 position 표현
- `Vision_Status` signal 목록
- `ADAS_Request` signal 목록, 단위, 범위
- result freshness timeout
- Gear D/R mode switching 정책
- Camera capture를 항상 유지할지 Gear에 따라 start/stop할지
- Gear R → Rear first valid result 목표 latency
- Vision algorithm/model/OpenCV 처리 방식
- process supervisor 방식
- IPC 방식과 queue depth/drop policy
- Pi CAN FD interface hardware
- CPU/RAM/thermal budget

## Stage 1 PASS

```text
Front capture + FPS
+ Rear capture + FPS
+ semantic result 1개 이상씩
+ valid/timeout 처리
+ Vision_Status / ADAS_Request 분리
+ bounded queue / metrics
```

실제 수치가 정해지면 [SPECIFICATION.md](SPECIFICATION.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TEST_REPORT.md](TEST_REPORT.md)의 `TBD`를 갱신한다.
