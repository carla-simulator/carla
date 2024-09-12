#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

include (FetchContent)

set (CARLA_DEPENDENCIES_PENDING)

macro (carla_dependency_add NAME TAG ARCHIVE_URL GIT_URL)
  if (PREFER_CLONE)
    carla_message ("Cloning ${NAME}...")
    FetchContent_Declare (
      ${NAME}
      GIT_REPOSITORY ${GIT_URL}
      GIT_TAG ${TAG}
      GIT_SUBMODULES_RECURSE ON
      GIT_SHALLOW ON
      GIT_PROGRESS ON
      OVERRIDE_FIND_PACKAGE
      ${ARGN}
    )
    list (APPEND CARLA_DEPENDENCIES_PENDING ${NAME})
  else ()
    carla_message ("Downloading ${NAME}...")
    FetchContent_Declare (
      ${NAME}
      URL ${ARCHIVE_URL}
      OVERRIDE_FIND_PACKAGE
      ${ARGN}
    )
    list (APPEND CARLA_DEPENDENCIES_PENDING ${NAME})
  endif ()
endmacro ()

macro (carla_dependencies_make_available)
  FetchContent_MakeAvailable (
    ${CARLA_DEPENDENCIES_PENDING})
  set (CARLA_DEPENDENCIES_PENDING)
endmacro ()

macro (carla_dependency_option NAME VALUE)
  set (${NAME} ${VALUE} CACHE INTERNAL "")
endmacro ()



# ==== SQLITE3 ====

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

string (REPLACE "." "" CARLA_SQLITE_TAG ${CARLA_SQLITE_VERSION})

carla_message ("Downloading sqlite3...")
FetchContent_Declare (
  sqlite3
  URL https://www.sqlite.org/2024/sqlite-amalgamation-${CARLA_SQLITE_TAG}.zip
  OVERRIDE_FIND_PACKAGE
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

# ==== ZLIB ====

carla_dependency_option (ZLIB_BUILD_EXAMPLES OFF)
carla_dependency_add (
  zlib
  ${CARLA_ZLIB_TAG}
  https://github.com/madler/zlib/archive/refs/tags/${CARLA_ZLIB_TAG}.zip
  https://github.com/madler/zlib.git
)
carla_dependencies_make_available ()
include_directories (
  ${zlib_SOURCE_DIR}
  ${zlib_BINARY_DIR}
) # @TODO HACK

if (WIN32)
  carla_dependency_option (ZLIB_LIBRARY ${zlib_BINARY_DIR}/zlibstatic${CARLA_DEBUG_AFFIX}.lib)
else ()
  carla_dependency_option (ZLIB_LIBRARY ${zlib_BINARY_DIR}/libz.a)
endif ()
carla_dependency_option (ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
carla_dependency_option (ZLIB_LIBRARIES ${ZLIB_LIBRARY})

# ==== LIBPNG ====

carla_dependency_option (PNG_SHARED OFF)
carla_dependency_option (PNG_STATIC ON)
if (APPLE)
  carla_dependency_option (PNG_FRAMEWORK OFF)
endif ()
carla_dependency_option (PNG_TESTS OFF)
carla_dependency_option (PNG_TOOLS OFF)
carla_dependency_option (PNG_DEBUG OFF)
carla_dependency_option (PNG_HARDWARE_OPTIMIZATIONS ON)
carla_dependency_add (
  libpng
  ${CARLA_LIBPNG_TAG}
  https://github.com/pnggroup/libpng/archive/refs/tags/${CARLA_LIBPNG_TAG}.zip
  https://github.com/pnggroup/libpng.git
)
carla_dependencies_make_available ()
include_directories (
  ${libpng_SOURCE_DIR}
  ${libpng_BINARY_DIR}
) # @TODO HACK



# ==== BOOST ====

carla_dependency_option (BOOST_ENABLE_PYTHON ${BUILD_PYTHON_API})
carla_dependency_option (BOOST_ENABLE_MPI OFF)
carla_dependency_option (BOOST_LOCALE_WITH_ICU OFF)
carla_dependency_option (BOOST_LOCALE_WITH_ICONV OFF)
set (
  BOOST_INCLUDED_PROJECTS
  asio
  iterator
  python
  date_time
  geometry
  gil
  container
  variant2
)
carla_dependency_option (BOOST_INCLUDE_LIBRARIES "${BOOST_INCLUDED_PROJECTS}")
carla_dependency_add(
  boost
  ${CARLA_BOOST_TAG}
  https://github.com/boostorg/boost/releases/download/${CARLA_BOOST_TAG}/${CARLA_BOOST_TAG}.zip
  https://github.com/boostorg/boost.git
)

# ==== EIGEN ====

carla_dependency_option (EIGEN_BUILD_PKGCONFIG OFF)
carla_dependency_option (BUILD_TESTING OFF)
carla_dependency_option (EIGEN_BUILD_DOC OFF)
carla_dependency_add (
  eigen
  ${CARLA_EIGEN_TAG}
  https://gitlab.com/libeigen/eigen/-/archive/${CARLA_EIGEN_TAG}/eigen-${CARLA_EIGEN_TAG}.tar.gz
  https://gitlab.com/libeigen/eigen.git
)

# ==== RPCLIB ====

carla_dependency_add (
  rpclib
  ${CARLA_RPCLIB_TAG}
  https://github.com/carla-simulator/rpclib/archive/refs/heads/${CARLA_RPCLIB_TAG}.zip
  https://github.com/carla-simulator/rpclib.git
)

# ==== RECAST ====

carla_dependency_option (RECASTNAVIGATION_BUILDER OFF)
carla_dependency_add (
  recastnavigation
  ${CARLA_RECAST_TAG}
  https://github.com/carla-simulator/recastnavigation/archive/refs/heads/${CARLA_RECAST_TAG}.zip
  https://github.com/carla-simulator/recastnavigation.git
)

# ==== PROJ ====

if (ENABLE_OSM2ODR)
  carla_dependency_option (BUILD_TESTING OFF)
  carla_dependency_option (ENABLE_TIFF OFF)
  carla_dependency_option (ENABLE_CURL OFF)
  carla_dependency_add (
    proj
    ${CARLA_PROJ_TAG}
    https://github.com/OSGeo/PROJ/archive/refs/tags/${CARLA_PROJ_TAG}.zip
    https://github.com/OSGeo/PROJ.git
  )
endif ()

# ==== XERCESC ====

if (ENABLE_OSM2ODR)
  carla_dependency_add (
    xercesc
    ${CARLA_XERCESC_TAG}
    https://github.com/apache/xerces-c/archive/refs/tags/${CARLA_XERCESC_TAG}.zip
    https://github.com/apache/xerces-c.git
  )
endif ()

# ==== LUNASVG ====

if (BUILD_OSM_WORLD_RENDERER)
  carla_dependency_add (
    lunasvg
    ${CARLA_LUNASVG_TAG}
    https://github.com/sammycage/lunasvg/archive/refs/tags/${CARLA_LUNASVG_TAG}.zip
    https://github.com/sammycage/lunasvg.git
  )
endif ()

# ==== LIBOSMSCOUT ====

if (BUILD_OSM_WORLD_RENDERER)
  carla_dependency_add (
    libosmscout
    ${CARLA_LIBOSMSCOUT_TAG}
    https://github.com/Framstag/libosmscout/archive/refs/tags/${CARLA_LIBOSMSCOUT_TAG}.zip
    https://github.com/Framstag/libosmscout.git
  )
endif ()

# ==== STREETMAP ====

if (BUILD_CARLA_UNREAL)
  carla_dependency_add (
    StreetMap
    ${CARLA_STREETMAP_TAG}
    https://github.com/carla-simulator/StreetMap/archive/refs/heads/${CARLA_STREETMAP_TAG}.zip
    https://github.com/carla-simulator/StreetMap.git
    SOURCE_DIR ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/StreetMap
  )
endif ()

if (ENABLE_ROS2)
  carla_dependency_option (
    FOONATHAN_MEMORY_FORCE_VENDORED_BUILD
    ON
  )

  carla_git_dependency_add (
    foonathan_memory
    ${CARLA_FOONATHAN_MEMORY_TAG}
    https://github.com/eProsima/foonathan_memory_vendor/archive/refs/tags/${CARLA_FOONATHAN_MEMORY_TAG}.zip
    https://github.com/eProsima/foonathan_memory_vendor.git
  )

  include_directories (
    ${foonathan_memory_SOURCE_DIR}
    ${foonathan_memory_BINARY_DIR}
  )

  carla_git_dependency_add (
    fastcdr
    ${CARLA_FAST_CDR_TAG}
    https://github.com/eProsima/Fast-CDR/archive/refs/tags/${CARLA_FAST_CDR_TAG}.zip
    https://github.com/eProsima/Fast-CDR.git
  )

  include_directories (
    ${fastcdr_SOURCE_DIR}
    ${fastcdr_BINARY_DIR}
  )

  carla_dependency_option (
    THIRDPARTY_Asio
    ON
  )

  carla_dependency_option (
    THIRDPARTY_TinyXML2
    ON
  )

  carla_dependency_option (
    COMPILE_TOOLS
    ON
  )
  
  # We currently git clone fast-dds to handle submodules:
  carla_git_dependency_add (
    fastdds
    ${CARLA_FAST_DDS_TAG}
    https://github.com/eProsima/Fast-DDS/archive/refs/tags/${CARLA_FAST_DDS_TAG}.zip
    https://github.com/eProsima/Fast-DDS.git
  )

endif ()

carla_dependencies_make_available ()
