cmake_minimum_required(VERSION 3.10)
project(CARLA)

# If no CMAKE_BUILD_TYPE is specified and the generator is single-configuration,
# default to Client so downstream CMake files that expect a specific build
# type don't fatal-error.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "No CMAKE_BUILD_TYPE specified. Defaulting to 'Client'.")
	set(CMAKE_BUILD_TYPE "Client" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Client" "Server" "Pytorch" "ros2")
endif()

include("Build/CMakeLists.txt.in")

add_subdirectory("LibCarla/cmake")
