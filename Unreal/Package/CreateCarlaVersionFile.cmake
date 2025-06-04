execute_process(
  COMMAND
    git log -1 --format=%H
  WORKING_DIRECTORY
    ${CARLA_WORKSPACE_PATH}
  OUTPUT_VARIABLE
    CARLA_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND
    git log -1 --format=%H
  WORKING_DIRECTORY
    ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla
  OUTPUT_VARIABLE
    CONTENT_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND
    git log -1 --format=%H
  WORKING_DIRECTORY
    ${CARLA_UNREAL_ENGINE_PATH}
  OUTPUT_VARIABLE
    UNREAL_ENGINE_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

file (
  WRITE
    ${CARLA_PACKAGE_VERSION_FILE}
    "Carla git hash:         ${CARLA_GIT_HASH}\n"
    "Content git hash:       ${CONTENT_GIT_HASH}\n"
    "UnrealEngine git hash:  ${UNREAL_ENGINE_GIT_HASH}\n"
)
