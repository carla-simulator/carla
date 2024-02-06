include (Util)

carla_string_option (
  CARLA_ZLIB_VERSION
  "Target zlib git tag."
  1.3.1)

carla_string_option (
  CARLA_ZLIB_TAG
  "Target zlib git tag."
  v${CARLA_ZLIB_VERSION})

carla_declare_git_dependency (
  zlib
  https://github.com/madler/zlib.git
  ${CARLA_ZLIB_TAG}
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DBUILD_SHARED_LIBS=OFF
)
