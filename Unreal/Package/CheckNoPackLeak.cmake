# Post-cook guard for content packs (see Docs/content_packs.md, Util/ContentPacks):
# a pack under Plugins/Packs/<Pack>/ must never be cooked into the base release.
# The only thing that keeps it out is that the pack is not enabled for the base
# cook (not listed in CarlaUnreal.uproject, ExplicitlyLoaded:true, no
# -EnablePlugins=<Pack>); this script turns that convention into a hard check
# and fails the package when a pack's mount root /<Pack>/ shows up in the base.
#
# What is checked, for every <Pack> that has Plugins/Packs/<Pack>/<Pack>.uplugin
# or Plugins/Packs/<Pack>/carla-pack.json:
#  1. the cooked base registries the -createreleaseversion step wrote,
#     Releases/<release>/<Platform>/AssetRegistry.bin and
#     .../Metadata/DevelopmentAssetRegistry.bin: any occurrence of the mount-root
#     form "/<Pack>/" (package names such as /<Pack>/Maps/<Map> are stored as
#     plain ASCII in the registry's name table);
#  2. the stage manifest UAT writes, <staging>/<Platform>/Manifest_UFSFiles_*.txt:
#     any line under CarlaUnreal/Plugins/Packs/<Pack>/ (the files that went into
#     the base containers), when the staging dir still exists;
#  3. the packaged project: <archive>/<Platform>/CarlaUnreal/Plugins/Packs/<Pack>
#     must not exist (the stager copies the .uplugin of every enabled plugin).
# The registry check runs `carla-pack check-base` when the tool and a python3
# are available (CARLA_CONTENT_PACKS_TOOL / CARLA_CONTENT_PACKS_PYTHON): it parses
# the registry's FName table and flags only names that start with /<Pack>/, so an
# asset called TestPack or a base folder Content/Carla/Static/Other never trigger
# it. Without python it falls back to a byte search for "/<Pack>/", which a pack
# named after a base content folder (Other, Maps, ...) would trip;
# CARLA_PACK_LEAK_IGNORE (semicolon-separated pack names) skips a pack on request.
#
# Inputs: CARLA_PACKS_ROOT, CARLA_RELEASES_PATH, CARLA_BASE_RELEASE_NAME,
#         UE_SYSTEM_NAME; optional CARLA_PACKAGE_STAGING_PATH,
#         CARLA_PACKAGE_ARCHIVE_PATH, CARLA_PACK_LEAK_IGNORE,
#         CARLA_CONTENT_PACKS_TOOL (Util/ContentPacks/carla_pack.py),
#         CARLA_CONTENT_PACKS_PYTHON.
# Standalone check against an existing build:
#   cmake -DCARLA_PACKS_ROOT=Unreal/CarlaUnreal/Plugins/Packs -DCARLA_RELEASES_PATH=Unreal/CarlaUnreal/Releases \
#         -DCARLA_BASE_RELEASE_NAME=carla-0.10.0-Linux -DUE_SYSTEM_NAME=Linux -P Unreal/Package/CheckNoPackLeak.cmake

cmake_minimum_required (VERSION 3.20)   # script mode: cmake_path, IN_LIST (CMP0057)

foreach (REQUIRED_VAR CARLA_PACKS_ROOT CARLA_RELEASES_PATH CARLA_BASE_RELEASE_NAME UE_SYSTEM_NAME)
  if (NOT DEFINED ${REQUIRED_VAR} OR "${${REQUIRED_VAR}}" STREQUAL "")
    message (FATAL_ERROR "CheckNoPackLeak.cmake: ${REQUIRED_VAR} is not set.")
  endif ()
endforeach ()

if (NOT IS_DIRECTORY "${CARLA_PACKS_ROOT}")
  message (STATUS "Content pack leak check: no ${CARLA_PACKS_ROOT} directory, nothing to check.")
  return ()
endif ()

# Pack names: every direct child of the packs root that looks like a pack.
set (PACK_NAMES)
file (GLOB PACK_DIRS LIST_DIRECTORIES true "${CARLA_PACKS_ROOT}/*")
foreach (PACK_DIR ${PACK_DIRS})
  if (IS_DIRECTORY "${PACK_DIR}")
    cmake_path (GET PACK_DIR FILENAME PACK_NAME)
    if (EXISTS "${PACK_DIR}/${PACK_NAME}.uplugin" OR EXISTS "${PACK_DIR}/carla-pack.json")
      if (NOT "${PACK_NAME}" MATCHES "^[A-Za-z][A-Za-z0-9_]*$")
        message (WARNING "Content pack leak check: skipping '${PACK_NAME}' (not a valid pack name).")
      elseif (PACK_NAME IN_LIST CARLA_PACK_LEAK_IGNORE)
        message (STATUS "Content pack leak check: skipping '${PACK_NAME}' (CARLA_PACK_LEAK_IGNORE).")
      else ()
        list (APPEND PACK_NAMES ${PACK_NAME})
      endif ()
    endif ()
  endif ()
endforeach ()
if (NOT PACK_NAMES)
  message (STATUS "Content pack leak check: no packs under ${CARLA_PACKS_ROOT}, nothing to check.")
  return ()
endif ()

# 1. cooked base registries (the pack cook's base; must exist - the metadata step
#    checks the same two files).
set (RELEASE_PLATFORM_DIR "${CARLA_RELEASES_PATH}/${CARLA_BASE_RELEASE_NAME}/${UE_SYSTEM_NAME}")
set (REGISTRIES
  "${RELEASE_PLATFORM_DIR}/AssetRegistry.bin"
  "${RELEASE_PLATFORM_DIR}/Metadata/DevelopmentAssetRegistry.bin")
if (DEFINED CARLA_PACKAGE_ARCHIVE_PATH AND NOT "${CARLA_PACKAGE_ARCHIVE_PATH}" STREQUAL "")
  # A loose copy in the packaged project, when the stage left one (usually it is inside the pak).
  file (GLOB ARCHIVE_REGISTRIES "${CARLA_PACKAGE_ARCHIVE_PATH}/*/CarlaUnreal/AssetRegistry.bin")
  list (APPEND REGISTRIES ${ARCHIVE_REGISTRIES})
endif ()
set (CHECKED_REGISTRIES 0)
set (LEAKS)

# 1a. precise form: `carla-pack check-base` parses the registry's FName table and
#     reports package names that *start* with /<Pack>/ (the strings in the table are
#     stored back to back, so a byte search cannot tell "/Game/Foo/Other/X" from a
#     leaked "/Other/X"). Used whenever the tool and a python3 are at hand.
set (PRECISE_CHECK FALSE)
if (DEFINED CARLA_CONTENT_PACKS_TOOL AND EXISTS "${CARLA_CONTENT_PACKS_TOOL}")
  if (NOT DEFINED CARLA_CONTENT_PACKS_PYTHON OR "${CARLA_CONTENT_PACKS_PYTHON}" STREQUAL "")
    set (CARLA_CONTENT_PACKS_PYTHON python3)
  endif ()
  find_program (CARLA_PACK_CHECK_PYTHON NAMES ${CARLA_CONTENT_PACKS_PYTHON} python3 python)
  if (CARLA_PACK_CHECK_PYTHON)
    set (PACK_ARGS)
    foreach (PACK_NAME ${PACK_NAMES})
      list (APPEND PACK_ARGS --pack ${PACK_NAME})
    endforeach ()
    set (PRECISE_CHECK TRUE)
    foreach (REGISTRY ${REGISTRIES})
      if (NOT EXISTS "${REGISTRY}")
        continue ()
      endif ()
      execute_process (
        COMMAND ${CARLA_PACK_CHECK_PYTHON} ${CARLA_CONTENT_PACKS_TOOL} check-base ${REGISTRY}
                --platform ${UE_SYSTEM_NAME} ${PACK_ARGS}
        RESULT_VARIABLE CHECK_RC
        OUTPUT_VARIABLE CHECK_OUT
        ERROR_VARIABLE CHECK_ERR
      )
      if (CHECK_RC EQUAL 0)
        math (EXPR CHECKED_REGISTRIES "${CHECKED_REGISTRIES} + 1")
      elseif (CHECK_RC EQUAL 1 AND CHECK_OUT MATCHES "LEAK")
        math (EXPR CHECKED_REGISTRIES "${CHECKED_REGISTRIES} + 1")
        string (REGEX MATCHALL "LEAK[^\n]*" LEAK_MSGS "${CHECK_OUT}")
        foreach (LEAK_MSG ${LEAK_MSGS})
          list (APPEND LEAKS "${LEAK_MSG}")
        endforeach ()
      else ()
        message (WARNING "Content pack leak check: carla-pack check-base failed on ${REGISTRY} "
                         "(rc ${CHECK_RC}): ${CHECK_ERR}${CHECK_OUT} - falling back to the byte search.")
        set (PRECISE_CHECK FALSE)
        set (CHECKED_REGISTRIES 0)
        set (LEAKS)
        break ()
      endif ()
    endforeach ()
  endif ()
endif ()

# 1b. fallback without the tool/python: byte search for the mount-root form.
foreach (REGISTRY ${REGISTRIES})
  if (PRECISE_CHECK OR NOT EXISTS "${REGISTRY}")
    continue ()
  endif ()
  math (EXPR CHECKED_REGISTRIES "${CHECKED_REGISTRIES} + 1")
  foreach (PACK_NAME ${PACK_NAMES})
    # file(STRINGS) extracts the printable-ASCII runs of the binary file; the
    # registry's name table keeps package names as such runs.
    file (STRINGS "${REGISTRY}" HITS REGEX "/${PACK_NAME}/")
    if (HITS)
      list (GET HITS 0 FIRST_HIT)
      string (FIND "${FIRST_HIT}" "/${PACK_NAME}/" POS)
      math (EXPR FROM "${POS} - 40")
      if (FROM LESS 0)
        set (FROM 0)
      endif ()
      string (SUBSTRING "${FIRST_HIT}" ${FROM} 120 EXCERPT)
      list (LENGTH HITS N)
      list (APPEND LEAKS "pack '${PACK_NAME}': '/${PACK_NAME}/' found in ${REGISTRY} (${N} string(s), e.g. \"...${EXCERPT}...\")")
    endif ()
  endforeach ()
endforeach ()
if (CHECKED_REGISTRIES EQUAL 0)
  message (FATAL_ERROR
    "Content pack leak check: no base registry to check - expected ${RELEASE_PLATFORM_DIR}/AssetRegistry.bin "
    "(written by the cook when UAT runs with -createreleaseversion=${CARLA_BASE_RELEASE_NAME}).")
endif ()

# 2. stage manifest: the files that went into the base containers.
set (CHECKED_MANIFESTS 0)
if (DEFINED CARLA_PACKAGE_STAGING_PATH AND NOT "${CARLA_PACKAGE_STAGING_PATH}" STREQUAL "")
  file (GLOB STAGE_MANIFESTS
    "${CARLA_PACKAGE_STAGING_PATH}/*/Manifest_UFSFiles_*.txt"
    "${CARLA_PACKAGE_STAGING_PATH}/*/Manifest_NonUFSFiles_*.txt")
  foreach (MANIFEST ${STAGE_MANIFESTS})
    math (EXPR CHECKED_MANIFESTS "${CHECKED_MANIFESTS} + 1")
    foreach (PACK_NAME ${PACK_NAMES})
      file (STRINGS "${MANIFEST}" HITS REGEX "(^|/)Plugins/Packs/${PACK_NAME}/")
      if (HITS)
        list (LENGTH HITS N)
        list (GET HITS 0 FIRST_HIT)
        list (APPEND LEAKS "pack '${PACK_NAME}': ${N} staged file(s) under Plugins/Packs/${PACK_NAME}/ in ${MANIFEST} (e.g. ${FIRST_HIT})")
      endif ()
    endforeach ()
  endforeach ()
endif ()

# 3. packaged project tree.
set (CHECKED_ARCHIVES 0)
if (DEFINED CARLA_PACKAGE_ARCHIVE_PATH AND NOT "${CARLA_PACKAGE_ARCHIVE_PATH}" STREQUAL "")
  file (GLOB PROJECT_DIRS LIST_DIRECTORIES true "${CARLA_PACKAGE_ARCHIVE_PATH}/*/CarlaUnreal")
  foreach (PROJECT_DIR ${PROJECT_DIRS})
    if (IS_DIRECTORY "${PROJECT_DIR}")
      math (EXPR CHECKED_ARCHIVES "${CHECKED_ARCHIVES} + 1")
      foreach (PACK_NAME ${PACK_NAMES})
        if (EXISTS "${PROJECT_DIR}/Plugins/Packs/${PACK_NAME}")
          list (APPEND LEAKS "pack '${PACK_NAME}': ${PROJECT_DIR}/Plugins/Packs/${PACK_NAME} was staged into the package")
        endif ()
      endforeach ()
    endif ()
  endforeach ()
endif ()

if (LEAKS)
  list (JOIN LEAKS "\n  - " LEAK_LINES)
  list (JOIN PACK_NAMES ", " PACK_LIST)
  message (FATAL_ERROR
    "Content pack(s) leaked into the base release ${CARLA_BASE_RELEASE_NAME}:\n  - ${LEAK_LINES}\n"
    "A pack under ${CARLA_PACKS_ROOT} must not be part of the base cook: remove it from "
    "CarlaUnreal.uproject (Plugins list), make sure its .uplugin keeps \"ExplicitlyLoaded\": true "
    "and \"EnabledByDefault\": false, do not pass -EnablePlugins=<Pack> to the base cook, then "
    "re-run the package target. Packs are cooked separately with carla-pack build. "
    "(CARLA_PACK_LEAK_IGNORE=<name> skips a pack whose name collides with a base content folder.)")
endif ()

list (JOIN PACK_NAMES ", " PACK_LIST)
if (PRECISE_CHECK)
  set (HOW "FName table")
else ()
  set (HOW "byte search")
endif ()
message (STATUS
  "Content pack leak check: OK - no /<Pack>/ for ${PACK_LIST} in ${CHECKED_REGISTRIES} registry file(s) (${HOW}), "
  "${CHECKED_MANIFESTS} stage manifest(s), ${CHECKED_ARCHIVES} packaged project dir(s).")
