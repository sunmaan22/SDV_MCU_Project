# Documentation Index

> 기준: **Architecture v1.2 / 2026-09-09 full docs sync**  
> 처음 참여한 팀원은 이 문서에서 시작한다.

## 1. 프로젝트를 한 줄로

```text
인지 → 판단 → 제어
      +
UI / 통신 / 진단
```

현재 6인 역할은 다음과 같다.

| 담당 | 역할 | 쉽게 말하면 | 주요 하드웨어 |
|---|---|---|---|
| A | Ultrasonic / 인지 | 장애물까지 거리를 잰다 | STM32 #1 + Ultrasonic |
| B | Cluster + IVI / UI | 차량 상태와 경고를 보여준다 | STM32H735 + TouchGFX |
| C | Motor + Steering / 제어 | 실제 모터와 조향 서보를 움직인다 | STM32 #2 + TB6612FNG 후보 + DC Motor + RC Servo |
| D | Lighting + Ambient / LIN-CAN | 조도와 조명을 제어하고 LIN과 CAN을 이어준다 | STM32 #3 Gateway + STM32 #4 LIN Slave |
| E | HPC + Camera Vision / 인지·판단 | 카메라 영상을 보고 차선·물체·주차 상황을 판단한다 | Raspberry Pi + Front/Rear Camera |
| F | VCU + DTC + CAN Integration / 최종 판단 | 운전자·ADAS·주차 요청을 합쳐 최종 명령을 만들고 통신/고장 규칙을 관리한다 | STM32 #5 + Pi Diagnostics 협업 |

## 2. 읽는 순서

1. [팀 역할 쉬운 설명](TEAM_ROLE_EASY_GUIDE.md)
2. [전자공학 선행학습](ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md)
3. [Architecture 작성 + Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
4. [Sensor List](SENSOR_LIST.md)
5. [Node별 명세/Architecture 예시](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
6. [4주 개발 계획](WEEKLY_PLAN.md)

## 3. 작성용 Template

- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [Node Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

각 담당자는 개발 시작 시 최소 다음 세 파일을 자기 기능 폴더에 작성한다.

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

## 4. 핵심 시스템 구조

```text
Front Camera ─┐
              ├→ Raspberry Pi Vision/HPC ─┐
Rear Camera ──┘                           │
                                          │ CAN FD
Ultrasonic → STM32 #1 ────────────────────┤
                                          ├→ STM32 #5 VCU → STM32 #2 Drive/Steer
STM32H735 Cockpit ←────────────────────────┤
                                          │
STM32 #3 Body Gateway ←────────────────────┘
        ↕ LIN
STM32 #4 Body LIN Slave
        ├ Ambient Sensor
        └ Lighting
```

### Vision 개발 방식

개발 중에는 Raspberry Pi 두 대를 사용해 병렬 개발할 수 있다.

```text
Pi #1 + Front Camera → Front ADAS Vision
Pi #2 + Rear Camera  → Rear Parking Vision
```

최종 차량에서는 두 서비스를 Raspberry Pi 한 대로 합치는 것을 목표로 한다. Front는 CSI, Rear는 USB Camera 구성을 기본안으로 둔다.

## 5. DTC 역할

DTC는 F 한 명이 모든 고장을 직접 만드는 기능이 아니다.

```text
각 Node → 자기 고장 검출
          ↓
      DTC Event
          ↓ CAN FD
Pi DTC Manager → Active / History / Timestamp / Count
          ↓
STM32H735 → Warning / 상세 DTC 화면
```

F는 DTC Code 규칙, CAN Diagnostic 계약, VCU의 중요 고장 대응을 통합한다.

## 6. Legacy

전체 문서 동기화 전 버전은 아래에 보존한다.

- [`archive/legacy_v1.2_before_full_sync_2026-09-09/`](archive/legacy_v1.2_before_full_sync_2026-09-09/)
- 기존 초기 README: [`archive/README_2026-09-08_legacy.md`](archive/README_2026-09-08_legacy.md)

현재 개발에서는 legacy 문서가 아니라 이 폴더의 최신 문서를 사용한다.

[Main README](../README.md)
