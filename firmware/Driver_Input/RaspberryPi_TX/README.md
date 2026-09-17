# RaspberryPi_TX

RF 송신측(조종기) 개발용 폴더. C ECU의 `Driver_Input`(STM32, 수신측)과 짝을 이루는 송신측 코드이며, 별도 ECU가 아니라 벤치용 컨트롤러다. STM32 쪽 가이드는 [DRIVER_INPUT_START.md](../../../docs/ecus/Motor_Steering_Control/DRIVER_INPUT_START.md) 참고.

## 현재 상태 (2026-09-17)

- 라즈베리파이: `pi14.local`, Python 3.13.5, `spidev`/`gpiozero` 설치됨
- **SPI 커널 인터페이스 비활성 상태.** 실배선 테스트 전 `sudo raspi-config` → Interface Options → SPI → Enable 하고 재부팅 필요
- nRF24L01 모듈 미배선. CE/CSN/SCK/MOSI/MISO 핀 배정 전부 TBD
- `nrf24.py`: STM32 `Core/Src/main.c`와 동일한 R_REGISTER/W_REGISTER 커맨드 레벨 드라이버. 라이브러리(RF24 등) 대신 `spidev` 직접 사용 — 송수신 양쪽에서 같은 원시 레지스터 접근 방식을 맞추기 위함
- 아직 미구현: 실제 gear/steering/speed 입력 읽기, 패킷 인코딩, 실제 송신 루프

## 다음 할 일

1. `sudo raspi-config`로 SPI 활성화, 재부팅
2. nRF24L01 모듈 전원/SPI 배선 (STM32 쪽과 동일하게 CE/CSN 핀 확정 후 양쪽 문서에 기록)
3. `python3 nrf24.py`로 STATUS/CONFIG 레지스터 read/write 확인 (STM32 쪽 bench 시험과 동일한 절차, [TEST_REPORT.md](../../../docs/ecus/Motor_Steering_Control/TEST_REPORT.md) 참고)
4. 패킷 포맷(`version`/`sequence`/`gear`/`steering_request`/`speed_request`/`input_valid`) 확정 — STM32 쪽과 공동 결정 사항이며 byte 배치/단위/endianness를 명시적으로 정한다. C 구조체를 그대로 덤프해서 보내지 않는다
