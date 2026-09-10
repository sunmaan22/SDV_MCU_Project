# STM32H735G-DK IVI Hardware Bring-up

[IVI 문서 홈](README.md) · [핀맵](PIN_MAP.md) · [Reference Project](REFERENCE_PROJECT.md)

> 목적: STM32H735G-DK에서 TouchGFX가 정상 동작하는 상태를 먼저 확보한 뒤, SDV CAN(FDCAN2)과 IVI FreeRTOS 구조를 단계적으로 추가한다.

## 1. Bring-up 원칙

최종 IVI 코드는 외부 예제 저장소를 그대로 복사해서 사용하지 않는다. 외부 프로젝트는 H735G-DK의 LTDC / DMA2D / OCTOSPI / HyperRAM / TouchGFX 설정을 검증하는 reference로만 사용한다.

작업 순서는 다음과 같이 고정한다.

```text
Reference project 확인
→ LCD 출력 PASS
→ Touch 입력 PASS
→ TouchGFX 화면 PASS
→ FDCAN2 추가
→ CAN loopback / physical CAN PASS
→ CanRxTask / CommandTxTask 추가
→ VehicleModelTask / Repository 추가
→ SDV CAN message 연결
→ 최종 Cockpit UI 연결
```

문제가 발생하면 항상 마지막으로 PASS한 단계까지 돌아가 원인을 분리한다.

---

## 2. Stage 0 — 개발 환경 확인

확인 항목:

- Board: `STM32H735G-DK`
- MCU: `STM32H735IGK6` 계열
- STM32CubeMX / CubeIDE 버전 기록
- STM32CubeH7 package 버전 기록
- TouchGFX Designer 버전 기록
- ST-LINK firmware update
- USB 전원 및 ST-LINK 연결 확인

외부 reference `.ioc`가 구버전 CubeMX로 생성되어 있으면 처음 열 때 migration 전 원본을 보존한다.

PASS 조건:

```text
[ ] Board debugger 인식
[ ] Empty/debug project download 가능
[ ] 사용한 tool version 기록 완료
```

---

## 3. Stage 1 — Reference TouchGFX 구동

Reference:

- Repository: `MaJerle/touchgfx-cmake-vscode-stm32-simulator`
- 대상 파일: `STM32H735G-DK.ioc`
- 자세한 기준은 `REFERENCE_PROJECT.md` 참조

이 단계에서는 FDCAN, SDV message, 사용자 application task를 추가하지 않는다.

먼저 아래 기능만 검증한다.

```text
LTDC
DMA2D
OCTOSPI1 external Flash
OCTOSPI2 HyperRAM
TouchGFX
FreeRTOS
LCD / Touch BSP
```

PASS 조건:

```text
[ ] Build 성공
[ ] Flash 성공
[ ] 480x272 LCD 정상 출력
[ ] 화면 깨짐 / tearing / 지속적인 flicker 없음
[ ] Touch 좌표 정상
[ ] 화면 전환 가능
[ ] 5분 이상 GUI hang 없음
```

실패 시 FDCAN이나 SDV 코드를 추가하지 않는다.

---

## 4. Stage 2 — CubeMX 설정 기준 확보

Reference의 설정에서 다음 항목을 우리 프로젝트 기준으로 가져온다.

### Graphics

- LTDC: 480 × 272
- Pixel format: RGB888
- DMA2D enabled
- CRC enabled
- TouchGFX display interface: LTDC
- TouchGFX hardware accelerator: DMA2D

### External Memory

- OCTOSPI1: external NOR Flash / TouchGFX asset 저장
- OCTOSPI2: HyperBus HyperRAM / framebuffer 저장
- framebuffer candidate:
  - Buffer 0: `0x70000000`
  - Buffer 1: `0x70060000`
- Cortex-M7 I-Cache enabled
- Cortex-M7 D-Cache enabled
- MPU region은 external memory 특성에 맞게 유지

중요: CubeMX migration 이후 MPU / clock / OCTOSPI timing이 자동 변경되었는지 diff로 확인한다.

PASS 조건:

```text
[ ] Graphics setting 기록
[ ] External memory setting 기록
[ ] Clock tree 기록
[ ] MPU/cache setting 기록
[ ] 새로 생성한 코드에서도 TouchGFX PASS 유지
```

---

## 5. Stage 3 — FDCAN2 추가

IVI의 SDV backbone은 `FDCAN2` 사용을 기본안으로 한다.

```text
FDCAN2_RX = PB5
FDCAN2_TX = PB6
```

세부 핀은 `PIN_MAP.md`를 기준으로 한다.

CAN bitrate / CAN FD data bitrate / BRS / message ID / DLC는 `FINAL_IMPLEMENTATION_SPEC.md`의 Owner Freeze 전에는 임의로 최종 확정하지 않는다.

### 5.1 먼저 Internal Loopback

초기 테스트에서는 실제 CAN bus 없이 controller 동작을 확인한다.

검증 항목:

```text
TX frame 생성
→ FDCAN2 internal loopback
→ RX FIFO interrupt
→ callback / queue 수신
→ payload 비교
```

PASS 조건:

```text
[ ] FDCAN init 성공
[ ] TX enqueue 성공
[ ] RX interrupt 발생
[ ] ID / DLC / payload 동일
[ ] 반복 송수신에서 error counter 증가 없음
```

### 5.2 Physical CAN

Internal Loopback PASS 이후 실제 CAN transceiver / connector를 사용한다.

확인:

- CANH / CANL 배선
- GND common
- bus 양 끝만 120 ohm termination
- H735G-DK termination jumper 상태 확인

PASS 조건:

```text
[ ] 두 노드 간 송수신
[ ] ACK 정상
[ ] Error Passive / Bus Off 없음
[ ] 예상 주기에서 frame drop 없음
```

---

## 6. Stage 4 — FreeRTOS Application 구조 추가

TouchGFX Task에 CAN decode와 application logic을 몰아넣지 않는다.

```text
FDCAN2 ISR
    ↓
CanRxQueue
    ↓
CanRxTask
    ↓
ModelUpdateQueue
    ↓
VehicleModelTask
    ↓
VehicleDataRepository
    ↓
GuiTask / TouchGFX

GuiTask
    ↓
UiCommandQueue
    ↓
CommandTxTask
    ↓
FDCAN2 TX

HealthTask
    ├ CanRxTask health
    ├ VehicleModelTask health
    ├ GuiTask health
    └ queue/stack counters
```

ISR에서는 다음만 수행한다.

- RX frame 최소 copy
- timestamp/counter
- queue push 또는 task notification

ISR 금지 항목:

- TouchGFX 호출
- 복잡한 CAN signal decode
- DTC lookup
- printf
- blocking operation

PASS 조건:

```text
[ ] CAN burst 중 GUI freeze 없음
[ ] Touch 중 CAN RX starvation 없음
[ ] queue overflow 0 (normal load)
[ ] task stack high-water 측정 가능
```

---

## 7. Stage 5 — SDV Logical Message 연결

H735에 raw sensor를 직접 연결하지 않는다. 다음 logical message를 CAN으로 받는다.

### RX

- `Vehicle_State`
- `Drive_Status`
- `Ultrasonic_Status`
- `Vision_Status`
- `Body_Status`
- `DTC_Event`
- `ECU_Heartbeat`

### TX

- `Body_User_Request`
- Diagnostic request (최종 protocol freeze 후)

H735는 아래를 직접 수행하지 않는다.

- motor PWM
- servo PWM
- accelerator/brake ADC
- ultrasonic measurement
- camera image processing
- LIN master/slave
- lamp GPIO control
- VCU arbitration

---

## 8. Stage 6 — DummyDataProvider 교체

UI 개발 초반에는 `DummyDataProvider`로 화면을 만든다.

완성 후 데이터 경로를 아래처럼 바꾼다.

```text
Before
DummyDataProvider
→ VehicleDataRepository
→ TouchGFX

After
FDCAN2
→ CanRxTask
→ SignalDecoder
→ VehicleModelTask
→ VehicleDataRepository
→ TouchGFX
```

View / Presenter가 CAN ID 또는 payload bit position을 직접 알지 않도록 한다.

---

## 9. 최종 Bring-up Gate

다음 항목을 모두 통과하기 전에는 HMI 기능 구현 완료로 보지 않는다.

```text
[ ] LCD PASS
[ ] Touch PASS
[ ] External Flash PASS
[ ] HyperRAM / framebuffer PASS
[ ] TouchGFX PASS
[ ] FDCAN2 internal loopback PASS
[ ] FDCAN2 physical CAN PASS
[ ] CanRxTask PASS
[ ] VehicleModelTask PASS
[ ] CommandTxTask PASS
[ ] GUI + CAN concurrent load PASS
[ ] Timeout / invalid 표시 PASS
[ ] Critical Warning overlay PASS
```

검증 결과와 실제 측정값은 `TEST_REPORT.md`에 기록한다.
