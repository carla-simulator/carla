# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

cmake_minimum_required(VERSION 3.20)
project(CarlaClientPrimitiveTests LANGUAGES CXX)

enable_testing()
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

get_filename_component(CARLA_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../../LibCarla/source" ABSOLUTE)
add_executable(carla-client-primitives
  "${CARLA_SOURCE}/test/client/test_recurrent_shared_future.cpp"
  "${CARLA_SOURCE}/test/client/test_recurrent_shared_future_lifetime.cpp")
target_compile_features(carla-client-primitives PRIVATE cxx_std_20)
target_compile_definitions(carla-client-primitives PRIVATE LIBCARLA_WITH_GTEST)
target_include_directories(carla-client-primitives PRIVATE "${CARLA_SOURCE}" "${CARLA_SOURCE}/test")
target_link_libraries(carla-client-primitives PRIVATE Boost::headers GTest::gtest_main Threads::Threads)

set(CARLA_PRIMITIVE_SANITIZERS "" CACHE STRING "Optional compiler sanitizers, e.g. address,undefined")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(carla-client-primitives PRIVATE -Wall -Wextra)
  if(CARLA_PRIMITIVE_SANITIZERS)
    target_compile_options(carla-client-primitives PRIVATE "-fsanitize=${CARLA_PRIMITIVE_SANITIZERS}" -fno-omit-frame-pointer)
    target_link_options(carla-client-primitives PRIVATE "-fsanitize=${CARLA_PRIMITIVE_SANITIZERS}")
  endif()
endif()

include(GoogleTest)
gtest_discover_tests(carla-client-primitives)
