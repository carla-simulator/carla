cmake_minimum_required(VERSION 3.5.1)
project(libcarla-client)

# Install rpclib (install in system folder to avoid extra warnings).
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include/system)
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

# Install boost headers (install in system folder to avoid extra warnings).
# @todo Remove boost from public interface of LibCarla.client.
install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include/system)
file(GLOB boost_libraries "${BOOST_LIB_PATH}/*.*")
install(FILES ${boost_libraries} DESTINATION lib)

# Windows need libpng alongside with zlib to be installed
if (WIN32)
    # Install zlib headers.
    file(GLOB zlib_includes "${ZLIB_INCLUDE_PATH}/*")
    install(FILES ${zlib_includes} DESTINATION include)
    # Install zlib lib.
    file(GLOB zlib_libraries "${ZLIB_LIB_PATH}/*")
    install(FILES ${zlib_libraries} DESTINATION lib)

    # Install libpng headers.
    file(GLOB pnglib_includes "${LIBPNG_INCLUDE_PATH}/*")
    install(FILES ${pnglib_includes} DESTINATION include)
    # Install zlib lib.
    file(GLOB libpng_libraries "${LIBPNG_LIB_PATH}/*")
    install(FILES ${libpng_libraries} DESTINATION lib)
endif (WIN32)

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

file(GLOB libcarla_carla_opendrive_parser_sources
    "${libcarla_source_path}/carla/opendrive/parser/*.cpp"
    "${libcarla_source_path}/carla/opendrive/parser/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_opendrive_parser_sources}")
install(FILES ${libcarla_carla_opendrive_parser_sources} DESTINATION include/carla/opendrive/parser)

file(GLOB libcarla_pugixml_sources
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_pugixml_sources}")

file(GLOB libcarla_carla_pointcloud_sources
    "${libcarla_source_path}/carla/pointcloud/*.cpp"
    "${libcarla_source_path}/carla/pointcloud/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_pointcloud_sources}")
install(FILES ${libcarla_carla_pointcloud_sources} DESTINATION include/carla/pointcloud)

file(GLOB libcarla_carla_profiler_headers
    "${libcarla_source_path}/carla/profiler/*.h")
install(FILES ${libcarla_carla_profiler_headers} DESTINATION include/carla/profiler)

file(GLOB libcarla_carla_road_sources
    "${libcarla_source_path}/carla/road/*.cpp"
    "${libcarla_source_path}/carla/road/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_sources}")
install(FILES ${libcarla_carla_road_sources} DESTINATION include/carla/road)

file(GLOB libcarla_carla_road_element_sources
    "${libcarla_source_path}/carla/road/element/*.cpp"
    "${libcarla_source_path}/carla/road/element/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_element_sources}")
install(FILES ${libcarla_carla_road_element_sources} DESTINATION include/carla/road/element)

file(GLOB libcarla_carla_cephes_sources
    "${libcarla_source_thirdparty_path}/cephes/*.cpp"
    "${libcarla_source_thirdparty_path}/cephes/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_cephes_sources}")

file(GLOB libcarla_carla_road_general_sources
    "${libcarla_source_path}/carla/road/general/*.cpp"
    "${libcarla_source_path}/carla/road/general/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_general_sources}")
install(FILES ${libcarla_carla_road_general_sources} DESTINATION include/carla/road/general)

file(GLOB libcarla_carla_road_object_sources
    "${libcarla_source_path}/carla/road/object/*.cpp"
    "${libcarla_source_path}/carla/road/object/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_object_sources}")
install(FILES ${libcarla_carla_road_object_sources} DESTINATION include/carla/road/object)

file(GLOB libcarla_carla_road_signal_sources
    "${libcarla_source_path}/carla/road/signal/*.cpp"
    "${libcarla_source_path}/carla/road/signal/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_road_signal_sources}")
install(FILES ${libcarla_carla_road_signal_sources} DESTINATION include/carla/road/signal)

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
    "${libcarla_source_thirdparty_path}/moodycamel/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_moodycamel_sources}")
install(FILES ${libcarla_moodycamel_sources} DESTINATION include/moodycamel)

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)

  add_library(carla_client STATIC ${libcarla_sources})

  target_include_directories(carla_client SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_client DESTINATION lib)

  if (WIN32) # @todo Fix PythonAPI build on Windows.
    set_target_properties(carla_client PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
  else ()
    if (NOT DEFINED CMAKE_CXX_FLAGS_RELEASE_CLIENT)
      set(CMAKE_CXX_FLAGS_RELEASE_CLIENT ${CMAKE_CXX_FLAGS_RELEASE})
    endif()
    set_target_properties(carla_client PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE_CLIENT}")
  endif (WIN32)

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_client_debug STATIC ${libcarla_sources})

  target_include_directories(carla_client_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_client_debug DESTINATION lib)

  set_target_properties(carla_client_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_client_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
