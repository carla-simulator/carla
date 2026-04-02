include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/Boost.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/Eigen.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/ZLIB.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/PNG.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/Recast.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/RPCLib.cmake)

set (PREFIX_PATHS ${CMAKE_PREFIX_PATH})
set (DEP_ROOTS)

if (TARGET boost_project)
  ExternalProject_Get_Property (boost_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DBoost_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/Boost-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_asio-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_iterator-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_python-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_date_time-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_geometry-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_container-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_variant2-${BOOST_VERSION})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/boost_gil-${BOOST_VERSION})
endif ()

if (TARGET eigen_project)
  ExternalProject_Get_Property (eigen_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DEigen3_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/share/eigen3/cmake)
endif ()

if (TARGET zlib_project)
  ExternalProject_Get_Property (zlib_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DZLIB_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET png_project)
  ExternalProject_Get_Property (png_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DPNG_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/PNG)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/libpng)
endif ()

if (TARGET recast_project)
  ExternalProject_Get_Property (recast_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DRRecastNavigation_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/recastnavigation)
endif ()

if (TARGET rpclib_project)
  ExternalProject_Get_Property (rpclib_project INSTALL_DIR)
  list (APPEND DEP_ROOTS -DRPCLib_ROOT=${INSTALL_DIR})
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/rpclib)
endif ()

string (REPLACE ";" "," PREFIX_PATHS_ESCAPED "${PREFIX_PATHS}")

ExternalProject_Add (
  libcarla_project
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CARLA_ROOT}/LibCarla
  LIST_SEPARATOR ,
  CMAKE_ARGS
    ${DEP_ROOTS}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCARLA_CONFIG_FILEPATH=${CMAKE_CURRENT_LIST_DIR}/Config.cmake
    -DCMAKE_PREFIX_PATH:STRING=${PREFIX_PATHS_ESCAPED}
    -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
  DEPENDS
    boost_project-install
    eigen_project-install
    zlib_project-install
    png_project-install
    recast_project-install
    rpclib_project-install
)
ExternalProject_Add_StepTargets (libcarla_project install) # Generates libcarla_project-install
