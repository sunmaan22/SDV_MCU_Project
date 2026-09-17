CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

.PHONY: all clean test
all: nrf24_bringup tx_loop packet_selftest

nrf24_bringup: main.cpp nrf24.cpp nrf24.hpp
	$(CXX) $(CXXFLAGS) main.cpp nrf24.cpp -o nrf24_bringup

tx_loop: tx_loop.cpp nrf24.cpp nrf24.hpp packet.hpp
	$(CXX) $(CXXFLAGS) tx_loop.cpp nrf24.cpp -o tx_loop

packet_selftest: packet_selftest.cpp packet.hpp
	$(CXX) $(CXXFLAGS) packet_selftest.cpp -o packet_selftest

# 하드웨어 필요 없는 부분만 검증 (SPI/GPIO 없이 CI/로컬에서도 돌릴 수 있음).
test: packet_selftest
	./packet_selftest

clean:
	rm -f nrf24_bringup tx_loop packet_selftest
