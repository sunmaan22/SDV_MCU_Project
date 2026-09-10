# IVI Cluster Bring-up

## 목적

이 폴더는 STM32H735G-DK 기반 Cluster/IVI 노드의 재현 가능한 bring-up 기준을 기록한다. 실제 제품 프로젝트는 [../SDV_IVI_H735](../SDV_IVI_H735/)에 두며, 이 문서는 생성 코드가 아니라 설정·검증 기록을 보관한다.

## 현재 구성

```text
firmware/IVI/
├─ IVI_Cluster/
│  └─ README.md
└─ SDV_IVI_H735/
   ├─ SDV_IVI_H735.ioc
   ├─ TouchGFX/SDV_IVI_H735.touchgfx
   ├─ Core/
   ├─ Drivers/
   ├─ Middlewares/
   └─ STM32CubeIDE/
```

## 확인된 개발 환경

| 항목 | 버전 / 선택 | 상태 |
| --- | --- | --- |
| 보드 | STM32H735G-DK | 실보드 검증 |
| TouchGFX Designer / framework | 4.26.1 | `SDV_IVI_H735` 생성 및 코드 생성에 사용 |
| Display | 480 × 272, RGB888 / 24 bpp, Landscape | H735G-DK Board Setup 기본 구성 |
| STM32CubeIDE | 1.19.0 | 프로젝트 빌드·디버그 환경 |
| CubeIDE 내장 CubeMX | 6.15.0 | 현재 `.ioc` 편집에는 사용하지 않음 |
| 프로젝트 `.ioc` 생성 버전 | STM32CubeMX 6.18.0 | 이후 `.ioc` 편집에는 6.18 이상 standalone 필요 |
| STM32CubeProgrammer | 2.23.0 | Target flash에 사용 |
| ST-LINK | V3, firmware V3J17M11 | H735G-DK 연결 확인 |

> `SDV_IVI_H735.ioc`는 STM32CubeMX 6.18.0으로 생성됐다. CubeIDE 1.19의 내장 CubeMX 6.15.0으로 열거나 재생성하지 않는다. 주변장치 설정은 CubeMX 6.18 이상 standalone에서 수행하고, 빌드·디버그는 CubeIDE에서 수행한다.

## 지금까지의 진행 순서

1. 기존 `touchgfx-cmake-vscode-stm32-simulator` (TouchGFX 4.21.0)를 비교용 hardware reference로 보존했다.
2. 공식 STM32H735G-DK TouchGFX Board Setup과 Blank UI로 `SDV_IVI_H735` 프로젝트를 `firmware/IVI` 아래에 생성했다.
3. TouchGFX code/assets, STM32CubeIDE metadata, GCC target project를 생성했다.
4. GCC target을 build하여 `target.elf`, `target.hex`, `intflash.elf`, `intflash.hex` 생성을 확인했다.
5. 최초 flash 시 STM32CubeProgrammer 미설치 오류를 확인하고 v2.23.0을 설치했다.
6. STM32CubeProgrammer에서 ST-LINK와 H735G-DK target 연결을 확인했다.
7. TouchGFX Designer의 **Program and Run Target**을 다시 실행해 target을 flash했다.
8. 보드에서 TouchGFX 4.26.1 baseline UI 실행을 확인했다.

## 현재 검증 결과

| Gate | 결과 |
| --- | --- |
| TouchGFX code generation | PASS |
| Asset generation | PASS |
| GCC compile / link | PASS |
| Internal + external HEX 생성 | PASS |
| ST-LINK target 연결 | PASS |
| TouchGFX Designer flash | PASS |
| H735G-DK에서 4.26.1 baseline 실행 | PASS |

현재의 빈 `Screen1`은 의도된 platform baseline이다. 아직 완성 UI가 아니다.

## 다음 Gate: FDCAN2

다음 변경은 FDCAN2 peripheral 활성화와 배선 검증으로 제한한다. Network contract 값은 아직 고정하지 않는다.

| 항목 | 계획 |
| --- | --- |
| Peripheral | FDCAN2 |
| RX | PB5 (`FDCAN2_RX`) |
| TX | PB6 (`FDCAN2_TX`) |
| 보드 CAN 포트 | CN17 |
| 내장 transceiver | MCP2562FDT CAN FD |
| 종단저항 | JP5 / 120 Ω, bus 끝단일 때만 사용 |
| Bitrate, BRS, CAN ID, DLC, payload | TBD — Network Freeze 이후 확정 |

```text
FDCAN2 pin mapping
→ internal loopback
→ CN17 physical CAN test
→ CanRxTask / VehicleModelTask / CommandTxTask
→ TouchGFX dummy vehicle data
→ frozen CAN contract와 real vehicle data 통합
```

## 작업 규칙

- CubeMX generated 영역은 지원되는 generation flow로만 변경한다.
- CAN decode, vehicle state, DTC, warning은 TouchGFX generated View에 직접 작성하지 않는다.
- FDCAN ISR은 queue/notification만 수행하고 decode와 rendering은 task에서 처리한다.
- 4.21 reference는 변경하지 않고 known-good 비교 기준으로 유지한다.
- build 디렉터리, 복사된 TouchGFX framework, debug output은 커밋하지 않는다.
