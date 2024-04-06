project (
  carla-python-api-extension
  LANGUAGES CXX
  VERSION ${CARLA_VERSION}
  DESCRIPTION
    "C++ extension for the CARLA Python API."
)

carla_message ("Selected Python executable: ${Python3_EXECUTABLE} (${Python3_INTERPRETER_ID})")

set (CARLA_PYTHON_API_EXTENSION_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (CARLA_PYTHON_API_INCLUDE_PATH ${CARLA_PYTHON_API_EXTENSION_PATH}/include)
set (CARLA_PYTHON_API_SOURCE_PATH ${CARLA_PYTHON_API_EXTENSION_PATH}/src)

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

Python3_add_library (
  carla-python-api-extension
  MODULE
  WITH_SOABI
  ${PYTHON_API_SOURCES}
)

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
