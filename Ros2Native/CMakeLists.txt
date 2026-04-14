cmake_minimum_required (
  VERSION
  ${CARLA_CMAKE_MINIMUM_REQUIRED_VERSION}
)

project (carla-ros2-native-project)

include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)
set (SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
set (ROS2_TAG jazzy)

set (COMMON_CMAKE_ARGS
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
  -DCMAKE_PREFIX_PATH=${PROJECT_INSTALL_PATH}
  -DBUILD_TESTING=OFF
  "-DCMAKE_C_FLAGS=-D_GNU_SOURCE"
  "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
  -Drosidl_generator_py_DIR=NOTFOUND
)

# ====================
# mono-repo downloads
# ====================

macro (download_ros2_repo name url)
  ExternalProject_Add (_dl_${name}
    GIT_REPOSITORY ${url}
    GIT_TAG ${ROS2_TAG}
    GIT_SHALLOW TRUE
    SOURCE_DIR ${SRC_DIR}/${name}
    CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_COMMAND ""
  )
endmacro ()

download_ros2_repo (rosidl                    https://github.com/ros2/rosidl.git)
download_ros2_repo (rcl                       https://github.com/ros2/rcl.git)
download_ros2_repo (rcl_interfaces            https://github.com/ros2/rcl_interfaces.git)
download_ros2_repo (rmw                       https://github.com/ros2/rmw.git)
download_ros2_repo (rcl_logging               https://github.com/ros2/rcl_logging.git)
download_ros2_repo (ros2_tracing              https://github.com/ros2/ros2_tracing.git)
download_ros2_repo (ament_index               https://github.com/ament/ament_index.git)
download_ros2_repo (rclcpp                    https://github.com/ros2/rclcpp.git)
download_ros2_repo (rosidl_typesupport        https://github.com/ros2/rosidl_typesupport.git)
download_ros2_repo (rosidl_typesupport_fastrtps https://github.com/ros2/rosidl_typesupport_fastrtps.git)
download_ros2_repo (rmw_implementation        https://github.com/ros2/rmw_implementation.git)
download_ros2_repo (common_interfaces         https://github.com/ros2/common_interfaces.git)
download_ros2_repo (geometry2                 https://github.com/ros2/geometry2.git)
download_ros2_repo (rmw_zenoh                 https://github.com/ros2/rmw_zenoh.git)
download_ros2_repo (rmw_fastrtps              https://github.com/ros2/rmw_fastrtps.git)
download_ros2_repo (rmw_cyclonedds            https://github.com/ros2/rmw_cyclonedds.git)
download_ros2_repo (rmw_dds_common            https://github.com/ros2/rmw_dds_common.git)

# ====================
# base libraries
# ====================

ExternalProject_Add(rcutils
  GIT_REPOSITORY https://github.com/ros2/rcutils.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/rcutils
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
)

ExternalProject_Add(rcpputils
  GIT_REPOSITORY https://github.com/ros2/rcpputils.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/rcpputils
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS rcutils
)

ExternalProject_Add(rosidl_runtime_c
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl
  SOURCE_SUBDIR rosidl_runtime_c
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl rcutils
)

ExternalProject_Add(rosidl_runtime_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl
  SOURCE_SUBDIR rosidl_runtime_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl rosidl_runtime_c
)

ExternalProject_Add(rosidl_typesupport_interface
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl
  SOURCE_SUBDIR rosidl_typesupport_interface
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl rosidl_runtime_c
)

# ====================
# serialization + typesupport
# ====================

ExternalProject_Add(fastcdr
  GIT_REPOSITORY https://github.com/eProsima/Fast-CDR.git
  GIT_TAG v2.2.5
  SOURCE_DIR ${SRC_DIR}/fastcdr
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
)

ExternalProject_Add(rosidl_typesupport_fastrtps_c
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl_typesupport_fastrtps
  SOURCE_SUBDIR rosidl_typesupport_fastrtps_c
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl_typesupport_fastrtps fastcdr rcutils rosidl_runtime_c rosidl_typesupport_interface
)

ExternalProject_Add(rosidl_typesupport_fastrtps_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl_typesupport_fastrtps
  SOURCE_SUBDIR rosidl_typesupport_fastrtps_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl_typesupport_fastrtps fastcdr rcutils rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_fastrtps_c
)

ExternalProject_Add(rosidl_typesupport_c
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl_typesupport
  SOURCE_SUBDIR rosidl_typesupport_c
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl_typesupport rcutils rosidl_runtime_c rosidl_typesupport_interface rosidl_typesupport_fastrtps_c
)

ExternalProject_Add(rosidl_typesupport_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl_typesupport
  SOURCE_SUBDIR rosidl_typesupport_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl_typesupport rcutils rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_fastrtps_cpp
)

ExternalProject_Add(rosidl_typesupport_introspection_c
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl
  SOURCE_SUBDIR rosidl_typesupport_introspection_c
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl rcutils rosidl_runtime_c rosidl_typesupport_interface
)

ExternalProject_Add(rosidl_typesupport_introspection_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rosidl
  SOURCE_SUBDIR rosidl_typesupport_introspection_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rosidl rcutils rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_introspection_c
)

# ====================
# core middleware
# ====================

ExternalProject_Add(rmw
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw
  SOURCE_SUBDIR rmw
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw rcutils rosidl_runtime_c rosidl_typesupport_interface
)

ExternalProject_Add(tracetools
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/ros2_tracing
  SOURCE_SUBDIR tracetools
  CMAKE_ARGS ${COMMON_CMAKE_ARGS} -DTRACETOOLS_DISABLED=ON
  DEPENDS _dl_ros2_tracing rcutils
)

ExternalProject_Add(rcl_logging_interface
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_logging
  SOURCE_SUBDIR rcl_logging_interface
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_logging rcutils
)

ExternalProject_Add(libyaml
  GIT_REPOSITORY https://github.com/yaml/libyaml.git
  GIT_TAG 0.2.5
  SOURCE_DIR ${SRC_DIR}/libyaml
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DBUILD_TESTING=OFF
    -DBUILD_SHARED_LIBS=ON
)

ExternalProject_Add(rcl_yaml_param_parser
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl
  SOURCE_SUBDIR rcl_yaml_param_parser
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl rcutils libyaml
)

ExternalProject_Add(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
  SOURCE_DIR ${SRC_DIR}/spdlog
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DSPDLOG_BUILD_SHARED=ON
    -DSPDLOG_BUILD_EXAMPLE=OFF
    -DSPDLOG_BUILD_TESTS=OFF
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
)

ExternalProject_Add(rcl_logging_spdlog
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_logging
  SOURCE_SUBDIR rcl_logging_spdlog
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_logging rcutils rcl_logging_interface spdlog
)

ExternalProject_Add(ament_index_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/ament_index
  SOURCE_SUBDIR ament_index_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_ament_index
)

ExternalProject_Add(rosidl_dynamic_typesupport
  GIT_REPOSITORY https://github.com/ros2/rosidl_dynamic_typesupport.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/rosidl_dynamic_typesupport
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS rcutils
)

# ====================
# rcl_interfaces messages
# ====================

ExternalProject_Add(builtin_interfaces
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR builtin_interfaces
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(type_description_interfaces
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR type_description_interfaces
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(service_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR service_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(rcl_interfaces_pkg
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR rcl_interfaces
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces builtin_interfaces service_msgs
)

ExternalProject_Add(rosgraph_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR rosgraph_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces builtin_interfaces rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(statistics_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR statistics_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces builtin_interfaces rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(unique_identifier_msgs
  GIT_REPOSITORY https://github.com/ros2/unique_identifier_msgs.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/unique_identifier_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(action_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl_interfaces
  SOURCE_SUBDIR action_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl_interfaces builtin_interfaces service_msgs unique_identifier_msgs rosidl_runtime_c rosidl_runtime_cpp
)

# ====================
# rcl + rclcpp
# ====================

ExternalProject_Add(rcl
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rcl
  SOURCE_SUBDIR rcl
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rcl rcutils rmw rcl_interfaces_pkg rcl_logging_interface
          rcl_yaml_param_parser service_msgs type_description_interfaces
          tracetools rosidl_runtime_c
)

ExternalProject_Add(libstatistics_collector
  GIT_REPOSITORY https://github.com/ros-tooling/libstatistics_collector.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/libstatistics_collector
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS rcutils rcpputils statistics_msgs builtin_interfaces rosidl_runtime_cpp rcl
)

ExternalProject_Add(rclcpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rclcpp
  SOURCE_SUBDIR rclcpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rclcpp rcl rcutils rcpputils rmw builtin_interfaces rcl_interfaces_pkg
          rcl_logging_interface rcl_yaml_param_parser rosgraph_msgs
          rosidl_dynamic_typesupport rosidl_runtime_c rosidl_runtime_cpp
          rosidl_typesupport_cpp rosidl_typesupport_introspection_cpp
          statistics_msgs tracetools ament_index_cpp
          libstatistics_collector
)

# ====================
# ROS2 message packages
# ====================

ExternalProject_Add(std_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/common_interfaces
  SOURCE_SUBDIR std_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_common_interfaces builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(geometry_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/common_interfaces
  SOURCE_SUBDIR geometry_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_common_interfaces std_msgs builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(sensor_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/common_interfaces
  SOURCE_SUBDIR sensor_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_common_interfaces std_msgs geometry_msgs builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(nav_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/common_interfaces
  SOURCE_SUBDIR nav_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_common_interfaces std_msgs geometry_msgs builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_c rosidl_typesupport_cpp
)

ExternalProject_Add(tf2_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/geometry2
  SOURCE_SUBDIR tf2_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_geometry2 geometry_msgs std_msgs builtin_interfaces rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_c rosidl_typesupport_cpp
)

# ====================
# CARLA custom messages
# ====================

ExternalProject_Add(carla_msgs
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/carla_msgs
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS std_msgs geometry_msgs rosidl_runtime_c rosidl_runtime_cpp rosidl_typesupport_interface rosidl_typesupport_c rosidl_typesupport_cpp
)

# ====================
# rmw_zenoh backend
# ====================

ExternalProject_Add(zenoh_c
  GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-c.git
  GIT_TAG 73e7a0bdcf78b8263a02b7aee453ee6de8187589
  SOURCE_DIR ${SRC_DIR}/zenoh-c
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DZENOHC_BUILD_WITH_UNSTABLE_API=TRUE
    "-DZENOHC_CARGO_FLAGS=--features=shared-memory zenoh/transport_serial"
    -DBUILD_TESTING=OFF
)

ExternalProject_Add(zenoh_cpp
  GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-cpp.git
  GIT_TAG 47a80d345d74ebef16f3783423bf3bbd89cd6c30
  SOURCE_DIR ${SRC_DIR}/zenoh-cpp
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DZENOHCXX_ZENOHC=OFF
  DEPENDS zenoh_c
)

ExternalProject_Add(zenoh_cpp_vendor
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_zenoh
  SOURCE_SUBDIR zenoh_cpp_vendor
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    -DUSE_SYSTEM_ZENOH=ON
  DEPENDS _dl_rmw_zenoh zenoh_c zenoh_cpp
)

ExternalProject_Add(rmw_zenoh_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_zenoh
  SOURCE_SUBDIR rmw_zenoh_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_zenoh zenoh_cpp_vendor fastcdr rcutils rcpputils rmw
          ament_index_cpp tracetools rosidl_typesupport_fastrtps_c
          rosidl_typesupport_fastrtps_cpp
)

# ====================
# rmw common (shared by rmw backends)
# ====================

ExternalProject_Add(rmw_dds_common
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_dds_common
  SOURCE_SUBDIR rmw_dds_common
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_dds_common rcutils rcpputils rmw rosidl_runtime_c
          rosidl_typesupport_c rosidl_typesupport_cpp
)

# ====================
# rmw_fastrtps backend
# ====================

ExternalProject_Add(foonathan_memory
  GIT_REPOSITORY https://github.com/eProsima/foonathan_memory_vendor.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/foonathan_memory
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DBUILD_SHARED_LIBS=ON
    -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
)

ExternalProject_Add(fastdds
  GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
  GIT_TAG 2.14.x
  SOURCE_DIR ${SRC_DIR}/fastdds
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_PREFIX_PATH=${PROJECT_INSTALL_PATH}
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    -DCOMPILE_EXAMPLES=OFF
    -DCOMPILE_TOOLS=OFF
    -DTHIRDPARTY_Asio=FORCE
    -DTHIRDPARTY_TinyXML2=FORCE
    -DSQLITE3_SUPPORT=OFF
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
  DEPENDS foonathan_memory fastcdr
)

ExternalProject_Add(rmw_fastrtps_shared_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_fastrtps
  SOURCE_SUBDIR rmw_fastrtps_shared_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_fastrtps fastcdr fastdds rcutils rcpputils rmw rmw_dds_common tracetools
)

ExternalProject_Add(rosidl_dynamic_typesupport_fastrtps
  GIT_REPOSITORY https://github.com/ros2/rosidl_dynamic_typesupport_fastrtps.git
  GIT_TAG ${ROS2_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR ${SRC_DIR}/rosidl_dynamic_typesupport_fastrtps
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS fastcdr fastdds rcutils rosidl_dynamic_typesupport
)

ExternalProject_Add(rmw_fastrtps_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_fastrtps
  SOURCE_SUBDIR rmw_fastrtps_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_fastrtps fastcdr fastdds rmw rmw_fastrtps_shared_cpp
          rmw_dds_common rcutils rcpputils tracetools
          rosidl_dynamic_typesupport rosidl_dynamic_typesupport_fastrtps
          rosidl_runtime_c rosidl_typesupport_fastrtps_c
          rosidl_typesupport_fastrtps_cpp
)

# ====================
# rmw_cyclonedds backend
# ====================

ExternalProject_Add(cyclonedds
  GIT_REPOSITORY https://github.com/eclipse-cyclonedds/cyclonedds.git
  GIT_TAG releases/0.10.x
  SOURCE_DIR ${SRC_DIR}/cyclonedds
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    -DBUILD_EXAMPLES=OFF
    -DBUILD_DDSPERF=OFF
    -DENABLE_SHM=OFF
    -DENABLE_SECURITY=OFF
    -DENABLE_SSL=OFF
    "-DCMAKE_C_FLAGS=-D_GNU_SOURCE -D__STDC_FORMAT_MACROS -include errno.h"
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
)

ExternalProject_Add(rmw_cyclonedds_cpp
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_cyclonedds
  SOURCE_SUBDIR rmw_cyclonedds_cpp
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_cyclonedds cyclonedds rmw rmw_dds_common
          rcutils rcpputils tracetools rosidl_runtime_c
          rosidl_typesupport_introspection_c rosidl_typesupport_introspection_cpp
)

# ====================
ExternalProject_Add(rmw_implementation
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SRC_DIR}/rmw_implementation
  SOURCE_SUBDIR rmw_implementation
  CMAKE_ARGS ${COMMON_CMAKE_ARGS}
  DEPENDS _dl_rmw_implementation rmw rcutils rcpputils ament_index_cpp
          rmw_zenoh_cpp rmw_fastrtps_cpp rmw_cyclonedds_cpp
)

# ====================
# carla-ros2-native-lib
# ====================

ExternalProject_Add (
  carla-ros2-native-lib
  DEPENDS rclcpp rmw_zenoh_cpp rmw_fastrtps_cpp rmw_cyclonedds_cpp rmw_implementation
          sensor_msgs nav_msgs tf2_msgs carla_msgs
          rosidl_typesupport_cpp rosidl_typesupport_c
          rcl_logging_spdlog action_msgs
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/LibCarlaRos2Native
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_PREFIX_PATH=${PROJECT_INSTALL_PATH}
    "-DCMAKE_CXX_FLAGS=-D_GNU_SOURCE -stdlib=libc++"
)

set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux)

make_directory (${CARLA_PLUGIN_BINARY_PATH})

add_custom_command (
  TARGET carla-ros2-native-lib
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${PROJECT_INSTALL_PATH}/lib/*.so*
    ${CARLA_PLUGIN_BINARY_PATH}
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_INSTALL_PATH}/share
    ${CARLA_PLUGIN_BINARY_PATH}/share
)

carla_add_custom_target (
  carla-ros2-native
  "Build the ROS2-Native CARLA subproject."
  DEPENDS carla-ros2-native-lib
)
