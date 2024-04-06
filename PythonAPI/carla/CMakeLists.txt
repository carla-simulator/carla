project (
  carla-python-api
  VERSION ${CARLA_VERSION}
  DESCRIPTION
    "Python API for communicating with the CARLA server."
)

add_custom_target (
  carla-python-api
  COMMAND
    ${CMAKE_COMMAND}
    -E rm
    -rf
    ${CARLA_PYTHON_API_CARLA_PATH}/carla
  COMMAND
    ${CMAKE_COMMAND}
      -E copy
      ${CARLA_WORKSPACE_PATH}/LICENSE
      ${CARLA_PYTHON_API_CARLA_PATH}/LICENSE
  COMMAND
    ${CMAKE_COMMAND}
      -E copy
      $<TARGET_FILE:carla-python-api-extension>
      ${CARLA_PYTHON_API_CARLA_PATH}/carla/carla_ext${PYTHON_MODULE_EXT}
  COMMAND
    ${Python3_EXECUTABLE} -m build --wheel
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_CARLA_PATH}
  USES_TERMINAL
  VERBATIM
)

carla_two_step_configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.in
  ${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.in.in
)

carla_two_step_configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml
  ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml.in
)
