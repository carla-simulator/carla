cmake_minimum_required (VERSION 3.20.0)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0140 NEW)

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
  LANGUAGES CXX
  DESCRIPTION "Open-source simulator for autonomous driving research."
  HOMEPAGE_URL https://carla.org
)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)

set (CARLA_WORKSPACE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

list (
  APPEND
  CMAKE_MODULE_PATH
  ${CARLA_WORKSPACE_PATH}/CMake
)

list (
  APPEND
  CMAKE_PREFIX_PATH
  ${CARLA_WORKSPACE_PATH}/CMake
)

macro (carla_string_option NAME DESCRIPTION VALUE)
  set (${NAME} ${VALUE} CACHE STRING ${DESCRIPTION})
endmacro ()

carla_string_option (
  CARLA_DEPENDENCIES_PATH
  "Where to build CARLA dependencies."
  ${CMAKE_CURRENT_BINARY_DIR}/Dependencies
)

carla_string_option (
  CARLA_DEPENDENCIES_INSTALL_PREFIX
  "Where to install CARLA dependencies."
  ${CMAKE_CURRENT_SOURCE_DIR}/Install
)

set (
  CMAKE_INSTALL_PREFIX
  ${CARLA_DEPENDENCIES_INSTALL_PREFIX}
)

option (
  BUILD_CARLA_CLIENT
  "Build the LibCarla client."
  ON
)

option (
  BUILD_CARLA_SERVER
  "Build the LibCarla server."
  ON
)

option (
  ENABLE_LIBCARLA_PYTORCH
  "Enable LibCarla-PyTorch."
  OFF
)

option (
  BUILD_PYTHON_API
  "Build the CARLA Python API."
  ON
)

option (
  ENABLE_OSM2ODR
  "Enable OSM2ODR."
  ${BUILD_PYTHON_API}
)

option (
  ENABLE_RSS
  "Enable RSS components (ad-rss-lib)"
  OFF
)

option (
  INSTALL_LIBCARLA
  "Install LibCarla and all of its dependencies."
  ON
)

option (
  BUILD_OSM_WORLD_RENDERER
  "Build OSM World Renderer."
  ON
)

option (
  ENABLE_RTTI
  "Enable C++ RTTI."
  OFF
)

option (
  ENABLE_EXCEPTIONS
  "Enable C++ Exceptions."
  OFF
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
    LIBCARLA_IMAGE_WITH_PNG_SUPPORT=true)

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

if (ENABLE_RTTI)
else ()
    list (
      APPEND
      CARLA_RTTI_DEFINITIONS
      BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
endif ()

if (BUILD_CARLA_CLIENT OR BUILD_CARLA_SERVER)
  add_subdirectory (LibCarla)
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  add_subdirectory (osm-world-renderer)
endif ()

if (BUILD_PYTHON_API)
  add_subdirectory (PythonAPI)
endif ()
