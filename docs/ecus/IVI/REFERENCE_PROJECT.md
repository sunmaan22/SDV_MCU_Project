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

2026-09-10 실제 bring-up 검증 환경:

```text
STM32CubeIDE 1.19.0
STM32CubeMX 6.15.0
STM32CubeH7 V1.10.0 compatibility mode
X-CUBE-TOUCHGFX 4.21.0
```

검증 시 처음 `.ioc`를 열었을 때 `Migrate` 대신 `Continue`를 선택하여 기존 STM32CubeH7 V1.10.0 설정을 유지했다.

향후 current CubeH7 package로 migration할 경우 다음을 반드시 재검증한다.

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

## 6. 실제 Bring-up 결과

2026-09-10 STM32H735G-DK 실기 시험에서 다음을 확인했다.

```text
[x] Build: 0 errors, 0 warnings
[x] ST-LINK flash / run
[x] LCD backlight 정상 점등
[x] TouchGFX Reference 화면 정상 표시
[x] Touch 입력 시 UI 정상 반응
```

따라서 Reference project는 현재 환경에서 **board configuration 기준으로 사용 가능**하다고 판정한다.

## 7. Build 시 확인된 주의사항

### 7.1 TouchGFX package

`.ioc`를 열 때 `STMicroelectronics.X-CUBE-TOUCHGFX.4.21.0`이 로컬에 없으면 해당 package를 설치해야 한다.

### 7.2 FreeRTOS newlib warning

Code generation 시 `USE_NEWLIB_REENTRANT` 경고가 발생할 수 있다. 검증 환경에서는 다음을 활성화한 뒤 생성했다.

```text
FreeRTOS
→ Advanced Settings
→ USE_NEWLIB_REENTRANT = Enabled
```

### 7.3 TouchGFX linker search path

Reference `STM32CubeIDE/.cproject`에는 원 개발 PC의 TouchGFX library 절대경로가 포함되어 있다. 다른 PC에서는 최종 link 단계에서 다음 오류가 발생할 수 있다.

```text
cannot find -l:libtouchgfx-float-abi-hard.a
```

라이브러리 파일 자체는 repository 내부에 존재하므로 `MCU G++ Linker`의 library search path를 repository-relative path로 맞춘다.

```text
../../Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc
```

`Libraries (-l)`의 다음 항목은 유지한다.

```text
:libtouchgfx-float-abi-hard.a
```

수정 후 실제 build가 `0 errors, 0 warnings`로 완료되었다.

### 7.4 CubeIDE serial-port provider warning

실행 중 아래 IDE 로그가 관찰될 수 있다.

```text
org.xml.sax.SAXParseException: Premature end of file
com.st.stm32cube.ide.mpu.remote.serial.internal.SerialPortProviderWindows
```

현재 검증에서는 이 로그가 발생해도 ST-LINK flash, MCU 실행, LCD 및 TouchGFX 동작은 정상이었다. 따라서 firmware fault가 아닌 **IDE tooling warning**으로 분리 기록한다. UART/serial 기능을 실제 사용하기 전에는 별도 원인 확인이 필요하다.

## 8. 우리 프로젝트로 옮길 때의 원칙

```text
External reference
    ↓ 검증 완료
Board hardware configuration
    ↓ 재구성
Our STM32H735G-DK IVI project
    ↓ 추가
FDCAN2 + SDV tasks + VehicleDataRepository
```

즉 source tree를 복사하는 방식이 아니라, board bring-up에 필요한 설정을 검증해서 우리 프로젝트에 적용한다.

현재 다음 구현 Gate는 **우리 IVI 프로젝트에서 TouchGFX board bring-up 재현 → FDCAN2 PB5/PB6 추가 → internal loopback**이다.

## 9. License 주의

확인 시점의 repository metadata에서는 명시적인 repository license가 확인되지 않았다. 따라서 해당 저장소 코드를 우리 repository에 대량 복사하거나 재배포하는 방식은 피하고, reference/configuration 비교 용도로 사용한다.
