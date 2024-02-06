include (Util)

carla_string_option (
  CARLA_GTEST_VERSION
  "Target googletest version."
  1.14.0
)

carla_string_option (
  CARLA_GTEST_TAG
  "Target googletest git tag."
  v${CARLA_GTEST_VERSION}
)

carla_declare_git_dependency (
  gtest
  https://github.com/google/googletest.git
  ${CARLA_GTEST_TAG}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DBUILD_GMOCK=OFF
    -DGTEST_HAS_ABSL=OFF
)
