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

set (CARLA_PYTHON_API_CARLA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/carla)

add_subdirectory (carla-python-extension)
add_subdirectory (carla)
