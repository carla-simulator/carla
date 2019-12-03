project(traffic_manager CXX)
cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_C_COMPILER /usr/bin/clang-7)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-7)
## Debug options
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -O3 -g -fsanitize=address" CACHE STRING "" FORCE)

## Release options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -DNDEBUG" CACHE STRING "" FORCE)

set(LIBCARLA_LOCATION $ENV{LIBCARLA_LOCATION})
set(CARLA_LOCATION $ENV{CARLA_LOCATION})

include_directories(source/pipeline/)
include_directories(${LIBCARLA_LOCATION}/source/)
include_directories(${CARLA_LOCATION}/Build/boost-1.69.0-c7-install/include)
include_directories(${CARLA_LOCATION}/Build/recast-e59fc7-c7-install/include)
include_directories(${CARLA_LOCATION}/Build/rpclib-v2.2.1_c2-c7-libstdcxx-install/include)

link_directories(${LIBCARLA_LOCATION}/lib/)
link_directories(${CARLA_LOCATION}/Build/boost-1.69.0-c7-install/lib)
link_directories(${CARLA_LOCATION}/Build/recast-e59fc7-c7-install/lib)
link_directories(${CARLA_LOCATION}/Build/rpclib-v2.2.1_c2-c7-libstdcxx-install/lib)
link_directories(${CARLA_LOCATION}/PythonAPI/carla/dependencies/lib)

file(GLOB SOURCES source/pipeline/*.cpp)
## Release executable
file(GLOB SOURCES_EXE source/pipeline/executable/PipelineExecutable.cpp)
## Debug executable
# file(GLOB SOURCES_EXE source/test/Test.cpp)

add_library(pipeline ${SOURCES})
add_executable(traffic_manager ${SOURCES_EXE})

target_link_libraries(traffic_manager pipeline carla_client rpc pthread boost_system Detour DetourCrowd DetourTileCache Recast)
