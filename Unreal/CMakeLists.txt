project (
  carla-ue
)

if (WIN32)
  set (
    CARLA_UE_GENERATE_PROJECT_SCRIPT
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.bat
  )
else ()
  set (
    CARLA_UE_GENERATE_PROJECT_SCRIPT
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.sh
  )
endif ()

set (
  CARLA_UE_PATH
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUE4
)

set (
  CARLA_UE_PROJECT_PATH
  ${CARLA_UE_PATH}/CarlaUE4.uproject
)

add_custom_target (
  carla-ue-generate-project-files
  ALL
  DEPENDS
    carla-server
  COMMENT
    "Generating Carla UE project files..."
  COMMAND
    ${CARLA_UE_GENERATE_PROJECT_SCRIPT}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -engine
    -makefiles
)

add_custom_target (
  carla-ue-build
  ALL
  DEPENDS
    carla-ue-generate-project-files
  COMMENT
    "Building Carla UE..."
  COMMAND
    make -C ${CARLA_UE_PROJECT_PATH} CarlaUE4Editor
)

file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)