#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

set (
  CARLA_CMAKE_MINIMUM_REQUIRED_VERSION
  3.27.2
)

cmake_minimum_required (
  VERSION
  ${CARLA_CMAKE_MINIMUM_REQUIRED_VERSION}
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
    C
    CXX
    ASM
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
  HOMEPAGE_URL
    https://carla.org
)

set (
  CARLA_WORKSPACE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (
  CARLA_BUILD_PATH
  ${CMAKE_BINARY_DIR}
)

set (
  CARLA_PACKAGE_PATH
  ${CARLA_BUILD_PATH}/Package
)

set (
  CARLA_UNREAL_PLUGINS_PATH
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins
)

set (
  CARLA_LICENSE_FILE_PATH
  ${CARLA_WORKSPACE_PATH}/LICENSE
)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (CheckLinkerFlag)
include (${CARLA_WORKSPACE_PATH}/CMake/Util.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/Options.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/Common.cmake)
include (${CARLA_WORKSPACE_PATH}/CMake/Dependencies.cmake)

if (BUILD_CARLA_CLIENT OR BUILD_CARLA_SERVER)
  add_subdirectory (LibCarla)
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  add_subdirectory (osm-world-renderer)
endif ()

if (ENABLE_ROS2)
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

carla_add_custom_target (
  carla-help
  "Display this message."
  COMMAND ${CMAKE_COMMAND} -E cat "${CMAKE_CURRENT_BINARY_DIR}/Help.md"
)

carla_get_option_docs (CARLA_OPTION_DOCS)
carla_get_target_docs (CARLA_TARGET_DOCS)
set (CARLA_CMAKE_HELP_MESSAGE)
string (
  APPEND 
  CARLA_CMAKE_HELP_MESSAGE
  "# CARLA - CMake Help\n"
  "## CMake Targets\n"
  "${CARLA_TARGET_DOCS}\n"
  "## CMake Options\n"
  "${CARLA_OPTION_DOCS}\n"
)

file (
  WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/Help.md
  ${CARLA_CMAKE_HELP_MESSAGE}
)

if (CMAKE_EXPORT_COMPILE_COMMANDS)
  configure_file (
    ${CARLA_WORKSPACE_PATH}/.clangd.in
    ${CARLA_WORKSPACE_PATH}/.clangd
  )
endif ()
