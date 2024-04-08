find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

if (WIN32)
  set (PYTHON_MODULE_EXT .pyd)
else ()
  set (PYTHON_MODULE_EXT .so)
endif ()

set (
  CARLA_PYTHON_API_CARLA_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/carla
)

set (
  CARLA_PYTHON_API_CARLA_EXTENSION_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/carla-python-extension
)

carla_message (
  "Selected Python executable: ${Python3_EXECUTABLE} (Interpreter ID: ${Python3_INTERPRETER_ID})"
)

set (
  CARLA_PYTHON_API_DIST_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/dist
)

set (
  CARLA_PYTHON_API_PACKAGE_WHEEL_PATH
   # @TODO THIS PATH IS ALMOST SURELY WRONG
  ${CARLA_PYTHON_API_DIST_PATH}/carla-${CARLA_VERSION}-py3-none-any.whl
)

set (
  CARLA_PYTHON_API_INCLUDE_PATH
  ${CARLA_PYTHON_API_CARLA_EXTENSION_PATH}/include
)

set (
  CARLA_PYTHON_API_SOURCE_PATH
  ${CARLA_PYTHON_API_CARLA_EXTENSION_PATH}/src
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
  PYTHON_API_SOURCES
  ${CARLA_PYTHON_API_SOURCE_PATH}/PythonAPI.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Actor.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Blueprint.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Client.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Commands.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Control.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Exception.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Geometry.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/LightManager.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Map.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Sensor.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/SensorData.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Snapshot.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/TrafficManager.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Weather.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/World.cpp
)

if (ENABLE_OSM2ODR)
  list (
    APPEND
    PYTHON_API_SOURCES
    ${CARLA_PYTHON_API_SOURCE_PATH}/OSM2ODR.cpp
  )
endif ()

if (ENABLE_RSS)
  list (
    APPEND
    PYTHON_API_SOURCES
    ${CARLA_PYTHON_API_SOURCE_PATH}/AdRss.cpp
  )
endif ()

set (
  USE_LIMITED_API
  OFF
)

set (
  PYTHON_API_ADD_LIBRARY_ARGS
)

if (USE_LIMITED_API)
  list (
    APPEND
    PYTHON_API_ADD_LIBRARY_ARGS
    USE_SABI ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}
  )
endif ()

list (
  APPEND
  PYTHON_API_ADD_LIBRARY_ARGS
  WITH_SOABI
)

Python3_add_library (
  carla-python-api-extension
  MODULE
  ${PYTHON_API_ADD_LIBRARY_ARGS}
  ${PYTHON_API_SOURCES}
)

if (BUILD_PYTHON_API_WHEEL_PACKAGE)

  add_custom_command (
    TARGET carla-python-api-extension POST_BUILD
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
      ${CARLA_PYTHON_API_BUILD_COMMAND}
    WORKING_DIRECTORY
      ${CARLA_PYTHON_API_CARLA_PATH}
    VERBATIM
    USES_TERMINAL
  )

  add_custom_target (
    carla-python-api-install
    COMMAND
      ${Python3_EXECUTABLE}
        -m pip install
        -e ${CARLA_PYTHON_API_CARLA_PATH}
    WORKING_DIRECTORY
      ${CARLA_PYTHON_API_CARLA_PATH}
    VERBATIM
    USES_TERMINAL
  )

endif ()

target_include_directories (
  carla-python-api-extension PRIVATE
  ${CARLA_PYTHON_API_INCLUDE_PATH}
)

target_compile_definitions (
  carla-python-api-extension PRIVATE
  BOOST_ALL_NO_LIB
  LIBCARLA_WITH_PYTHON_SUPPORT
)

target_link_libraries (
  carla-python-api-extension PRIVATE
  ${CARLA_PYTHON_API_DEPENDENCIES}
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/MANIFEST.in
  ${CARLA_PYTHON_API_CARLA_PATH}/MANIFEST.in.in
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml
  ${CARLA_PYTHON_API_CARLA_PATH}/pyproject.toml.in
)
