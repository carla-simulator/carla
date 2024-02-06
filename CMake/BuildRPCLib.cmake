include (Util)

carla_string_option (
  CARLA_RPCLIB_TAG
  "Target rpclib git tag."
  carla
)

carla_declare_git_dependency (
  rpclib
  https://github.com/carla-simulator/rpclib.git
  ${CARLA_RPCLIB_TAG}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DRPCLIB_BUILD_TESTS=OFF
    -DRPCLIB_GENERATE_COMPDB=OFF
    -DRPCLIB_BUILD_EXAMPLES=OFF
    -DRPCLIB_ENABLE_LOGGING=OFF
    -DRPCLIB_ENABLE_COVERAGE=OFF
    -DRPCLIB_MSVC_STATIC_RUNTIME=OFF
)
