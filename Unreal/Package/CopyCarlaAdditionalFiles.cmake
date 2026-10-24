
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/LICENSE ${CARLA_PACKAGE_ARCHIVE_PATH}/LICENSE)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/CHANGELOG.md ${CARLA_PACKAGE_ARCHIVE_PATH}/CHANGELOG)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Docs/release_readme.md ${CARLA_PACKAGE_ARCHIVE_PATH}/README)
if (WIN32)
    set (EXE_EXT .exe)
    set (RECASTBUILDER_SRC ${CARLA_BUILD_PATH}/_deps/recastnavigation-build/RecastBuilder/RecastBuilder.exe)
    set (RECASTBUILDER_DST ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools/RecastBuilder.exe)
elseif (APPLE)
    set (EXE_EXT)
    set (RECASTBUILDER_SRC ${CARLA_BUILD_PATH}/_deps/recastnavigation-build/RecastBuilder/RecastBuilder.app/Contents/MacOS/RecastBuilder)
    set (RECASTBUILDER_DST ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools/RecastBuilder)
else ()
    set (EXE_EXT)
    set (RECASTBUILDER_SRC ${CARLA_BUILD_PATH}/_deps/recastnavigation-build/RecastBuilder/RecastBuilder)
    set (RECASTBUILDER_DST ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools/RecastBuilder)
endif ()
make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/Tools)
# @TODO Temporary hack:
file (COPY_FILE ${RECASTBUILDER_SRC} ${RECASTBUILDER_DST})
# The packaged binary looks for RecastBuilder at FPaths::RootDir()/Tools, and in
# a packaged build RootDir is <archive>/<Platform>/ (the dir holding Engine/ and
# CarlaUnreal/), not the archive root (CarlaEpisode.cpp BuildRecastBuilderFile).
# Without this copy it silently fell back to the compile-time build-tree path.
cmake_path (GET CARLA_UE_PROJECT_PATH PARENT_PATH CARLA_UE_PROJECT_DIR)
file (GLOB CARLA_PACKAGE_PLATFORM_DIRS LIST_DIRECTORIES true
  ${CARLA_PACKAGE_ARCHIVE_PATH}/*/CarlaUnreal
  ${CARLA_PACKAGE_ARCHIVE_PATH}/*/*.app/Contents/UE/CarlaUnreal
)
foreach (CARLA_PACKAGE_PROJECT_DIR ${CARLA_PACKAGE_PLATFORM_DIRS})
  cmake_path (GET CARLA_PACKAGE_PROJECT_DIR PARENT_PATH CARLA_PACKAGE_PLATFORM_DIR)
  make_directory (${CARLA_PACKAGE_PLATFORM_DIR}/Tools)
  file (COPY_FILE ${RECASTBUILDER_SRC} ${CARLA_PACKAGE_PLATFORM_DIR}/Tools/RecastBuilder${EXE_EXT})
  # FCarlaModule::AddShaderSearchPaths maps /Plugin/Carla to <plugin>/Shaders at
  # startup; the plugin is staged without its Shaders folder, so a packaged
  # server logged "shader directory ... does not exist; shaders will be
  # unavailable" on every start. The .usf/.ush sources are needed at runtime by
  # the shader-based sensors.
  file (
    COPY ${CARLA_UE_PROJECT_DIR}/Plugins/Carla/Shaders/
    DESTINATION ${CARLA_PACKAGE_PROJECT_DIR}/Plugins/Carla/Shaders/)
endforeach ()

if (APPLE)
  # Update staged app bundle with freshly compiled Shipping binary if present
  set (FRESH_SHIPPING_BIN "${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Binaries/Mac/CarlaUnreal-Mac-Shipping.app/Contents/MacOS/CarlaUnreal-Mac-Shipping")
  if (EXISTS "${FRESH_SHIPPING_BIN}")
    file (GLOB CARLA_MAC_APP_BUNDLES LIST_DIRECTORIES true ${CARLA_PACKAGE_ARCHIVE_PATH}/*/*.app)
    foreach (CARLA_MAC_APP ${CARLA_MAC_APP_BUNDLES})
      if (EXISTS "${CARLA_MAC_APP}/Contents/MacOS/CarlaUnreal-Mac-Shipping")
        file (COPY_FILE "${FRESH_SHIPPING_BIN}" "${CARLA_MAC_APP}/Contents/MacOS/CarlaUnreal-Mac-Shipping")
      endif ()
    endforeach ()
  endif ()

  # Re-sign the staged macOS app bundles without sandbox entitlements.
  # Unreal Engine's default shipping config applies Sandbox.NoNet.entitlements,
  # which enables com.apple.security.app-sandbox without network permissions,
  # causing bind() to fail with "Operation not permitted".
  file (GLOB CARLA_MAC_APP_BUNDLES LIST_DIRECTORIES true ${CARLA_PACKAGE_ARCHIVE_PATH}/*/*.app)
  foreach (CARLA_MAC_APP ${CARLA_MAC_APP_BUNDLES})
    execute_process (
      COMMAND /usr/bin/codesign --force --sign - ${CARLA_MAC_APP}
      RESULT_VARIABLE CODESIGN_RESULT
    )
    if (CODESIGN_RESULT EQUAL 0)
      message (STATUS "Re-signed ${CARLA_MAC_APP} without app-sandbox restrictions.")
    else ()
      message (WARNING "Failed to re-sign ${CARLA_MAC_APP} (${CODESIGN_RESULT}).")
    endif ()
  endforeach ()

  make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/Mac/Tools)
  file (COPY_FILE ${RECASTBUILDER_SRC} ${CARLA_PACKAGE_ARCHIVE_PATH}/Mac/Tools/RecastBuilder)

  file (WRITE ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal.sh
"#!/bin/bash
SCRIPT_DIR=\"\$(cd \"\$(dirname \"\${BASH_SOURCE[0]}\")\" && pwd)\"
# Normalize space-separated port flags (-carla-rpc-port 2000 -> -carla-rpc-port=2000)
ARGS=()
while [[ \$# -gt 0 ]]; do
  case \"\$1\" in
    -carla-rpc-port|-carla-port|-world-port|-carla-streaming-port|-carla-secondary-port)
      if [[ -n \"\$2\" && \"\$2\" != -* ]]; then
        ARGS+=(\"\$1=\$2\")
        shift 2
      else
        ARGS+=(\"\$1\")
        shift
      fi
      ;;
    *)
      ARGS+=(\"\$1\")
      shift
      ;;
  esac
done
exec \"\$SCRIPT_DIR/Mac/CarlaUnreal-Mac-Shipping.app/Contents/MacOS/CarlaUnreal-Mac-Shipping\" \"\${ARGS[@]}\"
")
  execute_process (COMMAND chmod +x ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal.sh)

  file (WRITE ${CARLA_PACKAGE_ARCHIVE_PATH}/Mac/CarlaUnreal.sh
"#!/bin/bash
SCRIPT_DIR=\"\$(cd \"\$(dirname \"\${BASH_SOURCE[0]}\")\" && pwd)\"
# Normalize space-separated port flags (-carla-rpc-port 2000 -> -carla-rpc-port=2000)
ARGS=()
while [[ \$# -gt 0 ]]; do
  case \"\$1\" in
    -carla-rpc-port|-carla-port|-world-port|-carla-streaming-port|-carla-secondary-port)
      if [[ -n \"\$2\" && \"\$2\" != -* ]]; then
        ARGS+=(\"\$1=\$2\")
        shift 2
      else
        ARGS+=(\"\$1\")
        shift
      fi
      ;;
    *)
      ARGS+=(\"\$1\")
      shift
      ;;
  esac
done
exec \"\$SCRIPT_DIR/CarlaUnreal-Mac-Shipping.app/Contents/MacOS/CarlaUnreal-Mac-Shipping\" \"\${ARGS[@]}\"
")
  execute_process (COMMAND chmod +x ${CARLA_PACKAGE_ARCHIVE_PATH}/Mac/CarlaUnreal.sh)
endif ()

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/dist)
file (GLOB PYTHON_WHL_FILES ${CARLA_BUILD_PATH}/PythonAPI/dist/carla-*.whl)
file (COPY ${PYTHON_WHL_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/dist/)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Docs/python_api.md ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/python_api.md)
file (COPY ${CARLA_WORKSPACE_PATH}/PythonAPI/carla/agents/ DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/agents/)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/PythonAPI/carla/scene_layout.py ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/scene_layout.py)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/PythonAPI/carla/requirements.txt ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/requirements.txt)

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/)
file (GLOB PYTHON_EXAMPLE_FILES ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/*.py ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/*.xml)
file (COPY ${PYTHON_EXAMPLE_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/)
# make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/rss/)
# file (GLOB PYTHON_EXAMPLE_RSS_FILES ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/rss/*.py)
# file (COPY ${PYTHON_EXAMPLE_RSS_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/rss/)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/requirements.txt ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/requirements.txt)

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/ros2)
file (GLOB PYTHON_EXAMPLE_ROS2_FILES ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/ros2/*)
file (COPY ${PYTHON_EXAMPLE_ROS2_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/ros2)
make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/ros2/config)
file (GLOB PYTHON_EXAMPLE_CONFIG_ROS2_FILES ${CARLA_WORKSPACE_PATH}/PythonAPI/examples/ros2/config/*)
file (COPY ${PYTHON_EXAMPLE_CONFIG_ROS2_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/examples/ros2/config)

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/util/)
file (GLOB PYTHON_UTIL_FILES ${CARLA_WORKSPACE_PATH}/PythonAPI/util/*.py)
file (COPY ${PYTHON_UTIL_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/util/)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/PythonAPI/util/requirements.txt ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/util/requirements.txt)
file (COPY ${CARLA_WORKSPACE_PATH}/PythonAPI/util/opendrive/ DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/util/opendrive/)

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/Util/MapBuilder/)
file (GLOB MAPBUILDER_FILES ${CARLA_WORKSPACE_PATH}/Util/MapBuilder/*)
file (COPY ${MAPBUILDER_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/Util/MapBuilder/)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Util/MapBuilder/carla_map.py ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/util/carla_map.py)

# Top-level carla-map executable wrapper
file (WRITE ${CARLA_PACKAGE_ARCHIVE_PATH}/carla-map
"#!/usr/bin/env bash
SCRIPT_DIR=\"\$(cd \"\$(dirname \"\${BASH_SOURCE[0]}\")\" && pwd)\"

if command -v python3.10 &>/dev/null; then
    PY_BIN=\"python3.10\"
elif command -v python3 &>/dev/null; then
    PY_BIN=\"python3\"
else
    echo \"ERROR: python3 not found\" >&2
    exit 1
fi

WHL=\$(find \"\$SCRIPT_DIR/PythonAPI/carla/dist\" -name \"carla-*.whl\" 2>/dev/null | head -n 1)
if [[ -n \"\$WHL\" ]]; then
    export PYTHONPATH=\"\$WHL:\$SCRIPT_DIR:\$SCRIPT_DIR/Util/MapBuilder:\$PYTHONPATH\"
else
    export PYTHONPATH=\"\$SCRIPT_DIR:\$SCRIPT_DIR/Util/MapBuilder:\$PYTHONPATH\"
fi

if [[ -d \"\$SCRIPT_DIR/Tools\" ]]; then
    export PATH=\"\$SCRIPT_DIR/Tools:\$PATH\"
fi
if [[ -d \"\$SCRIPT_DIR/Mac/Tools\" ]]; then
    export PATH=\"\$SCRIPT_DIR/Mac/Tools:\$PATH\"
fi

exec \"\$PY_BIN\" \"\$SCRIPT_DIR/Util/MapBuilder/carla_map.py\" \"\$@\"
")
execute_process (COMMAND chmod +x ${CARLA_PACKAGE_ARCHIVE_PATH}/carla-map)
execute_process (COMMAND chmod +x ${CARLA_PACKAGE_ARCHIVE_PATH}/Util/MapBuilder/carla-map)

# file (COPY ${CARLA_WORKSPACE_PATH}/Co-Simulation/ DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/Co-Simulation/)

# make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/)
# file (GLOB PYTHON_HDMAP_FILES ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla/HDMaps/*.pcd)
# file (COPY ${PYTHON_HDMAP_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/)
# file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla/HDMaps/Readme.md ${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/README)
