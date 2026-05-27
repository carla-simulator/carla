cmake_minimum_required (VERSION 3.20)

project (carla-cpp-standalone CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate an installed CARLA. Pass -DCMAKE_PREFIX_PATH=/path/to/carla-install
# (the directory you used as cmake --install --prefix) on the configure line.
find_package (Carla CONFIG REQUIRED)

add_executable (my-carla-app main.cpp)
target_link_libraries (my-carla-app PRIVATE Carla::carla-client)
