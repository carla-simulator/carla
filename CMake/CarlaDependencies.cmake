include (FetchContent)
include (ExternalProject)



# These checks may be incomplete:

if (LINUX)
  set (THREADS_PREFER_PTHREAD_FLAG ON)
  find_package (Threads REQUIRED)
endif ()



set (CARLA_DEPENDENCIES_INSTALL_PATH)

string (REPLACE "." "" CARLA_SQLITE_TAG ${CARLA_SQLITE_VERSION})

FetchContent_Declare (
  sqlite3
  URL https://www.sqlite.org/2024/sqlite-amalgamation-${CARLA_SQLITE_TAG}.zip
)
FetchContent_MakeAvailable (sqlite3)

add_library (
  libsqlite3 STATIC
  ${sqlite3_SOURCE_DIR}/sqlite3.h
  ${sqlite3_SOURCE_DIR}/sqlite3.c
)

add_executable (
  sqlite3
  ${sqlite3_SOURCE_DIR}/shell.c
)

if (LINUX)
  target_link_libraries (libsqlite3 PRIVATE ${CMAKE_DL_LIBS})
  target_link_libraries (libsqlite3 PRIVATE Threads::Threads)
endif ()

target_link_libraries (
  sqlite3 PRIVATE
  libsqlite3
)



set (CARLA_DEPENDENCIES)

macro (carla_dependency_add NAME URL TAG)
  message ("Downloading+building ${NAME}...")
  FetchContent_Declare(
    ${NAME}
    GIT_SUBMODULES_RECURSE ON
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    GIT_REPOSITORY ${URL}
    GIT_TAG ${TAG}
    OVERRIDE_FIND_PACKAGE
    ${ARGN}
  )
  list (APPEND CARLA_DEPENDENCIES ${NAME})
endmacro ()

macro (carla_dependencies_make_available)
  FetchContent_MakeAvailable (
    ${CARLA_DEPENDENCIES})
  set (CARLA_DEPENDENCIES)
endmacro ()

macro (carla_fetchcontent_option NAME VALUE)
  set (${NAME} ${VALUE})
endmacro ()



carla_fetchcontent_option (ZLIB_BUILD_EXAMPLES OFF)
carla_dependency_add (
  zlib
  https://github.com/madler/zlib.git
  ${CARLA_ZLIB_TAG}
)
carla_dependencies_make_available ()
include_directories (${zlib_SOURCE_DIR} ${zlib_BINARY_DIR}) # HACK

carla_fetchcontent_option (PNG_TESTS OFF)
carla_fetchcontent_option (PNG_SHARED OFF)
carla_fetchcontent_option (PNG_TOOLS OFF)
carla_fetchcontent_option (PNG_BUILD_ZLIB ON)
if (WIN32)
  carla_fetchcontent_option (ZLIB_LIBRARY ${zlib_BINARY_DIR}/zlibstatic${CARLA_DEBUG_AFFIX}.lib)
else ()
  carla_fetchcontent_option (ZLIB_LIBRARY ${zlib_BINARY_DIR}/libz.a)
endif ()
carla_fetchcontent_option (ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
carla_fetchcontent_option (ZLIB_LIBRARIES ${ZLIB_LIBRARY})
carla_dependency_add (
  libpng
  https://github.com/glennrp/libpng.git
  ${CARLA_LIBPNG_TAG}
)
carla_dependencies_make_available ()
include_directories (${libpng_SOURCE_DIR} ${libpng_BINARY_DIR}) # HACK



carla_fetchcontent_option (BOOST_ENABLE_MPI OFF)
carla_fetchcontent_option (BOOST_LOCALE_WITH_ICU OFF)
carla_fetchcontent_option (BOOST_LOCALE_WITH_ICONV OFF)
carla_fetchcontent_option (BOOST_EXCLUDE_LIBRARIES "iostreams;locale;fiber;log")
if (BUILD_PYTHON_API)
carla_fetchcontent_option (BOOST_ENABLE_PYTHON ${BUILD_PYTHON_API})
endif ()
carla_dependency_add (
  boost
  https://github.com/boostorg/boost.git
  ${CARLA_BOOST_TAG}
)

carla_fetchcontent_option (EIGEN_BUILD_PKGCONFIG OFF)
carla_fetchcontent_option (BUILD_TESTING OFF)
carla_fetchcontent_option (EIGEN_BUILD_DOC OFF)
carla_dependency_add (
  eigen
  https://gitlab.com/libeigen/eigen.git
  ${CARLA_EIGEN_TAG}
)

carla_dependency_add (
  rpclib
  https://github.com/carla-simulator/rpclib.git
  ${CARLA_RPCLIB_TAG}
)

carla_fetchcontent_option (RECASTNAVIGATION_BUILDER OFF)
carla_dependency_add (
  recastnavigation
  https://github.com/carla-simulator/recastnavigation.git
  ${CARLA_RECAST_TAG}
)

if (ENABLE_OSM2ODR)
  carla_fetchcontent_option (BUILD_TESTING OFF)
  carla_fetchcontent_option (ENABLE_TIFF OFF)
  carla_fetchcontent_option (ENABLE_CURL OFF)
  carla_dependency_add (
    proj
    https://github.com/OSGeo/PROJ.git
    ${CARLA_PROJ_TAG}
  )
endif ()

if (ENABLE_OSM2ODR)
  carla_dependency_add (
    xercesc
    https://github.com/apache/xerces-c.git
    ${CARLA_XERCESC_TAG}
  )
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  carla_dependency_add (
    lunasvg
    https://github.com/sammycage/lunasvg.git
    ${CARLA_LUNASVG_TAG}
  )
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  carla_dependency_add (
    libosmscout
    https://github.com/Framstag/libosmscout.git
    ${CARLA_LIBOSMSCOUT_TAG}
  )
endif ()

if (BUILD_CARLA_UE)
  carla_dependency_add (
    StreetMap
    https://github.com/carla-simulator/StreetMap.git
    ${CARLA_STREETMAP_TAG}
    SOURCE_DIR ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/StreetMap
  )
endif ()

# carla_dependency_add (
#   gtest
#   https://github.com/google/googletest.git
#   ${CARLA_GTEST_TAG}
# )

carla_dependencies_make_available ()

#[[

ExternalProject_Add (
  recastnavigation
  GIT_REPOSITORY https://github.com/carla-simulator/recastnavigation.git
  GIT_TAG ${CARLA_RECAST_TAG}
)

ExternalProject_Add (
  proj
  GIT_REPOSITORY https://github.com/OSGeo/PROJ.git
  GIT_TAG ${CARLA_PROJ_TAG}
  CMAKE_ARGS
    -DSQLITE3_INCLUDE_DIR=${sqlite3_SOURCE_DIR}
    -DSQLITE3_LIBRARY=$<TARGET_FILE:libsqlite3>
    -DEXE_SQLITE3=$<TARGET_FILE:sqlite3>
    -DENABLE_TIFF=OFF
    -DENABLE_CURL=OFF
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_PROJSYNC=OFF
    -DBUILD_PROJINFO=OFF
    -DBUILD_CCT=OFF
    -DBUILD_CS2CS=OFF
    -DBUILD_GEOD=OFF
    -DBUILD_GIE=OFF
    -DBUILD_PROJ=OFF
    -DBUILD_TESTING=OFF
)

add_dependencies (proj sqlite3 libsqlite3)

ExternalProject_Add (
  xercesc
  GIT_REPOSITORY https://github.com/apache/xerces-c.git
  GIT_TAG ${CARLA_XERCESC_TAG}
)

ExternalProject_Add (
  libpng
  GIT_REPOSITORY https://github.com/glennrp/libpng.git
  GIT_TAG ${CARLA_LIBPNG_TAG}
  CMAKE_ARGS
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DPNG_TESTS=OFF
    -DPNG_SHARED=OFF
    -DPNG_TOOLS=OFF
)

ExternalProject_Add (
  lunasvg
  GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
)

ExternalProject_Add (
  libosmscout
  GIT_REPOSITORY https://github.com/Framstag/libosmscout.git
)

ExternalProject_Add (
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG ${CARLA_GTEST_TAG}
)]]
