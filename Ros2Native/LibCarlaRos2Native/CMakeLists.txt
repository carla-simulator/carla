cmake_minimum_required (
  VERSION
  3.27.2
)

project (
  carla-ros2-native
)

set (
  LIBCARLA_SOURCE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/../../LibCarla/source
)

file (
  GLOB
  LIBCARLA_ROS2_SOURCES
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.cpp
)

file (
  GLOB
  LIBCARLA_ROS2_HEADERS
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.h
)

add_library (
  carla-ros2-native
  SHARED
  ${LIBCARLA_ROS2_HEADERS}
  ${LIBCARLA_ROS2_SOURCES}
)

file (
  GLOB
  ROS2_INCLUDE_DIRS
  ${CMAKE_INSTALL_PREFIX}/include/*/
)
# CycloneDDS installs a features.h that shadows glibc's features.h
list (FILTER ROS2_INCLUDE_DIRS EXCLUDE REGEX "/include/(dds|ddsc|idl|idlc)$")

target_include_directories (
  carla-ros2-native
  SYSTEM PRIVATE
  ${LIBCARLA_SOURCE_PATH}
  ${ROS2_INCLUDE_DIRS}
)

target_link_libraries (
  carla-ros2-native PUBLIC
  ${CMAKE_INSTALL_PREFIX}/lib/librclcpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libsensor_msgs__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libnav_msgs__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libgeometry_msgs__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libstd_msgs__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libtf2_msgs__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libbuiltin_interfaces__rosidl_typesupport_cpp.so
  ${CMAKE_INSTALL_PREFIX}/lib/libcarla_msgs__rosidl_typesupport_cpp.so
)

install (
  TARGETS
  carla-ros2-native
  DESTINATION
  lib
)
