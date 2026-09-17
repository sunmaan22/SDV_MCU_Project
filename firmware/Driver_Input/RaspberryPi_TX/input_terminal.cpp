#include "input_terminal.hpp"

#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>

namespace {

constexpr int16_t kStep = 10;  // 한 스텝당 변화량
// OS 키 auto-repeat는 100ms 창 안에 여러 이벤트를 몰아서 보낼 수 있어서(예:
// 30->90처럼 급격히 뜀), 축(steer/speed)별로 이 간격보다 빨리 온 이벤트는
// 무시하고 최대 kStep/kStepInterval 속도로만 값이 바뀌게 디바운스한다.
// TX 주기(100ms)와 맞춰서 화면에는 "누르는 동안 매 줄 +10"처럼 선형으로 보인다.
constexpr std::chrono::milliseconds kStepInterval(100);
// "방향키가 전혀 안 들어온" 상태로 이만큼 지나야 0으로 복귀한다. 값 증가
// debounce(kStepInterval)와 일부러 분리했다 - 안 그러면 읽기 타이밍이 살짝만
// 어긋나도(예: SSH 네트워크 지연) 키를 누르고 있는 도중에도 순간적으로 0으로
// 튀었다가 다시 올라가는 것처럼 보인다.
constexpr std::chrono::milliseconds kCenterGrace(200);

int16_t ClampToPercent(long value) {
  return static_cast<int16_t>(std::max<long>(-100, std::min<long>(100, value)));
}

int16_t ClampToMagnitude(long value) {
  return static_cast<int16_t>(std::max<long>(0, std::min<long>(100, value)));
}

void PrintHelp() {
  std::printf(
      "방향키: ↑ 가속 / ↓ 감속 / → 우회전 / ← 좌회전 (누르는 동안 매 100ms마다 "
      "+-10, 떼면 200ms 안에 0/0으로 복귀)\n"
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

// raw 모드 + VTIME 100ms: read()가 최대 100ms 대기 후 아무 키도 없으면 0 반환.
void ConfigureRawMode(struct termios* orig) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, orig);
  raw = *orig;
  raw.c_lflag &= ~(ICANON | ECHO);  // ISIG는 유지 -> Ctrl+C(SIGINT) 계속 동작
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;  // 0.1s 단위 -> 100ms
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void InputLoop(TerminalInputState* state, std::atomic<bool>* quit_flag) {
  struct termios orig_termios;
  ConfigureRawMode(&orig_termios);
  PrintHelp();

  using Clock = std::chrono::steady_clock;
  Clock::time_point last_speed_step{};    // 기본값(epoch)이라 첫 입력은 항상 즉시 반영됨
  Clock::time_point last_steer_step{};
  Clock::time_point last_direction_key{};  // 값 변경 여부와 무관하게, 방향키가 인식된 마지막 시각

  while (!quit_flag->load()) {
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n <= 0) {
      // read() 자체는 100ms마다 깨어나지만, 실제 복귀는 kCenterGrace(200ms) 동안
      // 방향키가 전혀 없을 때만 한다 - 단발성 read 타임아웃 하나로 바로 0을
      // 만들면 네트워크 지연 등으로 키를 누르고 있는 도중에도 깜빡일 수 있다.
      if (Clock::now() - last_direction_key >= kCenterGrace) {
        state->steering.store(0);
        state->speed.store(0);
      }
      continue;
    }

    if (c == 0x1B) {  // ESC: 방향키 시퀀스(ESC [ A/B/C/D) 가능성
      unsigned char seq[2];
      ssize_t n1 = read(STDIN_FILENO, &seq[0], 1);
      ssize_t n2 = (n1 == 1) ? read(STDIN_FILENO, &seq[1], 1) : 0;
      if (n1 == 1 && n2 == 1 && seq[0] == '[' &&
          (seq[1] == 'A' || seq[1] == 'B' || seq[1] == 'C' || seq[1] == 'D')) {
        Clock::time_point now = Clock::now();
        last_direction_key = now;
        switch (seq[1]) {
          case 'A':  // Up: 가속
            if (now - last_speed_step >= kStepInterval) {
              state->speed.store(ClampToMagnitude(state->speed.load() + kStep));
              last_speed_step = now;
            }
            break;
          case 'B':  // Down: 감속
            if (now - last_speed_step >= kStepInterval) {
              state->speed.store(ClampToMagnitude(state->speed.load() - kStep));
              last_speed_step = now;
            }
            break;
          case 'C':  // Right: 우회전
            if (now - last_steer_step >= kStepInterval) {
              state->steering.store(ClampToPercent(state->steering.load() + kStep));
              last_steer_step = now;
            }
            break;
          case 'D':  // Left: 좌회전
            if (now - last_steer_step >= kStepInterval) {
              state->steering.store(ClampToPercent(state->steering.load() - kStep));
              last_steer_step = now;
            }
            break;
          default:
            break;
        }
      }
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
