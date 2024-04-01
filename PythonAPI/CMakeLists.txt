project (
  carla-python-api-module
  VERSION
    ${CARLA_VERSION}
  LANGUAGES
    CXX
  DESCRIPTION
    "Python API for communicating with the CARLA server."
)

set (PYTHON_API_PATH ${CARLA_WORKSPACE_PATH}/PythonAPI)
set (PYTHON_API_CARLA_PATH ${PYTHON_API_PATH}/carla)

carla_two_step_configure_file (
  ${PYTHON_API_PATH}/carla/pyproject.toml
  ${PYTHON_API_PATH}/carla/pyproject.toml.in
)

set (
  PYTHON_API_SOURCES
  ${PYTHON_API_CARLA_PATH}/src/PythonAPI.cpp
  ${PYTHON_API_CARLA_PATH}/src/Actor.cpp
  ${PYTHON_API_CARLA_PATH}/src/Blueprint.cpp
  ${PYTHON_API_CARLA_PATH}/src/Client.cpp
  ${PYTHON_API_CARLA_PATH}/src/Commands.cpp
  ${PYTHON_API_CARLA_PATH}/src/Control.cpp
  ${PYTHON_API_CARLA_PATH}/src/Exception.cpp
  ${PYTHON_API_CARLA_PATH}/src/Geometry.cpp
  ${PYTHON_API_CARLA_PATH}/src/LightManager.cpp
  ${PYTHON_API_CARLA_PATH}/src/Map.cpp
  ${PYTHON_API_CARLA_PATH}/src/Sensor.cpp
  ${PYTHON_API_CARLA_PATH}/src/SensorData.cpp
  ${PYTHON_API_CARLA_PATH}/src/Snapshot.cpp
  ${PYTHON_API_CARLA_PATH}/src/TrafficManager.cpp
  ${PYTHON_API_CARLA_PATH}/src/Weather.cpp
  ${PYTHON_API_CARLA_PATH}/src/World.cpp
)

if (ENABLE_OSM2ODR)
  list (APPEND PYTHON_API_SOURCES ${PYTHON_API_CARLA_PATH}/src/OSM2ODR.cpp)
endif ()

if (ENABLE_RSS)
  list (APPEND PYTHON_API_SOURCES ${PYTHON_API_CARLA_PATH}/src/AdRss.cpp)
endif ()

find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

carla_message (
  "Python Interpreter ID: \"${Python3_INTERPRETER_ID}\""
)

Python3_add_library (
  carla-python-api-module
  MODULE
  WITH_SOABI
  ${PYTHON_API_SOURCES}
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

install (
  TARGETS carla-python-api-module
  DESTINATION bin
)

add_custom_command (
  TARGET
    carla-python-api-module
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND}
      -E
      create_symlink
      $<TARGET_FILE:carla-python-api-module>
      ${CARLA_PYTHON_API_SYMLINK_PATH}
)

target_include_directories (
  carla-python-api-module PRIVATE
  ${PYTHON_API_CARLA_PATH}/include
)

target_link_libraries (
  carla-python-api-module PRIVATE
  carla-client
  Boost::asio
  Boost::algorithm
  Boost::geometry
  Boost::python
  Boost::assert
  Boost::gil
  RecastNavigation::Recast
  RecastNavigation::Detour
  RecastNavigation::DetourCrowd
  png_static
  zlibstatic
  rpc
)

target_compile_definitions (
  carla-python-api-module PRIVATE
  ${CARLA_COMMON_DEFINITIONS}
  ${CARLA_RTTI_DEFINITIONS}
  BOOST_ALL_NO_LIB
  LIBCARLA_WITH_PYTHON_SUPPORT
)

target_include_directories (
  carla-python-api-module PRIVATE
  ${PYTHON_API_CARLA_PATH}/src
  ${LIBCARLA_SOURCE_PATH}
)

set (
  CARLA_PYTHON_API_PACKAGE_PATH
  ${PYTHON_API_CARLA_PATH}/dist/carla-${CARLA_VERSION}-py${Python3_VERSION_MAJOR}-none-any.whl
)

add_custom_command (
  OUTPUT
    ${CARLA_PYTHON_API_PACKAGE_PATH}
  COMMAND
    ${CMAKE_COMMAND}
    ARGS
      -E copy
        $<TARGET_FILE:carla-python-api-module>
        ${PYTHON_API_CARLA_PATH}/carla${PYTHON_MODULE_EXT}
  COMMAND ${Python3_EXECUTABLE} -m build
  WORKING_DIRECTORY
    ${PYTHON_API_CARLA_PATH}
  COMMENT
    "Creating CARLA Python API package..."
  USES_TERMINAL
  VERBATIM
)

add_custom_target (
  carla-python-api-package
  DEPENDS
    ${CARLA_PYTHON_API_PACKAGE_PATH}
)

add_dependencies (
  carla-python-api-package
  carla-python-api-module
)

add_custom_target (
  carla-python-api-install
  COMMAND ${Python3_EXECUTABLE} -m install
  WORKING_DIRECTORY
    ${PYTHON_API_CARLA_PATH}
  COMMENT
    "Installing CARLA Python API..."
  USES_TERMINAL
  VERBATIM
)