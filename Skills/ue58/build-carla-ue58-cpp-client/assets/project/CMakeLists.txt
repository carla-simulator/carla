cmake_minimum_required (VERSION 3.20)

project (my-carla-client CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# -DCMAKE_PREFIX_PATH=<install prefix> finds Carla; -DCARLA_BUILD_DIR=<carla>/Build/<preset> supplies its dependencies.
find_package (Carla CONFIG REQUIRED)
include (${CMAKE_CURRENT_LIST_DIR}/CarlaDeps.cmake)

add_executable (my-carla-client main.cpp)
target_link_libraries (my-carla-client PRIVATE Carla::carla-client)
