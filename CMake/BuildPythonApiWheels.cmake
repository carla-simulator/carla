# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# CMake script-mode helper for the `carla-python-api-wheels` target.
#
# Builds the CARLA Python API wheel for one or more interpreters. The wheel
# links a version-specific Boost.Python target resolved at the outer CMake
# configure, so each interpreter other than the currently configured one is
# built in its own nested configure. The current interpreter reuses the main
# build tree (identical to the carla-python-api target).
#
# Driven entirely through -D variables set by the target in
# PythonAPI/CMakeLists.txt:
#   CARLA_WHEEL_VERSIONS         ';'/','-separated MAJOR.MINOR list (empty = current only)
#   CARLA_WHEEL_CURRENT_VERSION  MAJOR.MINOR of the configured interpreter
#   CARLA_WHEEL_CURRENT_PYTHON   path to the configured interpreter
#   CARLA_WHEEL_SOURCE_DIR       repository root
#   CARLA_WHEEL_MAIN_BUILD_DIR   the main (outer) build directory
#   CARLA_WHEEL_BINARY_ROOT      parent dir for per-version nested builds
#   CARLA_WHEEL_OUTPUT_DIR       directory the finished wheels are collected in
#   CARLA_WHEEL_BUILD_TYPE       CMAKE_BUILD_TYPE to forward to nested configures
#   CARLA_WHEEL_GENERATOR        CMAKE_GENERATOR to forward to nested configures
#   CARLA_WHEEL_TOOLCHAIN        CMAKE_TOOLCHAIN_FILE to forward (may be empty)
#   CARLA_WHEEL_MANYLINUX        ON to auditwheel-repair (best-effort)
#   CARLA_WHEEL_PLATFORM         forced auditwheel platform tag (may be empty)

set (CARLA_WHEEL_SUPPORTED_VERSIONS 3.8 3.9 3.10 3.11 3.12 3.13 3.14)

# Resolve the requested version list: normalize separators, default to the
# current interpreter when the user did not request any specific version.
string (REPLACE "," ";" CARLA_WHEEL_REQUESTED "${CARLA_WHEEL_VERSIONS}")
string (STRIP "${CARLA_WHEEL_REQUESTED}" CARLA_WHEEL_REQUESTED)
if (CARLA_WHEEL_REQUESTED STREQUAL "")
  set (CARLA_WHEEL_REQUESTED "${CARLA_WHEEL_CURRENT_VERSION}")
endif ()

file (MAKE_DIRECTORY "${CARLA_WHEEL_OUTPUT_DIR}")

set (CARLA_WHEEL_BUILT "")
set (CARLA_WHEEL_FAILED "")

foreach (CARLA_WHEEL_VERSION IN LISTS CARLA_WHEEL_REQUESTED)
  string (STRIP "${CARLA_WHEEL_VERSION}" CARLA_WHEEL_VERSION)
  if (CARLA_WHEEL_VERSION STREQUAL "")
    continue ()
  endif ()

  # Reject anything outside the supported range (e.g. 3.7).
  list (FIND CARLA_WHEEL_SUPPORTED_VERSIONS "${CARLA_WHEEL_VERSION}" CARLA_WHEEL_SUPPORTED_INDEX)
  if (CARLA_WHEEL_SUPPORTED_INDEX EQUAL -1)
    message (WARNING "Python ${CARLA_WHEEL_VERSION} is outside the supported range (3.8-3.14); skipping.")
    list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (unsupported)")
    continue ()
  endif ()

  set (CARLA_WHEEL_DIST_DIR "")
  set (CARLA_WHEEL_PYTHON "")

  if (CARLA_WHEEL_VERSION STREQUAL CARLA_WHEEL_CURRENT_VERSION)
    # Current interpreter: reuse the already-configured main build tree.
    set (CARLA_WHEEL_PYTHON "${CARLA_WHEEL_CURRENT_PYTHON}")
    message (STATUS "[carla-python-api-wheels] Building Python ${CARLA_WHEEL_VERSION} wheel (current interpreter, main build tree).")
    execute_process (
      COMMAND ${CMAKE_COMMAND} --build "${CARLA_WHEEL_MAIN_BUILD_DIR}" --target carla-python-api
      RESULT_VARIABLE CARLA_WHEEL_BUILD_RESULT
    )
    if (NOT CARLA_WHEEL_BUILD_RESULT EQUAL 0)
      message (WARNING "Build of carla-python-api failed for Python ${CARLA_WHEEL_VERSION}; skipping.")
      list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (build)")
      continue ()
    endif ()
    set (CARLA_WHEEL_DIST_DIR "${CARLA_WHEEL_MAIN_BUILD_DIR}/PythonAPI/dist")
  else ()
    # Other interpreter: locate it, then nest a configure + build.
    # find_program writes a CACHE entry; resolve into a dedicated cache variable
    # (never assigned as a normal variable) so the empty normal CARLA_WHEEL_PYTHON
    # set above cannot shadow the result and make every lookup read as "missing".
    unset (CARLA_WHEEL_FOUND_PYTHON CACHE)
    find_program (CARLA_WHEEL_FOUND_PYTHON NAMES "python${CARLA_WHEEL_VERSION}")
    if (NOT CARLA_WHEEL_FOUND_PYTHON)
      message (WARNING "python${CARLA_WHEEL_VERSION} not found on PATH; install it or use the CARLA dev container (which ships python3.8-3.14); skipping.")
      list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (interpreter missing)")
      continue ()
    endif ()
    set (CARLA_WHEEL_PYTHON "${CARLA_WHEEL_FOUND_PYTHON}")

    set (CARLA_WHEEL_NESTED_DIR "${CARLA_WHEEL_BINARY_ROOT}/py${CARLA_WHEEL_VERSION}")
    message (STATUS "[carla-python-api-wheels] Building Python ${CARLA_WHEEL_VERSION} wheel (${CARLA_WHEEL_PYTHON}, nested build at ${CARLA_WHEEL_NESTED_DIR}).")

    set (
      CARLA_WHEEL_CONFIGURE_COMMAND
      ${CMAKE_COMMAND}
        -G "${CARLA_WHEEL_GENERATOR}"
        -S "${CARLA_WHEEL_SOURCE_DIR}"
        -B "${CARLA_WHEEL_NESTED_DIR}"
    )
    if (NOT CARLA_WHEEL_TOOLCHAIN STREQUAL "")
      list (APPEND CARLA_WHEEL_CONFIGURE_COMMAND "--toolchain=${CARLA_WHEEL_TOOLCHAIN}")
    endif ()
    if (NOT CARLA_WHEEL_BUILD_TYPE STREQUAL "")
      list (APPEND CARLA_WHEEL_CONFIGURE_COMMAND "-DCMAKE_BUILD_TYPE=${CARLA_WHEEL_BUILD_TYPE}")
    endif ()
    # Pin BOTH find modules to this interpreter. CARLA's PythonAPI uses
    # find_package(Python3) while Boost.Python uses find_package(Python); with
    # several interpreters installed the two resolve different versions unless
    # both are pinned, which leaves the version-specific Boost::python3XY target
    # (named after Boost's pick) out of sync with the target PythonAPI requests.
    list (
      APPEND CARLA_WHEEL_CONFIGURE_COMMAND
      "-DBUILD_CARLA_UNREAL=OFF"
      "-DPython_EXECUTABLE=${CARLA_WHEEL_PYTHON}"
      "-DPython3_EXECUTABLE=${CARLA_WHEEL_PYTHON}"
    )

    execute_process (
      COMMAND ${CARLA_WHEEL_CONFIGURE_COMMAND}
      RESULT_VARIABLE CARLA_WHEEL_CONFIGURE_RESULT
    )
    if (NOT CARLA_WHEEL_CONFIGURE_RESULT EQUAL 0)
      message (WARNING "CMake configure failed for Python ${CARLA_WHEEL_VERSION}; skipping.")
      list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (configure)")
      continue ()
    endif ()

    execute_process (
      COMMAND ${CMAKE_COMMAND} --build "${CARLA_WHEEL_NESTED_DIR}" --target carla-python-api
      RESULT_VARIABLE CARLA_WHEEL_BUILD_RESULT
    )
    if (NOT CARLA_WHEEL_BUILD_RESULT EQUAL 0)
      message (WARNING "Build of carla-python-api failed for Python ${CARLA_WHEEL_VERSION}; skipping.")
      list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (build)")
      continue ()
    endif ()
    set (CARLA_WHEEL_DIST_DIR "${CARLA_WHEEL_NESTED_DIR}/PythonAPI/dist")
  endif ()

  # Locate the produced wheel by its interpreter tag (cp3X) so a stale wheel
  # from another interpreter in the same dist dir is never picked up.
  string (REPLACE "." "" CARLA_WHEEL_CP_TAG "${CARLA_WHEEL_VERSION}")
  file (GLOB CARLA_WHEEL_PRODUCED "${CARLA_WHEEL_DIST_DIR}/carla-*cp${CARLA_WHEEL_CP_TAG}*.whl")
  if (NOT CARLA_WHEEL_PRODUCED)
    message (WARNING "No wheel found for Python ${CARLA_WHEEL_VERSION} in ${CARLA_WHEEL_DIST_DIR}; skipping.")
    list (APPEND CARLA_WHEEL_FAILED "${CARLA_WHEEL_VERSION} (no wheel)")
    continue ()
  endif ()
  list (GET CARLA_WHEEL_PRODUCED 0 CARLA_WHEEL_FILE)

  if (CARLA_WHEEL_MANYLINUX)
    set (CARLA_WHEEL_AUDIT_ARGS "")
    if (NOT CARLA_WHEEL_PLATFORM STREQUAL "")
      list (APPEND CARLA_WHEEL_AUDIT_ARGS --plat "${CARLA_WHEEL_PLATFORM}")
    endif ()
    # auditwheel inspects ELF symbols and rewrites the wheel tag; it never imports
    # the wheel, so it need not run on the wheel's own interpreter. The newest
    # auditwheel that still supports Python 3.8 (6.1.0) defaults to an ancient
    # manylinux_2_5 target and refuses the wheel's modern glibc symbols, while a
    # current interpreter ships a newer auditwheel that auto-selects the right
    # tag. So try the current interpreter's auditwheel first, then fall back to
    # the wheel's own (deduplicated when they are the same interpreter).
    set (CARLA_WHEEL_AUDIT_PYTHONS "${CARLA_WHEEL_CURRENT_PYTHON}" "${CARLA_WHEEL_PYTHON}")
    list (REMOVE_DUPLICATES CARLA_WHEEL_AUDIT_PYTHONS)
    set (CARLA_WHEEL_REPAIRED FALSE)
    foreach (CARLA_WHEEL_AUDIT_PYTHON IN LISTS CARLA_WHEEL_AUDIT_PYTHONS)
      if (CARLA_WHEEL_REPAIRED)
        break ()
      endif ()
      execute_process (
        COMMAND
          "${CARLA_WHEEL_AUDIT_PYTHON}" -m auditwheel repair
          ${CARLA_WHEEL_AUDIT_ARGS}
          --wheel-dir "${CARLA_WHEEL_OUTPUT_DIR}"
          "${CARLA_WHEEL_FILE}"
        RESULT_VARIABLE CARLA_WHEEL_AUDIT_RESULT
      )
      if (CARLA_WHEEL_AUDIT_RESULT EQUAL 0)
        set (CARLA_WHEEL_REPAIRED TRUE)
      endif ()
    endforeach ()
    if (CARLA_WHEEL_REPAIRED)
      # The current interpreter builds its wheel directly into OUTPUT_DIR (the
      # main dist dir), so auditwheel leaves the unrepaired wheel beside the
      # repaired one. Drop it so OUTPUT_DIR holds exactly one wheel per version.
      # Nested interpreters build their unrepaired wheel in a separate dir, so
      # this comparison never removes those.
      get_filename_component (CARLA_WHEEL_SOURCE_WHEEL_DIR "${CARLA_WHEEL_FILE}" DIRECTORY)
      get_filename_component (CARLA_WHEEL_SOURCE_WHEEL_DIR "${CARLA_WHEEL_SOURCE_WHEEL_DIR}" REALPATH)
      get_filename_component (CARLA_WHEEL_OUTPUT_REAL "${CARLA_WHEEL_OUTPUT_DIR}" REALPATH)
      if (CARLA_WHEEL_SOURCE_WHEEL_DIR STREQUAL CARLA_WHEEL_OUTPUT_REAL)
        file (REMOVE "${CARLA_WHEEL_FILE}")
        message (STATUS "[carla-python-api-wheels] Dropped the unrepaired wheel for Python ${CARLA_WHEEL_VERSION}; kept the manylinux build.")
      endif ()
    else ()
      message (WARNING "auditwheel repair unavailable or failed for Python ${CARLA_WHEEL_VERSION}; keeping the unrepaired wheel.")
      file (COPY "${CARLA_WHEEL_FILE}" DESTINATION "${CARLA_WHEEL_OUTPUT_DIR}")
    endif ()
  else ()
    file (COPY "${CARLA_WHEEL_FILE}" DESTINATION "${CARLA_WHEEL_OUTPUT_DIR}")
  endif ()

  list (APPEND CARLA_WHEEL_BUILT "${CARLA_WHEEL_VERSION}")
endforeach ()

file (GLOB CARLA_WHEEL_COLLECTED "${CARLA_WHEEL_OUTPUT_DIR}/carla-*.whl")
message (STATUS "============================================================")
message (STATUS "[carla-python-api-wheels] Output directory: ${CARLA_WHEEL_OUTPUT_DIR}")
foreach (CARLA_WHEEL_ITEM IN LISTS CARLA_WHEEL_COLLECTED)
  get_filename_component (CARLA_WHEEL_ITEM_NAME "${CARLA_WHEEL_ITEM}" NAME)
  message (STATUS "  ${CARLA_WHEEL_ITEM_NAME}")
endforeach ()
if (CARLA_WHEEL_BUILT)
  message (STATUS "[carla-python-api-wheels] Built: ${CARLA_WHEEL_BUILT}")
endif ()
if (CARLA_WHEEL_FAILED)
  message (STATUS "[carla-python-api-wheels] Failed: ${CARLA_WHEEL_FAILED}")
endif ()
message (STATUS "============================================================")

if (NOT CARLA_WHEEL_BUILT)
  message (FATAL_ERROR "carla-python-api-wheels: no wheels were built. Requested: ${CARLA_WHEEL_REQUESTED}. Failed: ${CARLA_WHEEL_FAILED}.")
endif ()
