# IVI Firmware Workspace

이 폴더는 STM32H735G-DK 기반 Cluster/IVI 펌웨어의 작업 위치다.

현재 단계에서는 외부 TouchGFX reference source를 복사하지 않는다. 로컬에서 H735G-DK reference GUI를 실제 보드에 flash해 검증한 뒤, 동일한 board 설정을 기반으로 우리 `.ioc`와 generated project를 이 폴더에 추가한다.

## 목표 구조

```text
firmware/IVI/
├ STM32H735G-DK_IVI.ioc
├ Core/
├ Drivers/
├ Middlewares/
├ TouchGFX/
└ App/
   ├ communication/
   │  ├ can_rx/
   │  ├ can_tx/
   │  └ signal_codec/
   ├ vehicle_data/
   ├ validity/
   ├ warning/
   ├ diagnostics/
   ├ rtos/
   │  ├ ipc/
   │  └ task_health/
   └ platform/
      └ dummy_data/
```

## Generated code 규칙

- CubeMX generated HAL/BSP 영역과 application 코드를 분리한다.
- `USER CODE BEGIN/END` 밖의 generated source를 수동 수정하지 않는다.
- CAN codec, Repository, Warning, DTC logic을 TouchGFX generated View 파일에 직접 작성하지 않는다.
- TouchGFX widget 접근은 GUI task/context로 제한한다.
- FDCAN ISR에서 application decode 또는 rendering을 수행하지 않는다.

## Bring-up 순서

자세한 절차는 `../../docs/ecus/IVI/HARDWARE_BRINGUP.md`를 따른다.

```text
1. H735G-DK LCD/Touch reference PASS
2. 우리 STM32H735G-DK_IVI.ioc 생성
3. LTDC/DMA2D/OCTOSPI1/OCTOSPI2/MPU 검증
4. FDCAN2 PB5/PB6 추가
5. Internal loopback PASS
6. Physical CAN PASS
7. CanRxTask / VehicleModelTask / CommandTxTask 추가
8. TouchGFX DummyDataProvider 연결
9. 실제 CAN VehicleDataRepository 연결
```

## 현재 Coding Gate

CAN bit timing / CAN ID / DLC / payload layout은 `docs/system/FINAL_IMPLEMENTATION_SPEC.md`의 Network Freeze 이후 코드에 확정한다.
