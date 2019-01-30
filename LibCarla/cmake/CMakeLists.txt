cmake_minimum_required(VERSION 3.5.1)
project(libcarla)

message(STATUS "Building ${PROJECT_NAME} version ${CARLA_VERSION}")

set(libcarla_source_path "${PROJECT_SOURCE_DIR}/../source")

include_directories(${libcarla_source_path})

configure_file(${libcarla_source_path}/carla/Version.h.in ${libcarla_source_path}/carla/Version.h)

if (CMAKE_BUILD_TYPE STREQUAL "Client")
  add_subdirectory("client")
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  add_subdirectory("server")
else ()
  message(FATAL_ERROR "Unknown build type '${CMAKE_BUILD_TYPE}'")
endif ()

# GTest is not compiled on Windows.
if (NOT WIN32)
  add_subdirectory("test")
endif()
