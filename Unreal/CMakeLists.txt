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
  ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal
)

set (
  CARLA_UE_PROJECT_PATH
  ${CARLA_UE_PATH}/CarlaUnreal.uproject
)

execute_process (
  COMMAND
    ${CARLA_UE_GENERATE_PROJECT_SCRIPT}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -engine
    -makefiles
  RESULT_VARIABLE
    GENERATE_RPJECT_FILES_RESULT
)

if (NOT ${GENERATE_RPJECT_FILES_RESULT})
  message (
    FATAL_ERROR
    "Could not generate Unreal Engine project files."
  )
endif ()

file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)

add_subdirectory (
  CarlaUnreal
)