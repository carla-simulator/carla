cmake_minimum_required (VERSION 2.6)
project (CarlaServer)

SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED 14)

# Boost configuration
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED system date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_definitions(-DBOOST_NO_EXCEPTIONS)

include_directories("${PROJECT_SOURCE_DIR}/source")
include_directories("${PROJECT_SOURCE_DIR}/../Common")
include_directories("${PROJECT_SOURCE_DIR}/source/carla/Protocol/carlaProtocol/include")

add_subdirectory(source/carla/server)
add_subdirectory(source/test)

