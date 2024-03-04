cmake_minimum_required (VERSION 3.20.0)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0091 NEW)
cmake_policy (SET CMP0135 NEW)
cmake_policy (SET CMP0077 OLD)

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 9)
set (CARLA_VERSION_PATCH 15)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

project (
  CARLA
  VERSION ${CARLA_VERSION}
  LANGUAGES
    C
    CXX
    ASM # This is required by some dependencies, such as LibPNG.
  DESCRIPTION "Open-source simulator for autonomous driving research."
  HOMEPAGE_URL https://carla.org
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
include (${CARLA_WORKSPACE_PATH}/CMake/CarlaOptions.cmake)

macro (carla_two_step_configure_file DESTINATION SOURCE)
  message ("Configuring file \"${DESTINATION}\"")
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION})
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endmacro ()

set (
  CMAKE_INSTALL_PREFIX
  ${CARLA_DEPENDENCIES_INSTALL_PREFIX}
)

if (WIN32)
  set (LIB_EXT .lib)
else ()
  set (LIB_EXT .a)
endif ()

if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CARLA_DEBUG_AFFIX d)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
  else ()
    set (CARLA_DEBUG_AFFIX )
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif ()
endif ()

set (
  CARLA_COMMON_DEFINITIONS
  LIBCARLA_IMAGE_WITH_PNG_SUPPORT=true
)

if (WIN32)
    # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # --- Windows 10 ---
  list (APPEND CARLA_COMMON_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
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

if (BUILD_PYTHON_API)
  add_subdirectory (PythonAPI)
endif ()

if (BUILD_CARLA_UE)
  add_subdirectory (Unreal)
endif ()

carla_two_step_configure_file (
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Carla.Build.cs
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Carla.Build.cs.in
)

carla_two_step_configure_file (
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs.in
)
