# STM32H735G-DK IVI Pin Map

[IVI 문서 홈](README.md) · [Bring-up](HARDWARE_BRINGUP.md) · [Reference Project](REFERENCE_PROJECT.md)

> 목적: TouchGFX/LCD/외부 메모리와 충돌하지 않도록 IVI에서 예약해야 할 핀과 SDV CAN 핀을 한 곳에 정리한다.

## 1. SDV IVI에서 직접 사용하는 외부 인터페이스

| 기능 | Peripheral | Pin | 비고 |
|---|---|---|---|
| SDV CAN RX | FDCAN2_RX | `PB5` | IVI backbone 기본안 |
| SDV CAN TX | FDCAN2_TX | `PB6` | IVI backbone 기본안 |
| Touch SCL | I2C4_SCL | `PF14` | board touch controller |
| Touch SDA | I2C4_SDA | `PF15` | board touch controller |
| Touch IRQ | GPIO/EXTI | `PG2` | BSP와 충돌 금지 |
| SWDIO | SYS Debug | `PA13` | ST-LINK |
| SWCLK | SYS Debug | `PA14` | ST-LINK |

## 2. LTDC RGB888 예약 핀

### Red

| Signal | Pin |
|---|---|
| R0 | PE0 |
| R1 | PH3 |
| R2 | PH8 |
| R3 | PH9 |
| R4 | PH10 |
| R5 | PH11 |
| R6 | PE1 |
| R7 | PE15 |

### Green

| Signal | Pin |
|---|---|
| G0 | PB1 |
| G1 | PB0 |
| G2 | PA6 |
| G3 | PE11 |
| G4 | PH15 |
| G5 | PH4 |
| G6 | PC7 |
| G7 | PD3 |

### Blue

| Signal | Pin |
|---|---|
| B0 | PG14 |
| B1 | PD0 |
| B2 | PD6 |
| B3 | PA8 |
| B4 | PE12 |
| B5 | PA3 |
| B6 | PB8 |
| B7 | PB9 |

### LTDC Control

| Signal | Pin |
|---|---|
| DE | PE13 |
| HSYNC | PC6 |
| VSYNC | PA4 |
| Pixel Clock | PG7 |

## 3. LCD Control 예약 핀

| 기능 | Pin | 주의 |
|---|---|---|
| LCD DISP | PD10 | GPIO output / BSP |
| LCD Backlight Control | PG15 | GPIO output / BSP |
| LCD Reset | PH6 | BSP 사용 가능, 다른 기능 재할당 금지 |

## 4. OCTOSPI1 External Flash 예약 핀

TouchGFX image/font/asset 저장용 external NOR Flash 영역이다.

| Signal | Pin |
|---|---|
| NCS | PG6 |
| CLK | PF10 |
| DQS | PB2 |
| IO0 | PD11 |
| IO1 | PD12 |
| IO2 | PE2 |
| IO3 | PD13 |
| IO4 | PD4 |
| IO5 | PD5 |
| IO6 | PG9 |
| IO7 | PD7 |

## 5. OCTOSPI2 HyperRAM 예약 핀

TouchGFX framebuffer / external RAM 영역이다.

| Signal | Pin |
|---|---|
| NCS | PG12 |
| CLK | PF4 |
| DQS/RWDS | PF12 |
| IO0 | PF0 |
| IO1 | PF1 |
| IO2 | PF2 |
| IO3 | PF3 |
| IO4 | PG0 |
| IO5 | PG1 |
| IO6 | PG10 |
| IO7 | PG11 |

## 6. Clock / Debug 예약

| 기능 | Pin |
|---|---|
| HSE OSC_IN | PH0 |
| HSE OSC_OUT | PH1 |
| SWDIO | PA13 |
| SWCLK | PA14 |

## 7. IVI에서 사용하지 않는 직접 제어 핀

다음 기능은 H735 IVI에서 핀을 확보하지 않는다.

```text
Ultrasonic TRIG/ECHO
Accelerator ADC
Brake ADC
Steering input ADC/encoder
Motor PWM
Motor direction GPIO
Servo PWM
LIN TX/RX
Lamp output GPIO/PWM
Camera DCMI
```

이 기능들은 각 담당 ECU가 처리하고 H735에는 CAN logical message로 전달한다.

## 8. CubeMX 핀 충돌 규칙

새 peripheral을 추가할 때 다음 순서로 확인한다.

```text
1. 이 문서 예약 핀 여부 확인
2. CubeMX conflict 표시 확인
3. Board schematic / BSP 확인
4. TouchGFX build & flash 재검증
5. 새 peripheral 단독 테스트
```

특히 `PF14`, `PF15`, `PG2`, `PH6`은 CubeMX 화면만 보고 free GPIO로 판단하지 않는다.

## 9. 현재 IVI Pin Freeze 후보

| 항목 | 후보값 | 상태 |
|---|---|---|
| IVI MCU | STM32H735G-DK / STM32H735IGKx | 적용 기준 |
| SDV CAN peripheral | FDCAN2 | 후보 Freeze |
| SDV CAN RX | PB5 | 후보 Freeze |
| SDV CAN TX | PB6 | 후보 Freeze |
| Touch | Board BSP / I2C4 | 유지 |
| Display | LTDC RGB888 | 유지 |
| Asset memory | OCTOSPI1 | 유지 |
| Framebuffer memory | OCTOSPI2 HyperRAM | 유지 |

CAN bitrate, BRS, ID, DLC, timeout은 `FINAL_IMPLEMENTATION_SPEC.md`의 Network Freeze를 따른다.
