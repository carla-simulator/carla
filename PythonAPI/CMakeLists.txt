find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

execute_process (
  COMMAND
    ${Python3_EXECUTABLE}
      -c
      "import numpy; print(numpy.__version__)"
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE
    NUMPY_VERSION_STRING
  RESULT_VARIABLE
    NUMPY_VERSION_QUERY_RESULT
)
if (NUMPY_VERSION_QUERY_RESULT)
  carla_warning ("Failed to query numpy version.")
endif ()
carla_message ("Found Numpy ${NUMPY_VERSION_STRING} for Python ${Python3_VERSION} (${Python3_EXECUTABLE}).")

set (
  PYTHON_API_DEPENDENCIES
  carla-client
  rpc
  png_static
  zlibstatic
  Boost::asio
  Boost::python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}
  Boost::geometry
  Boost::gil
  RecastNavigation::Recast
  RecastNavigation::Detour
  RecastNavigation::DetourCrowd
)

get_target_property (
  Boost_asio_TARGET_NAME
  Boost::asio
  ALIASED_TARGET
)

get_target_property (
  Boost_python_TARGET_NAME
  Boost::python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}
  ALIASED_TARGET
)

get_target_property (
  Boost_geometry_TARGET_NAME
  Boost::geometry
  ALIASED_TARGET
)

get_target_property (
  Boost_gil_TARGET_NAME
  Boost::gil
  ALIASED_TARGET
)

get_target_property (
  RecastNavigation_Recast_TARGET_NAME
  RecastNavigation::Recast
  ALIASED_TARGET
)

get_target_property (
  RecastNavigation_Detour_TARGET_NAME
  RecastNavigation::Detour
  ALIASED_TARGET
)

get_target_property (
  RecastNavigation_DetourCrowd_TARGET_NAME
  RecastNavigation::DetourCrowd
  ALIASED_TARGET
)

set (
  CARLA_PYTHON_API_CARLA_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/carla
)

set (
  CARLA_PYTHON_API_CMAKE_ARGS
  "\t'-G${CMAKE_GENERATOR}'"
)

set (
  CARLA_PYTHON_API_CMAKE_ARGS
  "${CARLA_PYTHON_API_CMAKE_ARGS},\n\t'-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}'"
)

set (
  CARLA_PYTHON_API_CMAKE_ARGS
  "${CARLA_PYTHON_API_CMAKE_ARGS},\n\t'-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}'"
)

if (CMAKE_TOOLCHAIN_FILE)
  set (
    CARLA_PYTHON_API_CMAKE_ARGS
    "${CARLA_PYTHON_API_CMAKE_ARGS},\n\t\'--toolchain=${CMAKE_TOOLCHAIN_FILE}\'"
  )
endif ()

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml.in
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/CMakeLists.txt
  ${CARLA_PYTHON_API_CARLA_PATH}/CMakeLists.txt.in
)

set (
  CARLA_PYTHON_API_DIST_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/dist
)

file (
  COPY
  ${CARLA_WORKSPACE_PATH}/LICENSE
  DESTINATION ${CARLA_PYTHON_API_CARLA_PATH}
)

if (ENABLE_PEP517)
  set (
    CARLA_PYTHON_API_BUILD_COMMAND
    ${Python3_EXECUTABLE}
    -m build
    --outdir ${CARLA_PYTHON_API_DIST_PATH}
    --wheel
  )
else ()
  file (
    WRITE
    ${CARLA_PYTHON_API_CARLA_PATH}/setup.py
    "from setuptools import setup\n"
    "setup()\n"
  )
  set (
    CARLA_PYTHON_API_BUILD_COMMAND
    ${Python3_EXECUTABLE}
    setup.py
    bdist_wheel
    --dist-dir ${CARLA_PYTHON_API_DIST_PATH}
  )
endif ()

carla_add_custom_target (
  carla-python-api
  "Build the CARLA Python API."
  COMMAND
    ${CMAKE_COMMAND}
      -E copy
      ${CARLA_WORKSPACE_PATH}/LICENSE
      ${CARLA_PYTHON_API_CARLA_PATH}/LICENSE
  COMMAND
    ${CARLA_PYTHON_API_BUILD_COMMAND}
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_CARLA_PATH}
  VERBATIM
  USES_TERMINAL
)

carla_add_custom_target (
  carla-python-api-install
  "Build & install the CARLA Python API"
  COMMAND
    ${Python3_EXECUTABLE}
      -m pip install
      -e ${CARLA_PYTHON_API_CARLA_PATH}
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_CARLA_PATH}
  VERBATIM
  USES_TERMINAL
)

add_dependencies (
  carla-python-api
  ${PYTHON_API_DEPENDENCIES}
)

add_dependencies (
  carla-python-api-install
  carla-python-api
)

# Build the CARLA Python API wheel for one or more interpreters. With
# CARLA_PYTHON_API_VERSIONS empty (default) this builds only the current
# interpreter by reusing this build tree (identical to carla-python-api). When a
# version list is given, each non-current version is built in its own nested
# configure (the wheel links a version-specific Boost.Python target, so a
# separate outer configure per interpreter is required). The per-version loop,
# interpreter discovery and optional auditwheel repair live in the CMake
# script-mode module below; missing/out-of-range versions are reported but do
# not stop the run, which fails only if no wheel could be built.
#
# Pass the version list with ',' separators: a ';'-separated CMake list would be
# split into separate arguments inside the custom-command COMMAND. The script
# normalizes ',' back to ';'.
string (REPLACE ";" "," CARLA_PYTHON_API_VERSIONS_ARG "${CARLA_PYTHON_API_VERSIONS}")

carla_add_custom_target (
  carla-python-api-wheels
  "Build the CARLA Python API wheel for the interpreters in CARLA_PYTHON_API_VERSIONS (default: the current one)."
  COMMAND
    ${CMAKE_COMMAND}
      -D CARLA_WHEEL_VERSIONS=${CARLA_PYTHON_API_VERSIONS_ARG}
      -D CARLA_WHEEL_CURRENT_VERSION=${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}
      -D CARLA_WHEEL_CURRENT_PYTHON=${Python3_EXECUTABLE}
      -D CARLA_WHEEL_SOURCE_DIR=${CARLA_WORKSPACE_PATH}
      -D CARLA_WHEEL_MAIN_BUILD_DIR=${CMAKE_BINARY_DIR}
      -D CARLA_WHEEL_BINARY_ROOT=${CMAKE_BINARY_DIR}/python-wheels
      -D CARLA_WHEEL_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/dist
      -D CARLA_WHEEL_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -D CARLA_WHEEL_GENERATOR=${CMAKE_GENERATOR}
      -D CARLA_WHEEL_TOOLCHAIN=${CMAKE_TOOLCHAIN_FILE}
      -D CARLA_WHEEL_MANYLINUX=${CARLA_PYTHON_API_WHEEL_MANYLINUX}
      -D CARLA_WHEEL_PLATFORM=${CARLA_PYTHON_API_WHEEL_PLATFORM}
      -P ${CARLA_WORKSPACE_PATH}/CMake/BuildPythonApiWheels.cmake
  USES_TERMINAL
  VERBATIM
)
