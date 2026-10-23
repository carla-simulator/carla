# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

cmake_minimum_required(VERSION 3.20)
project(CarlaStreamListenerTests LANGUAGES CXX)

enable_testing()
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_path(CARLA_RPCLIB_INCLUDE_DIR rpc/msgpack.hpp REQUIRED)

get_filename_component(CARLA_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../../LibCarla/source" ABSOLUTE)
add_library(carla-stream-listener-subset STATIC
  "${CARLA_SOURCE}/carla/Buffer.cpp"
  "${CARLA_SOURCE}/carla/streaming/detail/StreamStateBase.cpp"
  "${CARLA_SOURCE}/carla/streaming/detail/Token.cpp"
  "${CARLA_SOURCE}/carla/streaming/detail/Dispatcher.cpp"
  "${CARLA_SOURCE}/carla/streaming/detail/tcp/Server.cpp"
  "${CARLA_SOURCE}/carla/streaming/detail/tcp/ServerSession.cpp")
target_compile_features(carla-stream-listener-subset PUBLIC cxx_std_20)
target_include_directories(carla-stream-listener-subset PUBLIC "${CARLA_SOURCE}")
target_include_directories(carla-stream-listener-subset SYSTEM PUBLIC "${CARLA_RPCLIB_INCLUDE_DIR}")
target_link_libraries(carla-stream-listener-subset PUBLIC Boost::headers Threads::Threads)

add_executable(carla-stream-listener-tests "${CARLA_SOURCE}/test/common/test_stream_listener_state.cpp")
target_include_directories(carla-stream-listener-tests PRIVATE "${CARLA_SOURCE}/test")
target_link_libraries(carla-stream-listener-tests PRIVATE carla-stream-listener-subset GTest::gtest_main)
add_test(NAME stream-listener-state COMMAND carla-stream-listener-tests)
set_tests_properties(stream-listener-state PROPERTIES TIMEOUT 60)

option(CARLA_STREAM_BUILD_BENCHMARK "Build the sequential listener getter benchmark" OFF)
if(CARLA_STREAM_BUILD_BENCHMARK)
  add_executable(carla-stream-listener-benchmark benchmark.cpp benchmark_poll.cpp)
  target_link_libraries(carla-stream-listener-benchmark PRIVATE carla-stream-listener-subset)
  # The separate translation unit prevents loop hoisting of the getter.
  set_property(TARGET carla-stream-listener-benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

set(CARLA_STREAM_SANITIZERS "" CACHE STRING "Optional sanitizers: thread or address,undefined")
set(stream_targets carla-stream-listener-subset carla-stream-listener-tests)
if(CARLA_STREAM_BUILD_BENCHMARK)
  list(APPEND stream_targets carla-stream-listener-benchmark)
endif()
foreach(target IN LISTS stream_targets)
  if(MSVC)
    target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra)
  endif()
  if(CARLA_STREAM_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" OR MSVC)
      message(FATAL_ERROR "CARLA_STREAM_SANITIZERS requires GCC or Clang with -fsanitize support")
    endif()
    target_compile_options(${target} PRIVATE "-fsanitize=${CARLA_STREAM_SANITIZERS}" -fno-omit-frame-pointer)
    target_link_options(${target} PRIVATE "-fsanitize=${CARLA_STREAM_SANITIZERS}")
  endif()
endforeach()
if(WIN32)
  target_link_libraries(carla-stream-listener-subset PUBLIC ws2_32)
endif()
