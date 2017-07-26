cmake_minimum_required(VERSION 3.5.0)
project(CARLA)

set(CARLA_UTIL_PATH "${PROJECT_SOURCE_DIR}/..")

set(CARLA_LIBCXX_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/llvm-install")
set(CARLA_BOOST_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/boost-install")
set(CARLA_PROTOBUF_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/protobuf-install")
set(CARLA_GOOGLETEST_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/googletest-install")

add_subdirectory("CarlaServer")
