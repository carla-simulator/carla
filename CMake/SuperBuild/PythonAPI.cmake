include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

if (NOT TARGET libcarla_project)
  message (FATAL_ERROR "libcarla_project does not exist.")
endif ()

set (PREFIX_PATHS ${CMAKE_PREFIX_PATH})
ExternalProject_Get_Property(libcarla_project INSTALL_DIR)
list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/LibCarla)

string (REPLACE ";" "," PREFIX_PATHS_ESCAPED "${PREFIX_PATHS}")

ExternalProject_Add (
    carla_python_api_project
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CARLA_ROOT}/PythonAPI
    INSTALL_COMMAND ""
    LIST_SEPARATOR ,
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCARLA_CONFIG_FILEPATH=${CMAKE_CURRENT_LIST_DIR}/Config.cmake
      -DENABLE_PEP517=${ENABLE_PEP517}
      -DCMAKE_PREFIX_PATH:STRING=${PREFIX_PATHS_ESCAPED}
      -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
    DEPENDS
      libcarla_project-install
)
ExternalProject_Add_StepTargets (carla_python_api_project install) # Generates carla_python_api_project-install
ExternalProject_Add_StepTargets (carla_python_api_project configure) # Generates carla_python_api_project-configure

file (
  COPY
    ${CARLA_ROOT}/LICENSE
  DESTINATION
    ${CARLA_ROOT}/PythonAPI/carla/
)
