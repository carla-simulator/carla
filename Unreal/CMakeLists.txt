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

add_custom_command (
  OUTPUT ${CARLA_UE_PATH}/CMakeLists.txt
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
  carla-ue-generate-project-files
  DEPENDS ${CARLA_UE_PATH}/CMakeLists.txt
)

add_custom_command (
  OUTPUT carla-ue-configure.stamp
  COMMAND
    ${CMAKE_COMMAND} -E touch carla-ue-configure.stamp
  COMMENT
    "Configuring Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND}
    -S ${CARLA_UE_PATH}
    -G ${CMAKE_GENERATOR}
    -B ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target (
  carla-ue-configure
  DEPENDS
    carla-ue-configure.stamp
    carla-ue-generate-project-files
)

add_custom_command (
  OUTPUT carla-ue.stamp
  VERBATIM
  COMMAND
    ${CMAKE_COMMAND} -E touch carla-ue.stamp
  COMMENT
    "Building Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND}
      --build ${CMAKE_CURRENT_BINARY_DIR}
      --target CarlaUnrealEditor
      --verbose
)

add_custom_target (
  carla-ue
  ALL
  DEPENDS
    carla-ue-configure
    carla-ue.stamp
)

file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)