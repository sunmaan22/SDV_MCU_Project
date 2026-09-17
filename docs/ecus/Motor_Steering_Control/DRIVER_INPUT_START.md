# C: Driver_Input RF 수신 시작 가이드

[C 담당 안내](README.md) · [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md)

## 이번에 만드는 것

RF 송신기에서 보낸 **기어·조향·속도 요청**을 RF 수신기를 통해 STM32에서 읽고 확인한다. `Driver_Input` 프로젝트는 C ECU의 입력부를 먼저 검증하는 프로젝트이며, 별도 ECU를 추가한다는 의미는 아니다.

```text
조종기 → RF 무선 구간 → RF 수신기 → STM32 Driver_Input
                                    ↓
                     raw 값 + 유효성 + 수신 시각 확인
                                    ↓ (후속 통합)
                   Driver_Input(CAN) → F의 최종 중재
                                    ↓
                     Final_Drive_Command → C의 출력
```

여기서 속도는 우선 **목표 속도/스로틀 요청이라는 작업 가정**이다. 실측 속도를 받으려는 경우에는 송신측 센서와 단위/유효성 계약부터 추가해야 한다. 요청값을 `Drive_Status.vehicle_speed`에 복사하지 않는다. 기존 차량 표시 speed/RPM은 적용한 모터 명령 기반 추정값이며 실측이 아니다.

## 확인한 로컬 프로젝트 상태

2026-09-17 파일 확인 기준이며 빌드·다운로드·실기 동작은 아직 시험하지 않았다.

| 항목 | 확인 내용 |
|---|---|
| 경로 | `C:\Users\User\STM32CubeIDE\workspace_1.19.0\Driver_Input` |
| 설정 파일 | `Driver_Input.ioc` |
| 설정된 보드 / MCU | `NUCLEO-G431KB` / `STM32G431KBT6` (실물 보드 일치는 별도 확인) |
| 현재 코드 | Cube 생성 기본 HAL/BSP 초기화, LED, COM1 샘플 |
| 디버그 출력 | COM1 115200, 8 data bits, no parity, 1 stop bit 설정; 시작 메시지 있음 |
| 현재 예약 | PA2/PA3 USART2 TX/RX, PA13/PA14 SWD, PB3 SWO, PB8 GPIO |
| 미구현 | RF 수신/해석/timeout, FDCAN, FreeRTOS 입력 처리 |

USART2는 디버그용으로 유지하고 RF는 SPI로 연결한다. 최종 CAN/모터/서보 핀까지 확인한 뒤 SPI와 CE/CSN/IRQ 핀을 배정한다. 현재 로컬 펌웨어와 `.ioc`는 이번 문서 수정으로 변경하지 않았다.

## 처음 할 일

1. **현재 프로젝트를 먼저 Build하고 보드에 Download한다.** ST-LINK 디버그에서 `main()` 도달을 확인한다. COM1 출력 경로가 연결되어 있으면 시리얼 터미널을 115200 / 8N1로 열고 Reset 후 `Welcome to STM32 world !`가 보이는지 확인한다. 보이지 않으면 COM 포트와 BSP printf 경로를 먼저 점검한다.
2. **nRF24L01 모듈의 전원과 SPI 배선을 확인한다.** 모델은 사용자 확인 완료다. 모듈 보드의 전원 사양을 확인하고 SCK/MOSI/MISO, CE/CSN, 필요 시 IRQ를 배정한다. 송신측 MCU와 nRF24L01도 준비한다.
3. **기어·조향·속도 요청이 어느 채널/필드인지 정한다.** 기어 스위치 위치 수, 조향 좌/중립/우, 속도 최소/최대/중립과 브레이크 표현을 기록한다. 3위치 스위치를 P/R/N/D 네 상태로 임의 대응시키지 않는다.
4. **SPI 레지스터와 시험 패킷부터 읽는다.** SPI 설정 후 레지스터 쓰기/읽기가 일치하는지 확인하고, 송신측 고정 패킷 하나를 수신한다. 모터/서보 출력은 비활성 상태로 둔다.
5. **세 요청과 유효성을 함께 확인한다.** 디버거 또는 제한된 주기의 로그로 `gear_raw`, `steering_raw`, `speed_raw`, `valid`, `age_ms`를 본다. 송신기 OFF/수신기 분리/재연결 시험까지 완료한 다음 CAN과 RTOS 통합을 진행한다.

현재 `while (1)`의 LED 토글에는 지연이 없어 육안 점멸 확인에 적합하지 않다. 최초 보드 단독 LED 시험에서만 `USER CODE BEGIN 3` 안에 `HAL_Delay(500);`을 넣어 확인할 수 있다. RF 수신 루프나 RTOS 제어 경로에는 이 지연을 남기지 않는다. 샘플 코드를 바꾸고 재생성할 경우 USER CODE 밖 변경이 덮어써지는지도 확인한다.

## nRF24L01 연결과 남은 결정

### nRF24L01 / SPI 확정 (2026-09-17)

사용자가 **nRF24L01**로 확인했다. STM32는 **SPI로 패킷을 수신**한다. 송신측 MCU/펌웨어가 기어·조향·속도 요청을 합의한 바이트 패킷으로 만들어 보내야 한다.

```text
송신측 입력 → 송신측 MCU → SPI → nRF24L01
                                    ~ RF ~
STM32 Driver_Input ← SPI ← nRF24L01
```

Nordic의 nRF24L01 규격에는 SPI 제어, CE/CSN 신호, 최대 32바이트 payload, 칩 공급전압 1.9~3.6 V가 명시되어 있다. 기본 모듈은 3.3 V 전원을 전제로 검토하고 VCC에 5 V를 직접 연결하지 않는다. 어댑터/PA·LNA 모듈은 보드별 전원 사양과 전류 요구량을 별도로 확인한다. 근거: [Nordic nRF24L01 Product Specification v2.0, §2/4/7/8](https://devzone.nordicsemi.com/cfs-file/__key/communityserver-discussions-components-files/4/content.pdf). nRF24L01+ 전용 기능을 전제하지 않고 nRF24L01 규격을 따른다.

| 연결 신호 | STM32 측 역할 | 현재 배정 |
|---|---|---|
| SCK / MOSI / MISO | SPI master | 핀/인스턴스 TBD |
| CSN | GPIO output, SPI chip select | 핀 TBD |
| CE | GPIO output, radio mode 제어 | 핀 TBD |
| IRQ | 상태 알림 입력, 사용 시 EXTI | 처음에는 status polling 가능; 핀 TBD |
| VCC / GND | 모듈 사양에 맞는 안정된 전원 / 공통 접지 | 모듈 실물 확인 |

첫 RF 작업은 **SPI 레지스터 읽기 → 설정 레지스터 쓰기/읽기 일치 → 송신측 고정 시험 패킷 수신** 순서다. 그다음 시험 패킷을 `version`, `sequence`, `gear`, `steering_request`, `speed_request`, `input_valid` 같은 최소 논리 항목으로 바꾼다. 이는 패킷 초안이며 byte 수·단위·signedness·endianness는 송수신측이 함께 정한다. C 구조체 메모리를 그대로 송신하지 말고 바이트 배치를 명시한다. RF 패킷은 CAN 메시지와 별도 계약이다.

송수신측 RF 주소/채널/data rate/CRC/고정 payload 길이/ACK 설정을 일치시킨다. 초기에는 고정 길이 패킷 하나로 시작한다. SPI 읽기 성공은 RF 통신 성공이 아니므로 송신측 MCU와 호환 모듈도 준비해야 한다. sequence 중복/역순/재시작 처리와 수신 timeout은 bench 값으로 분리해 시험한다. 같은 조작값이라도 새로운 패킷은 유효할 수 있지만 반복된 오래된 패킷으로 freshness를 갱신하지 않는다.

모델/SPI 선택은 DEC-HW-024에서 확정했다. 아래 미정 항목은 DEC-HW-029와 제어/CAN 계약에서 결정한다.

| 항목 | 결정 / 측정값 |
|---|---|
| RF IC / 실장 모듈 | nRF24L01 확정 / 모듈 제조사·보드 사양 TBD |
| STM32 연결 방식 | SPI + CE/CSN GPIO, IRQ 선택 |
| 전원 / 신호 전압 / GND / 필요 레벨 변환 | TBD |
| 채널/필드: 기어 / 조향 / 속도 | TBD |
| 브레이크 | TBD (별도 채널/통합 스틱/미제공) |
| raw 범위 / 중립 / 방향 / deadband | TBD, 실제 측정 |
| 주기 / RF timeout / 복구 조건 | TBD, bench 값은 BENCH ONLY / NOT FROZEN |
| RF loss / failsafe 식별 방법 | TBD, 송신기 OFF에서 반드시 확인 |
| SPI 인스턴스 / SCK·MOSI·MISO / CE·CSN·IRQ 핀 | TBD, 다른 기능과 충돌 검사 |

SPI 클럭과 radio data rate는 서로 다르다. 핀·SPI 클럭·RF 설정은 모듈 사양과 송신측 설정을 확인한 후 bench 값으로 기록한다.

## 최소 구현 순서와 완료 기준

처음에는 생성된 `main.c`의 USER CODE 구간에서 raw 값 하나를 읽는다. 수신 처리가 커지면 `driver_input.c/.h` 한 쌍으로 분리한다. 초기에 통합 아키텍처의 모든 폴더/Task를 만들 필요는 없다. 이 단독 bench 단계는 최종 FreeRTOS 통합 완료와 구분한다.

| 단계 | 완료 기준 | 상태 |
|---|---|---|
| 보드 확인 | Build/Download/main 도달, 로그 또는 LED 확인 | NOT RUN |
| RF raw 수신 | 실제 입력 변화와 raw 값 변화 일치 | NOT RUN |
| 세 요청 해석 | 기어/조향/속도 요청 매핑·범위·중립 기록 | NOT RUN |
| 오류/두절 | 부팅 미수신, invalid, 송신기 OFF, 재연결 시 유효성 전이 확인 | NOT RUN |
| C↔F 통합 | CAN 계약 확정 후 같은 payload를 양쪽에서 해석, F만 최종 명령 생성 | NOT RUN |
| 출력 통합 | E-Stop·timeout·복구 검증 후 모터/서보 시험 | NOT RUN |

송신기가 꺼져도 마지막 payload는 STM32 RAM에 남는다. 저장값을 다시 읽는 것만으로 수신 시각을 갱신하지 않는다. 새로운 유효 패킷이 timeout 안에 들어오지 않으면 입력을 invalid로 바꾸고, 송신기가 보낸 input_valid=false도 반영한다.

RF 속도 요청을 기존 `Driver_Input.accel/brake`로 변환할지, CAN에 `speed_request`를 추가할지는 C/F 공통 결정(`DEC-CTRL-019`, 최상위 명세 §4.8.1)이다. 수신기에서 브레이크를 제공하지 않는데 측정된 브레이크처럼 보고하지 않는다. CAN ID/단위/byte layout은 이번 가이드에서 새로 확정하지 않는다.

첫 성공 목표는 **모터를 움직이는 것 이전에, 조종기를 움직였을 때 STM32의 세 요청값이 바뀌고 조종기를 껐을 때 입력이 invalid가 되는 것을 확인하는 것**이다. 시험 결과는 [TEST_REPORT.md](TEST_REPORT.md)에 기록한다.
