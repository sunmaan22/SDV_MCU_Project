# IVI 개발일지 (Development Log)

[프로젝트 홈](../../../README.md) · [문서 안내](../../README.md) · [폴더 목록](README.md)

> 목적: STM32H735G-DK IVI 펌웨어의 단계별 구현/디버깅 과정과 실기에서 관찰한 값을 날짜순으로 남긴다.
> 확정된 시험 판정은 [TEST_REPORT.md](TEST_REPORT.md), 구조는 [ARCHITECTURE.md](ARCHITECTURE.md)를 따른다. 이 문서는 작업 로그다.

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
