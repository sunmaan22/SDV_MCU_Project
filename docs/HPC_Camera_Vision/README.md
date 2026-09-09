# HPC + Camera Vision / 인지·판단 Documentation

이 폴더는 **E 담당: Raspberry Pi HPC + Front/Rear Camera Vision** 역할의 작성 예시다.

개발 단계에서는 Front Vision과 Rear Vision을 Raspberry Pi 2대로 병렬 개발할 수 있고, 최종 차량에서는 Raspberry Pi 1대로 통합하는 것을 목표로 한다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - Front/Rear Vision이 무엇을 해야 하는지
   - Input / Output / Scenario
   - Gear D/R에 따른 활성화 조건
   - Camera / CAN / IPC Interface
   - FPS / Latency / Fault / Recovery 요구사항

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - Linux Service / Process / Thread 구조
   - Camera Capture / Vision / Vehicle Manager / CAN Service
   - Front/Rear Vision 분리 개발과 최종 통합
   - IPC Queue / Shared State / Logging / Health
   - Runtime / Deployment / Fault Recovery

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Front/Rear Camera capture
   - Lane/Object/Parking Vision 결과
   - FPS / Inference latency
   - Gear D/R Camera mode switching
   - Process crash/restart
   - CAN result transmission
   - CPU / Memory / Queue / Soak test

## 역할을 쉽게 보면

```text
Front Camera / Rear Camera
          ↓
     Raspberry Pi
          ↓
   영상에서 의미 추출
          ↓
Lane / Object / Parking / Warning
          ↓
  ADAS / Parking Request 생성
          ↓ CAN FD
         VCU
```

중요한 역할 경계:

```text
Pi Vision
→ "이런 상황으로 보인다"
→ "이 정도 속도/조향 요청이 필요하다"

VCU
→ 운전자 입력 / Fault / Parking / ADAS를 모두 확인
→ 최종 Speed / Steering 결정
```

즉 Pi가 Motor PWM이나 Servo PWM을 직접 만들지 않는다.

## 개발 단계

```text
Developer Pi #1
Front Camera
→ Front ADAS Vision

Developer Pi #2
Rear Camera
→ Rear Parking Vision
```

최종 단계:

```text
Front Camera ─┐
              ├→ Raspberry Pi HPC
Rear Camera ──┘
```

권장 운용:

```text
Gear D
→ Front Vision ACTIVE
→ Rear Vision IDLE

Gear R
→ Front Vision PAUSE/IDLE
→ Rear Vision ACTIVE
```

## Raw Camera 원칙

Camera Frame 자체는 Pi 내부에서 처리한다.

```text
Raw Frame
→ OpenCV / AI / Vision Pipeline
→ Semantic Result
→ CAN
```

CAN으로 내보낼 후보:
- lane_offset
- lane_angle
- object_detected
- collision_level
- speed_request
- steering_request
- rear_object_detected
- rear_object_position
- parking_vision_warning
- vision_valid

## 실행 환경

```text
Raspberry Pi
→ Linux
→ Process / Service / Thread
→ Queue / IPC / Socket / Shared State 후보
```

이 Node에는 FreeRTOS를 억지로 적용하지 않는다.

## 현재 주요 TBD

- 최종 Raspberry Pi 정확한 모델/OS image
- Front/Rear Camera 정확한 모델
- 최종 Resolution / FPS
- Vision Algorithm / Model
- Object class 범위
- Lane representation
- CAN interface hardware
- CAN ID / DLC / signal layout
- Process supervisor 방식
- IPC 방식
- Gear R 전환 후 first frame latency 목표
- CPU / Memory budget

실제 측정 전에는 숫자를 확정값처럼 적지 않는다.
