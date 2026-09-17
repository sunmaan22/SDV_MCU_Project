// nRF24L01 레지스터 레벨 드라이버 (bring-up + TX 전용, C++/Linux 커널 uAPI 직접 사용).
//
// STM32 Driver_Input 쪽 코드(firmware/Driver_Input/Core/Src/main.c)와 동일한
// 커맨드/레지스터 상수를 쓴다. 외부 라이브러리(WiringPi, libgpiod 등) 없이
// spidev ioctl + GPIO character device(v2 uAPI)를 직접 호출한다.
//
// 핀 배정은 아직 TBD (배선 전). CE는 GPIO로 별도 제어하고, CSN은 spidev의
// 하드웨어 CS0(GPIO8)를 그대로 쓴다.
//
// 채널/주소/air data rate 등은 전부 bench 값이며 NOT FROZEN이다 (DRIVER_INPUT_START.md 참고).
// 송수신측이 실제로 맞춰야 확정된다.
#pragma once

#include <cstddef>
#include <cstdint>

// --- nRF24L01 커맨드 (STM32 쪽과 동일 + TX용 추가) ---
constexpr uint8_t NRF24_CMD_R_REGISTER    = 0x00;
constexpr uint8_t NRF24_CMD_W_REGISTER    = 0x20;
constexpr uint8_t NRF24_CMD_W_TX_PAYLOAD  = 0xA0;
constexpr uint8_t NRF24_CMD_FLUSH_TX      = 0xE1;
constexpr uint8_t NRF24_CMD_NOP           = 0xFF;

// --- 레지스터 주소 ---
constexpr uint8_t NRF24_REG_CONFIG      = 0x00;
constexpr uint8_t NRF24_REG_EN_AA       = 0x01;
constexpr uint8_t NRF24_REG_EN_RXADDR   = 0x02;
constexpr uint8_t NRF24_REG_SETUP_AW    = 0x03;
constexpr uint8_t NRF24_REG_SETUP_RETR  = 0x04;
constexpr uint8_t NRF24_REG_RF_CH       = 0x05;
constexpr uint8_t NRF24_REG_RF_SETUP    = 0x06;
constexpr uint8_t NRF24_REG_STATUS      = 0x07;
constexpr uint8_t NRF24_REG_RX_ADDR_P0  = 0x0A;
constexpr uint8_t NRF24_REG_TX_ADDR     = 0x10;
constexpr uint8_t NRF24_REG_RX_PW_P0    = 0x11;

// --- STATUS 레지스터 비트 ---
constexpr uint8_t NRF24_STATUS_TX_DS   = 0x20;  // 송신 성공(ACK 수신 또는 AutoAck 비활성)
constexpr uint8_t NRF24_STATUS_MAX_RT  = 0x10;  // 재전송 한도 초과(ACK 못 받음)

// CE 핀: TBD, 배선 확정 전까지 임시값. BCM 번호 기준.
constexpr unsigned NRF24_CE_GPIO_BCM = 22;

// --- Bench 전용 TX 설정값 (NOT FROZEN, 송수신측 합의 전) ---
constexpr uint8_t NRF24_BENCH_CHANNEL = 76;
constexpr uint8_t NRF24_BENCH_ADDRESS[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
constexpr size_t NRF24_ADDR_WIDTH = 5;

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
  // 여러 바이트 레지스터(주소류) 쓰기.
  void WriteRegisterMulti(uint8_t reg, const uint8_t* data, size_t len);
  // NOP 커맨드로 STATUS만 읽는다.
  uint8_t ReadStatus();
  // STATUS 레지스터 값을 그대로 클리어(TX_DS/MAX_RT/RX_DR 비트에 1을 써서 clear).
  void ClearStatusFlags(uint8_t mask);

  // PTX(송신 전용) 모드로 초기화: 채널/주소/payload 크기/CRC/ACK 설정.
  // payload_size는 고정 길이(초기에는 하나로 시작, 최대 32).
  void InitAsTransmitter(uint8_t channel = NRF24_BENCH_CHANNEL,
                          const uint8_t* address = NRF24_BENCH_ADDRESS,
                          uint8_t payload_size = 8);

  // payload_size 바이트를 TX FIFO에 넣고 CE 펄스(>10us)로 전송을 트리거한 뒤
  // TX_DS 또는 MAX_RT가 뜰 때까지(또는 timeout_ms) 기다린다.
  // 반환값: true면 TX_DS(성공), false면 MAX_RT 또는 timeout.
  bool SendPayload(const uint8_t* payload, size_t len, uint32_t timeout_ms = 50);

 private:
  int spi_fd_;
  int ce_line_fd_;
  uint8_t payload_size_;

  void SpiTransfer(uint8_t* tx, uint8_t* rx, size_t len);
  void SetCe(bool high);
};
