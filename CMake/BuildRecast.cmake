include (Util)

carla_string_option (
  CARLA_RECAST_TAG
  "Target recastnavigation git tag."
  carla
)

carla_declare_git_dependency (
  recastnavigation
  https://github.com/carla-simulator/recastnavigation.git
  ${CARLA_RECAST_TAG}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DRECASTNAVIGATION_DEMO=OFF
    -DRECASTNAVIGATION_TESTS=OFF
    -DRECASTNAVIGATION_EXAMPLES=OFF
)
