// 터미널 raw 키 입력으로 steer/speed/gear를 입력받는 bench 전용 입력기.
//
// 방향키(위/아래/좌/우)는 스프링 복귀형 조이스틱처럼 동작한다: 눌려있는
// 동안(OS auto-repeat로 반복 이벤트가 계속 들어오는 동안)만 값이 유지/변화하고,
// 100ms 안에 아무 키 이벤트도 없으면 steering/speed 둘 다 0으로 자동 복귀한다.
// 기어는 p/r/n/d(대소문자 무관) 한 글자를 누르는 즉시 바뀐다.
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
  std::atomic<int16_t> steering{0};  // -100(좌)~100(우) (DEC-CTRL-014와 동일 스케일)
  std::atomic<int16_t> speed{0};     // 0~100 크기만(음수 없음) - 방향은 gear(P/R/N/D)가 담당
  std::atomic<uint8_t> gear{2};      // P=0/R=1/N=2/D=3 (Final_Drive_Command enum과 동일, bench 매핑)
  std::atomic<bool> valid{true};     // RF invalid 시뮬레이션용, 'v' 키로 토글
};

// 입력 스레드를 detach 상태로 시작한다(별도 join 불필요). 터미널을 raw 모드로
// 바꿨다가 종료 시(quit_flag=true 또는 'q' 입력) 원래 모드로 복원한다.
void StartTerminalInputThread(TerminalInputState* state, std::atomic<bool>* quit_flag);
