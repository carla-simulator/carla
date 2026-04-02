project (
  carla-unreal
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    CXX
  DESCRIPTION
    "Open-source simulator for autonomous driving research."
)

option (
  ENABLE_DIRECTORY_CLEAN
  "Enable ADDITIONAL_CLEAN_FILES on directory level"
  ON
)

set (
  CARLA_PACKAGE_PATH
  ${CMAKE_BINARY_DIR}/Package
)

set (
  CARLA_UE_PATH
  ${CARLA_ROOT}/Unreal/CarlaUnreal
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

set (
  CARLA_UE_CONTENT_ORIGIN_URL
  https://bitbucket.org/carla-simulator/carla-content.git
)

cmake_path (
  CONVERT
    "$ENV{CARLA_UNREAL_ENGINE_PATH}"
  TO_CMAKE_PATH_LIST
    CARLA_UNREAL_ENGINE_PATH_INFERRED
  NORMALIZE
)

set (
  CARLA_UNREAL_ENGINE_PATH
  "${CARLA_UNREAL_ENGINE_PATH_INFERRED}"
  CACHE FILEPATH
  "Path to the CARLA fork of Unreal Engine."
)

message (STATUS "Using ${CARLA_UNREAL_ENGINE_PATH} as Unreal Engine root path.")

if (EXISTS ${CARLA_UNREAL_ENGINE_PATH})
  set (CARLA_HAS_UNREAL_ENGINE_PATH ON)
else ()
  set (CARLA_HAS_UNREAL_ENGINE_PATH OFF)
endif ()

option (BUILD_CARLA_UNREAL "Build the Carla Unreal Engine project." ${CARLA_HAS_UNREAL_ENGINE_PATH})
if (WIN32)
  set (CARLA_UNREAL_RHI_DEFAULT d3d12)
elseif (LINUX)
  set (CARLA_UNREAL_RHI_DEFAULT vulkan)
endif ()

set (
  CARLA_UNREAL_RHI
  "${CARLA_UNREAL_RHI_DEFAULT}"
  CACHE STRING
  "Target CARLA Unreal Engine RHI."
)

if (BUILD_CARLA_UNREAL)
  if (NOT ${CARLA_HAS_UNREAL_ENGINE_PATH})
    message (FATAL_ERROR 
      "Could not add UE project to build since the option CARLA_UNREAL_ENGINE_PATH "
      "is not set to a valid path (\"${CARLA_UNREAL_ENGINE_PATH}\")."
      "Please set it to point to the root path of your CARLA Unreal Engine installation."
    )
  endif ()
endif ()

# Docs for UE5 build configurations:
# https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/DevelopmentSetup/BuildConfigurations/

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set (CARLA_UNREAL_BUILD_TYPE Debug)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  set (CARLA_UNREAL_BUILD_TYPE Development)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set (CARLA_UNREAL_BUILD_TYPE Shipping)
else ()
  message (WARNING "Unexpected CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\". Unreal packages will default to Development. Manually override DEFAULT_PACKAGE_CONFIGURATION if this behavior is not desired.")
  set (CARLA_UNREAL_BUILD_TYPE Development)
endif ()

set (
  CARLA_UNREAL_BUILD_TYPE
  "${CARLA_UNREAL_BUILD_TYPE}"
  CACHE STRING
  "Override the CARLA Unreal build configuration."
)

set (CARLA_UNREAL_PACKAGE_NO_COMPRESSION_DEFAULT OFF)
if ("${CARLA_UNREAL_BUILD_TYPE}" STREQUAL "Shipping")
  set (CARLA_UNREAL_PACKAGE_NO_COMPRESSION_DEFAULT ON)
endif ()

option (CARLA_UNREAL_PACKAGE_NO_COMPRESSION "Whether to skip the package compression step." ${CARLA_UNREAL_PACKAGE_NO_COMPRESSION_DEFAULT})

set (
  CARLA_ADDITIONAL_CONTENT_SEARCH_PATHS
  ""
  CACHE STRING
  "Comma-separated list of additional paths to look for a cloned CARLA Unreal Engine Content repo."
)

set (CARLA_HAS_CONTENT FALSE)
set (ASSUME_CONTENT_IS_GIT_REPO TRUE) # TODO VERIFY USING HASH IF RAW DOWNLOAD.
if (ASSUME_CONTENT_IS_GIT_REPO)

  # Assume CARLA Content has been downloaded using git.
  # Therefore, we can use git commands to test the validity of the repo.

  function (carla_test_is_valid_content_directory RETURN_VALUE PATH)
    if (IS_DIRECTORY "${PATH}")
      message (STATUS "Searching CARLA Content in ${PATH}")
      execute_process (
        COMMAND
          git -C ${PATH} config --get remote.origin.url
        OUTPUT_VARIABLE
          GIT_ORIGIN_URL
        RESULT_VARIABLE
          GIT_ORIGIN_URL_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if (NOT ${GIT_ORIGIN_URL_RESULT})
        if ("${GIT_ORIGIN_URL}" STREQUAL "${CARLA_UE_CONTENT_ORIGIN_URL}")
          set (${RETURN_VALUE} TRUE PARENT_SCOPE)
          return ()
        endif ()
      endif ()
    endif ()
    set (${RETURN_VALUE} FALSE PARENT_SCOPE)
  endfunction ()

  carla_test_is_valid_content_directory (CARLA_HAS_CONTENT "${CARLA_UE_CONTENT_PATH}")

  if (NOT ${CARLA_HAS_CONTENT})
    foreach (ADDITIONAL_CONTENT_SEARCH_PATH ${CARLA_ADDITIONAL_CONTENT_SEARCH_PATHS})
      carla_test_is_valid_content_directory (CARLA_HAS_CONTENT "${ADDITIONAL_CONTENT_SEARCH_PATH}")
      if (${CARLA_HAS_CONTENT})
        message (STATUS "Found CARLA Content in \"${ADDITIONAL_CONTENT_SEARCH_PATH}\", creating symlink.")
        file (
          CREATE_LINK
            "${CARLA_UE_CONTENT_PATH}"
            "${ADDITIONAL_CONTENT_SEARCH_PATH}"
        )
      endif ()
    endforeach ()
  endif ()

else ()

  # Assume CARLA Content has been downloaded directly, without using git.
  # In this case use a simple directory existence check.

  if (IS_DIRECTORY ${CARLA_UE_CONTENT_PATH})
    set (CARLA_HAS_CONTENT TRUE)
  endif ()

endif ()

if (NOT ${CARLA_HAS_CONTENT})
  message (FATAL_ERROR 
    "Could not find CARLA Content. Either clone it from ${CARLA_UE_CONTENT_ORIGIN_URL} or, if you already downloaded it somewhere else in your system, add its path to CARLA_ADDITIONAL_CONTENT_SEARCH_PATHS.")
else ()
  message (STATUS 
    "Found CARLA Content."
  )
endif ()

if (WIN32)
  set (UE_SYSTEM_NAME Win64)
  set (UE_PLATFORM_NAME Windows)
else ()
  if (LINUX)
    set (UE_SYSTEM_NAME Linux)
  elseif (APPLE)
    set (UE_SYSTEM_NAME Mac)
  else ()
    message (FATAL_ERROR "Unknown target system.")
  endif ()
  set (UE_PLATFORM_NAME ${UE_SYSTEM_NAME})
endif ()

set (
  UNREAL_EDITOR_PATH
  ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor${EXE_EXT}
)

set (
  UE_DEPENDENCIES
  carla-server
  SQLite::SQLite3
  Boost::asio
  Boost::python
  Boost::geometry
  Boost::gil
  Eigen3::Eigen
  ZLIB::ZLIB
  PNG::PNG
  RPC::RPC
)

set (
  UE_DEPENDENCIES_ORDER_ONLY
  ${UE_DEPENDENCIES}
  carla-client
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

foreach (UE_DEP ${UE_DEPENDENCIES})
  list (
    APPEND
    CARLA_UNREAL_INCLUDE_PATHS
    "$<TARGET_PROPERTY:${UE_DEP},INTERFACE_INCLUDE_DIRECTORIES>"
  )
endforeach ()

#[[ @TODO

set (
  CARLA_UNREAL_LIBRARIES
  $<TARGET_FILE:SQLite::SQLite3>
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
]]

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
  ${CARLA_UE_CONTENT_PATH}/ExportedMaps
)



add_custom_target (
  ue-compile-shaders
  "Compile all CARLA Unreal Engine Shaders."
  COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor-Cmd${EXE_EXT}
    ${CARLA_UE_PROJECT_PATH}
    -run=DerivedDataCache
    -fill
)

add_custom_target (
  ue-cook-content
  "Compile all CARLA Unreal Engine Shaders."
  COMMAND
    ${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/${UE_SYSTEM_NAME}/UnrealEditor-Cmd${EXE_EXT}
    ${CARLA_UE_PROJECT_PATH}
    -run=Cook
    -TargetPlatform=${UE_SYSTEM_NAME}
    -unattended
    -nop4
)

add_custom_target (
  carla-unreal
  "Build the CarlaUnreal subproject."
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX}
    CarlaUnreal
    ${UE_SYSTEM_NAME}
    ${CARLA_UNREAL_BUILD_TYPE}
    _project=${CARLA_UE_PROJECT_PATH}
    -game
    -buildscw
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  ue-editor
  "Build the CarlaUnrealEditor subproject."
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX}
    CarlaUnrealEditor
    ${UE_SYSTEM_NAME}
    ${CARLA_UNREAL_BUILD_TYPE}
    _project=${CARLA_UE_PROJECT_PATH}
    -game
    -buildscw
  DEPENDS
    ${CARLA_UNREAL_CONFIGURE_OUTPUTS}
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-unreal-editor
  DEPENDS
    ue-editor
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
  set (TARGET_NAME ue-${TARGET_NAME_SHORT})
  set (TARGET_NAME_LONG carla-${TARGET_NAME})

  add_custom_target (
    ${TARGET_NAME}
    "Create a CARLA package in ${PACKAGE_CONFIGURATION} mode."
    COMMAND
      ${CARLA_UE_BUILD_COMMAND_PREFIX}
      CarlaUnreal
      ${UE_SYSTEM_NAME}
      ${UE_BUILD_CONFIGURATION}
      _project=${CARLA_UE_PROJECT_PATH}
      -game
      -buildscw
    COMMAND
      ${CARLA_UE_UAT_COMMAND_PREFIX}
      BuildCookRun
      _project=${CARLA_UE_PROJECT_PATH}
      -nocompileeditor
      -nop4
      -cookonthefly
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
  
  add_custom_target (${TARGET_NAME_SHORT} DEPENDS ${TARGET_NAME})
  add_custom_target (${TARGET_NAME_LONG} DEPENDS ${TARGET_NAME})

  set (
    PACKAGE_POST_BUILD_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE CREATING VERSION FILE UNREAL PACKAGE EXTRA FILES STARTED **********"
    COMMAND ${CMAKE_COMMAND}
      -DCARLA_ROOT_PATH=${CARLA_ROOT}
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
      -DCARLA_BUILD_PATH=${CMAKE_BINARY_DIR}
      -DCARLA_ROOT_PATH=${CARLA_ROOT}
      -DCARLA_PACKAGE_ARCHIVE_PATH=${CARLA_PACKAGE_ARCHIVE_PATH}
      -P${CMAKE_CURRENT_SOURCE_DIR}/Package/CopyCarlaAdditionalFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES COMPLETED **********"
  )

  if (NOT ${CARLA_UNREAL_PACKAGE_NO_COMPRESSION})
    set (
      PACKAGE_POST_BUILD_COMMANDS
      ${PACKAGE_POST_BUILD_COMMANDS}
      COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE STARTED **********"
      COMMAND ${CMAKE_COMMAND}
        -DCARLA_PACKAGE_PATH=${CARLA_PACKAGE_PATH}
        -DCARLA_PACKAGE_ARCHIVE_PATH=${CARLA_PACKAGE_ARCHIVE_PATH}
        -DCARLA_CURRENT_PACKAGE_PATH=${CARLA_CURRENT_PACKAGE_PATH}
        -P${CMAKE_CURRENT_SOURCE_DIR}/Package/Compress.cmake
      COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE COMPLETED **********"
    )
  endif ()

  add_custom_command (
    TARGET ${TARGET_NAME}
    POST_BUILD
    ${PACKAGE_POST_BUILD_COMMANDS}
    USES_TERMINAL
    VERBATIM
  )

endfunction()



# Docs for UE5 build configurations:
# https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/DevelopmentSetup/BuildConfigurations/

add_carla_ue_package_target("" ${CARLA_UNREAL_BUILD_TYPE})
add_carla_ue_package_target(Shipping Shipping)
add_carla_ue_package_target(Debug Debug)
add_carla_ue_package_target(DebugGame DebugGame)
add_carla_ue_package_target(Development Development)
add_carla_ue_package_target(Test Test)

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
)

if (ENABLE_STREETMAP)
  set_property (
    DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CARLA_UE_PLUGINS_PATH}/StreetMap/Binaries"
    "${CARLA_UE_PLUGINS_PATH}/StreetMap/Intermediate"
    "${CARLA_UE_PLUGINS_PATH}/StreetMap/Saved"
  )
endif ()