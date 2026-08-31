# Post-package step for content packs (see Docs / Util/ContentPacks):
#  1. stamp the release name into <archive>/<Platform>/CarlaUnreal/BaseRelease
#     (single line, next to the packaged project's Content/Paks) so the runtime
#     can refuse packs cooked against another base;
#  2. archive Unreal/CarlaUnreal/Releases/<release>/ (AssetRegistry.bin and the
#     pak/container file lists written by -createreleaseversion) as
#     <CARLA_PACKAGE_PATH>/<release>-release-metadata.tar.gz, next to the
#     package tarball. A pack cook (`-basedonreleaseversion=<release>`) needs
#     exactly this directory under Releases/ of the project it is cooked from.
#
# Inputs: CARLA_BASE_RELEASE_NAME, CARLA_RELEASES_PATH, CARLA_PACKAGE_PATH,
#         CARLA_PACKAGE_ARCHIVE_PATH, UE_SYSTEM_NAME

set (RELEASE_DIR ${CARLA_RELEASES_PATH}/${CARLA_BASE_RELEASE_NAME})
set (RELEASE_PLATFORM_DIR ${RELEASE_DIR}/${UE_SYSTEM_NAME})
foreach (REQUIRED_FILE AssetRegistry.bin Metadata/DevelopmentAssetRegistry.bin)
  if (NOT EXISTS ${RELEASE_PLATFORM_DIR}/${REQUIRED_FILE})
    message (FATAL_ERROR
      "Base release metadata incomplete: expected ${RELEASE_PLATFORM_DIR}/${REQUIRED_FILE} "
      "(written by the cook when UAT runs with -createreleaseversion=${CARLA_BASE_RELEASE_NAME}; "
      "a content-pack (DLC) cook needs ${RELEASE_PLATFORM_DIR}/AssetRegistry.bin and "
      "${RELEASE_PLATFORM_DIR}/Metadata/DevelopmentAssetRegistry.bin). "
      "Check the UAT log for the -createreleaseversion cook step.")
  endif ()
endforeach ()

# The packaged project dir: <archive>/<Platform>/CarlaUnreal (that is what
# FPaths::ProjectDir() resolves to in the packaged binary).
file (GLOB PROJECT_DIRS LIST_DIRECTORIES true ${CARLA_PACKAGE_ARCHIVE_PATH}/*/CarlaUnreal)
if (NOT PROJECT_DIRS)
  message (FATAL_ERROR "No <archive>/<Platform>/CarlaUnreal directory under ${CARLA_PACKAGE_ARCHIVE_PATH}.")
endif ()
foreach (PROJECT_DIR ${PROJECT_DIRS})
  if (IS_DIRECTORY ${PROJECT_DIR})
    file (WRITE ${PROJECT_DIR}/BaseRelease "${CARLA_BASE_RELEASE_NAME}\n")
    message (STATUS "Wrote ${PROJECT_DIR}/BaseRelease = ${CARLA_BASE_RELEASE_NAME}")
  endif ()
endforeach ()
# Also at the archive root for humans/tools that never open the project dir.
file (WRITE ${CARLA_PACKAGE_ARCHIVE_PATH}/BaseRelease "${CARLA_BASE_RELEASE_NAME}\n")

set (METADATA_ARCHIVE ${CARLA_PACKAGE_PATH}/${CARLA_BASE_RELEASE_NAME}-release-metadata.tar.gz)
file (REMOVE ${METADATA_ARCHIVE})
# UAT also copies the base pak/utoc/ucas into Releases/<release>/<Platform>/
# ("for patches", CopyBuildToStagingDirectory.Automation.cs:5007-5030). A pack
# (DLC) cook only needs AssetRegistry.bin + Metadata/DevelopmentAssetRegistry.bin
# (verified 2026-08-29: identical DLC containers with and without them), so the
# containers are left out - otherwise the "metadata" tarball is the size of the
# game (5.9 GB for the EmptyMap spike).
file (GLOB_RECURSE RELEASE_FILES RELATIVE ${CARLA_RELEASES_PATH} ${RELEASE_DIR}/*)
list (FILTER RELEASE_FILES EXCLUDE REGEX "\\.(pak|utoc|ucas|sig|uondemandtoc)$")
if (NOT RELEASE_FILES)
  message (FATAL_ERROR "No release metadata files under ${RELEASE_DIR}.")
endif ()
# Archive relative to Releases/ so extracting into <project>/Releases/ recreates
# Releases/<release>/<Platform>/... exactly.
execute_process (
  COMMAND ${CMAKE_COMMAND} -E tar cfz ${METADATA_ARCHIVE} ${RELEASE_FILES}
  WORKING_DIRECTORY ${CARLA_RELEASES_PATH}
  RESULT_VARIABLE TAR_RESULT
)
if (NOT TAR_RESULT EQUAL 0)
  message (FATAL_ERROR "Archiving ${RELEASE_DIR} failed (${TAR_RESULT}).")
endif ()
message (STATUS "Wrote ${METADATA_ARCHIVE}")

# UAT copies the base containers into the release dir on every package run;
# they are not needed for pack cooks (see above) and cost the size of the game,
# so drop them once the registry files are archived.
file (GLOB_RECURSE RELEASE_CONTAINERS ${RELEASE_DIR}/*.pak ${RELEASE_DIR}/*.utoc ${RELEASE_DIR}/*.ucas ${RELEASE_DIR}/*.sig)
foreach (CONTAINER ${RELEASE_CONTAINERS})
  file (SIZE ${CONTAINER} CONTAINER_SIZE)
  file (REMOVE ${CONTAINER})
  message (STATUS "Removed ${CONTAINER} (${CONTAINER_SIZE} bytes, not needed for content-pack cooks)")
endforeach ()
