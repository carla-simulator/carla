cmake_minimum_required(VERSION 3.28)
project(carla_msgs)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CarlaCollisionEvent.msg"
  "msg/CarlaEgoVehicleControl.msg"
  "msg/LaneInvasionEvent.msg"
  DEPENDENCIES std_msgs geometry_msgs
)

ament_package()
