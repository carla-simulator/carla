cmake_minimum_required (
  VERSION
  ${CARLA_CMAKE_MINIMUM_REQUIRED_VERSION}
)

project (carla-ros2-native-project)

include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)

set (ROS2_COMMON_CMAKE_ARGS)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
  list (APPEND ROS2_COMMON_CMAKE_ARGS -DCMAKE_POLICY_VERSION_MINIMUM=3.5)
endif ()
if (APPLE)
  list (APPEND ROS2_COMMON_CMAKE_ARGS
    -DCMAKE_MACOSX_RPATH=ON
    -DCMAKE_INSTALL_RPATH=@loader_path
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
  )
elseif (LINUX)
  list (APPEND ROS2_COMMON_CMAKE_ARGS
    -DCMAKE_INSTALL_RPATH=\$ORIGIN
    -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
  )
endif ()

set (FASTDDS_OPENSSL_ARGS)
if (UE_OPENSSL_INCLUDE AND UE_OPENSSL_LIBS AND EXISTS "${UE_OPENSSL_LIBS}/libssl.a")
  list (APPEND FASTDDS_OPENSSL_ARGS
    -DOPENSSL_FOUND:BOOL=ON
    -DOPENSSL_INCLUDE_DIR:FILEPATH=${UE_OPENSSL_INCLUDE}
    -DOPENSSL_SSL_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libssl.a
    -DOPENSSL_CRYPTO_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libcrypto.a
  )
endif ()

ExternalProject_add (
  foonathan_memory
  URL https://github.com/eProsima/foonathan_memory_vendor/archive/refs/heads/${CARLA_FOONATHAN_MEMORY_VENDOR_TAG}.zip
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DBUILD_SHARED_LIBS=ON
    -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
    ${ROS2_COMMON_CMAKE_ARGS}
  BUILD_COMMAND
    ${CMAKE_COMMAND} -E env CMAKE_POLICY_VERSION_MINIMUM=3.5 ${CMAKE_COMMAND} --build <BINARY_DIR>
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
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    -DCOMPILE_EXAMPLES=OFF
    -DCOMPILE_TOOLS=OFF
    -DTHIRDPARTY_Asio=FORCE
    -DTHIRDPARTY_fastcdr=FORCE
    -DTHIRDPARTY_TinyXML2=FORCE
    -DSQLITE3_SUPPORT=OFF
    ${FASTDDS_OPENSSL_ARGS}
    ${ROS2_COMMON_CMAKE_ARGS}
  INSTALL_BYPRODUCTS
    ${PROJECT_INSTALL_PATH}/lib/libfastcdr${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${PROJECT_INSTALL_PATH}/lib/libfastrtps${CMAKE_SHARED_LIBRARY_SUFFIX}
  DEPENDS foonathan_memory
)

# Note: We are using GIT_REPOSITORY instead of URL for cyclonedds because it
#       contains submodules and we need the .git folders to fetch them.
#       Built as a shared library (libddsc.so / libddsc.dylib) so it is staged
#       into the plugin Binaries alongside the FastDDS shared libraries.
ExternalProject_add (
  cyclonedds
  GIT_REPOSITORY https://github.com/eclipse-cyclonedds/cyclonedds.git
  GIT_TAG ${CARLA_CYCLONEDDS_TAG}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    -DBUILD_EXAMPLES=OFF
    -DBUILD_IDLC=OFF
    -DBUILD_DDSPERF=OFF
    -DENABLE_SSL=OFF
    -DENABLE_SECURITY=OFF
    -DENABLE_SHM=OFF
    ${ROS2_COMMON_CMAKE_ARGS}
  INSTALL_BYPRODUCTS
    ${PROJECT_INSTALL_PATH}/lib/libddsc${CMAKE_SHARED_LIBRARY_SUFFIX}
  DEPENDS foonathan_memory
)

if (APPLE)
  if ("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "^(arm64|aarch64)$")
    set (CARLA_ZENOH_C_TARGET_TRIPLE "aarch64-apple-darwin")
  else ()
    set (CARLA_ZENOH_C_TARGET_TRIPLE "x86_64-apple-darwin")
  endif ()
else ()
  if ("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "^(arm64|aarch64)$")
    set (CARLA_ZENOH_C_TARGET_TRIPLE "aarch64-unknown-linux-gnu")
  else ()
    set (CARLA_ZENOH_C_TARGET_TRIPLE "x86_64-unknown-linux-gnu")
  endif ()
endif ()

# Zenoh: prebuilt zenoh-c standalone release. The archive ships a static PIC
# libzenohc.a (Rust) that is linked directly into carla-ros2-native, so
# there is no runtime shared lib to stage. Download-only: no configure/build step,
# just unpack and copy the headers and the archive into the shared install.
ExternalProject_add (
  zenoh-c
  URL https://github.com/eclipse-zenoh/zenoh-c/releases/download/${CARLA_ZENOH_C_TAG}/zenoh-c-${CARLA_ZENOH_C_VERSION}-${CARLA_ZENOH_C_TARGET_TRIPLE}-standalone.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PROJECT_INSTALL_PATH}/include
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libzenohc.a ${PROJECT_INSTALL_PATH}/lib/libzenohc.a
  INSTALL_BYPRODUCTS
    ${PROJECT_INSTALL_PATH}/lib/libzenohc.a
)

ExternalProject_Add (
  carla-ros2-native-lib
  DEPENDS fastdds cyclonedds zenoh-c
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/LibCarlaRos2Native
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_STANDARD=20
    -DCMAKE_CXX_STANDARD_REQUIRED=ON
    ${ROS2_COMMON_CMAKE_ARGS}
)

if (APPLE)
  set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Mac)
  set (CARLA_ROS2_SHARED_LIB_GLOB "${PROJECT_INSTALL_PATH}/lib/*.dylib*")
else ()
  set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux)
  set (CARLA_ROS2_SHARED_LIB_GLOB "${PROJECT_INSTALL_PATH}/lib/*.so*")
endif ()

make_directory (${CARLA_PLUGIN_BINARY_PATH})

add_custom_command (
  TARGET carla-ros2-native-lib
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CARLA_ROS2_SHARED_LIB_GLOB}
    ${CARLA_PLUGIN_BINARY_PATH}
)

# Stage the Zenoh session config next to libcarla-ros2-native.so so the
# middleware resolves it relative to the loaded library (via dladdr) in both
# editor and packaged runs.
add_custom_command (
  TARGET carla-ros2-native-lib
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/LibCarla/source/carla/ros2/middleware/zenoh/config/zenoh_session_config.json5
    ${CARLA_PLUGIN_BINARY_PATH}
)

carla_add_custom_target (
  carla-ros2-native
  "Build the ROS2-Native CARLA subproject."
  DEPENDS carla-ros2-native-lib
)
