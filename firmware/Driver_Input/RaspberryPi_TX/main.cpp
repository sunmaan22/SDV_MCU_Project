// STM32 쪽 T-RF-000 이후 bench 시험과 동일한 SPI read/write 확인.
//
// 모듈 미연결 상태에서는 STM32(0xFF, MISO no-pull)와 다르게 라즈베리파이는
// SPI0 MISO(GPIO9)에 기본 pull-down이 있어 0x00이 나오는 게 정상이다.
// 실제 모듈을 연결한 뒤 STATUS 기본값(약 0x0E)과 CONFIG 기본값(약 0x08)이
// 보이는지, write 후 값이 실제로 바뀌는지 확인한다.
#include <cstdio>

#include "nrf24.hpp"

#include <chrono>
#include <exception>
#include <thread>

int main() {
  try {
    NRF24 nrf;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 전원 안정화 대기

    uint8_t status_before = nrf.ReadRegister(NRF24_REG_STATUS);
    uint8_t config_before = nrf.ReadRegister(NRF24_REG_CONFIG);

    nrf.WriteRegister(NRF24_REG_CONFIG, 0x0A);
    uint8_t config_after = nrf.ReadRegister(NRF24_REG_CONFIG);

    std::printf(
        "NRF24 STATUS=0x%02X CONFIG(before)=0x%02X "
        "CONFIG(after write 0x0A)=0x%02X\n",
        status_before, config_before, config_after);
  } catch (const std::exception& e) {
    std::fprintf(stderr, "NRF24 bringup FAILED: %s\n", e.what());
    return 1;
  }

  return 0;
}
