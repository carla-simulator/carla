cmake_minimum_required (VERSION 3.20.0)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0140 NEW)
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
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4/Plugins
)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (FetchContent)
include (${CARLA_WORKSPACE_PATH}/CMake/CarlaOptions.cmake)

set (
  CMAKE_INSTALL_PREFIX
  ${CARLA_DEPENDENCIES_INSTALL_PREFIX}
)

if (WIN32)
  set (LIB_EXT .lib)
else ()
  set (LIB_EXT .a)
endif ()

set (CARLA_DEFAULT_DEPENDENCY_ARGS)

if (CMAKE_C_COMPILER)
  list (
    APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  )
endif ()

if (CMAKE_INSTALL_PREFIX)
  list (
    APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  )
endif ()

if (CMAKE_CXX_COMPILER)
  list (
    APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  )
endif ()

if (CMAKE_BUILD_TYPE)
  list (
    APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )
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
endif ()

set (CARLA_EXCEPTION_DEFINITIONS)

if (ENABLE_EXCEPTIONS)
else ()
  list (APPEND CARLA_EXCEPTION_DEFINITIONS ASIO_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS BOOST_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS LIBCARLA_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS PUGIXML_NO_EXCEPTIONS)
endif ()

set (CARLA_RTTI_DEFINITIONS)

add_compile_definitions (_USE_MATH_DEFINES)

if (ENABLE_RTTI)
else ()
  list (
    APPEND
    CARLA_RTTI_DEFINITIONS
    BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY
  )
endif ()

if (BUILD_PYTHON_API)
  set (BOOST_ENABLE_PYTHON ${BUILD_PYTHON_API})
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

macro (carla_two_step_configure_file DESTINATION SOURCE)
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION})
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endmacro ()

carla_two_step_configure_file (
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4/Plugins/Carla/Source/Carla/Carla.Build.cs
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4/Plugins/Carla/Source/Carla/Carla.Build.cs.in
)

carla_two_step_configure_file (
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4/Plugins/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4/Plugins/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs.in
)