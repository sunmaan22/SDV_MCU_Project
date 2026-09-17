// nRF24L01 레지스터 레벨 드라이버 (bring-up 전용, C++/Linux 커널 uAPI 직접 사용).
//
// STM32 Driver_Input 쪽 코드(firmware/Driver_Input/Core/Src/main.c)와 동일한
// 커맨드/레지스터 상수를 쓴다. 외부 라이브러리(WiringPi, libgpiod 등) 없이
// spidev ioctl + GPIO character device(v2 uAPI)를 직접 호출한다.
//
// 핀 배정은 아직 TBD (배선 전). CE는 GPIO로 별도 제어하고, CSN은 spidev의
// 하드웨어 CS0(GPIO8)를 그대로 쓴다.
#pragma once

#include <cstddef>
#include <cstdint>

// nRF24L01 커맨드 (STM32/Python 쪽과 동일)
constexpr uint8_t NRF24_CMD_R_REGISTER = 0x00;
constexpr uint8_t NRF24_CMD_W_REGISTER = 0x20;
constexpr uint8_t NRF24_CMD_NOP        = 0xFF;

// 자주 쓰는 레지스터 주소
constexpr uint8_t NRF24_REG_CONFIG = 0x00;
constexpr uint8_t NRF24_REG_STATUS = 0x07;

// CE 핀: TBD, 배선 확정 전까지 임시값. BCM 번호 기준.
constexpr unsigned NRF24_CE_GPIO_BCM = 22;

class NRF24 {
 public:
  explicit NRF24(const char* spi_device = "/dev/spidev0.0",
                  const char* gpiochip_device = "/dev/gpiochip0",
                  unsigned ce_offset = NRF24_CE_GPIO_BCM,
                  uint32_t speed_hz = 1000000);
  ~NRF24();

  NRF24(const NRF24&) = delete;
  NRF24& operator=(const NRF24&) = delete;

  void CeLow();
  void CeHigh();

  // R_REGISTER: 두 번째로 보낸 바이트에 대한 응답이 레지스터 값.
  uint8_t ReadRegister(uint8_t reg);
  // W_REGISTER. 반환값은 트랜잭션 첫 바이트로 온 STATUS.
  uint8_t WriteRegister(uint8_t reg, uint8_t value);
  // NOP 커맨드로 STATUS만 읽는다.
  uint8_t ReadStatus();

 private:
  int spi_fd_;
  int ce_line_fd_;

  void SpiTransfer(uint8_t* tx, uint8_t* rx, size_t len);
  void SetCe(bool high);
};
