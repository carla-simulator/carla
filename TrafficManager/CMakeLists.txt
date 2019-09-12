
project(traffic_manager CXX)
cmake_minimum_required(VERSION 3.2.2)

set(CMAKE_C_COMPILER /usr/bin/clang-7)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-7)

## Debug options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -O3 -g -fsanitize=address" CACHE STRING "" FORCE)

## Release options
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -DNDEBUG" CACHE STRING "" FORCE)

set(LIBCARLA_LOCATION $ENV{LIBCARLA_LOCATION})

include_directories(source/pipeline/)
include_directories(${LIBCARLA_LOCATION}/include)
include_directories(${LIBCARLA_LOCATION}/include/system/)

link_directories(${LIBCARLA_LOCATION}/lib/)

file(GLOB SOURCES source/pipeline/*.cpp)
file(GLOB SOURCES_EXE source/pipeline/executable/PipelineExecutable.cpp)
# file(GLOB SOURCES_EXE source/test/Test.cpp)

add_library(pipeline ${SOURCES})
add_executable(traffic_manager ${SOURCES_EXE})

target_link_libraries(traffic_manager pipeline carla_client rpc pthread boost_system DetourCrowd Detour)
