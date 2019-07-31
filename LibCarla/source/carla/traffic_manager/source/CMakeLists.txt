
project(traffic_manager CXX)
cmake_minimum_required(VERSION 3.2.2)

set(CMAKE_C_COMPILER /usr/bin/clang-7)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-7)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -O3 -DNDEBUG -g -fsanitize=address" CACHE STRING "" FORCE)

set(LIBCARLA_LOCATION $ENV{LIBCARLA_LOCATION})

include_directories(include/)
include_directories(${LIBCARLA_LOCATION}/include)
include_directories(${LIBCARLA_LOCATION}/include/system/)

link_directories(${LIBCARLA_LOCATION}/lib/)

file(GLOB SOURCES src/*.cpp)
file(GLOB SOURCES_EXE test/*.cpp)

add_library(pipeline ${SOURCES})
add_executable(traffic_manager ${SOURCES_EXE})

target_link_libraries(traffic_manager pipeline carla_client rpc pthread boost_system DetourCrowd Detour)
