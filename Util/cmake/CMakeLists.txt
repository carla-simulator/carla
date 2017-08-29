cmake_minimum_required(VERSION 3.5.0)
project(CARLA)

set(CARLA_UTIL_PATH "${PROJECT_SOURCE_DIR}/..")

set(CARLA_LIBCXX_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/llvm-install")
set(CARLA_BOOST_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/boost-install")
set(CARLA_PROTOBUF_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/protobuf-install")
set(CARLA_GOOGLETEST_INSTALL_PATH "${CARLA_UTIL_PATH}/Build/googletest-install")

# Suppress windows warning, http://stackoverflow.com/a/40217291
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

add_subdirectory("CarlaServer")
