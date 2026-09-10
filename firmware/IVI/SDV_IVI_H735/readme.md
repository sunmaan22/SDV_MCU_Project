# STM32H735G-DK TBS

The default IDE is set to STM32CubeIDE, to change IDE open the SDV_IVI_H735.ioc with STM32CubeMX and select from the supported IDEs (EWARM from version 8.50.9, MDK-ARM, and STM32CubeIDE). Supports flashing of the STM32H735G-DK board directly from TouchGFX Designer using GCC and STM32CubeProgrammer. Flashing the board requires STM32CubeProgrammer which can be downloaded from the ST webpage.

This TBS is configured for 272 x 480 pixels 24bpp screen resolution.

Performance testing can be done using the GPIO pins designated with the following signals: 

 - VSYNC_FREQ  - CN8-D3 (PA0)
 - RENDER_TIME - CN8-D2 (PG3)
 - FRAME_RATE  - CN8-D1 (PB14)
 - MCU_ACTIVE  - CN8-D0 (PB15)
