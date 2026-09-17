// 터미널(stdin) 명령으로 steer/speed/gear/valid를 입력받는 bench 전용 입력기.
//
// *** 긴급정지(E-Stop)는 여기 포함하지 않는다. ***
// FINAL_IMPLEMENTATION_SPEC.md의 DEC-HW-020에 따라 E-Stop은 C(STM32)가 로컬
// GPIO/EXTI로 직접 읽고 CAN/RF 경유 없이 즉시 차단해야 하는 안전 요구사항이다.
// RF 링크는 간섭/끊김이 있을 수 있어 비상정지를 여기 태우면 안 된다. 실제
// E-Stop은 C 보드에 물리 버튼이 달리면(DEC-HW-027, 아직 OPEN) 그쪽에서 처리한다.
#pragma once

#include <atomic>
#include <cstdint>
#include <thread>

struct TerminalInputState {
  std::atomic<int16_t> steering{0};  // -100~100 (DEC-CTRL-014와 동일 스케일)
  std::atomic<int16_t> speed{0};     // 0~100 크기만(음수 없음) - 방향은 gear(P/R/N/D)가 담당
  std::atomic<uint8_t> gear{2};      // P=0/R=1/N=2/D=3 (Final_Drive_Command enum과 동일, bench 매핑)
  std::atomic<bool> valid{true};     // RF invalid 시뮬레이션용
};

// 입력 스레드를 detach 상태로 시작한다(별도 join 불필요).
// quit_flag는 "quit" 명령 입력 시 true로 설정된다 — 메인 루프가 이 값을
// 주기적으로 확인해 종료하면 된다.
void StartTerminalInputThread(TerminalInputState* state, std::atomic<bool>* quit_flag);
