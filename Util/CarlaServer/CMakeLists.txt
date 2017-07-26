cmake_minimum_required (VERSION 3.4.2)
project (CarlaServer)

# ==============================================================================
# -- Compiler config -----------------------------------------------------------
# ==============================================================================

if (UNIX)

  set(CMAKE_CXX_COMPILER /usr/bin/clang++)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -Werror -Wall -Wextra -Wno-unused-parameter")
  find_package(Threads)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG -O0")
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
  endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

endif (UNIX)

# ==============================================================================
# -- Suppress windows warning --------------------------------------------------
# ==============================================================================

# http://stackoverflow.com/a/40217291
if (WIN32)
  macro(get_WIN32_WINNT version)
    if (CMAKE_SYSTEM_VERSION)
      set(ver ${CMAKE_SYSTEM_VERSION})
      string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
      string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
      # Check for Windows 10, b/c we'll need to convert to hex 'A'.
      if ("${verMajor}" MATCHES "10")
          set(verMajor "A")
          string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
      endif ("${verMajor}" MATCHES "10")
      # Remove all remaining '.' characters.
      string(REPLACE "." "" ver ${ver})
      # Prepend each digit with a zero.
      string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
      set(${version} "0x${ver}")
    endif(CMAKE_SYSTEM_VERSION)
  endmacro(get_WIN32_WINNT)
  get_WIN32_WINNT(ver)
  add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)

# ==============================================================================
# -- Boost ---------------------------------------------------------------------
# ==============================================================================

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED system date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

# ==============================================================================
# -- Protobuf ------------------------------------------------------------------
# ==============================================================================

if (UNIX)

  include(FindProtobuf)
  find_package(Protobuf REQUIRED)
  include_directories(${PROTOBUF_INCLUDE_DIR})
  set(Protobuf_LIBRARIES protobuf)

elseif (WIN32)

  if (CMAKE_BUILD_TYPE MATCHES Debug)

    if (EXISTS $ENV{PROTOBUF_ROOT}/Debug)
      message(STATUS "Using Protobuf DEBUG at " $ENV{PROTOBUF_ROOT}/Debug)
      include_directories($ENV{PROTOBUF_ROOT}/Debug/include)
      link_directories($ENV{PROTOBUF_ROOT}/Debug/lib)
      set(Protobuf_LIBRARIES libprotobufd.lib)
    else (EXISTS $ENV{PROTOBUF_ROOT}/Debug)
      message(FATAL_ERROR "Cannot find PROTOBUF_ROOT/Debug")
    endif (EXISTS $ENV{PROTOBUF_ROOT}/Debug)

  else (CMAKE_BUILD_TYPE MATCHES Release)

    if (EXISTS $ENV{PROTOBUF_ROOT}/Release)
      message(STATUS "Using Protobuf RELEASE at " $ENV{PROTOBUF_ROOT}/Release)
      include_directories($ENV{PROTOBUF_ROOT}/Release/include)
      link_directories($ENV{PROTOBUF_ROOT}/Release/lib)
      set(Protobuf_LIBRARIES libprotobuf.lib)
    else (EXISTS $ENV{PROTOBUF_ROOT}/Release)
      message(FATAL_ERROR "Cannot find PROTOBUF_ROOT/Release")
    endif (EXISTS $ENV{PROTOBUF_ROOT}/Release)

  endif (CMAKE_BUILD_TYPE MATCHES Debug)

endif (UNIX)

# ==============================================================================
# -- GTest ---------------------------------------------------------------------
# ==============================================================================

# Download and unpack googletest at configure time.
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(GTest_LIBRARIES gtest_main)

# ==============================================================================
# -- Project config ------------------------------------------------------------
# ==============================================================================

set(CarlaServer_Deps_LIBRARIES
    ${Protobuf_LIBRARIES}
    ${GTest_LIBRARIES}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin_debug)
  set(CarlaServer_LIBRARIES carlaserverd ${CarlaServer_Deps_LIBRARIES})
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
  set(CarlaServer_LIBRARIES carlaserver ${CarlaServer_Deps_LIBRARIES})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

include_directories("${PROJECT_SOURCE_DIR}/source")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)
