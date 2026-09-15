# 2026-09-15 변경 검토 및 충돌주의 전환

[프로젝트 홈](../../README.md) · [최상위 명세](FINAL_IMPLEMENTATION_SPEC.md)

검토 기준은 main `c99ab5b4087127ec3b15cc691ea75e095d5a225b`다. 오늘의 단일 Screen 결정 이후 범위 변경 커밋 `a788e72`, `633da54`, `c99ab5b`와 현재 문서·펌웨어를 비교했다. 범위 축소와 역할 이전은 최상위 계약에 반영되어 있으나 하위 문서의 메시지 경로와 다이어그램에 불일치가 남아 있었다. 이 변경은 문서 정합성과 기능 계약을 수정하며 전체 펌웨어 구현 완료를 뜻하지 않는다.

## 확인 및 수정

| 항목 | 발견 사항 | 처리 |
|---|---|---|
| 조명 명령 owner | Lighting 명세/구조도/모델 표와 Overview가 B와 F 모두 Body_Command를 발행하는 것으로 기재 | B → Body_User_Request → F → Body_Command → D로 통일, 회귀 시험 추가 |
| ADAS 수신 대상 | B 문서 및 공통 표에 ADAS_Request 수신이 남음 | 최상위 §1.1대로 Vision_Status는 F/B, ADAS_Request는 F로 통일 |
| 깨진 Mermaid | C ARCHITECTURE의 RPM(est) edge label, F SPECIFICATION의 Driver_Input(...) node label에서 parse error 재현 | 라벨 quoting 수정, HTML/리터럴 줄바꿈 라벨 정리 |
| 안전 입력 이전 | C 구조도에 Gear/E-Stop 입력 경로 누락 | Gear 입력과 로컬 출력 차단 경로 추가. F의 “GPIO 전혀 없음”을 해당 물리 입력 미소유로 정확히 표현 |
| 안전 우선순위 | Overview의 “초음파 항상 최우선”이 E-Stop/Critical Fault와 충돌 | E-Stop/Critical Fault 다음, ADAS/Driver보다 우선으로 명시 |
| 엔코더 제거 후 정지 표현 | 출력 차단과 실제 정지가 혼용됨 | 추정 speed=0은 실제 정지 증거가 아님을 명시, D↔R/감속 판정의 OPEN 결정 보완 |
| 결정 상태 | OPEN/FROZEN만 허용한다고 쓰면서 REMOVED 행 사용, 센서 개수 OPEN과 4개 FROZEN 혼재 | REMOVED 의미 명시, 센서 모델 OPEN과 개수 4개 FROZEN 분리 |

## 사용자 결정: Parking → 충돌주의

- 기능명은 **충돌주의(Collision Warning)** 이며 FL/FR/RL/RR 4방향 거리 기반이다. 주차 공간 탐색·자동 주차 경로·주차 조향은 포함하지 않는다.
- 모든 기어에서 충돌주의 패널 접근을 제공하며 Gear R 전용 자동 화면 전환을 요구하지 않는다. 기어 P 자체는 유지한다.
- 사용자가 선택한 **기존 안전 개입 유지**를 반영한다. CRITICAL 시 VCU 안전 개입이 ADAS/Driver보다 우선하며 E-Stop/Critical Fault가 최우선이다.
- 패널 닫기는 경고 해제나 제어 해제 명령이 아니다. invalid/stale 센서를 SAFE로 표시하지 않는다.
- `Ultrasonic_Status`의 기존 publisher/consumer 및 zone 구조는 유지한다. 거리·시간 수치, 제어 대상 zone, 감속/정지/복구 조건은 계속 OPEN이다. 4방향 표시와 방향별 제동 판단은 구분한다.
- 공통/역할 명세, 아키텍처, README, 일정, 미실행 시험 계획을 함께 갱신했다. 과거 revision 행과 archive의 Parking 표기는 당시 기록으로 남긴다.

## 실제 구현 상태 및 남은 결정

| 항목 | 현재 근거 / 남은 작업 |
|---|---|
| 기본 계기판 | `vehicle_data.h`, `freertos.c`, `Screen1View.cpp`에 speed/RPM, gear/READY, 일반 warning, Dummy provider와 내부 CAN loopback이 존재 |
| 충돌주의/ADAS/DTC 패널 | 4방향 거리 모델·실제 CAN 디코더·상세 패널·Active DTC 동기화 전체 구현은 아직 없음. 기존 기본 표시 PASS를 이 기능들의 PASS로 확대하지 않음 |
| 추정값 UI 표시 | 현재 View는 km/h/rpm 및 DEMO를 표시한다. 실제 Drive 연동 시 estimated 표시를 구현해야 함. DEMO와 estimated는 서로 다른 의미 |
| 제어/통신 수치 | Gate A~D 미완료. CAN payload/주기/timeout, 방향별 위험 zone, 정지 판정, 복구 기준은 Owner 결정과 bench 근거 필요 |
| 조명 상태 피드백 | 헤드램프 실제 밝기를 표시하려면 현재 `lamp_status` bitfield의 표현/스케일을 Gate B에서 완결해야 함 |
| 명칭 | 검토 시점 main의 문서/펌웨어 경로는 아직 `IVI`, `SDV_IVI_H735`다. Cluster 일괄 이름 변경 완료 상태는 아님 |

## 검증

- 현재 문서 및 템플릿 Mermaid 41개를 Mermaid 11.12.0 + headless Chromium으로 렌더링했다. 수정 전 2개 parse error, 수정 후 41개 모두 성공했다. GitHub 서버의 실제 렌더러 버전/표시를 직접 검증한 것은 아니다.
- 현재 문서의 상대 파일 링크를 기준 커밋의 Git tree와 대조했다. 누락된 파일 경로는 없었다.
- 검토에 사용한 펌웨어 340개 파일의 Git blob hash가 기준 커밋과 일치했다. 이 PR은 펌웨어 파일을 변경하지 않는다.
- H735 TEST_REPORT의 기존 §0 bring-up/실기 기록을 원문과 비교해 동일함을 확인했다.
- A/B/F의 충돌주의 및 D의 조명 경로 회귀 시험을 추가했으며 모두 **NOT RUN**이다. 보드 빌드·외부 CAN 통합·실기 제어 시험은 이 검토에서 수행하지 않았다.
