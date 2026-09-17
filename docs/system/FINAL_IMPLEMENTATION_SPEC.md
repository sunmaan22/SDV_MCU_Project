# Final Implementation Specification

> **2026-09-17 E-Stop 전체 삭제:** 사용자가 데모용 프로젝트임을 근거로 E-Stop을 소프트웨어·하드웨어(물리 키스위치 포함) 전부 삭제하기로 결정했다. `DEC-HW-020`/`DEC-HW-027`은 REMOVED로 바뀌었고, `Driver_Input.estop_status` 필드·`Vehicle_State.vehicle_mode`의 `ESTOP` 상태·§1.2 안전 우선순위의 E-Stop 항목·`DEC-CTRL-001`의 ESTOP 전이·`DEC-CTRL-006`(E-Stop recovery)을 문서 전체에서 제거했다. 이 문서의 이전 버전에 있던 "E-Stop은 로컬 GPIO/EXTI로 처리" 관련 서술은 전부 이 결정으로 대체된 이력이다. 정지는 이제 Driver_Input의 speed_request(가속 요청)를 낮추는 일반 경로로만 이뤄진다 — CAN/RTOS 상태와 무관한 하드웨어 차단 경로는 없다.

> **2026-09-17 C 입력 계획 변경:** 기어·조향·속도 요청은 RF로 STM32(C)에 수신한다. RF 모델은 nRF24L01, STM32 연결은 SPI로 확정했다. 모듈 보드/핀/패킷/수치와 CAN 매핑은 OPEN이다. 아래 2026-09-15 기록의 가변저항·로컬 Gear GPIO 설명은 변경 이력이며 현재 입력 구성에 적용하지 않는다.

> **2026-09-17 하드웨어 추가 확정:** Front Camera(`DEC-HW-015`, Full HD 1080p USB-A, AU1425), Ultrasonic Sensor(`DEC-HW-009`, HC-SR04), CAN 트랜시버(`DEC-HW-006`, TJA1051(T) 모듈 — CAN FD passive, ~2Mbps), LIN 트랜시버(`DEC-HW-007`, LIN 2.1/SAE J2602 모듈)를 사용자가 구매 확정했다. 모델 선택 동결이며, 실물 회로도/정확한 칩 품번/전압 레벨 확인과 Pin/Timer/ADC/SPI 배정 등 Gate A 나머지 항목은 별도다.
>
> **2026-09-17 CAN 트랜시버 재확정:** 처음 검토했던 MCP2515+TJA1050 모듈은 CAN FD(BRS) 미지원이라 TJA1051(T)로 교체 확정했다 (`DEC-HW-006`). MCP2515는 이제 사용하지 않는다.

> **2026-09-17 Pi CAN 미사용 결정:** 모든 라즈베리파이(RF 송신측, E HPC Vision)는 CAN 인터페이스를 직접 쓰지 않는다(`DEC-HW-008` REMOVED). E(HPC Camera Vision)의 Pi는 `Vision_Status`/`ADAS_Request`를 UART로 B(IVI, STM32H735G-DK)에 보내고, **B가 그 값을 그대로 CAN에 relay 발행**한다(`DEC-HW-030`, 인터페이스는 UART로 FROZEN, 핀/보드레이트는 OPEN). 데이터의 논리적 owner/생성자는 여전히 E다 — B는 값을 해석·수정하지 않고 CAN 프레임으로 옮기기만 하는 대행자다. 사용자도 이 방식이 비효율적임을 인지한 상태에서, B에 여유 핀/CAN 포트가 있다는 이유로 선택했다.

> **2026-09-17 speed 필드 부호 제거:** RF `speed_request`(및 이와 동일 원칙을 쓰는 `Final_Drive_Command.speed_request`, `ADAS_Request.requested_speed`, `Drive_Status.vehicle_speed`)를 부호 있는 값(-100~100, 방향 포함)에서 **0~100 크기(magnitude)만**으로 바꿨다. 전/후진 방향은 이미 같은 메시지의 `gear`(P/R/N/D) 필드가 담당하는데 speed로 또 표현하면 중복이라는 지적에 따른 수정이다(`DEC-CTRL-013/019`). `accel`은 이제 speed_request를 그대로 쓰고, `brake`는 더 이상 speed_request의 음수부에서 파생하지 않으며 별도 입력 채널 여부는 `DEC-HW-018`(OPEN)에서 결정한다. 라즈베리파이 TX 쪽 `packet.hpp`/`input_terminal.cpp`에 먼저 반영하고 실행 확인(clamp 0~100)했다.

> **2026-09-17 일괄 설계 동결:** Project Owner가 직접 "실측/하드웨어 없이 결정 가능한 항목은 전부 동결하라"고 지시했다. 이에 따라 Network(§3.2, CAN ID/DLC/cycle/timeout/LIN 전부), Vehicle/Control 로직 설계(§3.3), Perception/Vision 알고리즘·표현 방식(§3.4), HMI 우선순위·정책(§3.5), DTC/Health 코드 체계(§3.6), CAN 메시지 byte layout(§4), LIN 매핑(§5.2)을 FROZEN했다. **실측이 있어야 의미 있는 수치**(calibration 계수, threshold/hysteresis, Stack/Queue 실측값, TouchGFX frame budget 등)는 여전히 OPEN이며, "방식/형식은 FROZEN, 수치는 OPEN"으로 표기된 항목은 부분 동결이다. Cycle/Timeout류는 BENCH 초기값으로 FROZEN했으며 Gate D 실측 후 재검증 대상이다(`FROZEN`은 설계 계약이며 `TEST PASS`와 별개, §7.1). Gate A~D 체크리스트도 이에 맞춰 갱신했다.

[프로젝트 홈](../../README.md) · [문서 안내](../README.md) · [폴더 목록](README.md)

> **Status:** PARTIAL FREEZE — 2026-09-17 / Implementation Baseline v1.0 미도달 (실측 필요 수치 다수 OPEN)
> **Decision Authority:** Project Owner  
> **Purpose:** 코드 작성 직전 모든 공통 Hardware / CAN / LIN / RTOS / State / DTC 값을 한 곳에서 확정하는 최종 명세서

이 문서는 프로젝트의 **최상위 구현 계약(Source of Truth)** 이다.

2026-09-11 Owner의 “H735 제외 STM 보드는 G431KB 구매 완료, 근거가 있으면 freeze” 지시에 따라
`DEC-HW-001~005`, `DEC-HW-021~023`만 이번에 동결했다. 구매 결정과 기존 H735 시험 기록을 근거로 하며,
새 실기 시험을 수행한 것은 아니다. 상세 근거와 다음 동결 조건은 [Freeze Review](FREEZE_REVIEW_2026-09-11.md)를 따른다.

> **2026-09-15 범위 변경:** Gear 물리 입력을 F에서 C로 이전했다. C가 Gear 상태를 `Driver_Input`에 포함해 CAN으로 F에 보고한다(당시 함께 이전했던 E-Stop 관련 내용은 2026-09-17에 기능 자체가 삭제됐다). Pi DTC History DB(중앙 저장/이력) 기능은 삭제했다 — 이 프로젝트에 OBD2/외부 진단 커넥터가 없어 이력 조회의 실효성이 낮으므로, 각 Node가 발행하는 `DTC_Event`를 B(IVI)가 직접 구독해 실시간(Active만) 표시한다. History/Severity 지속 저장은 없다.

```text
FINAL_IMPLEMENTATION_SPEC.md
        ↓
각 역할 SPECIFICATION.md
        ↓
각 역할 ARCHITECTURE.md
        ↓
Code
        ↓
TEST_REPORT.md
```

충돌이 발생하면 우선순위는 다음과 같다.

```text
1. FINAL_IMPLEMENTATION_SPEC.md
2. 역할별 SPECIFICATION.md
3. 역할별 ARCHITECTURE.md
4. README / 예시 / 과거 문서
```

`docs/archive/`는 구현 기준으로 사용하지 않는다.

---

# 1. 이미 고정된 시스템 규칙

아래 항목은 다시 토론하지 않고 구현 기준으로 사용한다.

| 영역 | 고정 규칙 |
|---|---|
| A Ultrasonic | 4방향(FL/FR/RL/RR) 거리 / valid / warning / local fault owner — 초음파 충돌 위험도 판단 전담 |
| B H735 | UI 표시 + 사용자 Request 생성 + `DTC_Event` 실시간 구독·표시(Active only, History 없음) + E의 `Vision_Status`/`ADAS_Request` UART 수신 → CAN relay 발행 대행(2026-09-17, `DEC-HW-030`; 값 생성/해석 없이 그대로 전달만 함) |
| C Drive | Motor / Servo 실제 actuator output owner + Driver 입력(RF) owner + RF Gear 입력 owner |
| D Gateway | CAN↔LIN mapping + LIN Master schedule owner |
| D Slave | Lamp actual state owner |
| E Vision | 전방 카메라 객체인식(COCO) 결과 + 전방 회피 ADAS 요청 owner (주차 관여 안 함). CAN 인터페이스 없음 — UART로 B에 전달, B가 CAN relay 발행 대행(2026-09-17) |
| F VCU | 최종 vehicle arbitration + Final Drive / Body Command owner |

## 1.1 고정 Message Publisher

| Logical Message | Publisher | Consumer |
|---|---|---|
| `Ultrasonic_Status` | A | F, B, E |
| `Vision_Status` | E (논리적 owner) — CAN 프레임은 B가 UART relay로 대행 발행 | F, B |
| `ADAS_Request` | E (논리적 owner) — CAN 프레임은 B가 UART relay로 대행 발행 | F |
| `Final_Drive_Command` | F | C |
| `Drive_Status` | C | F, B, E |
| `Body_User_Request` | B | F |
| `Body_Command` | F | D Gateway |
| `Body_Status` | D Gateway | F, B, E |
| `Vehicle_State` | F | All |
| `Driver_Input` | C | F |
| `DTC_Event` | 각 Local Node | F, B |
| `ECU_Heartbeat` | 각 Node | F, Pi |

같은 최종 Message를 두 Node가 동시에 publish하지 않는다. `Driver_Input`은 accel/brake/steering뿐 아니라 gear도 포함한다 (2026-09-15부터 C가 Gear 물리 입력 owner, §4.8.1 참고).

`Vision_Status`/`ADAS_Request`는 예외적으로 **논리적 owner(E)와 물리적 CAN 송신자(B)가 다르다** (2026-09-17, `DEC-HW-030`). E가 값을 생성하고 UART로 B에 보내면 B는 그 값을 그대로 CAN 프레임에 옮겨 발행할 뿐, 값을 해석·가공·재판단하지 않는다. F/다른 Node 입장에서는 여전히 "E가 발행한 메시지"로 취급하며 B를 신뢰 경계나 데이터 owner로 착각하지 않는다.

## 1.2 고정 Safety Rule

```text
Critical Fault
> Ultrasonic Collision Critical
> ADAS Safety Request
> Normal Driver Request
```

- Vision(전방 객체 회피 요청)은 Ultrasonic Collision `CRITICAL`을 해제/override하지 않는다.
- `valid=false`인 센서값은 정상 판단에 사용하지 않는다.
- `SafetyTask`는 final command를 직접 쓰지 않는다.
- `VcuControlTask`만 `final_command`의 writer다.
- Drive ECU가 `Final_Drive_Command` timeout을 감지한다.
- VCU는 `Drive_Status`, Heartbeat, Peer Message timeout을 감지한다.
- Brake와 Accelerator가 동시에 유효하게 입력되면 Brake 우선을 기본 정책으로 한다.
- D↔R은 차량이 움직이는 상태에서 즉시 반전하지 않는다.

---

### 1.3 충돌주의 기능 범위 (2026-09-15 사용자 결정)

- Parking/주차 보조 기능을 **충돌주의(Collision Warning)** 로 대체한다. 주차 공간 탐색, 주차 경로 생성, 자동 주차 조향은 포함하지 않는다. 기어 P는 기존 기어 상태이며 기능명 변경과 무관하다.
- A는 기어 R 진입을 전제로 하지 않고 FL/FR/RL/RR의 거리·valid·warning을 생성한다. B는 모든 기어에서 4방향 충돌주의 패널에 접근할 수 있게 한다. Gear R 전용 화면 자동 전환은 요구하지 않는다.
- B는 유효하고 최신인 zone별 위험도와 센서 invalid/통신 stale을 구분한다. CRITICAL 경고는 상세 패널을 닫아도 기본 계기판에서 보이며, 패널 닫기가 경고 해제나 VCU 안전 개입 해제가 되어서는 안 된다.
- 사용자는 **기존 안전 개입 유지**를 선택했다. F의 우선순위는 Critical Fault > Ultrasonic Collision Critical > ADAS Safety Request > Driver Request다. A는 위험도를 산출하고, F만 최종 감속·정지 명령을 결정하며 C가 출력한다.
- 전후진별 제어 대상 zone, 정차 시 처리, 거리 threshold/hysteresis, 감속·정지·복구 조건은 `DEC-PER-003`, `DEC-CTRL-011/012`의 OPEN 결정이다. 4방향 표시를 모든 방향의 동일 제동 규칙으로 해석하지 않는다. E의 전방 카메라 ADAS 범위는 유지한다.
- `Ultrasonic_Status` 메시지명, publisher/consumer 및 zone 필드는 유지한다. 새 `Parking_Status`/`Collision_Status` CAN 메시지를 추가하지 않는다. payload/주기/timeout 수치는 계속 OWNER INPUT이다.

# 2. 실행 환경 고정

```text
STM32 Node
→ FreeRTOS + CMSIS-RTOS2 기본

Raspberry Pi
→ Linux Service / Process / Thread
```

공통 MCU 규칙:
- ISR은 timestamp / counter / flag / notification 등 최소 처리만 한다.
- Control / Safety Task에서 blocking log 금지.
- Queue / Notification / Event / single-owner data 구조를 우선한다.
- Task period / jitter / stack high-water / queue overflow / watchdog을 측정한다.
- 실제 numeric priority / stack / queue depth는 이 문서에서 Owner가 최종 Freeze한다.

---

# 3. Owner Decision Registry

아래 값은 **Project Owner가 직접 확정**한다. 담당자나 AI가 독자적으로 최종값을 바꾸지 않는다.

Status는 `OPEN / FROZEN / REMOVED`를 사용한다. `REMOVED`는 삭제된 결정의 이력이며 구현 대상이 아니다.

## 3.1 Hardware Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-HW-001` | A Ultrasonic STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-002` | C Drive STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-003` | D Gateway STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-004` | D LIN Slave STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-005` | F VCU STM32 모델 | STM32G431KB 기반 구매 보드 | FROZEN |
| `DEC-HW-006` | CAN FD Transceiver 모델 | TJA1051(T) 고속 저전력 CAN 트랜시버 모듈 (2026-09-17 사용자 확정, MCP2515+TJA1050 대체). NXP 분류상 "CAN FD passive" 등급 — CAN FD 데이터 phase 약 2Mbps까지 안정 동작(TJA1050과 달리 FD 사용 가능). STM32(A/C/D Gateway/D Slave/F)의 native FDCAN 페리페럴에 CANH/CANL/TXD/RXD/VCC/GND로 직결하며, 외부 CAN 컨트롤러(SPI 방식)는 사용하지 않는다. 실제 CAN FD(BRS) 사용 여부와 bitrate는 `DEC-NET-001/002`에서 별도 확정 | FROZEN |
| `DEC-HW-007` | LIN Transceiver 모델 | LIN 2.1/SAE J2602 트랜시버, LIN 버스 모듈(마스터-슬레이브 프로토콜 컨트롤러) (2026-09-17 사용자 확정). 정확한 트랜시버 칩 품번은 실물 수령 후 회로도/실크스크린으로 재확인 필요 (모델 선택 동결이며 §3.1 하단 "동결 범위" 원칙과 동일하게 실물 확인은 별도) | FROZEN |
| `DEC-HW-008` | Pi CAN FD Interface | 미사용 — 모든 라즈베리파이(E Vision, RF 송신측)는 CAN 인터페이스를 직접 쓰지 않는다 (2026-09-17 사용자 결정). E는 UART로 B에 전달하고 B가 CAN relay 발행을 대행한다(`DEC-HW-030`) | REMOVED |
| `DEC-HW-009` | Ultrasonic Sensor 모델 | HC-SR04 (2026-09-17 사용자 확정; 개수 4개는 DEC-HW-025에서 FROZEN). ECHO 출력이 5V라 STM32 3.3V GPIO에 직결하지 않고 레벨 다운(전압 분배 등)을 거친다 | FROZEN |
| `DEC-HW-010` | Motor 모델 | OWNER INPUT | OPEN |
| `DEC-HW-011` | Motor Driver | OWNER INPUT | OPEN |
| `DEC-HW-012` | Encoder/Hall | 미사용 — Speed/RPM 표시는 명령값(PWM 등) 기반 추정으로 대체 | REMOVED |
| `DEC-HW-013` | RC Servo | OWNER INPUT | OPEN |
| `DEC-HW-014` | Ambient Sensor | 미사용 — Ambient 기능 삭제 | REMOVED |
| `DEC-HW-015` | Front Camera | Full HD 1080p USB-A 웹캠, AU1425 (2026-09-17 사용자 확정, 전방 전용, COCO 기반 객체인식용) | FROZEN |
| `DEC-HW-016` | Rear Camera | 미사용 — 충돌주의는 초음파 4방향 전용, Rear Vision 삭제 | REMOVED |
| `DEC-HW-017` | RF 속도/스로틀 요청 소스 | OWNER INPUT (별도 ADC 센서 연결을 전제하지 않음) | OPEN |
| `DEC-HW-018` | RF 브레이크 요청 표현 | OWNER INPUT (별도 채널/통합 스틱/미제공 여부 확인) | OPEN |
| `DEC-HW-019` | RF 조향 요청 소스 | OWNER INPUT (채널/필드/중립/방향) | OPEN |
| `DEC-HW-020` | E-Stop owner node / 동작 방식 | 미사용 — E-Stop 기능 전체 삭제 (2026-09-17 사용자 결정, 데모용 프로젝트라 불필요 판단; 물리 키스위치도 없음) | REMOVED |
| `DEC-HW-024` | RF 송수신기 모델 / MCU 인터페이스 | nRF24L01 / SPI (2026-09-17 사용자 확정); 모듈 보드/전원/핀/무선 설정/패킷은 DEC-HW-029 | FROZEN |
| `DEC-HW-029` | nRF24L01 실장 모듈 / 배선 / RF 패킷 설정 | OWNER INPUT (모듈 제조사/전원, SPI·CE·CSN·IRQ 핀, RF 주소/채널/data rate/CRC/ACK/payload) | OPEN |
| `DEC-HW-025` | Ultrasonic 4방향 센서 배치 | 전좌(FL) / 전우(FR) / 후좌(RL) / 후우(RR) 4개 고정 | FROZEN |
| `DEC-HW-026` | Gear owner node / 동작 방식 | C가 RF Gear 요청을 수신, `Driver_Input.gear`로 CAN 발행 (2026-09-17 사용자 변경 지시; 채널/값은 DEC-HW-028) | FROZEN |
| `DEC-HW-027` | E-Stop 회로/부품 (스위치 모델, pull-up/down, debounce) | 미사용 — E-Stop 기능 전체 삭제 (`DEC-HW-020`과 동일 사유) | REMOVED |
| `DEC-HW-028` | RF Gear 채널/필드와 P/R/N/D 매핑 | OWNER INPUT (스위치 위치 수, invalid/failsafe 포함) | OPEN |
| `DEC-HW-030` | E(Vision Pi) ↔ B(IVI) 연결 방식 | UART (2026-09-17 사용자 확정, 인터페이스 종류만). B가 `Vision_Status`/`ADAS_Request`를 CAN으로 relay 발행하는 대행 경로(§1.1 예외 참고). 구체 UART 포트/핀/보드레이트/프레이밍(패킷 포맷)은 별도 OPEN 항목(`DEC-HW-031`) | FROZEN |
| `DEC-HW-031` | E↔B UART 포트/핀/보드레이트/프레이밍 | OWNER INPUT | OPEN |
| `DEC-HW-021` | B IVI 보드 | STM32H735G-DK | FROZEN |
| `DEC-HW-022` | B CAN peripheral / 핀 예약 | FDCAN2, PB5 RX / PB6 TX (설계 배정; 외부 통신 검증 미완료) | FROZEN |
| `DEC-HW-023` | B Display / Touch / 외부 메모리 역할 | LTDC RGB888 / BSP I2C4 touch / OCTOSPI1 NOR asset @ 0x90000000 / OCTOSPI2 HyperRAM framebuffer @ 0x70000000 | FROZEN |

동결 범위:
- G431KB는 **MCU 모델 선택** 동결이다. NUCLEO 정품 여부, 제조사/보드 revision/실장 MCU 전체 품번은 구매 실물과 회로도로 기록한다. NUCLEO-G431KB 핀맵을 다른 G431KB 보드에 자동 적용하지 않는다.
- 모델 선택은 자원·배선 적합성 시험 PASS를 뜻하지 않는다. G431KB의 역할별 Pin/Timer/ADC/UART/SPI/FDCAN 배정, 메모리 예산과 실기 검증은 Gate A/D에 남는다.
- B의 핀 예약은 현재 `.ioc` 및 [PIN_MAP](../ecus/IVI/PIN_MAP.md) 기준이다. Internal loopback은 PB5/PB6의 외부 전기 경로를 검증하지 않는다.
- `DEC-HW-023`은 메모리 역할/주소 기반 동결이다. 전체 MPU/cache 설정, 모든 clock/timing, GUI frame budget, RTOS stack/queue 수치는 동결하지 않는다.

## 3.2 Network Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-NET-001` | CAN nominal bitrate | 500 kbps | FROZEN |
| `DEC-NET-002` | CAN FD data bitrate | 미사용 — Classic CAN 프레임 고정(BRS 비활성), 현재 STM32 FDCAN 초기화(`FDCAN_FRAME_CLASSIC`)와 일치. 향후 필요 시 `DEC-HW-006`(TJA1051, ~2Mbps)이 지원 | FROZEN |
| `DEC-NET-003` | CAN sample / mode parameters | Sample point 87.5%, SJW=1(CiA 301 권장값), Classic CAN Non-FD 모드. 실제 Prescaler/TSEG1/TSEG2 레지스터값은 보드별 FDCAN 커널 클럭 기준 계산 필요(설계값이며 실기 계산/검증은 Gate B) | FROZEN |
| `DEC-NET-004` | Message ID allocation | 11-bit 표준 ID. Node ID: A=1/B=2/C=3/D_Gateway=4/D_Slave=5/F=6/E=7(논리 ID, 물리 송신은 B가 대행). 메시지 ID는 §4 각 표에 명시(0x100~0x1B7 대역) | FROZEN |
| `DEC-NET-005` | DLC / payload layout | Classic CAN 고정 DLC=8, 미사용 바이트는 0 패딩. Byte offset은 §4 각 표에 명시 | FROZEN |
| `DEC-NET-006` | Signal endian / signedness | Little-endian 고정. 부호 있는 정수는 2의 보수. bool=1byte(0/1), enum=1byte | FROZEN |
| `DEC-NET-007` | Message cycle / timeout | §4 각 메시지 표에 cycle/timeout 명시 (BENCH 초기값 — Gate D 실측 후 재검증 대상, §7.1 참고) | FROZEN |
| `DEC-NET-008` | Heartbeat node ID 방식 | `DEC-NET-004`와 동일 Node ID enum 사용(A=1~F=6). E는 CAN `ECU_Heartbeat`를 직접 발행하지 않음 — B가 `uart_bridge_service`의 UART 링크 liveness로 E 상태를 판단하고 필요 시 자체 `DTC_Event`(source_node=B)로 보고 | FROZEN |
| `DEC-NET-009` | LIN bitrate | 19200 bps | FROZEN |
| `DEC-NET-010` | LIN frame ID | `Lamp_Command`=0x10, `Lamp_Status`=0x11, `Lamp_Diagnostic`=0x3C(LIN 표준 진단 프레임 ID) | FROZEN |
| `DEC-NET-011` | LIN checksum | Enhanced Checksum (LIN 2.x 표준, `DEC-HW-007` LIN 2.1 트랜시버와 일치) | FROZEN |
| `DEC-NET-012` | LIN schedule / slot period | 10ms slot 기준. `Lamp_Command`/`Lamp_Status` 20ms cycle(2 slot), `Lamp_Diagnostic`은 on-demand slot | FROZEN |

## 3.3 Vehicle / Control Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-CTRL-001` | Vehicle State Machine | `INIT → READY → ACTIVE`(Drive Enable), 어느 상태에서든 `FAULT`로 override 가능. `INIT→READY`: 부팅 완료+필수 heartbeat 정상. `READY↔ACTIVE`: `DEC-CTRL-003` 조건. `Any→FAULT`: Critical DTC. `FAULT→READY`: fault 해소 확인(자동복귀 없음) | FROZEN |
| `DEC-CTRL-002` | READY 조건 | 부팅 완료 + 필수 Node(A/C/D/F) heartbeat 정상 + Critical DTC 없음 | FROZEN |
| `DEC-CTRL-003` | Drive Enable 조건 | READY 상태 + `Driver_Input.request_valid=true` + gear ≠ P | FROZEN |
| `DEC-CTRL-004` | D↔R 전환 허용 조건 | 방식은 결정: 추정 speed가 threshold 이하로 일정 시간 유지된 정지 상태에서만 전환 허용(움직이는 상태에서 즉시 반전 금지, §1.2). 정확한 threshold/유지시간은 `DEC-CTRL-005`와 함께 미정(bench 필요, 추정치라 여유 마진 필요) — 수치 미확정으로 Status는 OPEN 유지 | OPEN |
| `DEC-CTRL-005` | Stop speed threshold | OWNER INPUT (bench 필요 — 추정 speed 기반이라 실측 정지와 일치하지 않을 수 있어 여유 마진 확보 후 결정) | OPEN |
| `DEC-CTRL-006` | E-Stop recovery 정책 | 미사용 — E-Stop 기능 전체 삭제 (`DEC-HW-020`과 동일 사유) | REMOVED |
| `DEC-CTRL-007` | Accelerator calibration/deadband | OWNER INPUT (RF raw range 실측 필요) | OPEN |
| `DEC-CTRL-008` | Brake calibration/deadband | OWNER INPUT (RF raw range 실측 필요) | OPEN |
| `DEC-CTRL-009` | Brake-over-Accelerator threshold | 정책은 결정(§1.2와 일치: brake가 deadband 초과 시 accel 무시). deadband 수치는 `DEC-CTRL-008`과 함께 미정 — Status는 OPEN 유지 | OPEN |
| `DEC-CTRL-010` | Steering input range/calibration | OWNER INPUT (RF raw range 실측 필요) | OPEN |
| `DEC-CTRL-011` | Ultrasonic SAFE/WARNING/CRITICAL action | 정책은 결정: SAFE=조치 없음, WARNING=HMI 표시만(속도 제한 없음), CRITICAL=F가 감속/정지 명령 생성 후 C가 출력. threshold/hysteresis 수치는 `DEC-PER-003`에서 별도 결정 — Status는 OPEN 유지 | OPEN |
| `DEC-CTRL-012` | ADAS arbitration rule | §1.2와 동일: Critical Fault > Ultrasonic Critical > ADAS > Driver Request. ADAS는 Ultrasonic Critical을 override하지 않는다 | FROZEN |
| `DEC-CTRL-013` | Final speed unit/range | uint8 %, 0(정지)~100(최대 속도) **크기(magnitude)만** — 전/후진 방향은 `Final_Drive_Command.gear`(P/R/N/D)가 이미 갖고 있어서 speed로 또 표현하지 않는다(2026-09-17 사용자 결정, gear 필드와의 중복 제거) | FROZEN |
| `DEC-CTRL-014` | Final steering unit/range | 부호 있는 정수 %, -100(최대 좌)~100(최대 우), 0=중립 | FROZEN |
| `DEC-CTRL-015` | Drive command timeout | 100ms (`Final_Drive_Command` cycle 20ms의 5배, `DEC-NET-007` 참고) | FROZEN |
| `DEC-CTRL-016` | Steering timeout action | timeout 시 마지막 값 유지하지 않고 중립(0)으로 복귀 | FROZEN |
| `DEC-CTRL-017` | Encoder invalid fallback | 미사용 — Encoder 삭제 | REMOVED |
| `DEC-CTRL-018` | PID 적용 여부 / tuning policy | 1차 구현은 PID 미적용(open-loop 명령을 그대로 PWM/servo 출력) — 실측 피드백이 없어 폐루프 근거 부족. 필요 시 추정치 기반 제한적 PID는 2단계에서 별도 결정 | FROZEN (1차 정책) |
| `DEC-CTRL-019` | RF 속도 요청→accel/brake 또는 speed_request 계약 및 조향 매핑 | 방식은 결정(2026-09-17 수정): RF `speed_request`는 **0~100 크기(magnitude)만** 사용 — 전/후진 방향은 RF `gear`(P/R/N/D)가 이미 담당하므로 speed로 또 인코딩하지 않는다(부호 없음, `DEC-CTRL-013`과 동일 원칙). `accel`=`speed_request`(그대로 사용, 파생 없음). `brake`는 speed_request에서 파생하지 않으며 별도 입력 채널 여부가 `DEC-HW-018`(OPEN)에서 결정된다. `steering`은 raw 대비 선형 매핑(부호 있음, 좌/우는 speed와 무관). raw→request 변환 계수는 미정(실측 필요) — Status는 OPEN 유지 | OPEN |
| `DEC-CTRL-020` | Brake 입력 감속 감지 → `brake_lamp` 자동 점등 threshold | 방식은 결정: `accel`(구 speed_request)이 이전 대비 일정 비율/양 이상 감소하면 점등, 또는 `DEC-HW-018`에서 별도 brake 채널이 확정되면 그 값을 직접 사용. 정확한 threshold는 미정 — Status는 OPEN 유지 | OPEN |
| `DEC-CTRL-021` | Motor 명령값→speed/rpm 표시값 추정 함수 (실측 아님을 UI에 명시) | 형태는 결정: 명령값(accel/speed_request 또는 PWM duty, 항상 0~100 크기)에 선형 비례하는 1차 함수로 시작, 실측 대비 오차가 크면 보정 테이블로 전환 검토. 계수는 미정 — Status는 OPEN 유지 | OPEN |

## 3.4 Perception / Vision Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-PER-001` | Ultrasonic zone / 장착 위치 | 전좌(FL) / 전우(FR) / 후좌(RL) / 후우(RR) 4개 고정 | FROZEN |
| `DEC-PER-002` | Ultrasonic filter | 방식은 결정: median-of-3 또는 3-sample 이동평균. window/계수는 실측 필요 — Status는 OPEN 유지 | OPEN |
| `DEC-PER-003` | Ultrasonic threshold / hysteresis | OWNER INPUT (HC-SR04 실측 필요; 센서 스펙상 유효 range는 2cm~400cm, `DEC-HW-009` 참고) | OPEN |
| `DEC-PER-004` | Ultrasonic scan period / gap | 4개 센서 순차 스캔, 센서당 측정 주기 60ms(HC-SR04 안정 재측정 권장 간격) × 4 ≈ 240ms/round, crosstalk 방지를 위해 센서 간 최소 gap 확보 | FROZEN |
| `DEC-VIS-001` | Front Vision Stage 기능 | YOLO 계열(COCO 80 class) 객체 탐지 → 가장 근접/중요한 객체의 class+zone(LEFT/CENTER/RIGHT) 산출 → `Vision_Status`/`ADAS_Request` 갱신 | FROZEN |
| `DEC-VIS-002` | Rear Vision Stage 기능 | 미사용 — Rear Vision 삭제 | REMOVED |
| `DEC-VIS-003` | Camera resolution / FPS | 1920x1080(Full HD, `DEC-HW-015` AU1425 캡처 해상도 그대로 사용). 추론 FPS는 모델/Pi 성능에 따라 가변(추론 입력은 리사이즈 가능, 캡처 해상도는 유지) | FROZEN |
| `DEC-VIS-004` | Vision algorithm/model | 경량 YOLO 계열(예: YOLOv8n) COCO pretrained 모델을 기본으로 시작. 정확한 모델/버전/커스텀 학습 여부는 Pi 성능 확인 후 결정 | OPEN |
| `DEC-VIS-005` | Vision freshness timeout | 500ms (`Vision_Status` cycle 100ms의 5배, `DEC-NET-007` 참고 — 카메라 추론 특성상 여유 반영) | FROZEN |
| `DEC-VIS-006` | Gear D/R camera switching | 미사용 — 카메라는 전방 1대 상시 동작 | REMOVED |
| `DEC-VIS-007` | Gear R → first valid result latency | 미사용 — Rear Vision 삭제 | REMOVED |
| `DEC-VIS-008` | 객체 class(COCO)/방향(zone: 좌/중/우 등) 표현 방식 | `detected_class`=COCO 80 class index(uint8, 0~79), `direction/zone`=enum{LEFT=0, CENTER=1, RIGHT=2}(uint8) | FROZEN |

## 3.5 HMI / Body Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-HMI-001` | Cluster 필수 표시항목 | 추정 속도, 기어, 전원/배터리 상태, 4방향 충돌주의 요약, Active DTC 목록, 턴시그널/헤드램프 상태. 레이아웃/배치는 OPEN | OPEN |
| `DEC-HMI-002` | Warning 표시 우선순위 | Critical DTC > Ultrasonic Critical > ADAS 경고 > 일반 상태 표시 (§1.2와 일치) | FROZEN |
| `DEC-HMI-003` | 전방 객체 알림 및 4방향 충돌주의 overlay/panel 상세 정책 | 기본 화면에 4방향 요약 아이콘 상시 표시, 상세 패널은 터치로 열람. CRITICAL은 패널을 닫아도 기본 화면에 표시 유지(§1.3 기존 규칙과 일치). 세부 UI 디자인은 OPEN | OPEN |
| `DEC-HMI-004` | DTC Clear 구현 여부 | 미구현 — 현재 범위에 포함하지 않는다 (§4.10 실시간 fault 표시 계약과 일치, IVI 수동 Clear 없음) | FROZEN |
| `DEC-HMI-005` | TouchGFX update/frame budget | OWNER INPUT (bench 계측 필요) | OPEN |
| `DEC-BODY-001` | 구현 Lamp 범위 | 좌/우 턴시그널 + 헤드램프(밝기 가변) + 브레이크등 | FROZEN |
| `DEC-BODY-002` | Body_User_Request 기능 범위 | 좌/우 턴시그널 요청 + 헤드램프 밝기 요청 (brake_lamp는 사용자 요청이 아니라 F가 감속 감지로 자동 생성) | FROZEN |
| `DEC-BODY-003` | Ambient 단위/filter/calibration | 미사용 — Ambient 기능 삭제 | REMOVED |

## 3.6 DTC / Health Freeze

| ID | Decision | Final Value | Status |
|---|---|---|---|
| `DEC-DTC-000` | DTC History DB / 저장 여부 | 미사용 — Pi DTC Manager/History DB 삭제. OBD2/외부 진단 커넥터가 없어 이력 조회 실효성이 낮으므로, `DTC_Event`는 B(IVI)가 실시간(Active만) 구독·표시하고 지속 저장하지 않는다 | REMOVED |
| `DEC-DTC-001` | DTC code numbering | 형식: `[Node ID 1byte][Code 1byte]`. Node ID는 `DEC-NET-004/008`과 동일(A=1~F=6). Code는 Node별로 0x01부터 순차 정의, 개별 코드 목록은 각 역할 ARCHITECTURE 문서에 나열 | FROZEN (형식) |
| `DEC-DTC-002` | DTC status enum (Active/Inactive 중심, History 상태 불필요) | `{INACTIVE=0, ACTIVE=1}` (uint8) | FROZEN |
| `DEC-DTC-003` | DTC severity enum | `{INFO=0, WARNING=1, CRITICAL=2}` (uint8) | FROZEN |
| `DEC-DTC-004` | fault 확정 / 해소 판정 규칙 (소스 ECU) | 방식은 결정: 조건이 N회 연속 충족되면 확정, N회 연속 미충족이면 해소(debounce). 정확한 연속횟수/판정주기는 bench 필요 — Status는 OPEN 유지 | OPEN |
| `DEC-DTC-005` | Critical DTC → Safe Action mapping | CRITICAL DTC는 자동으로 §1.2 안전 우선순위에 따라 F가 개입(감속/정지 등). 코드별 구체 매핑 표는 OPEN | OPEN |
| `DEC-HLT-001` | Heartbeat period / timeout | 100ms cycle, 3회 연속 누락(300ms) 시 timeout (`DEC-NET-008`과 일치) | FROZEN |
| `DEC-HLT-002` | Watchdog refresh condition | 모든 Critical Task가 자기 주기 내 정상 실행 완료를 보고해야 refresh. 하나라도 실패 시 refresh 보류(reset 유도). Task별 구체 조건은 OPEN | OPEN |
| `DEC-HLT-003` | RTOS task priority / period / stack / queue depth | OWNER INPUT (Gate D 실측 필요 — TEST_REPORT.md 각 ECU 표에서 실측 후 확정) | OPEN |

---

# 4. Final Logical Message Contracts

> 실제 CAN ID / bit position은 Owner Freeze 후 이 표에 직접 기록한다. 하위 문서는 이 표를 복제해 새 값을 만들지 않는다.

## 4.1 `Ultrasonic_Status`

> 4방향 고정: `FL`(전좌) / `FR`(전우) / `RL`(후좌) / `RR`(후우). zone별로 독립된 CAN 프레임 4개를 사용한다(단일 프레임에 4 zone을 욱여넣지 않음).

**CAN ID:** `0x120`+zone_index(FL=0,FR=1,RL=2,RR=3) → `0x120`~`0x123` · **DLC:** 8 · **Cycle:** 50ms · **Timeout:** 250ms (`DEC-NET-004/007`, BENCH 초기값)

| Field | Unit / Type | Range | Valid Rule | Final | Byte |
|---|---|---|---|---|---|
| zone id | enum(uint8) | FL=0/FR=1/RL=2/RR=3 | defined ID | FROZEN | 0 |
| distance | mm(uint16 LE) | 20~4000 (HC-SR04 스펙, `DEC-HW-009`), 0xFFFF=무응답/invalid | `valid=true` | FROZEN | 1-2 |
| valid | bool | 0/1 | source health | FROZEN | 3 |
| warning_level | enum(uint8) | SAFE=0/WARNING=1/CRITICAL=2 | valid only | FROZEN (enum) / OPEN (threshold, `DEC-PER-003`) | 4 |
| fault_flags | bitfield(uint8) | TBD | local fault | OPEN | 5 |
| sequence | uint8 | 0~255 wraparound | monotonic | FROZEN | 6 |
| (reserved) | — | 0 고정 | — | FROZEN | 7 |

## 4.2 `Vision_Status`

> Rear Vision/주차 관련 필드 삭제 (2026-09-15). 전방 카메라 1대의 COCO 기반 객체인식 결과만 전달한다. Raw image는 CAN payload로 보내지 않는다 (§8 E HPC 참고). E가 UART로 B에 보내고 B가 아래 프레임으로 CAN 발행한다(§1.1 예외, `DEC-HW-030`).

**CAN ID:** `0x160` (B가 송신) · **DLC:** 8 · **Cycle:** 100ms · **Timeout:** 500ms (`DEC-VIS-005`)

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| front_valid | bool | FROZEN | 0 |
| detected_class | uint8 (COCO class index 0~79) | FROZEN | 1 |
| direction/zone | enum(uint8) LEFT=0/CENTER=1/RIGHT=2 | FROZEN | 2 |
| vision_warning | enum(uint8) NONE=0/CAUTION=1/WARNING=2 | FROZEN (enum) / OPEN (판정 기준) | 3 |
| sequence | uint8 (0~255 wraparound, E가 채번) | FROZEN | 4 |
| (reserved) | — 0 고정 | FROZEN | 5-7 |

## 4.3 `ADAS_Request`

> 범위: 전방 객체 감지에 따른 회피/감속 요청만 담당한다. 초음파 위험도는 A가 `Ultrasonic_Status`로 F에 직접 제공한다. E는 A의 위험도를 재판정하거나 해제하지 않으며 §1.2 우선순위를 따른다. E가 UART로 B에 보내고 B가 CAN 발행한다(§1.1 예외).

**CAN ID:** `0x150` (B가 송신) · **DLC:** 8 · **Cycle:** 50ms · **Timeout:** 250ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| request_valid | bool | FROZEN | 0 |
| requested_speed | uint8, % (0~100 크기만, `DEC-CTRL-013`과 동일 원칙 — 전방 회피 요청은 항상 감속 방향이라 부호 불필요) | FROZEN | 1 |
| requested_steering | int8, % (-100~100, `DEC-CTRL-014`와 동일 스케일) | FROZEN | 2 |
| request_reason/type | enum(uint8) NONE=0/OBJECT_AVOID=1 (전방 객체 회피 사유만; 주차 사유 없음) | FROZEN | 3 |
| sequence | uint8 (0~255 wraparound, E가 채번) | FROZEN | 4 |
| (reserved) | — 0 고정 | FROZEN | 5-7 |

## 4.4 `Final_Drive_Command`

**CAN ID:** `0x100` (가장 높은 arbitration 우선순위) · **DLC:** 8 · **Cycle:** 20ms · **Timeout:** 100ms (`DEC-CTRL-015`)

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| drive_enable | bool | FROZEN | 0 |
| speed_request | uint8, % (0~100 크기만, `DEC-CTRL-013` — 방향은 같은 표의 `gear` 필드) | FROZEN | 1 |
| steering_request | int8, % (-100~100, `DEC-CTRL-014`) | FROZEN | 2 |
| gear/direction | enum(uint8) P=0/R=1/N=2/D=3 | FROZEN | 3 |
| command_valid | bool | FROZEN | 4 |
| sequence | uint8 (0~255 wraparound) | FROZEN | 5 |
| (reserved) | — 0 고정 | FROZEN | 6-7 |

## 4.5 `Drive_Status`

> `motor_rpm`/`vehicle_speed`는 실측 센서(Encoder/Hall)가 아니라 **모터 명령값(PWM 등) 기반 추정 함수의 결과**다. IVI/Cluster 표시 시 이 필드가 추정값임을 UI에서 구분한다 (`DEC-CTRL-021`).

**CAN ID:** `0x130` · **DLC:** 8 · **Cycle:** 50ms · **Timeout:** 250ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| motor_rpm (estimated) | int16 LE, rpm | FROZEN (type) / OPEN (추정 계수, `DEC-CTRL-021`) | 0-1 |
| vehicle_speed (estimated) | uint8, % (0~100 크기만 — 방향은 `Vehicle_State.gear`로 별도 확인) | FROZEN (type) / OPEN (추정 계수) | 2 |
| steering_target | int8, % (-100~100) | FROZEN | 3 |
| command_valid | bool | FROZEN | 4 |
| fault_flags | bitfield(uint8) | OPEN | 5 |
| (reserved) | — 0 고정 | FROZEN | 6-7 |

명령값 기반 추정 speed/RPM은 실제 정지·감속의 측정값이 아니다. D↔R 허용을 추정 speed=0만으로 확정하지 않는다. 실측 피드백이 없는 구성의 정지 확인/전환 대기/복구 기준은 `DEC-CTRL-004/005`에서 bench 근거와 함께 결정한다. brake_lamp 판정도 실제 감속으로 단정하지 않고 유효한 brake 입력 및 명령 감속 기반 정책을 `DEC-CTRL-020`에서 구체화한다.

## 4.6 `Body_User_Request`

> 범위: 좌/우 턴시그널 요청 + 헤드램프 밝기 요청만 (`DEC-BODY-002`). `brake_lamp`는 사용자 요청 대상이 아니다 — F가 감속을 감지해 자동으로 `Body_Command.brake_lamp`를 생성한다 (`DEC-CTRL-020`).

**CAN ID:** `0x180` · **DLC:** 8 · **Cycle:** 100ms(요청 발생 시 즉시 송신 가능) · **Timeout:** 500ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| request_type | enum(uint8) TURN_LEFT=0/TURN_RIGHT=1/HEADLAMP_BRIGHTNESS=2 | FROZEN | 0 |
| requested_value | uint8 (턴시그널: 0/1, 헤드램프: 0~100%) | FROZEN | 1 |
| request_valid | bool | FROZEN | 2 |
| (reserved) | — 0 고정 | FROZEN | 3-7 |

## 4.7 `Body_Command`

> `hazard`, `tail_lamp` 필드 삭제 (요구 범위 밖). `headlamp`는 on/off가 아니라 밝기값이다. `brake_lamp`는 F가 자동 생성한다.

**CAN ID:** `0x170` · **DLC:** 8 · **Cycle:** 100ms · **Timeout:** 500ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| headlamp_brightness | uint8, 0~100% | FROZEN | 0 |
| brake_lamp | bool (F가 감속 감지로 자동 설정) | FROZEN | 1 |
| turn_left | bool | FROZEN | 2 |
| turn_right | bool | FROZEN | 3 |
| (reserved) | — 0 고정 | FROZEN | 4-7 |

## 4.8 `Body_Status`

> `ambient` 필드 삭제 (Ambient 기능 삭제).

**CAN ID:** `0x190` · **DLC:** 8 · **Cycle:** 100ms · **Timeout:** 500ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| lamp_status | bitfield(uint8): bit0=headlamp, bit1=turn_left, bit2=turn_right, bit3=brake_lamp | FROZEN | 0 |
| lin_health | enum/flags(uint8) | OPEN | 1 |
| fault_flags | bitfield(uint8) | OPEN | 2 |
| (reserved) | — 0 고정 | FROZEN | 3-7 |

## 4.8.1 `Driver_Input`

### 2026-09-17 RF 입력 변경 계약

사용자가 RF 모델을 `nRF24L01`로 확인했다. `DEC-HW-024`는 nRF24L01/SPI 선택만 FROZEN이며, 모듈 보드·배선·무선 설정·패킷은 `DEC-HW-029`에 OPEN으로 분리한다. 송신측 MCU/펌웨어와 RF 주소·채널·data rate·CRC·payload 규격을 맞춘다. [C 시작 가이드](../ecus/Motor_Steering_Control/DRIVER_INPUT_START.md)의 SPI 절차를 따른다.

- 사용자 요청으로 C의 기어·조향·속도 요청 소스를 RF로 통일한다. 별도 로컬 Gear GPIO/가변저항은 현재 기본 구성에서 제외한다.
- **속도는 우선 목표 속도/스로틀 요청이라는 작업 가정**이며 실제 속도 센서값으로 확정한 것이 아니다. `Drive_Status.vehicle_speed` 및 `motor_rpm`의 명령값 기반 추정 의미는 그대로다.
- 아래 기존 CAN 필드 목록은 유지하되, RF 속도 요청을 `accel/brake`로 변환할지 `speed_request` 필드를 추가할지는 `DEC-CTRL-019`에서 C/F가 결정한다. 단일 속도 요청만으로 독립된 브레이크 입력을 받았다고 가정하지 않는다. 결정 전 새 CAN 필드/단위/payload를 통합 상수로 사용하지 않는다.
- `gear`는 운전자 요청이다. 실제 기어/방향은 F의 중재를 거친 `Final_Drive_Command`와 `Vehicle_State`를 따른다. RF 요청을 모터 PWM/DIR/서보에 바로 연결하지 않는다.
- 기어·조향·속도 요청 중 하나라도 누락/범위 오류/오래된 값이거나 수신기가 RF failsafe를 표시하면 전체 요청을 invalid로 취급한다. 부팅 미수신도 invalid다. 새 유효 수신/샘플로만 freshness를 갱신하고, 같은 값으로 계속 조작 중인 상태를 단순 값 불변만으로 timeout 처리하지 않는다.
- RF timeout과 VCU command timeout은 별도 감시한다. invalid를 F에 전달하고 안전 정책에 따라 처리하며, 마지막 유효 입력을 정상으로 계속 재발행하지 않는다. nRF24L01 수신이 멈춰도 이전 payload가 RAM에 남을 수 있으므로 새 유효 패킷 기준 timeout을 송신기 OFF 시험으로 검증한다. timeout 수치와 출력 안전 동작/복구 조건은 OPEN이다.
- 변경 근거: 2026-09-17 사용자 C 담당 계획 및 로컬 `Driver_Input.ioc`/`Core/Src/main.c` 확인. `DEC-HW-026`의 소스를 RF로 변경하고 owner C는 유지한다. 사용자 추가 확인에 따라 nRF24L01/SPI 선택을 동결했다. 모듈 보드/무선 설정/패킷/핀/단위는 동결하지 않았다. 새 실기 시험은 NOT RUN이다.
- 영향/재시험: C 수신·매핑·두절, F 입력 validity·기어·속도 중재, B 요청/추정 속도 구분 및 C↔F CAN encode/decode. 시작 절차는 [C Driver_Input 가이드](../ecus/Motor_Steering_Control/DRIVER_INPUT_START.md)를 따른다.

> Publisher는 C다 (2026-09-15부터 accel/brake/steering + gear까지 포함, §7.1에서 지적된 누락 계약을 채움). C는 RF로 기어·조향·속도 요청을 수신해 하나의 `Driver_Input` 메시지로 CAN 발행한다.

**CAN ID:** `0x110` · **DLC:** 8 · **Cycle:** 20ms · **Timeout:** 100ms (`DEC-NET-007`, `Final_Drive_Command`와 동일 등급)

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| accel | uint8, 0~100 (RF `speed_request`를 그대로 사용, `DEC-CTRL-019`) | FROZEN (type) / OPEN (매핑 계수) | 0 |
| brake | uint8, 0~100 (speed_request에서 파생하지 않음 — 별도 입력 채널 여부는 `DEC-HW-018` OPEN) | FROZEN (type) / OPEN (입력 소스) | 1 |
| steering | int8, % (-100~100, `DEC-CTRL-019` 선형 매핑) | FROZEN (type) / OPEN (매핑 계수) | 2 |
| gear | enum(uint8) P=0/R=1/N=2/D=3 (`Final_Drive_Command`와 동일 enum) | FROZEN | 3 |
| request_valid | bool | FROZEN | 4 |
| sequence | uint8 (0~255 wraparound) | FROZEN | 5 |
| (reserved) | — 0 고정 | FROZEN | 6-7 |

`Driver_Input`의 invalid/stale/timeout을 F가 유효한 Gear로 간주해서는 안 된다. 안전 상태는 `DEC-CTRL-004/005`와 freshness 계약에 따라 처리한다(E-Stop 관련 조항은 2026-09-17 삭제됨). Enable/STBY 비활성 극성과 실제 핀은 선택된 드라이버/회로 기준으로 확정한다.

## 4.9 `Vehicle_State`

**CAN ID:** `0x140` · **DLC:** 8 · **Cycle:** 100ms(상태 변경 시 즉시 송신 가능) · **Timeout:** 500ms

| Field | Unit / Type | Final | Byte |
|---|---|---|---|
| gear | enum(uint8) P=0/R=1/N=2/D=3 | FROZEN | 0 |
| ready | bool | FROZEN | 1 |
| vehicle_mode | enum(uint8) INIT=0/READY=1/ACTIVE=2/FAULT=3 (`DEC-CTRL-001` 상태와 동일) | FROZEN | 2 |
| safety_state | enum(uint8) NORMAL=0/WARNING=1/CRITICAL=2 | FROZEN | 3 |
| (reserved) | — 0 고정 | FROZEN | 4-7 |

## 4.10 `DTC_Event`

> Pi DTC History DB는 삭제됐다 (`DEC-DTC-000` REMOVED). `DTC_Event`는 B(IVI)가 실시간으로 구독·표시하며 지속 저장하지 않는다 — 소스 Node의 fault가 해소되면 해당 이벤트도 화면에서 사라진다(Active만 표시).

**CAN ID:** `0x1A0`+node_id(송신 Node 기준, A=0x1A1/B=0x1A2/C=0x1A3/D_Gateway=0x1A4/D_Slave=0x1A5/F=0x1A6) · **DLC:** 8 · **Cycle:** 변경 시 즉시 + 1000ms keepalive snapshot 재전송 · **Timeout:** 3000ms(keepalive 3회 누락)

> E(source_node=7)는 CAN을 직접 쓰지 않으므로 B가 자신의 슬롯(0x1A2)으로 대신 송신할 수 있다 — 이때도 payload의 `source_node`는 실제 논리적 발생지(E 자신의 fault면 7, B 자신의 UART 링크 감시 fault면 2)를 정확히 담는다.

| Field | Type | Final | Byte |
|---|---|---|---|
| source_node | enum(uint8) A=1/B=2/C=3/D_Gateway=4/D_Slave=5/F=6/E=7 | FROZEN | 0 |
| code | uint8 (`DEC-DTC-001` 형식의 Code 바이트만; Node ID는 source_node 필드와 중복이라 생략) | FROZEN (형식) / OPEN (개별 코드 목록) | 1 |
| status | enum(uint8) INACTIVE=0/ACTIVE=1 (`DEC-DTC-002`) | FROZEN | 2 |
| severity | enum(uint8) INFO=0/WARNING=1/CRITICAL=2 (`DEC-DTC-003`) | FROZEN | 3 |
| sequence | uint8 (0~255 wraparound) | FROZEN | 4 |
| (reserved) | — 0 고정 | FROZEN | 5-7 |

### 실시간 fault 표시 계약

- 각 ECU가 자기 fault의 Active/Inactive를 판정하고 `DTC_Event` 또는 합의된 fault flag로 직접 발행한다. B는 `(source_node, code)`별 현재 상태를 RAM에만 유지한다. severity는 현재 표시/안전 판단에 사용하며, 이력 저장 삭제와 severity 필드 삭제를 혼동하지 않는다.
- 소스의 유효한 Inactive 통보 또는 해당 fault가 해소되었음을 나타내는 최신 상태를 받으면 B는 Active 목록에서 제거한다. 이벤트 미수신만으로 정상 복귀를 추정하지 않는다. IVI 수동 DTC Clear 요청은 현재 범위에 포함하지 않는다.
- IVI 재시작/재접속, Active 또는 Inactive 프레임 누락 후에도 현재 상태를 회복할 수 있도록 소스의 현재 fault 상태 재전송/주기 snapshot 계약이 필요하다. 방식·주기·timeout·최대 항목 수·sequence 처리 규칙은 `DEC-DTC-002/004`, `DEC-NET-004~007`, `DEC-HLT-001`에서 OWNER INPUT으로 확정한다. 과거 이벤트 재생은 하지 않는다.
- 소스 heartbeat/fault 상태가 stale이면 통신 두절/상태 미확인으로 구분하고, 이전 값을 현재 Active 또는 정상으로 확정 표시하지 않는다. 재접속 후에는 최신 유효 상태로 갱신한다.
- 화면을 보지 않는 동안 발생했다가 해소된 간헐적 fault는 나중에 확인할 수 없다. 이는 사용자가 수용한 범위 제한이다. fault 확정 threshold는 별도이며, CAN 프레임 한 번 누락을 반드시 DTC로 확정한다는 뜻은 아니다.

## 4.11 `ECU_Heartbeat`

**CAN ID:** `0x1B0`+node_id (A=0x1B1/B=0x1B2/C=0x1B3/D_Gateway=0x1B4/D_Slave=0x1B5/F=0x1B6) · **DLC:** 8 · **Cycle:** 100ms · **Timeout:** 300ms(3회 연속 누락, `DEC-HLT-001`)

> E는 CAN heartbeat를 직접 보내지 않는다 — B가 `uart_bridge_service` 링크 liveness로 E 상태를 판단한다(`DEC-NET-008`).

| Field | Type | Final | Byte |
|---|---|---|---|
| node_id | enum(uint8) A=1/B=2/C=3/D_Gateway=4/D_Slave=5/F=6 | FROZEN | 0 |
| alive_counter | uint8 (0~255 wraparound) | FROZEN | 1 |
| health_flags | bitfield(uint8) | OPEN | 2 |
| (reserved) | — 0 고정 | FROZEN | 3-7 |

---

# 5. LIN Contract

## 5.1 Frame Set

| LIN Frame | Publisher | Subscriber | Period | ID | Checksum |
|---|---|---|---|---|---|
| `Lamp_Command` | Gateway | Slave | 20ms | 0x10 | Enhanced |
| `Lamp_Status` | Slave | Gateway | 20ms | 0x11 | Enhanced |
| `Lamp_Diagnostic` | Slave | Gateway | On-demand | 0x3C | Enhanced |

> `Ambient_Status` 프레임 삭제 (Ambient 기능 삭제, 2026-09-15).

## 5.2 CAN ↔ LIN Mapping

| CAN Signal | LIN Signal | Mapping / Scale | Final |
|---|---|---|---|
| `Body_Command.headlamp_brightness` | `Lamp_Command` byte0 | 1:1, uint8 0~100% 그대로 전달 | FROZEN |
| `Body_Command.turn_left/turn_right` | `Lamp_Command` byte1 bit0/bit1 | bool → bit, 1:1 | FROZEN |
| `Body_Command.brake_lamp` | `Lamp_Command` byte1 bit2 | bool → bit, 1:1 | FROZEN |
| `Lamp_Status` byte0(bit0=head/bit1=turn_left/bit2=turn_right/bit3=brake) | `Body_Status.lamp_status` | 1:1 bitfield 그대로 CAN에 반영 | FROZEN |
| `Lamp_Status` byte1 | `Body_Status.fault_flags` | 1:1 | OPEN (fault_flags 정의 자체가 OPEN) |

---

# 6. RTOS / Linux Final Execution Contract

각 MCU 역할은 최종 코드 작성 전에 아래 표의 `Final Period / Priority / Stack / Queue`를 Owner가 Freeze한다.

| Node | Critical Tasks |
|---|---|
| A | UltrasonicTask, PerceptionTask, CanTxTask, HealthTask |
| B | CanRxTask, VehicleModelTask, GuiTask, CommandTxTask, VisionRelayTask, HealthTask |
| C | CanRxTask, ControlTask, DriverInputTask, CanTxTask, StatusTask, HealthTask |
| D Gateway | CanRxTask, GatewayMappingTask, LinScheduleTask, CanTxTask, HealthTask |
| D Slave | LinRxTask, LightingTask, StatusTask, HealthTask |
| F | SafetyTask, VcuControlTask, CanRxTask, CanTxTask, DiagnosticTask, HealthTask |

> C에 `DriverInputTask` 추가 (RF 수신기 입력 읽기 + `Driver_Input` CAN 발행 — 입력 하드웨어가 실제로 C에 물리적으로 붙기 때문에 publisher를 F에서 C로 이전, §1.1 참고). C의 `FeedbackTask`(Encoder 기반)는 Encoder 삭제로 제거. F의 `DriverInputTask`는 publisher 이전에 따라 제거.
>
> **2026-09-17:** B에 `VisionRelayTask` 추가 — E가 UART로 보낸 `Vision_Status`/`ADAS_Request` raw 값을 그대로 CAN에 옮겨 발행한다(E는 CAN 인터페이스가 없음, `DEC-HW-008/030/031`). 이 Task는 값을 해석·재판단하지 않고 단순 relay만 하며, GUI 표시용 `VehicleModelTask` 로직과 분리한다. E의 `can_service`는 삭제되고 `uart_bridge_service`로 대체됐다(§6 Linux E Node 표 참고).
>
> **2026-09-15:** Gear GPIO도 F에서 C로 이전했다(당시 함께 이전한 E-Stop 관련 내용은 2026-09-17에 기능 자체가 삭제됐다). `DriverInputTask`가 gear를 `Driver_Input`에 실어 CAN 발행한다. F는 더 이상 Gear GPIO를 직접 읽지 않는다. F의 `DiagnosticTask`는 Pi DTC Manager 없이 `DTC_Event` 발행과 현재 fault/status 처리를 담당하며, 지속 이력 저장은 하지 않는다. B는 각 ECU의 이벤트를 직접 구독해 표시한다.

Linux E Node는 다음을 Freeze한다.

| Service | Owner Decision |
|---|---|
| front_vision | lifetime / mode / restart |
| vehicle_manager | state ownership |
| uart_bridge_service | E↔B UART 링크 single owner (2026-09-17, `DEC-HW-030/031`; CAN 없음) |
| health_monitor | timeout / restart condition |
| logger | storage / rotation / blocking policy |

Pi DTC Manager 서비스는 삭제됐다 (`DEC-DTC-000` REMOVED). Diagnostics history/storage는 어떤 Node도 소유하지 않는다.

`can_service`는 삭제됐다 (2026-09-17, `DEC-HW-008` REMOVED) — E는 CAN을 직접 쓰지 않는다. `Vision_Status`/`ADAS_Request`는 `uart_bridge_service`가 B에 UART로 전달하고, B의 `VisionRelayTask`가 CAN으로 대신 발행한다.

---

# 7. Coding Gate

## Gate A: Hardware Ready

코드의 Hardware Layer를 확정하기 전에:
- [x] 모든 STM32 모델 확정 (2026-09-11: A/C/D Gateway/D Slave/F G431KB, B H735G-DK)
- [x] FDCAN 지원 확인 (2026-09-17: C의 FDCAN1 Internal Loopback PASS 확인, [TEST_REPORT.md](../ecus/Motor_Steering_Control/TEST_REPORT.md) 참고 — 다른 노드의 실물 bring-up과 외부 CAN 버스 검증은 별도)
- [x] CAN/LIN Transceiver 확정 (2026-09-17: `DEC-HW-006` TJA1051(T), `DEC-HW-007` LIN 2.1/SAE J2602 모듈 — 모델 선택만, 실물 회로도/배선 검증은 별도)
- [ ] Sensor/Actuator 모델 확정 (2026-09-17: Front Camera `DEC-HW-015`, Ultrasonic `DEC-HW-009` 확정; Motor/Motor Driver/Servo는 여전히 OPEN)
- [ ] Pin/Timer/ADC/UART/SPI/FDCAN peripheral 확정

## Gate B: Interface Freeze

ECU 간 통합 코드 작성 전에:
- [x] 모든 Logical Message field 확정 (2026-09-17, §4; 일부 calibration 계수/fault_flags 비트 정의는 여전히 OPEN)
- [x] CAN ID/DLC/bit layout 확정 (2026-09-17, `DEC-NET-004/005`, §4 각 표)
- [x] Unit/Scale/Offset/Range 확정 (2026-09-17, `DEC-NET-006`, §4; 실측 calibration 계수는 OPEN)
- [x] Cycle/Timeout 확정 (2026-09-17, `DEC-NET-007`, BENCH 초기값 — Gate D 실측 후 재검증)
- [x] Heartbeat/DTC 확정 (2026-09-17, `DEC-NET-008`, `DEC-DTC-001~003`, `DEC-HLT-001`)
- [x] LIN Schedule/Mapping 확정 (2026-09-17, `DEC-NET-009~012`, §5)

체크는 **설계값 확정**을 의미하며, §7.1의 물리 bitrate/mode 실기 검증(H735↔G431, LIN pair)은 별도로 남아있다.

## Gate C: Control Freeze

VCU/Drive 제어 코드 작성 전에:
- [x] Vehicle State Machine 확정 (2026-09-17, `DEC-CTRL-001`)
- [x] READY/Enable 조건 확정 (2026-09-17, `DEC-CTRL-002/003`)
- [x] Arbitration Rule 확정 (§1.2, `DEC-CTRL-012`)
- [ ] D↔R 조건 확정 (방식은 결정, threshold/유지시간 수치는 bench 필요 — `DEC-CTRL-004/005`)
- [x] Command Timeout/Safe State 확정 (2026-09-17, `DEC-CTRL-015/016`, BENCH 초기값)

## Gate D: RTOS Freeze

통합 빌드 전에:
- [ ] Task period / priority 확정
- [ ] Stack / Queue depth 확정
- [ ] Watchdog refresh condition 확정
- [ ] ISR→Task path 확정
- [ ] blocking logging 제거 확인

`Gate B`가 끝나기 전에는 담당자가 임의의 CAN ID나 bit position을 코드에 영구 상수로 박지 않는다.

## 7.1 단계별 동결 시점

동결은 전 항목을 한 번에 완료하는 행사가 아니다. **의존하는 구현을 확정하기 직전**, 해당 범위의 값과 검증 근거를 동결한다.
`FROZEN`은 선택한 설계 계약이며 `TEST PASS`와 별개다. Gate A~D는 현재 모두 미완료다.

| 시점 | 동결 범위 | 완료 조건 |
|---|---|---|
| 지금 | 구매 모델 + 검증된 B 기반 | 위 Hardware 8개 결정; 기존 §1/§2 역할·실행 원칙 유지 |
| Week 1, 역할별 Hardware Layer 확정 전 | Gate A: transceiver, sensor/actuator, pin/peripheral | 실제 보드/부품 회로도, 전압/정격, 핀 중복과 timer channel/AF 검토, 최소 bring-up 기록 |
| Week 1 말~Week 2 첫 ECU pair 통합 전 | Gate B: Network + 전체 message/LIN + DTC/Heartbeat 계약 | ID 중복 없음, field/단위/범위/invalid/enum/byte layout 완비, 양쪽 encode/decode 일치; 물리 bitrate/mode는 실제 H735↔G431 및 LIN pair 검증 |
| VCU/Drive 실제 출력 제어 확정 전 | Gate C: State/Enable/Arbitration/Timeout/Safe Action | 상태·전이·복구·stale/invalid 처리를 결정하고 수치 근거 및 bench 검증 기록; 먼저 mock/출력 비활성 시험 가능 |
| Week 2 계측 후~Week 3 통합 baseline 빌드 전 | Gate D: RTOS/Linux 자원·주기·watchdog | 대표 통신/GUI/제어 부하에서 period/jitter, stack high-water, queue 최대 점유/overflow, starvation, fault 경로 측정 |
| 관련 기능 통합 전, 늦어도 전체 baseline 직전 | Perception/Vision/HMI/Body 잔여 결정 | 실제 장착·보정·baseline 계측 후 수치와 기능 범위 결정, 시험의 Target/Expected에서 TBD 제거 |

주차별 일정은 [WEEKLY_PLAN](../getting_started/WEEKLY_PLAN.md)의 목표이며 달력 경과만으로 gate를 통과하지 않는다.
Bring-up, RTOS skeleton, mock, 계측용 bench 코드는 OPEN 값으로도 작성할 수 있다.
이때 값은 `BENCH ONLY / NOT FROZEN` 설정으로 분리하고 최종 통합 상수나 PASS 근거로 승격하지 않는다.
Gate D 이전의 계측용 통합 빌드는 허용하되 최종 통합 baseline으로 취급하지 않는다.

Gate B 추가 완결성 확인 (2026-09-17 갱신):
- §4.8.1 `Driver_Input`은 field/type/CAN ID/DLC/byte layout/cycle/timeout까지 FROZEN됐다. 남은 OPEN은 RF raw→accel/brake/steering 변환 계수(`DEC-CTRL-019`, 실측 필요)뿐이다.
- DTC/Heartbeat는 노드 식별·ID 할당까지 정의했다 (`DEC-NET-004/008`, §4.10/4.11).
- §6의 Task 이름 목록만으로 Gate D를 닫지 않는다. Node별 실제 Task/Period/Priority/Stack(bytes)/Queue(depth와 item bytes)/Watchdog 조건은 여전히 실측 필요(`DEC-HLT-003`).
- Gate B가 진짜로 닫히려면 위 설계값에 더해 §7.1의 물리 bitrate/mode 실기 검증(H735↔G431, LIN pair)이 PASS해야 한다 — 지금은 설계 동결 단계다.

---

# 8. 역할별 코드 작성 기준

## A Ultrasonic

```text
Driver → Measurement → Perception → Repository → CAN
```

코드 전에 `DEC-HW-009`, `DEC-PER-001~004`, `Ultrasonic_Status`가 FROZEN이어야 한다.

## B IVI

```text
CAN → Repository → TouchGFX
Touch → Body_User_Request
DTC_Event(CAN) → 실시간 구독 → Diagnostics 화면 (Active only, History 없음)
E UART(Vision_Status/ADAS_Request raw) → VisionRelayTask → CAN 발행 (relay만, 값 해석/재판단 없음)
```

View에서 CAN Driver를 직접 호출하지 않는다. `VisionRelayTask`는 UART로 받은 E의 값을 그대로 CAN 프레임에 실어 보낼 뿐이며, `VehicleModelTask`/GUI 로직과 섞지 않는다(2026-09-17, `DEC-HW-008/030/031`).

## C Drive

```text
RF 수신기(기어·조향·속도 요청) → DriverInputTask → Driver_Input(CAN, C 발행: accel/brake/steering/gear)
Final_Drive_Command → Validation → Control → PWM/DIR/Servo
PWM/모터 명령값 → 추정 함수 → Motor_RPM/Speed(estimated) → Status
```

Motor driver rating과 command timeout/safe state가 FROZEN이어야 한다. Encoder/Hall 실측 Feedback은 사용하지 않는다 (`DEC-HW-012` REMOVED) — speed/rpm 표시는 명령값 기반 추정 함수(`DEC-CTRL-021`)로 대체한다. Driver 입력(RF, `DEC-HW-024`)의 속도 요청 표현 및 기존 accel/brake 계약과의 매핑은 `DEC-CTRL-019`에서 확정한다.

## D Body

```text
Body_Command(headlamp_brightness/turn/brake) → CAN↔LIN Mapping → LIN → Lamp
Lamp Status → LIN → CAN Body_Status
```

LIN schedule은 Gateway만 소유한다. Ambient 관련 기능은 삭제되어 D Slave는 Lamp actual state owner만 담당한다. `brake_lamp`는 F가 감속을 감지해 자동 생성하며, D는 그 값을 그대로 LIN으로 중계할 뿐 판단하지 않는다.

## E HPC

```text
Front Camera(1대) → YOLO/COCO 객체인식 → detected_class + direction/zone
→ Vision_Status(팝업용) / ADAS_Request(회피요청) → uart_bridge_service → UART → B
                                                                          (B가 CAN relay 발행, F/B가 소비)
```

Raw image는 CAN으로 보내지 않는다. Rear Camera/Rear Vision/주차 Vision 기능은 삭제되었다 (`DEC-HW-016`, `DEC-VIS-002/006/007` REMOVED) — 초음파 충돌 위험도 판단은 A(Ultrasonic)가 전담한다. E는 CAN 인터페이스를 직접 쓰지 않는다 (2026-09-17, `DEC-HW-008` REMOVED) — `can_service`는 삭제되고 `uart_bridge_service`로 대체됐다. E가 여전히 `Vision_Status`/`ADAS_Request`의 논리적 owner이며, B는 CAN으로 옮겨 보내는 대행자일 뿐이다(§1.1 예외 참고).

## F VCU

```text
Driver_Input(CAN, gear 포함) + Perception + Status + Fault
→ Validation/Freshness
→ Safety/Arbitration
→ Final_Drive_Command / Body_Command
```

Final command writer는 `VcuControlTask` 하나다. F는 Gear 물리 GPIO를 더 이상 직접 읽지 않는다 (2026-09-15부터 C 소유).

---

# 9. AI / 담당자 수정 규칙

AI에게 문서를 넘길 때 반드시 이 규칙을 같이 준다.

```text
FINAL_IMPLEMENTATION_SPEC.md가 최상위 규칙이다.
고정된 Publisher/Owner/역할 경계를 변경하지 마라.
OWNER INPUT 또는 OPEN 상태의 값을 임의로 최종 확정하지 마라.
Project Owner가 승인한 값만 FROZEN으로 바꿔라.
역할별 내부 구현은 제안할 수 있지만 ECU 간 Interface는 이 문서를 따른다.
충돌 시 FINAL_IMPLEMENTATION_SPEC.md를 우선한다.
```

---

# 10. Final Freeze Checklist

검증용 코드로 필요한 근거를 확보한 뒤, 최종 구현 baseline을 확정하기 전 Project Owner 확인:

- [ ] Hardware Decision 전부 FROZEN
- [ ] Network Decision 전부 FROZEN
- [ ] Vehicle/Control Decision 전부 FROZEN
- [ ] Perception/Vision Decision 전부 FROZEN
- [ ] HMI/Body Decision 전부 FROZEN
- [ ] DTC/Health Decision 전부 FROZEN
- [ ] CAN Message Contract 전부 FROZEN
- [ ] LIN Contract 전부 FROZEN
- [ ] RTOS/Linux Execution Contract 전부 FROZEN

모든 핵심 항목이 FROZEN된 시점을 **Implementation Baseline v1.0**으로 태그/커밋한다.

현재(2026-09-17)는 Hardware/Network/Vehicle-Control/Perception-Vision/HMI-Body/DTC-Health의 **설계 결정 대부분**과 CAN/LIN 메시지 layout까지 동결됐지만, 실측이 필요한 수치(calibration 계수, threshold/hysteresis, RTOS Stack/Queue 실측값, TouchGFX frame budget, 물리 bitrate 검증 등)가 다수 OPEN으로 남아있어 v1.0 태그를 만들지 않는다.
각 동결 기록에는 결정 ID, 날짜, Owner 지시/승인, 근거 문서·시험 대상 소스 SHA, 적용 범위와 미검증 범위를 남긴다.
동결 후 변경은 사유·영향 ECU/메시지·재시험 범위·Owner 승인을 기록하고 새 revision으로 반영한다.
Baseline v1.0은 구현 계약의 동결이며 차량 전체 시험 PASS나 최종 release를 대신하지 않는다.
