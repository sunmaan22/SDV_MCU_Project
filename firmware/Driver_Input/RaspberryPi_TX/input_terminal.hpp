// 터미널 raw 키 입력으로 steer/speed/gear를 입력받는 최종 조종 인터페이스.
//
// SSH 원격 터미널 입력이 최종 인터페이스라서(파이에 물리 키보드를 꽂지 않음),
// "누르고 있는 동안만 유지되는 스프링 복귀"는 쓰지 않는다 - OS/터미널의
// auto-repeat는 보통 한 번에 키 하나만 반복 전송하기 때문에 조향+속도를
// "동시에 누르고 있기"가 원격 터미널에서는 신뢰할 수 없다. 대신 방향키를
// 누르면 그 값이 그대로 유지되는 래칭 방식을 쓴다 - 키를 순서대로만 눌러도
// (예: 위,위,오른쪽,오른쪽) 각 값이 독립적으로 남아있어서 실질적으로
// steer/speed가 동시에 걸려있는 상태가 된다.
//
// *** 긴급정지(E-Stop)는 여기 포함하지 않는다. ***
// FINAL_IMPLEMENTATION_SPEC.md의 DEC-HW-020에 따라 E-Stop은 C(STM32)가 로컬
// GPIO/EXTI로 직접 읽고 CAN/RF 경유 없이 즉시 차단해야 하는 안전 요구사항이다.
// RF 링크는 간섭/끊김이 있을 수 있어 비상정지를 여기 태우면 안 된다. 실제
// E-Stop은 C 보드에 물리 버튼이 달리면(DEC-HW-027, 아직 OPEN) 그쪽에서 처리한다.
// space 키는 긴급정지가 아니라 그냥 steer/speed를 0으로 되돌리는 중립 버튼이다.
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
