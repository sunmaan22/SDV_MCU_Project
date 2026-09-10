# STM32H735G-DK TouchGFX Reference Project

[IVI 문서 홈](README.md) · [Bring-up](HARDWARE_BRINGUP.md) · [핀맵](PIN_MAP.md)

## 1. Reference

- Repository: `MaJerle/touchgfx-cmake-vscode-stm32-simulator`
- Branch: `master`
- Reviewed commit: `f5c3b1ee03992cfd2d98587da270d9b3f9edd82a`
- Target board file: `STM32H735G-DK.ioc`

이 프로젝트의 목적은 VSCode/CMake 기반 TouchGFX simulator 개발 예제다. 우리 SDV IVI의 완성 구조와 목적은 다르므로 전체 소스를 최종 코드 베이스로 복제하지 않는다.

## 2. 가져올 설정

Reference에서 확인하고 우리 H735G-DK bring-up에 재사용할 항목:

```text
STM32H735G-DK pin assignment
LTDC 480x272 RGB888
DMA2D
OCTOSPI1 external Flash
OCTOSPI2 HyperRAM
TouchGFX framebuffer placement
Cortex-M7 MPU / I-Cache / D-Cache
FreeRTOS + TouchGFX task 기본 구조
Clock tree 기준
```

Reference에서 확인된 framebuffer 기준:

```text
Framebuffer 0 = 0x70000000
Framebuffer 1 = 0x70060000
Double buffering
LTDC display interface
DMA2D hardware accelerator
```

## 3. 그대로 가져오지 않을 항목

```text
Application UI design
예제용 화면 로직
예제 CMake/VSCode 구성을 프로젝트 표준으로 강제
구버전 CubeMX/TouchGFX 설정을 검토 없이 migration
SDV와 무관한 example code
```

## 4. Version 주의

Reference `.ioc`는 다음 generation 환경을 기록하고 있다.

```text
STM32CubeMX 6.5.0
STM32CubeH7 1.10.0
TouchGFX 4.21.0
```

현재 개발 PC의 CubeMX / CubeH7 / TouchGFX가 더 최신이면 migration 후 다음을 반드시 재검증한다.

- clock tree
- LTDC timing
- OCTOSPI timing
- HyperRAM memory mapping
- MPU attributes
- framebuffer addresses
- Touch BSP
- FreeRTOS generated code

## 5. Local Reference 사용 방법

Reference는 프로젝트 외부에 clone해서 사용한다.

```bash
git clone https://github.com/MaJerle/touchgfx-cmake-vscode-stm32-simulator.git
cd touchgfx-cmake-vscode-stm32-simulator
git checkout f5c3b1ee03992cfd2d98587da270d9b3f9edd82a
```

그 뒤 `STM32H735G-DK.ioc`와 실제 generated/BSP 코드를 비교 기준으로 사용한다.

## 6. 우리 프로젝트로 옮길 때의 원칙

```text
External reference
    ↓ 확인
Board hardware configuration
    ↓ 재구성
Our STM32H735G-DK IVI project
    ↓ 추가
FDCAN2 + SDV tasks + VehicleDataRepository
```

즉 source tree를 복사하는 방식이 아니라, board bring-up에 필요한 설정을 검증해서 우리 프로젝트에 적용한다.

## 7. License 주의

확인 시점의 repository metadata에서는 명시적인 repository license가 확인되지 않았다. 따라서 해당 저장소 코드를 우리 repository에 대량 복사하거나 재배포하는 방식은 피하고, reference/configuration 비교 용도로 사용한다.
