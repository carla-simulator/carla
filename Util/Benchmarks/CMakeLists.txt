# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

cmake_minimum_required(VERSION 3.20)
project(CarlaClientBenchmarks LANGUAGES CXX)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_path(CARLA_RPCLIB_INCLUDE_DIR rpc/msgpack.hpp REQUIRED)

get_filename_component(CARLA_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../LibCarla/source" ABSOLUTE)
add_executable(carla-recurrent-future-benchmark recurrent_shared_future.cpp)
target_compile_features(carla-recurrent-future-benchmark PRIVATE cxx_std_20)
target_include_directories(carla-recurrent-future-benchmark PRIVATE "${CARLA_SOURCE}")
target_include_directories(carla-recurrent-future-benchmark SYSTEM PRIVATE "${CARLA_RPCLIB_INCLUDE_DIR}")
target_link_libraries(carla-recurrent-future-benchmark PRIVATE Boost::headers Threads::Threads)

if(MSVC)
  target_compile_definitions(carla-recurrent-future-benchmark PRIVATE
    NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
else()
  target_compile_options(carla-recurrent-future-benchmark PRIVATE -Wall -Wextra)
endif()
