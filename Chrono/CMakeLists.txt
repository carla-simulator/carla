cmake_minimum_required (
  VERSION
  ${CARLA_CMAKE_MINIMUM_REQUIRED_VERSION}
)

project (carla-chrono-project)

include (ExternalProject)

set (PROJECT_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/install)

# Unreal/ is added after this directory and needs the include paths.
set (CARLA_CHRONO_INSTALL_PATH ${PROJECT_INSTALL_PATH} PARENT_SCOPE)

# Chrono 6.0.0 is a C++14 code base: compiling it at CARLA's C++20 level trips
# constructs that C++20 removed. Pin the subproject to C++14 and let it keep its
# own standard - the installed headers still include cleanly from the C++20
# Carla module, which is all UChronoMovementComponent needs.
if (CARLA_CHRONO_SOURCE_PATH STREQUAL "")
  set (
    CARLA_CHRONO_SOURCE_ARGS
    GIT_REPOSITORY https://github.com/projectchrono/chrono.git
    GIT_TAG ${CARLA_CHRONO_TAG}
    GIT_SHALLOW ON
  )
else ()
  carla_message ("Using local Project Chrono source at ${CARLA_CHRONO_SOURCE_PATH}")
  set (
    CARLA_CHRONO_SOURCE_ARGS
    SOURCE_DIR ${CARLA_CHRONO_SOURCE_PATH}
    BUILD_ALWAYS OFF
  )
endif ()

ExternalProject_Add (
  chrono
  ${CARLA_CHRONO_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_INSTALL_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_STANDARD=14
    -DEIGEN3_INCLUDE_DIR=${eigen_SOURCE_DIR}
    -DENABLE_MODULE_VEHICLE=ON
    -DBUILD_DEMOS=OFF
    -DBUILD_TESTING=OFF
    -DBUILD_BENCHMARKING=OFF
)

# The sedan templates under Co-Simulation/ reference Chrono's own sedan meshes.
# Those files are byte-identical to what Chrono installs, so stage them from the
# install tree rather than vendoring a second copy into the repository. Only the
# JSON templates, which carry CARLA's tuning, are tracked by git.
set (CARLA_CHRONO_VEHICLE_DATA_PATH ${CMAKE_SOURCE_DIR}/Co-Simulation/Chrono/Vehicles/sedan)

add_custom_command (
  TARGET chrono
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_chassis_vis.obj
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_chassis_vis.mtl
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_rim.obj
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_rim.mtl
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_tire.obj
    ${PROJECT_INSTALL_PATH}/share/chrono/data/vehicle/sedan/sedan_tire.mtl
    ${CARLA_CHRONO_VEHICLE_DATA_PATH}
)

set (CARLA_PLUGIN_BINARY_PATH ${CMAKE_SOURCE_DIR}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux)

make_directory (${CARLA_PLUGIN_BINARY_PATH})

add_custom_command (
  TARGET chrono
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${PROJECT_INSTALL_PATH}/lib/*.so*
    ${CARLA_PLUGIN_BINARY_PATH}
)

carla_add_custom_target (
  carla-chrono
  "Build the Project Chrono CARLA subproject."
  DEPENDS chrono
)
