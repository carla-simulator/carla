set (CARLA_PYTHON_API_PATH ${CARLA_WORKSPACE_PATH}/PythonAPI)
set (CARLA_PYTHON_API_CARLA_PATH ${CARLA_PYTHON_API_PATH}/carla)

find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

set (CARLA_PYTHON_API_LINK_DEPENDENCIES_FLAT)
macro (carla_enumerate_link_dependencies TARGET_NAME)
  get_target_property (TARGET_TYPE ${TARGET_NAME} TYPE)
  if (("${TARGET_TYPE}" STREQUAL "STATIC_LIBRARY") OR ("${TARGET_TYPE}" STREQUAL "SHARED_LIBRARY"))
    list (APPEND CARLA_PYTHON_API_LINK_DEPENDENCIES_FLAT ${TARGET_NAME})
  endif ()
  get_target_property (
    TARGET_LINK_DEPENDENCIES
    ${TARGET_NAME}
    INTERFACE_LINK_LIBRARIES
  )
  foreach (DEP ${TARGET_LINK_DEPENDENCIES})
    if (TARGET ${DEP})
      carla_enumerate_link_dependencies (${DEP})
    endif ()
  endforeach ()
endmacro ()

set (
  CARLA_PYTHON_API_DEPENDENCIES
  carla-client
  Boost::asio
  Boost::python
  Boost::geometry
  Boost::gil
  RecastNavigation::Recast
  RecastNavigation::Detour
  RecastNavigation::DetourCrowd
  png_static
  zlibstatic
  rpc
)

set (
  CARLA_PYTHON_API_INCLUDE_LIST_FILE_PATH
  ${CARLA_PYTHON_API_CARLA_PATH}/Include.txt
)

set (
  CARLA_PYTHON_API_LINK_LIST_FILE_PATH
  ${CARLA_PYTHON_API_CARLA_PATH}/Link.txt
)

foreach (CARLA_PYTHON_API_DEPENDENCY ${CARLA_PYTHON_API_DEPENDENCIES})
  carla_enumerate_link_dependencies (${CARLA_PYTHON_API_DEPENDENCY})
endforeach ()
list (REMOVE_DUPLICATES CARLA_PYTHON_API_LINK_DEPENDENCIES_FLAT)

set (
  CARLA_PYTHON_API_INCLUDE_PATHS
  # @TODO HACK
  ${libpng_SOURCE_DIR} ${libpng_BINARY_DIR}
  ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR}
)
foreach (DEPENDENCY ${CARLA_PYTHON_API_DEPENDENCIES})
  list (APPEND CARLA_PYTHON_API_INCLUDE_PATHS "$<TARGET_PROPERTY:${DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>")
endforeach ()
list (REMOVE_DUPLICATES CARLA_PYTHON_API_INCLUDE_PATHS)

set (CARLA_PYTHON_API_INCLUDE_LIST_CONTENT ${CARLA_PYTHON_API_INCLUDE_PATHS})
set (CARLA_PYTHON_API_LINK_LIST_CONTENT)
foreach (DEPENDENCY ${CARLA_PYTHON_API_LINK_DEPENDENCIES_FLAT})
  list (APPEND CARLA_PYTHON_API_LINK_LIST_CONTENT "$<TARGET_FILE:${DEPENDENCY}>")
endforeach ()

file (WRITE ${CARLA_PYTHON_API_INCLUDE_LIST_FILE_PATH} "${CARLA_PYTHON_API_INCLUDE_LIST_CONTENT}")
file (WRITE ${CARLA_PYTHON_API_LINK_LIST_FILE_PATH} "${CARLA_PYTHON_API_LINK_LIST_CONTENT}")

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_INCLUDE_LIST_FILE_PATH}
  ${CARLA_PYTHON_API_INCLUDE_LIST_FILE_PATH}
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_LINK_LIST_FILE_PATH}
  ${CARLA_PYTHON_API_LINK_LIST_FILE_PATH}
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/CMakeLists.txt
  ${CARLA_PYTHON_API_CARLA_PATH}/CMakeLists.txt.in
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml.in
)

if (WIN32)
  set (PYTHON_MODULE_EXT .pyd)
else ()
  set (PYTHON_MODULE_EXT .so)
endif ()

add_custom_target (
  carla-python-api
  COMMAND
    ${Python3_EXECUTABLE} -m build
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_CARLA_PATH}
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-python-api-install
  COMMAND
    ${Python3_EXECUTABLE} -m pip install -e .
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_PATH}
  USES_TERMINAL
  VERBATIM
)
