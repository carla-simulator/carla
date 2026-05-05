cmake_minimum_required(VERSION 3.5.1)
project(libcarla-unit-tests)

if (CMAKE_BUILD_TYPE STREQUAL "Client")
  set(carla_config client)
elseif (CMAKE_BUILD_TYPE STREQUAL "Server")
  set(carla_config server)
endif ()
if (BUILD_RSS_VARIANT)
  set(carla_target_postfix "_rss")
else()
  set(carla_target_postfix "")
endif()

link_directories(
    ${RPCLIB_LIB_PATH}
    ${GTEST_LIB_PATH})

file(GLOB libcarla_test_sources
    "${libcarla_source_path}/carla/profiler/*.cpp"
    "${libcarla_source_path}/carla/profiler/*.h"
    "${libcarla_source_path}/test/*.cpp"
    "${libcarla_source_path}/test/*.h"
    "${libcarla_source_path}/test/${carla_config}/*.cpp"
    "${libcarla_source_path}/test/${carla_config}/*.h"
    "${libcarla_source_path}/test/common/*.cpp"
    "${libcarla_source_path}/test/common/*.h")

if (NOT LIBCARLA_WITH_ROS2)
  list(REMOVE_ITEM libcarla_test_sources
      "${libcarla_source_path}/test/server/test_middleware.cpp")
endif()

file(GLOB libcarla_test_client_sources "")

if (LIBCARLA_BUILD_DEBUG)
  list(APPEND build_targets libcarla_test_${carla_config}_debug)
endif()

if (LIBCARLA_BUILD_RELEASE)
  list(APPEND build_targets libcarla_test_${carla_config}_release)
endif()

# Create targets for debug and release in the same build type.
foreach(target ${build_targets})

  add_executable(${target} ${libcarla_test_sources})

  target_compile_definitions(${target} PUBLIC
      -DLIBCARLA_ENABLE_PROFILER
      -DLIBCARLA_WITH_GTEST)

  target_include_directories(${target} SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${GTEST_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_PATH}")

  target_include_directories(${target} PRIVATE
      "${libcarla_source_path}/test")

  # Server tests exercise CDR serialization (CdrSerialization.h) and
  # GenericCdrPubSubType (inherits TopicDataType from fastrtps).
  # Enable exceptions because Fast-CDR templates use try/catch internally.
  # Only link FastDDS when ROS2 support was explicitly enabled via --ros2.
  if (CMAKE_BUILD_TYPE STREQUAL "Server" AND LIBCARLA_WITH_ROS2)
    target_include_directories(${target} SYSTEM PRIVATE "${FASTDDS_INCLUDE_PATH}")
    if (WIN32)
      target_compile_options(${target} PRIVATE /EHsc)
      file(GLOB _foonathan_lib "${FASTDDS_LIB_PATH}/foonathan_memory-*.lib")
      if (NOT _foonathan_lib)
        message(FATAL_ERROR "foonathan_memory .lib not found in ${FASTDDS_LIB_PATH}")
      endif()
      target_link_libraries(${target} "${FASTDDS_LIB_PATH}/fastrtps.lib")
      target_link_libraries(${target} "${FASTDDS_LIB_PATH}/fastcdr.lib")
      target_link_libraries(${target} ${_foonathan_lib})
    else()
      target_compile_options(${target} PRIVATE -fexceptions)
      file(GLOB _foonathan_lib "${FASTDDS_LIB_PATH}/libfoonathan_memory-*.a")
      if (NOT _foonathan_lib)
        message(FATAL_ERROR "libfoonathan_memory .a not found in ${FASTDDS_LIB_PATH}")
      endif()
      target_link_libraries(${target} "${FASTDDS_LIB_PATH}/libfastrtps.a")
      target_link_libraries(${target} "${FASTDDS_LIB_PATH}/libfastcdr.a")
      target_link_libraries(${target} ${_foonathan_lib})
    endif()
  endif()

  if (WIN32)
      target_link_libraries(${target} "gtest_main.lib")
      target_link_libraries(${target} "gtest.lib")
      target_link_libraries(${target} "rpc.lib")
  else()
      target_link_libraries(${target} "-lrpc")
      target_link_libraries(${target} "-lgtest_main")
      target_link_libraries(${target} "-lgtest")
  endif()

  install(TARGETS ${target} DESTINATION test OPTIONAL)
endforeach(target)

if (LIBCARLA_BUILD_DEBUG)
  # Specific options for debug.
  set_target_properties(libcarla_test_${carla_config}_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_link_libraries(libcarla_test_${carla_config}_debug "carla_${carla_config}${carla_target_postfix}_debug")
  target_compile_definitions(libcarla_test_${carla_config}_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(libcarla_test_${carla_config}_debug "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()

if (LIBCARLA_BUILD_RELEASE)
  # Specific options for release.
  set_target_properties(libcarla_test_${carla_config}_release PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
  target_link_libraries(libcarla_test_${carla_config}_release "carla_${carla_config}${carla_target_postfix}")
  if (CMAKE_BUILD_TYPE STREQUAL "Client")
      target_link_libraries(libcarla_test_${carla_config}_release "${BOOST_LIB_PATH}/libboost_filesystem.a")
  endif()
endif()
