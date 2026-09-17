# RaspberryPi_TX

RF 송신측(조종기) 개발용 폴더. C ECU의 `Driver_Input`(STM32, 수신측)과 짝을 이루는 송신측 코드이며, 별도 ECU가 아니라 벤치용 컨트롤러다. STM32 쪽 가이드는 [DRIVER_INPUT_START.md](../../../docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md) 참고.

## 언어: C++

STM32 쪽과 같은 언어로 맞추고, 외부 GPIO/SPI 라이브러리(WiringPi, libgpiod 등) 없이 Linux 커널 uAPI를 직접 호출한다.

- SPI: `<linux/spi/spidev.h>` + `ioctl(SPI_IOC_MESSAGE(1), ...)`
- CE: `<linux/gpio.h>` GPIO character device **v2 uAPI** (`GPIO_V2_GET_LINE_IOCTL`, `GPIO_V2_LINE_SET_VALUES_IOCTL`) — v1 handle API는 최신 커널에서 deprecated라 v2로 구현

## 빌드 / 실행

```bash
make
./nrf24_bringup
```

의존성은 표준 g++/make뿐이다 (추가 apt 패키지 불필요).

## 현재 상태 (2026-09-17)

- 라즈베리파이: `pi14.local`, Debian(Trixie 계열), g++ 14.2, kernel 6.18 (GPIO v2 uAPI 사용 가능)
- SPI 커널 인터페이스 활성화 완료(`sudo raspi-config` → Interface Options → SPI), `/dev/spidev0.0` 확인됨
- nRF24L01 모듈 미배선. CE/CSN/SCK/MOSI/MISO 핀 배정 전부 TBD (CE는 임시로 BCM 22)
- `nrf24.hpp`/`nrf24.cpp`: STM32 `Core/Src/main.c`와 동일한 R_REGISTER/W_REGISTER 커맨드 레벨 드라이버
- `main.cpp`: STM32 T-RF-000 이후 bench 시험과 동일한 STATUS/CONFIG read-write-read 확인
- 실행 결과: 예외 없이 완료, `STATUS=0x00 CONFIG(before)=0x00 CONFIG(after)=0x00` (모듈 미연결 상태의 예상값 — SPI0 MISO(GPIO9)에 기본 pull-down이 있어 STM32의 `0xFF`와 달리 `0x00`으로 읽힘. 둘 다 "버스는 정상, 슬레이브 없음"과 동일한 의미)
- 아직 미구현: 실제 gear/steering/speed 입력 읽기, 패킷 인코딩, 실제 송신 루프

## 다음 할 일

1. nRF24L01 모듈 전원/SPI 배선 (STM32 쪽과 동일하게 CE/CSN 핀 확정 후 양쪽 문서에 기록)
2. `./nrf24_bringup`으로 STATUS/CONFIG 레지스터 read/write 확인 (STM32 쪽 bench 시험과 동일한 절차, [TEST_REPORT.md](../../../docs/ecus/Motor_Steering_Control/TEST_REPORT.md) 참고)
3. 패킷 포맷(`version`/`sequence`/`gear`/`steering_request`/`speed_request`/`input_valid`) 확정 — STM32 쪽과 공동 결정 사항이며 byte 배치/단위/endianness를 명시적으로 정한다. C 구조체를 그대로 덤프해서 보내지 않는다
