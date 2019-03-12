cmake_minimum_required(VERSION 3.5.1)
project(libcarla)

set(libcarla_source_path "${PROJECT_SOURCE_DIR}/../source")

include_directories(${libcarla_source_path})

if (CARLA_VERSION)
  configure_file(${libcarla_source_path}/carla/Version.h.in ${libcarla_source_path}/carla/Version.h)
endif()

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
