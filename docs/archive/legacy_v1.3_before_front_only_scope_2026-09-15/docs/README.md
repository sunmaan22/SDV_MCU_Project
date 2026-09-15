# 문서 안내

[프로젝트 홈](../README.md)

원하는 주제를 아래에서 선택하거나, 담당 A–F의 명세·구조·테스트로 바로 이동한다.

## 처음 읽는 순서

1. [팀 시작 가이드](getting_started/TEAM_GUIDE.md)에서 역할과 진행 방식을 확인한다.
2. [프로젝트 상세 설명](overview/PROJECT_OVERVIEW.md)에서 전체 구성을 파악한다.
3. [최종 구현 명세](system/FINAL_IMPLEMENTATION_SPEC.md)에서 공통 계약과 미확정 항목을 확인한다.
4. 아래 담당 문서에서 **안내 → 기능 명세 → 아키텍처 → 테스트** 순서로 읽는다.

## 주제별 문서

| 분류 | 문서 |
|---|---|
| 시작·일정 | [팀 가이드](getting_started/TEAM_GUIDE.md) · [주간 계획](getting_started/WEEKLY_PLAN.md) |
| 전체 프로젝트 | [상세 설명](overview/PROJECT_OVERVIEW.md) |
| 공통 시스템 | [최종 구현 명세](system/FINAL_IMPLEMENTATION_SPEC.md) · [참고 문서](system/PROJECT_REFERENCE.md) · [문서 운영 규칙](system/DOCUMENTATION_POLICY.md) |
| 새 문서 작성 | [기능 명세 양식](templates/FUNCTIONAL_SPECIFICATION_TEMPLATE.md) · [아키텍처 양식](templates/SOFTWARE_ARCHITECTURE_TEMPLATE.md) · [테스트 양식](templates/TEST_REPORT_TEMPLATE.md) |
| 이전 자료 | [과거 자료 전체 목록](archive/README.md) |

## 담당별 바로가기

| 담당 | 역할 | 시작 | 기능 명세 | 구조 | 시험 기록 |
|---|---|---|---|---|---|
| A | 초음파·주차 거리 감지 | [안내](ecus/Ultrasonic_Perception/README.md) | [명세](ecus/Ultrasonic_Perception/SPECIFICATION.md) | [아키텍처](ecus/Ultrasonic_Perception/ARCHITECTURE.md) | [테스트](ecus/Ultrasonic_Perception/TEST_REPORT.md) |
| B | 화면·사용자 요청 | [안내](ecus/IVI/README.md) | [명세](ecus/IVI/SPECIFICATION.md) | [아키텍처](ecus/IVI/ARCHITECTURE.md) | [테스트](ecus/IVI/TEST_REPORT.md) |
| C | 모터·조향 출력 | [안내](ecus/Motor_Steering_Control/README.md) | [명세](ecus/Motor_Steering_Control/SPECIFICATION.md) | [아키텍처](ecus/Motor_Steering_Control/ARCHITECTURE.md) | [테스트](ecus/Motor_Steering_Control/TEST_REPORT.md) |
| D | 조명·CAN/LIN Gateway | [안내](ecus/Lighting_Ambient_LIN_CAN/README.md) | [명세](ecus/Lighting_Ambient_LIN_CAN/SPECIFICATION.md) | [아키텍처](ecus/Lighting_Ambient_LIN_CAN/ARCHITECTURE.md) | [테스트](ecus/Lighting_Ambient_LIN_CAN/TEST_REPORT.md) |
| E | 카메라·ADAS 요청 | [안내](ecus/HPC_Camera_Vision/README.md) | [명세](ecus/HPC_Camera_Vision/SPECIFICATION.md) | [아키텍처](ecus/HPC_Camera_Vision/ARCHITECTURE.md) | [테스트](ecus/HPC_Camera_Vision/TEST_REPORT.md) |
| F | VCU·최종 명령 중재·DTC | [안내](ecus/VCU_DTC_CAN_Integration/README.md) | [명세](ecus/VCU_DTC_CAN_Integration/SPECIFICATION.md) | [아키텍처](ecus/VCU_DTC_CAN_Integration/ARCHITECTURE.md) | [테스트](ecus/VCU_DTC_CAN_Integration/TEST_REPORT.md) |

## 찾는 기능이 있다면

| 찾는 내용 | 우선 볼 문서 |
|---|---|
| 기어·후진 요청, 차량 상태, 최종 명령 중재 | [VCU 기능 명세](ecus/VCU_DTC_CAN_Integration/SPECIFICATION.md) |
| 모터·조향 출력, Drive 상태 | [Drive 기능 명세](ecus/Motor_Steering_Control/SPECIFICATION.md) |
| 초음파 거리·주차 경고 | [초음파 기능 명세](ecus/Ultrasonic_Perception/SPECIFICATION.md) |
| 카메라 인식·ADAS 요청 | [Vision 기능 명세](ecus/HPC_Camera_Vision/SPECIFICATION.md) |
| 화면·사용자 입력·상태 표시 | [IVI 기능 명세](ecus/IVI/SPECIFICATION.md) |
| 조명·LIN·CAN Gateway | [Body 기능 명세](ecus/Lighting_Ambient_LIN_CAN/SPECIFICATION.md) |
| 통신 계약·DTC 역할·공통 실행 기준 | [최종 구현 명세](system/FINAL_IMPLEMENTATION_SPEC.md) |

## 문서 기준

**최종 구현 명세 → 역할별 명세 → 역할별 아키텍처 → README·예시 → 과거 자료** 순서로 적용한다. `OWNER INPUT` / `OPEN` 값을 임의로 확정하지 않는다. 자세한 기준은 [문서 운영 규칙](system/DOCUMENTATION_POLICY.md)을 따른다.

폴더별 목록: [시작](getting_started/README.md) · [전체 설명](overview/README.md) · [공통 시스템](system/README.md) · [ECU](ecus/README.md) · [템플릿](templates/README.md) · [과거 자료](archive/README.md)
