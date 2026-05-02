find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

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
  carla_warning ("Failed to query numpy version.")
endif ()
carla_message ("Found Numpy ${NUMPY_VERSION_STRING} for Python ${Python3_VERSION} (${Python3_EXECUTABLE}).")
if ("${NUMPY_VERSION_STRING}" VERSION_GREATER "2.0.0")
  carla_error ("Unsupported Numpy version, please downgrade to a version prior to numpy 2.")
endif ()
# --

set (
  PYTHON_API_DEPENDENCIES
  carla-client
  rpc
  png_static
  zlibstatic
  Boost::asio
  Boost::python
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
