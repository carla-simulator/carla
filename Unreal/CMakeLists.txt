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

if (ENABLE_ROS2)
  set (LAUNCH_ARGS ${LAUNCH_ARGS} --ros2)
endif()

if (WIN32)
  set (
    CARLA_UE_GENERATE_PROJECT_FILES_COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.bat
    -project=${CARLA_UE_PROJECT_PATH}
    -CMakefile
    -game
    -engine
    -progress
  )
else ()
  set (
    CARLA_UE_GENERATE_PROJECT_FILES_COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.sh
    -project=${CARLA_UE_PROJECT_PATH}
    -CMakefile
    -game
    -engine
    -progress
  )
endif ()

set (CMAKE_NINJA_FORCE_RESPONSE_FILE ON)



add_custom_command (
  OUTPUT
    ${CARLA_UE_PATH}/CMakeLists.txt
  COMMENT
    "Generating UE project files for CARLA..."
  COMMAND
    ${CARLA_UE_GENERATE_PROJECT_FILES_COMMAND}
  # Remember to add this on most Unreal-related commands,
  # otherwise nothing is printed while running the command:
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-ue-generate-project-files
  DEPENDS
    ${CARLA_UE_PATH}/CMakeLists.txt
)

add_dependencies (
  carla-ue-generate-project-files
  carla-server
  libsqlite3
  Boost::asio
  Boost::python
  Boost::numpy
  Boost::geometry
  Boost::gil
  Eigen3::Eigen
  zlibstatic
  png_static
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
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/carla-ue-configure.stamp
  COMMENT
    "Configuring Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND}
      -S ${CARLA_UE_PATH}
      -B ${CMAKE_CURRENT_BINARY_DIR}
      ${TOOLCHAIN_FILE_OPTION}
  COMMAND
    ${CMAKE_COMMAND}
      -E touch carla-ue-configure.stamp
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-ue-configure
  DEPENDS
    carla-ue-configure.stamp
)

add_dependencies (
  carla-ue-configure
  carla-ue-generate-project-files
)



add_custom_command (
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/carla-ue.stamp
  COMMENT
    "Building Carla Unreal..."
  COMMAND
    ${CMAKE_COMMAND}
      --build ${CMAKE_CURRENT_BINARY_DIR}
      --target CarlaUnrealEditor
  COMMAND
    ${CMAKE_COMMAND}
      -E touch carla-ue.stamp
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-ue
  DEPENDS
    carla-ue.stamp
)

add_dependencies (
  carla-ue
  carla-ue-configure
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
  COMMENT
    "Launching Carla Unreal..."
  COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
      ${CARLA_UE_PROJECT_PATH}
      -${CARLA_UNREAL_RHI}
      ${LAUNCH_ARGS}
  USES_TERMINAL
  VERBATIM
)



add_custom_target (
  launch
  COMMENT
    "Launching Carla Unreal..."
  COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
      ${CARLA_UE_PROJECT_PATH}
      -${CARLA_UNREAL_RHI}
      ${LAUNCH_ARGS}
  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  launch
  carla-ue
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