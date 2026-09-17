# SDV MCU Project

> **2026-09-17 C 입력 계획 변경:** 기어·조향·속도 요청은 RF로 STM32(C)에 수신한다. E-Stop은 로컬 GPIO/EXTI 차단을 유지한다. RF 모델은 nRF24L01, STM32 연결은 SPI로 확정했다. 모듈 보드/핀/패킷/수치와 CAN 매핑은 OPEN이다. 아래 2026-09-15 기록의 가변저항·로컬 Gear GPIO 설명은 변경 이력이며 현재 입력 구성에 적용하지 않는다.

저속 RC 모형 기반의 6인 Mini SDV E/E 아키텍처 프로젝트다. 운전자 입력(RF)을 중심으로 전방 카메라 객체인식(ADAS) + 초음파 4방향 충돌주의 기능을 구성하며, ECU 간 역할·통신·진단을 함께 설계한다.

> **2026-09-15 범위 변경:** 후방 카메라/Rear Vision/주차 Vision과 Ambient 조도 센서 기능을 삭제했다. 충돌주의는 초음파 4방향(전좌/전우/후좌/후우) 전용, 전방 카메라는 COCO 기반 객체인식(class+방향)만 담당한다. 변경 전 문서는 [과거 자료](docs/archive/legacy_v1.3_before_front_only_scope_2026-09-15/README.md) 참고.

**[전체 문서 보기](docs/README.md)** · **[최종 구현 명세](docs/system/FINAL_IMPLEMENTATION_SPEC.md)** · **[프로젝트 상세 설명](docs/overview/PROJECT_OVERVIEW.md)**

> 현재 문서는 설계·구현 기준을 정리한 자료다. `OWNER INPUT` / `OPEN` 항목은 Owner 확정이 필요하며, 문서나 테스트 양식이 있다는 것만으로 구현·검증 완료를 의미하지 않는다.

> **충돌주의 전환:** 모든 기어에서 4방향 거리/경고를 확인하며, CRITICAL 시 기존 VCU 안전 개입은 유지한다. 주차 공간 탐색·자동 주차는 포함하지 않는다. [변경 검토 결과](docs/system/CHANGE_REVIEW_2026-09-15.md)를 참고한다.

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
| A | 초음파 4방향(FL/FR/RL/RR) 충돌 위험 감지 전담 | [안내](docs/ecus/Ultrasonic_Perception/README.md) | [명세](docs/ecus/Ultrasonic_Perception/SPECIFICATION.md) | [아키텍처](docs/ecus/Ultrasonic_Perception/ARCHITECTURE.md) | [테스트](docs/ecus/Ultrasonic_Perception/TEST_REPORT.md) |
| B | 화면·사용자 요청(턴시그널/헤드램프 밝기) | [안내](docs/ecus/IVI/README.md) | [명세](docs/ecus/IVI/SPECIFICATION.md) | [아키텍처](docs/ecus/IVI/ARCHITECTURE.md) | [테스트](docs/ecus/IVI/TEST_REPORT.md) |
| C | 모터·조향 출력 + RF Driver/Gear 입력 + 로컬 E-Stop | [안내](docs/ecus/Motor_Steering_Control/README.md) | [명세](docs/ecus/Motor_Steering_Control/SPECIFICATION.md) | [아키텍처](docs/ecus/Motor_Steering_Control/ARCHITECTURE.md) | [테스트](docs/ecus/Motor_Steering_Control/TEST_REPORT.md) |
| D | 조명(턴시그널/헤드램프/브레이크등)·CAN/LIN Gateway | [안내](docs/ecus/Lighting_LIN_CAN/README.md) | [명세](docs/ecus/Lighting_LIN_CAN/SPECIFICATION.md) | [아키텍처](docs/ecus/Lighting_LIN_CAN/ARCHITECTURE.md) | [테스트](docs/ecus/Lighting_LIN_CAN/TEST_REPORT.md) |
| E | 전방 카메라 COCO 객체인식·ADAS 요청 (Rear/주차 Vision 없음) | [안내](docs/ecus/HPC_Camera_Vision/README.md) | [명세](docs/ecus/HPC_Camera_Vision/SPECIFICATION.md) | [아키텍처](docs/ecus/HPC_Camera_Vision/ARCHITECTURE.md) | [테스트](docs/ecus/HPC_Camera_Vision/TEST_REPORT.md) |
| F | VCU·최종 명령 중재·DTC | [안내](docs/ecus/VCU_DTC_CAN_Integration/README.md) | [명세](docs/ecus/VCU_DTC_CAN_Integration/SPECIFICATION.md) | [아키텍처](docs/ecus/VCU_DTC_CAN_Integration/ARCHITECTURE.md) | [테스트](docs/ecus/VCU_DTC_CAN_Integration/TEST_REPORT.md) |

RF 기어·조향·속도 요청과 로컬 E-Stop 입력 및 실제 구동·조향 출력은 **C**, CAN으로 받은 요청의 최종 중재는 **F**, 카메라 기반 전방 객체 회피 요청은 **E**, 초음파 4방향 충돌 위험 감지는 **A** 문서에서 확인한다. 초음파 충돌 위험도 판단은 **A만** 담당하며 E는 관여하지 않는다. ECU 간 공통 계약은 최종 구현 명세를 우선한다.

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
