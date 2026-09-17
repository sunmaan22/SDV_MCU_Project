CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

TARGET := nrf24_bringup
SRCS := main.cpp nrf24.cpp

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRCS) nrf24.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
