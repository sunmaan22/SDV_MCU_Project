# ECU별 문서

> **2026-09-17 C 입력 계획 변경:** 기어·조향·속도 요청은 RF로 STM32(C)에 수신한다. E-Stop은 로컬 GPIO/EXTI 차단을 유지한다. RF 모델은 nRF24L01, STM32 연결은 SPI로 확정했다. 모듈 보드/핀/패킷/수치와 CAN 매핑은 OPEN이다. 아래 2026-09-15 기록의 가변저항·로컬 Gear GPIO 설명은 변경 이력이며 현재 입력 구성에 적용하지 않는다.

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [공통 최종 명세](../system/FINAL_IMPLEMENTATION_SPEC.md)

| 담당 | 역할 | 시작 | 기능 명세 | 구조 | 시험 기록 |
|---|---|---|---|---|---|
| A | 초음파 4방향(FL/FR/RL/RR) 충돌 위험 감지 전담 | [안내](Ultrasonic_Perception/README.md) | [명세](Ultrasonic_Perception/SPECIFICATION.md) | [아키텍처](Ultrasonic_Perception/ARCHITECTURE.md) | [테스트](Ultrasonic_Perception/TEST_REPORT.md) |
| B | 화면·사용자 요청(턴시그널/헤드램프 밝기) | [안내](IVI/README.md) | [명세](IVI/SPECIFICATION.md) | [아키텍처](IVI/ARCHITECTURE.md) | [테스트](IVI/TEST_REPORT.md) |
| C | 모터·조향 출력 + RF Driver/Gear 입력 + 로컬 E-Stop | [안내](Motor_Steering_Control/README.md) | [명세](Motor_Steering_Control/SPECIFICATION.md) | [아키텍처](Motor_Steering_Control/ARCHITECTURE.md) | [테스트](Motor_Steering_Control/TEST_REPORT.md) |
| D | 조명(턴시그널/헤드램프/브레이크등)·CAN/LIN Gateway | [안내](Lighting_LIN_CAN/README.md) | [명세](Lighting_LIN_CAN/SPECIFICATION.md) | [아키텍처](Lighting_LIN_CAN/ARCHITECTURE.md) | [테스트](Lighting_LIN_CAN/TEST_REPORT.md) |
| E | 전방 카메라 COCO 객체인식·ADAS 요청 (Rear/주차 Vision 없음) | [안내](HPC_Camera_Vision/README.md) | [명세](HPC_Camera_Vision/SPECIFICATION.md) | [아키텍처](HPC_Camera_Vision/ARCHITECTURE.md) | [테스트](HPC_Camera_Vision/TEST_REPORT.md) |
| F | VCU·최종 명령 중재·DTC | [안내](VCU_DTC_CAN_Integration/README.md) | [명세](VCU_DTC_CAN_Integration/SPECIFICATION.md) | [아키텍처](VCU_DTC_CAN_Integration/ARCHITECTURE.md) | [테스트](VCU_DTC_CAN_Integration/TEST_REPORT.md) |

각 역할은 안내, 기능 명세, 아키텍처, 테스트 기록의 동일한 구조를 사용한다. 테스트 문서의 실제 결과·증빙을 확인해 검증 상태를 판단한다.
