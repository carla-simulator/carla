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
  CARLA_UE_PLUGINS_PATH
  ${CARLA_UE_PATH}/Plugins
)

set (
  CARLA_UE_PROJECT_PATH
  ${CARLA_UE_PATH}/CarlaUnreal.uproject
)

carla_two_step_configure_file (
  ${CARLA_UE_PLUGINS_PATH}/Carla/Source/Carla/Carla.Build.cs
  ${CARLA_UE_PLUGINS_PATH}/Carla/Source/Carla/Carla.Build.cs.in
)

carla_two_step_configure_file (
  ${CARLA_UE_PLUGINS_PATH}/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs
  ${CARLA_UE_PLUGINS_PATH}/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs.in
)

add_custom_target (
  carla-ue-generate-project-files
  ALL
  COMMENT
    "Generating UE project files for CARLA..."
  COMMAND
    ${CARLA_UE_GENERATE_PROJECT_SCRIPT}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -engine
    -CMakefile
)

add_custom_target (
  carla-ue-build
  ALL
  DEPENDS
    carla-ue-generate-project-files
  COMMENT
    "Building CARLA UE..."
  COMMAND
    ${CMAKE_COMMAND}
      -S ${CARLA_UE_PATH}
      -B ${CMAKE_CURRENT_BINARY_DIR}
      -G ${CMAKE_GENERATOR}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)