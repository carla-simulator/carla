cmake_minimum_required (
  VERSION
  3.27.2
)

project (
  carla-ros2-native
)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (
  LIBCARLA_SOURCE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/../../LibCarla/source
)

file (
  GLOB
  LIBCARLA_ROS2_SOURCES
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/fastdds/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/cyclonedds/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/zenoh/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/listeners/*.cpp
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/types/*.cpp
)

file (
  GLOB
  LIBCARLA_ROS2_HEADERS
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/fastdds/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/cyclonedds/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/middleware/zenoh/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/listeners/*.h
  ${LIBCARLA_SOURCE_PATH}/carla/ros2/types/*.h
)

add_library (
  carla-ros2-native
  SHARED
  ${LIBCARLA_ROS2_HEADERS}
  ${LIBCARLA_ROS2_SOURCES}
)

target_include_directories (
  carla-ros2-native
  SYSTEM PRIVATE
  ${LIBCARLA_SOURCE_PATH}
  ${CMAKE_INSTALL_PREFIX}/include
)

target_compile_definitions (
  carla-ros2-native
  PUBLIC
  BOOST_ASIO_ENABLE_BUFFER_DEBUGGING
  PRIVATE
  CARLA_ROS2_MIDDLEWARE_FASTDDS
  CARLA_ROS2_MIDDLEWARE_CYCLONEDDS
  CARLA_ROS2_MIDDLEWARE_ZENOH
)

target_link_libraries (
  carla-ros2-native PUBLIC
  ${CMAKE_INSTALL_PREFIX}/lib/libfastrtps.so
  ${CMAKE_INSTALL_PREFIX}/lib/libddsc.so
  # zenoh-c is a static PIC archive (Rust); link it into the shared lib and
  # pull in the system libraries the Rust runtime needs. dl is also required
  # by ZenohSharedSession's dladdr-based config path resolution.
  ${CMAKE_INSTALL_PREFIX}/lib/libzenohc.a
  pthread
  ${CMAKE_DL_LIBS}
  m
  rt
)

install (
  TARGETS
  carla-ros2-native
  DESTINATION
  lib
)
