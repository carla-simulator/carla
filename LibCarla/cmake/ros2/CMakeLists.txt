cmake_minimum_required(VERSION 3.5.1)
project(libcarla_ros2)

# All ros2 source files compiled ONCE with both middleware macros defined.
# This ensures ROS2::Enable, DDSMiddlewareFactory::IsMiddlewareAvailable,
# and all CreatePublisher<T>/CreateSubscriber<S> instantiations see both
# CARLA_ROS2_DDS_FASTDDS and CARLA_ROS2_DDS_CYCLONEDDS at compile time.
# cmake/fast_dds and cmake/cyclone_dds are install-only (no library targets);
# all compiled code lives here.

file(GLOB libcarla_ros2_sources
  "${libcarla_source_path}/carla/ros2/*.cpp"
  "${libcarla_source_path}/carla/ros2/publishers/*.cpp"
  "${libcarla_source_path}/carla/ros2/subscribers/*.cpp"
  "${libcarla_source_path}/carla/ros2/listeners/*.cpp"
  "${libcarla_source_path}/carla/ros2/types/*.cpp"
  "${libcarla_source_path}/carla/ros2/dds/cyclonedds/*.cpp")

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)
  add_library(carla_ros2 STATIC ${libcarla_ros2_sources})

  target_compile_definitions(carla_ros2 PRIVATE
      CARLA_ROS2_DDS_FASTDDS
      CARLA_ROS2_DDS_CYCLONEDDS)
  target_compile_options(carla_ros2 PRIVATE -fexceptions)

  target_include_directories(carla_ros2 SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_ros2 PRIVATE
      "${FASTDDS_INCLUDE_PATH}"
      "${CYCLONEDDS_INCLUDE_PATH}"
      "${libcarla_source_path}/carla/ros2")

  install(TARGETS carla_ros2 DESTINATION lib)
  set_target_properties(carla_ros2 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)
  add_library(carla_ros2_debug STATIC ${libcarla_ros2_sources})

  target_compile_definitions(carla_ros2_debug PRIVATE
      CARLA_ROS2_DDS_FASTDDS
      CARLA_ROS2_DDS_CYCLONEDDS)
  target_compile_options(carla_ros2_debug PRIVATE -fexceptions)

  target_include_directories(carla_ros2_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_ros2_debug PRIVATE
      "${FASTDDS_INCLUDE_PATH}"
      "${CYCLONEDDS_INCLUDE_PATH}"
      "${libcarla_source_path}/carla/ros2")

  install(TARGETS carla_ros2_debug DESTINATION lib)
  set_target_properties(carla_ros2_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_ros2_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
