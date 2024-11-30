# ROS

![ros_carla](../img/ros_carla.png)

[__Robotic Operating System (ROS)__](https://www.ros.org/)은 로봇 및 자율주행 응용 프로그램을 위한 소프트웨어 라이브러리 모음입니다. CARLA는 ROS와 직접 연결될 수 있으며, 이를 통해 CARLA의 액터에 제어 신호를 전달하거나 센서 데이터를 ROS 주제를 통해 접근할 수 있습니다.

CARLA와 ROS를 연결하는 방법에는 두 가지가 있습니다.

- __CARLA 네이티브 인터페이스__: CARLA 서버에 직접 내장된 ROS 인터페이스  
- __ROS 브리지__: ROS와 CARLA 간 신호를 전송하기 위한 별도의 라이브러리  

## CARLA 네이티브 ROS 인터페이스

이 방법은 가장 낮은 지연 시간으로 최고의 성능을 제공하므로 권장되는 인터페이스입니다. 현재 네이티브 인터페이스는 ROS 2만 지원합니다. ROS 1을 사용하는 경우 ROS 브리지를 사용해야 합니다.

## CARLA ROS 브리지

[__CARLA ROS 브리지__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)는 ROS를 CARLA와 연결하기 위한 라이브러리로, ROS 1과 ROS 2 모두 호환됩니다. CARLA ROS 브리지는 별도의 패키지이므로 네이티브 인터페이스에 비해 추가적인 지연 시간이 발생합니다. ROS 브리지는 ROS 1과 ROS 2의 레거시 구현을 지원하기 위해 여전히 제공됩니다. 