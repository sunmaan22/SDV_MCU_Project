# Cluster + IVI Cockpit Test Report

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: `SPECIFICATION.md` 요구사항을 실제 시험으로 검증한다.  
> Reference board bring-up은 2026-09-10에 실제 STM32H735G-DK에서 수행했으며, 이후 SDV IVI 기능 시험은 단계적으로 추가한다.

## Document Information

| Item | Value |
|---|---|
| Node / Feature | Cluster + IVI Cockpit |
| Owner | B |
| Board / Platform | STM32H735G-DK + TouchGFX |
| Execution Model | FreeRTOS + CMSIS-RTOS2 |
| Reference Commit | `f5c3b1ee03992cfd2d98587da270d9b3f9edd82a` (MaJerle reference) |
| Firmware Commit | `835e48d` feat(ivi): FDCAN2 internal loopback bench test — `main`에 PR #2(`22d6e4f`)로 병합 |
| Test Date | 2026-09-10 |
| STM32CubeIDE | 1.19.0 |
| STM32CubeMX | 6.18.0 (SDV_IVI_H735) / 6.15.0 (reference 검증 시) |
| STM32Cube FW_H7 | V1.13.0 (SDV_IVI_H735) / V1.10.0 compatibility (reference) |
| TouchGFX | 4.26.1 (SDV_IVI_H735) / 4.21.0 (MaJerle reference) |
| Specification Revision | v0.2 |
| Architecture Revision | v0.2 |

### Revision History

| Revision | Date | Author | Change |
|---|---|---|---|
| v0.1 | 2026-09-09 | Team | Initial planned test |
| v0.2 | 2026-09-09 | Team | RTOS timing/stack/queue/watchdog tests added |
| v0.3 | 2026-09-10 | Team | STM32H735G-DK Reference TouchGFX 실기 bring-up 결과 기록 |
| v0.4 | 2026-09-10 | Team | FDCAN2 internal loopback bench 결과(§0.5) 반영, 관련 매트릭스 / RTOS / Evidence / Final Result 갱신 |
| v0.5 | 2026-09-10 | Team | §0.6 SDV_IVI_H735 자체 External Memory(OCTOSPI1 NOR / OCTOSPI2 HyperRAM) bring-up — `.map` + 플래시 verify + 육안으로 **PASS**, §0.6.5 빈 `Error_Handler` 관찰, Final Result / Remaining Issues 갱신 |

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
| 장시간 hang/tearing/flicker 및 응답시간 | NOT RUN | 별도 측정 결과 없음 |
| FDCAN2 loopback / physical CAN | NOT RUN | 아직 송수신 시험 전 |

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
| T-HMI-001 | REQ-HMI-001 | Speed/RPM/Gear 표시 | NOT RUN |
| T-HMI-002 | REQ-HMI-002 | READY/Warning 표시 | NOT RUN |
| T-HMI-003 | REQ-HMI-003 | ADAS 상태 표시 | NOT RUN |
| T-HMI-004 | REQ-HMI-004 | Parking 거리/Warning 표시 | NOT RUN |
| T-HMI-005 | REQ-HMI-005 | DTC list/detail | NOT RUN |
| T-HMI-006 | REQ-HMI-006 | Touch 화면 전환 | REFERENCE TOUCH PASS / IVI UI NOT RUN |
| T-HMI-007 | REQ-HMI-007 | timeout data invalid 표시 | NOT RUN |
| T-HMI-008 | REQ-HMI-008 | Body_User_Request CAN TX | NOT RUN |
| T-HMI-009 | REQ-HMI-009 | raw camera CAN path 없음 | NOT RUN |
| T-HMI-010 | REQ-HMI-010 | critical warning 우선 표시 | NOT RUN |
| T-HMI-011 | REQ-HMI-011 | CAN→Model ≤100 ms 목표 | NOT RUN |
| T-HMI-012 | REQ-HMI-012 | Touch≤150 ms 목표 | FUNCTIONAL PASS / TIMING NOT RUN |
| T-HMI-013 | REQ-HMI-013 | CAN/GUI task 분리 | NOT RUN |
| T-HMI-014 | REQ-HMI-014 | FDCAN ISR 최소 처리 | BENCH PARTIAL PASS (loopback: ISR enqueue-only, `irq_count` = 100) / 통합 NOT RUN |
| T-HMI-015 | REQ-HMI-015 | Queue/Repository 전달 | BENCH: ISR→queue→task 경로 PASS (`loopbackQueue`) / Repository 경로 NOT RUN |
| T-HMI-016 | REQ-HMI-016 | load 중 critical warning block 없음 | NOT RUN |
| T-HMI-017 | REQ-HMI-017 | stack/queue overflow 검증 | NOT RUN |
| T-HMI-018 | REQ-HMI-018 | HealthTask/watchdog-ready 구조 | NOT RUN |

---

# 4. Stage 1 Dummy UI Test

| Test | Input | Expected | Actual | Result |
|---|---|---|---|---|
| Cluster | speed=24, rpm=1250, gear=D | 값 표시 | NOT RUN | TBD |
| Parking | RR=180 mm, CRITICAL | right critical UI | NOT RUN | TBD |
| DTC | 2 entries | list/detail | NOT RUN | TBD |
| Screen Flow | 5개 화면 이동 | hang 없이 전환 | NOT RUN | TBD |
| Warning Overlay | Settings + CRITICAL injection | warning 우선 표시 | NOT RUN | TBD |

Stage 1에서도 가능하면 DummyDataProvider가 직접 GUI를 건드리지 않고 Model update 경로를 통과하게 한다.

---

# 5. CAN Integration Test

| Message | Direction | Expected | Actual | Timeout Test | Result |
|---|---|---|---|---|---|
| `Vehicle_State` | RX | gear/mode update | NOT RUN | planned | TBD |
| `Drive_Status` | RX | speed/rpm update | NOT RUN | planned | TBD |
| `Ultrasonic_Status` | RX | distance/warning | NOT RUN | planned | TBD |
| `Vision_Status` | RX | ADAS/Parking update | NOT RUN | planned | TBD |
| `Body_Status` | RX | lamp/ambient | NOT RUN | planned | TBD |
| `DTC_Event` | RX | DTC model update | NOT RUN | event | TBD |
| `Body_User_Request` | TX | UI request transmitted | NOT RUN | N/A | TBD |

---

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

| Fault | Expected Status | Pi Stored? | H735 Displayed? | Result |
|---|---|---|---|---|
| Ultrasonic timeout | sensor DTC | NOT RUN | NOT RUN | TBD |
| Body LIN fault | Body DTC | NOT RUN | NOT RUN | TBD |
| Vision fault | HPC DTC | NOT RUN | NOT RUN | TBD |
| HMI queue/task fault 후보 | HMI local health/DTC | NOT RUN | NOT RUN | TBD |

---

# 12. Evidence

- Reference build log: PASS, `0 errors, 0 warnings`
- Reference LCD visual confirmation: PASS
- Reference TouchGFX visual confirmation: PASS
- Reference Touch input confirmation: PASS
- TouchGFX screenshot/video artifact: not stored yet
- FDCAN2 loopback `g_fdcan_loopback` dump (2026-09-10): `state=2, tx=100, rx=100, pass=100, mismatch=0, timeout=0, irq_count=100, queue_overflow=0, TEC/REC/bus_off=0, stack_free_bytes=1684`
- FDCAN2 loopback 개발 로그: [DEVLOG.md](DEVLOG.md) · 코드 커밋 `835e48d`
- 외부 메모리 bring-up (§0.6, 2026-09-10): `.map` `SDV_IVI_H735.map` — `BufferSection @ 0x70000000` (0x17e800), `ExtFlashSection @ 0x90000000` (0x236600); 플래시 로그 `Erasing external memory sectors [0 35]` + `Download verified successfully`
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
FULL IVI INTEGRATION: NOT RUN
```

## 완료된 항목

- [x] STM32H735G-DK Reference Build / Flash
- [x] LCD 정상 출력
- [x] TouchGFX 화면 정상 표시
- [x] Touch 입력 UI 반응
- [x] FDCAN2 internal loopback bench (`state = 2`, tx/rx/pass 100/100, `irq_count` 100, stack free 1684/2048 B) — 커밋 `835e48d`
- [x] SDV_IVI_H735 자체 board bring-up — OCTOSPI1 NOR GUI asset(`0x90000000`) + OCTOSPI2 HyperRAM framebuffer(`0x70000000`) 실동작 (§0.6, `.map` + 플래시 verify + 육안)

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
- FDCAN2 physical CAN 시험 (트랜시버 + 2nd node / external loopback)
- 빈 `Error_Handler` 본문 — `while (1)` / fault 로깅 추가 (§0.6.5)
- 외부 메모리 런타임 디버거 보강 (선택): `HAL_OSPI_GetState` = mem-mapped, HyperRAM 임의주소 write/read, `0x70000000` 프레임 변화 (§0.6 D4/D5)
- CAN signal layout freeze (`DEC-NET-004~007`)
- task numeric priority (`DEC-HLT-001~003`)
- task stack size
- queue depth
- IWDG policy
- DTC Clear protocol
