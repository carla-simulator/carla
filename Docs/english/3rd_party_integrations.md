# 서드파티 통합

CARLA는 유용성과 확장성을 극대화하기 위해 여러 서드파티 애플리케이션과의 통합을 지원하도록 개발되었습니다.

-   [__ROS 브리지__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)
-   [__SUMO__](adv_sumo.md)  
-   [__Scenic__](tuto_G_scenic.md)
-   [__CarSIM__](tuto_G_carsim_integration.md)
-   [__Chrono__](tuto_G_chrono.md)
-   [__ASAM OpenDRIVE__](adv_opendrive.md) 
-   [__PTV Vissim__](adv_ptv.md)
-   [__RSS__](adv_rss.md) 
-   [__AWS와 RLlib__](tuto_G_rllib_integration.md)

---
## ROS 브리지

__ROS 브리지의 전체 문서는 [__여기__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)에서 확인할 수 있습니다.__

ROS 브리지는 ROS와 CARLA 간의 양방향 통신을 가능하게 합니다. CARLA 서버의 정보는 ROS 토픽으로 변환됩니다. 마찬가지로 ROS의 노드 간에 전송되는 메시지는 CARLA에서 적용될 명령으로 변환됩니다.

ROS 브리지는 ROS 1과 ROS 2 모두와 호환됩니다.

ROS 브리지는 다음과 같은 기능을 제공합니다:

- LIDAR, Semantic LIDAR, 카메라(깊이, 분할, RGB, DVS), GNSS, 레이더, IMU의 센서 데이터 제공
- 변환, 신호등 상태, 시각화 마커, 충돌 및 차선 침범과 같은 객체 데이터 제공
- 조향, 가속, 제동을 통한 자율주행 에이전트 제어
- 동기 모드, 시뮬레이션 실행/일시 정지, 시뮬레이션 매개변수 설정과 같은 CARLA 시뮬레이션 측면 제어

---

## SUMO

CARLA는 [__SUMO__](https://www.eclipse.org/sumo/)와의 코시뮬레이션 기능을 개발했습니다. 이를 통해 작업을 원하는 대로 분산하고 각 시뮬레이션의 기능을 사용자에게 유리하게 활용할 수 있습니다.

전체 문서는 [__여기__](adv_sumo.md)를 참조하세요.

---

## PTV Vissim

[__PTV Vissim__](https://www.ptvgroup.com/en/solutions/products/ptv-vissim/)은 강력한 GUI를 갖춘 종합적인 교통 시뮬레이션 솔루션을 제공하는 독점 소프트웨어 패키지입니다. CARLA에서 PTV-Vissim을 사용하는 방법은 [__이 가이드__](adv_ptv.md)를 참조하세요.

---

## Scenic

Scenic은 시나리오 명세와 장면 생성을 위한 라이브러리와 언어 세트입니다. CARLA와 Scenic은 원활하게 함께 작동할 수 있으며, Scenic을 CARLA와 함께 사용하는 방법은 [__이 가이드__](tuto_G_scenic.md)를 참조하세요.

Scenic에 대해 더 자세히 알아보려면 ["Scenic 시작하기"](https://scenic-lang.readthedocs.io/en/latest/quickstart.html) 가이드를 읽고 [정적](https://scenic-lang.readthedocs.io/en/latest/tutorials/tutorial.html) 및 [동적](https://scenic-lang.readthedocs.io/en/latest/tutorials/dynamics.html) 시나리오 생성 튜토리얼을 살펴보세요.

---

## CarSIM

CARLA의 CarSim 통합을 통해 CARLA의 차량 제어를 CarSim으로 전달할 수 있습니다. CarSim은 차량의 모든 필요한 물리 계산을 수행하고 새로운 상태를 CARLA로 반환합니다.

CARLA를 CarSIM과 함께 사용하는 방법은 [여기](tuto_G_carsim_integration.md)에서 확인할 수 있습니다.

## ASAM OpenDRIVE

[__ASAM OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/)는 도로 네트워크의 로직을 설명하는 데 사용되는 개방형 형식 사양으로, 디지털 형식의 도로 네트워크 설명을 표준화하고 다른 애플리케이션이 도로 네트워크 데이터를 교환할 수 있도록 합니다. 전체 문서는 [__여기__](adv_opendrive.md)를 참조하세요.

## RSS - 책임 민감형 안전

CARLA는 클라이언트 라이브러리에 [책임 민감형 안전을 위한 C++ 라이브러리](https://github.com/intel/ad-rss-lib)를 통합합니다. 이 기능을 통해 사용자는 아무것도 구현하지 않고도 RSS 동작을 조사할 수 있습니다. CARLA는 입력 제공과 실시간으로 자율주행 시스템에 출력을 적용하는 작업을 처리합니다. 전체 문서는 [__여기__](adv_rss.md)를 참조하세요.

## AWS와 RLlib 통합

RLlib 통합은 Ray/RLlib 라이브러리와 CARLA 간의 지원을 제공하여 학습 및 추론 목적으로 CARLA 환경을 쉽게 사용할 수 있게 합니다. Ray는 분산 애플리케이션을 구축하기 위한 간단하고 범용적인 API를 제공하는 오픈 소스 프레임워크입니다. Ray는 확장 가능한 강화학습 라이브러리인 RLlib과 확장 가능한 하이퍼파라미터 튜닝 라이브러리인 Tune과 함께 패키징됩니다. AWS와 RLlib에서 CARLA를 운영하는 방법에 대해 [__여기__](tuto_G_rllib_integration.md)에서 자세히 알아보세요.

## Chrono 물리엔진

[__Chrono__](https://projectchrono.org/)는 템플릿을 사용하여 높은 현실감의 차량 동역학을 제공하는 멀티 물리 시뮬레이션 엔진입니다. CARLA의 Chrono 통합을 통해 CARLA 사용자는 차량 동역학을 시뮬레이션하기 위해 Chrono 템플릿을 추가할 수 있습니다. 전체 문서는 [__여기__](tuto_G_chrono.md)를 참조하세요.

---