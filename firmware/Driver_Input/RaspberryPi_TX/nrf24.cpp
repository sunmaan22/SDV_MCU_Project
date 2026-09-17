#include "nrf24.hpp"

#include <fcntl.h>
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <system_error>

namespace {

void ThrowErrno(const char* what) {
  throw std::system_error(errno, std::generic_category(), what);
}

}  // namespace

NRF24::NRF24(const char* spi_device, const char* gpiochip_device,
             unsigned ce_offset, uint32_t speed_hz)
    : spi_fd_(-1), ce_line_fd_(-1) {
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

uint8_t NRF24::ReadStatus() {
  uint8_t tx[1] = {NRF24_CMD_NOP};
  uint8_t rx[1] = {0};
  SpiTransfer(tx, rx, sizeof(tx));
  return rx[0];
}
