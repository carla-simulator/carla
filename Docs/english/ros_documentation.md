# ROS 브리지

__ROS 브리지의 전체 문서는 [__여기__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)에서 확인할 수 있습니다.__

---

ROS 브리지는 ROS와 CARLA 간의 양방향 통신을 가능하게 합니다. CARLA 서버의 정보는 ROS 토픽으로 변환됩니다. 마찬가지로, ROS의 노드 간에 전송되는 메시지는 CARLA에서 적용될 명령으로 변환됩니다.

ROS 브리지는 ROS 1과 ROS 2 모두와 호환됩니다.

ROS 브리지는 다음과 같은 기능을 제공합니다:

- LIDAR, Semantic LIDAR, 카메라(깊이, 분할, RGB, DVS), GNSS, 레이더, IMU에 대한 센서 데이터 제공
- 변환, 신호등 상태, 시각화 마커, 충돌 및 차선 침범과 같은 객체 데이터 제공
- 조향, 가속, 제동을 통한 자율주행 에이전트 제어
- 동기 모드, 시뮬레이션 실행/일시 정지, 시뮬레이션 매개변수 설정과 같은 CARLA 시뮬레이션 측면 제어