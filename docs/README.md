# Documentation Index

현재 문서는 **Architecture v1.1** 기준이다.

```text
Raspberry Pi 4 HPC
      ↕ CAN FD
VCU / Drive / Parking / Body Gateway / H735 Cockpit
                             │
                            LIN
                             │
                      Body LIN Slave
```

STM32H735는 **Cluster + IVI 통합 Cockpit ECU**이고, STM32 #4/#5는 각각 **Body Gateway LIN Master / Body LIN Slave** 역할을 한다.

## 처음 참여한 팀원 읽는 순서

1. [전자공학 선행학습](ELECTRONICS_PREREQUISITES_FOR_SW_TEAM.md)
2. [Architecture 작성 & Stage 1 Guide](BEGINNER_ARCHITECTURE_STAGE1_GUIDE.md)
3. [Sensor List](SENSOR_LIST.md)
4. [Node별 명세/Architecture 예시](NODE_SPEC_ARCHITECTURE_EXAMPLES.md)
5. [4주 개발 계획](WEEKLY_PLAN.md)

## 작성용 Template

- [Node Specification Template](templates/NODE_SPECIFICATION_TEMPLATE.md)
- [ECU / Node Architecture Template](templates/ECU_ARCHITECTURE_TEMPLATE.md)
- [Stage 1 Test Report Template](templates/STAGE1_TEST_REPORT_TEMPLATE.md)

## 팀원이 자기 담당 Node에서 만들어야 하는 문서

```text
SPECIFICATION.md
ARCHITECTURE.md
STAGE1_TEST_REPORT.md
```

Stage 1이 끝난 뒤 CAN/LIN 통합 결과와 DTC/Timing 시험 문서를 추가한다.

## 현재 역할

| 담당 | Node |
|---|---|
| A | STM32 #1 VCU / Driver Input / Safety |
| B | STM32 #2 Drive + Steering |
| C | Raspberry Pi 4 HPC / ADAS / DTC |
| D | STM32 #3 Parking + Rear Camera 협업 |
| E | STM32H735 Cluster + IVI Cockpit |
| F | STM32 #4 Body Gateway + STM32 #5 LIN Body Slave |

[Main README](../README.md)
