project(traffic_manager CXX)
cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_C_COMPILER /usr/bin/clang-7)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-7)
## Debug options
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -O3 -g -fsanitize=address" CACHE STRING "" FORCE)

## Release options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -O3 -DNDEBUG" CACHE STRING "" FORCE)

# Scanning for python wrapper dependencies
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

set(CARLA_LOCATION $ENV{CARLA_LOCATION})

include_directories(${CARLA_LOCATION}/TrafficManager/source/pipeline/)
include_directories(${CARLA_LOCATION}/LibCarla/source/)
include_directories(${CARLA_LOCATION}/Build/boost-1.69.0-c7-install/include)
include_directories(${CARLA_LOCATION}/Build/recast-cdce4e-c7-install/include)
include_directories(${CARLA_LOCATION}/Build/rpclib-v2.2.1_c2-c7-libstdcxx-install/include)
# Including python wrapper dependencies
include_directories(${PYTHON_INCLUDE_DIRS})

link_directories(${CARLA_LOCATION}/Build/boost-1.69.0-c7-install/lib)
link_directories(${CARLA_LOCATION}/Build/recast-cdce4e-c7-install/lib)
link_directories(${CARLA_LOCATION}/Build/rpclib-v2.2.1_c2-c7-libstdcxx-install/lib)
link_directories(${CARLA_LOCATION}/PythonAPI/carla/dependencies/lib)

file(GLOB SOURCES ${CARLA_LOCATION}/TrafficManager/source/pipeline/*.cpp)

## Release executable
# file(GLOB SOURCES_EXE ${CARLA_LOCATION}/TrafficManager/source/pipeline/executable/PipelineExecutable.cpp)

## Debug executable
# file(GLOB SOURCES_EXE ${CARLA_LOCATION}/TrafficManager/source/test/Test.cpp)

add_library(pipeline ${SOURCES})

# Executable target
# add_executable(traffic_manager ${SOURCES_EXE})

# Python wrapper target
add_library(traffic_manager SHARED ${CARLA_LOCATION}/TrafficManager/source/pipeline/python/Wrapper.cpp)

# Linking executable target
# target_link_libraries(traffic_manager pipeline carla_client rpc pthread boost_system
#                       Detour DetourCrowd DetourTileCache Recast)

# Linking python wrapper
target_link_libraries(traffic_manager ${PYTHON_LIBRARIES}
                      pipeline carla_client rpc pthread boost_system boost_python27
                      Detour DetourCrowd DetourTileCache Recast)

# Setting target properties for python wrapper
set_target_properties(traffic_manager PROPERTIES SUFFIX .so)
set_target_properties(traffic_manager PROPERTIES PREFIX "")
