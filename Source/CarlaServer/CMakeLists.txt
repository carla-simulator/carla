cmake_minimum_required (VERSION 3.0.2)
project (CarlaServer)

if (UNIX)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  find_package(Threads)
endif (UNIX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

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
# -- TurboJPEG -----------------------------------------------------------------
# ==============================================================================

if (UNIX)

  if (EXISTS $ENV{TURBOJPEG_LIB_PATH})
    set(TurboJPEG_LIBRARIES ${TurboJPEG_LIBRARIES} $ENV{TURBOJPEG_LIB_PATH}/libturbojpeg.a)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_TURBOJPEG")
  else (EXISTS $ENV{TURBOJPEG_LIB_PATH})
    message(FATAL_ERROR "TURBOJPEG_LIB_PATH is not defined, or points to a non-existant directory, please set this environment variable.")
  endif (EXISTS $ENV{TURBOJPEG_LIB_PATH})

elseif (WIN32)

  message(WARNING "TurboJPEG not available for Windows yet")

endif (UNIX)

# ==============================================================================
# -- Project config ------------------------------------------------------------
# ==============================================================================

set(CarlaServer_Deps_LIBRARIES
    ${TurboJPEG_LIBRARIES}
    ${Protobuf_LIBRARIES}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin_debug)
  set(CarlaServer_LIBRARIES carlaserverd ${CarlaServer_Deps_LIBRARIES})
elseif (CMAKE_BUILD_TYPE MATCHES Release)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
  set(CarlaServer_LIBRARIES carlaserver ${CarlaServer_Deps_LIBRARIES})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

include_directories("${PROJECT_SOURCE_DIR}/source")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)

install(FILES ${PROJECT_SOURCE_DIR}/source/carla/CarlaServer.h DESTINATION ${PROJECT_SOURCE_DIR}/include/carla)
