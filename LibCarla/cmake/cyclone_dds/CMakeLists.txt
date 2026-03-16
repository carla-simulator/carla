cmake_minimum_required(VERSION 3.5.1)
project(libcarla_cyclonedds)

# Install headers shared with Fast DDS build.

file(GLOB libcarla_carla_cyclonedds_headers
  "${libcarla_source_path}/carla/ros2/*.h"
  "${libcarla_source_path}/carla/ros2/publishers/*.h"
  "${libcarla_source_path}/carla/ros2/subscribers/*.h"
  "${libcarla_source_path}/carla/ros2/listeners/*.h"
  "${libcarla_source_path}/carla/ros2/types/*.h"
  )
install(FILES ${libcarla_carla_cyclonedds_headers} DESTINATION include/carla/ros2)

file(GLOB libcarla_carla_dds_headers
  "${libcarla_source_path}/carla/ros2/dds/*.h"
  )
install(FILES ${libcarla_carla_dds_headers} DESTINATION include/carla/ros2/dds)

file(GLOB libcarla_carla_cyclonedds_dds_headers
  "${libcarla_source_path}/carla/ros2/dds/cyclonedds/*.h"
  )
install(FILES ${libcarla_carla_cyclonedds_dds_headers} DESTINATION include/carla/ros2/dds/cyclonedds)

file(GLOB cyclonedds_dependencies "${CYCLONEDDS_LIB_PATH}/*.a")
install(FILES ${cyclonedds_dependencies} DESTINATION lib)

# Sources: ROS2.cpp + DDS-agnostic infrastructure + FastDDS message types + idlc C descriptors.
# types/*.cpp MUST be included: CddsSertype<T> calls msg_pubsub_type::serialize(),
# deserialize(), and getSerializedSizeProvider() — all defined in *PubSubTypes.cpp —
# and message constructors/destructors defined in the companion *.cpp files.
# libfastrtps is linked by Carla.Build.cs for the cyclonedds build (TopicDataType base).
# The idlc-generated *.c files provide dds_topic_descriptor_t for each message type.

file(GLOB libcarla_cyclonedds_type_sources
  "${libcarla_source_path}/carla/ros2/types/cyclonedds/*/*.c")

file(GLOB libcarla_cyclonedds_sources
  "${libcarla_source_path}/carla/ros2/ROS2.cpp"
  "${libcarla_source_path}/carla/ros2/publishers/*.cpp"
  "${libcarla_source_path}/carla/ros2/subscribers/*.cpp"
  "${libcarla_source_path}/carla/ros2/listeners/*.cpp"
  "${libcarla_source_path}/carla/ros2/dds/*.cpp"
  "${libcarla_source_path}/carla/ros2/types/*.cpp")

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)
  # Compile idlc-generated C files as a separate pure-C library to avoid
  # passing -std=c++17 to C source files (invalid argument for C compiler).
  add_library(carla_cyclonedds_types STATIC ${libcarla_cyclonedds_type_sources})
  target_include_directories(carla_cyclonedds_types PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds_types PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
  set_target_properties(carla_cyclonedds_types PROPERTIES COMPILE_FLAGS "-std=c11")

  add_library(carla_cyclonedds STATIC ${libcarla_cyclonedds_sources})

  target_compile_options(carla_cyclonedds PRIVATE -fexceptions -std=c++17)
  target_compile_definitions(carla_cyclonedds PRIVATE CARLA_ROS2_DDS_CYCLONEDDS)

  target_include_directories(carla_cyclonedds SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_cyclonedds PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  # FastDDS headers are needed for *PubSubTypes.h type definitions (inherit from
  # eprosima::fastdds::dds::TopicDataType) and for FastDDSPublisherBackend<T>.
  # libfastrtps is resolved by the UE4 plugin linker, not at static-lib stage.
  target_include_directories(carla_cyclonedds PRIVATE "${FASTDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds PRIVATE "${libcarla_source_path}/carla/ros2")
  target_include_directories(carla_cyclonedds PRIVATE "${libcarla_source_path}")
  # Allow idlc-generated headers to resolve cross-type includes (e.g. "std_msgs/Header.h")
  target_include_directories(carla_cyclonedds PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
  # Link only the C core (ddsc); cyclonedds-cxx cannot be built with the UE4
  # toolchain because it requires exceptions (-fno-exceptions is set by the toolchain).
  target_link_libraries(carla_cyclonedds ddsc carla_cyclonedds_types)
  install(TARGETS carla_cyclonedds carla_cyclonedds_types DESTINATION lib)
  set_target_properties(carla_cyclonedds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_cyclonedds_types_debug STATIC ${libcarla_cyclonedds_type_sources})
  target_include_directories(carla_cyclonedds_types_debug PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds_types_debug PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
  set_target_properties(carla_cyclonedds_types_debug PROPERTIES COMPILE_FLAGS "-std=c11")

  add_library(carla_cyclonedds_debug STATIC ${libcarla_cyclonedds_sources})

  target_compile_options(carla_cyclonedds_debug PRIVATE -fexceptions -std=c++17)
  target_compile_definitions(carla_cyclonedds_debug PRIVATE CARLA_ROS2_DDS_CYCLONEDDS)

  target_include_directories(carla_cyclonedds_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_cyclonedds_debug PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds_debug PRIVATE "${FASTDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds_debug PRIVATE "${libcarla_source_path}/carla/ros2")
  target_include_directories(carla_cyclonedds_debug PRIVATE "${libcarla_source_path}")
  target_include_directories(carla_cyclonedds_debug PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
  target_link_libraries(carla_cyclonedds_debug ddsc carla_cyclonedds_types_debug)
  install(TARGETS carla_cyclonedds_debug carla_cyclonedds_types_debug DESTINATION lib)
  set_target_properties(carla_cyclonedds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_cyclonedds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
