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
)
