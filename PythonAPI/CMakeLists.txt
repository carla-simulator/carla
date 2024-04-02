set (PYTHON_API_PATH ${CARLA_WORKSPACE_PATH}/PythonAPI)
set (PYTHON_API_CARLA_PATH ${PYTHON_API_PATH}/carla)

carla_two_step_configure_file (
  ${PYTHON_API_CARLA_PATH}/CMakeLists.txt
  ${PYTHON_API_CARLA_PATH}/CMakeLists.txt.in
)

carla_two_step_configure_file (
  ${PYTHON_API_CARLA_PATH}/pyproject.toml
  ${PYTHON_API_CARLA_PATH}/pyproject.toml.in
)

if (WIN32)
  set (PYTHON_MODULE_EXT .pyd)
else ()
  set (PYTHON_MODULE_EXT .so)
endif ()

set (
  CARLA_PYTHON_API_SYMLINK_PATH
  ${PYTHON_API_PATH}/examples/carla${PYTHON_MODULE_EXT}
)

set (
  CARLA_PYTHON_API_PACKAGE_PATH
  ${PYTHON_API_PATH}/dist/carla-${CARLA_VERSION}-py${Python3_VERSION_MAJOR}-none-any.whl
)

add_custom_command (
  OUTPUT
    ${CARLA_PYTHON_API_PACKAGE_PATH}
  COMMAND ${Python3_EXECUTABLE} -m build
  WORKING_DIRECTORY
    ${PYTHON_API_PATH}
  COMMENT
    "Creating CARLA Python API package...\n"
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-python-api-package
  DEPENDS
    ${CARLA_PYTHON_API_PACKAGE_PATH}
)

add_custom_target (
  carla-python-api-install
  COMMAND ${Python3_EXECUTABLE} -m pip install -e .
  WORKING_DIRECTORY
    ${PYTHON_API_PATH}
  COMMENT
    "Installing CARLA Python API...\n"
  USES_TERMINAL
  VERBATIM
)
