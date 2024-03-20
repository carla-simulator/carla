#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

cmake_minimum_required (
  VERSION
  3.28.0
)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0091 NEW)
cmake_policy (SET CMP0074 NEW)
cmake_policy (SET CMP0077 NEW)
if (${CMAKE_MINOR_VERSION} GREATER_EQUAL 24)
  cmake_policy (SET CMP0135 NEW)
endif ()

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 9)
set (CARLA_VERSION_PATCH 15)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

project (
  CARLA
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    C
    CXX
    ASM # This is required by some dependencies, such as LibPNG.
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
  HOMEPAGE_URL
    https://carla.org
)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (
  CARLA_WORKSPACE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (
  CARLA_UNREAL_PLUGINS_PATH
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins
)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (CheckLinkerFlag)
include (${CARLA_WORKSPACE_PATH}/CMake/Util.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/CarlaOptions.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/Warnings.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/Errors.cmake)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  carla_warning (
    "CARLA is not set to be built in Release mode. This may cause issues when building CarlaUnrealEditor."
  )
endif ()

if (LINUX)
  check_linker_flag (CXX -lpthread HAS_PTHREAD)
  if (HAS_PTHREAD)
    add_link_options (-lpthread)
  endif ()
endif ()

# Similar to configure_file, but also expands variables
# that are set at generate time, like generator expressions.
macro (carla_two_step_configure_file DESTINATION SOURCE)
  carla_message ("Configuring file ${DESTINATION}")
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION})
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endmacro ()



set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CARLA_DEBUG_AFFIX d)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
  else ()
    set (CARLA_DEBUG_AFFIX )
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif ()
endif ()



set (CARLA_COMMON_DEFINITIONS)

foreach (FORMAT ${LIBCARLA_IMAGE_SUPPORTED_FORMATS})
  carla_message ("Enabling CARLA image support for \"${FORMAT}\".")
  string (TOUPPER "${FORMAT}" FORMAT_UPPERCASE)
  list (APPEND CARLA_COMMON_DEFINITIONS LIBCARLA_IMAGE_SUPPORT_${FORMAT_UPPERCASE}=1)
endforeach ()

if (WIN32)
  add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
endif ()

if (WIN32)
  # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # <- Windows 10
  list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
  list (APPEND CARLA_COMMON_DEFINITIONS _USE_MATH_DEFINES)
endif ()

set (CARLA_EXCEPTION_DEFINITIONS)

if (ENABLE_EXCEPTIONS)
  # Nothing
else ()
  list (APPEND CARLA_EXCEPTION_DEFINITIONS ASIO_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS BOOST_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS LIBCARLA_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS PUGIXML_NO_EXCEPTIONS)
endif ()

include (${CARLA_WORKSPACE_PATH}/CMake/CarlaDependencies.cmake)

if (BUILD_CARLA_CLIENT OR BUILD_CARLA_SERVER)
  add_subdirectory (LibCarla)
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  add_subdirectory (osm-world-renderer)
endif ()

if (ENABLE_ROS2)
  set (BOOST_INCLUDE_PATH ${CMAKE_BINARY_DIR}/_deps/boost-src/libs)
  add_subdirectory (Ros2Native)
endif()

if (BUILD_PYTHON_API)
  add_subdirectory (PythonAPI)
endif ()

if (BUILD_CARLA_UNREAL)
  add_subdirectory (Unreal)
endif ()

if (BUILD_EXAMPLES)
  add_subdirectory (Examples)
endif ()
