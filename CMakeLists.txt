cmake_minimum_required(VERSION 3.5.1)
project(CARLA)

include("Build/CMakeLists.txt.in")

add_subdirectory("LibCarla/cmake")
