# Resolves the version string carla::version() returns and writes Version.h.
#
# Client and server both compile LibCarla/source/carla/Version.h, so this one
# string is what Client::GetClientVersion() and the server's "version" RPC
# report, and what the client's mismatch warning compares.
#
#   CARLA_RELEASE_VERSION set      -> that version, e.g. "1.0" (release CI)
#   HEAD is exactly at a git tag    -> the tag, e.g. "1.0"
#   otherwise (nightly, local work) -> <CARLA_VERSION>-<short hash>[-dirty]
#   no git / no .git                -> <CARLA_VERSION>
#
# Used two ways: include()d at configure time, so Version.h exists before
# anything compiles, and run with `cmake -P` by the carla-version target on
# every build, so the hash follows new commits without reconfiguring.
# configure_file only rewrites Version.h when the string changes, so an
# unchanged version triggers no recompilation.
#
# Inputs: CARLA_VERSION, CARLA_RELEASE_VERSION, CARLA_VERSION_GIT_DIR,
#         CARLA_VERSION_SOURCE (Version.h.in), CARLA_VERSION_DESTINATION.

set (CARLA_VERSION_STRING "${CARLA_VERSION}")

if (NOT "${CARLA_RELEASE_VERSION}" STREQUAL "")
  set (CARLA_VERSION_STRING "${CARLA_RELEASE_VERSION}")
else ()
  find_package (Git QUIET)
  if (GIT_FOUND AND EXISTS "${CARLA_VERSION_GIT_DIR}/.git")
    execute_process (
      COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match HEAD
      WORKING_DIRECTORY ${CARLA_VERSION_GIT_DIR}
      RESULT_VARIABLE CARLA_GIT_TAG_RESULT
      OUTPUT_VARIABLE CARLA_GIT_TAG
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if (CARLA_GIT_TAG_RESULT EQUAL 0 AND NOT "${CARLA_GIT_TAG}" STREQUAL "")
      set (CARLA_VERSION_STRING "${CARLA_GIT_TAG}")
    else ()
      execute_process (
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=9 HEAD
        WORKING_DIRECTORY ${CARLA_VERSION_GIT_DIR}
        RESULT_VARIABLE CARLA_GIT_HASH_RESULT
        OUTPUT_VARIABLE CARLA_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if (CARLA_GIT_HASH_RESULT EQUAL 0 AND NOT "${CARLA_GIT_HASH}" STREQUAL "")
        set (CARLA_VERSION_STRING "${CARLA_VERSION}-${CARLA_GIT_HASH}")
        # Tracked files only: build outputs and untracked content do not count.
        # (git status refreshes the index stat info; diff-index alone would
        # report files whose timestamps changed as modified.)
        execute_process (
          COMMAND ${GIT_EXECUTABLE} status --porcelain --untracked-files=no
          WORKING_DIRECTORY ${CARLA_VERSION_GIT_DIR}
          RESULT_VARIABLE CARLA_GIT_STATUS_RESULT
          OUTPUT_VARIABLE CARLA_GIT_STATUS
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ERROR_QUIET
        )
        if (CARLA_GIT_STATUS_RESULT EQUAL 0 AND NOT "${CARLA_GIT_STATUS}" STREQUAL "")
          string (APPEND CARLA_VERSION_STRING "-dirty")
        endif ()
      endif ()
    endif ()
  endif ()
endif ()

configure_file (
  ${CARLA_VERSION_SOURCE}
  ${CARLA_VERSION_DESTINATION}
  @ONLY
)
