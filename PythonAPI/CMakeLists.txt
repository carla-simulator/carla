cmake_minimum_required (
  VERSION
  3.28
)

if (CARLA_CONFIG_FILEPATH)
  include (${CARLA_CONFIG_FILEPATH})
else ()
  message (WARNING "Missing \"CARLA_CONFIG_FILEPATH\"")
endif ()

project (
  PythonAPI
  LANGUAGES
    CXX
  VERSION
    ${CARLA_VERSION}
)

option (ENABLE_PEP517 "Whether to use PEP 517." ON)

find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

#[[
# -- Temporary check for numpy 2 presence, which breaks our current version of Boost.Python.
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
  message (WARNING "Failed to query numpy version.")
endif ()
message (STATUS "Found Numpy ${NUMPY_VERSION_STRING} for Python ${Python3_VERSION} (${Python3_EXECUTABLE}).")
if ("${NUMPY_VERSION_STRING}" VERSION_GREATER "2.0.0")
  message (FATAL_ERROR "Unsupported Numpy version, please downgrade to a version prior to numpy 2.")
endif ()
# --
]]

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

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/carla/pyproject.toml.in
  ${CMAKE_CURRENT_SOURCE_DIR}/carla/pyproject.toml
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/carla/CMakeLists.txt.in
  ${CMAKE_CURRENT_SOURCE_DIR}/carla/CMakeLists.txt
  @ONLY
)

set (
  CARLA_PYTHON_API_DIST_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/dist
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
    ${CMAKE_CURRENT_SOURCE_DIR}/carla/setup.py
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

add_custom_target (
  python-api ALL
  COMMAND
    ${CARLA_PYTHON_API_BUILD_COMMAND}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/carla
  VERBATIM
  USES_TERMINAL
)
