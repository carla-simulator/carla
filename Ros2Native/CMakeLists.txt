cmake_minimum_required (
  VERSION
  ${CARLA_CMAKE_MINIMUM_REQUIRED_VERSION}
)

project (carla-ros2-native-project)

include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)

if (WIN32) 
  ExternalProject_add (
    foonathan_memory
    URL https://github.com/eProsima/foonathan_memory_vendor/archive/refs/heads/${CARLA_FOONATHAN_MEMORY_VENDOR_TAG}.zip
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DBUILD_SHARED_LIBS=OFF
      -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
  )

  ExternalProject_add (
    fastcdr
    GIT_REPOSITORY https://github.com/eProsima/Fast-CDR.git
    GIT_TAG v1.1.0
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DBUILD_SHARED_LIBS=OFF
  )

  # Note: We are using GIT_REPOSITORY instead URL for fastdds because fastdss contains submodules
  #       and we need the .git folders to download the submodules
  ExternalProject_add (
    fastdds
    GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
    GIT_TAG ${CARLA_FASTDDS_TAG}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DBUILD_SHARED_LIBS=OFF
      -DBUILD_TESTING=OFF
      -DCOMPILE_EXAMPLES=OFF
      -DCOMPILE_TOOLS=OFF
      -DTHIRDPARTY_Asio=FORCE
      -DTHIRDPARTY_fastcdr=FORCE
      -DTHIRDPARTY_TinyXML2=FORCE
      -DSQLITE3_SUPPORT=OFF
      -DSECURITY=OFF
    DEPENDS foonathan_memory
  )

  ExternalProject_Add (
    carla-ros2-native-lib
    DEPENDS fastdds
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/LibCarlaRos2Native
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_CXX_STANDARD=17
  )
else() 
  ExternalProject_add (
    foonathan_memory
    URL https://github.com/eProsima/foonathan_memory_vendor/archive/refs/heads/${CARLA_FOONATHAN_MEMORY_VENDOR_TAG}.zip
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DBUILD_SHARED_LIBS=ON
      -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
  )

  # Note: We are using GIT_REPOSITORY instead URL for fastdds because fastdss contains submodules
  #       and we need the .git folders to download the submodules
  ExternalProject_add (
    fastdds
    GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
    GIT_TAG ${CARLA_FASTDDS_TAG}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DBUILD_SHARED_LIBS=OFF
      -DBUILD_TESTING=ON
      -DCOMPILE_EXAMPLES=OFF
      -DCOMPILE_TOOLS=OFF
      -DTHIRDPARTY_Asio=FORCE
      -DTHIRDPARTY_fastcdr=FORCE
      -DTHIRDPARTY_TinyXML2=FORCE
      -DSQLITE3_SUPPORT=OFF
      -DOPENSSL_FOUND:BOOL=ON
      -DOPENSSL_INCLUDE_DIR:FILEPATH=${UE_OPENSSL_INCLUDE}
      -DOPENSSL_SSL_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libssl.a
      -DOPENSSL_CRYPTO_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libcrypto.a    DEPENDS foonathan_memory
  )

  ExternalProject_Add (
    carla-ros2-native-lib
    DEPENDS fastdds
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/LibCarlaRos2Native
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  )
endif()

carla_add_custom_target (
  carla-ros2-native
  "Build the ROS2-Native CARLA subproject."
  DEPENDS carla-ros2-native-lib
)

set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries)
make_directory (${CARLA_PLUGIN_BINARY_PATH})

if (WIN32)
  add_custom_command (
    TARGET carla-ros2-native-lib
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${PROJECT_INSTALL_PATH}/lib/carla-ros2-native.lib
      ${PROJECT_INSTALL_PATH}/lib/libfastcdr-1.1.lib
      ${PROJECT_INSTALL_PATH}/lib/libfastrtps-2.11.lib
      ${PROJECT_INSTALL_PATH}/lib/foonathan_memory-0.7.3.lib
      ${CARLA_PLUGIN_BINARY_PATH}
  )
else()
  add_custom_command (
    TARGET carla-ros2-native-lib
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy
      ${PROJECT_INSTALL_PATH}/lib/*.so*
      ${CARLA_PLUGIN_BINARY_PATH}
  )
endif()
