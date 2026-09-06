cmake_minimum_required (VERSION 3.20)

project (carla-agent-demo CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Standalone build path: locate an installed CARLA via
#   cmake --install Build --prefix /opt/carla
# then point this project at it via -DCMAKE_PREFIX_PATH=/opt/carla.
find_package (Carla CONFIG REQUIRED)

add_executable (carla-agent-demo main.cpp)
target_link_libraries (carla-agent-demo PRIVATE Carla::carla-client)
