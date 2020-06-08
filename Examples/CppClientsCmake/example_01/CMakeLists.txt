# standard version  3.5.2 for Ubuntu 16
# statndard version 3.10.2 for Ubuntu 18
cmake_minimum_required (VERSION 3.5)

project (carla_client_sync)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../LibCarla/source)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../Build/LibCarla/cmake/client)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../Build/libcarla-client-build.debug/LibCarla/cmake/client/)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../Build/libcarla-client-build.release/LibCarla/cmake/client/)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})

if(CMAKE_BUILD_TYPE MATCHES Debug)
set(CARLA_LIB carla_client_debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
set(CARLA_LIB carla_client)
endif()

message("CARLA_LIB: ${CARLA_LIB}")
target_link_libraries(${PROJECT_NAME} ${CARLA_LIB} png jpeg tiff rt pthread rpc Detour DetourCrowd boost_filesystem)