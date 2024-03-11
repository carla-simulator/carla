project (
  carla-ue
)

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
  # Remember to add this on most Unreal-related commands,
  # otherwise nothing is printed while running the command:
  USES_TERMINAL
)

add_custom_target (
  carla-ue-generate-project-files
  DEPENDS ${CARLA_UE_PATH}/CMakeLists.txt
)



if (CMAKE_TOOLCHAIN_FILE)
  cmake_path (
    ABSOLUTE_PATH CMAKE_TOOLCHAIN_FILE
    BASE_DIRECTORY ${CARLA_WORKSPACE_PATH}
    NORMALIZE
    OUTPUT_VARIABLE TOOLCHAIN_FILE
  )
  set (TOOLCHAIN_FILE_OPTION --toolchain=${TOOLCHAIN_FILE})
else ()
  set (TOOLCHAIN_FILE_OPTION)
endif ()



add_custom_command (
  OUTPUT carla-ue-configure.stamp
  COMMENT
    "Configuring Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND} -E touch carla-ue-configure.stamp
  COMMAND
    ${CMAKE_COMMAND}
    -S ${CARLA_UE_PATH}
    -G ${CMAKE_GENERATOR}
    -B ${CMAKE_CURRENT_BINARY_DIR}
    ${TOOLCHAIN_FILE_OPTION}
  USES_TERMINAL
)

add_custom_target (
  carla-ue-configure
  DEPENDS
    carla-ue-configure.stamp
    carla-ue-generate-project-files
)



add_custom_command (
  OUTPUT carla-ue.stamp
  COMMENT
    "Building Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND} -E touch carla-ue.stamp
  COMMAND
    ${CMAKE_COMMAND}
      --build ${CMAKE_CURRENT_BINARY_DIR}
      --target CarlaUnrealEditor
      --verbose
  USES_TERMINAL
)

add_custom_target (
  carla-ue
  ALL
  DEPENDS
    carla-server
    carla-ue-configure
    carla-ue.stamp
)



if (WIN32)
  set (EXE_EXT .exe)
else ()
  set (EXE_EXT)
endif ()

if (WIN32)
  set (UE_SYSTEM_NAME Win64)
elseif (LINUX)
  set (UE_SYSTEM_NAME Linux)
else ()
endif ()

add_custom_target (
  launch-only
  COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
    ${CARLA_UE_PROJECT_PATH}
    -${CARLA_UNREAL_RHI}
    ${LAUNCH_ARGS}
  USES_TERMINAL
)



add_custom_target (
  launch
  DEPENDS
    carla-ue
    launch-only
)



file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)

carla_two_step_configure_file (
  ${CARLA_UE_PLUGINS_PATH}/Carla/Source/Carla/Carla.Build.cs
  ${CARLA_UE_PLUGINS_PATH}/Carla/Source/Carla/Carla.Build.cs.in
)

carla_two_step_configure_file (
  ${CARLA_UE_PLUGINS_PATH}/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs
  ${CARLA_UE_PLUGINS_PATH}/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs.in
)