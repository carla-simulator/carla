cmake_minimum_required (VERSION 3.28.0)

project(carla-ros2-native-project)

include (${CARLA_WORKSPACE_PATH}/CMake/CarlaOptions.cmake)
include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)
set (PROJECT_CMAKE_FLAGS -DCMAKE_CXX_COMPILER=${GXX_COMPILER} -DCMAKE_C_COMPILER=${GCC_COMPILER} -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-D_GLIBCXX_USE_CXX11_ABI=0 -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH})

ExternalProject_add (
  foonathan_memory
  URL https://github.com/eProsima/foonathan_memory_vendor/archive/refs/heads/master.zip
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS} -DBUILD_SHARED_LIBS=ON -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
)

ExternalProject_add (
  fastcdr
  URL https://github.com/eProsima/Fast-CDR/archive/refs/heads/1.1.x.zip
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS}
)

# Note: We are using GIT_REPOSITORY instead URL for fastdds because fastdss contains submodules
#       and we need the .git folders to download the submodules
ExternalProject_add (
  fastdds
  GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
  GIT_TAG 2.11.2
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS} -DTHIRDPARTY_Asio=FORCE -DTHIRDPARTY_TinyXML2=FORCE
  DEPENDS foonathan_memory fastcdr
)

ExternalProject_Add(
  carla-ros2-native-lib
  DEPENDS fastdds
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/LibCarlaRos2Native
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS} -DBOOST_INCLUDE_PATH=${BOOST_INCLUDE_PATH}
)

set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux)
add_custom_command(
    TARGET carla-ros2-native-lib
    POST_BUILD
    COMMAND cmake -E make_directory ${CARLA_PLUGIN_BINARY_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy
      ${PROJECT_INSTALL_PATH}/lib/*.so*
      ${CARLA_PLUGIN_BINARY_PATH}
)

add_custom_target (
  carla-ros2-native
  DEPENDS carla-ros2-native-lib
)