cmake_minimum_required (VERSION 3.0.2)
project (CarlaServer)

if (UNIX)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  find_package(Threads)
endif (UNIX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

# Boost configuration.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED system date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

# Protobuf.
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
if (UNIX)
  set(Protobuf_LIBRARIES ${Protobuf_LIBRARIES} protobuf)
endif (UNIX)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}/source")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)

install(FILES ${PROJECT_SOURCE_DIR}/source/carla/CarlaServer.h DESTINATION ${PROJECT_SOURCE_DIR}/include/carla)
