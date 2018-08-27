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

    if (WIN32)
        target_link_libraries(${target} "gtest_main.lib")
        target_link_libraries(${target} "gtest.lib")
        target_link_libraries(${target} "rpc.lib")
    else()
        target_link_libraries(${target} "-lrpc")
        target_link_libraries(${target} "-lgtest_main")
        target_link_libraries(${target} "-lgtest")
    endif()

    install(TARGETS ${target} DESTINATION test)
endforeach(target)

# Specific options for debug.
set_target_properties(libcarla_test_debug PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
target_link_libraries(libcarla_test_debug "carla_server_debug")
target_compile_definitions(libcarla_test_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

# Specific options for release.
set_target_properties(libcarla_test_release PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
target_link_libraries(libcarla_test_release "carla_server")
