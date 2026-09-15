# Cluster + IVI Cockpit Test Report

> **2026-09-15 사용자 결정 — 단일 Screen UI:** Cluster를 유지하는 하나의 TouchGFX Screen 안에서 ADAS/Parking/Diagnostics/Settings 패널을 표시·숨긴다. 화면 구성만 변경하며 ECU 간 CAN/LIN 메시지, publisher/consumer, 신호·주기·timeout, 제어 권한과 최상위 명세의 OPEN/FROZEN 상태는 변경하지 않는다. 기능 구현·실기 PASS를 의미하지 않는다.

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md` 요구사항을 실제 시험으로 검증한다.  
> Reference board bring-up은 2026-09-10에 실제 STM32H735G-DK에서 수행했으며, 이후 SDV IVI 기능 시험은 단계적으로 추가한다.

> **2026-09-15 범위 변경 (최소 수정):** 아직 `NOT RUN`인 계획 항목에서 `Body_Status.ambient` 소비와 `Vision_Status`의 ADAS/Parking 필드명만 새 계약에 맞게 조정했다. 기존 bring-up/실기 시험 결과(§0)는 변경하지 않았다.
>
> **2026-09-15 추가 변경:** Pi DTC Manager/History 삭제에 따라 DTC 시험을 "저장 확인"이 아닌 "실시간 표시 + 해소 시 소멸 확인"으로 변경했다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Cluster + IVI Cockpit |
| Owner | B |
| Board / Platform | STM32H735G-DK + TouchGFX |
| Execution Model | FreeRTOS + CMSIS-RTOS2 |
| Reference Commit | `f5c3b1ee03992cfd2d98587da270d9b3f9edd82a` (MaJerle reference) |
| Firmware Commit | 최신 Cluster: `e437940`; 기존 FDCAN2 loopback: `835e48d` (PR #2 `22d6e4f`); 항목별 기준 커밋 참조 |
| Test Date | 2026-09-10 ~ 2026-09-15 (항목별 수행일 참조) |
| STM32CubeIDE | 1.19.0 |
| STM32CubeMX | 6.18.0 (SDV_IVI_H735) / 6.15.0 (reference 검증 시) |
| STM32Cube FW_H7 | V1.13.0 (SDV_IVI_H735) / V1.10.0 compatibility (reference) |
| TouchGFX | 4.26.1 (SDV_IVI_H735) / 4.21.0 (MaJerle reference) |
| Specification Revision | v0.3 (조명 요청 경로 정합성 반영, 기능 시험 미실시) |
| Architecture Revision | v0.3 (조명 요청 경로 정합성 반영, 기능 시험 미실시) |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial planned test |
| v0.2 | 2026-09-09 | Team | RTOS timing/stack/queue/watchdog tests added |
| v0.3 | 2026-09-10 | Team | STM32H735G-DK Reference TouchGFX 실기 bring-up 결과 기록 |
| v0.4 | 2026-09-10 | Team | FDCAN2 internal loopback bench 결과(§0.5) 반영, 관련 매트릭스 / RTOS / Evidence / Final Result 갱신 |
| v0.5 | 2026-09-10 | Team | §0.6 SDV_IVI_H735 자체 External Memory(OCTOSPI1 NOR / OCTOSPI2 HyperRAM) bring-up — `.map` + 플래시 verify + 육안으로 **PASS**, §0.6.5 빈 `Error_Handler` 관찰, Final Result / Remaining Issues 갱신 |
| v0.6 | 2026-09-15 | Team | §0.7 HyperRAM MPU Region2 8MB→16MB 정합화 및 재검증(LCD ≥5분, FDCAN2 loopback 회귀 없음) 반영, Final Result / 완료된 항목 갱신 |
| v0.7 | 2026-09-15 | Team | §0.4 hang/tearing·FDCAN2 loopback 행을 §0.5/§0.7 결과로 갱신(PARTIAL, 물리 CAN·응답시간은 여전히 NOT RUN), Remaining Issues에서 완료된 `Error_Handler` 수정(`0a06b06`) 및 MPU 커밋(`a226f9a`) 반영 |
| v0.8 | 2026-09-15 | Team | §0.8 VehicleModel + DummyDataProvider bench(가이드 §2-4, pre-GUI) 결과 반영 — DUMMY-01~07/09 PASS, DUMMY-08 NOT RUN(GUI 없음), DUMMY-10 PARTIAL. Final Result / 완료된 항목 / Remaining Issues 갱신 |
| v0.9 | 2026-09-15 | Team | §0.9 Cluster 기본 표시/외부 로더 사용자 확인과 정적·증분 빌드 검사 기록; GUI 상태 전이/터치·부하 시험은 미실시 유지 |
| v0.10 | 2026-09-15 | Team | Body_Status.ambient 소비 제거, Vision_Status ADAS/Parking 필드명 조정(계획 항목만, 기존 실기 결과 §0 유지) |

---

# 0. Reference Board Bring-up Result

MaJerle STM32H735G-DK TouchGFX reference를 실제 보드에 build / flash / run하여 기본 디스플레이 및 터치 동작을 확인했다.

## 0.1 Build / Flash

| Test | Expected | Actual | Result |
|---|---|---|---|
| Code generation | 정상 생성 | 완료 | PASS |
| Build | 0 error | `0 errors, 0 warnings` | PASS |
| ELF generation | `.elf` 생성 | `STM32H735G-DK.elf` 생성 | PASS |
| ST-LINK flash/run | firmware 실행 | 정상 다운로드 및 실행 | PASS |

Build size:

```text
text = 1,039,918 bytes
data =       304 bytes
bss  =    45,016 bytes
```

## 0.2 LCD / Touch 사용자 확인

| Test | Expected | Actual | Result |
|---|---|---|---|
| LCD backlight | 정상 점등 | 정상 점등 확인 | PASS |
| TouchGFX reference screen | 480x272 화면 표시 | 화면 정상 표시 확인 | PASS |
| Touch input | 터치 시 UI 반응 | UI 정상 반응 확인 | PASS |

현재 확인 범위에서 **Reference LCD / Touch / TouchGFX Bring-up = PASS**로 판정한다.

장시간 안정성 항목인 tearing/flicker 지속 관찰 및 5분 이상 GUI hang 시험은 아직 별도 수행하지 않았다. Reference PASS를 근거로 다음 단계인 우리 IVI project 재현 및 FDCAN2 bring-up으로 진행한다.

## 0.3 Tooling Notes

- Reference `.ioc`는 CubeMX 6.5.0 / STM32CubeH7 V1.10.0 기반이다.
- 검증 시 CubeIDE 1.19.0 / CubeMX 6.15.0에서 `Continue`를 선택하여 기존 FW_H7 V1.10.0 호환 상태를 유지했다.
- `X-CUBE-TOUCHGFX 4.21.0` 패키지를 설치했다.
- FreeRTOS `USE_NEWLIB_REENTRANT`를 활성화한 뒤 code generation을 수행했다.
- Reference `.cproject`의 TouchGFX library search path에는 원 개발 PC 절대경로가 포함되어 있어, 로컬 `../../Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc` 경로로 수정 후 link 성공했다.
- CubeIDE의 Windows serial-port provider에서 `org.xml.sax.SAXParseException: Premature end of file` 로그가 관찰되었지만, ST-LINK flash 및 Reference GUI 실행에는 영향을 주지 않았다. 현 단계에서는 IDE tooling warning으로 분리 기록한다.

---

## 0.4 SDV_IVI_H735 — 클럭 수정 후 GUI 재검증 (2026-09-10)

우리 TouchGFX 4.26.1 프로젝트에서 FDCAN2 핀/초기화 추가 및 클럭 수정 후 GUI를 재검증했다. 근거는 로컬 설정·생성 코드 확인, 사용자가 제공한 ST-LINK 다운로드 로그 및 LCD/Touch 정상 확인이다. 아래 클럭은 설정값이며 계측값이 아니다.

| 항목 | 확인 내용 |
|---|---|
| Project | firmware/IVI/SDV_IVI_H735 |
| CubeMX / FW package | 6.18.0 / STM32Cube FW_H7 V1.13.0 |
| CPU / HCLK | 500 MHz / 250 MHz (기존 550/275 MHz와 다름) |
| LTDC | 9.6 MHz |
| PLL3 | M=25, N=288, P=2, Q=2, R=30, FRACN=0; input range 1–2 MHz, Medium VCO; VCO 288 MHz |
| OCTOSPI kernel | 200 MHz |
| FDCAN2 | PB5 RX / PB6 TX, HSE 25 MHz |
| Debug tools (제공 로그) | ST-LINK GDB server 7.11.0 / STM32CubeProgrammer 2.20.0 / ST-LINK V3J17M11 |

| 시험 | 결과 | 근거 |
|---|---|---|
| 코드 생성 및 PLL3/FDCAN 소스 반영 | PASS | .ioc 및 생성 코드 확인 |
| 내부/외부 메모리 다운로드·검증 | PASS | File download complete; Download verified successfully |
| LCD 화면 출력 | PASS | 사용자 실보드 확인 |
| Touch UI 반응 | PASS | 사용자 GUI 재검증 완료 기록 요청 |
| 장시간 hang/tearing/flicker 및 응답시간 | PARTIAL — hang/tearing: PASS / 응답시간: NOT RUN | hang/tearing은 §0.7(2026-09-15, ≥5분 연속 실행)에서 확인됨; Touch/Critical 응답시간 계측은 아직 없음 |
| FDCAN2 loopback / physical CAN | PARTIAL — internal loopback: PASS / physical CAN: NOT RUN | internal loopback은 §0.5(2026-09-10, state=2, 100/100)에서 완료; 트랜시버 + 2nd node 물리 CAN 시험은 아직 수행 전 |

로그의 다운로드 크기는 2.16 MB, 다운로드 16.010초, 검증 5.765초였다. 이번 빌드의 error/warning 개수는 제공되지 않아 0 errors/0 warnings로 단정하지 않는다.

**클럭 수정 후 GUI 기본 재검증: PASS.** 이 판정은 CAN 통신 또는 전체 IVI 기능 PASS를 의미하지 않는다. 로컬 수정 펌웨어의 커밋 SHA는 아직 고정하지 않았으며, 이 문서 업데이트가 해당 소스 변경의 업로드를 의미하지 않는다.

다음은 FDCAN2 Internal Loopback 설정 → RX/TX FIFO 및 수신 interrupt 준비 → 시험 프레임 송수신·payload 비교다. 현재 확인된 설정은 Classic/Normal, RX FIFO0=0, TX FIFO=0이므로 추가 설정과 시험 코드가 필요하다. 시험용 bitrate/ID는 임시 bench 값으로 관리하며 최종 Network Freeze로 간주하지 않는다.

---

## 0.5 FDCAN2 Internal Loopback Bench Test (2026-09-10)

§0.4의 다음 단계로, `SDV_IVI_H735`에 FDCAN2를 enable하고 FreeRTOS one-shot 벤치 태스크(`CanLoopback`)를 추가하여
트랜시버/물리 버스 없이 MAC 내부 loopback으로 Classic CAN 프레임 TX→RX 왕복 100회를 반복하고 프레임 무결성과
에러 카운터를 확인했다. 상세 설계·절차는 [DEVLOG.md](DEVLOG.md) 참조. 아래 bitrate/ID는 bench 값이며 Network Freeze가 아니다.

### 0.5.1 설정

| 항목 | 값 |
|---|---|
| Peripheral / 핀 | FDCAN2, PB6 = TX, PB5 = RX, `GPIO_AF9_FDCAN2` |
| Mode / Frame | `FDCAN_MODE_INTERNAL_LOOPBACK` / `FDCAN_FRAME_CLASSIC` |
| Kernel clock | `RCC_FDCANCLKSOURCE_HSE` = 25 MHz (시스템 클럭 변경과 무관) |
| Nominal timing | prescaler 5, TSeg1 7, TSeg2 2, SJW 1 → 500 kbit/s, sample point 80 % |
| Std filter | index 0, mask, ID1 `0x123`, ID2 `0x7FF` → RX FIFO0; non-matching / remote reject |
| IRQ | `FDCAN2_IT0_IRQn` (line 0), preempt priority 5 |
| Bench task | `CanLoopback`, stack 2048 B, priority `osPriorityBelowNormal`, one-shot |
| 시험 프레임 | std ID `0x123`, DLC 8, payload `53 44 56 <seq[31:0] LE> A5` |
| 반복 | 100회, 프레임 간 `osDelay(100 ms)` |

### 0.5.2 결과 — `g_fdcan_loopback` (실기 1회 실행, 루프 종료 후 디버거 확인)

환경: STM32H735G-DK, ST-LINK GDB server, `arm-none-eabi-gdb 14.2.90.20240526`.

| 필드 | 값 | Result |
|---|---:|---|
| `state` | 2 (PASS) | PASS |
| `tx_count` / `rx_count` / `pass_count` | 100 / 100 / 100 | PASS |
| `mismatch_count` | 0 | PASS |
| `timeout_count` | 0 | PASS |
| `irq_count` | 100 | PASS (RX FIFO0 new-message IRQ = 프레임당 1회, coalescing 없음) |
| `queue_overflow` | 0 | PASS |
| `rx_error` / `api_error` / `last_hal_error` | 0 / 0 / 0 | PASS |
| `tx_error_counter` (TEC) / `rx_error_counter` (REC) / `bus_off` | 0 / 0 / 0 | PASS |
| `stack_free_bytes` | 1684 / 2048 B (사용 364 B, 여유 82 %) | PASS |

### 0.5.3 판정

**FDCAN2 INTERNAL LOOPBACK BENCH: PASS (state = 2, 100/100).**

- ISR(`HAL_FDCAN_RxFifo0Callback`)은 카운트 + bounded drain 후 큐 enqueue만 수행하고 헤더/payload 비교는 태스크에서 처리 → `REQ-HMI-014`(FDCAN ISR 최소 처리) 구조를 bench 범위에서 확인.
- 안전 가드: Mode ≠ `INTERNAL_LOOPBACK` 또는 format ≠ `CLASSIC` 이면 즉시 fail → 실제 버스에 `0x123`을 송신하지 않는다.
- 이 판정은 **물리 CAN 버스 / 트랜시버 / 노드 간 통신 / 최종 network bitrate·ID를 의미하지 않는다.**

---

## 0.6 SDV_IVI_H735 자체 External Memory Bring-up (OCTOSPI1 NOR / OCTOSPI2 HyperRAM)

목적: MaJerle reference가 아닌 우리 `firmware/IVI/SDV_IVI_H735` 프로젝트에서
**OCTOSPI1 외부 NOR Flash(GUI asset)** 와 **OCTOSPI2 HyperRAM(TouchGFX 프레임버퍼)** 이 실보드에서 동작함을 확인한다.
확인되면 README NEXT의 "우리 IVI project에서 board setting 재현" 항목을 닫는다.

### 0.6.1 설계 요약 (코드 기준, `main.c` / linker / TouchGFXGeneratedHAL)

| 항목 | 값 |
|---|---|
| OCTOSPI1 대상 | Macronix NOR `MX25LM51245G`, OPI + DTR, `BSP_OSPI_NOR_EnableMemoryMappedMode(0)` |
| OCTOSPI1 매핑 | `0x90000000`, linker `OSPI` LENGTH 64M, TouchGFX `ExtFlashSection` → `>OSPI` |
| OCTOSPI2 대상 | HyperRAM `S70KL1281`, HyperBus, `HAL_OSPI_MemoryMapped(&hospi2, …)` |
| OCTOSPI2 매핑 | `0x70000000`, linker `HYPERRAM` LENGTH 16M |
| LTDC layer 0 | `pLayerCfg.FBStartAdress = 0x70000000`, RGB888, 480 × 272 |
| Framebuffer | `frameBuf` (더블버퍼) section `TouchGFX_Framebuffer` → `>HYPERRAM` |
| 기타 HYPERRAM | `Video_RGB_Buffer` (video 디코드 출력) 동일 영역 |
| MPU | R1 `0x70000000` 512MB NO_ACCESS 배경 + R2 8MB FULL/cacheable(HyperRAM 창) · R3 `0x90000000` 512MB NO_ACCESS 배경 + R4 64MB FULL/cacheable(OSPI 창) |
| 실패 처리 | `BSP_OSPI_NOR_Init` / `…EnableMemoryMappedMode` / `HAL_OSPI_MemoryMapped` 실패 시 `Error_Handler()` **호출되지만 현재 `Error_Handler` 본문이 비어 있어 그대로 return됨** (아래 0.6.5 참고) |

### 0.6.2 시험 절차

**A. 빌드 증거 (`.map` 확인)**

1. `SDV_IVI_H735` Debug 빌드 — error/warning 수 기록.
2. `.map`에서 다음 심볼 주소 확인:
   - `frameBuf` == `0x70000000`
   - 두 번째 버퍼(`frameBuf + sizeof/2`) 가 `0x70000000`~`0x71000000`(16M) 안
   - `Video_RGB_Buffer` 가 HYPERRAM 16M 안
   - TouchGFX image/font 데이터가 `ExtFlashSection` / `0x90000000` 대역에 배치
3. `.map`에 HYPERRAM/OSPI region overflow 경고 없음.

**B. 플래시**

- 앱 → 내부 Flash `0x08000000`.
- **GUI asset → 외부 OSPI NOR**: STM32CubeProgrammer external loader `MX25LM51245G_STM32H735G-DK`
  (또는 TouchGFX Designer "Run Target" 이 앱+asset 동시 처리).
- asset 미플래시 시 이미지가 빈 사각형/placeholder로 표시됨 → 이것이 주요 실패 신호.

**C. 실행 / 육안**

| 확인 | 판정 근거 |
|---|---|
| 모든 이미지 / 아이콘 / 폰트가 정상 렌더 (빈 박스 없음) | OSPI NOR `0x90000000` 읽기 정상 |
| slider 드래그 · analog clock 초침 애니메이션이 tearing / 깨진 라인 없이 부드러움 | HyperRAM 프레임버퍼 + 더블버퍼 swap 정상 |
| ≥ 5분 연속 실행, hang / freeze 없음 | 메모리 매핑 안정성 |

**D. 디버거 증거**

| 확인 | 방법 | 기대 |
|---|---|---|
| OSPI/LTDC init 중 `Error_Handler` 미진입 | `Error_Handler` 에 BP | `MX_OCTOSPI1_Init` / `MX_OCTOSPI2_Init` / `MX_LTDC_Init` 통과 |
| 프레임버퍼 주소 | Expressions `&frameBuf`, `hltdc.LayerCfg[0].FBStartAdress` | 둘 다 `0x70000000` |
| OSPI NOR 내용 | Memory `0x90000000` | all `0xFF` / `0x00` 아님, flash한 asset blob 시작과 일치 |
| HyperRAM R/W | Memory: 미사용 주소(예 `0x70800000`)에 패턴 write 후 read | write == read |
| HyperRAM 스캔아웃 | GUI 동작 중 Memory `0x70000000` | 픽셀 데이터, 프레임마다 변화 |
| (선택) 매핑 상태 | `HAL_OSPI_GetState(&hospi1)` / `(&hospi2)` | `HAL_OSPI_STATE_BUSY_MEM_MAPPED` |

### 0.6.3 결과 (2026-09-10, Debug 빌드 `.map` + 실보드)

증거:
`.map` = `firmware/IVI/SDV_IVI_H735/STM32CubeIDE/Debug/SDV_IVI_H735.map` (2026-09-10 빌드),
플래시 로그 = ST-LINK GDB server 7.11.0 / STM32CubeProgrammer 2.20.0 / ST-LINK V3J17M11, 육안 = 사용자 실보드 확인.

`.map` Memory Configuration: `OSPI 0x90000000 len 0x04000000` (64M), `HYPERRAM 0x70000000 len 0x01000000` (16M) — linker와 일치.

| # | 시험 | 기대 | 실제 | Result |
|---|---|---|---|---|
| A1 | 빌드 | error 0 | `.elf` 생성 + 플래시 verify 통과 → error 0. warning 수 미기록 | PASS (warning 미기록) |
| A2 | framebuffer가 `0x70000000` 대역 | HYPERRAM 시작 | `.map`: `BufferSection 0x70000000 size 0x17e800`; `TouchGFX_Framebuffer` @ `0x70000000`(0x5fa00, TouchGFXHAL.o) + `0x7005fa00`(0xbf400, `frameBuf` 더블버퍼) | PASS |
| A3 | `Video_RGB_Buffer` in HYPERRAM 16M | 범위 내 | `.map`: `Video_RGB_Buffer` @ `0x7011ee00` (0x5fa00), 영역 끝 `0x7017e800` — HYPERRAM `0x71000000` 한계 내 | PASS |
| A4 | GUI asset이 OSPI(`0x90000000`) | 범위 내 | `.map`: `ExtFlashSection 0x90000000 size 0x236600` (~2.21 MB), 첫 asset `image_alternate_theme_..._analogclock_backgrounds` @ `0x90000000` | PASS |
| B1 | 앱 + 외부 asset 플래시 | 완료 | 로그: `Erasing internal memory sectors [0 1]` + `Erasing external memory sectors [0 35]`, `Download verified successfully` (다운로드 17.9 s / 검증 6.4 s) | PASS |
| C1 | 이미지/폰트 정상 렌더 | 빈 박스 없음 | 정상 (사용자 확인) → OSPI NOR `0x90000000` 런타임 read 동작 | PASS |
| C2 | 애니메이션 tearing/corruption 없음 | 없음 | 정상 (사용자 확인) → HyperRAM 프레임버퍼 write + LTDC 스캔아웃 동작 | PASS |
| C3 | ≥ 5분 hang 없음 | 없음 | 정상 (사용자 확인) | PASS |
| D1 | OSPI/LTDC init 실패 없음 | 실패 없음 | `Error_Handler` 본문이 비어 BP 무의미 (0.6.5). 대신 C1/C2가 정상 → `MX_OCTOSPI1_Init` / `MX_OCTOSPI2_Init` / `MX_LTDC_Init` 통과 확정 | PASS (간접) |
| D2 | framebuffer 링크 주소 | `0x70000000` 대역 | `.map` 정적 배치로 확인 (A2). 런타임 `hltdc.LayerCfg[0].FBStartAdress` 는 TouchGFX가 갱신하지 않는 필드라 값(예 `0x482b9000`)은 의미 없음 | PASS (정적) |
| D3 | `0x90000000` asset 데이터 | 실제 데이터 | `.map` asset 배치 + 플래시 verify 통과 + C1 정상 | PASS (정적 + verify + C1) |
| D4 | HyperRAM write/read | 일치 | 런타임 디버거 미확인 — C2가 기능적으로 갈음 | NOT RUN (C2로 갈음) |
| D5 | `0x70000000` 프레임 데이터 변화 | 변화 | 런타임 디버거 미확인 — C2가 기능적으로 갈음 | NOT RUN (C2로 갈음) |

### 0.6.4 판정

```text
SDV_IVI_H735 EXTERNAL MEMORY BRING-UP: PASS
  OCTOSPI1 NOR  (GUI asset @ 0x90000000):  PASS  (.map 배치 + 플래시 verify + 런타임 렌더 정상)
  OCTOSPI2 HyperRAM (framebuffer @ 0x70000000): PASS  (.map 배치 + 애니메이션 tearing 없이 정상)
```

- 판정 근거는 정적(`.map`) + 플래시 verify + 육안이다. `HAL_OSPI_GetState`, HyperRAM 임의주소 write/read, `0x70000000` 프레임 변화 같은 런타임 디버거 확인(D4/D5)은 하지 않았고, 필요 시 후속 보강한다.
- 이 판정은 **외부 메모리 2개가 memory-mapped로 정상 동작**함만 의미하며, 5개 화면 / CAN / RTOS 통합 PASS를 의미하지 않는다.

### 0.6.5 관찰: 빈 `Error_Handler`

`Core/Src/main.c`의 `Error_Handler()` 본문이 비어 있어(`USER CODE` 블록만 존재) **HAL 오류가 조용히 무시되고 실행이 계속된다.**
현재는 OSPI/LTDC init이 실제로 성공(C1/C2)해서 문제가 드러나지 않지만, 진단·안전상 `while (1)` 또는 fault 기록/로깅을 추가하는 것을 별도 항목으로 남긴다. (§13 Remaining Issues)

---

## 0.7 HyperRAM MPU Region2 크기 정합화 (8MB → 16MB, 2026-09-15)

§0.6.1에 기록된 대로 MPU Region2(HyperRAM 창)는 8MB로 설정되어 있었으나, 실제 보드 HyperRAM은 128Mbit(16MiB)이고 링커 `HYPERRAM` 영역도 16M로 선언되어 있어 정합이 맞지 않았다. 상위 8MiB(`0x70800000`~`0x70FFFFFF`)에 접근하면 MPU 기본 배경 영역(Region1, 512MB NO_ACCESS)이 적용되어 fault 위험이 있었다.

### 0.7.1 변경

| 항목 | 변경 전 | 변경 후 |
|---|---|---|
| CubeMX(.ioc) MPU Region2 Size | `MPU_REGION_SIZE_8MB` | `MPU_REGION_SIZE_16MB` |
| `main.c` `MPU_Config()` Region2 | `MPU_REGION_SIZE_8MB` | `MPU_REGION_SIZE_16MB` (Generate Code로 반영) |
| Region2 Base / 속성 | `0x70000000`, FULL_ACCESS / Cacheable / Bufferable | 변경 없음 |
| 링커 `HYPERRAM` | 16M | 변경 없음 |

### 0.7.2 적용 중 관찰된 현상 (코드 결함 아님, 운영 노트)

변경 직후 1차 플래시에서 LCD가 아무것도 표시되지 않는 현상이 있었다. 조사 결과 코드/MPU 설정 문제가 아니라, 별도로 진행 중이던 TouchGFX Designer 작업에서 생성된 미사용 프로젝트(`TouchGFX/MyApplication/`)와 무관한 이전 실험 중 보드가 fault/hang 상태로 멈춰 있었던 것이 원인이었고, 단순 재플래시만으로는 그 상태가 풀리지 않았다. ST-LINK를 재연결(완전 전원 재기동)한 뒤 정상 표시를 확인했다.

**운영 노트**: MPU/외부 메모리 설정 실험 후 화면 이상이 있으면, 재빌드/재플래시만으로 판단하지 말고 보드 전원을 완전히 재기동(ST-LINK 재연결 또는 리셋)한 뒤 재확인한다.

### 0.7.3 재검증 결과

| 시험 | 기대 | 실제 | Result |
|---|---|---|---|
| 빌드 | 0 error | `0 errors, 0 warnings` (text=1,039,918 / data=304 / bss=45,016 — §0.1과 동일) | PASS |
| LCD 표시 (완전 전원 재기동 후) | 정상 표시 | 정상 표시 확인 | PASS |
| ≥5분 연속 실행 | hang / 화면 깨짐 없음 | 5분 이상 정상, 이상 없음 확인 | PASS |
| FDCAN2 internal loopback 회귀 재시험 | §0.5와 동일 결과 | `state=2, tx=100, rx=100, pass=100, mismatch=0, timeout=0, irq_count=100, queue_overflow=0, rx_error/api_error/last_hal_error=0, TEC/REC/bus_off=0, stack_free_bytes=1684` — §0.5.2와 완전 동일 | PASS (회귀 없음) |

### 0.7.4 판정

```text
HYPERRAM MPU REGION2 8MB -> 16MB ALIGNMENT: PASS
  Build: PASS (0 error / 0 warning)
  LCD >=5min stability: PASS
  FDCAN2 loopback regression: PASS (no change vs 0.5)
```

가이드([IVI_MPU_Dummy_TouchGFX_Guide.md] 1단계, HyperRAM MPU 정리) 완료 기준을 충족한다. 상위 8MiB(구 8MB~16MB 구간) 실제 write/read 검증(디버거 pattern test)은 아직 별도로 수행하지 않았으며, 필요 시 §0.6 D4/D5와 함께 후속 보강한다. 이 변경은 이 문서 갱신 시점 기준 아직 커밋되지 않은 로컬 변경이다.

---

## 0.8 VehicleModel + DummyDataProvider Bench (가이드 §2-4, pre-GUI, 2026-09-15)

목적: TouchGFX Cluster 화면(가이드 §5)이 아직 없는 상태에서, Dummy 신호 → `VehicleModelTask` → `VehicleDataRepository`(`g_vehicle_data`) 데이터 경로가 신호별 freshness/timeout/invalid 정책대로 동작하는지 디버거로 직접 확인한다.

### 0.8.1 구성 요소

| 항목 | 내용 |
|---|---|
| `VehicleModelTask` | `Core/Src/freertos.c`. `osMessageQueueGet` 100ms 유한 대기로 주기적으로 깨어나 메시지 유무와 무관하게 매번 timeout을 재계산 |
| `VehicleDataRepository` (`g_vehicle_data`) | mutex(`s_repoMutex`)로 보호되는 단일 쓰기자 스냅샷. `g_fdcan_loopback`과 동일하게 디버거 Expressions에서 직접 확인 가능 |
| `DummyDataProvider` (`DummyDataTask`) | 200ms 주기, `g_dummy_mode`(디버거에서 실시간 변경 가능)에 따라 NORMAL/SOURCE_INVALID/PAUSE_DRIVE/CRITICAL/RECOVERY 5개 시나리오 재현 |
| Signal status 판정 | `received → source_valid → timeout` 순서로 `SIGNAL_NO_DATA`/`SIGNAL_INVALID`/`SIGNAL_TIMEOUT`/`SIGNAL_VALID` 결정. **주의**: 한 번 `source_valid=0`이 찍힌 신호는 이후 갱신이 아예 끊겨도 `SIGNAL_INVALID`로 남고 `SIGNAL_TIMEOUT`으로 전이하지 않는다(우선순위상 invalid가 timeout보다 앞섬) — 실제 이 순서로 시험하다 관찰됨(§0.8.3 참고) |

빌드 관련 참고: `vehicle_data.c`/`dummy_data_provider.c`를 별도 소스 파일로 처음 추가했으나, 이 프로젝트가 CDT per-file 빌드 등록 방식(`.project`의 개별 `<link>` + `.cproject`의 파일별 툴 설정)이라 `.project`에 링크만 추가해서는 컴파일 대상에 안 잡혔다. 로직을 기존에 이미 빌드 대상인 `Core/Src/freertos.c`(기존 `CanLoopback` 벤치 코드가 있는 파일)로 옮겨서 해결했다. 헤더(`vehicle_data.h`, `dummy_data_provider.h`)는 `-I` 경로로만 참조되므로 별도 파일로 유지.

### 0.8.2 시험 절차 및 원시 결과

**A. 부팅 직후 (모드 변경 전)**

```text
g_vehicle_data: drive.status=SIGNAL_NO_DATA, gear_ready.status=SIGNAL_NO_DATA, warning.status=SIGNAL_NO_DATA
g_dummy_mode = DUMMY_MODE_NORMAL
```

약 1초 후:

```text
g_vehicle_data: drive={speed_kmh=25, rpm=1260, received=1, source_valid=1, status=SIGNAL_VALID}
                gear_ready={gear='D', ready=1, status=SIGNAL_VALID}
                warning={severity=WARNING_NONE, active=0, status=SIGNAL_VALID}
g_dummy_stats: update_count=15, queue_overflow=0
```

**B. `g_dummy_mode = DUMMY_MODE_SOURCE_INVALID`(1)**

```text
drive={source_valid=0, status=SIGNAL_INVALID}, gear_ready={source_valid=0, status=SIGNAL_INVALID}
warning: 영향 없음 (status=SIGNAL_VALID 유지)
```

**C. `g_dummy_mode = DUMMY_MODE_PAUSE_DRIVE`(2) — B 상태에서 바로 전환**

```text
drive: last_update_tick 고정(8800), status=SIGNAL_INVALID 유지 (TIMEOUT으로 전이하지 않음, §0.8.1 참고)
gear_ready: last_update_tick 계속 증가(8800→16800), status=SIGNAL_VALID 유지
```

**D. `g_dummy_mode = DUMMY_MODE_NORMAL`(0) 복귀**

```text
drive.status=SIGNAL_VALID로 즉시 복귀, source_valid=1
```

**E. `g_dummy_mode = DUMMY_MODE_CRITICAL`(3)**

```text
warning={severity=WARNING_CRITICAL, active=1, status=SIGNAL_VALID}
drive/gear_ready는 평소대로 계속 갱신
```

**F. `g_dummy_mode = DUMMY_MODE_RECOVERY`(4)**

```text
warning={severity=WARNING_NONE, active=0} 로 해제
```

**G. `g_dummy_mode = DUMMY_MODE_NORMAL`(0) 상태로 5분 방치**

```text
snapshot_version: 계속 증가하여 5369까지 도달, 정지 없음
g_dummy_stats: update_count=3206, queue_overflow=0 (5분 내내 0 유지)
```

**H. 클린 재시험 — NORMAL에서 바로 `DUMMY_MODE_PAUSE_DRIVE`(2)로 전환 (INVALID를 거치지 않은 경우)**

```text
drive={source_valid=1 유지, last_update_tick 고정(216200), status=SIGNAL_TIMEOUT}
gear_ready={last_update_tick 계속 증가(216200→220000), status=SIGNAL_VALID}
g_dummy_stats: queue_overflow=0
```

### 0.8.3 결과 매트릭스 (가이드 §6 DUMMY-01~10 기준)

| ID | 시험 | 결과 | 근거 |
|---|---|---|---|
| DUMMY-01 | 공급원 시작 전 부팅 | PASS | A: 부팅 직후 전부 `SIGNAL_NO_DATA` |
| DUMMY-02 | 정상 속도·RPM 공급 | PASS | A: 1초 후 `SIGNAL_VALID`, 값 지속 변화 |
| DUMMY-03 | valid=false 주입 | PASS | B: `SIGNAL_INVALID`, 직전 값 그대로 유지 |
| DUMMY-04 | Drive 업데이트만 중단 → timeout | PASS | H(클린 재시험): `SIGNAL_VALID → SIGNAL_TIMEOUT` |
| DUMMY-05 | 다른 신호 갱신 지속 | PASS | C, H: drive가 멈춰도 gear_ready는 계속 갱신 |
| DUMMY-06 | 정상 데이터 재개 | PASS | D: 즉시 `SIGNAL_VALID` 복귀 |
| DUMMY-07 | Critical 주입·해제 | PASS | E, F: 주입/해제 모두 확인 |
| DUMMY-08 | 터치하면서 값 갱신 | NOT RUN | TouchGFX Cluster 화면(가이드 §5) 미구현으로 터치 자체가 없음 |
| DUMMY-09 | 5분 이상 연속 실행 | PASS | G: `snapshot_version` 5369까지 정지 없이 증가 |
| DUMMY-10 | 임시 과부하/큐 포화 | PARTIAL | 전 구간 `queue_overflow=0` 확인. 강제 포화(burst) 주입은 별도 시험 필요 |

### 0.8.4 판정

```text
VEHICLEMODEL + DUMMYDATAPROVIDER BENCH: PASS (pre-GUI)
  DUMMY-01~07, 09: PASS
  DUMMY-08: NOT RUN (GUI 없음)
  DUMMY-10: PARTIAL (queue_overflow=0 확인, 강제 포화 미시험)
```

이 판정은 **Repository/DummyDataProvider 데이터 경로**만 검증하며, TouchGFX Cluster 화면 연동(가이드 §5) 및 실제 CAN 연동은 포함하지 않는다.

### 0.8.5 관찰: INVALID가 TIMEOUT보다 우선하는 상태 전이

C 시험에서 확인된 대로, `source_valid=0`이 한 번 기록된 신호는 이후 갱신이 완전히 끊겨도(§C에서 8초 이상 미갱신) `SIGNAL_TIMEOUT`이 아니라 `SIGNAL_INVALID`로 남는다. 코드상 의도된 우선순위(`received → source_valid → timeout` 순서)의 결과이며 버그는 아니지만, "invalid였던 소스가 완전히 끊겼을 때도 계속 INVALID로 표시할지, 아니면 일정 시간 후 COMM LOST(TIMEOUT)로 재분류할지"는 실제 표시 정책 확정 시(가이드 §5) 재검토가 필요하다.

---

## 0.9 TouchGFX Cluster 기본 표시 및 플래시 재확인 (2026-09-15)

### 0.9.1 대상 및 근거

- 코드: [`e437940`](https://github.com/sunmaan22/SDV_MCU_Project/commit/e437940c31451eec38a813bf34d9f0292b49e4a9), STM32H735G-DK / TouchGFX 4.26.1 / CubeIDE 1.19.0.
- Repository → Model → Presenter → View 연결, Unicode 문자열 처리, 텍스트 영역 확대, 화면 리소스 및 외부 플래시 로더 설정을 포함한다.
- 하드웨어 판정 근거: 사용자의 “오키 다 나온다” 확인. 상세 원시 플래시 로그/스크린샷 및 화면의 정확한 숫자는 이번 세션에 저장하지 않았다.
- §0.8은 GUI 연결 전 시험 기록이다. 해당 데이터 경로 PASS를 이번 GUI 상태 전이/부하 시험 PASS로 확대하지 않는다.

### 0.9.2 결과

| Test ID | 시험 | Actual / Evidence | Result |
|---|---|---|---|
| CLUSTER-01 | 기존 Debug 빌드 확인 | `make -j4 all` exit 0; 기존 산출물 증분 검사, clean rebuild 미실시 | PASS (incremental) |
| CLUSTER-02 | 화면/리소스 정합성 | Designer JSON, text XML 파싱 및 참조 이미지·폰트 존재 확인; `git diff --check` 통과 | PASS (static) |
| CLUSTER-03 | 외부 플래시 다운로드 후 표시 | 비어 있던 External Loader에 `MX25LM51245G_STM32H735G-DK.stldr` 활성화; 사용자 정상 표시 확인 | PASS (user-confirmed smoke) |
| CLUSTER-04 | 속도/RPM/기어 기본 표시 | `???`/잘림 수정 후 사용자 정상 표시 확인; 정확한 수치·경계값별 캡처 없음 | PASS (user-confirmed smoke) |
| CLUSTER-05 | 긴 텍스트의 영역 폭 | 176 px 영역에 `8000 rpm` 108 px, `200 km/h` 111 px, `-- INVALID` 127 px, `-- COMM LOST` 161 px; 폰트 advance 합산 | PASS (static) / 각 상태 실기 NOT RUN |
| CLUSTER-06 | NO_DATA/INVALID/TIMEOUT 및 복구 화면 전이 | 구현 코드 확인만 수행; 상태별 주입 후 화면 관찰 미실시 | NOT RUN |
| CLUSTER-07 | READY/DEMO/CRITICAL/RECOVERY 표시 전이 | 구현 코드 확인만 수행; 상태별 실기 판정 근거 없음 | NOT RUN |
| DUMMY-08 (GUI 후속) | 터치 중 값 갱신 | Cluster 화면 구성 완료; 터치 상호작용 중 갱신 시험은 미실시 | NOT RUN |
| CLUSTER-08 | GUI 부하/장시간/타이밍 및 물리 CAN | 이번 변경 기준 별도 시험 미실시 | NOT RUN |

빌드 size 출력(bytes): `text=3492756, data=732, bss=1666376`.
외부 Flash/HyperRAM 영역을 포함하므로 위 값을 MCU 내부 메모리 사용량으로 해석하지 않는다.

### 0.9.3 재현 및 다음 시험

1. TouchGFX Designer에서 기존 프로젝트를 열고 Generate Code 후 CubeIDE 빌드한다.
2. Debug Configurations → Debugger → External loaders에서 해당 보드 로더를 활성화한다. 커밋된 경로는 로컬 설치 경로이므로 다른 PC에서 조정한다.
3. Flash/run 후 속도/RPM/기어가 문자 깨짐이나 잘림 없이 표시되는지 캡처한다.
4. `g_dummy_mode`로 NORMAL → SOURCE_INVALID → NORMAL → PAUSE_DRIVE → NORMAL, CRITICAL → RECOVERY를 주입하여 화면과 Repository를 함께 기록한다.
5. 터치 중 갱신, GUI 장시간 실행, 경고 지연, 큐 포화/stack 여유를 별도 측정한다.

```text
TOUCHGFX CLUSTER BASIC DISPLAY: PASS (user-confirmed smoke)
GUI STATE TRANSITIONS / TOUCH-UPDATE / LOAD / TIMING: NOT RUN
FULL IVI INTEGRATION: NOT RUN
```

---

# 1. Test Objective

H735 Cockpit이 Dummy Data와 실제 CAN 데이터를 이용해 Cluster/ADAS/Parking/Diagnostics/Settings 화면을 정상 표시하는지 검증한다. 동시에 FreeRTOS 기반 `CanRxTask`, `VehicleModelTask`, `GuiTask`, `CommandTxTask`, `HealthTask`가 의도한 구조로 실행되고, CAN burst나 UI load에서도 queue overflow, stack overflow, starvation 없이 주요 Timing 요구사항을 만족하는지 확인한다.

---

# 2. Test Environment

| Item | Value |
|---|---|
| Board | STM32H735G-DK |
| RTOS | FreeRTOS (CubeMX 생성), `configUSE_NEWLIB_REENTRANT = 1` / 최종 수치 TBD |
| API | CMSIS-RTOS2 |
| UI | TouchGFX 4.26.1 (SDV_IVI_H735) / 최종 프로젝트 버전 확정 TBD |
| Interface | LCD / Touch / FDCAN2 (PB6 TX, PB5 RX) |
| CAN bitrate | 500 kbit/s (bench loopback 값, network freeze 아님) |
| Debug | STM32CubeIDE 1.19.0 / ST-LINK GDB server / `arm-none-eabi-gdb 14.2.90.20240526` / runtime stats 후보 |
| Watchdog | IWDG policy TBD |

---

# 3. Requirement Verification Matrix

| Test ID | Requirement | Expected | Result |
|---|---|---|---|
| T-HMI-001 | REQ-HMI-001 | Speed/RPM/Gear 표시 | DUMMY BASIC DISPLAY PASS (사용자 확인, §0.9) / 실제 CAN·경계값 NOT RUN |
| T-HMI-002 | REQ-HMI-002 | READY/Warning 표시 | NOT RUN |
| T-HMI-003 | REQ-HMI-003 | ADAS 상태 표시 | NOT RUN |
| T-HMI-004 | REQ-HMI-004 | Parking 거리/Warning 표시 | NOT RUN |
| T-HMI-005 | REQ-HMI-005 | DTC list/detail | NOT RUN |
| T-HMI-006 | REQ-HMI-006 | 단일 Screen 내 패널 열기·닫기 | REFERENCE TOUCH PASS / IVI UI NOT RUN |
| T-HMI-007 | REQ-HMI-007 | timeout data invalid 표시 | NOT RUN |
| T-HMI-008 | REQ-HMI-008 | Body_User_Request CAN TX | NOT RUN |
| T-HMI-009 | REQ-HMI-009 | raw camera CAN path 없음 | NOT RUN |
| T-HMI-010 | REQ-HMI-010 | critical warning 우선 표시 | NOT RUN |
| T-HMI-011 | REQ-HMI-011 | CAN→Model ≤100 ms 목표 | NOT RUN |
| T-HMI-012 | REQ-HMI-012 | Touch≤150 ms 목표 | FUNCTIONAL PASS / TIMING NOT RUN |
| T-HMI-013 | REQ-HMI-013 | CAN/GUI task 분리 | NOT RUN |
| T-HMI-014 | REQ-HMI-014 | FDCAN ISR 최소 처리 | BENCH PARTIAL PASS (loopback: ISR enqueue-only, `irq_count` = 100) / 통합 NOT RUN |
| T-HMI-015 | REQ-HMI-015 | Queue/Repository 전달 | BENCH: loopback queue PASS; Dummy Repository PASS (§0.8), GUI 기본 표시 확인 (§0.9) / 실제 CAN 통합 NOT RUN |
| T-HMI-016 | REQ-HMI-016 | load 중 critical warning block 없음 | NOT RUN |
| T-HMI-017 | REQ-HMI-017 | stack/queue overflow 검증 | NOT RUN |
| T-HMI-018 | REQ-HMI-018 | HealthTask/watchdog-ready 구조 | NOT RUN |

---

# 4. Stage 1 Dummy UI Test

| Test | Input | Expected | Actual | Result |
|---|---|---|---|---|
| Cluster | Dummy speed/rpm/gear (정확한 관찰값 미기록) | 값 표시 | 사용자 기본 표시 확인 (§0.9); 고정 입력 24/1250/D 별도 재시험 필요 | BASIC DISPLAY PASS / 고정값·경계값 NOT RUN |
| Parking | RR=180 mm, CRITICAL | right critical UI | NOT RUN | TBD |
| DTC | 2 entries | list/detail | NOT RUN | TBD |
| Panel Flow | 동일 Screen에서 4개 패널 열기·닫기 | Cluster 유지, hang/잔상 없음 | NOT RUN | TBD |
| Warning Overlay | Settings + CRITICAL injection | warning 우선 표시 | NOT RUN | TBD |

Stage 1에서도 가능하면 DummyDataProvider가 직접 GUI를 건드리지 않고 Model update 경로를 통과하게 한다.

---

# 5. CAN Integration Test

| Message | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Vehicle_State` | RX | gear/mode update | NOT RUN | planned | TBD |
| `Drive_Status` | RX | speed/rpm update | NOT RUN | planned | TBD |
| `Ultrasonic_Status` | RX | distance/warning | NOT RUN | planned | TBD |
| `Vision_Status` / `ADAS_Request` | RX | detected_class/direction/warning update | NOT RUN | planned | TBD |
| `Body_Status` | RX | lamp | NOT RUN | planned | TBD |
| `DTC_Event` | RX | DTC model update | NOT RUN | event | TBD |
| `Body_User_Request` | TX | UI request transmitted | NOT RUN | N/A | TBD |

---

## 5.1 Lighting Request 경로 검증 기준 (v0.3)

최상위 명세에 따라 `IVI → Body_User_Request → VCU → Body_Command → Body Gateway` 경로를 검증한다.

- IVI Touch 이벤트가 UiCommandQueue와 CommandTxTask를 거쳐 VCU 대상 Body_User_Request로 송신되는지 확인한다.
- IVI가 Body_Command를 직접 송신하지 않는지 확인한다.
- 통합 시험에서 VCU가 최종 Body_Command를 발행하고 Gateway가 수신하는지 확인한다.
- CAN ID·DLC·payload는 최상위 명세에서 FROZEN된 값을 사용한다.

**상태: NOT RUN.** 문서 정합성 수정이며 기존 bring-up 결과를 위 기능의 PASS 근거로 사용하지 않는다.

# 6. Fault / Edge Case Test

| Test ID | Fault / Edge Case | Expected | Actual | Result |
|---|---|---|---|---|
| F-HMI-001 | Drive timeout | speed/rpm invalid + warning | NOT RUN | TBD |
| F-HMI-002 | Ultrasonic valid=false | Sensor Invalid | NOT RUN | TBD |
| F-HMI-003 | Vision timeout | Vision Unavailable | NOT RUN | TBD |
| F-HMI-004 | Unknown DTC | raw code/source 표시 | NOT RUN | TBD |
| F-HMI-005 | Touch 연타 | GUI freeze 없음 | NOT RUN | TBD |
| F-HMI-006 | CAN unavailable | Comm Fault | NOT RUN | TBD |
| F-HMI-007 | CanRxQueue overflow injection | counter/health policy | NOT RUN | TBD |
| F-HMI-008 | GuiTask artificial load | CAN model ingestion 유지 | NOT RUN | TBD |

---

# 7. RTOS Task Test

## 7.1 Task Inventory

| Task | Expected Trigger / Period | Priority Direction | Observed | Result |
|---|---|---|---|---|
| `CanRxTask` | event | High | NOT RUN | TBD |
| `VehicleModelTask` | event / 10~20 ms 후보 | Normal~High | NOT RUN | TBD |
| `GuiTask` | TouchGFX tick | Normal | Reference GUI running | PARTIAL |
| `CommandTxTask` | event | Normal | NOT RUN | TBD |
| `HealthTask` | 100 ms 후보 | Low | NOT RUN | TBD |
| `CanLoopback` (bench, 최종 구조 아님) | one-shot, 100회 후 `osThreadExit` | BelowNormal | 100회 완료, `state = 2` | BENCH PASS |

## 7.2 Period / Jitter

| Task | Target | Min | Avg | Max | Jitter | Result |
|---|---:|---:|---:|---:|---:|---|
| VehicleModelTask periodic mode, 사용 시 | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| GuiTask effective update | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |
| HealthTask | TBD | NOT RUN | NOT RUN | NOT RUN | NOT RUN | TBD |

## 7.3 Stack / Memory

| Task / Item | Configured | High-Water / Minimum Free | Result |
|---|---:|---:|---|
| CanRxTask stack | TBD | NOT RUN | TBD |
| VehicleModelTask stack | TBD | NOT RUN | TBD |
| GuiTask stack | generated/TBD | NOT RUN | TBD |
| CommandTxTask stack | TBD | NOT RUN | TBD |
| HealthTask stack | TBD | NOT RUN | TBD |
| `CanLoopback` (bench) stack | 2048 B | 1684 B free (used 364 B) | BENCH PASS |
| Heap free | TBD | NOT RUN | TBD |

## 7.4 Queue / Event

| Object | Depth | Max Occupancy | Overflow Test | Result |
|---|---:|---:|---|---|
| `CanRxQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `ModelUpdateQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `UiCommandQueue` | TBD | NOT RUN | NOT RUN | TBD |
| `SystemEvents` | flags | N/A | NOT RUN | TBD |
| `loopbackQueue` (bench) | 8 | `queue_overflow` = 0 | 강제 overflow N/A | BENCH PASS |

## 7.5 ISR → Task

| Interrupt | Expected ISR Action | Expected Task | Actual | Result |
|---|---|---|---|---|
| FDCAN RX | enqueue/notify only | CanRxTask | bench: `HAL_FDCAN_RxFifo0Callback` → count + bounded drain + queue put만, 비교는 태스크. `irq_count` = 100, 큐 경로 정상 | BENCH PARTIAL PASS / 최종 `CanRxTask` NOT RUN |
| Touch/BSP IRQ | framework event only | GuiTask | Reference Touch response confirmed | PARTIAL |

Code Review에서 ISR 내부 decode/render/printf가 없는지 확인한다.

---

# 8. Load / Starvation Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| CAN burst + Cluster rendering | CanRxQueue overflow 0, GUI freeze 0 | NOT RUN | TBD |
| DTC list update + critical warning | warning path 지연 최소 | NOT RUN | TBD |
| Touch 연속 입력 + CAN RX | both continue | NOT RUN | TBD |
| Debug log enabled | timing target 유지 또는 영향 기록 | NOT RUN | TBD |

---

# 9. Timing Test

| Metric | Target | Measured | Method | Result |
|---|---:|---:|---|---|
| CAN RX → Vehicle Model | ≤100 ms 목표 | NOT RUN | timestamps | TBD |
| Critical warning → UI | ≤200 ms 목표 | NOT RUN | injection/display timestamp | TBD |
| Touch → UI response | ≤150 ms 목표 | FUNCTIONAL ONLY | visual check | TIMING NOT RUN |
| Queue backlog recovery | TBD | NOT RUN | burst test | TBD |

---

# 10. Health / Watchdog Test

| Scenario | Expected | Actual | Result |
|---|---|---|---|
| all task heartbeat healthy | HealthTask healthy | NOT RUN | TBD |
| CanRxTask heartbeat missing | fault state, watchdog policy 적용 후보 | NOT RUN | TBD |
| GuiTask heartbeat missing | HMI task fault | NOT RUN | TBD |
| Queue overflow | health counter 증가 | NOT RUN | TBD |
| Stack low watermark | warning/diagnostic candidate | NOT RUN | TBD |

실제 IWDG reset 시험은 bench 상태에서만 수행하고, 구현 전에는 논리/health flag 검증부터 한다.

---

# 11. DTC / Diagnostics Test

> Pi DTC Manager/History DB는 삭제됐다. 저장 여부가 아니라 **실시간 표시**와 fault 해소 시 목록에서 사라지는지를 확인한다.

| Fault | Expected Status | H735 Displayed (Active)? | 해소 시 사라짐? | Result |
|---|---|---|---|---|
| Ultrasonic timeout | sensor DTC | NOT RUN | NOT RUN | TBD |
| Body LIN fault | Body DTC | NOT RUN | NOT RUN | TBD |
| Vision fault | HPC DTC | NOT RUN | NOT RUN | TBD |
| HMI queue/task fault 후보 | HMI local health/DTC | NOT RUN | NOT RUN | TBD |

---

## 11.1 Active 표시 경계 조건 (추가 시험 계획)

| 조건 | 기대 동작 | 실행 상태 |
|---|---|---|
| 동일 source/code Active 중복 수신 | 목록 중복 없이 최신 상태 갱신 | NOT RUN |
| 서로 다른 ECU의 동일 code | source_node로 구분 | NOT RUN |
| Active → 유효한 Inactive/해소 flag | 목록에서 제거, 이력 없음 | NOT RUN |
| 다른 화면에서 발생 후 해소 | Diagnostics 복귀 시 과거 fault 표시 없음 | NOT RUN |
| fault 유지 중 IVI 재시작/재접속 | 현재 상태 재동기화 후 Active 표시 | NOT RUN |
| Active/Inactive 프레임 누락 | 확정된 재전송/snapshot 계약으로 현재 상태 회복 | NOT RUN |
| 소스 통신 두절 | 상태 미확인/통신 두절 표시, 정상 해소로 오인하지 않음 | NOT RUN |

재동기화·timeout 수치가 OWNER INPUT이므로 관련 실기 판정은 계약 동결 후 수행한다.

# 12. Evidence

- Cluster 표시/플래시 (§0.9, 2026-09-15, 코드 `e437940`): 사용자 정상 표시 확인, 기존 Debug 증분 빌드 성공, JSON/XML/리소스 참조 및 diff 검사 통과. 스크린샷/원시 플래시 로그 미저장.

- Reference build log: PASS, `0 errors, 0 warnings`
- Reference LCD visual confirmation: PASS
- Reference TouchGFX visual confirmation: PASS
- Reference Touch input confirmation: PASS
- TouchGFX screenshot/video artifact: not stored yet
- FDCAN2 loopback `g_fdcan_loopback` dump (2026-09-10): `state=2, tx=100, rx=100, pass=100, mismatch=0, timeout=0, irq_count=100, queue_overflow=0, TEC/REC/bus_off=0, stack_free_bytes=1684`
- FDCAN2 loopback 개발 로그: [DEVLOG.md](DEVLOG.md) · 코드 커밋 `835e48d`
- 외부 메모리 bring-up (§0.6, 2026-09-10): `.map` `SDV_IVI_H735.map` — `BufferSection @ 0x70000000` (0x17e800), `ExtFlashSection @ 0x90000000` (0x236600); 플래시 로그 `Erasing external memory sectors [0 35]` + `Download verified successfully`
- HyperRAM MPU Region2 8MB→16MB 정합화 재검증 (§0.7, 2026-09-15): 빌드 0 error/0 warning, LCD ≥5분 정상, FDCAN2 loopback 회귀 재시험 `g_fdcan_loopback` = §0.5.2와 완전 동일(`state=2, tx=100, rx=100, pass=100, mismatch=0, timeout=0, irq_count=100, queue_overflow=0, stack_free_bytes=1684`)
- VehicleModel + DummyDataProvider bench (§0.8, 2026-09-15): `g_vehicle_data`/`g_dummy_mode`/`g_dummy_stats` 디버거 원시 덤프 — 부팅 NO_DATA, SOURCE_INVALID/PAUSE_DRIVE/CRITICAL/RECOVERY 전이, 5분 방치 `snapshot_version` 5369까지 정지 없음, `queue_overflow=0` 유지
- CAN log (physical bus): TBD
- Runtime stats (통합 태스크): TBD
- stack high-water log (통합 태스크): TBD
- queue occupancy log (통합 태스크): TBD
- trace/scope: TBD

예시 로그:

```text
[RTOS][TASK] CanRx alive
[RTOS][QUEUE] CanRxQueue high=4/16
[RTOS][STACK] GuiTask watermark=TBD
[CAN][RX] Drive_Status
[MODEL] speed=24 rpm=1250
[HMI][WARN] PARKING_CRITICAL
```

---

# 13. Final Result

```text
REFERENCE BOARD BRING-UP: PASS
SDV_IVI_H735 CLOCK-CHANGE GUI RETEST: PASS
FDCAN2 INTERNAL LOOPBACK BENCH: PASS (state = 2, 100/100)
SDV_IVI_H735 EXTERNAL MEMORY BRING-UP: PASS (OCTOSPI1 NOR + OCTOSPI2 HyperRAM)
HYPERRAM MPU REGION2 8MB -> 16MB ALIGNMENT: PASS (build/LCD >=5min/FDCAN2 regression)
VEHICLEMODEL + DUMMYDATAPROVIDER BENCH: PASS pre-GUI (DUMMY-01~07,09 PASS; 08 NOT RUN; 10 PARTIAL)
TOUCHGFX CLUSTER BASIC DISPLAY: PASS (user-confirmed smoke, e437940; GUI state/touch/load tests NOT RUN)
FULL IVI INTEGRATION: NOT RUN
```

## 완료된 항목

- [x] STM32H735G-DK Reference Build / Flash
- [x] LCD 정상 출력
- [x] TouchGFX 화면 정상 표시
- [x] Touch 입력 UI 반응
- [x] FDCAN2 internal loopback bench (`state = 2`, tx/rx/pass 100/100, `irq_count` 100, stack free 1684/2048 B) — 커밋 `835e48d`
- [x] SDV_IVI_H735 자체 board bring-up — OCTOSPI1 NOR GUI asset(`0x90000000`) + OCTOSPI2 HyperRAM framebuffer(`0x70000000`) 실동작 (§0.6, `.map` + 플래시 verify + 육안)
- [x] HyperRAM MPU Region2 8MB→16MB 정합화 — 빌드 0 error/0 warning, LCD ≥5분 정상, FDCAN2 loopback 회귀 없음 (§0.7, 커밋 `a226f9a`)
- [x] VehicleModel + DummyDataProvider (가이드 §2-4, pre-GUI) — DUMMY-01~07/09 PASS, 신호별 freshness/timeout/invalid 정책 디버거로 확인 (§0.8)

- [x] Cluster 데이터 연결 및 기본 표시 사용자 확인 (§0.9, `e437940`); 상태 전이/터치·부하는 별도 미실시

## 전체 IVI PASS 조건

- [ ] 주요 SDV UI 기능 정상
- [ ] CAN RX/TX 정상
- [ ] timeout/invalid 정상
- [ ] CanRx/Model/Gui/Command/Health Task 정상
- [ ] ISR 최소 처리 확인
- [ ] 예상 부하에서 Queue overflow 0
- [ ] Stack 여유 측정
- [ ] critical warning load test 통과
- [ ] Timing 목표 측정
- [ ] Health/Watchdog 정책 검증
- [ ] 증거 저장

## Remaining Issues / Next Gate

- ~~검증된 로컬 펌웨어 변경의 소스 커밋 고정~~ → 완료 (`835e48d`, PR #2 `22d6e4f`)
- ~~FDCAN2 internal loopback~~ → 완료 (§0.5, bench PASS)
- ~~`SDV_IVI_H735` 자체 HyperRAM(OCTOSPI2) · external Flash(OCTOSPI1) 실동작 확인~~ → 완료 (§0.6, PASS)
- ~~HyperRAM MPU Region2 8MB→16MB 정합화~~ → 완료 (§0.7, PASS, 커밋 `a226f9a`)
- ~~빈 `Error_Handler` 본문 — `while (1)` / fault 로깅 추가 (§0.6.5)~~ → 완료 (커밋 `0a06b06`, halt loop + `g_error_handler_caller` 기록 추가)
- ~~VehicleModel + DummyDataProvider 데이터 경로 (가이드 §2-4)~~ → 완료 (§0.8, pre-GUI bench PASS)
- ~~TouchGFX Cluster 화면 구성 및 데이터 연결 (가이드 §5)~~ → 구현 및 기본 표시 사용자 확인 완료 (§0.9, `e437940`); 상태별 GUI 전이 시험은 미실시
- DUMMY-08 터치 중 갱신, GUI 경고/INVALID/TIMEOUT 전이·복구 및 경계값 표시 시험
- DUMMY-10 실제 큐 포화(burst) 주입 시험 — 현재는 `queue_overflow=0` 관찰만 확인
- INVALID→TIMEOUT 우선순위 정책 재검토 (§0.8.5) — 한 번 invalid였던 신호가 이후 완전히 끊겨도 계속 INVALID로 표시되는 현재 동작이 최종 표시 정책에 맞는지 확인
- FDCAN2 physical CAN 시험 (트랜시버 + 2nd node / external loopback)
- 외부 메모리 런타임 디버거 보강 (선택): `HAL_OSPI_GetState` = mem-mapped, HyperRAM 임의주소 write/read, `0x70000000` 프레임 변화 (§0.6 D4/D5)
- CAN signal layout freeze (`DEC-NET-004~007`)
- task numeric priority (`DEC-HLT-001~003`)
- task stack size
- queue depth
- IWDG policy
- DTC Active/Inactive 수신 및 현재 상태 재동기화 계약

## 14. 단일 Screen / 패널 전환 추가 시험 계획 (2026-09-15)

사용자 UI 결정에 따른 향후 검증 기준이다. 이전 bring-up/Cluster PASS 기록은 보존하며 아래 시험으로 확대하지 않는다.

| Test ID | 시험 | 기대 결과 | Result |
|---|---|---|---|
| PANEL-01 | 4개 패널 반복 열기·닫기 | 동일 Screen 유지, 일반 패널 최대 1개, 잔상 없음 | NOT RUN |
| PANEL-02 | 패널 표시 중 계기판 확인 | 속도·기어·READY 및 주요 경고 읽기 가능 | NOT RUN |
| PANEL-03 | 숨긴 패널 위치 터치 / 모달 배경 터치 | 숨김 위젯·배경 설정 요청 발생 없음 | NOT RUN |
| PANEL-04 | 패널 숨김 중 데이터 갱신·timeout 후 다시 열기 | 최신 snapshot과 invalid/timeout 표시 | NOT RUN |
| PANEL-05 | 각 패널 및 모달 중 critical 주입 | 경고가 가려지지 않으며 목표 200ms 측정 | NOT RUN |
| PANEL-06 | critical 중 일반 패널 닫기 | 활성 critical이 해제되지 않음 | NOT RUN |
| PANEL-07 | 패널 전환과 명시적 조명 조작 분리 | 전환만으로 TX 없음; 조명 조작은 기존 VCU 요청 경로 | NOT RUN |
| PANEL-08 | 패널 연타·CAN burst·장시간 실행 | 기존 timing 목표, stack/queue/메모리 예산 검증 | NOT RUN |

Gear R 자동 호출/복귀 시험은 DEC-HMI-003 확정 후 추가한다.
