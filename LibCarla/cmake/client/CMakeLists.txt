cmake_minimum_required(VERSION 3.9.0)
project(libcarla-client)

# Install rpclib.
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

file(GLOB libcarla_sources
    "${libcarla_source_path}/carla/*.h"
    "${libcarla_source_path}/carla/*.cpp")

file(GLOB_RECURSE libcarla_client_sources
    "${libcarla_source_path}/carla/client/*.h"
    "${libcarla_source_path}/carla/client/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h"
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/streaming/*.h"
    "${libcarla_source_path}/carla/streaming/*.cpp")

# Create targets for debug and release in the same build type.
foreach(target carla_client_debug carla_client)
  add_library(${target} STATIC ${libcarla_client_sources} ${libcarla_sources})

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

# Install headers.

install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

file(GLOB boost_libraries "${BOOST_LIB_PATH}/*")
install(FILES ${boost_libraries} DESTINATION lib)

file(GLOB libcarla_carla_headers
    "${libcarla_source_path}/carla/*.cpp"
    "${libcarla_source_path}/carla/*.h")
install(FILES ${libcarla_carla_headers} DESTINATION include/carla)

file(GLOB libcarla_moodycamel_headers
    "${libcarla_source_path}/moodycamel/*.h")
install(FILES ${libcarla_moodycamel_headers} DESTINATION include/moodycamel)

file(GLOB libcarla_carla_client_headers
    "${libcarla_source_path}/carla/client/*.cpp"
    "${libcarla_source_path}/carla/client/*.h")
install(FILES ${libcarla_carla_client_headers} DESTINATION include/carla/client)

file(GLOB libcarla_carla_rpc_headers
    "${libcarla_source_path}/carla/rpc/*.cpp"
    "${libcarla_source_path}/carla/rpc/*.h")
install(FILES ${libcarla_carla_rpc_headers} DESTINATION include/carla/rpc)

file(GLOB libcarla_carla_streaming_headers
    "${libcarla_source_path}/carla/streaming/*.cpp"
    "${libcarla_source_path}/carla/streaming/*.h")
install(FILES ${libcarla_carla_streaming_headers} DESTINATION include/carla/streaming)

file(GLOB libcarla_carla_streaming_detail_headers
    "${libcarla_source_path}/carla/streaming/detail/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/*.h")
install(FILES ${libcarla_carla_streaming_detail_headers} DESTINATION include/carla/streaming/detail)

file(GLOB libcarla_carla_streaming_detail_tcp_headers
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.cpp"
    "${libcarla_source_path}/carla/streaming/detail/tcp/*.h")
install(FILES ${libcarla_carla_streaming_detail_tcp_headers} DESTINATION include/carla/streaming/detail/tcp)

file(GLOB libcarla_carla_streaming_low_level_headers
    "${libcarla_source_path}/carla/streaming/low_level/*.cpp"
    "${libcarla_source_path}/carla/streaming/low_level/*.h")
install(FILES ${libcarla_carla_streaming_low_level_headers} DESTINATION include/carla/streaming/low_level)
