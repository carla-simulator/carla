cmake_minimum_required(VERSION 3.5.1)
project(libcarla_cyclonedds)

# Install headers.

file(GLOB libcarla_carla_ros2_headers        "${libcarla_source_path}/carla/ros2/*.h")
file(GLOB libcarla_carla_publishers_headers  "${libcarla_source_path}/carla/ros2/publishers/*.h")
file(GLOB libcarla_carla_subscribers_headers "${libcarla_source_path}/carla/ros2/subscribers/*.h")
file(GLOB libcarla_carla_listeners_headers   "${libcarla_source_path}/carla/ros2/listeners/*.h")
file(GLOB libcarla_carla_types_headers       "${libcarla_source_path}/carla/ros2/types/*.h")
file(GLOB libcarla_carla_types_msg_headers   "${libcarla_source_path}/carla/ros2/types/msg/*.h")
file(GLOB libcarla_carla_dds_headers         "${libcarla_source_path}/carla/ros2/dds/*.h")
file(GLOB libcarla_carla_dds_cyclonedds_headers "${libcarla_source_path}/carla/ros2/dds/cyclonedds/*.h")

install(FILES ${libcarla_carla_ros2_headers}          DESTINATION include/carla/ros2)
install(FILES ${libcarla_carla_publishers_headers}    DESTINATION include/carla/ros2/publishers)
install(FILES ${libcarla_carla_subscribers_headers}   DESTINATION include/carla/ros2/subscribers)
install(FILES ${libcarla_carla_listeners_headers}     DESTINATION include/carla/ros2/listeners)
install(FILES ${libcarla_carla_types_headers}         DESTINATION include/carla/ros2/types)
install(FILES ${libcarla_carla_types_msg_headers}     DESTINATION include/carla/ros2/types/msg)
install(FILES ${libcarla_carla_dds_headers}           DESTINATION include/carla/ros2/dds)
install(FILES ${libcarla_carla_dds_cyclonedds_headers} DESTINATION include/carla/ros2/dds/cyclonedds)

file(GLOB cyclonedds_dependencies "${CYCLONEDDS_LIB_PATH}/*.a")
install(FILES ${cyclonedds_dependencies} DESTINATION lib)
install(DIRECTORY "${CYCLONEDDS_INCLUDE_PATH}/dds"   DESTINATION include)
install(DIRECTORY "${CYCLONEDDS_INCLUDE_PATH}/ddsc"  DESTINATION include)

file(GLOB libcarla_cyclonedds_sources
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
  add_library(carla_cyclonedds STATIC ${libcarla_cyclonedds_sources})

  target_compile_definitions(carla_cyclonedds PRIVATE CARLA_ROS2_DDS_CYCLONEDDS CARLA_ROS2_DDS_FASTDDS)
  target_compile_options(carla_cyclonedds PRIVATE -fexceptions)

  target_include_directories(carla_cyclonedds SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  # CycloneDDS C headers (dds/dds.h, dds/ddsi/*.h)
  target_include_directories(carla_cyclonedds PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  # Fast-CDR headers (fastcdr/Cdr.h) — used by CdrSerialization.h
  target_include_directories(carla_cyclonedds PRIVATE "${FASTDDS_INCLUDE_PATH}")

  target_link_libraries(carla_cyclonedds
      "${CYCLONEDDS_LIB_PATH}/libddsc.a"
      "${FASTDDS_LIB_PATH}/libfastcdr.a")
  install(TARGETS carla_cyclonedds DESTINATION lib)
  set_target_properties(carla_cyclonedds PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_cyclonedds_debug STATIC ${libcarla_cyclonedds_sources})

  target_compile_definitions(carla_cyclonedds_debug PRIVATE CARLA_ROS2_DDS_CYCLONEDDS CARLA_ROS2_DDS_FASTDDS)
  target_compile_options(carla_cyclonedds_debug PRIVATE -fexceptions)

  target_include_directories(carla_cyclonedds_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_cyclonedds_debug PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
  target_include_directories(carla_cyclonedds_debug PRIVATE "${FASTDDS_INCLUDE_PATH}")

  target_link_libraries(carla_cyclonedds_debug
      "${CYCLONEDDS_LIB_PATH}/libddsc.a"
      "${FASTDDS_LIB_PATH}/libfastcdr.a")
  install(TARGETS carla_cyclonedds_debug DESTINATION lib)
  set_target_properties(carla_cyclonedds_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_cyclonedds_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
