# Documentation Guide

> 기준: **Architecture v1.2 + RTOS Development Policy / 2026-09-09**  
> 목적: 문서를 최소화하면서도 각 담당자가 `무엇을 만들지`, `어떻게 나눌지`, `어떤 Task가 언제 돌아야 하는지`까지 같은 기준으로 설계한다.

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

## 2.2 Node별 적용

| 담당 | Node | 실행 환경 |
|---|---|---|
| A | Ultrasonic STM32 | FreeRTOS |
| B | STM32H735 Cluster + IVI | FreeRTOS + TouchGFX |
| C | Drive + Steering STM32 | FreeRTOS |
| D | Body Gateway STM32 | FreeRTOS |
| D | Body LIN Slave STM32 | FreeRTOS 기본, MCU 자원 부족 시 예외 검토 |
| E | Raspberry Pi Vision/HPC | Linux, RTOS 미적용 |
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

# 4. 각 담당자가 작성할 문서

각 기능 폴더에는 아래 세 파일만 둔다.

```text
SPECIFICATION.md
ARCHITECTURE.md
TEST_REPORT.md
```

- `SPECIFICATION.md`: 무엇을 해야 하는가, Timing/RTOS 요구사항 포함
- `ARCHITECTURE.md`: Component + Task + ISR + Queue + Runtime 구조
- `TEST_REPORT.md`: 기능 검증 + Timing/Jitter/Stack/Queue/Watchdog 검증

현재 채운 예시:

- **B / Cluster + IVI:** [`IVI/`](IVI/)
- **A / Ultrasonic Perception:** [`Ultrasonic_Perception/`](Ultrasonic_Perception/)

각 예시의 `TBD`와 `NOT RUN`은 실제 부품 선정/구현/시험 후 담당자가 채운다.

---

# 5. 기능 명세서 기준

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
- **Execution / RTOS Requirements**
- Safety / Fail-safe / DTC
- Acceptance Criteria
- Revision / TBD

MCU 기능의 경우 `RTOS Task가 무엇인지` 자체보다 먼저 **왜 Task 분리가 필요한지와 주기/Deadline 요구사항**을 명세한다.

---

# 6. Software Architecture 기준

`SOFTWARE_ARCHITECTURE_TEMPLATE.md`는 기존 Context/Component/Runtime/Deployment View에 더해 RTOS Node에서 다음을 반드시 다룬다.

- Task Model
- Task Priority와 Period/Trigger
- ISR → Task 연결
- Queue / Task Notification / Event / Mutex
- Shared Resource Owner
- Watchdog / Health Monitoring
- Stack / Heap 정책
- Deadline / Overrun 처리
- Priority inversion / starvation 위험
- Requirement → Component/Task → Test Traceability

즉 MCU Architecture는 아래 두 그림이 모두 있어야 한다.

```text
Component View
무엇이 무엇을 책임지는가?
```

```text
RTOS / Runtime View
어떤 Task가 언제 실행되고 어떻게 데이터를 넘기는가?
```

---

# 7. 문서 작성 규칙

1. 같은 내용을 여러 파일에 복붙하지 않는다.
2. 값이 미정이면 `TBD`로 둔다.
3. Candidate period는 `10 ms 후보`처럼 확정값과 구분한다.
4. ISR에서 긴 연산, blocking I/O, printf를 하지 않는다.
5. Task table에 Priority만 적고 이유가 없으면 설계가 덜 끝난 것이다.
6. RTOS를 사용해도 Hard Real-Time loop의 Deadline을 자동으로 보장해주는 것은 아니다. 실제 측정한다.
7. Pi는 Linux이므로 FreeRTOS 문서 형식을 억지로 적용하지 않는다. 대신 Process/Thread/Queue/Service 구조를 Architecture에 적는다.
8. 설계 결정이 중요하면 ADR에 남긴다.

---

# 8. Legacy

- `archive/README_2026-09-08_legacy.md`
- `archive/legacy_v1.2_before_full_sync_2026-09-09/`
- `archive/legacy_v1.2_before_doc_cleanup_2026-09-09/`

현재 개발에서는 archive 문서를 기준으로 사용하지 않는다.

[Main README](../README.md)
