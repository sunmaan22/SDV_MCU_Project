// Driver_Input RF 송신 루프 (bench 전용, 더미 입력값).
//
// 실제 gear/steering/speed 센서가 아직 배선 안 됐으므로 여기서는 고정
// 패턴(왕복하는 steering/speed 값)으로 패킷을 채워서 보낸다. 나중에 실제
// 입력 읽기로 교체할 때 이 루프의 "더미 입력 만들기" 부분만 바꾸면 된다.
//
// 주기(100ms)는 bench 값이며 NOT FROZEN이다 (DRIVER_INPUT_START.md 참고).
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <exception>
#include <thread>

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

    std::printf("TX loop 시작 (Ctrl+C로 종료). 채널=%u 페이로드=%zuB\n",
                NRF24_BENCH_CHANNEL, kDriverInputPacketSize);

    uint8_t sequence = 0;
    int16_t steer_sweep = -100;
    int8_t steer_dir = 1;

    while (!g_stop) {
      DriverInputPacket pkt;
      pkt.version = 1;
      pkt.sequence = sequence++;
      pkt.gear = 0;  // raw 코드 TBD, 지금은 항상 0(placeholder)
      pkt.steering_request = steer_sweep;
      pkt.speed_request = 0;  // 더미: 모터/서보 비활성 상태 유지
      pkt.input_valid = true;

      // 더미 입력: steering 값을 -100~100 사이로 왕복시켜서 패킷이
      // 실제로 매번 바뀌는지 눈으로 확인할 수 있게 한다.
      steer_sweep = static_cast<int16_t>(steer_sweep + steer_dir * 10);
      if (steer_sweep >= 100 || steer_sweep <= -100) {
        steer_dir = static_cast<int8_t>(-steer_dir);
      }

      std::array<uint8_t, kDriverInputPacketSize> buf{};
      EncodeDriverInputPacket(pkt, &buf);

      bool ok = nrf.SendPayload(buf.data(), buf.size());

      std::printf(
          "[TX] seq=%u steer=%d speed=%d valid=%d -> %s\n",
          pkt.sequence, pkt.steering_request, pkt.speed_request,
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
