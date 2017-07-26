cmake_minimum_required (VERSION 3.0.2)
project (CarlaServer)

# ==============================================================================
# -- Compiler config -----------------------------------------------------------
# ==============================================================================

if (UNIX)

  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -Werror -Wall -Wextra")
  find_package(Threads)

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
# -- LibPNG --------------------------------------------------------------------
# ==============================================================================

if (CMAKE_BUILD_TYPE MATCHES "(Debug|Release)PNG")

  if (UNIX)

    if (EXISTS $ENV{UE4_ROOT})

      # Use Unreal Engine's libpng and libz.

      set(LIBPNG_ROOT $ENV{UE4_ROOT}/Engine/Source/ThirdParty/libPNG/libPNG-1.5.2)
      set(LIBZ_ROOT $ENV{UE4_ROOT}/Engine/Source/ThirdParty/zlib/v1.2.8)

      if (NOT EXISTS ${LIBPNG_ROOT})
        message(FATAL_ERROR "Unable to find Unreal Engine's libpng")
      elseif (NOT EXISTS ${LIBZ_ROOT})
        message(FATAL_ERROR "Unable to find Unreal Engine's libz")
      endif (NOT EXISTS ${LIBPNG_ROOT})
      message(STATUS "Using libPNG-1.5.2 within Unreal Engine's installation folder")

      set(PNG_LIBRARIES
          ${LIBPNG_ROOT}/lib/Linux/x86_64-unknown-linux-gnu/libpng.a
          ${LIBZ_ROOT}/lib/Linux/x86_64-unknown-linux-gnu/libz_fPIC.a)

      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_WITH_PNG_COMPRESSION")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_INCLUDE=\\\"${LIBPNG_ROOT}/png.h\\\"")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_MAJOR=1")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_MINOR=5")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_RELEASE=2")

    else (EXISTS $ENV{UE4_ROOT})

      # Use system's libpng and libz.

      message(WARNING
          "Using system's libpng, this won't work with Unreal Engine. "
          "Please define the environment variable UE4_ROOT pointing to "
          "Unreal Engine's installation folder so we can link against "
          "the right version of libpng")

      include(FindPNG)
      find_package(PNG REQUIRED)

      # Retrieve version number for extra check.
      string(REPLACE "." ";" PNG_VERSION_LIST ${PNG_VERSION_STRING})
      list(GET PNG_VERSION_LIST 0 PNG_VERSION_MAJOR)
      list(GET PNG_VERSION_LIST 1 PNG_VERSION_MINOR)
      list(GET PNG_VERSION_LIST 2 PNG_VERSION_RELEASE)

      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_WITH_PNG_COMPRESSION")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_INCLUDE=\\\"png.h\\\"")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_MAJOR=${PNG_VERSION_MAJOR}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_MINOR=${PNG_VERSION_MINOR}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCARLA_LIBPNG_VERSION_RELEASE=${PNG_VERSION_RELEASE}")

    endif (EXISTS $ENV{UE4_ROOT})

  elseif (WIN32)

    message(WARNING "PNG compression not available for Windows yet")

  endif (UNIX)

endif (CMAKE_BUILD_TYPE MATCHES "(Debug|Release)PNG")

# ==============================================================================
# -- Project config ------------------------------------------------------------
# ==============================================================================

set(CarlaServer_Deps_LIBRARIES
    ${PNG_LIBRARIES}
    ${Protobuf_LIBRARIES}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin_debug)
  set(CarlaServer_LIBRARIES carlaserverd ${CarlaServer_Deps_LIBRARIES})
elseif (CMAKE_BUILD_TYPE STREQUAL "DebugPNG")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin_debug)
  set(CarlaServer_LIBRARIES carlaserverpngd ${CarlaServer_Deps_LIBRARIES})
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
  set(CarlaServer_LIBRARIES carlaserver ${CarlaServer_Deps_LIBRARIES})
elseif (CMAKE_BUILD_TYPE STREQUAL "ReleasePNG")
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
  set(CarlaServer_LIBRARIES carlaserverpng ${CarlaServer_Deps_LIBRARIES})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

include_directories("${PROJECT_SOURCE_DIR}/source")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)

install(FILES ${PROJECT_SOURCE_DIR}/source/carla/CarlaServer.h DESTINATION ${PROJECT_SOURCE_DIR}/include/carla)
