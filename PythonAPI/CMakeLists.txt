project (
  carla-python-api
  LANGUAGES CXX
  VERSION ${CARLA_VERSION}
)

find_package (
  Python
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
  REQUIRED
)
    
set (PYTHON_API_PATH ${CARLA_WORKSPACE_PATH}/PythonAPI)
set (PYTHON_API_CARLA_PATH ${PYTHON_API_PATH}/carla)
set (PYTHON_API_SOURCE_PATH ${PYTHON_API_CARLA_PATH}/source/libcarla)

file (
  GENERATE
  OUTPUT ${PYTHON_API_PATH}/examples/util/find_carla.py
  INPUT ${PYTHON_API_PATH}/examples/util/find_carla.py.in
)

file (
  GENERATE
  OUTPUT ${PYTHON_API_PATH}/carla/pyproject.toml
  INPUT ${PYTHON_API_PATH}/carla/pyproject.toml.in
)

set (
  PYTHON_API_SOURCES
  ${PYTHON_API_SOURCE_PATH}/PythonAPI.cpp
  ${PYTHON_API_SOURCE_PATH}/PythonAPI.h
  ${PYTHON_API_SOURCE_PATH}/Actor.cpp
  ${PYTHON_API_SOURCE_PATH}/Blueprint.cpp
  ${PYTHON_API_SOURCE_PATH}/Client.cpp
  ${PYTHON_API_SOURCE_PATH}/Commands.cpp
  ${PYTHON_API_SOURCE_PATH}/Control.cpp
  ${PYTHON_API_SOURCE_PATH}/Exception.cpp
  ${PYTHON_API_SOURCE_PATH}/Geom.cpp
  ${PYTHON_API_SOURCE_PATH}/LightManager.cpp
  ${PYTHON_API_SOURCE_PATH}/Map.cpp
  ${PYTHON_API_SOURCE_PATH}/OSM2ODR.cpp
  ${PYTHON_API_SOURCE_PATH}/Sensor.cpp
  ${PYTHON_API_SOURCE_PATH}/SensorData.cpp
  ${PYTHON_API_SOURCE_PATH}/Snapshot.cpp
  ${PYTHON_API_SOURCE_PATH}/TrafficManager.cpp
  ${PYTHON_API_SOURCE_PATH}/Weather.cpp
  ${PYTHON_API_SOURCE_PATH}/World.cpp
)

if (ENABLE_RSS)
  list (APPEND PYTHON_API_SOURCES ${PYTHON_API_SOURCE_PATH}/AdRss.cpp)
endif ()

Python_add_library (
  carla-python-api MODULE WITH_SOABI
  ${PYTHON_API_SOURCES}
)

target_link_libraries (
  carla-python-api PRIVATE
  carla-client
  Boost::python
  png_static
  zlibstatic
)

target_compile_definitions (
  carla-python-api PRIVATE
  ${CARLA_COMMON_DEFINITIONS}
  ${CARLA_RTTI_DEFINITIONS}
  BOOST_ALL_NO_LIB
  BOOST_PYTHON_STATIC_LIB
  LIBCARLA_WITH_PYTHON_SUPPORT
)

target_include_directories (
  carla-python-api PRIVATE
  ${PYTHON_API_SOURCE_PATH}
  ${LIBCARLA_SOURCE_PATH}
)