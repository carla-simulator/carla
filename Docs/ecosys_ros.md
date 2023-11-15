# ROS

![ros_carla](../img/ros_carla.png)

The [__Robotic Operating System (ROS)__](https://www.ros.org/) is a set of software libraries for robotic and autonomous driving applications. CARLA can be directly connected to ROS through its ROS interfaces, control signals can be sent to CARLA actors and sensor data can be accessed through ROS topics. 

There are two options for connecting CARLA and ROS.

- __CARLA native interface__: a ROS interface build directly into the CARLA server
- __ROS Bridge__: a separate library for transfering signals between ROS and CARLA

## CARLA native ROS interface

This is the recommended interface, since it offers the best performance with the lowest latency. At the moment the native interface only supports ROS 2. If you are using ROS 1, you must use the ROS Bridge. 

## CARLA ROS Bridge

The [__CARLA ROS Bridge__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/) is a library for connecting ROS to CARLA, it is compatible with both ROS 1 and ROS 2. Since the CARLA ROS Bridge is a separate package, there is additional latency compared to the native interface. The ROS Bridge is still provide to support ROS 1 and legacy implementations with ROS 2. 

