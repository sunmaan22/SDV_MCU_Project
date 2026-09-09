# IVI / Cluster Cockpit Documentation

이 폴더는 **B 담당: STM32H735 + TouchGFX Cluster/IVI Cockpit**의 작성 예시다.

현재 공통 Template을 실제 프로젝트 역할에 맞춰 채운 상태이며, 실제 구현/시험이 진행되면 `TBD`, `NOT RUN` 항목을 실제 값으로 교체한다.

## 문서

1. [SPECIFICATION.md](SPECIFICATION.md)
   - 무엇을 해야 하는지
   - Input / Output
   - 화면/사용 시나리오
   - Functional Requirement
   - CAN Interface
   - Timeout / Edge Case
   - Acceptance Criteria

2. [ARCHITECTURE.md](ARCHITECTURE.md)
   - 어떻게 구현할지
   - Context / Component / Runtime / Deployment View
   - TouchGFX 구조
   - Vehicle Data Model
   - Warning / DTC / CAN 처리 구조
   - Architecture Decision / Risk / Traceability

3. [TEST_REPORT.md](TEST_REPORT.md)
   - Requirement별 Test ID
   - Dummy Data Stage 1 시험
   - CAN 통합 시험
   - Timeout / Invalid / Warning 시험
   - Timing 측정
   - DTC 표시 시험

## 역할 한 줄 요약

```text
다른 ECU가 만든 차량 정보
        ↓ CAN FD
STM32H735 Vehicle Data Model
        ↓
Cluster + IVI TouchGFX
        ↓
운전자에게 상태 / 경고 / DTC 표시
```

사용자 설정은 직접 Actuator를 움직이지 않는다.

```text
Touch Input
   ↓
H735 Request
   ↓ CAN FD
VCU / Body Gateway
   ↓
실제 기능 수행
```

## 현재 주요 TBD

- CAN ID / DLC / Signal bit layout
- H735 FDCAN Transceiver와 실제 Pin Map
- DTC Clear Request protocol
- Battery SOC의 데이터 Owner/계산 방식
- TouchGFX 실제 latency / update 성능

이 항목들은 추측해서 확정하지 않고 해당 담당자와 통합하면서 채운다.
