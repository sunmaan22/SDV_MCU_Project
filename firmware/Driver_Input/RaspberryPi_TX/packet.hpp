// Driver_Input RF 패킷 초안 (DRIVER_INPUT_START.md 참고).
//
// *** 이 레이아웃은 초안이며 NOT FROZEN이다. ***
// byte 수·단위·signedness·endianness는 송수신측(STM32 C / 이 라즈베리파이)이
// 함께 정해야 확정된다. CAN 메시지와도 별도 계약이다.
//
// C 구조체를 그대로 메모리 덤프해서 보내지 않는다 - 항상 이 파일의
// Encode/Decode 함수로 바이트 배치를 명시한다 (패딩/엔디안이 컴파일러·아키텍처마다
// 달라질 수 있기 때문).
#pragma once

#include <array>
#include <cstdint>
#include <cstring>

// 고정 8바이트 payload (bench 값, NOT FROZEN):
//   [0]    version
//   [1]    sequence (0..255 wraparound)
//   [2]    gear (raw 스위치 코드, 위치 수/매핑 TBD - 임의로 P/R/N/D 4상태로
//          해석하지 않는다)
//   [3..4] steering_request, int16 little-endian, 중립 0 기준 (raw 범위 TBD)
//   [5..6] speed_request, int16 little-endian, 중립 0 기준 (raw 범위 TBD)
//   [7]    input_valid (0/1)
constexpr size_t kDriverInputPacketSize = 8;

struct DriverInputPacket {
  uint8_t version = 1;
  uint8_t sequence = 0;
  uint8_t gear = 0;
  int16_t steering_request = 0;
  int16_t speed_request = 0;
  bool input_valid = false;
};

inline void EncodeDriverInputPacket(const DriverInputPacket& p,
                                     std::array<uint8_t, kDriverInputPacketSize>* out) {
  auto& b = *out;
  b[0] = p.version;
  b[1] = p.sequence;
  b[2] = p.gear;
  // int16 little-endian: 하위 바이트 먼저. 캐스팅은 2의 보수 표현을 그대로 씀.
  uint16_t steer_u = static_cast<uint16_t>(p.steering_request);
  b[3] = static_cast<uint8_t>(steer_u & 0xFF);
  b[4] = static_cast<uint8_t>((steer_u >> 8) & 0xFF);
  uint16_t speed_u = static_cast<uint16_t>(p.speed_request);
  b[5] = static_cast<uint8_t>(speed_u & 0xFF);
  b[6] = static_cast<uint8_t>((speed_u >> 8) & 0xFF);
  b[7] = p.input_valid ? 1 : 0;
}

inline DriverInputPacket DecodeDriverInputPacket(
    const std::array<uint8_t, kDriverInputPacketSize>& b) {
  DriverInputPacket p;
  p.version = b[0];
  p.sequence = b[1];
  p.gear = b[2];
  uint16_t steer_u = static_cast<uint16_t>(b[3]) | (static_cast<uint16_t>(b[4]) << 8);
  p.steering_request = static_cast<int16_t>(steer_u);
  uint16_t speed_u = static_cast<uint16_t>(b[5]) | (static_cast<uint16_t>(b[6]) << 8);
  p.speed_request = static_cast<int16_t>(speed_u);
  p.input_valid = (b[7] != 0);
  return p;
}
