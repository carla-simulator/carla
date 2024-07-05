```make
cmake_minimum_required(VERSION 3.5.1)
project(example)

link_directories(
    ${RPCLIB_LIB_PATH})

file(GLOB example_sources "*.cpp" "*.h")

file(GLOB example_client_sources "")

set(carla_config client)
list(APPEND build_targets example_${carla_config}_debug)

# Create targets for debug and release in the same build type.
foreach(target ${build_targets})

  add_executable(${target} ${example_sources})

  target_compile_definitions(${target} PUBLIC
      -DLIBCARLA_ENABLE_PROFILER)

  target_include_directories(${target} SYSTEM PRIVATE
      "../../LibCarla/source"
      "../../Build/boost-1.80.0-install/include"
      "../../Build/rpclib-install/include/"
      "../../Build/recast-22dfcb-install/include/"
      "../../Build/zlib-install/include/"
      "../../Build/libpng-1.2.37-install/include/"
      "../../LibCarla/source/third-party/")

  target_link_directories(${target} SYSTEM PRIVATE
      "../../Build/boost-1.80.0-install/lib"
      "../../Build/rpclib-install/lib/"
      "../../Build/recast-22dfcb-install/lib/"
      "../../Build/zlib-install/lib/"
      "../../Build/libcarla-visualstudio/LibCarla/cmake/client/Release/"
      "../../Build/libpng-1.2.37-install/lib/")

  target_include_directories(${target} PRIVATE
      "${libcarla_source_path}/test")

  if (WIN32)
      target_link_libraries(${target} "rpc.lib")
      target_link_libraries(${target} "carla_client.lib")
      target_link_libraries(${target} "DebugUtils.lib")
      target_link_libraries(${target} "Detour.lib")
      target_link_libraries(${target} "DetourCrowd.lib")
      target_link_libraries(${target} "DetourTileCache.lib")
      target_link_libraries(${target} "Recast.lib")
      target_link_libraries(${target} "Shlwapi.lib")
  else()
      target_link_libraries(${target} "-lrpc")
  endif()

  install(TARGETS ${target} DESTINATION test OPTIONAL)
endforeach(target)

if (LIBCARLA_BUILD_DEBUG)
  # Specific options for debug.
  set_target_properties(example_${carla_config}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_link_libraries(example_${carla_config}_debug "carla_${carla_config}${carla_target_postfix}_debug")
  target_compile_definitions(example_${carla_config}_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(example_${carla_config}_debug "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()
```
