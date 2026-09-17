#include "input_terminal.hpp"

#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace {

constexpr int16_t kStep = 10;  // 방향키 한 번 누를 때마다 변화량

int16_t ClampToPercent(long value) {
  return static_cast<int16_t>(std::max<long>(-100, std::min<long>(100, value)));
}

int16_t ClampToMagnitude(long value) {
  return static_cast<int16_t>(std::max<long>(0, std::min<long>(100, value)));
}

void PrintHelp() {
  std::printf(
      "방향키: ↑ 가속 / ↓ 감속 / → 우회전 / ← 좌회전 (누른 값 그대로 유지, "
      "다시 바꿀 때까지 안 풀림)\n"
      "space: steer/speed 중립(0/0) 복귀\n"
      "기어: p/r/n/d 한 글자 (대소문자 무관) 즉시 반영\n"
      "v: input_valid 토글, q 또는 Ctrl+C: 종료\n"
      "(긴급정지는 여기 없음 - C 보드의 물리 E-Stop 버튼이 처리함)\n");
}

bool SetGearFromChar(char c, TerminalInputState* state) {
  switch (std::tolower(static_cast<unsigned char>(c))) {
    case 'p': state->gear.store(0); return true;
    case 'r': state->gear.store(1); return true;
    case 'n': state->gear.store(2); return true;
    case 'd': state->gear.store(3); return true;
    default: return false;
  }
}

// raw 모드: 줄바꿈 기다리지 않고 키 하나씩 바로 읽는다. VTIME은 짧게 둬서
// quit_flag 확인 주기를 확보한다(값 유지/복귀 로직에는 더 이상 안 쓰임).
void ConfigureRawMode(struct termios* orig) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, orig);
  raw = *orig;
  raw.c_lflag &= ~(ICANON | ECHO);  // ISIG는 유지 -> Ctrl+C(SIGINT) 계속 동작
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 2;  // 0.1s 단위 -> 200ms마다 quit_flag 확인
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void InputLoop(TerminalInputState* state, std::atomic<bool>* quit_flag) {
  struct termios orig_termios;
  ConfigureRawMode(&orig_termios);
  PrintHelp();

  while (!quit_flag->load()) {
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n <= 0) {
      continue;  // 타임아웃: 아무 값도 안 바꾸고 quit_flag만 다시 확인
    }

    if (c == 0x1B) {  // ESC: 방향키 시퀀스(ESC [ A/B/C/D) 가능성
      unsigned char seq[2];
      ssize_t n1 = read(STDIN_FILENO, &seq[0], 1);
      ssize_t n2 = (n1 == 1) ? read(STDIN_FILENO, &seq[1], 1) : 0;
      if (n1 == 1 && n2 == 1 && seq[0] == '[') {
        switch (seq[1]) {
          case 'A':  // Up: 가속 (누른 값 유지)
            state->speed.store(ClampToMagnitude(state->speed.load() + kStep));
            break;
          case 'B':  // Down: 감속 (누른 값 유지)
            state->speed.store(ClampToMagnitude(state->speed.load() - kStep));
            break;
          case 'C':  // Right: 우회전 (누른 값 유지)
            state->steering.store(ClampToPercent(state->steering.load() + kStep));
            break;
          case 'D':  // Left: 좌회전 (누른 값 유지)
            state->steering.store(ClampToPercent(state->steering.load() - kStep));
            break;
          default:
            break;
        }
      }
      continue;
    }

    if (c == ' ') {  // space: 중립 복귀 (긴급정지 아님, DEC-HW-020 참고)
      state->steering.store(0);
      state->speed.store(0);
      continue;
    }

    if (SetGearFromChar(static_cast<char>(c), state)) {
      continue;
    }

    if (c == 'v' || c == 'V') {
      bool new_valid = !state->valid.load();
      state->valid.store(new_valid);
      std::printf("input_valid = %d\n", new_valid ? 1 : 0);
      continue;
    }

    if (c == 'q' || c == 'Q') {
      quit_flag->store(true);
      break;
    }
    // 그 외 키는 무시 (raw 모드라 별도 에러 출력 안 함)
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

}  // namespace

void StartTerminalInputThread(TerminalInputState* state, std::atomic<bool>* quit_flag) {
  std::thread(InputLoop, state, quit_flag).detach();
}
