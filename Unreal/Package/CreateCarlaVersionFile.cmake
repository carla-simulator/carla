# Writes the package VERSION file: the commits of CARLA, its content and the
# Unreal Engine the package was built from.
#
# A hash is only taken from a directory that is itself the root of a git
# checkout. Otherwise git walks up to an enclosing repository and reports its
# commit instead (a content folder that is not a clone reported CARLA's hash),
# and a directory git cannot read (not a checkout, refused by safe.directory)
# used to leave the field blank without a word. Both now write "unknown (...)"
# and print a warning; the package is still produced.

function (carla_git_hash DIR OUT_VAR)
  if ("${DIR}" STREQUAL "")
    set (REASON "path not set")
  elseif (NOT IS_DIRECTORY "${DIR}")
    set (REASON "${DIR} does not exist")
  else ()
    execute_process (
      COMMAND git -C "${DIR}" rev-parse --show-toplevel
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE TOPLEVEL
      ERROR_VARIABLE ERROR
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
    if (NOT RESULT EQUAL 0)
      set (REASON "not a git checkout: ${ERROR}")
    else ()
      file (REAL_PATH "${DIR}" DIR_REAL)
      file (REAL_PATH "${TOPLEVEL}" TOPLEVEL_REAL)
      if (NOT DIR_REAL STREQUAL TOPLEVEL_REAL)
        set (REASON "not a git checkout of its own, inside ${TOPLEVEL_REAL}")
      else ()
        execute_process (
          COMMAND git -C "${DIR}" rev-parse HEAD
          RESULT_VARIABLE RESULT
          OUTPUT_VARIABLE HASH
          ERROR_VARIABLE ERROR
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_STRIP_TRAILING_WHITESPACE
        )
        if (RESULT EQUAL 0 AND NOT HASH STREQUAL "")
          set (${OUT_VAR} "${HASH}" PARENT_SCOPE)
          return ()
        endif ()
        set (REASON "git rev-parse HEAD failed: ${ERROR}")
      endif ()
    endif ()
  endif ()
  # Keep the VERSION field on one line: git's errors can span several.
  string (REGEX REPLACE "\n.*" "" REASON "${REASON}")
  message (WARNING "VERSION file: no git hash for ${OUT_VAR} (${REASON}).")
  set (${OUT_VAR} "unknown (${REASON})" PARENT_SCOPE)
endfunction ()

# The engine's own version, so a package built from an engine that is not a
# git checkout (unpacked from an archive) can still be traced.
function (carla_unreal_engine_version ENGINE_PATH OUT_VAR)
  set (VERSION_FILE "${ENGINE_PATH}/Engine/Build/Build.version")
  if ("${ENGINE_PATH}" STREQUAL "")
    message (WARNING "VERSION file: no engine version (engine path not set).")
    set (${OUT_VAR} "unknown" PARENT_SCOPE)
    return ()
  elseif (NOT EXISTS "${VERSION_FILE}")
    message (WARNING "VERSION file: no engine version (${VERSION_FILE} not found).")
    set (${OUT_VAR} "unknown" PARENT_SCOPE)
    return ()
  endif ()
  file (READ "${VERSION_FILE}" JSON)
  string (JSON MAJOR ERROR_VARIABLE JSON_ERROR GET "${JSON}" MajorVersion)
  string (JSON MINOR ERROR_VARIABLE JSON_ERROR GET "${JSON}" MinorVersion)
  string (JSON PATCH ERROR_VARIABLE JSON_ERROR GET "${JSON}" PatchVersion)
  string (JSON CHANGELIST ERROR_VARIABLE JSON_ERROR GET "${JSON}" Changelist)
  string (JSON BRANCH ERROR_VARIABLE JSON_ERROR GET "${JSON}" BranchName)
  set (${OUT_VAR}
    "${MAJOR}.${MINOR}.${PATCH} (changelist ${CHANGELIST}, branch ${BRANCH})"
    PARENT_SCOPE)
endfunction ()

carla_git_hash ("${CARLA_WORKSPACE_PATH}" CARLA_GIT_HASH)
carla_git_hash ("${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Content/Carla" CONTENT_GIT_HASH)
carla_git_hash ("${CARLA_UNREAL_ENGINE_PATH}" UNREAL_ENGINE_GIT_HASH)
carla_unreal_engine_version ("${CARLA_UNREAL_ENGINE_PATH}" UNREAL_ENGINE_VERSION)

file (
  WRITE
    ${CARLA_PACKAGE_VERSION_FILE}
    "Carla git hash:         ${CARLA_GIT_HASH}\n"
    "Content git hash:       ${CONTENT_GIT_HASH}\n"
    "UnrealEngine git hash:  ${UNREAL_ENGINE_GIT_HASH}\n"
    "UnrealEngine version:   ${UNREAL_ENGINE_VERSION}\n"
)
