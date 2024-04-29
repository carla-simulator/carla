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

set (
  CARLA_UE_CONTENT_PATH
  ${CARLA_UE_PATH}/Content/Carla
)

if (NOT EXISTS ${CARLA_UE_CONTENT_PATH}/.git)
  carla_error ("Missing CARLA content.")
endif ()

if (ENABLE_ROS2)
  set (LAUNCH_ARGS ${LAUNCH_ARGS} --ros2)
endif()

if (WIN32)
  set (UE_SYSTEM_NAME Win64)
elseif (LINUX)
  set (UE_SYSTEM_NAME Linux)
elseif (APPLE)
  set (UE_SYSTEM_NAME Mac)
else ()
  carla_error ("Unknown target system.")
endif ()

set (
  UNREAL_EDITOR_PATH
  ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
)

set (CARLA_UE_CONTENT_CHECK_PATH ${CARLA_UE_PATH}/Content/Carla/Blueprints)
if (IS_DIRECTORY ${CARLA_UE_CONTENT_CHECK_PATH})
  carla_message ("CARLA Unreal Content - FOUND")
else ()
  carla_message ("CARLA Unreal Content - MISSING")
  carla_warning ("Could not find CARLA Content, download it or running the editor will result in a crash.")
endif ()



set (
  UE_DEPENDENCIES_ORDER_ONLY
  carla-server
  libsqlite3
  Boost::asio
  Boost::geometry
  Boost::gil
  Eigen3::Eigen
  zlibstatic
  png_static
  rpc
)

if (BUILD_PYTHON_API)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY Boost::python)
endif ()

set (UE_DEPENDENCIES ${UE_DEPENDENCIES_ORDER_ONLY})

if (BUILD_CARLA_CLIENT)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY carla-client)
endif ()
if (BUILD_PYTHON_API)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY carla-python-api)
endif ()



set (
  CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME
  PublicIncludePaths.def
)

set (
  CARLA_UNREAL_PUBLIC_INCLUDE_PATHS
  # These will be added to PublicIncludePaths.def
  ${sqlite3_SOURCE_DIR}
)

foreach (UE_DEP ${UE_DEPENDENCIES})
  list (
    APPEND
    CARLA_UNREAL_PUBLIC_INCLUDE_PATHS
    "$<TARGET_PROPERTY:${UE_DEP},INTERFACE_INCLUDE_DIRECTORIES>"
  )
endforeach ()



set (
  CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME
  PublicAdditionalLibraries.def
)

set (
  CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES
  # These will be added to PublicAdditionalLibraries.def
  $<TARGET_FILE:libsqlite3>
)

foreach (UE_DEP ${UE_DEPENDENCIES})
  get_target_property (UE_DEP_TYPE ${UE_DEP} TYPE)
  if (NOT ("${UE_DEP_TYPE}" STREQUAL "INTERFACE_LIBRARY"))
    list (
      APPEND
      CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES
      "$<TARGET_FILE:${UE_DEP}>"
    )
  endif ()
endforeach ()



set (
  CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME
  PublicDefinitions.def
)

set (
  CARLA_UNREAL_PUBLIC_DEFINITIONS
  # These will be added to PublicDefinitions.def
)

if (ENABLE_ROS2)
  list (
    APPEND
    CARLA_UNREAL_PUBLIC_DEFINITIONS
    "WITH_ROS2"
  )
endif()

if (ENABLE_OSM2ODR)
  list (
    APPEND
    CARLA_UNREAL_PUBLIC_DEFINITIONS
    "WITH_OSM2ODR"
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
  set (
    CARLA_UE_UAT_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/RunUAT.bat
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
  set (
    CARLA_UE_UAT_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/RunUAT.sh
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
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
)



file (
  GENERATE
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
  CONTENT
    "${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS}"
)

file (
  GENERATE
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
  CONTENT
    "${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES}"
)

file (
  GENERATE
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
  CONTENT
    "${CARLA_UNREAL_PUBLIC_DEFINITIONS}"
)

add_custom_command (
  OUTPUT
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
      ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
      ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_INCLUDE_PATHS_FILE_NAME}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
      ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
      ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_ADDITIONAL_LIBRARIES_FILE_NAME}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
      ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
      ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_PUBLIC_DEFINITIONS_FILE_NAME}
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
  carla-unreal
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
  carla-unreal
  ${UE_DEPENDENCIES_ORDER_ONLY}
)

add_custom_target (
  carla-unreal-editor
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
  carla-unreal-editor
  ${UE_DEPENDENCIES_ORDER_ONLY}
)

function(add_carla_ue_package_target TARGET_NAME_SUFFIX UE_BUILD_CONFIGURATION)
  add_custom_target (
    carla-unreal-package${TARGET_NAME_SUFFIX}
    COMMAND
      ${CARLA_UE_BUILD_COMMAND_PREFIX}
      CarlaUnreal
      ${UE_SYSTEM_NAME}
      ${UE_BUILD_CONFIGURATION}
      -project=${CARLA_UE_PROJECT_PATH}
      -game
      -buildscw
    COMMAND
      ${CARLA_UE_UAT_COMMAND_PREFIX}
      BuildCookRun
      -project=${CARLA_UE_PROJECT_PATH}
      -nocompileeditor
      -nop4
      -cook
      -stage
      -archive
      -package
      -iterate
      -clientconfig=${UE_BUILD_CONFIGURATION}
      -TargetPlatform=${UE_SYSTEM_NAME}
      -Platform=${UE_SYSTEM_NAME}
      -prereqs
      -build
      -archivedirectory=${CARLA_PACKAGE_PATH}
    DEPENDS
      ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
      USES_TERMINAL
      VERBATIM
    )

  set(CARLA_TARGET_PACKAGE_PATH ${CARLA_PACKAGE_PATH}/${UE_SYSTEM_NAME})
  add_custom_command(
    TARGET carla-unreal-package${TARGET_NAME_SUFFIX}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE REMOVING UNREAL PACKAGE EXTRA FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_TARGET_PACKAGE_PATH=${CARLA_TARGET_PACKAGE_PATH}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/RemoveUnrealPackageExtraFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE REMOVING UNREAL PACKAGE EXTRA FILES COMPLETED **********"

    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_SOURCE_DIR=${CARLA_SOURCE_DIR}
      -DCARLA_TARGET_PACKAGE_PATH=${CARLA_TARGET_PACKAGE_PATH}
      -DCARLA_BINARY_DIR=${CARLA_BINARY_DIR}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/CopyCarlaAdditionalFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES COMPLETED **********"

    COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_PACKAGE_PATH=${CARLA_PACKAGE_PATH}
      -DCARLA_TARGET_PACKAGE_PATH=${CARLA_TARGET_PACKAGE_PATH}
      -DTARGET_NAME_SUFFIX=${TARGET_NAME_SUFFIX}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/Compress.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE COMPLETED **********"
  )

  add_dependencies (
    carla-unreal-package${TARGET_NAME_SUFFIX}
    ${UE_DEPENDENCIES_ORDER_ONLY}
  )

  add_custom_target (
    package${TARGET_NAME_SUFFIX}
  )

  add_dependencies (
    package${TARGET_NAME_SUFFIX}
    carla-unreal-package${TARGET_NAME_SUFFIX}
  )
endfunction()

# Docs for UE5 build configurations:
# https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/DevelopmentSetup/BuildConfigurations/
add_carla_ue_package_target("" Shipping)
add_carla_ue_package_target(-Shipping Shipping)
add_carla_ue_package_target(-Debug Debug)
add_carla_ue_package_target(-DebugGame DebugGame)
add_carla_ue_package_target(-Development Development)
add_carla_ue_package_target(-Test Test)



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

if (WIN32)
  set (
    PRELAUNCH_CONTENT_CHECK_COMMAND
    if exist ${CARLA_UE_CONTENT_CHECK_PATH} (exit 0) else (exit 1)
  )
else ()
  set (
    PRELAUNCH_CONTENT_CHECK_COMMAND
    test -d ${CARLA_UE_CONTENT_CHECK_PATH}
  )
endif ()

add_custom_target (
  prelaunch-content-check
  COMMAND
    ${PRELAUNCH_CONTENT_CHECK_COMMAND}
  COMMENT
    "Performing prelaunch check for CARLA Content presence."
  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  launch
  carla-unreal-editor
  prelaunch-content-check
)

add_custom_target (
  launch-only
  ${CARLA_LAUNCH_TARGET_OPTIONS}
)

add_dependencies (
  launch-only
  prelaunch-content-check
)
