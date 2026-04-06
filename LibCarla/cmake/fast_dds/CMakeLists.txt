cmake_minimum_required(VERSION 3.5.1)
project(libcarla_fastdds)

# Install headers.

file(GLOB libcarla_carla_ros2_headers      "${libcarla_source_path}/carla/ros2/*.h")
file(GLOB libcarla_carla_publishers_headers "${libcarla_source_path}/carla/ros2/publishers/*.h")
file(GLOB libcarla_carla_subscribers_headers "${libcarla_source_path}/carla/ros2/subscribers/*.h")
file(GLOB libcarla_carla_listeners_headers  "${libcarla_source_path}/carla/ros2/listeners/*.h")
file(GLOB libcarla_carla_types_headers      "${libcarla_source_path}/carla/ros2/types/*.h")
file(GLOB libcarla_carla_types_msg_headers  "${libcarla_source_path}/carla/ros2/types/msg/*.h")
file(GLOB libcarla_carla_dds_headers        "${libcarla_source_path}/carla/ros2/dds/*.h")
file(GLOB libcarla_carla_dds_fastdds_headers "${libcarla_source_path}/carla/ros2/dds/fastdds/*.h")

install(FILES ${libcarla_carla_ros2_headers}       DESTINATION include/carla/ros2)
install(FILES ${libcarla_carla_publishers_headers}  DESTINATION include/carla/ros2/publishers)
install(FILES ${libcarla_carla_subscribers_headers} DESTINATION include/carla/ros2/subscribers)
install(FILES ${libcarla_carla_listeners_headers}   DESTINATION include/carla/ros2/listeners)
install(FILES ${libcarla_carla_types_headers}       DESTINATION include/carla/ros2/types)
install(FILES ${libcarla_carla_types_msg_headers}   DESTINATION include/carla/ros2/types/msg)
install(FILES ${libcarla_carla_dds_headers}         DESTINATION include/carla/ros2/dds)
install(FILES ${libcarla_carla_dds_fastdds_headers} DESTINATION include/carla/ros2/dds/fastdds)

file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.a")
install(FILES ${fast_dds_dependencies} DESTINATION lib)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastcdr"   DESTINATION include)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastdds"   DESTINATION include)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastrtps"  DESTINATION include)


file(GLOB libcarla_fastdds_sources
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
  add_library(carla_fastdds STATIC ${libcarla_fastdds_sources})

  target_compile_definitions(carla_fastdds PRIVATE CARLA_ROS2_DDS_FASTDDS CARLA_ROS2_DDS_CYCLONEDDS)
  target_compile_options(carla_fastdds PRIVATE -fexceptions)

  target_include_directories(carla_fastdds SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_fastdds PRIVATE "${FASTDDS_INCLUDE_PATH}")
  target_include_directories(carla_fastdds PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_fastdds PRIVATE "${libcarla_source_path}/carla/ros2")
  target_link_libraries(carla_fastdds fastrtps fastcdr "${FAST_DDS_LIBRARIES}")
  install(TARGETS carla_fastdds DESTINATION lib)
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_fastdds_debug STATIC ${libcarla_fastdds_sources})

  target_compile_definitions(carla_fastdds_debug PRIVATE CARLA_ROS2_DDS_FASTDDS CARLA_ROS2_DDS_CYCLONEDDS)
  target_compile_options(carla_fastdds_debug PRIVATE -fexceptions)

  target_include_directories(carla_fastdds_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_fastdds_debug PRIVATE "${FASTDDS_INCLUDE_PATH}")
  target_include_directories(carla_fastdds_debug PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_fastdds_debug PRIVATE "${libcarla_source_path}/carla/ros2")
  install(TARGETS carla_fastdds_debug DESTINATION lib)
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_fastdds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
