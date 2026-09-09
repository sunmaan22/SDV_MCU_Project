# Documentation Guide

> 기준: **Architecture v1.2 / 2026-09-09**  
> 목적: 문서를 적게 유지하면서도, 팀원이 `무엇을 만들지`와 `어떻게 만들지`를 헷갈리지 않게 한다.

## 1. 현재 활성 문서

```text
docs/
├ README.md                  # 문서 인덱스와 작성 기준
├ TEAM_GUIDE.md              # 초심자용 역할/전자기초/개발순서
├ PROJECT_REFERENCE.md       # 현재 HW, 센서, 데이터 흐름, 역할별 예시
├ WEEKLY_PLAN.md             # 4주 일정
└ templates/
   ├ FUNCTIONAL_SPECIFICATION_TEMPLATE.md
   ├ SOFTWARE_ARCHITECTURE_TEMPLATE.md
   └ TEST_REPORT_TEMPLATE.md
```

과거 세부 가이드와 이전 템플릿은 `docs/archive/`에 보존한다. 현재 개발에서는 위 7개 파일만 기준으로 사용한다.

---

## 2. 현재 6인 역할

| 담당 | 역할 | 한 줄 설명 |
|---|---|---|
| A | Ultrasonic / 인지 | 초음파로 장애물까지 거리를 측정한다. |
| B | Cluster + IVI / UI | H735에서 차량 상태, 경고, ADAS, Parking, DTC를 보여준다. |
| C | Motor + Steering / 제어 | DC Motor와 RC Servo를 실제로 움직인다. |
| D | Lighting + Ambient / LIN-CAN | 조도/조명을 제어하고 LIN과 CAN FD를 연결한다. |
| E | HPC + Camera Vision / 인지·판단 | Front/Rear Camera 영상을 처리하고 ADAS/Parking 판단 결과를 만든다. |
| F | VCU + DTC + CAN Integration / 최종 판단 | 운전자·ADAS·Parking 요청을 중재하고 CAN/DTC 규칙을 통합한다. |

전체 흐름은 다음 한 줄로 본다.

```text
인지 → 판단 → 제어
      +
UI / 통신 / 진단
```

---

## 3. 팀원이 작성해야 하는 문서

각 담당자는 자기 기능 폴더에 아래 세 파일만 만든다.

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

- `SPECIFICATION.md`: **무엇을 해야 하는가**
- `ARCHITECTURE.md`: **그 기능을 어떤 구조로 구현하는가**
- `TEST_REPORT.md`: **실제로 요구사항을 만족했는지 어떻게 확인했는가**

문서 수를 늘리기보다 이 세 문서 안에서 링크와 표를 사용한다.

---

# 4. 기능 명세서 작성 기준 검토

기능 명세서는 EdrawSoft의 기능 명세서 가이드에서 제시하는 실무 항목을 모두 포함하도록 템플릿을 정리했다.

참고: https://www.edrawsoft.com/kr/diagram-tutorial/functional-specification-guide.html

가이드의 핵심 항목과 현재 템플릿 대응은 다음과 같다.

| 가이드 항목 | 현재 템플릿 |
|---|---|
| 기능 ID / 명칭 | `Feature ID / Name` |
| 사용자 시나리오 | `Usage / System Scenario` |
| 기능 상세 설명 | `Functional Flow`, `Functional Requirements` |
| 입력값 / 출력값 | `Inputs`, `Outputs` |
| 조건 / 예외 처리 | `Rules / Preconditions`, `Exceptions / Edge Cases` |
| UI/UX 링크 또는 Screenshot | `UI / UX Reference` |
| 우선순위 | Requirement별 `Priority` |
| 기능 흐름 | Mermaid / Flowchart 영역 |
| 변경 이력 | `Revision History` |

MCU처럼 사람이 직접 조작하지 않는 Node는 `사용자 시나리오`를 **System Scenario**로 작성하고, UI가 없는 Node는 UI/UX 항목을 `N/A`로 적는다.

추가로 본 프로젝트에서는 임베디드 특성 때문에 다음도 포함한다.

- Hardware / Network Interface
- Timing / Timeout
- Safety / Fail-safe
- DTC / Fault handling
- Verification / Acceptance criteria

---

# 5. Software Architecture 작성 기준 검토

Software Architecture 문서는 단순 Block Diagram 한 장으로 끝내지 않는다. SEI의 **Views and Beyond** 방식과 실무형 arc42 구조를 참고해, 프로젝트 규모에 맞게 필요한 항목만 남겼다.

참고:

- SEI Views and Beyond: https://www.sei.cmu.edu/library/views-and-beyond-the-sei-approach-for-architecture-documentation/
- arc42 overview: https://arc42.org/overview/

현재 `SOFTWARE_ARCHITECTURE_TEMPLATE.md`는 다음 내용을 포함한다.

| 설계 관점 | 템플릿 대응 |
|---|---|
| 목적 / Scope / Stakeholder | Introduction & Goals |
| 품질 목표 | Quality Goals |
| 제약조건 | Constraints |
| 외부 시스템과 경계 | Context & Scope |
| 핵심 설계 방향과 이유 | Solution Strategy & Rationale |
| Module / Component 구조 | Building Block View |
| 실제 동작 순서 | Runtime View |
| MCU/Pi/센서/네트워크 배치 | Deployment / Hardware View |
| Interface Contract | CAN/LIN/API/Peripheral Interfaces |
| Data / State | Data & State Model |
| 공통 정책 | Cross-cutting Concepts |
| 중요한 설계 선택 | Architecture Decisions |
| Fault / DTC / Timing | Reliability & Diagnostics |
| 품질 검증 | Quality Scenarios & Verification |
| 위험 / 미해결 문제 | Risks & Technical Debt |
| 요구사항 연결 | Traceability |
| 용어 | Glossary |

SEI가 강조하는 **여러 View와 View를 가로지르는 정보**, arc42가 강조하는 **Context, Building Block, Runtime, Deployment, Decisions, Quality, Risks**를 이 프로젝트 규모에 맞춰 하나의 Template에 합쳤다.

---

## 6. 문서 작성 규칙

1. 같은 내용을 여러 파일에 복붙하지 않는다. 상세 내용은 한 곳에 적고 링크한다.
2. 요구사항은 `~해야 한다` 형태로 쓰고 ID를 붙인다.
3. Architecture에는 그림만 두지 말고 각 Component 역할, Interface, Runtime Flow를 설명한다.
4. CAN/LIN Signal의 Owner는 하나만 둔다.
5. 값이 미정이면 임의 숫자를 확정값처럼 적지 말고 `TBD`로 둔다.
6. 변경 시 `Revision History`를 갱신한다.
7. 설계가 바뀐 이유가 중요하면 Architecture Decision에 남긴다.

---

## 7. Legacy

- `archive/README_2026-09-08_legacy.md`
- `archive/legacy_v1.2_before_full_sync_2026-09-09/`
- `archive/legacy_v1.2_before_doc_cleanup_2026-09-09/`

현재 기준 문서와 과거 문서를 섞어서 사용하지 않는다.

[Main README](../README.md)
