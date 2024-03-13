cmake_minimum_required (VERSION 3.28.0)

project(carla-ros2-native-project)

include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)
set (PROJECT_CMAKE_FLAGS -DCMAKE_CXX_COMPILER=/usr/bin/g++-7 -DCMAKE_C_COMPILER=/usr/bin/gcc-7 -DCMAKE_CXX_FLAGS_RELEASE=-D_GLIBCXX_USE_CXX11_ABI=0 -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH})

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

ExternalProject_add (
  asio
  URL https://github.com/chriskohlhoff/asio/archive/b84e6c16b2ea907dbad94206b7510d85aafc0b42.zip
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_property (asio SOURCE_DIR)
set (ASIO_SOURCE_DIR ${SOURCE_DIR})

ExternalProject_add (
  tinyxml2
  URL https://github.com/leethomason/tinyxml2/archive/8c8293ba8969a46947606a93ff0cb5a083aab47a.zip
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS}
)

ExternalProject_add (
  fastdds
  URL https://github.com/eProsima/Fast-DDS/archive/refs/heads/2.11.2.zip
  CMAKE_ARGS ${PROJECT_CMAKE_FLAGS} -DAsio_INCLUDE_DIR=${ASIO_SOURCE_DIR}/asio/include
  DEPENDS foonathan_memory fastcdr asio tinyxml2
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