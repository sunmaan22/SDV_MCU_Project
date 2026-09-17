// Driver_Input RF 송신 루프 - 터미널 명령으로 steer/speed/gear를 입력받아 송신.
//
// 긴급정지(E-Stop)는 여기 없다 — DEC-HW-020에 따라 C(STM32)가 로컬 GPIO/EXTI로
// 처리해야 하는 안전 요구사항이며, RF 링크에 태우지 않는다(input_terminal.hpp 참고).
//
// 주기(100ms)는 bench 값이며 NOT FROZEN이다 (DRIVER_INPUT_START.md 참고).
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <exception>
#include <thread>

#include "input_terminal.hpp"
#include "nrf24.hpp"
#include "packet.hpp"

namespace {
std::atomic<bool> g_stop{false};
void HandleSigint(int) { g_stop = true; }
}  // namespace

int main() {
  std::signal(SIGINT, HandleSigint);

  try {
    NRF24 nrf;
    nrf.InitAsTransmitter();

    TerminalInputState input;
    StartTerminalInputThread(&input, &g_stop);

    std::printf("TX loop 시작 (Ctrl+C 또는 quit 명령으로 종료). 채널=%u 페이로드=%zuB\n",
                NRF24_BENCH_CHANNEL, kDriverInputPacketSize);

    uint8_t sequence = 0;

    while (!g_stop) {
      DriverInputPacket pkt;
      pkt.version = 1;
      pkt.sequence = sequence++;
      pkt.gear = input.gear.load();
      pkt.steering_request = input.steering.load();
      pkt.speed_request = input.speed.load();
      pkt.input_valid = input.valid.load();

      std::array<uint8_t, kDriverInputPacketSize> buf{};
      EncodeDriverInputPacket(pkt, &buf);

      bool ok = nrf.SendPayload(buf.data(), buf.size());

      std::printf(
          "[TX] seq=%u gear=%u steer=%d speed=%d valid=%d -> %s\n",
          pkt.sequence, pkt.gear, pkt.steering_request, pkt.speed_request,
          pkt.input_valid ? 1 : 0, ok ? "TX_DS" : "MAX_RT/timeout");

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::printf("TX loop 종료\n");
  } catch (const std::exception& e) {
    std::fprintf(stderr, "TX loop FAILED: %s\n", e.what());
    return 1;
  }

  return 0;
}
