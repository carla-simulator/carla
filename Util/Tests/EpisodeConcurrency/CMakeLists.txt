# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Load with CMAKE_PROJECT_TOP_LEVEL_INCLUDES; production targets must exist first.
function(carla_add_episode_concurrency_tests)
  if(NOT TARGET carla-client OR NOT TARGET GTest::gtest_main)
    message(FATAL_ERROR "Episode tests require BUILD_CARLA_CLIENT and BUILD_LIBCARLA_TESTS")
  endif()

  enable_testing()
  add_executable(carla-episode-tests
    "${CARLA_WORKSPACE_PATH}/LibCarla/source/test/client/test_episode_concurrency.cpp")
  target_compile_features(carla-episode-tests PRIVATE cxx_std_20)
  target_compile_definitions(carla-episode-tests PRIVATE LIBCARLA_WITH_GTEST)
  target_include_directories(carla-episode-tests PRIVATE
    "${CARLA_WORKSPACE_PATH}/LibCarla/source/test"
    "${CARLA_WORKSPACE_PATH}/LibCarla/source/third-party")
  target_link_libraries(carla-episode-tests PRIVATE carla-client GTest::gtest_main)

  add_test(NAME episode_concurrency COMMAND carla-episode-tests)
  set_tests_properties(episode_concurrency PROPERTIES TIMEOUT 60)
endfunction()

cmake_language(DEFER CALL carla_add_episode_concurrency_tests)
