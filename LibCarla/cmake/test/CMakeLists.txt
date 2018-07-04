cmake_minimum_required(VERSION 3.9.0)
project(libcarla-unit-tests)

file(GLOB_RECURSE libcarla_test_sources
    "${libcarla_source_path}/carla/profiler/*.h"
    "${libcarla_source_path}/carla/profiler/*.cpp"
    "${libcarla_source_path}/test/*.h"
    "${libcarla_source_path}/test/*.cpp")

link_directories(
    ${RPCLIB_LIB_PATH}
    ${GTEST_LIB_PATH})

# Create targets for debug and release in the same build type.
foreach(target libcarla_test_debug libcarla_test_release)
  add_executable(${target} ${libcarla_test_sources})

  target_compile_definitions(${target} PUBLIC
      -DLIBCARLA_ENABLE_PROFILER
      -DLIBCARLA_WITH_GTEST)

  target_include_directories(${target} PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}"
      "${GTEST_INCLUDE_PATH}")
  target_link_libraries(${target} "-lrpc -lgtest_main -lgtest")

  install(TARGETS ${target} DESTINATION test)
endforeach(target)

# Specific options for debug.
set_target_properties(libcarla_test_debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
target_compile_definitions(libcarla_test_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

# Specific options for release.
set_target_properties(libcarla_test_release PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
