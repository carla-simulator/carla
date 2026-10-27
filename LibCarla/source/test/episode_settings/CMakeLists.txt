cmake_minimum_required(VERSION 3.28)
project(EpisodeSettingsConversionTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
get_filename_component(default_root "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(CARLA_SOURCE_ROOT "${default_root}" CACHE PATH "CARLA source containing the conversion under test")
set(CARLA_BOOST_SOURCE_DIR "" CACHE PATH "Configured modular Boost source")
set(CARLA_RPCLIB_SOURCE_DIR "" CACHE PATH "Configured rpclib source")
if(NOT EXISTS "${CARLA_BOOST_SOURCE_DIR}/libs/asio/include/boost/asio.hpp" OR
   NOT EXISTS "${CARLA_RPCLIB_SOURCE_DIR}/include/rpc/msgpack.hpp")
  message(FATAL_ERROR "Set CARLA_BOOST_SOURCE_DIR and CARLA_RPCLIB_SOURCE_DIR to configured dependency sources")
endif()
file(GLOB boost_includes "${CARLA_BOOST_SOURCE_DIR}/libs/*/include"
                         "${CARLA_BOOST_SOURCE_DIR}/libs/numeric/*/include")
add_executable(episode_settings_units test_episode_settings_units.cpp)
target_include_directories(episode_settings_units PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/unreal_boundary"
  "${CARLA_SOURCE_ROOT}/LibCarla/source"
  "${CARLA_SOURCE_ROOT}/Unreal/CarlaUnreal/Plugins/Carla/Source")
target_include_directories(episode_settings_units SYSTEM PRIVATE
  ${boost_includes} "${CARLA_RPCLIB_SOURCE_DIR}/include")
target_compile_definitions(episode_settings_units PRIVATE
  BOOST_ALL_NO_LIB RPCLIB_MSGPACK=clmdep_msgpack)
if(WIN32)
  target_compile_definitions(episode_settings_units PRIVATE _WIN32_WINNT=0x0601)
endif()
if(MSVC)
  target_compile_options(episode_settings_units PRIVATE /W4 /permissive-)
else()
  target_compile_options(episode_settings_units PRIVATE -Wall -Wextra -Wpedantic)
endif()
enable_testing()
add_test(NAME episode_settings_units COMMAND episode_settings_units)
