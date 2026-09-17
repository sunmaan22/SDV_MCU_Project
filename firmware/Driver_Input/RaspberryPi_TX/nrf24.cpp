#include "nrf24.hpp"

#include <fcntl.h>
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace {

void ThrowErrno(const char* what) {
  throw std::system_error(errno, std::generic_category(), what);
}

}  // namespace

NRF24::NRF24(const char* spi_device, const char* gpiochip_device,
             unsigned ce_offset, uint32_t speed_hz)
    : spi_fd_(-1), ce_line_fd_(-1), payload_size_(0) {
  // --- SPI: mode 0 (CPOL=0, CPHA=0), 8bit, speed_hz ---
  spi_fd_ = open(spi_device, O_RDWR);
  if (spi_fd_ < 0) {
    ThrowErrno("open spidev");
  }

  uint8_t mode = SPI_MODE_0;
  uint8_t bits = 8;
  if (ioctl(spi_fd_, SPI_IOC_WR_MODE, &mode) < 0 ||
      ioctl(spi_fd_, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
      ioctl(spi_fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) {
    close(spi_fd_);
    ThrowErrno("configure spidev");
  }

  // --- CE: GPIO character device v2 uAPI, output line ---
  int chip_fd = open(gpiochip_device, O_RDONLY);
  if (chip_fd < 0) {
    close(spi_fd_);
    ThrowErrno("open gpiochip");
  }

  struct gpio_v2_line_request req{};
  req.num_lines = 1;
  req.offsets[0] = ce_offset;
  req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
  std::strncpy(req.consumer, "nrf24_ce", sizeof(req.consumer) - 1);

  int ret = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req);
  close(chip_fd);
  if (ret < 0) {
    close(spi_fd_);
    ThrowErrno("request CE gpio line");
  }
  ce_line_fd_ = req.fd;

  CeLow();  // idle 상태: 수신/송신 대기
}

NRF24::~NRF24() {
  if (ce_line_fd_ >= 0) {
    SetCe(false);
    close(ce_line_fd_);
  }
  if (spi_fd_ >= 0) {
    close(spi_fd_);
  }
}

void NRF24::SetCe(bool high) {
  struct gpio_v2_line_values values{};
  values.mask = 1ULL;
  values.bits = high ? 1ULL : 0ULL;
  if (ioctl(ce_line_fd_, GPIO_V2_LINE_SET_VALUES_IOCTL, &values) < 0) {
    ThrowErrno("set CE value");
  }
}

void NRF24::CeLow() { SetCe(false); }
void NRF24::CeHigh() { SetCe(true); }

void NRF24::SpiTransfer(uint8_t* tx, uint8_t* rx, size_t len) {
  struct spi_ioc_transfer tr{};
  tr.tx_buf = reinterpret_cast<uint64_t>(tx);
  tr.rx_buf = reinterpret_cast<uint64_t>(rx);
  tr.len = static_cast<uint32_t>(len);
  tr.bits_per_word = 8;

  if (ioctl(spi_fd_, SPI_IOC_MESSAGE(1), &tr) < 0) {
    ThrowErrno("SPI transfer");
  }
}

uint8_t NRF24::ReadRegister(uint8_t reg) {
  uint8_t tx[2] = {static_cast<uint8_t>(NRF24_CMD_R_REGISTER | (reg & 0x1F)),
                    NRF24_CMD_NOP};
  uint8_t rx[2] = {0, 0};
  SpiTransfer(tx, rx, sizeof(tx));
  return rx[1];
}

uint8_t NRF24::WriteRegister(uint8_t reg, uint8_t value) {
  uint8_t tx[2] = {static_cast<uint8_t>(NRF24_CMD_W_REGISTER | (reg & 0x1F)),
                    value};
  uint8_t rx[2] = {0, 0};
  SpiTransfer(tx, rx, sizeof(tx));
  return rx[0];
}

void NRF24::WriteRegisterMulti(uint8_t reg, const uint8_t* data, size_t len) {
  std::vector<uint8_t> tx(len + 1);
  std::vector<uint8_t> rx(len + 1);
  tx[0] = static_cast<uint8_t>(NRF24_CMD_W_REGISTER | (reg & 0x1F));
  for (size_t i = 0; i < len; ++i) {
    tx[i + 1] = data[i];
  }
  SpiTransfer(tx.data(), rx.data(), tx.size());
}

uint8_t NRF24::ReadStatus() {
  uint8_t tx[1] = {NRF24_CMD_NOP};
  uint8_t rx[1] = {0};
  SpiTransfer(tx, rx, sizeof(tx));
  return rx[0];
}

void NRF24::ClearStatusFlags(uint8_t mask) {
  WriteRegister(NRF24_REG_STATUS, mask);
}

void NRF24::InitAsTransmitter(uint8_t channel, const uint8_t* address,
                               uint8_t payload_size) {
  payload_size_ = payload_size;

  CeLow();

  WriteRegister(NRF24_REG_CONFIG, 0x00);  // PWR_UP=0로 시작해 안전하게 재설정
  WriteRegister(NRF24_REG_EN_AA, 0x01);        // pipe0 Auto-Ack만 사용
  WriteRegister(NRF24_REG_EN_RXADDR, 0x01);    // pipe0만 활성 (ACK 수신용)
  WriteRegister(NRF24_REG_SETUP_AW, 0x03);     // 5바이트 주소
  WriteRegister(NRF24_REG_SETUP_RETR, 0x1A);   // ARD=500us, ARC=10회 (bench 값, NOT FROZEN)
  WriteRegister(NRF24_REG_RF_CH, channel);
  WriteRegister(NRF24_REG_RF_SETUP, 0x06);     // 1Mbps, 0dBm (bench 값)

  // TX_ADDR과 RX_ADDR_P0을 동일하게 맞춰야 Auto-Ack 수신이 된다.
  WriteRegisterMulti(NRF24_REG_TX_ADDR, address, NRF24_ADDR_WIDTH);
  WriteRegisterMulti(NRF24_REG_RX_ADDR_P0, address, NRF24_ADDR_WIDTH);
  WriteRegister(NRF24_REG_RX_PW_P0, payload_size);

  ClearStatusFlags(NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT | 0x40 /*RX_DR*/);

  WriteRegister(NRF24_REG_CONFIG, 0x0E);  // PWR_UP=1, PRIM_RX=0(TX), CRC 16bit enable
}

bool NRF24::SendPayload(const uint8_t* payload, size_t len,
                         uint32_t timeout_ms) {
  // FLUSH_TX: 이전 전송 잔여물 제거
  uint8_t flush_tx = NRF24_CMD_FLUSH_TX;
  uint8_t flush_rx = 0;
  SpiTransfer(&flush_tx, &flush_rx, 1);

  std::vector<uint8_t> tx(len + 1);
  std::vector<uint8_t> rx(len + 1);
  tx[0] = NRF24_CMD_W_TX_PAYLOAD;
  for (size_t i = 0; i < len; ++i) {
    tx[i + 1] = payload[i];
  }
  SpiTransfer(tx.data(), rx.data(), tx.size());

  // CE를 >10us 하이로 유지해야 전송이 시작된다.
  CeHigh();
  usleep(15);
  CeLow();

  const auto start = std::chrono::steady_clock::now();
  while (true) {
    uint8_t status = ReadStatus();
    if (status & NRF24_STATUS_TX_DS) {
      ClearStatusFlags(NRF24_STATUS_TX_DS);
      return true;
    }
    if (status & NRF24_STATUS_MAX_RT) {
      ClearStatusFlags(NRF24_STATUS_MAX_RT);
      return false;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    if (static_cast<uint32_t>(elapsed.count()) >= timeout_ms) {
      return false;  // timeout: 응답 자체가 없음(모듈 미연결 등)
    }
  }
}
