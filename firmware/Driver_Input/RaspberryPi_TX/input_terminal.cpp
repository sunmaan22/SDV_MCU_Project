#include "input_terminal.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int16_t ClampToPercent(long value) {
  return static_cast<int16_t>(std::max<long>(-100, std::min<long>(100, value)));
}

int16_t ClampToMagnitude(long value) {
  return static_cast<int16_t>(std::max<long>(0, std::min<long>(100, value)));
}

bool ParseGear(const std::string& token, uint8_t* out) {
  if (token == "P" || token == "p") { *out = 0; return true; }
  if (token == "R" || token == "r") { *out = 1; return true; }
  if (token == "N" || token == "n") { *out = 2; return true; }
  if (token == "D" || token == "d") { *out = 3; return true; }
  return false;
}

void PrintHelp() {
  std::printf(
      "명령: steer <-100~100> | speed <0~100, 방향은 gear가 결정> | "
      "gear <P|R|N|D> | valid <0|1> | help | quit\n"
      "(긴급정지는 여기 없음 - C 보드의 물리 E-Stop 버튼이 처리함)\n");
}

void InputLoop(TerminalInputState* state, std::atomic<bool>* quit_flag) {
  PrintHelp();
  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "steer") {
      long v;
      if (iss >> v) {
        state->steering.store(ClampToPercent(v));
      } else {
        std::printf("사용법: steer <-100~100>\n");
      }
    } else if (cmd == "speed") {
      long v;
      if (iss >> v) {
        state->speed.store(ClampToMagnitude(v));
      } else {
        std::printf("사용법: speed <0~100> (방향은 gear로 결정, 음수 없음)\n");
      }
    } else if (cmd == "gear") {
      std::string g;
      uint8_t code;
      if (iss >> g && ParseGear(g, &code)) {
        state->gear.store(code);
      } else {
        std::printf("사용법: gear <P|R|N|D>\n");
      }
    } else if (cmd == "valid") {
      int v;
      if (iss >> v) {
        state->valid.store(v != 0);
      } else {
        std::printf("사용법: valid <0|1>\n");
      }
    } else if (cmd == "help") {
      PrintHelp();
    } else if (cmd == "quit" || cmd == "exit") {
      quit_flag->store(true);
      break;
    } else if (!cmd.empty()) {
      std::printf("알 수 없는 명령: %s (help 참고)\n", cmd.c_str());
    }
  }
}

}  // namespace

void StartTerminalInputThread(TerminalInputState* state, std::atomic<bool>* quit_flag) {
  std::thread(InputLoop, state, quit_flag).detach();
}
