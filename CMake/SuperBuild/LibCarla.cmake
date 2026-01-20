include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)
include (ExternalProject)

set (PREFIX_PATHS ${CMAKE_PREFIX_PATH})

if (TARGET boost_project)
  ExternalProject_Get_Property (boost_project INSTALL_DIR)
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
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/share/eigen3/cmake)
endif ()

if (TARGET libosmscout_project)
  ExternalProject_Get_Property (libosmscout_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET lunasvg_project)
  ExternalProject_Get_Property (lunasvg_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET zlib_project)
  ExternalProject_Get_Property (zlib_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET png_project)
  ExternalProject_Get_Property (png_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/PNG)
endif ()

if (TARGET proj_project)
  ExternalProject_Get_Property (proj_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET recast_project)
  ExternalProject_Get_Property (recast_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/recastnavigation)
endif ()

if (TARGET rpclib_project)
  ExternalProject_Get_Property (rpclib_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR}/lib/cmake/rpclib)
endif ()

if (TARGET sqlite3_project)
  ExternalProject_Get_Property (sqlite3_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET streetmap_project)
  ExternalProject_Get_Property (streetmap_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

if (TARGET xercesc_project)
  ExternalProject_Get_Property (xercesc_project INSTALL_DIR)
  list (APPEND PREFIX_PATHS ${INSTALL_DIR})
endif ()

string (REPLACE ";" "," PREFIX_PATHS_ESCAPED "${PREFIX_PATHS}")

ExternalProject_Add (
    libcarla_project
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CARLA_ROOT}/LibCarla
    LIST_SEPARATOR ,
    CMAKE_ARGS
      -DCMAKE_PREFIX_PATH:STRING=${PREFIX_PATHS_ESCAPED}
      -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
)
ExternalProject_Add_StepTargets (libcarla_project install) # Generates libcarla_project-install
ExternalProject_Add_StepTargets (libcarla_project configure) # Generates libcarla_project-configure
add_dependencies (libcarla_project-configure boost_project-install)
add_dependencies (libcarla_project-configure eigen_project-install)
# add_dependencies (libcarla_project-configure libosmscout_project-install)
# add_dependencies (libcarla_project-configure lunasvg_project-install)
add_dependencies (libcarla_project-configure zlib_project-install)
add_dependencies (libcarla_project-configure png_project-install)
# add_dependencies (libcarla_project-configure proj_project-install)
add_dependencies (libcarla_project-configure recast_project-install)
add_dependencies (libcarla_project-configure rpclib_project-install)
# add_dependencies (libcarla_project-configure sqlite3_project-install) Handled at configure time!
# add_dependencies (libcarla_project-configure streetmap_project-install)
# add_dependencies (libcarla_project-configure xercesc_project-install)
