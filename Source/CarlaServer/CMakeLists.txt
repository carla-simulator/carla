cmake_minimum_required (VERSION 2.6)
project (CarlaServer)

# Boost configuration
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED system date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}/source")
include_directories("${PROJECT_SOURCE_DIR}/../Common")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)
