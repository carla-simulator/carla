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
  Boost::python
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

add_custom_target (
  carla-unreal-package
  COMMAND
    ${CARLA_UE_BUILD_COMMAND_PREFIX}
    CarlaUnreal
    ${UE_SYSTEM_NAME}
    Shipping
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
    -clientconfig=Shipping
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
file(GLOB CARLA_PYTHON_WHL_FILES ${CMAKE_BINARY_DIR}/PythonAPI/dist/carla-*.whl)
file(GLOB CARLA_PYTHON_EXAMPLE_FILES ${CMAKE_SOURCE_DIR}/PythonAPI/examples/*.py)
file(GLOB CARLA_PYTHON_RSS_FILES ${CMAKE_SOURCE_DIR}/PythonAPI/examples/rss/*.py)
file(GLOB CARLA_PYTHON_UTIL_FILES ${CMAKE_SOURCE_DIR}/PythonAPI/util/*.py)
file(GLOB CARLA_HDMAPS_FILES ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Content/Carla/HDMaps/*.pcd)
add_custom_target (
  carla-package
  DEPENDS carla-unreal-package
  COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES STARTED **********"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CARLA_TARGET_PACKAGE_PATH}/LICENSE
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/CHANGELOG.md ${CARLA_TARGET_PACKAGE_PATH}/CHANGELOG
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Docs/release_readme.md ${CARLA_TARGET_PACKAGE_PATH}/README

  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Docs/python_api.md ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/python_api.md
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/carla/dist
  COMMAND ${CMAKE_COMMAND} -E copy ${CARLA_PYTHON_WHL_FILES} ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/carla/dist/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/PythonAPI/carla/agents/ ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/carla/agents/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/PythonAPI/carla/scene_layout.py ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/carla/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/PythonAPI/carla/requirements.txt ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/carla/

  COMMAND ${CMAKE_COMMAND} -E make_directory ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/examples/
  COMMAND ${CMAKE_COMMAND} -E copy ${CARLA_PYTHON_EXAMPLE_FILES} ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/examples/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/examples/rss/
  COMMAND ${CMAKE_COMMAND} -E copy ${CARLA_PYTHON_RSS_FILES} ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/examples/rss/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/PythonAPI/examples/requirements.txt ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/examples/

  COMMAND ${CMAKE_COMMAND} -E make_directory ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/util/
  COMMAND ${CMAKE_COMMAND} -E copy ${CARLA_PYTHON_UTIL_FILES} ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/util/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/PythonAPI/util/requirements.txt ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/util/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/PythonAPI/util/opendrive/ ${CARLA_TARGET_PACKAGE_PATH}/PythonAPI/util/opendrive/

  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Co-Simulation/ ${CARLA_TARGET_PACKAGE_PATH}/Co-Simulation/

  COMMAND ${CMAKE_COMMAND} -E make_directory ${CARLA_TARGET_PACKAGE_PATH}/HDMaps/
  COMMAND ${CMAKE_COMMAND} -E copy ${CARLA_HDMAPS_FILES} ${CARLA_TARGET_PACKAGE_PATH}/HDMaps/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Content/Carla/HDMaps/Readme.md ${CARLA_TARGET_PACKAGE_PATH}/HDMaps/README
  
  COMMAND ${CMAKE_COMMAND} -E remove ${CARLA_TARGET_PACKAGE_PATH}/Manifest_NonUFSFiles_Linux.txt
  COMMAND ${CMAKE_COMMAND} -E remove ${CARLA_TARGET_PACKAGE_PATH}/Manifest_UFSFiles_Linux.txt
  COMMAND ${CMAKE_COMMAND} -E remove ${CARLA_TARGET_PACKAGE_PATH}/Manifest_DebugFiles_Linux.txt
  COMMAND ${CMAKE_COMMAND} -E echo "********** PACKAGE COPYING CARLA ADDITIONAL FILES COMPLETED **********"

  USES_TERMINAL
  VERBATIM
)

add_dependencies (
  carla-unreal-package
  ${UE_DEPENDENCIES_ORDER_ONLY}
)

if (WIN32)
  set(CARLA_PACKAGE_FILE_NAME_AND_ARGS cvf ${CARLA_PACKAGE_PATH}/CARLA.zip --format=zip)
else()
  set(CARLA_PACKAGE_FILE_NAME_AND_ARGS cvfz ${CARLA_PACKAGE_PATH}/CARLA.tar.gz)
endif()
file(GLOB CARLA_PACKAGE_FILES ${CARLA_TARGET_PACKAGE_PATH}/*)
add_custom_target (
  compress-package
  DEPENDS carla-unreal-package carla-package
  COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE STARTED **********"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CARLA_TARGET_PACKAGE_PATH} cmake -E tar ${CARLA_PACKAGE_FILE_NAME_AND_ARGS} ${CARLA_PACKAGE_FILES}
  COMMAND ${CMAKE_COMMAND} -E echo "********** COMPRESSING PACKAGE COMPLETED **********"
  USES_TERMINAL
  VERBATIM
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

add_custom_target (
  package
)

add_dependencies (
  package
  carla-unreal-package
  carla-package
  compress-package
)
