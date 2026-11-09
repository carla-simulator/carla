# UAT archives the build into <archive>/<Platform>/ (CarlaUnreal/, Engine/, the
# launcher, NOTICES.txt). Move its contents up to the archive root so the
# package root holds the launcher, CarlaUnreal/ and Engine/ next to PythonAPI/.
#
# Inputs: CARLA_PACKAGE_ARCHIVE_PATH, UE_SYSTEM_NAME

# The archive names the platform dir after the cook platform: Win64 is "Windows".
set (ARCHIVE_PLATFORM_NAME ${UE_SYSTEM_NAME})
if ("${UE_SYSTEM_NAME}" STREQUAL "Win64")
  set (ARCHIVE_PLATFORM_NAME Windows)
endif ()
set (PLATFORM_DIR ${CARLA_PACKAGE_ARCHIVE_PATH}/${ARCHIVE_PLATFORM_NAME})

if (NOT IS_DIRECTORY ${PLATFORM_DIR}/CarlaUnreal)
  message (FATAL_ERROR "No ${PLATFORM_DIR}/CarlaUnreal: UAT did not archive the build.")
endif ()

# The archive dir is reused across runs, so an entry from a previous flattened
# package is replaced rather than merged.
file (GLOB PLATFORM_ENTRIES RELATIVE ${PLATFORM_DIR} LIST_DIRECTORIES true ${PLATFORM_DIR}/*)
foreach (ENTRY ${PLATFORM_ENTRIES})
  file (REMOVE_RECURSE ${CARLA_PACKAGE_ARCHIVE_PATH}/${ENTRY})
  file (RENAME ${PLATFORM_DIR}/${ENTRY} ${CARLA_PACKAGE_ARCHIVE_PATH}/${ENTRY})
endforeach ()
file (REMOVE_RECURSE ${PLATFORM_DIR})
message (STATUS "Moved ${PLATFORM_DIR}/* to ${CARLA_PACKAGE_ARCHIVE_PATH}")
