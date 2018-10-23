cmake_minimum_required(VERSION 3.9.0)
project(libcarla-client)

# Install rpclib.
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

# Install boost headers.

install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

file(GLOB boost_libraries "${BOOST_LIB_PATH}/*")
install(FILES ${boost_libraries} DESTINATION lib)

# Add sources.

file(GLOB libcarla_carla_sources
    "${libcarla_source_path}/carla/*.cpp"
    "${libcarla_source_path}/carla/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sources}")
install(FILES ${libcarla_carla_sources} DESTINATION include/carla)

file(GLOB libcarla_carla_client_sources
    "${libcarla_source_path}/carla/client/*.cpp"
    "${libcarla_source_path}/carla/client/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_client_sources}")
install(FILES ${libcarla_carla_client_sources} DESTINATION include/carla/client)

file(GLOB libcarla_carla_client_detail_sources
    "${libcarla_source_path}/carla/client/detail/*.cpp"
    "${libcarla_source_path}/carla/client/detail/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_client_detail_sources}")
install(FILES ${libcarla_carla_client_detail_sources} DESTINATION include/carla/client/detail)

file(GLOB libcarla_carla_geom_sources
    "${libcarla_source_path}/carla/geom/*.cpp"
    "${libcarla_source_path}/carla/geom/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_geom_sources}")
install(FILES ${libcarla_carla_geom_sources} DESTINATION include/carla/geom)

file(GLOB libcarla_carla_image_sources
    "${libcarla_source_path}/carla/image/*.cpp"
    "${libcarla_source_path}/carla/image/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_image_sources}")
install(FILES ${libcarla_carla_image_sources} DESTINATION include/carla/image)

file(GLOB libcarla_carla_opendrive_sources
    "${libcarla_source_path}/carla/opendrive/*.cpp"
    "${libcarla_source_path}/carla/opendrive/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_opendrive_sources}")
install(FILES ${libcarla_carla_opendrive_sources} DESTINATION include/carla/opendrive)

file(GLOB libcarla_carla_pointcloud_sources
    "${libcarla_source_path}/carla/pointcloud/*.cpp"
    "${libcarla_source_path}/carla/pointcloud/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_pointcloud_sources}")
install(FILES ${libcarla_carla_pointcloud_sources} DESTINATION include/carla/pointcloud)

file(GLOB libcarla_carla_profiler_sources
    "${libcarla_source_path}/carla/profiler/*.cpp"
    "${libcarla_source_path}/carla/profiler/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_profiler_sources}")
install(FILES ${libcarla_carla_profiler_sources} DESTINATION include/carla/profiler)

file(GLOB libcarla_carla_road_sources
    "${libcarla_source_path}/carla/road/*.cpp"
    "${libcarla_source_path}/carla/road/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_sources}")
install(FILES ${libcarla_carla_road_sources} DESTINATION include/carla/road)

file(GLOB libcarla_carla_rpc_sources
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_rpc_sources}")
install(FILES ${libcarla_carla_rpc_sources} DESTINATION include/carla/rpc)

file(GLOB libcarla_carla_sensor_sources
    "${libcarla_source_path}/carla/sensor/*.cpp"
    "${libcarla_source_path}/carla/sensor/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_sources}")
install(FILES ${libcarla_carla_sensor_sources} DESTINATION include/carla/sensor)

file(GLOB libcarla_carla_sensor_data_sources
    "${libcarla_source_path}/carla/sensor/data/*.cpp"
    "${libcarla_source_path}/carla/sensor/data/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_data_sources}")
install(FILES ${libcarla_carla_sensor_data_sources} DESTINATION include/carla/sensor/data)

file(GLOB libcarla_carla_sensor_s11n_sources
    "${libcarla_source_path}/carla/sensor/s11n/*.cpp"
    "${libcarla_source_path}/carla/sensor/s11n/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_sensor_s11n_sources}")
install(FILES ${libcarla_carla_sensor_s11n_sources} DESTINATION include/carla/sensor/s11n)

file(GLOB libcarla_carla_streaming_sources
    "${libcarla_source_path}/carla/streaming/*.cpp"
    "${libcarla_source_path}/carla/streaming/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_sources}")
install(FILES ${libcarla_carla_streaming_sources} DESTINATION include/carla/streaming)

file(GLOB libcarla_carla_streaming_detail_sources
    "${libcarla_source_path}/carla/streaming/detail/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_detail_sources}")
install(FILES ${libcarla_carla_streaming_detail_sources} DESTINATION include/carla/streaming/detail)

file(GLOB libcarla_carla_streaming_detail_tcp_sources
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_detail_tcp_sources}")
install(FILES ${libcarla_carla_streaming_detail_tcp_sources} DESTINATION include/carla/streaming/detail/tcp)

file(GLOB libcarla_carla_streaming_low_level_sources
    "${libcarla_source_path}/carla/streaming/low_level/*.cpp"
    "${libcarla_source_path}/carla/streaming/low_level/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_streaming_low_level_sources}")
install(FILES ${libcarla_carla_streaming_low_level_sources} DESTINATION include/carla/streaming/low_level)

file(GLOB libcarla_moodycamel_sources
    "${libcarla_source_path}/moodycamel/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_moodycamel_sources}")
install(FILES ${libcarla_moodycamel_sources} DESTINATION include/moodycamel)

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

foreach(target carla_client_debug carla_client)
  add_library(${target} STATIC ${libcarla_sources})

  target_include_directories(${target} PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  # @todo This was disabled because now everything is built in the setup.py.
  # install(TARGETS ${target} DESTINATION lib)
endforeach(target)

# Specific options for debug.
set_target_properties(carla_client_debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
target_compile_definitions(carla_client_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

# Specific options for release.
set_target_properties(carla_client PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
