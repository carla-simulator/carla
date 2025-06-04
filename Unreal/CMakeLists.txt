project (
  carla-unreal
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    CXX
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
)

carla_option (
  ENABLE_DIRECTORY_CLEAN
  "Enable ADDITIONAL_CLEAN_FILES on directory level"
  ON
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
  CARLA_UE_CARLA_EXPORTER_PATH
  ${CARLA_UE_PLUGINS_PATH}/CarlaExporter
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

set (
  UNREAL_EDITOR_PATH
  ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
)

set (CARLA_UE_CONTENT_CHECK_PATH ${CARLA_UE_PATH}/Content/Carla/Blueprints)
if (IS_DIRECTORY ${CARLA_UE_CONTENT_CHECK_PATH})
  carla_message_verbose ("CARLA Unreal Content - FOUND")
else ()
  carla_warning ("Could not find CARLA Content, download it or running the editor will result in a crash.")
endif ()



set (
  UE_DEPENDENCIES
  carla-server
  libsqlite3
  Boost::asio
  Boost::python
  Boost::geometry
  Boost::gil
  Eigen3::Eigen
  zlibstatic
  png_static
  rpc
)

set (UE_DEPENDENCIES_ORDER_ONLY ${UE_DEPENDENCIES})

if (BUILD_CARLA_CLIENT)
  list (APPEND UE_DEPENDENCIES_ORDER_ONLY carla-client)
endif ()

list (
  APPEND
  UE_DEPENDENCIES_ORDER_ONLY
  RecastBuilder
)



set (
  CARLA_UNREAL_INCLUDE_PATHS_FILE_NAME
  Includes.def
)

set (
  CARLA_UNREAL_LIBRARIES_PATHS_FILE_NAME
  Libraries.def
)

set (
  CARLA_UNREAL_OPTIONS_FILE_NAME
  Options.def
)

set (
  CARLA_UNREAL_DEFINITIONS_FILE_NAME
  Definitions.def
)

set (
  CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_INCLUDE_PATHS_FILE_NAME}
)

set (
  CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_LIBRARIES_PATHS_FILE_NAME}
)

set (
  CARLA_UNREAL_OPTIONS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_OPTIONS_FILE_NAME}
)

set (
  CARLA_UNREAL_DEFINITIONS_FILE_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/${CARLA_UNREAL_DEFINITIONS_FILE_NAME}
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

set (
  CARLA_UNREAL_DEFINITIONS
  RECASTBUILDER_PATH="$<TARGET_FILE:RecastBuilder>"
)

set (CARLA_UNREAL_OPTIONS)

if (ENABLE_ROS2)
  list (
    APPEND
    CARLA_UNREAL_OPTIONS
    "ROS2"
  )
endif()

if (ENABLE_ROS2_DEMO)
  list (
    APPEND
    CARLA_UNREAL_OPTIONS
    "ROS2_DEMO"
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
    CARLA_UE_BUILD_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/Build.bat
  )
  set (
    CARLA_UE_UAT_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/RunUAT.bat
  )
else ()
  set (
    CARLA_UE_BUILD_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/${UE_SYSTEM_NAME}/Build.sh
  )
  set (
    CARLA_UE_UAT_COMMAND_PREFIX
      ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/BatchFiles/RunUAT.sh
  )
endif ()



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
    ${CARLA_UNREAL_OPTIONS_FILE_PATH}
  CONTENT
    "${CARLA_UNREAL_OPTIONS}"
)

file (
  GENERATE
  OUTPUT
    ${CARLA_UNREAL_DEFINITIONS_FILE_PATH}
  CONTENT
    "${CARLA_UNREAL_DEFINITIONS}"
)



set (
  CARLA_UNREAL_CONFIGURE_OUTPUTS
    ${CARLA_UNREAL_INCLUDE_PATHS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_INCLUDE_PATHS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_INCLUDE_PATHS_FILE_NAME}
    ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_LIBRARIES_PATHS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_LIBRARIES_PATHS_FILE_NAME}
    ${CARLA_UNREAL_OPTIONS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_OPTIONS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_OPTIONS_FILE_NAME}
    ${CARLA_UNREAL_DEFINITIONS_FILE_PATH}
    ${CARLA_UE_CARLA_PATH}/${CARLA_UNREAL_DEFINITIONS_FILE_NAME}
    ${CARLA_UE_CARLA_TOOLS_PATH}/${CARLA_UNREAL_DEFINITIONS_FILE_NAME}
)

set (CARLA_UNREAL_CONFIGURE_COMMANDS)

set (
  CARLA_UNREAL_DEF_FILES
  ${CARLA_UNREAL_INCLUDE_PATHS_FILE_NAME}
  ${CARLA_UNREAL_LIBRARIES_PATHS_FILE_NAME}
  ${CARLA_UNREAL_OPTIONS_FILE_NAME}
  ${CARLA_UNREAL_DEFINITIONS_FILE_NAME}
)

foreach (DEF_FILE ${CARLA_UNREAL_DEF_FILES})
  list (
    APPEND
    CARLA_UNREAL_CONFIGURE_COMMANDS
    COMMAND
      ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${DEF_FILE}
      ${CARLA_UE_CARLA_PATH}/${DEF_FILE}
    COMMAND
      ${CMAKE_COMMAND}
    ARGS
      -E create_symlink
      ${CMAKE_CURRENT_BINARY_DIR}/${DEF_FILE}
      ${CARLA_UE_CARLA_TOOLS_PATH}/${DEF_FILE}
  )
endforeach ()

add_custom_command (
  OUTPUT
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  ${CARLA_UNREAL_CONFIGURE_COMMANDS}
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-unreal-configure
  DEPENDS
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
)

list (
  APPEND
  UE_DEPENDENCIES_ORDER_ONLY
  carla-unreal-configure
)



file (
  MAKE_DIRECTORY
  ${CARLA_UE_PATH}/Content/Carla/ExportedMaps
)



carla_add_custom_target (
  carla-unreal
  "Build the CarlaUnreal subproject."
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX}
    CarlaUnreal
    ${UE_SYSTEM_NAME}
    ${CARLA_UNREAL_BUILD_TYPE}
    -project=${CARLA_UE_PROJECT_PATH}
    -game
    -buildscw
  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  carla-unreal
  ${UE_DEPENDENCIES_ORDER_ONLY}
)

carla_add_custom_target (
  carla-unreal-editor
  "Build the CarlaUnrealEditor subproject."
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
  carla-unreal
  ${UE_DEPENDENCIES_ORDER_ONLY}
)



function (
  add_carla_ue_package_target
  PACKAGE_CONFIGURATION
  UE_BUILD_CONFIGURATION)

  set (
    CARLA_PACKAGE_NAME
    Carla-${CARLA_VERSION}-${UE_SYSTEM_NAME}-${UE_BUILD_CONFIGURATION}
  )

  set (
    CARLA_CURRENT_PACKAGE_PATH
    ${CARLA_PACKAGE_PATH}/${CARLA_PACKAGE_NAME}
  )

  set (
    CARLA_CURRENT_PACKAGE_ARCHIVE_PATH
    ${CARLA_PACKAGE_PATH}/${CARLA_PACKAGE_NAME}
  )

  set (
    CARLA_PACKAGE_STAGING_PATH
    ${CARLA_PACKAGE_PATH}/StagedBuilds/${CARLA_PACKAGE_NAME}
  )
  
  set (
    CARLA_PACKAGE_ARCHIVE_PATH
    ${CARLA_CURRENT_PACKAGE_PATH}
  )
  
  if (NOT "${PACKAGE_CONFIGURATION}" STREQUAL "")
    set (TARGET_NAME_SUFFIX -${PACKAGE_CONFIGURATION})
    string (TOLOWER "${TARGET_NAME_SUFFIX}" TARGET_NAME_SUFFIX)
  else ()
    set (PACKAGE_CONFIGURATION "default (shipping)")
  endif ()

  set (TARGET_NAME_SHORT package${TARGET_NAME_SUFFIX})
  set (TARGET_NAME carla-unreal-${TARGET_NAME_SHORT})

  carla_add_custom_target (
    ${TARGET_NAME}
    "Create a CARLA package in ${PACKAGE_CONFIGURATION} mode."
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
      -stagingdirectory=${CARLA_PACKAGE_STAGING_PATH}
      -archivedirectory=${CARLA_PACKAGE_ARCHIVE_PATH}
    USES_TERMINAL
    VERBATIM
  )
  
  add_custom_command (
    TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE CREATING VERSION FILE UNREAL PACKAGE EXTRA FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_WORKSPACE_PATH=${CARLA_WORKSPACE_PATH}
      -DCARLA_UNREAL_ENGINE_PATH=${CARLA_UNREAL_ENGINE_PATH}
      -DCARLA_PACKAGE_VERSION_FILE=${CARLA_PACKAGE_ARCHIVE_PATH}/VERSION
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/CreateCarlaVersionFile.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE VERSION FILE UNREAL PACKAGE EXTRA FILES COMPLETED **********"

    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE REMOVING UNREAL PACKAGE EXTRA FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_PACKAGE_ARCHIVE_PATH=${CARLA_PACKAGE_ARCHIVE_PATH}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/RemoveUnrealPackageExtraFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE REMOVING UNREAL PACKAGE EXTRA FILES COMPLETED **********"

    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_BUILD_PATH=${CARLA_BUILD_PATH}
      -DCARLA_WORKSPACE_PATH=${CARLA_WORKSPACE_PATH}
      -DCARLA_PACKAGE_ARCHIVE_PATH=${CARLA_PACKAGE_ARCHIVE_PATH}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/CopyCarlaAdditionalFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES COMPLETED **********"

    COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_PACKAGE_PATH=${CARLA_PACKAGE_PATH}
      -DCARLA_PACKAGE_ARCHIVE_PATH=${CARLA_PACKAGE_ARCHIVE_PATH}
      -DCARLA_CURRENT_PACKAGE_PATH=${CARLA_CURRENT_PACKAGE_PATH}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/Compress.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE COMPLETED **********"
    USES_TERMINAL
    VERBATIM
  )

  add_dependencies (
    ${TARGET_NAME}
    carla-python-api
    carla-unreal-editor
  )

  carla_add_custom_target (
    ${TARGET_NAME_SHORT}
    "Create a CARLA package in ${PACKAGE_CONFIGURATION} mode."
  )

  add_dependencies (
    ${TARGET_NAME_SHORT}
    ${TARGET_NAME}
  )

endfunction()



# Docs for UE5 build configurations:
# https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/DevelopmentSetup/BuildConfigurations/

add_carla_ue_package_target("" ${CARLA_UNREAL_PACKAGE_BUILD_TYPE})
add_carla_ue_package_target(Shipping Shipping)
add_carla_ue_package_target(Debug Debug)
add_carla_ue_package_target(DebugGame DebugGame)
add_carla_ue_package_target(Development Development)
add_carla_ue_package_target(Test Test)



set (LAUNCH_ARGS ${CARLA_LAUNCH_ARGS})

if (ENABLE_ROS2)
  list (APPEND LAUNCH_ARGS --ros2)
endif()

if (ENABLE_ROS2_DEMO)
  list (APPEND LAUNCH_ARGS --ros2-demo)
endif()

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

carla_add_custom_target (
  check-unreal-content
  "Perform some basic checks to ensure that the CARLA Unreal Editor will not open without its assets."
  COMMAND
    ${PRELAUNCH_CONTENT_CHECK_COMMAND}
  COMMENT
    "Performing prelaunch check for CARLA Content presence."
  USES_TERMINAL
  VERBATIM
)

carla_add_custom_target (
  launch-only
  "Open CARLA in the Unreal Editor. This will not rebuild outdated targets."
  ${CARLA_LAUNCH_TARGET_OPTIONS}
)

add_dependencies (
  launch-only
  check-unreal-content
)

carla_add_custom_target (
  launch
  "Build and open CARLA in the Unreal Editor."
  ${CARLA_LAUNCH_TARGET_OPTIONS}
)

add_dependencies (
  launch
  carla-unreal-editor
  check-unreal-content
)

set_property (
  DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
  "${CARLA_UE_PATH}/Binaries"
  "${CARLA_UE_PATH}/Build"
  "${CARLA_UE_PATH}/DerivedDataCache"
  "${CARLA_UE_PATH}/Intermediate"
  "${CARLA_UE_PATH}/Saved"
  "${CARLA_UE_CARLA_TOOLS_PATH}/Binaries"
  "${CARLA_UE_CARLA_TOOLS_PATH}/Intermediate"
  "${CARLA_UE_CARLA_TOOLS_PATH}/Saved"
  "${CARLA_UE_CARLA_EXPORTER_PATH}/Binaries"
  "${CARLA_UE_CARLA_EXPORTER_PATH}/Intermediate"
  "${CARLA_UE_CARLA_EXPORTER_PATH}/Saved"
  "${CARLA_UE_CARLA_PATH}/Binaries"
  "${CARLA_UE_CARLA_PATH}/Intermediate"
  "${CARLA_UE_CARLA_PATH}/Saved"
  "${CARLA_UE_PLUGINS_PATH}/StreetMap/Binaries"
  "${CARLA_UE_PLUGINS_PATH}/StreetMap/Intermediate"
  "${CARLA_UE_PLUGINS_PATH}/StreetMap/Saved"
  "${CARLA_BUILD_PATH}/Package"
  "${CARLA_BUILD_PATH}/PythonAPI/dist"
)
