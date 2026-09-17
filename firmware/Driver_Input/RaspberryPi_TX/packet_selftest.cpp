// packet.hpp의 Encode/Decode가 서로 맞물리는지 확인하는 순수 소프트웨어
// 테스트. 하드웨어(SPI/GPIO) 전혀 필요 없음 - 배선 전에도 바로 돌릴 수 있다.
#include <cstdio>

#include "packet.hpp"

namespace {

int g_failures = 0;

void Check(bool cond, const char* what) {
  if (!cond) {
    std::printf("FAIL: %s\n", what);
    ++g_failures;
  }
}

void RoundTrip(const DriverInputPacket& in) {
  std::array<uint8_t, kDriverInputPacketSize> buf{};
  EncodeDriverInputPacket(in, &buf);
  DriverInputPacket out = DecodeDriverInputPacket(buf);

  Check(out.version == in.version, "version round-trip");
  Check(out.sequence == in.sequence, "sequence round-trip");
  Check(out.gear == in.gear, "gear round-trip");
  Check(out.steering_request == in.steering_request, "steering_request round-trip");
  Check(out.speed_request == in.speed_request, "speed_request round-trip");
  Check(out.input_valid == in.input_valid, "input_valid round-trip");
}

}  // namespace

int main() {
  // 경계값 위주로 확인: 0, 최대/최소 근처, 음수, invalid 플래그.
  RoundTrip(DriverInputPacket{1, 0, 0, 0, 0, false});
  RoundTrip(DriverInputPacket{1, 255, 3, 32767, -32768, true});
  RoundTrip(DriverInputPacket{1, 128, 1, -1, 1, true});

  if (g_failures == 0) {
    std::printf("packet_selftest: PASS\n");
    return 0;
  }
  std::printf("packet_selftest: %d FAILURE(S)\n", g_failures);
  return 1;
}
