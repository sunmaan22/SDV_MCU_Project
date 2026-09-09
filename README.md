# SDV MCU Project

저속 RC 모형 기반의 6인 Mini SDV E/E 아키텍처 프로젝트다. 운전자 입력을 중심으로 ADAS와 주차 보조 기능을 구성하며, ECU 간 역할·통신·진단을 함께 설계한다.

**[전체 문서 보기](docs/README.md)** · **[최종 구현 명세](docs/system/FINAL_IMPLEMENTATION_SPEC.md)** · **[프로젝트 상세 설명](docs/overview/PROJECT_OVERVIEW.md)**

> 현재 문서는 설계·구현 기준을 정리한 자료다. `OWNER INPUT` / `OPEN` 항목은 Owner 확정이 필요하며, 문서나 테스트 양식이 있다는 것만으로 구현·검증 완료를 의미하지 않는다.

## 무엇을 찾고 있나요?

| 목적 | 바로가기 |
|---|---|
| 처음 참여해 프로젝트와 역할 파악하기 | [팀 시작 가이드](docs/getting_started/TEAM_GUIDE.md) |
| 전체 구성·데이터 흐름·데모 범위 이해하기 | [프로젝트 상세 설명](docs/overview/PROJECT_OVERVIEW.md) |
| 공통 인터페이스·Owner 결정 항목 확인하기 | [최종 구현 명세](docs/system/FINAL_IMPLEMENTATION_SPEC.md) |
| 설계 배경·참고 내용 찾기 | [공통 참고 문서](docs/system/PROJECT_REFERENCE.md) |
| 주차별 할 일 확인하기 | [주간 계획](docs/getting_started/WEEKLY_PLAN.md) |
| 명세·아키텍처·테스트 문서 작성하기 | [문서 템플릿](docs/templates/README.md) |
| 문서 우선순위·Freeze 규칙 확인하기 | [문서 운영 규칙](docs/system/DOCUMENTATION_POLICY.md) |
| 이전 버전 찾아보기 | [과거 자료 목록](docs/archive/README.md) |

## 내 담당 문서로 바로 이동

| 담당 | 역할 | 시작 | 기능 명세 | 구조 | 시험 기록 |
|---|---|---|---|---|---|
| A | 초음파·주차 거리 감지 | [안내](docs/ecus/Ultrasonic_Perception/README.md) | [명세](docs/ecus/Ultrasonic_Perception/SPECIFICATION.md) | [아키텍처](docs/ecus/Ultrasonic_Perception/ARCHITECTURE.md) | [테스트](docs/ecus/Ultrasonic_Perception/TEST_REPORT.md) |
| B | 화면·사용자 요청 | [안내](docs/ecus/IVI/README.md) | [명세](docs/ecus/IVI/SPECIFICATION.md) | [아키텍처](docs/ecus/IVI/ARCHITECTURE.md) | [테스트](docs/ecus/IVI/TEST_REPORT.md) |
| C | 모터·조향 출력 | [안내](docs/ecus/Motor_Steering_Control/README.md) | [명세](docs/ecus/Motor_Steering_Control/SPECIFICATION.md) | [아키텍처](docs/ecus/Motor_Steering_Control/ARCHITECTURE.md) | [테스트](docs/ecus/Motor_Steering_Control/TEST_REPORT.md) |
| D | 조명·CAN/LIN Gateway | [안내](docs/ecus/Lighting_Ambient_LIN_CAN/README.md) | [명세](docs/ecus/Lighting_Ambient_LIN_CAN/SPECIFICATION.md) | [아키텍처](docs/ecus/Lighting_Ambient_LIN_CAN/ARCHITECTURE.md) | [테스트](docs/ecus/Lighting_Ambient_LIN_CAN/TEST_REPORT.md) |
| E | 카메라·ADAS 요청 | [안내](docs/ecus/HPC_Camera_Vision/README.md) | [명세](docs/ecus/HPC_Camera_Vision/SPECIFICATION.md) | [아키텍처](docs/ecus/HPC_Camera_Vision/ARCHITECTURE.md) | [테스트](docs/ecus/HPC_Camera_Vision/TEST_REPORT.md) |
| F | VCU·최종 명령 중재·DTC | [안내](docs/ecus/VCU_DTC_CAN_Integration/README.md) | [명세](docs/ecus/VCU_DTC_CAN_Integration/SPECIFICATION.md) | [아키텍처](docs/ecus/VCU_DTC_CAN_Integration/ARCHITECTURE.md) | [테스트](docs/ecus/VCU_DTC_CAN_Integration/TEST_REPORT.md) |

기어·후진 요청과 최종 명령 중재는 **F**, 실제 구동·조향 출력은 **C**, 카메라 기반 ADAS 요청은 **E**, 초음파 주차 감지는 **A** 문서에서 확인한다. ECU 간 공통 계약은 최종 구현 명세를 우선한다.

## 문서 폴더

| 폴더 | 내용 |
|---|---|
| [getting_started](docs/getting_started/README.md) | 팀 참여·역할·주간 계획 |
| [overview](docs/overview/README.md) | 프로젝트 전체 설명 |
| [system](docs/system/README.md) | 공통 명세·참고·문서 운영 규칙 |
| [ecus](docs/ecus/README.md) | A–F 담당별 안내·명세·아키텍처·테스트 |
| [templates](docs/templates/README.md) | 새 문서 작성 양식 |
| [archive](docs/archive/README.md) | 현재 구현 기준과 구분한 과거 자료 |

문서가 충돌하면 **최종 구현 명세 → 역할별 명세 → 역할별 아키텍처 → README·예시 → 과거 자료** 순서로 확인한다.

[라이선스](LICENSE)
