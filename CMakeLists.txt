#[[

  Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

cmake_minimum_required (
  VERSION
  3.30
)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0091 NEW)
cmake_policy (SET CMP0074 NEW)
cmake_policy (SET CMP0077 NEW)
cmake_policy (SET CMP0117 NEW)

if (${CMAKE_MINOR_VERSION} GREATER_EQUAL 24)
  cmake_policy (SET CMP0135 NEW)
endif ()

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 10)
set (CARLA_VERSION_PATCH 0)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

project (
  CARLA
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    C CXX ASM
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
  HOMEPAGE_URL
    https://carla.org
)

set (CARLA_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (CheckLinkerFlag)



if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message (WARNING 
    "CARLA is set to be built in Debug mode. This may cause issues when building CarlaUnrealEditor."
  )
endif ()

if (LINUX)
  check_linker_flag (CXX -lpthread HAS_PTHREAD)
  if (HAS_PTHREAD)
    add_link_options (-lpthread)
  endif ()
endif ()

if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
  else ()
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif ()
endif ()

if (WIN32)
  check_cxx_compiler_flag (/utf-8 HAS_MSVC_UTF8)
  if (HAS_MSVC_UTF8)
    # @TODO This causes warnings with MASM. A better approach should be looked into.
    add_compile_options ($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
  endif ()
endif ()

include (${CARLA_ROOT}/CMake/Common.cmake)

option (BUILD_SERVER "Build the CARLA server." ON)
option (BUILD_CLIENT "Build the CARLA client." ON)
option (BUILD_PYTHON_API "Build the CARLA Python API." OFF)
option (BUILD_EXAMPLES "Build CARLA examples." ON)
option (ENABLE_ROS2 "Enable ROS2." OFF)
option (ENABLE_ROS2_DEMO "Enable ROS2 demo." OFF)
option (ENABLE_OSM2ODR "Enable OSM2ODR." OFF)
option (ENABLE_RSS "Enable RSS components (ad-rss-lib)" OFF)
option (BUILD_OSM_WORLD_RENDERER "Build OSM World Renderer." OFF)
option (ENABLE_RTTI "Enable C++ RTTI." ON)
option (ENABLE_EXCEPTIONS "Enable C++ Exceptions." ON)
option (PREFER_CLONE "Whether to clone dependencies instead of directly downloading a compressed archive." OFF)
option (ENABLE_PEP517 "Whether to use PEP 517." ON)
option (ENABLE_ALL_WARNINGS "Whether to emit extra build warnings." ON)
option (ENABLE_WARNINGS_TO_ERRORS "Whether to convert build warnings to errors." OFF)

option (ENABLE_STREETMAP "Whether to download the Streetmap UE plugin." ON)

include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/Boost.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/Eigen.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/LibOSMScout.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/LunaSVG.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/ZLIB.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/PNG.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/PROJ.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/Recast.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/RPC.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/SQLite3.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/Streetmap.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/XercesC.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/CMake/SuperBuild/LibCarla.cmake)

include (ExternalProject)

if (BUILD_PYTHON_API)
  add_subdirectory (PythonAPI)
endif ()

# if (BUILD_CARLA_UNREAL)
#   add_subdirectory (Unreal)
# endif ()

if (BUILD_OSM_WORLD_RENDERER)
  add_subdirectory (osm-world-renderer)
endif ()

if (ENABLE_ROS2)
  add_subdirectory (Ros2Native)
endif()

file (
  WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/targets.txt
  ${CARLA_TARGETS_HELPER_TEXT}
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/targets.txt)
  file (REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/targets.txt)
endif ()

file (
  CREATE_LINK
    ${CMAKE_CURRENT_BINARY_DIR}/targets.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/targets.txt
)

if (CMAKE_EXPORT_COMPILE_COMMANDS)
  configure_file (
    ${CARLA_ROOT}/.clangd.in
    ${CARLA_ROOT}/.clangd
  )
endif ()
