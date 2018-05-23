cmake_minimum_required (VERSION 3.5.0)
project (CarlaServer)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CarlaServer_Lib_Target carlaserverd)
  set(CarlaServer_Test_Target test_carlaserverd)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CarlaServer_Lib_Target carlaserver)
  set(CarlaServer_Test_Target test_carlaserver)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# ==============================================================================
# -- Compiler and dependencies -------------------------------------------------
# ==============================================================================

if (UNIX)

  find_package(Threads)

  set(CMAKE_CXX_COMPILER /usr/bin/clang++-3.9)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wno-unused-parameter")

  set(CMAKE_POSITION_INDEPENDENT_CODE ON)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D_DEBUG -O0")
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
  endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

  # Setup libc++.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  include_directories("${CARLA_LIBCXX_INSTALL_PATH}/include/c++/v1")
  link_directories("${CARLA_LIBCXX_INSTALL_PATH}/lib")
  file(GLOB LibCXX_Shared_Libraries "${CARLA_LIBCXX_INSTALL_PATH}/lib/libc++*.so*")
  file(GLOB LibCXX_Static_Libraries "${CARLA_LIBCXX_INSTALL_PATH}/lib/libc++*.a")

  # Setup boost.
  include_directories("${CARLA_BOOST_INSTALL_PATH}/include")
  set(Boost_Static_Libraries "${CARLA_BOOST_INSTALL_PATH}/lib/libboost_system.a")

  # Setup protobuf.
  include_directories("${CARLA_PROTOBUF_INSTALL_PATH}/include")
  set(Protobuf_Static_Libraries "${CARLA_PROTOBUF_INSTALL_PATH}/lib/libprotobuf.a")

  # Setup googletest.
  include_directories("${CARLA_GOOGLETEST_INSTALL_PATH}/include")
  set(GTest_Static_Libraries "${CARLA_GOOGLETEST_INSTALL_PATH}/lib/libgtest.a")

  install(FILES
      ${LibCXX_Shared_Libraries}
    DESTINATION shared)

  install(FILES
      ${LibCXX_Static_Libraries}
      ${Boost_Static_Libraries}
      ${Protobuf_Static_Libraries}
    DESTINATION lib)

elseif (WIN32)

  # Setup boost.
  include_directories("${CARLA_BOOST_INSTALL_PATH}/boost-1_64")
  file(GLOB Boost_System_Static_Libraries "${CARLA_BOOST_INSTALL_PATH}/lib/libboost_system*.lib")
  file(GLOB Boost_DateTime_Static_Libraries "${CARLA_BOOST_INSTALL_PATH}/lib/libboost_date_time*.lib")
  set(Boost_Static_Libraries
      "${Boost_System_Static_Libraries}"
      "${Boost_DateTime_Static_Libraries}")

  # Setup protobuf.
  include_directories("${CARLA_PROTOBUF_INSTALL_PATH}/include")
  set(Protobuf_Static_Libraries "${CARLA_PROTOBUF_INSTALL_PATH}/lib/libprotobuf.lib")

  # Setup googletest.
  include_directories("${CARLA_GOOGLETEST_INSTALL_PATH}/include")
  set(GTest_Static_Libraries "${CARLA_GOOGLETEST_INSTALL_PATH}/lib/gtest.lib")

  install(FILES
      ${Boost_Static_Libraries}
      ${Protobuf_Static_Libraries}
    DESTINATION lib)

else (UNIX)
  message(FATAL_ERROR "Build configuration not yet available for this platform")
endif (UNIX)

# ==============================================================================
# -- Project config ------------------------------------------------------------
# ==============================================================================

set(CarlaServer_Path "${CARLA_UTIL_PATH}/CarlaServer")

include_directories("${CarlaServer_Path}/source")
include_directories("${CarlaServer_Path}/include")

# libcarlaserver

file(GLOB carlaserver_SRC
    "${CarlaServer_Path}/include/carla/carla_server.h"
    "${CarlaServer_Path}/source/carla/*.h"
    "${CarlaServer_Path}/source/carla/*.cpp"
    "${CarlaServer_Path}/source/carla/server/*.h"
    "${CarlaServer_Path}/source/carla/server/*.cpp"
    "${CarlaServer_Path}/source/carla/server/*.pb.h"
    "${CarlaServer_Path}/source/carla/server/*.pb.cc")

add_library(${CarlaServer_Lib_Target} STATIC ${carlaserver_SRC})
install(DIRECTORY "${CarlaServer_Path}/include/carla" DESTINATION include)
install(TARGETS ${CarlaServer_Lib_Target} DESTINATION lib)

# unit tests

file(GLOB test_carlaserver_SRC
    "${CarlaServer_Path}/source/test/*.h"
    "${CarlaServer_Path}/source/test/*.cpp")

set(CarlaServer_Static_LIBRARIES
    ${CarlaServer_Lib_Target}
    ${GTest_Static_Libraries}
    ${Protobuf_Static_Libraries}
    ${Boost_Static_Libraries}
    ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
  add_executable(${CarlaServer_Test_Target} ${test_carlaserver_SRC})
  target_link_libraries(${CarlaServer_Test_Target} ${CarlaServer_Static_LIBRARIES})
  install(TARGETS ${CarlaServer_Test_Target} DESTINATION bin)
endif (UNIX)
