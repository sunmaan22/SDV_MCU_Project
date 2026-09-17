# RaspberryPi_TX

RF 송신측(조종기) 개발용 폴더. C ECU의 `Driver_Input`(STM32, 수신측)과 짝을 이루는 송신측 코드이며, 별도 ECU가 아니라 벤치용 컨트롤러다. STM32 쪽 가이드는 [DRIVER_INPUT_START.md](../../../docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md) 참고.

## 언어: C++

STM32 쪽과 같은 언어로 맞추고, 외부 GPIO/SPI 라이브러리(WiringPi, libgpiod 등) 없이 Linux 커널 uAPI를 직접 호출한다.

- SPI: `<linux/spi/spidev.h>` + `ioctl(SPI_IOC_MESSAGE(1), ...)`
- CE: `<linux/gpio.h>` GPIO character device **v2 uAPI** (`GPIO_V2_GET_LINE_IOCTL`, `GPIO_V2_LINE_SET_VALUES_IOCTL`) — v1 handle API는 최신 커널에서 deprecated라 v2로 구현

## 빌드 / 실행

```bash
make            # nrf24_bringup, tx_loop, packet_selftest 전부 빌드
make test       # packet_selftest만 실행 (하드웨어 불필요)
./nrf24_bringup # SPI 레지스터 read/write bench 확인
./tx_loop       # 더미 입력값으로 패킷 인코딩 + 송신 반복 (Ctrl+C 종료)
```

의존성은 표준 g++/make뿐이다 (추가 apt 패키지 불필요).

## 파일 구성

| 파일 | 내용 | 하드웨어 필요? |
|---|---|---|
| `nrf24.hpp` / `nrf24.cpp` | STM32 `Core/Src/main.c`와 동일한 커맨드의 레지스터 read/write + PTX 초기화(`InitAsTransmitter`) + 패킷 송신(`SendPayload`) | SPI 버스 자체는 불필요, 실제 응답 확인만 필요 |
| `packet.hpp` | Driver_Input 패킷 초안 Encode/Decode (구조체 덤프 금지, 명시적 byte 배치) | 불필요 |
| `packet_selftest.cpp` | Encode→Decode 왕복 검증 | **불필요** (`make test`) |
| `main.cpp` | STM32 T-RF-000 이후 bench 시험과 동일한 STATUS/CONFIG read-write-read | SPI 버스만(모듈 없어도 실행 가능) |
| `tx_loop.cpp` | 더미 steering 값을 왕복시키며 주기적으로 패킷 송신, TX_DS/MAX_RT 로그 | SPI 버스만(모듈 없어도 실행은 됨, ACK는 당연히 안 옴) |

## 현재 상태 (2026-09-17)

- 라즈베리파이: `pi14.local`, Debian(Trixie 계열), g++ 14.2, kernel 6.18 (GPIO v2 uAPI 사용 가능)
- SPI 커널 인터페이스 활성화 완료(`sudo raspi-config` → Interface Options → SPI), `/dev/spidev0.0` 확인됨
- nRF24L01 모듈 미배선. CE/CSN/SCK/MOSI/MISO 핀 배정 전부 TBD (CE는 임시로 BCM 22)
- `main.cpp` 실행 결과: 예외 없이 완료, `STATUS=0x00 CONFIG(before)=0x00 CONFIG(after)=0x00` (모듈 미연결 상태의 예상값 — SPI0 MISO(GPIO9)에 기본 pull-down이 있어 STM32의 `0xFF`와 달리 `0x00`으로 읽힘. 둘 다 "버스는 정상, 슬레이브 없음"과 동일한 의미)
- `packet_selftest`: PASS (경계값 포함 Encode/Decode 왕복 검증)
- `tx_loop` 실행 결과: 예외 없이 시작/종료, sequence 증가·steering 왕복(-100~100) 정상, 매 전송마다 `MAX_RT/timeout` (모듈 미연결이라 ACK가 올 수 없는 예상된 결과)
- 아직 미구현: 실제 gear/steering/speed 센서 읽기(지금은 더미 값), IRQ 기반 처리(지금은 polling), 실제 RF 송수신 검증

## 다음 할 일

1. nRF24L01 모듈 전원/SPI 배선 (STM32 쪽과 동일하게 CE/CSN 핀 확정 후 양쪽 문서에 기록)
2. `./nrf24_bringup`으로 STATUS/CONFIG 레지스터 read/write 확인 (STM32 쪽 bench 시험과 동일한 절차, [TEST_REPORT.md](../../../docs/ecus/Motor_Steering_Control/TEST_REPORT.md) 참고)
3. `./tx_loop` 돌리면서 STM32 쪽에서 실제로 패킷을 수신하는지 맞춰보기 (양쪽 다 모듈 배선 끝난 뒤)
4. 패킷 포맷(`version`/`sequence`/`gear`/`steering_request`/`speed_request`/`input_valid`) 확정 — STM32 쪽과 공동 결정 사항이며 지금 `packet.hpp`의 값은 초안(NOT FROZEN)이다
5. 실제 gear/steering/speed 입력(스위치/포텐셔미터 등)을 `tx_loop.cpp`의 더미 입력 부분과 교체
