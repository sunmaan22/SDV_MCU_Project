# Legacy Documentation Snapshot — Front-Only Scope 전환 전

이 폴더는 2026-09-15 "후방 카메라/주차 Vision 삭제, 전방 카메라 객체인식 전용 전환" 작업 직전의 `docs/` 및 루트 `README.md`를 보존한다.

- 목적: E(Vision) 주차 기능 삭제, A(Ultrasonic) 4방향 전용화, D(Lighting) Ambient 삭제, C(Drive) 실측 Encoder 대신 명령값 기반 추정, F(VCU) 중재 로직 단순화 — 이 범위의 재구성 전 상태 보존
- 이 폴더의 문서는 참고용이며 현재 개발 기준으로 사용하지 않는다. 현재 기준은 [FINAL_IMPLEMENTATION_SPEC](../../system/FINAL_IMPLEMENTATION_SPEC.md)에서 확인한다.
- `docs/ecus/Lighting_Ambient_LIN_CAN`은 새 구조에서 `docs/ecus/Lighting_LIN_CAN`으로 이름이 바뀌었다. 이 스냅샷에는 이전 이름 그대로 보존되어 있다.
- IVI(H735 Cockpit)는 이번 변경에서 실기 구현/테스트 결과를 삭제하지 않았다. 이 스냅샷의 `ecus/IVI/`는 변경 전 문서 diff 확인용이며, 실제 유지되는 최신 IVI 문서는 `docs/ecus/IVI/`를 그대로 참고한다.
