cmake_minimum_required(VERSION 3.27)
project(test_libcarla CXX)

find_package(carla REQUIRED CONFIG)

add_executable(test_libcarla src/test_libcarla.cpp)
target_link_libraries(test_libcarla carla::carla-server carla::carla-client)
target_compile_features(test_libcarla PRIVATE cxx_std_20)
