
# The archive directory is reused between builds and the copies below only add files:
# clear what this script owns, so files of earlier builds (old wheels, removed
# examples, older DLSS snippets) never reach a new package.
file (REMOVE_RECURSE
  ${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI
  ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools
  ${CARLA_PACKAGE_ARCHIVE_PATH}/Skills
  ${CARLA_PACKAGE_ARCHIVE_PATH}/Util
  ${CARLA_PACKAGE_ARCHIVE_PATH}/AGENTS.md
  ${CARLA_PACKAGE_ARCHIVE_PATH}/.mcp.json
  ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Plugins/Carla/Shaders)
file (GLOB STALE_DLSS_SNIPPETS
  ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Binaries/*/libnvidia-ngx-*
  ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Binaries/*/nvngx_*.dll
  ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Binaries/*/NVIDIA_DLSS_LICENSE.txt)
if (STALE_DLSS_SNIPPETS)
  file (REMOVE ${STALE_DLSS_SNIPPETS})
endif ()

file (COPY_FILE ${CARLA_WORKSPACE_PATH}/LICENSE ${CARLA_PACKAGE_ARCHIVE_PATH}/LICENSE)
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/CHANGELOG.md ${CARLA_PACKAGE_ARCHIVE_PATH}/CHANGELOG)
# One README per platform: the commands differ (CarlaUnreal.sh / CarlaUnreal.exe, python3 / python).
if (WIN32)
  file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Docs/release_readme_windows.md ${CARLA_PACKAGE_ARCHIVE_PATH}/README)
else ()
  file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Docs/release_readme.md ${CARLA_PACKAGE_ARCHIVE_PATH}/README)
endif ()
# The release Docker image is built from the package root:
#   docker build -t carlasim/carla:<version> .
if (NOT WIN32)
  file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Util/Docker/Release.Dockerfile ${CARLA_PACKAGE_ARCHIVE_PATH}/Dockerfile)
endif ()
if (WIN32)
    set (EXE_EXT .exe)
else ()
    set (EXE_EXT)
endif ()
make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/Tools)
# @TODO Temporary hack:
# The packaged binary looks for RecastBuilder at FPaths::RootDir()/Tools, the
# archive root once FlattenPackageLayout.cmake has run (CarlaEpisode.cpp
# BuildRecastBuilderFile).
file (COPY_FILE ${CARLA_BUILD_PATH}/_deps/recastnavigation-build/RecastBuilder/RecastBuilder${EXE_EXT} ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools/RecastBuilder${EXE_EXT})
# carla-pack installs content packs (AdditionalMaps) into this package; standard library only.
file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Util/ContentPacks/carla_pack.py ${CARLA_PACKAGE_ARCHIVE_PATH}/Tools/carla_pack.py)
# FCarlaModule::AddShaderSearchPaths maps /Plugin/Carla to <plugin>/Shaders at
# startup; the plugin is staged without its Shaders folder, and the shader-based
# sensors need the .usf/.ush sources at runtime.
cmake_path (GET CARLA_UE_PROJECT_PATH PARENT_PATH CARLA_UE_PROJECT_DIR)
file (
  COPY ${CARLA_UE_PROJECT_DIR}/Plugins/Carla/Shaders/
  DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Plugins/Carla/Shaders/)

make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/PythonAPI/carla/dist)
# Every wheel the build made, one per Python version in CARLA_PYTHON_VERSIONS:
if (NOT CARLA_PYTHON_API_DIST_PATH)
  set (CARLA_PYTHON_API_DIST_PATH ${CARLA_BUILD_PATH}/PythonAPI/dist)
endif ()
# Only this version's wheels: dist/ keeps wheels of earlier versions built in the same tree.
file (GLOB PYTHON_WHL_FILES ${CARLA_PYTHON_API_DIST_PATH}/carla-${CARLA_VERSION}-*.whl)
if (NOT PYTHON_WHL_FILES)
  message (FATAL_ERROR "No carla-${CARLA_VERSION} wheel in ${CARLA_PYTHON_API_DIST_PATH}")
endif ()
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

# Trees packaged at the path they have in the repository. Only git-tracked files:
# they also hold local models, venvs, caches and outputs that must not be packaged.
#  - the example folders the docs and CHANGELOG point at;
#  - the agent skill library and the MCP server that serves it, which resolves
#    the library as ../../Skills from its own folder (Docs/agent_skills.md).
set (CARLA_PACKAGE_TRACKED_TREES
  PythonAPI/examples/nvidia
  PythonAPI/examples/av_stacks
  PythonAPI/util/tools
  Skills
  Util/AgenticTools)
find_package (Git QUIET)
if (GIT_FOUND AND EXISTS ${CARLA_WORKSPACE_PATH}/.git)
  execute_process (
    COMMAND ${GIT_EXECUTABLE} -C ${CARLA_WORKSPACE_PATH} ls-files -- ${CARLA_PACKAGE_TRACKED_TREES}
    OUTPUT_VARIABLE CARLA_PACKAGE_TRACKED_FILES
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
  string (REPLACE "\n" ";" CARLA_PACKAGE_TRACKED_FILES "${CARLA_PACKAGE_TRACKED_FILES}")
  foreach (TRACKED_FILE ${CARLA_PACKAGE_TRACKED_FILES})
    cmake_path (GET TRACKED_FILE PARENT_PATH TRACKED_DIR)
    file (COPY ${CARLA_WORKSPACE_PATH}/${TRACKED_FILE} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/${TRACKED_DIR})
  endforeach ()

  # What makes an assistant opened in the package find the library: AGENTS.md
  # (the package's own guidance, then the skill catalogue of the repository's
  # AGENTS.md) and .mcp.json (the CARLA MCP server only: unreal-mcp is the
  # editor's and no package runs it).
  file (READ ${CARLA_WORKSPACE_PATH}/AGENTS.md REPOSITORY_AGENTS)
  string (FIND "${REPOSITORY_AGENTS}" "## Skill catalogue" CATALOGUE_BEGIN)
  if (CATALOGUE_BEGIN EQUAL -1)
    message (FATAL_ERROR "No \"## Skill catalogue\" section in ${CARLA_WORKSPACE_PATH}/AGENTS.md")
  endif ()
  string (SUBSTRING "${REPOSITORY_AGENTS}" ${CATALOGUE_BEGIN} -1 SKILL_CATALOGUE)
  # The catalogue ends where the next section of the same level starts.
  string (FIND "${SKILL_CATALOGUE}" "\n## " CATALOGUE_END)
  if (NOT CATALOGUE_END EQUAL -1)
    string (SUBSTRING "${SKILL_CATALOGUE}" 0 ${CATALOGUE_END} SKILL_CATALOGUE)
  endif ()
  string (STRIP "${SKILL_CATALOGUE}" SKILL_CATALOGUE)
  file (READ ${CMAKE_CURRENT_LIST_DIR}/PackageAgents.md PACKAGE_AGENTS)
  file (WRITE ${CARLA_PACKAGE_ARCHIVE_PATH}/AGENTS.md "${PACKAGE_AGENTS}${SKILL_CATALOGUE}\n")
  file (COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/PackageMcp.json ${CARLA_PACKAGE_ARCHIVE_PATH}/.mcp.json)
else ()
  message (WARNING "Not a git checkout: ${CARLA_PACKAGE_TRACKED_TREES} are not packaged.")
endif ()

# NGX loads the DLSS / DLSS-RR model snippets from the executable's folder; without
# them DLSS upscaling and the rt_lens denoiser silently fall back.
if (CARLA_DLSS_SDK_RESOLVED AND NOT CARLA_DLSS_SDK_RESOLVED STREQUAL "disabled")
  if (WIN32)
    set (DLSS_SNIPPET_GLOBS
      ${CARLA_DLSS_SDK_RESOLVED}/lib/Windows_x86_64/rel/nvngx_dlss.dll
      ${CARLA_DLSS_SDK_RESOLVED}/lib/Windows_x86_64/rel/nvngx_dlssd.dll)
    set (DLSS_SNIPPET_DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Binaries/Win64)
  else ()
    set (DLSS_SNIPPET_GLOBS
      ${CARLA_DLSS_SDK_RESOLVED}/lib/Linux_x86_64/rel/libnvidia-ngx-dlss.so.*
      ${CARLA_DLSS_SDK_RESOLVED}/lib/Linux_x86_64/rel/libnvidia-ngx-dlssd.so.*)
    set (DLSS_SNIPPET_DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/CarlaUnreal/Binaries/Linux)
  endif ()
  file (GLOB DLSS_SNIPPETS ${DLSS_SNIPPET_GLOBS})
  list (LENGTH DLSS_SNIPPETS DLSS_SNIPPET_COUNT)
  if (NOT DLSS_SNIPPET_COUNT EQUAL 2)
    message (FATAL_ERROR "Expected the DLSS and DLSS-RR snippets in ${CARLA_DLSS_SDK_RESOLVED}, found: ${DLSS_SNIPPETS}")
  endif ()
  file (COPY ${DLSS_SNIPPETS} DESTINATION ${DLSS_SNIPPET_DESTINATION})
  if (EXISTS ${CARLA_DLSS_SDK_RESOLVED}/LICENSE.txt)
    file (COPY_FILE ${CARLA_DLSS_SDK_RESOLVED}/LICENSE.txt ${DLSS_SNIPPET_DESTINATION}/NVIDIA_DLSS_LICENSE.txt)
  endif ()
endif ()

# file (COPY ${CARLA_WORKSPACE_PATH}/Co-Simulation/ DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/Co-Simulation/)

# make_directory (${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/)
# file (GLOB PYTHON_HDMAP_FILES ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla/HDMaps/*.pcd)
# file (COPY ${PYTHON_HDMAP_FILES} DESTINATION ${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/)
# file (COPY_FILE ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla/HDMaps/Readme.md ${CARLA_PACKAGE_ARCHIVE_PATH}/HDMaps/README)
