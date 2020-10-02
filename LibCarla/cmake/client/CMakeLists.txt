cmake_minimum_required(VERSION 3.5.1)
project(libcarla-client)

# Enable/Install RSS VARIANT
option (BUILD_RSS_VARIANT "Enables ad-rss-lib based RSS components" OFF)
set(carla_target_postfix "")
if (BUILD_RSS_VARIANT)
  set(carla_target_postfix "_rss")
  set(ADRSS_INCLUDE_DIRS)
  set(ADRSS_LIBS)

  find_package(spdlog CONFIG REQUIRED
    HINTS ${ADRSS_INSTALL_DIR}/spdlog/)
  get_target_property(spdlog_include_dirs spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(spdlog_file spdlog::spdlog LOCATION)
  foreach(dir ${spdlog_include_dirs})
    file(GLOB spdlog_includes "${dir}/*")
    install(DIRECTORY ${spdlog_includes} DESTINATION include/system)
    list(APPEND ADRSS_INCLUDE_DIRS ${dir})
  endforeach()
  install(FILES ${spdlog_file} DESTINATION lib)
  list(APPEND ADRSS_LIBS ${spdlog_file})

  install(FILES ${ADRSS_INSTALL_DIR}/PROJ4/lib/libproj.a DESTINATION lib)

  foreach(ad_lib ad_physics ad_rss ad_map_access ad_map_opendrive_reader ad_rss_map_integration)
    set(${ad_lib}_file ${ADRSS_INSTALL_DIR}/${ad_lib}/lib/lib${ad_lib}.a)
    install(FILES ${${ad_lib}_file} DESTINATION lib)
    list(APPEND ADRSS_LIBS ${${ad_lib}_file})

    # Install corresponding python libs if available
    file(GLOB ${ad_lib}_python_libs "${ADRSS_INSTALL_DIR}/${ad_lib}/lib/lib${ad_lib}_python*.a")
    foreach(ad_python_lib ${${ad_lib}_python_libs})
      if (EXISTS ${ad_python_lib})
        install(FILES ${ad_python_lib} DESTINATION lib)
      endif()
    endforeach()

    set(${ad_lib}_include_dir ${ADRSS_INSTALL_DIR}/${ad_lib}/include)
    file(GLOB ${ad_lib}_includes "${${ad_lib}_include_dir}/*")
    install(DIRECTORY ${${ad_lib}_includes} DESTINATION include/system)
    list(APPEND ADRSS_INCLUDE_DIRS ${${ad_lib}_include_dir})
  endforeach()
endif()

# Install Recast&Detour libraries
install(DIRECTORY "${RECAST_INCLUDE_PATH}/recast" DESTINATION include/system)
file(GLOB libcarla_carla_recastlib "${RECAST_LIB_PATH}/*.*")
install(FILES ${libcarla_carla_recastlib} DESTINATION lib)

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
else ()
  # Install libpng library
  install(DIRECTORY "${LIBPNG_INCLUDE_PATH}" DESTINATION include/system)
  file(GLOB libcarla_carla_libpnglib "${LIBPNG_LIB_PATH}/*.*")
  install(FILES ${libcarla_carla_libpnglib} DESTINATION lib)
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

file(GLOB libcarla_carla_nav_sources
    "${libcarla_source_path}/carla/nav/*.cpp"
    "${libcarla_source_path}/carla/nav/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_nav_sources}")
install(FILES ${libcarla_carla_nav_sources} DESTINATION include/carla/nav)

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

if (BUILD_RSS_VARIANT)
  file(GLOB libcarla_carla_rss_sources
      "${libcarla_source_path}/carla/rss/*.cpp"
      "${libcarla_source_path}/carla/rss/*.h")
  set(libcarla_sources "${libcarla_sources};${libcarla_carla_rss_sources}")
  install(FILES ${libcarla_carla_rss_sources} DESTINATION include/carla/rss)
endif()

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

file(GLOB libcarla_odr_spiral_sources
    "${libcarla_source_thirdparty_path}/odrSpiral/*.cpp"
    "${libcarla_source_thirdparty_path}/odrSpiral/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_odr_spiral_sources}")
install(FILES ${libcarla_odr_spiral_sources} DESTINATION include/odrSpiral)

file(GLOB libcarla_moodycamel_sources
    "${libcarla_source_thirdparty_path}/moodycamel/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_moodycamel_sources}")
install(FILES ${libcarla_moodycamel_sources} DESTINATION include/moodycamel)

file(GLOB libcarla_pugixml_sources
    "${libcarla_source_thirdparty_path}/pugixml/*.cpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.hpp"
    "${libcarla_source_thirdparty_path}/pugixml/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_pugixml_sources}")
install(FILES ${libcarla_pugixml_sources} DESTINATION include/pugixml)

file(GLOB libcarla_carla_trafficmanager_sources
    "${libcarla_source_path}/carla/trafficmanager/*.cpp"
    "${libcarla_source_path}/carla/trafficmanager/*.h")
set(libcarla_sources "${libcarla_sources};${libcarla_carla_trafficmanager_sources}")
install(FILES ${libcarla_carla_trafficmanager_sources} DESTINATION include/carla/trafficmanager)

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================


if (LIBCARLA_BUILD_RELEASE)
  add_library(carla_client${carla_target_postfix} STATIC ${libcarla_sources})

  target_include_directories(carla_client${carla_target_postfix} SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${RECAST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")

  if (BUILD_RSS_VARIANT)
    target_compile_definitions(carla_client${carla_target_postfix} PRIVATE RSS_ENABLED RSS_USE_TBB)
    target_link_libraries(carla_client${carla_target_postfix} ${ADRSS_LIBS} tbb)
    target_include_directories(carla_client${carla_target_postfix} SYSTEM PRIVATE
      ${ADRSS_INCLUDE_DIRS})
  endif()

  install(TARGETS carla_client${carla_target_postfix} DESTINATION lib)

  if (WIN32) # @todo Fix PythonAPI build on Windows.
    set_target_properties(carla_client${carla_target_postfix} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/Recast.lib")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/Detour.lib")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/DetourCrowd.lib")
  else ()
    if (NOT DEFINED CMAKE_CXX_FLAGS_RELEASE_CLIENT)
      set(CMAKE_CXX_FLAGS_RELEASE_CLIENT ${CMAKE_CXX_FLAGS_RELEASE})
    endif()

    set_target_properties(carla_client${carla_target_postfix} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE_CLIENT}")

    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libRecast.a")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libDetour.a")
    target_link_libraries(carla_client${carla_target_postfix} "${RECAST_LIB_PATH}/libDetourCrowd.a")

  endif (WIN32)

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_client${carla_target_postfix}_debug STATIC ${libcarla_sources})

  target_include_directories(carla_client${carla_target_postfix}_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${RECAST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")

  if (BUILD_RSS_VARIANT)
    target_compile_definitions(carla_client${carla_target_postfix}_debug PRIVATE RSS_ENABLED RSS_USE_TBB)
    target_link_libraries(carla_client${carla_target_postfix}_debug ${ADRSS_LIBS} tbb)
    target_include_directories(carla_client${carla_target_postfix}_debug SYSTEM PRIVATE
      ${ADRSS_INCLUDE_DIRS})
  endif()

  install(TARGETS carla_client${carla_target_postfix}_debug DESTINATION lib)

  if (WIN32) # @todo Fix PythonAPI build on Windows.
    set_target_properties(carla_client${carla_target_postfix}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")

    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/Recast.lib")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/Detour.lib")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/DetourCrowd.lib")
  else ()
    if (NOT DEFINED CMAKE_CXX_FLAGS_DEBUG_CLIENT)
      set(CMAKE_CXX_FLAGS_DEBUG_CLIENT ${CMAKE_CXX_FLAGS_DEBUG})
    endif()

    set_target_properties(carla_client${carla_target_postfix}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG_CLIENT}")

    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libRecast.a")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libDetour.a")
    target_link_libraries(carla_client${carla_target_postfix}_debug "${RECAST_LIB_PATH}/libDetourCrowd.a")

  endif (WIN32)

  target_compile_definitions(carla_client${carla_target_postfix}_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
