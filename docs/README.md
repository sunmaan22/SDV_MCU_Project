# Documentation Guide

> 기준: **Architecture v1.2 + RTOS Development Policy / 2026-09-09**  
> 목적: 문서를 최소화하면서도 각 담당자가 `무엇을 만들지`, `어떻게 나눌지`, `어떤 Task/Service가 언제 돌아야 하는지`까지 같은 기준으로 설계한다.

# 1. 현재 활성 문서

```text
docs/
├ README.md
├ TEAM_GUIDE.md
├ PROJECT_REFERENCE.md
├ WEEKLY_PLAN.md
├ IVI/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
├ Ultrasonic_Perception/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
├ Motor_Steering_Control/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
├ Lighting_Ambient_LIN_CAN/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
├ HPC_Camera_Vision/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
├ VCU_DTC_CAN_Integration/
│  ├ README.md
│  ├ SPECIFICATION.md
│  ├ ARCHITECTURE.md
│  └ TEST_REPORT.md
└ templates/
   ├ FUNCTIONAL_SPECIFICATION_TEMPLATE.md
   ├ SOFTWARE_ARCHITECTURE_TEMPLATE.md
   └ TEST_REPORT_TEMPLATE.md
```

과거 문서는 `docs/archive/`에만 보존한다.

---

# 2. 프로젝트 RTOS 기본 정책

## 2.1 한 줄 원칙

```text
STM32 Node → FreeRTOS 기본
Raspberry Pi → Linux Service / Process / Thread
```

STM32에서는 가능한 경우 **FreeRTOS Kernel + CMSIS-RTOS2 API**를 기본 구조로 사용한다. STM32CubeMX/STM32CubeIDE에서 생성되는 설정과 사용 보드의 실제 RAM/Flash가 최종 기준이다.

단, RTOS를 넣는 목적은 Task 숫자를 늘리는 것이 아니다.

```text
주기 제어
이벤트 처리
CAN / LIN 통신
UI
진단 / Watchdog
```

을 서로 방해하지 않게 분리하고, 실행 주기와 우선순위를 설명 가능하게 만드는 것이 목적이다.

Raspberry Pi는 Linux이므로 FreeRTOS 형식을 억지로 적용하지 않는다. 대신 **Service / Process / Thread / IPC / Queue / Health / Restart** 구조를 설계한다.

## 2.2 Node별 적용

| 담당 | Node | 실행 환경 |
|---|---|---|
| A | Ultrasonic STM32 | FreeRTOS |
| B | STM32H735 Cluster + IVI | FreeRTOS + TouchGFX |
| C | Drive + Steering STM32 | FreeRTOS |
| D | Body Gateway STM32 | FreeRTOS |
| D | Body LIN Slave STM32 | FreeRTOS 기본, MCU 자원 부족 시 예외 검토 |
| E | Raspberry Pi Vision/HPC | Linux Service / Process / Thread |
| F | VCU STM32 | FreeRTOS |

작은 LIN Slave도 프로젝트 기본안은 FreeRTOS 사용으로 잡는다. 다만 최종 MCU가 너무 작은 경우 RAM/Flash 측정 결과를 근거로 Bare-metal 예외를 허용할 수 있다. 예외는 `ARCHITECTURE.md`의 Architecture Decision에 이유를 남긴다.

---

# 3. RTOS 공통 설계 규칙

1. **ISR은 짧게 끝낸다.** Timestamp/flag 저장 후 Task Notification 또는 Queue로 Task를 깨운다.
2. 주기 Task는 가능하면 `osDelayUntil()` 또는 `vTaskDelayUntil()` 계열로 주기를 관리한다.
3. Task 간 데이터 전달은 전역변수 난사보다 **Queue / Task Notification / Event Flags**를 우선한다.
4. Mutex는 실제 공유 자원 보호가 필요한 곳에만 사용한다.
5. Motor/VCU 같은 중요한 Task가 UART log, printf, UI 때문에 Block되지 않게 한다.
6. Scheduler 시작 후 불필요한 동적 메모리 할당을 피하고, 가능하면 Static Allocation을 검토한다.
7. 각 Task는 `Period / Trigger / Priority / Deadline / Stack`을 문서화한다.
8. Queue overflow, Task starvation, Stack overflow를 검출하거나 시험한다.
9. MCU Node는 **Health Monitor + Independent Watchdog** 구조를 권장한다. 모든 중요 Task가 정상일 때만 Watchdog refresh를 허용하는 방향으로 설계한다.
10. 정확한 Task period와 Priority 숫자는 처음부터 감으로 확정하지 않고 실제 Timing Test 후 조정한다.

## 우선순위 기본 방향

```text
Safety / Hard Real-Time Control
        ↓
Critical CAN RX / Command handling
        ↓
Sensor / State / Gateway processing
        ↓
UI model / Status transmission
        ↓
Diagnostics / Logging
```

FreeRTOS numeric priority는 각 Node의 Task 수와 실제 측정 후 정한다.

---

# 4. Linux HPC 공통 설계 규칙

Raspberry Pi Vision/HPC는 다음을 중심으로 본다.

1. Camera Capture와 무거운 Vision Processing이 서로 불필요하게 block되지 않게 한다.
2. Frame/Result Queue는 bounded 구조를 사용한다.
3. backlog가 생기면 오래된 frame을 계속 처리하기보다 **최신성(Freshness)** 을 우선한다.
4. CAN interface는 가능하면 한 Service가 소유하고 다른 기능은 IPC로 요청한다.
5. Front/Rear Vision은 독립 개발/시험 가능하도록 분리한다.
6. Process crash, Camera disconnect, CAN failure를 Health 상태로 만들고 recovery/restart 정책을 둔다.
7. FPS, processing latency, CPU, memory, thermal, queue occupancy를 실제 측정한다.
8. Raw Camera Frame은 CAN FD로 전송하지 않는다.

---

# 5. 각 담당자가 작성할 문서

각 기능 폴더에는 아래 세 파일을 기준으로 둔다.

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

- `SPECIFICATION.md`: 무엇을 해야 하는가, Timing/RTOS/Linux 실행 요구사항 포함
- `ARCHITECTURE.md`: Component + Task/Service + ISR/IPC + Runtime 구조
- `TEST_REPORT.md`: 기능 검증 + Timing/Resource/Health 검증

현재 채운 예시:

- **A / Ultrasonic Perception:** [`Ultrasonic_Perception/`](Ultrasonic_Perception/)
- **B / Cluster + IVI:** [`IVI/`](IVI/)
- **C / Motor + Steering Control:** [`Motor_Steering_Control/`](Motor_Steering_Control/)
- **D / Lighting + Ambient / LIN-CAN:** [`Lighting_Ambient_LIN_CAN/`](Lighting_Ambient_LIN_CAN/)
- **E / HPC + Camera Vision:** [`HPC_Camera_Vision/`](HPC_Camera_Vision/)
- **F / VCU + DTC + CAN Integration:** [`VCU_DTC_CAN_Integration/`](VCU_DTC_CAN_Integration/)

이제 A~F 전체 역할에 대해 채운 예시가 있다. 각 예시의 `TBD`, `후보`, `NOT RUN`은 실제 부품 선정/구현/시험 후 담당자가 채운다.

---

# 6. 기능 명세서 기준

`FUNCTIONAL_SPECIFICATION_TEMPLATE.md`는 다음을 포함한다.

- Feature ID / Name
- Scope / 제외 범위
- Usage/System Scenario
- Functional Flow
- Input / Output
- Functional Requirements
- Rules / Edge Cases
- UI/UX Reference
- Hardware/CAN/LIN/API Interface
- Timing / Performance
- Execution / RTOS 또는 Linux Service Requirements
- Safety / Fail-safe / DTC
- Acceptance Criteria
- Revision / TBD

MCU 기능은 Task와 Timing 요구사항을, Linux HPC 기능은 Process/Service/Thread/IPC/Restart 요구사항을 명확히 적는다.

---

# 7. Software Architecture 기준

`SOFTWARE_ARCHITECTURE_TEMPLATE.md`의 공통 관점:

- Context / Scope
- Building Block / Component
- Runtime
- Deployment
- Interface Contract
- Data / State
- Quality / Risk
- Architecture Decision
- Requirement Traceability

RTOS Node에서는 추가로:

- Task Model
- Task Priority와 Period/Trigger
- ISR → Task 연결
- Queue / Notification / Event / Mutex
- Watchdog / Stack / Heap

Linux HPC Node에서는 추가로:

- Service / Process / Thread
- IPC / Queue
- Shared resource owner
- Supervisor / Restart
- Frame/Result freshness
- CPU / Memory / Thermal / FPS / Latency

즉 설계서에는 **무엇으로 나눴는지**와 **실행 중 어떻게 협력하는지**가 둘 다 보여야 한다.

---

# 8. 문서 작성 규칙

1. 같은 내용을 여러 파일에 복붙하지 않는다.
2. 값이 미정이면 `TBD`로 둔다.
3. Candidate period는 `10 ms 후보`처럼 확정값과 구분한다.
4. ISR에서 긴 연산, blocking I/O, printf를 하지 않는다.
5. Task/Service 이름만 적고 책임과 입력/출력이 없으면 설계가 덜 끝난 것이다.
6. RTOS를 사용해도 Deadline을 자동으로 보장해주는 것은 아니다. 실제 측정한다.
7. Linux를 사용해도 Process를 많이 쪼갠다고 좋은 Architecture가 되는 것은 아니다. Failure isolation과 IPC 비용을 같이 본다.
8. 설계 결정이 중요하면 ADR에 남긴다.

---

# 9. Legacy

- `archive/README_2026-09-08_legacy.md`
- `archive/legacy_v1.2_before_full_sync_2026-09-09/`
- `archive/legacy_v1.2_before_doc_cleanup_2026-09-09/`

현재 개발에서는 archive 문서를 기준으로 사용하지 않는다.

[Main README](../README.md)
