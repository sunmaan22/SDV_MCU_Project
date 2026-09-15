# IVI 개발일지 (Development Log)

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: STM32H735G-DK IVI 펌웨어의 단계별 구현/디버깅 과정과 실기에서 관찰한 값을 날짜순으로 남긴다.
> 확정된 시험 판정은 [TEST_REPORT.md](TEST_REPORT.md), 구조는 [ARCHITECTURE.md](ARCHITECTURE.md)를 따른다. 이 문서는 작업 로그다.

---

## 2026-09-15 · VehicleModel + DummyDataProvider (가이드 §2-4, pre-GUI)

### 1. 이번 작업 범위

`IVI_MPU_Dummy_TouchGFX_Guide.md` 2~4단계를 구현했다: App 데이터 경로(§2), 차량 데이터 모델(§3), DummyDataProvider(§4).
TouchGFX Cluster 화면(§5)은 아직 없으므로, 이번 단계는 **GUI 없이 디버거로 데이터 경로만** 검증한다.

### 2. 소프트웨어 구조

```text
DummyDataTask (200ms 주기, Core/Src/freertos.c)
  └ g_dummy_mode에 따라 Drive/GearReady/Warning 업데이트 생성
        ↓ VehicleModel_PushUpdate() (non-blocking, 큐 depth 8)
VehicleModelTask (100ms 유한 대기로 주기적 wake-up)
  ├ 메시지 있으면 s_drive/s_gearReady/s_warning에 반영
  ├ 메시지 유무와 무관하게 매 wake마다 상태 재계산
  │   ComputeSignalStatus(received, source_valid, last_update_tick, now, timeout)
  │   → SIGNAL_NO_DATA / SIGNAL_INVALID / SIGNAL_TIMEOUT / SIGNAL_VALID
  └ mutex(s_repoMutex)로 g_vehicle_data(Repository)에 스냅샷 갱신

g_vehicle_data, g_dummy_mode, g_dummy_stats
  → g_fdcan_loopback과 동일한 방식으로 디버거 Expressions에서 직접 확인
```

시험용 주기/timeout(`VEHICLE_MODEL_PERIOD_MS=100`, `DRIVE_TIMEOUT_MS=500`, `GEAR_READY_TIMEOUT_MS=800`, `WARNING_TIMEOUT_MS=3000`, `DUMMY_PERIOD_MS=200`)은 전부 벤치용 값이며 최종 스펙이 아니다.

### 3. 빌드 이슈와 해결

`vehicle_data.c`/`dummy_data_provider.c`를 새 소스 파일로 추가했으나 링커에서 `undefined reference` 발생.
원인: 이 CubeIDE 프로젝트가 폴더 단위가 아니라 **파일 하나하나를 `.project`에 개별 `<link>`로 등록**하는 구조(`MCUAdvancedStructureProjectNature`)였고, 새 파일은 그 목록에 없어 Eclipse가 존재 자체를 몰랐다. `.project`에 `<link>`를 수동으로 추가해도 `.cproject`의 파일별 툴체인 설정이 없어 `sources.mk`에 여전히 안 잡혔다.
해결: 새 로직을 이미 빌드 대상인 `Core/Src/freertos.c`(기존 `CanLoopback` 벤치 코드가 있는 파일)에 병합하고, 별도 `.c` 파일은 삭제했다. 헤더(`vehicle_data.h`, `dummy_data_provider.h`)는 `-I` 경로로만 참조되므로 별도 파일로 유지해도 무방하다.

### 4. 관찰값 (실기, 디버거 Expressions)

| 단계 | `g_dummy_mode` | 관찰 |
|---|---|---|
| 부팅 직후 | NORMAL | 전 신호 `SIGNAL_NO_DATA` |
| 약 1초 후 | NORMAL | `speed_kmh=25, rpm=1260, status=SIGNAL_VALID`(drive/gear_ready) |
| SOURCE_INVALID | 1 | drive/gear_ready `source_valid=0, status=SIGNAL_INVALID` |
| PAUSE_DRIVE (INVALID 상태에서 전환) | 2 | drive는 `SIGNAL_INVALID`로 정지(8초+ 경과해도 TIMEOUT 전이 안 함, §5 참고), gear_ready는 계속 갱신 |
| NORMAL 복귀 | 0 | drive 즉시 `SIGNAL_VALID` |
| CRITICAL | 3 | `warning.severity=WARNING_CRITICAL, active=1` |
| RECOVERY | 4 | `warning.active=0`로 해제 |
| 5분 방치 | 0 | `snapshot_version` 5369까지 정지 없이 증가, `queue_overflow=0` |
| 클린 재시험: NORMAL→PAUSE_DRIVE | 0→2 | drive `source_valid=1` 유지한 채 `SIGNAL_VALID → SIGNAL_TIMEOUT` (500ms 경과 후), gear_ready는 계속 `SIGNAL_VALID` |

### 5. 판정

```text
VEHICLEMODEL + DUMMYDATAPROVIDER BENCH: PASS (pre-GUI)
```

가이드 §6 DUMMY-01~10 기준 DUMMY-01~07/09 PASS, DUMMY-08은 GUI가 없어 NOT RUN, DUMMY-10은 `queue_overflow=0` 관찰만 하고 강제 포화는 미시험(PARTIAL)이다. 상세 매트릭스는 [TEST_REPORT.md](TEST_REPORT.md) §0.8 참조.

**관찰된 설계 특성**: `source_valid=0`이 한 번 찍힌 신호는 이후 갱신이 완전히 끊겨도 `SIGNAL_TIMEOUT`이 아니라 `SIGNAL_INVALID`로 남는다(`received → source_valid → timeout` 우선순위 때문). 버그는 아니지만 최종 표시 정책 확정 시(가이드 §5) 이 우선순위가 맞는지 재검토가 필요하다.

### 6. 관련 파일

```text
firmware/IVI/SDV_IVI_H735/
├ Core/Inc/vehicle_data.h          : SignalStatus, VehicleDataSnapshot, VehicleUpdateMsg, API 선언
├ Core/Inc/dummy_data_provider.h   : DummyMode, DummyDataStats 선언
├ Core/Src/freertos.c              : VehicleModelTask, DummyDataTask, g_vehicle_data/g_dummy_mode/g_dummy_stats 구현
└ Core/Src/main.c                  : RTOS_THREADS에 VehicleModel_Create() → DummyDataProvider_Create() 호출 추가
```

---

## 2026-09-15 · HyperRAM MPU Region2 8MB → 16MB 정합화

### 1. 이번 작업 범위

`IVI_MPU_Dummy_TouchGFX_Guide.md` 1단계(HyperRAM MPU 정리)를 적용했다.
보드 HyperRAM은 128Mbit(16MiB)이고 링커 `HYPERRAM` 영역도 16M로 선언되어 있었지만,
MPU Region2(HyperRAM 창)는 8MB만 허용하고 있어 정합이 맞지 않았다. 상위 8MiB
(`0x70800000`~`0x70FFFFFF`)에 접근하면 배경 Region1(512MB NO_ACCESS)이 적용되어 fault 위험이 있었다.

### 2. 변경

| 항목 | 변경 전 | 변경 후 |
|---|---|---|
| `SDV_IVI_H735.ioc` MPU Region2 Size | `MPU_REGION_SIZE_8MB` | `MPU_REGION_SIZE_16MB` |
| `main.c` `MPU_Config()` Region2 | `MPU_REGION_SIZE_8MB` | `MPU_REGION_SIZE_16MB` (CubeMX Generate Code로 반영) |
| Region2 Base(`0x70000000`) / 속성(FULL_ACCESS, Cacheable, Bufferable) | 변경 없음 | 변경 없음 |
| 링커 `HYPERRAM` (16M) | 변경 없음 | 변경 없음 |

CubeMX는 Standalone 6.18.0에서 `.ioc`만 수정 후 Generate Code로 재생성했다 (CubeIDE 내장 6.15로 재생성하지 않음, 가이드 권고 준수).

### 3. 디버깅 경위 (원인 오인 → 재확인)

1. `.ioc` Region2를 16MB로 바꾸고 재생성 후 첫 플래시에서 LCD가 아무것도 표시되지 않았다.
2. MPU 크기 변경 자체가 원인인지 의심해 `.ioc`/`main.c`를 8MB로 되돌렸으나, 되돌린 뒤에도 화면이 안 나오는 현상이 계속됐다.
3. 조사 중 별개로, TouchGFX Designer가 기존 `SDV_IVI_H735.touchgfx`를 열지 않고 새 프로젝트 `TouchGFX/MyApplication/`(독립 BSP/HAL 트리 포함)을 생성해놓은 것을 발견 — STM32CubeIDE Run Configurations에 `STM32H735G-DK`가 2개 뜨는 원인이었다. 이 미사용 프로젝트를 완전 삭제하고, 부수적으로 함께 삭제됐던 `TouchGFX/ApplicationTemplate.touchgfx.part`는 `git checkout`으로 복구했다.
4. 위 정리 후에도 화면이 안 나와, git diff로 실제 소스(`main.c`, `TouchGFX/`, `Middlewares/`)가 정리 전/후 커밋 기준과 동일함을 확인 — 즉 **코드/설정 문제가 아님**을 확인했다.
5. ST-LINK를 뽑았다 다시 꽂아 보드를 완전 전원 재기동하자 정상 표시됐다. 이전 실험 중 보드가 fault/hang 상태로 멈춰 있었고, 단순 재빌드·재플래시로는 그 상태가 풀리지 않았던 것으로 결론지었다.
6. 이후 Region2를 다시 16MB로 적용 → 정상 동작 확인.

**교훈**: MPU/외부 메모리 설정을 바꾼 뒤 화면 이상이 있으면, 코드를 되돌리기 전에 먼저 보드를 완전 전원 재기동(ST-LINK 재연결 또는 리셋)해서 재현되는지 확인한다.

### 4. 재검증 결과

| 시험 | 결과 |
|---|---|
| 빌드 | `0 errors, 0 warnings` (text=1,039,918 / data=304 / bss=45,016) |
| LCD 표시 (완전 전원 재기동 후) | 정상 |
| ≥5분 연속 실행 | hang / 화면 깨짐 없음 |
| FDCAN2 internal loopback 회귀 재시험 | `g_fdcan_loopback`: `state=2, tx=100, rx=100, pass=100, mismatch=0, timeout=0, irq_count=100, queue_overflow=0, rx_error=0, api_error=0, last_hal_error=0, tx_error_counter=0, rx_error_counter=0, bus_off=0, stack_free_bytes=1684` — 2026-09-10 §0.5.2와 완전 동일, 회귀 없음 |

### 5. 판정

```text
HYPERRAM MPU REGION2 8MB -> 16MB ALIGNMENT: PASS
```

상세 시험 매트릭스는 [TEST_REPORT.md](TEST_REPORT.md) §0.7 참조. 상위 8MiB 실제 write/read 디버거 검증은 아직 미수행(§0.6 D4/D5와 함께 후속 보강 예정)이며, 이 변경은 작성 시점 기준 아직 커밋 전이다.

### 6. 관련 파일

```text
firmware/IVI/SDV_IVI_H735/
├ SDV_IVI_H735.ioc                : CORTEX_M7 MPU Region2 Size 8MB → 16MB
└ Core/Src/main.c                 : MPU_Config() Region2 Size 8MB → 16MB
```

---

## 2026-09-10 · FDCAN2 internal loopback bench test

### 1. 이번 작업 범위

`docs/ecus/IVI/README.md`의 NEXT 항목 중 두 가지를 구현했다.

```text
[x] FDCAN2 PB5/PB6 추가
[x] FDCAN2 internal loopback
```

CubeMX(.ioc) 재생성으로 FDCAN2를 enable하고, FreeRTOS 위에서 도는 one-shot 벤치 태스크(`CanLoopback`)를 추가해
STM32H735G-DK 한 보드 안에서 CAN 프레임 TX→RX 왕복을 100회 반복하며 프레임 무결성과 에러 카운터를 확인한다.
TouchGFX는 같은 CubeMX 패스에서 4.26.0 → 4.26.1로 올라갔고 데모 위젯(slider/analog clock/line)이 화면에 추가됐다.

### 2. 하드웨어 / 주변장치 설정

| 항목 | 값 | 비고 |
|---|---|---|
| Peripheral | FDCAN2 | PB6 = FDCAN2_TX, PB5 = FDCAN2_RX, `GPIO_AF9_FDCAN2` |
| Mode | `FDCAN_MODE_INTERNAL_LOOPBACK` | 트랜시버/버스 없이 MAC 내부에서 되돌림 |
| Frame format | `FDCAN_FRAME_CLASSIC` | Classic CAN, BRS 미사용 |
| Kernel clock | `RCC_FDCANCLKSOURCE_HSE` = 25 MHz | 시스템 클럭 변경과 무관하게 고정 |
| Nominal bit timing | prescaler 5, TSeg1 7, TSeg2 2, SJW 1 | tq = 200 ns, bit = 10 tq = 2000 ns |
| Bit rate | 500 kbit/s | sample point (1+7)/10 = 80 % |
| Std filter | index 0, mask, ID1 `0x123`, ID2 `0x7FF` → RX FIFO0 | 정확히 `0x123`만 수신 |
| Global filter | non-matching / remote 프레임 모두 reject | |
| NVIC | `FDCAN2_IT0_IRQn`, preempt priority 5 | ISR line 0 사용 |
| Interrupt | `FDCAN_IT_RX_FIFO0_NEW_MESSAGE` → `FDCAN_INTERRUPT_LINE0` | |

부수 변경(같은 CubeMX 재생성 결과):

| 파일 | 변경 | 이유 / 메모 |
|---|---|---|
| `SDV_IVI_H735.ioc`, `main.c` | SYSCLK PLLN 44 → 40 (275 → 250 MHz) | CubeMX clock tree 재계산 결과 |
| `.cproject` | debug cpuclock 힌트 275 → 250 | 위 변경에 맞춘 SWV 타임스탬프용 값 |
| `main.c` | PLL2 재조정 (M 5→2, N 80→16, R 2→1, RGE 2→3) | OSPI 클럭 소스 유지 |
| `main.c` | I2C4 Timing `0x00D049FB` → `0x00C042E4` | 커널 클럭 변경에 따른 재계산 |
| `.ioc` | MPU/메모리맵 영역(DTCMRAM/RAM/RAM_D2/RAM_D3/ITCMRAM) 명시 | CubeMX 6.18 memory-map |
| `FreeRTOSConfig.h`, `.ioc` | `configUSE_NEWLIB_REENTRANT = 1` | newlib 재진입 안전성 |
| `stm32h7xx_hal_conf.h` | `HAL_FDCAN_MODULE_ENABLED` 활성화 | HAL FDCAN 드라이버 컴파일 |
| `stm32h7xx_hal_fdcan.c/.h` | 신규 추가 | ST HAL FDCAN 드라이버 |

### 3. 소프트웨어 구조

```text
main()
 └ MX_FDCAN2_Init()            : 핸들 초기화(아직 Start 안 함)
 └ osKernelStart()
     └ CanLoopback_Create()    : RTOS_THREADS 훅에서 큐 + 태스크 생성
         ├ loopbackQueue       : osMessageQueue, depth 8 x sizeof(LoopbackFrame)
         └ LoopbackTask        : "CanLoopback", stack 2048 B, prio BelowNormal, one-shot

LoopbackTask
 ├ 안전 가드: Mode != INTERNAL_LOOPBACK 또는 format != CLASSIC 이면 즉시 fail
 │            → 실제 버스에 0x123을 절대 송신하지 않음
 ├ ConfigFilter / ConfigGlobalFilter / ConfigInterruptLines
 ├ ActivateNotification(RX_FIFO0_NEW_MESSAGE) / HAL_FDCAN_Start
 └ for seq in 0..99:
     ├ payload = { 'S','D','V', seq[7:0], seq[15:8], seq[23:16], seq[31:24], 0xA5 }
     ├ HAL_FDCAN_AddMessageToTxFifoQ(0x123, DLC8)      → tx_count++
     ├ osMessageQueueGet(timeout 100 ms)               → 실패 시 timeout_count++, fail
     ├ header/payload 비교(memcmp 8B)                  → 불일치 시 mismatch_count++, fail
     ├ GetErrorCounters / GetProtocolStatus
     │   TEC/REC/BusOff/queue_overflow/rx_error 중 하나라도 0 아니면 fail
     ├ pass_count++
     └ stack_free_bytes = osThreadGetStackSpace(); osDelay(100 ms)
 └ HAL_FDCAN_Stop(); state = 2 (PASS); osThreadExit()

HAL_FDCAN_RxFifo0Callback (ISR, FDCAN2_IT0_IRQHandler → HAL_FDCAN_IRQHandler)
 ├ irq_count++
 └ 최대 8개까지 bounded drain: GetRxMessage → osMessageQueuePut(timeout 0)
    비교/판정은 ISR에서 하지 않고 태스크로 넘김 (ISR 최소 처리 원칙)
```

`g_fdcan_loopback` (`volatile LoopbackStats`)가 pass/fail 오라클이다.
`state`: 0 idle / 1 running / 2 PASS / 3 FAIL.

### 4. 디버깅 / 실행 절차

- STM32CubeIDE에서 빌드 → ST-LINK로 실기(STM32H735G-DK) 플래시 → 디버거 실행.
- Expressions 뷰에 `g_fdcan_loopback` 등록, 루프 종료 후 값 확인.
- 디버그 설정도 이번에 함께 조정(`STM32H735G-DK.launch`):
  - `org.eclipse.cdt.dsf.gdb.NON_STOP` `true` → `false` (all-stop 모드).
  - reset 전략 `Reset` → `Software system reset` (schema v1), hardware/core reset 선택지 추가.

### 5. 관찰값 (현재 코드 기준, 실기 1회 실행)

환경: STM32H735G-DK, ST-LINK GDB server, `arm-none-eabi-gdb 14.2.90.20240526`,
디버거 Expressions 뷰에서 루프 종료 후(`state = 2`) `g_fdcan_loopback` 전체 필드 확인.

```text
Name : g_fdcan_loopback   (volatile LoopbackStats)
Details: { state = 2, tx_count = 100, rx_count = 100, pass_count = 100,
           mismatch_count = 0, timeout_count = 0, irq_count = 100,
           queue_overflow = 0, rx_error = 0, api_error = 0, last_hal_error = 0,
           tx_error_counter = 0, rx_error_counter = 0, bus_off = 0,
           stack_free_bytes = 1684 }
```

| 필드 | 값 | 해석 |
|---|---:|---|
| `state` | **2** | PASS (0 idle / 1 running / 2 PASS / 3 FAIL) |
| `tx_count` | **100** | `AddMessageToTxFifoQ` 성공 100회 |
| `rx_count` | **100** | 큐에서 프레임 100개 수신 |
| `pass_count` | **100** | 헤더+payload 일치 & 에러카운터 clean 100회 |
| `mismatch_count` | 0 | 헤더/payload 불일치 0 |
| `timeout_count` | 0 | 100 ms 내 RX 실패 0 |
| `irq_count` | **100** | RX FIFO0 new-message IRQ = 프레임당 정확히 1회 (coalescing 없음) |
| `queue_overflow` | 0 | ISR→태스크 큐(depth 8) 넘침 0 |
| `rx_error` | 0 | `HAL_FDCAN_GetRxMessage` 실패 0 |
| `api_error` | 0 | HAL API 실패 0 |
| `last_hal_error` | 0 | `HAL_FDCAN_GetError` = 0 |
| `tx_error_counter` (TEC) | 0 | 프로토콜 에러 없음 |
| `rx_error_counter` (REC) | 0 | 〃 |
| `bus_off` | 0 | bus-off 미발생 |
| `stack_free_bytes` | **1684** | `CanLoopback` 태스크 2048 B 중 최소 여유 1684 B → high-water 사용량 364 B, 여유 82 % |

### 6. 판정

```text
FDCAN2 INTERNAL LOOPBACK BENCH: PASS (state = 2, 100/100)
```

- 100회 전송 프레임이 모두 std ID `0x123` / DLC 8 / payload 일치로 되돌아옴.
- TEC/REC/BusOff = 0, 큐 오버플로 / RX 에러 0.
- `irq_count = 100`: RX 인터럽트가 프레임당 정확히 1회, ISR은 enqueue만 하고
  비교는 태스크에서 수행 → `REQ-HMI-014` (FDCAN ISR 최소 처리) 구조 확인.
- `CanLoopback` 태스크 stack 여유 1684 / 2048 B (사용 364 B) → 현재 스택 크기 충분.

미검증(다음 게이트):

- 실제 트랜시버 + 물리 CAN 버스 loopback / 노드 간 통신.
- 500 kbit/s가 SDV 백본 최종 bitrate인지 (Owner `DEC-NET-004~007` 확정 전이므로 벤치값).
- CAN signal / ID / DLC / cycle / timeout layout freeze 전이라 `0x123`은 벤치 전용 ID다.

### 7. 관련 파일

```text
firmware/IVI/SDV_IVI_H735/
├ SDV_IVI_H735.ioc
├ Core/Src/main.c                 : MX_FDCAN2_Init, CanLoopback_Create 호출
├ Core/Src/freertos.c             : LoopbackTask, HAL_FDCAN_RxFifo0Callback, g_fdcan_loopback
├ Core/Src/stm32h7xx_it.c(.h)     : FDCAN2_IT0_IRQHandler
├ Core/Src/stm32h7xx_hal_msp.c    : HAL_FDCAN_MspInit/DeInit (PB5/PB6, HSE 커널클럭, NVIC)
├ Core/Inc/stm32h7xx_hal_conf.h   : HAL_FDCAN_MODULE_ENABLED
├ Core/Inc/FreeRTOSConfig.h       : configUSE_NEWLIB_REENTRANT
└ Drivers/STM32H7xx_HAL_Driver/{Inc,Src}/stm32h7xx_hal_fdcan.{h,c}
```
