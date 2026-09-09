# Documentation Index

현재 문서는 **Architecture v1.2** 기준이다.

프로젝트를 처음 보는 팀원은 어려운 ECU 이름부터 외우지 말고 아래 한 줄부터 이해한다.

```text
인지 → 판단 → 제어
       +
UI / 통신 / 진단
```

현재 역할은 다음과 같다.

| 담당 | 쉬운 역할 | 주요 Node |
|---|---|---|
| A | 초음파로 주변 거리 보기 | STM32 #1 Ultrasonic Perception |
| B | 운전자에게 정보 보여주기 | STM32H735 Cluster + IVI |
| C | 모터와 조향 실제로 움직이기 | STM32 #2 Drive + Steering |
| D | 조명/조도 + LIN/CAN 연결 | STM32 #3 Gateway + STM32 #4 LIN Slave |
| E | 카메라 영상 보고 상황 판단 | Raspberry Pi Front/Rear Vision |
| F | 최종 차량 판단 + DTC + CAN 통합 | STM32 #5 VCU + Diagnostics |

최종 구조:

```text
Front / Rear Camera
       ↓
Raspberry Pi Vision / HPC
       ↕ CAN FD

Ultrasonic / Drive / VCU / Body Gateway / H735
                       │
                      LIN
                       │
                Body LIN Slave
```

개발 중에는 Front Vision과 Rear Vision을 Raspberry Pi 두 대에서 병렬 개발할 수 있고, 최종 차량에서는 한 Pi로 통합하는 것을 목표로 한다.

---

# 처음 참여한 팀원 읽는 순서

## 1단계 — 내 역할부터 이해

1. **[팀 역할 쉬운 설명](TEAM_ROLE_EASY_GUIDE.md)**

이 문서에서 먼저 확인한다.

```text
나는 무엇을 입력받는가?
나는 무엇을 계산/판단하는가?
나는 어떤 값을 만들어 누구에게 보내는가?
내 기능이 고장나면 어떻게 아는가?
```

## 2단계 — 필요한 기초 공부

2. [전자공학 선행학습](ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md)
3. [Architecture 작성 & Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
4. [Sensor List](SENSOR_LIST.md)

## 3단계 — 내 문서 작성

5. [Node별 명세/Architecture 예시](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
6. [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
7. [ECU / Node Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
8. [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

## 4단계 — 일정과 통합

9. [4주 개발 계획](WEEKLY_PLAN.md)

---

# 팀원이 자기 담당 Node에서 만들어야 하는 문서

각자 최소 다음 3개를 작성한다.

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

### Specification

"내 기능이 무엇을 해야 하는가"를 적는다.

예:

```text
초음파 ECU는 거리값을 mm 단위로 계산해야 한다.
센서 응답이 없으면 Invalid 상태를 만들어야 한다.
```

### Architecture

"그 기능을 어떤 구조로 만들 것인가"를 적는다.

```text
Ultrasonic
→ Timer / GPIO
→ Distance Calculation
→ Validation
→ Warning Level
→ CAN Status
```

### Stage 1 Test Report

"실제로 연결해봤더니 제대로 동작했는가"를 기록한다.

---

# 팀 역할 핵심 경계

- **A Ultrasonic:** 거리와 Warning을 만든다. Motor를 직접 정지시키지 않는다.
- **B H735:** 데이터를 받아 보여준다. 차량 최종 제어를 하지 않는다.
- **C Drive/Steering:** 최종 명령대로 Motor/Servo를 움직인다.
- **D Body:** LIN Master/Slave와 CAN↔LIN Gateway, Lighting을 맡는다.
- **E Vision:** Camera Frame을 Pi에서 처리하고 ADAS/Parking Request를 만든다.
- **F VCU/Diagnostics:** 모든 요청을 보고 최종 안전 판단을 하며 CAN/DTC 규칙을 통합한다.

DTC는 F 혼자 만드는 기능이 아니다. 각 담당자는 자기 Node의 Local Fault Detection을 구현하고 F가 코드/통합 규칙을 관리한다.

---

# Template

- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [ECU / Node Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

---

[Main README](../README.md)


## 이 폴더의 전체 문서

[과거 자료 목록](../README.md) · [현재 문서 안내](../../README.md)

- [BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
- [ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md](ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md)
- [MANIFEST.md](MANIFEST.md)
- [NODE_SPEC_ARCHITECTURE_EXAMPLES.md](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
- [SENSOR_LIST.md](SENSOR_LIST.md)
- [TEAM_ROLE_EASY_GUIDE.md](TEAM_ROLE_EASY_GUIDE.md)
- [WEEKLY_PLAN.md](WEEKLY_PLAN.md)
- [templates/ECU_ARCHITECTURE_TEMPLATE.md](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [templates/NODE_SPECIFICATION_TEMPLATE.md](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [templates/STAGE1_TEST_REPORT_TEMPLATE.md](templates/STAGE1_TEST_REPORT_TEMPLATE.md)
