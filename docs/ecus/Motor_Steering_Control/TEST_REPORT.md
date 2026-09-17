# Motor + Steering Control Test Report

> **2026-09-17 C 입력 계획 변경:** 기어·조향·속도 요청은 RF로 STM32(C)에 수신한다. E-Stop은 로컬 GPIO/EXTI 차단을 유지한다. RF 모델은 nRF24L01, STM32 연결은 SPI로 확정했다. 모듈 보드/핀/패킷/수치와 CAN 매핑은 OPEN이다. 아래 2026-09-15 기록의 가변저항·로컬 Gear GPIO 설명은 변경 이력이며 현재 입력 구성에 적용하지 않는다.

> 2026-09-11: STM32G431KB 구매 모델 부분 동결. [최상위 명세](../../system/FINAL_IMPLEMENTATION_SPEC.md) DEC-HW-001~005를 따른다. 제조사/revision/핀 배정과 실기 시험은 별도이며, 아래 시험 결과/측정값을 PASS로 변경한 것은 아니다.

> **2026-09-15 범위 변경 (1차):** Encoder/Hall 관련 테스트 항목을 전부 삭제했다. RF/가변저항 `Driver_Input` 읽기 시험과 명령값 기반 speed/rpm 추정 검증 항목으로 대체했다.
>
> **2026-09-15 범위 변경 (2차):** E-Stop/Gear 물리 입력 시험 항목을 F에서 C로 이전했다. E-Stop은 CAN 비의존 로컬 즉시 차단 시험을 추가했다. 근거: [`FINAL_IMPLEMENTATION_SPEC.md` §1, §1.2, §3.1, §4.8.1, §8 C Drive](../../system/FINAL_IMPLEMENTATION_SPEC.md).

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md`의 요구사항을 실제 시험으로 검증하고, FreeRTOS 기반 제어 Node의 기능뿐 아니라 **ControlTask timing, ISR→Task, Stack, Queue, Watchdog/Health**까지 확인한다.
> **현재는 시험 전 계획 상태이므로 실제 측정값을 임의로 채우지 않는다.** 시험 후 `NOT RUN`과 `TBD`를 실제 결과로 교체한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Motor + Steering Control ECU |
| Owner | C |
| Board / Platform | STM32G431KB (STM32 #2) + Motor Driver + Brushed DC Motor + RC Servo + RF 수신기 |
| Execution Model | FreeRTOS + CMSIS-RTOS2 기본 |
| Firmware / SW Commit | `main` HEAD (Driver_Input, SPI1 NSS Pulse Disable + `USE_NUCLEO_32` 보드 매크로 수정 + FDCAN1 Internal Loopback 시험 코드 포함) |
| Test Date | 2026-09-17 (T-RF-000, FDCAN1 internal loopback, SPI1 bus 기본 동작만; RF 실통신은 미실행) |
| Specification Revision | v0.4 |
| Architecture Revision | v0.4 |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.6 | 2026-09-17 | C 사용자 | FDCAN1 Internal Loopback PASS, SPI1 bus 기본 동작 확인(모듈 미연결, 타임아웃 없음). `USE_NUCLEO_64`→`USE_NUCLEO_32` 보드 매크로 오류 발견/수정(§13) |
| v0.5 | 2026-09-17 | C 사용자 | T-RF-000 보드 bring-up 시험 실행 (PASS). Firmware/Test Date 갱신. RF raw 입력(T-RF-001~005)은 여전히 NOT RUN |
| v0.4 | 2026-09-17 | C 사용자 요청 | RF 기어·조향·속도 요청, Driver_Input 시작 순서, RF 오류/두절 시험 계획; 실기 NOT RUN |
| v0.1 | 2026-09-09 | Team | Initial planned RTOS control test example |
| v0.2 | 2026-09-15 | Team | Encoder/Hall 시험 항목 삭제, Driver_Input 읽기 및 speed/rpm 추정 시험 항목으로 대체 |

---

## 추가 E-Stop 검증 계획

| 조건 | 기대 동작 | 실행 상태 |
|---|---|---|
| 부팅 전부터 E-Stop active | Motor Enable/STBY 비활성 유지 | NOT RUN |
| E-Stop active + 반복 enable command | ControlTask가 로컬 차단을 덮어쓰지 않음 | NOT RUN |
| E-Stop active + CAN 단절/RTOS 부하 | CAN 수신/Task 실행을 기다리지 않고 로컬 차단 | NOT RUN |
| E-Stop 해제 | 단순 해제만으로 재구동하지 않음; DEC-CTRL-006 확정 조건 적용 | NOT RUN |

# 1. Test Objective

## Driver_Input RF 우선 시험 (2026-09-17 추가)

T-RF-000(보드 bring-up)만 실행해 PASS했고, RF raw 입력 관련 T-RF-001~005는 nRF24L01 모듈 배선 전이라 아직 미실행이다. RF 모델·프로토콜·채널·단위·timeout을 먼저 기록하고 시험한다. 초기에는 모터/서보 출력 비활성 상태로 raw 값과 validity만 확인한다.

| Test ID | Requirement | 조건 | 기대 결과 | 실행 상태 |
|---|---|---|---|---|
| T-RF-000 | 보드 bring-up | 현재 Driver_Input Build/Download/Reset | main 도달, COM1 시작 로그 또는 LED 확인 | PASS (2026-09-17) |
| T-RF-001 | REQ-DRV-RF-001 | 기어 각 위치, 조향 좌/중립/우, 속도 최소/최대 | 세 raw 값/수신 시각/valid 확인, 측정한 매핑과 일치 | NOT RUN |
| T-RF-002 | REQ-DRV-RF-002 | 부팅 미수신, 누락 채널, invalid gear, 범위 초과, 패킷 오류 | 전체 요청 invalid, freshness를 정상 수신처럼 갱신하지 않음 | NOT RUN |
| T-RF-003 | REQ-DRV-RF-003 | 송신기 OFF, 수신기 분리, 중복/오래된 패킷 | 합의한 timeout/failsafe 규칙으로 invalid; 저장된 payload 재사용으로 freshness 갱신 금지 | NOT RUN |
| T-RF-004 | REQ-DRV-RF-004 | 재연결, 조작 유지, E-Stop 활성/해제 | 자동 구동 재개 없음, E-Stop 차단 유지, 합의된 복구 조건 확인 | NOT RUN |
| T-RF-005 | REQ-DRV-001 | CAN 계약 확정 후 C↔F 통합 | 요청/추정 속도 구분, RF Gear 요청을 F가 중재, 동일 encode/decode | NOT RUN |

**T-RF-000 결과 (2026-09-17):** SPI1(PA5/PA6/PA7, 8bit, /128 prescaler)·FDCAN1(PA11/PA12)·NRF_CE/NRF_CSN(PA1/PA4) 핀 구성을 반영한 `Driver_Input`을 STM32CubeIDE에서 Build → ST-LINK로 NUCLEO-G431KB에 Download, STM32CubeProgrammer verify 성공. 리셋 후 COM1(115200 8N1)에서 `Welcome to STM32 world !` 정상 출력 확인(§12 로그 참고). SPI/FDCAN 신호 자체는 아직 nRF24L01/CAN 버스에 연결하지 않아 시험하지 않았고, main 도달과 디버그 UART 경로만 확인한 것이다.

**추가 bench 확인 (2026-09-17, 배선 없이):**
- **FDCAN1 Internal Loopback: PASS.** Global filter를 accept-all로 설정하고 ID `0x123`/8바이트 더미 데이터를 TX FIFO에 넣은 뒤 RX FIFO0에서 동일 ID/데이터로 수신 확인. 외부 트랜시버/버스 연결 없이 FDCAN1 주변장치 자체 동작만 검증한 것이며, 실제 CAN 버스 통신 검증은 아니다.
- **SPI1 bus 기본 동작: 조건부 확인.** nRF24L01 미연결 상태에서 STATUS/CONFIG 레지스터 read/write 명령을 보내면 항상 `0xFF`가 반환된다(MISO floating 상태의 예상 동작). 처음에는 모든 SPI 호출이 `HAL_TIMEOUT`으로 실패했는데, 원인은 `.cproject`의 보드 매크로가 `USE_NUCLEO_64`로 잘못 설정되어 BSP LED 드라이버가 PA5(SPI1_SCK)를 LED 핀으로 오인, `BSP_LED_Init/On`이 PA5를 GPIO로 재설정해 SPI 클럭을 깨뜨린 것이었다. `USE_NUCLEO_32`로 수정 후 타임아웃 없이 SPI 트랜잭션이 완료됨(값은 여전히 `0xFF`, 슬레이브 미연결이므로 예상대로). 상세는 §13 참고.

로그에는 RF 모델, 펌웨어 식별자, raw 값, 해석한 요청, validity 사유, 마지막 유효 수신 시각과 timeout 검출 시각을 남긴다. RF 속도와 기존 CAN accel/brake 매핑 확정 전 아래 accel/brake 시험은 미실행 계획이며 채널이 존재한다는 증거가 아니다.

Drive + Steering ECU가 RF Driver 입력을 읽어 `Driver_Input`으로 발행하고, VCU의 최종 Command를 받아 Motor/Servo 출력으로 변환하며, 모터 명령값 기반 추정 함수로 Speed/RPM 표시값을 산출하는지 확인한다. 또한 Command Timeout, Invalid Input, CAN burst 등의 조건에서도 ControlTask가 정의된 주기 안에서 동작하고 안전한 상태 전환 및 Health 정보를 제공하는지 검증한다.

초기 Motor/Servo 시험은 낮은 출력의 bench 조건에서 수행하며, 전체 차량 주행 시험보다 먼저 단독 기능과 timeout 동작을 검증한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board / MCU | STM32G431KB (STM32 #2, NUCLEO-G431KB), 구매 모델 확정 / bring-up(빌드·다운로드·COM1 로그) PASS, RF/CAN 기능 시험은 NOT RUN |
| RTOS | FreeRTOS version TBD |
| CMSIS-RTOS API | CMSIS-RTOS2 |
| Motor | TBD |
| Motor Driver | TB6612FNG 후보, 최종 확정 전 |
| Driver Input 장치 | RF 수신기, `DEC-HW-024`/`DEC-HW-029` 확정 전 |
| Steering Servo | TBD |
| Power | 실제 시험 시 기록 |
| CAN Interface | FDCAN1, PA11(RX)/PA12(TX) 핀 구성 완료 / Internal Loopback PASS / 실버스(트랜시버) 시험 NOT RUN |
| CAN Bitrate | TBD |
| Debug | STM32CubeIDE / ST-Link(FW V3J16M9) / UART COM1 115200 8N1 — bring-up 확인 완료 |
| Measurement | Logic Analyzer / Oscilloscope / CAN logger 후보 |

## Wiring / Setup

| Device | Pin / Port | Connection | Note |
|---|---|---|---|
| Motor Driver PWM | TBD | STM32 TIM PWM | actual pin TBD |
| Motor Driver DIR | TBD | STM32 GPIO | actual pin TBD |
| Motor Driver STBY/Enable | TBD | STM32 GPIO | safe init 확인 |
| Driver Input (기어/조향/속도 요청) | SPI1: SCK=PA5/MISO=PA6/MOSI=PA7, CE=PA1, CSN=PA4 | SPI + CE/CSN GPIO | 핀 구성 완료(코드), nRF24L01 실배선/통신은 NOT RUN. 실제 보드 커넥터 위치는 배선 전 재확인 필요 |
| Servo PWM | TBD | STM32 TIM PWM | actual servo spec 기준 |
| CAN FD Transceiver | PA11(RX)/PA12(TX) | STM32 FDCAN1 | 핀 구성 완료(코드), 트랜시버 연결/버스 시험 NOT RUN |

사진/회로/핀맵 링크: TBD

---

# 3. Requirement Verification Matrix

| Test ID | Requirement ID | Test Method | Expected | Result | PASS/FAIL |
|---|---|---|---|---|---|
| T-DRV-001 | REQ-DRV-001 | RF(기어·조향·속도 요청) 신호 입력 | `Driver_Input` CAN 발행 | NOT RUN | TBD |
| T-DRV-001a | REQ-DRV-001a | E-Stop 활성화 (CAN 연결 끊은 상태) | CAN 없이도 Motor Driver 즉시 disable | NOT RUN | TBD |
| T-DRV-001b | REQ-DRV-001b | E-Stop 활성화 | `Driver_Input.estop_status=true` CAN 발행 | NOT RUN | TBD |
| T-DRV-002 | REQ-DRV-002 | Dummy/real CAN command | Drive/Steering command 수신 | NOT RUN | TBD |
| T-DRV-003 | REQ-DRV-003 | invalid/out-of-range command | actuator에 직접 적용되지 않음 | NOT RUN | TBD |
| T-DRV-004 | REQ-DRV-004 | `Drive_Enable=false` | Motor safe state | NOT RUN | TBD |
| T-DRV-005 | REQ-DRV-005 | speed command step | PWM/DIR mapping | NOT RUN | TBD |
| T-DRV-006 | REQ-DRV-006 | steering min/center/max | calibrated servo command | NOT RUN | TBD |
| T-DRV-007 | REQ-DRV-007 | 명령값 단계 변화 | speed/rpm 추정값 변화, estimated 표기 확인 | NOT RUN | TBD |
| T-DRV-008 | REQ-DRV-008 | VCU command stop | timeout 후 stale command 미유지 | NOT RUN | TBD |
| T-DRV-009 | REQ-DRV-009 | CAN status monitor | Drive_Status/Fault TX | NOT RUN | TBD |
| T-DRV-010 | REQ-DRV-010 | power/reset | unintended motor movement 없음 | NOT RUN | TBD |
| T-DRV-011 | REQ-DRV-011 | architecture/code inspect | Encoder/Hall 하드웨어 의존성 없음 | NOT RUN | TBD |
| T-DRV-012 | REQ-DRV-012 | RTOS inspect | CAN/Control/DriverInput/Status 분리 | NOT RUN | TBD |
| T-DRV-013 | REQ-DRV-013 | load/logging test | ControlTask timing 유지 | NOT RUN | TBD |
| T-DRV-014 | REQ-DRV-014 | health injection | task/queue/timeout health 검출 | NOT RUN | TBD |
| T-DRV-015 | REQ-DRV-015 | watchdog health test | policy대로 refresh/withhold | NOT RUN | TBD |
| T-DRV-016 | REQ-DRV-016 | datasheet/spec review | Motor/Driver 적합성 확인 | NOT RUN | TBD |

---

# 4. Normal Function Test

| Test ID | Input / Condition | Expected Output | Actual / Measured | Evidence | Result |
|---|---|---|---|---|---|
| T-DRV-001-A | RF accel 입력 변화 | `Driver_Input.accel` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-001-B | RF brake 입력 변화 | `Driver_Input.brake` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-001-C | RF steer 입력 변화 | `Driver_Input.steering` 반영 | NOT RUN | log TBD | TBD |
| T-DRV-005-A | low speed request | 낮은 Motor PWM | NOT RUN | scope/log TBD | TBD |
| T-DRV-005-B | speed request increase | PWM mapping 증가 | NOT RUN | scope/log TBD | TBD |
| T-DRV-005-C | stop request | Motor output safe/zero policy | NOT RUN | scope/video TBD | TBD |
| T-DRV-006-A | steering left candidate | left calibrated output | NOT RUN | video/scope TBD | TBD |
| T-DRV-006-B | steering center | center output | NOT RUN | video/scope TBD | TBD |
| T-DRV-006-C | steering right candidate | right calibrated output | NOT RUN | video/scope TBD | TBD |
| T-DRV-007-A | PWM 명령값 증가 | 추정 rpm/speed 증가, estimated flag 유지 | NOT RUN | UART/log TBD | TBD |
| T-DRV-009-A | normal active state | Drive_Status periodic TX | NOT RUN | CAN log TBD | TBD |

---

# 5. Boundary / Calibration Test

## 5.1 Motor PWM / 추정 RPM

| Condition | Command | PWM | Estimated RPM | Note | Result |
|---|---:|---:|---:|---|---|
| Stop | TBD | TBD | NOT RUN | | TBD |
| Low | TBD | TBD | NOT RUN | | TBD |
| Mid | TBD | TBD | NOT RUN | | TBD |
| High test limit | TBD | TBD | NOT RUN | bench 범위만 | TBD |

Motor/Driver의 실제 전기적 한계를 확인하기 전 무리하게 최대 출력 시험을 하지 않는다. 여기서 기록하는 RPM은 실측이 아니라 추정 함수 출력값이다.

## 5.2 Steering Calibration

| Position | Command | Servo Output | Mechanical Angle / Position | Note | Result |
|---|---:|---:|---:|---|---|
| Left safe limit | TBD | TBD | NOT RUN | 기구 한계 확인 | TBD |
| Center | TBD | TBD | NOT RUN | 기준점 | TBD |
| Right safe limit | TBD | TBD | NOT RUN | 기구 한계 확인 | TBD |

실제 Servo 사양과 차량 steering linkage를 기준으로 limit을 정한다.

## 5.3 Driver Input Calibration

| Item | Value |
|---|---|
| Driver Input 장치 (RF) | TBD (`DEC-HW-024`/`DEC-HW-029`) |
| Accel 입력 → speed 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| Brake 입력 → speed 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| Brake 감속 감지 → brake_lamp threshold | TBD (`DEC-CTRL-020`) |
| Steer 입력 → steering 선형 매핑 계수 | TBD (`DEC-CTRL-019`) |
| 매핑 검증됨? | NOT RUN |

## 5.4 Speed/RPM 추정 함수 Calibration

| Item | Value |
|---|---|
| PWM → RPM 추정 함수 형태 | TBD (`DEC-CTRL-021`) |
| PWM → Vehicle Speed 추정 함수 형태 | TBD (`DEC-CTRL-021`) |
| 실측 대비 참고 오차(있는 경우) | N/A — 실측 비교 대상이 아님, 참고용 bench 비교만 가능 |
| 추정 함수 검증됨? | NOT RUN |

---

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected Detection | Expected Safe/Recovery Action | Actual | Result |
|---|---|---|---|---|---|
| F-DRV-000 | E-Stop active + CAN cable 분리 | GPIO EXTI (CAN 무관) | Motor Driver Enable/STBY 즉시 disable | NOT RUN | TBD |
| F-DRV-001 | VCU command timeout | last_rx timeout | Motor safe state + fault | NOT RUN | TBD |
| F-DRV-002 | `Drive_Enable=false` while command exists | enable check | output disable | NOT RUN | TBD |
| F-DRV-003 | speed request out-of-range | range validation | reject/clamp policy | NOT RUN | TBD |
| F-DRV-004 | steering request beyond calibrated limit | range validation | mechanical limit 밖 command 금지 | NOT RUN | TBD |
| F-DRV-005 | Driver Input 신호 disconnected | signal timeout | `Driver_Input` invalid, 안전 기본값 | NOT RUN | TBD |
| F-DRV-006 | Driver Input 신호 implausible | plausibility | invalid flag | NOT RUN | TBD |
| F-DRV-007 | CAN burst | queue occupancy | ControlTask deadline 유지 | NOT RUN | TBD |
| F-DRV-008 | CanRxQueue full | RTOS API/counter | overflow health flag | NOT RUN | TBD |
| F-DRV-009 | ControlTask artificial delay | overrun detector | health fault / evidence | NOT RUN | TBD |
| F-DRV-010 | CanTxTask delayed | period monitor | ControlTask 영향 제한 | NOT RUN | TBD |
| F-DRV-011 | debug logging load | timing trace | control jitter 영향 제한 | NOT RUN | TBD |

---

# 7. Timing / Performance Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| `ControlTask` period | 5~10 ms 후보 | NOT RUN | GPIO toggle / trace / timestamp | TBD |
| `ControlTask` execution time | TBD | NOT RUN | runtime timestamp | TBD |
| `ControlTask` jitter | TBD | NOT RUN | trace/statistics | TBD |
| `DriverInputTask` update | 5~10 ms 후보/event | NOT RUN | timestamp | TBD |
| CAN RX → valid command | TBD | NOT RUN | RX/task timestamp | TBD |
| command → PWM update | TBD | NOT RUN | CAN timestamp + scope | TBD |
| Status CAN period | 20~50 ms 후보 | NOT RUN | CAN log | TBD |
| Command timeout detection | TBD | NOT RUN | CAN stop + timestamp | TBD |

후보 값은 초기 Architecture 가정이며 실제 결과를 보고 Specification/Architecture를 함께 수정한다.

---

# 8. RTOS Test

## 8.1 Task Inventory

| Task | Expected Period / Trigger | Relative Priority | Observed | Result |
|---|---|---|---|---|
| `CanRxTask` | CAN event | High | NOT RUN | TBD |
| `ControlTask` | 5~10 ms 후보 | Highest application | NOT RUN | TBD |
| `DriverInputTask` | event / 5~10 ms 후보 | High | NOT RUN | TBD |
| `CanTxTask` | 20~50 ms 후보 + event | Normal | NOT RUN | TBD |
| `HealthTask` | 50~100 ms 후보 | Low/Normal | NOT RUN | TBD |

## 8.2 Period / Jitter

| Task | Target Period | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|
| ControlTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| DriverInputTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| CanTxTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| HealthTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |

## 8.3 Stack / Memory

| Task / Item | Configured | Minimum Free / High-Water | Target | Result |
|---|---:|---:|---:|---|
| ControlTask stack | TBD | NOT RUN | TBD | TBD |
| CanRxTask stack | TBD | NOT RUN | TBD | TBD |
| DriverInputTask stack | TBD | NOT RUN | TBD | TBD |
| CanTxTask stack | TBD | NOT RUN | TBD | TBD |
| HealthTask stack | TBD | NOT RUN | TBD | TBD |
| Free heap | TBD | NOT RUN | TBD | TBD |

## 8.4 Queue / Event / Notification

| Object | Depth / Config | Max Occupancy / Result | Overflow Test | Result |
|---|---|---|---|---|
| CanRxQueue | TBD | NOT RUN | Planned | TBD |
| CommandQueue/latest object | TBD | NOT RUN | Planned | TBD |
| DriverInputNotify | notification | NOT RUN | Planned | TBD |
| DriverInputQueue | TBD | NOT RUN | Planned | TBD |
| StatusQueue | TBD | NOT RUN | Planned | TBD |

## 8.5 ISR → Task Test

| Interrupt | Expected ISR Action | Expected Task Wake-up | Actual | Result |
|---|---|---|---|---|
| FDCAN RX | enqueue/notify only | CanRxTask | NOT RUN | TBD |
| Driver Input RF 수신 IRQ, 필요 시 | raw sample/timestamp only | DriverInputTask | NOT RUN | TBD |

Code Review 항목:
- [ ] ISR에서 제어 연산 없음
- [ ] ISR에서 `printf` 없음
- [ ] ISR에서 blocking API 없음
- [ ] ISR에서 Servo/Motor 전체 control logic 수행하지 않음

## 8.6 Priority / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| CAN burst + normal control | ControlTask deadline 유지 | NOT RUN | TBD |
| heavy UART logging | control timing 영향 제한 | NOT RUN | TBD |
| CanTxTask delayed | motor/steering output timing 유지 | NOT RUN | TBD |
| Driver Input event burst | deadlock/starvation 없음 | NOT RUN | TBD |

## 8.7 Watchdog / Health Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all critical tasks healthy | watchdog refresh 후보 | NOT RUN | TBD |
| ControlTask health missing | health fault / refresh 중단 정책 | NOT RUN | TBD |
| command timeout | safe output + health fault | NOT RUN | TBD |
| queue overflow | counter/fault 반영 | NOT RUN | TBD |
| stack low watermark | health warning 후보 | NOT RUN | TBD |

Watchdog reset 시험은 bench 상태에서 수행하고, Motor/Servo가 안전 상태인지 먼저 확인한다.

---

# 9. Communication Test

## CAN / CAN FD

| Message / Signal | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Driver_Input` | TX | accel/brake/steer 발행 | NOT RUN | N/A | TBD |
| `Final_Speed_Request` | RX | command update | NOT RUN | Planned | TBD |
| `Final_Steering_Request` | RX | steering update | NOT RUN | Planned | TBD |
| `Drive_Enable` | RX | output enable/disable | NOT RUN | Planned | TBD |
| `Vehicle_Gear` | RX | drive context | NOT RUN | Planned | TBD |
| `Motor_RPM` (estimated) | TX | 추정 rpm | NOT RUN | N/A | TBD |
| `Drive_Status` | TX | state/health | NOT RUN | N/A | TBD |
| `Steering_Status` | TX | target/valid | NOT RUN | N/A | TBD |
| `ECU_Heartbeat` | TX | node alive | NOT RUN | N/A | TBD |
| `DTC_Event` | TX | local fault event | NOT RUN | N/A | TBD |

## LIN

N/A.

---

# 10. DTC / Diagnostics Test

| Fault | Expected DTC / Status | H735 Active 표시? | 해소 시 제거? | Result |
|---|---|---|---|---|
| VCU command timeout | `DRV_COMM_TIMEOUT` 후보 | NOT RUN | NOT RUN | TBD |
| Driver Input timeout | `DRV_INPUT_TIMEOUT` 후보 | NOT RUN | NOT RUN | TBD |
| ControlTask overrun | `DRV_TASK_OVERRUN` 후보 | NOT RUN | NOT RUN | TBD |
| CAN fault | `DRV_CAN_FAULT` 후보 | NOT RUN | NOT RUN | TBD |

실제 DTC code와 status lifecycle은 F/DTC 통합 규격을 따른다.

---

# 11. Soak / Load Test

| Test | Duration / Load | Expected | Actual | Result |
|---|---|---|---|---|
| Normal low-output bench soak | TBD | reset/deadlock/overflow 없음 | NOT RUN | TBD |
| CAN burst + periodic control | TBD | ControlTask timing 유지 | NOT RUN | TBD |
| Driver Input activity + CAN + status | TBD | queue/stack 정상 | NOT RUN | TBD |
| Repeated enable/disable | TBD | stale/unsafe output 없음 | NOT RUN | TBD |
| Repeated steering command | TBD | calibrated range 유지 | NOT RUN | TBD |

---

# 12. Logs / Evidence

- UART / Console Log: T-RF-000 (2026-09-17, COM1 115200 8N1, 리셋 직후):
  ```text
  Welcome to STM32 world !
  ```
  (최초 확인 시 `printf` 개행이 `\n\r` 순서라 터미널에서 계단식으로 출력됐음 → `\r\n`으로 수정 후 정상 출력 확인)
- Wiring Photo: TBD
- Motor/Servo Test Video: TBD
- CAN Log: TBD
- PWM Scope Capture: TBD
- Driver Input Capture Trace: TBD
- FreeRTOS Runtime Stats: TBD
- Stack High-Water Log: TBD
- Queue Occupancy/Overflow Log: TBD

예시 로그 형식:

```text
[DRV][INIT] output=safe
[DRV][INPUT] accel=... brake=... steer=... valid=1
[DRV][CAN] speed_req=... steer_req=... enable=1
[DRV][CTRL] pwm=... dir=... servo=...
[DRV][EST] rpm_estimated=... speed_estimated=...
[DRV][TIMEOUT] VCU command stale
[DRV][SAFE] motor_output=0
[DRV][HEALTH] control_alive=1 queue_overflow=0
```

---

# 13. Problems and Fixes

| Problem | Root Cause | Fix | Retest Result | Prevention |
|---|---|---|---|---|
| nRF24 SPI 레지스터 read/write가 항상 `HAL_TIMEOUT`으로 실패 (2026-09-17) | `.cproject` preprocessor define이 `USE_NUCLEO_64`로 잘못 설정됨(보드는 Nucleo-**32**). `stm32g4xx_nucleo.h`의 `#if defined(USE_NUCLEO_64)` 분기가 선택되어 `LED2_PIN=PA5`(GPIOA)로 정의됨 — SPI1_SCK와 동일 핀. `BSP_LED_Init()`/`BSP_LED_On()`(SPI1 Init 이후 호출)이 PA5를 GPIO_Output으로 재설정해 SPI 클럭을 깨뜨림 | `.cproject`의 `USE_NUCLEO_64` 정의 6곳을 `USE_NUCLEO_32`로 수정(Debug/Release 빌드 설정 전체). Nucleo-32 정의를 쓰면 `LED2_PIN=PB8`(GPIOB)로 정상 매핑되어 `.ioc`의 기존 `PB8-BOOT0` GPIO_Output 설정과도 일치 | 수정 후 SPI1 read/write 타임아웃 사라짐(값은 `0xFF`, nRF24L01 미연결 상태의 예상값). Clean 후 재빌드 필요(증분 빌드로는 매크로 변경이 반영 안 될 수 있음) | 향후 CubeMX로 이 프로젝트를 재생성/마이그레이션할 때마다 `.cproject`의 보드 매크로가 `USE_NUCLEO_32`인지 재확인. LED/버튼 등 BSP 함수를 쓰는 프로젝트에서 그 핀과 겹치는 주변장치를 추가할 때는 BSP 핀 매핑을 먼저 확인 |

---

# 14. Final Result

```text
RESULT: NOT RUN
```

## PASS 조건

- [ ] Motor output이 안전한 초기 상태에서 시작한다.
- [ ] 낮은 출력 bench test에서 PWM/DIR 동작을 재현한다.
- [ ] Servo center/left/right calibration을 기록한다.
- [ ] RF 입력을 읽어 `Driver_Input`을 CAN으로 발행한다.
- [ ] 명령값 기반 speed/rpm 추정값을 산출하고 estimated임을 확인한다.
- [ ] VCU CAN command를 받아 output에 반영한다.
- [ ] Command timeout에서 stale command를 유지하지 않는다.
- [ ] Drive_Status / Motor_RPM(estimated) / Steering_Status CAN TX를 확인한다.
- [ ] ControlTask/DriverInputTask period/jitter를 측정한다.
- [ ] Stack high-water를 확인한다.
- [ ] Queue occupancy/overflow 정책을 확인한다.
- [ ] ISR→Task 구조를 확인한다.
- [ ] logging/CAN burst에서 ControlTask starvation이 없다.
- [ ] Watchdog/Health 정책을 확인한다, 해당 시.
- [ ] Motor/Driver 정격 적합성 검토를 완료한다.
- [ ] 로그/사진/영상/trace 증거를 남긴다.

## Remaining Issues

- 실제 Motor/Driver 정격 확정 필요
- nRF24L01 모듈 보드/핀/패킷 확정 필요 (`DEC-HW-024`/`DEC-HW-029`)
- 입력값→speed/steering 선형 매핑 계수 확정 필요 (`DEC-CTRL-019`)
- Motor 명령값→speed/rpm 추정 함수 형태 확정 필요 (`DEC-CTRL-021`)
- Brake 감속 감지→brake_lamp threshold 확정 필요 (`DEC-CTRL-020`)
- Servo calibration 필요
- STM32/FDCAN pin map 확정 필요
- CAN Matrix 확정 필요
- ControlTask/DriverInputTask 실제 주기 확정 필요
- Command timeout 및 steering timeout policy 확정 필요
