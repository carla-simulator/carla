cmake_minimum_required(VERSION 3.9.0)
project(libcarla-server)

# Install libc++ shared libraries.
file(GLOB LibCXX_Libraries "${LLVM_LIB_PATH}/libc++*")
install(FILES ${LibCXX_Libraries} DESTINATION lib)

# Install rpclib.
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)
install(FILES "${RPCLIB_LIB_PATH}/librpc.a" DESTINATION lib)

# Install headers.

install(DIRECTORY "${libcarla_source_path}/compiler" DESTINATION include)

file(GLOB libcarla_carla_headers "${libcarla_source_path}/carla/*.h")
install(FILES ${libcarla_carla_headers} DESTINATION include/carla)

file(GLOB libcarla_carla_rpc_headers "${libcarla_source_path}/carla/rpc/*.h")
install(FILES ${libcarla_carla_rpc_headers} DESTINATION include/carla/rpc)

file(GLOB libcarla_carla_streaming_headers "${libcarla_source_path}/carla/streaming/*.h")
install(FILES ${libcarla_carla_streaming_headers} DESTINATION include/carla/streaming)

install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)
