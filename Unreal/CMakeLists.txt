project (
  carla-unreal
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    CXX
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
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
  CARLA_UE_CARLA_PATH
  ${CARLA_UE_PLUGINS_PATH}/Carla
)

set (
  CARLA_UE_CARLA_TOOLS_PATH
  ${CARLA_UE_PLUGINS_PATH}/CarlaTools
)

set (
  CARLA_UE_PROJECT_PATH
  ${CARLA_UE_PATH}/CarlaUnreal.uproject
)

if (ENABLE_ROS2)
  set (LAUNCH_ARGS ${LAUNCH_ARGS} --ros2)
endif()

if (WIN32)
  set (EXE_EXT .exe)
  set (UE_SYSTEM_NAME Win64)
elseif (LINUX)
  set (EXE_EXT)
  set (UE_SYSTEM_NAME Linux)
elseif (APPLE)
  set (EXE_EXT)
  set (UE_SYSTEM_NAME Mac)
else ()
  carla_error ("Unknown target system.")
endif ()

set (
  UNREAL_EDITOR_PATH
  ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
)

if (IS_DIRECTORY ${CARLA_UE_PATH}/Content/Carla)
  carla_message ("Found CARLA Unreal Content.")
else ()
  carla_error ("Missing CARLA Unreal Content.")
endif ()



set (
  UE_DEPENDENCIES_ORDER_ONLY
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
  rpc
)

set (UE_DEPENDENCIES ${UE_DEPENDENCIES_ORDER_ONLY})

if (BUILD_CARLA_CLIENT)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY carla-client)
endif ()
if (BUILD_PYTHON_API)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY carla-python-api)
endif ()



set (
  CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/Includes.inc
)

set (
  CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/Libraries.inc
)

set (
  CARLA_UNREAL_OPTIONS_PATHS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/Options.inc
)

set (
  CARLA_UNREAL_INCLUDE_PATHS
  ${sqlite3_SOURCE_DIR}
)

foreach (UE_DEP ${UE_DEPENDENCIES})
  list (
    APPEND
    CARLA_UNREAL_INCLUDE_PATHS
    "$<TARGET_PROPERTY:${UE_DEP},INTERFACE_INCLUDE_DIRECTORIES>"
  )
endforeach ()

set (
  CARLA_UNREAL_LIBRARIES
  $<TARGET_FILE:libsqlite3>
)

foreach (UE_DEP ${UE_DEPENDENCIES})
  get_target_property (UE_DEP_TYPE ${UE_DEP} TYPE)
  if (NOT ("${UE_DEP_TYPE}" STREQUAL "INTERFACE_LIBRARY"))
    list (
      APPEND
      CARLA_UNREAL_LIBRARIES
      "$<TARGET_FILE:${UE_DEP}>"
    )
  endif ()
endforeach ()

set (UNREAL_PROJECT_OPTIONS)

if (ENABLE_ROS2)
  list (
    APPEND
    CARLA_UNREAL_OPTIONS
    "ROS2"
  )
endif()

if (ENABLE_OSM2ODR)
  list (
    APPEND
    CARLA_UNREAL_OPTIONS
    "OSM2ODR"
  )
endif()



if (WIN32)
  set (
    CARLA_UE_GENERATE_PROJECT_FILES_COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.bat
      -project=${CARLA_UE_PROJECT_PATH}
      -CMakefile
      -game
      -engine
  )
  set (
    CARLA_UE_BUILD_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/Build.bat
  )
else ()
  set (
    CARLA_UE_GENERATE_PROJECT_FILES_COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/GenerateProjectFiles.sh
    -project=${CARLA_UE_PROJECT_PATH}
    -CMakefile
    -game
    -engine
  )
  set (
    CARLA_UE_BUILD_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/${UE_SYSTEM_NAME}/Build.sh
  )
endif ()



if (CMAKE_TOOLCHAIN_FILE)
  cmake_path (
    ABSOLUTE_PATH
      CMAKE_TOOLCHAIN_FILE
    BASE_DIRECTORY
      ${CARLA_WORKSPACE_PATH}
    NORMALIZE
    OUTPUT_VARIABLE
      TOOLCHAIN_FILE
  )
  set (
    TOOLCHAIN_FILE_OPTION
    --toolchain=${TOOLCHAIN_FILE}
  )
else ()
  set (TOOLCHAIN_FILE_OPTION)
endif ()



set (
  CARLA_UNREAL_CONFIGURE_OUTPUTS
    ${CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/Includes.def
    ${CARLA_UE_CARLA_TOOLS_PATH}/Includes.def
    ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/Libraries.def
    ${CARLA_UE_CARLA_TOOLS_PATH}/Libraries.def
    ${CARLA_UNREAL_OPTIONS_PATHS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/Options.def
    ${CARLA_UE_CARLA_TOOLS_PATH}/Options.def
)



file (
  GENERATE
  OUTPUT
    ${CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH}
  CONTENT
    "${CARLA_UNREAL_INCLUDE_PATHS}"
)

file (
  GENERATE
  OUTPUT
    ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH}
  CONTENT
    "${CARLA_UNREAL_LIBRARIES}"
)

file (
  GENERATE
  OUTPUT
    ${CARLA_UNREAL_OPTIONS_PATHS_FILE_PATH}
  CONTENT
    "${CARLA_UNREAL_OPTIONS}"
)

add_custom_command (
  OUTPUT
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH} ${CARLA_UE_CARLA_PATH}/Includes.def
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH} ${CARLA_UE_CARLA_TOOLS_PATH}/Includes.def
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH} ${CARLA_UE_CARLA_PATH}/Libraries.def
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH} ${CARLA_UE_CARLA_TOOLS_PATH}/Libraries.def
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_OPTIONS_PATHS_FILE_PATH} ${CARLA_UE_CARLA_PATH}/Options.def
  COMMAND ${CMAKE_COMMAND} ARGS -E create_symlink ${CARLA_UNREAL_OPTIONS_PATHS_FILE_PATH} ${CARLA_UE_CARLA_TOOLS_PATH}/Options.def
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-unreal-configure
  DEPENDS
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
)



file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)



add_custom_target (
  CarlaUnreal
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX} 
    CarlaUnreal
    ${UE_SYSTEM_NAME}
    ${CARLA_UNREAL_BUILD_TYPE}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -buildscw
  DEPENDS
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  CarlaUnreal
  ${UE_DEPENDENCIES_ORDER_ONLY}
)

add_custom_target (
  CarlaUnrealEditor
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX} 
    CarlaUnrealEditor
    ${UE_SYSTEM_NAME}
    ${CARLA_UNREAL_BUILD_TYPE}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -buildscw
  DEPENDS
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  CarlaUnrealEditor
  ${UE_DEPENDENCIES_ORDER_ONLY}
)



set (
  CARLA_LAUNCH_TARGET_OPTIONS
  COMMENT
    "Launching Carla Unreal..."
  COMMAND
    ${UNREAL_EDITOR_PATH}
      ${CARLA_UE_PROJECT_PATH}
      -${CARLA_UNREAL_RHI}
      ${LAUNCH_ARGS}
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  launch
  ${CARLA_LAUNCH_TARGET_OPTIONS}
)

add_dependencies (
  launch
  CarlaUnrealEditor
)

add_custom_target (
  launch-only
  ${CARLA_LAUNCH_TARGET_OPTIONS}
)
