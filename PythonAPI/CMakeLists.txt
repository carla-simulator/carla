find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

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

set (
  PYTHON_API_DEPENDENCIES
  carla-client
  rpc
  png_static
  zlibstatic
  Boost::asio
  Boost::python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}
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

set (CARLA_PYTHON_API_MAIN_VERSION ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})

set (
  CARLA_PYTHON_API_CARLA_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/carla
)

set (
  CARLA_PYTHON_API_BUILD_INFO
  ${CMAKE_CURRENT_BINARY_DIR}/carla-python-api-build-info.cmake
)

carla_two_step_configure_file (
  ${CARLA_PYTHON_API_BUILD_INFO}
  ${CMAKE_CURRENT_SOURCE_DIR}/carla-python-api-build-info.cmake.in
)

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
  CACHE PATH
  "Directory the carla-python-api target writes the wheels to."
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

# Everything specific to this build reaches the wheel build through its
# environment, not through files in the source tree:
set (
  CARLA_PYTHON_API_BUILD_ENV
  CMAKE_GENERATOR=${CMAKE_GENERATOR}
  SKBUILD_CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  SKBUILD_CMAKE_DEFINE=CARLA_PYTHON_API_BUILD_INFO=${CARLA_PYTHON_API_BUILD_INFO}
)

carla_add_custom_target (
  carla-python-api
  "Build the CARLA Python API."
  COMMAND
    ${CMAKE_COMMAND}
      -E copy
      ${CARLA_WORKSPACE_PATH}/LICENSE
      ${CARLA_PYTHON_API_CARLA_PATH}/LICENSE
  COMMAND
    ${CMAKE_COMMAND}
      -E env ${CARLA_PYTHON_API_BUILD_ENV}
      ${CARLA_PYTHON_API_BUILD_COMMAND}
  WORKING_DIRECTORY
    ${CARLA_PYTHON_API_CARLA_PATH}
  VERBATIM
  USES_TERMINAL
)

# carla-python-api-install installs into the active Python (the python3 in
# PATH, e.g. the activated conda or virtual environment): an editable install
# when it is the build's main Python version, the version's wheel from the dist
# directory when it is one of CARLA_PYTHON_VERSIONS, and an error otherwise,
# rather than installing a wheel built for another Python.
set (
  CARLA_PYTHON_API_INSTALL_SCRIPT
  ${CMAKE_CURRENT_BINARY_DIR}/carla-python-api-install.py
)

file (
  WRITE
  ${CARLA_PYTHON_API_INSTALL_SCRIPT}
  [=[
import glob
import os
import re
import subprocess
import sys

main_version, source_path, dist_path = sys.argv[1:4]
version = '%d.%d' % sys.version_info[:2]
tag = 'cp%d%d' % sys.version_info[:2]

if version == main_version:
    command = ['-m', 'pip', 'install', '-e', source_path]
else:
    wheels = {}
    for wheel in glob.glob(os.path.join(dist_path, 'carla-*.whl')):
        match = re.match(r'carla-[^-]+-cp(\d)(\d+)-', os.path.basename(wheel))
        if match:
            wheels['%s.%s' % match.groups()] = wheel
    if version not in wheels:
        available = sorted(set(wheels) | {main_version}, key=lambda v: tuple(map(int, v.split('.'))))
        sys.exit(
            'error: the active Python is %s (%s), but the CARLA Python API is built for Python %s. '
            'Activate one of those, or add %s to CARLA_PYTHON_VERSIONS and rebuild.'
            % (version, sys.executable, ', '.join(available), version))
    command = ['-m', 'pip', 'install', '--force-reinstall', '--no-deps', wheels[version]]

print('Installing the CARLA Python API for Python %s into %s.' % (version, sys.executable))
sys.exit(subprocess.call([sys.executable] + command))
]=]
)

if (WIN32)
  set (CARLA_PYTHON_API_ACTIVE_PYTHON python)
else ()
  set (CARLA_PYTHON_API_ACTIVE_PYTHON python3)
endif ()

carla_add_custom_target (
  carla-python-api-install
  "Build & install the CARLA Python API into the active Python"
  COMMAND
    ${CMAKE_COMMAND}
      -E env ${CARLA_PYTHON_API_BUILD_ENV}
      ${CARLA_PYTHON_API_ACTIVE_PYTHON}
      ${CARLA_PYTHON_API_INSTALL_SCRIPT}
      ${CARLA_PYTHON_API_MAIN_VERSION}
      ${CARLA_PYTHON_API_CARLA_PATH}
      ${CARLA_PYTHON_API_DIST_PATH}
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



# ==== EXTRA PYTHON VERSIONS ====
# Boost.Python builds for a single Python per configure, so each version of
# CARLA_PYTHON_VERSIONS other than the one found above gets its own build of
# this project, reduced to the client and the Python API, that writes its
# wheel to the same dist directory. They run one after another and before the
# main wheel, since all of them build from PythonAPI/carla.


if (CARLA_PYTHON_VERSIONS)
  include (ExternalProject)

  # Reuse the dependency sources this build already fetched:
  set (CARLA_PYTHON_API_FETCHCONTENT_ARGS)
  foreach (DEPENDENCY sqlite3 zlib libpng boost eigen rpclib recastnavigation)
    if (DEFINED ${DEPENDENCY}_SOURCE_DIR)
      string (TOUPPER ${DEPENDENCY} DEPENDENCY_UPPERCASE)
      list (
        APPEND
        CARLA_PYTHON_API_FETCHCONTENT_ARGS
        -DFETCHCONTENT_SOURCE_DIR_${DEPENDENCY_UPPERCASE}=${${DEPENDENCY}_SOURCE_DIR}
      )
    endif ()
  endforeach ()

  # List-valued arguments travel with "|" as separator (LIST_SEPARATOR below):
  string (REPLACE ";" "|" CARLA_PYTHON_API_IMAGE_FORMATS "${LIBCARLA_IMAGE_SUPPORTED_FORMATS}")

  set (CARLA_PYTHON_API_PREVIOUS_TARGET)
  set (CARLA_PYTHON_API_EXTRA_VERSIONS)
endif ()

foreach (EXTRA_PYTHON_VERSION ${CARLA_PYTHON_VERSIONS})
  if (NOT EXTRA_PYTHON_VERSION MATCHES "^[0-9]+\\.[0-9]+$")
    carla_error (
      "Invalid entry \"${EXTRA_PYTHON_VERSION}\" in CARLA_PYTHON_VERSIONS, "
      "expected a Python version like 3.11."
    )
  endif ()
  if (EXTRA_PYTHON_VERSION VERSION_EQUAL CARLA_PYTHON_API_MAIN_VERSION OR
      EXTRA_PYTHON_VERSION IN_LIST CARLA_PYTHON_API_EXTRA_VERSIONS)
    continue ()
  endif ()
  list (APPEND CARLA_PYTHON_API_EXTRA_VERSIONS ${EXTRA_PYTHON_VERSION})

  string (REPLACE "." "" EXTRA_PYTHON_VERSION_TAG ${EXTRA_PYTHON_VERSION})
  set (EXTRA_PYTHON_EXECUTABLE_VAR CARLA_PYTHON_${EXTRA_PYTHON_VERSION_TAG}_EXECUTABLE)

  find_program (
    ${EXTRA_PYTHON_EXECUTABLE_VAR}
    NAMES python${EXTRA_PYTHON_VERSION}
    DOC "Python ${EXTRA_PYTHON_VERSION} interpreter to build the Python API with (see CARLA_PYTHON_VERSIONS)."
  )
  if (NOT ${EXTRA_PYTHON_EXECUTABLE_VAR})
    carla_error (
      "CARLA_PYTHON_VERSIONS lists Python ${EXTRA_PYTHON_VERSION} but python${EXTRA_PYTHON_VERSION} "
      "is not in PATH. Set ${EXTRA_PYTHON_EXECUTABLE_VAR} to its interpreter."
    )
  endif ()
  set (EXTRA_PYTHON_EXECUTABLE ${${EXTRA_PYTHON_EXECUTABLE_VAR}})

  execute_process (
    COMMAND
      ${EXTRA_PYTHON_EXECUTABLE}
        -c
        "import sys; print('%d.%d' % sys.version_info[:2])"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE
      EXTRA_PYTHON_EXECUTABLE_VERSION
    RESULT_VARIABLE
      EXTRA_PYTHON_QUERY_RESULT
  )
  if (EXTRA_PYTHON_QUERY_RESULT OR
      NOT EXTRA_PYTHON_EXECUTABLE_VERSION VERSION_EQUAL EXTRA_PYTHON_VERSION)
    carla_error (
      "${EXTRA_PYTHON_EXECUTABLE_VAR} (${EXTRA_PYTHON_EXECUTABLE}) is Python "
      "\"${EXTRA_PYTHON_EXECUTABLE_VERSION}\", expected ${EXTRA_PYTHON_VERSION}."
    )
  endif ()

  execute_process (
    COMMAND
      ${EXTRA_PYTHON_EXECUTABLE}
        -c
        "import numpy"
    RESULT_VARIABLE
      NUMPY_QUERY_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
  )
  if (NUMPY_QUERY_RESULT)
    carla_warning ("numpy is not installed for Python ${EXTRA_PYTHON_VERSION} (${EXTRA_PYTHON_EXECUTABLE}).")
  endif ()
  carla_message ("Building the Python API for Python ${EXTRA_PYTHON_VERSION} too (${EXTRA_PYTHON_EXECUTABLE}).")

  set (EXTRA_PYTHON_API_TARGET carla-python-api-${EXTRA_PYTHON_VERSION_TAG})
  set (EXTRA_PYTHON_API_BINARY_DIR ${CMAKE_BINARY_DIR}/PythonAPI-${EXTRA_PYTHON_VERSION_TAG})

  ExternalProject_Add (
    ${EXTRA_PYTHON_API_TARGET}
    SOURCE_DIR ${CARLA_WORKSPACE_PATH}
    BINARY_DIR ${EXTRA_PYTHON_API_BINARY_DIR}
    PREFIX ${EXTRA_PYTHON_API_BINARY_DIR}/ExternalProject
    LIST_SEPARATOR |
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF
      -DCARLA_UNREAL_ENGINE_PATH=${CARLA_UNREAL_ENGINE_PATH}
      -DBUILD_CARLA_UNREAL=OFF
      -DBUILD_CARLA_SERVER=OFF
      -DBUILD_LIBCARLA_TESTS=OFF
      -DBUILD_EXAMPLES=OFF
      -DBUILD_OSM_WORLD_RENDERER=OFF
      -DENABLE_ROS2=OFF
      -DBUILD_PYTHON_API=ON
      -DENABLE_RSS=${ENABLE_RSS}
      -DENABLE_RTTI=${ENABLE_RTTI}
      -DENABLE_EXCEPTIONS=${ENABLE_EXCEPTIONS}
      -DENABLE_PEP517=${ENABLE_PEP517}
      -DPREFER_CLONE=${PREFER_CLONE}
      -DLIBCARLA_IMAGE_SUPPORTED_FORMATS=${CARLA_PYTHON_API_IMAGE_FORMATS}
      -DPython3_EXECUTABLE=${EXTRA_PYTHON_EXECUTABLE}
      -DPython_EXECUTABLE=${EXTRA_PYTHON_EXECUTABLE}
      -DCARLA_PYTHON_VERSIONS=
      -DCARLA_PYTHON_API_DIST_PATH=${CARLA_PYTHON_API_DIST_PATH}
      ${CARLA_PYTHON_API_FETCHCONTENT_ARGS}
    BUILD_COMMAND
      ${CMAKE_COMMAND} --build ${EXTRA_PYTHON_API_BINARY_DIR} --target carla-python-api
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
  )

  if (CARLA_PYTHON_API_PREVIOUS_TARGET)
    add_dependencies (${EXTRA_PYTHON_API_TARGET} ${CARLA_PYTHON_API_PREVIOUS_TARGET})
  endif ()
  set (CARLA_PYTHON_API_PREVIOUS_TARGET ${EXTRA_PYTHON_API_TARGET})

  add_dependencies (
    carla-python-api
    ${EXTRA_PYTHON_API_TARGET}
  )
endforeach ()
