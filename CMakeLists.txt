cmake_minimum_required (VERSION 3.20.0)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0140 NEW)

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 9)
set (CARLA_VERSION_PATCH 15)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

project (
    CARLA
    VERSION ${CARLA_VERSION}
    LANGUAGES CXX
    DESCRIPTION "Open-source simulator for autonomous driving research."
    HOMEPAGE_URL https://carla.org
)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (ExternalProject)
include (FetchContent)

set (CARLA_WORKSPACE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

macro (carla_string_option NAME DESCRIPTION VALUE)
    set (${NAME} ${VALUE} CACHE STRING ${DESCRIPTION})
endmacro ()

carla_string_option (
    CARLA_DEPENDENCIES_PATH
    "Where to build CARLA dependencies."
    ${CMAKE_CURRENT_BINARY_DIR}/Dependencies
)

carla_string_option (
    CARLA_DEPENDENCIES_INSTALL_PREFIX
    "Where to install CARLA dependencies."
    ${CMAKE_CURRENT_SOURCE_DIR}/Install
)

set (
    CMAKE_INSTALL_PREFIX
    ${CARLA_DEPENDENCIES_INSTALL_PREFIX}
)

option (
  BUILD_CARLA_CLIENT
  "Build the LibCarla client."
  ON
)

option (
  BUILD_CARLA_SERVER
  "Build the LibCarla server."
  ON
)

option (
  ENABLE_LIBCARLA_PYTORCH
  "Enable LibCarla-PyTorch."
  OFF
)

option (
  BUILD_PYTHON_API
  "Build the CARLA Python API."
  ON
)

option (
  ENABLE_OSM2ODR
  "Enable OSM2ODR."
  ${BUILD_PYTHON_API}
)

option (
  ENABLE_RSS
  "Enable RSS components (ad-rss-lib)"
  OFF
)

option (
  INSTALL_LIBCARLA
  "Install LibCarla and all of its dependencies."
  ON
)

option (
  BUILD_OSM_WORLD_RENDERER
  "Build OSM World Renderer."
  ON
)

option (
  ENABLE_RTTI
  "Enable C++ RTTI."
  OFF
)

option (
  ENABLE_EXCEPTIONS
  "Enable C++ Exceptions."
  OFF
)

set (EXTERNAL_DEPENDENCY_LIST)

macro (carla_declare_git_dependency NAME URL TAG)
    ExternalProject_Add (
        ${NAME}
        GIT_REPOSITORY ${URL}
        GIT_TAG ${TAG}
        GIT_PROGRESS ON
        GIT_SHALLOW ON
        ${ARGN}
    )
    list (APPEND EXTERNAL_DEPENDENCY_LIST ${NAME})
endmacro ()

macro (carla_declare_download_dependency NAME URL)
    FetchContent_Declare (
        ${NAME}
        URL ${URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        ${ARGN}
    )
    FetchContent_MakeAvailable (${NAME})
endmacro ()

if (WIN32)
    set (LIB_EXT .lib)
else ()
    set (LIB_EXT .a)
endif ()

function (carla_find_library OUT_RESULT PATTERN)
    message ("Looking for ${PATTERN}")
    file (
        GLOB_RECURSE
        CANDIDATES
        LIST_DIRECTORIES FALSE
        ${PATTERN}
    )
    message ("Found ${CANDIDATES}")
    list (GET CANDIDATES 0 FIRST_VALUE)
    message ("Selected ${FIRST_VALUE}")
    set (${OUT_RESULT} ${FIRST_VALUE})
    return (PROPAGATE ${OUT_RESULT})
endfunction ()

set (CARLA_DEFAULT_DEPENDENCY_ARGS)

if (CMAKE_C_COMPILER)
    list (
      APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
endif ()

if (CMAKE_INSTALL_PREFIX)
    list (
      APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    )
endif ()

if (CMAKE_CXX_COMPILER)
    list (
      APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    )
endif ()

if (CMAKE_BUILD_TYPE)
    list (
      APPEND CARLA_DEFAULT_DEPENDENCY_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )
endif ()

# --- BOOST ---

carla_string_option (CARLA_BOOST_TAG "Target boost git tag." boost-1.84.0)

set (
    REQUIRED_BOOST_LIBRARIES
    assert
    algorithm
    python
    iterator
    asio
)

carla_declare_git_dependency (
    boost
    https://github.com/boostorg/boost.git
    ${CARLA_BOOST_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
      ${CARLA_DEFAULT_DEPENDENCY_ARGS}
      -DBOOST_INCLUDE_LIBRARIES=${REQUIRED_BOOST_LIBRARIES}
      -DBOOST_ENABLE_PYTHON=ON
    GIT_SUBMODULES
      tools/cmake
      libs/core
      libs/assert
      libs/algorithm
      libs/python
      libs/iterator
      libs/asio
)



# --- ZLIB ---

carla_string_option (CARLA_ZLIB_TAG "Target zlib git tag." v1.3.1)

carla_declare_git_dependency (
    zlib
    https://github.com/madler/zlib.git
    ${CARLA_ZLIB_TAG}
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DBUILD_SHARED_LIBS=OFF)

set (
    ZLIB_INCLUDE_PATHS
    ${CARLA_DEPENDENCIES_PATH}/Source/zlib
    ${CARLA_DEPENDENCIES_PATH}/Build/zlib
)



# --- LIBPNG ---

carla_string_option (CARLA_LIBPNG_TAG "Target libpng git tag." v1.6.40)

carla_declare_git_dependency (
    libpng
    https://github.com/glennrp/libpng.git
    ${CARLA_LIBPNG_TAG}
    INSTALL_COMMAND ""
    DEPENDS
        zlib
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DPNG_TESTS=OFF
        -DPNG_SHARED=OFF
        -DPNG_TOOLS=OFF)

add_dependencies (libpng zlib)

set (
  LIBPNG_INCLUDE_PATH
  ${CARLA_DEPENDENCIES_PATH}/Install/libpng/include
)

set (
  LIBPNG_LIBRARY_PATH
  ${CARLA_DEPENDENCIES_PATH}/Install/libpng/lib
)



# --- SQLITE ---

carla_string_option (CARLA_SQLITE_VERSION_MAJOR "Target SQLite version" 3)
carla_string_option (CARLA_SQLITE_VERSION_MINOR "Target SQLite version" 45)
carla_string_option (CARLA_SQLITE_VERSION_PATCH "Target SQLite version" 0000)

set (SQLITE_VERSION ${CARLA_SQLITE_VERSION_MAJOR}${CARLA_SQLITE_VERSION_MINOR}${CARLA_SQLITE_VERSION_PATCH})

project (
  sqlite-amalgam
  LANGUAGES C
  VERSION ${SQLITE_VERSION}
)

carla_declare_download_dependency (
    sqlite3
    https://www.sqlite.org/2024/sqlite-amalgamation-${SQLITE_VERSION}.zip
)

add_library (
    libsqlite3 STATIC
    ${sqlite3_SOURCE_DIR}/sqlite3.h
    ${sqlite3_SOURCE_DIR}/sqlite3.c
)

add_executable (
    sqlite3
    ${sqlite3_SOURCE_DIR}/shell.c
)

target_link_libraries (
  sqlite3 PRIVATE
  libsqlite3
)



# --- PROJ ---

carla_string_option (CARLA_PROJ_TAG "Target PROJ git tag." 7.2.1)

carla_declare_git_dependency (
    proj
    https://github.com/OSGeo/PROJ.git
    ${CARLA_PROJ_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DSQLITE3_INCLUDE_DIR=${sqlite3_SOURCE_DIR}
        -DSQLITE3_LIBRARY=$<TARGET_FILE:libsqlite3>
        -DEXE_SQLITE3=$<TARGET_FILE:sqlite3>
        -DWIN32_LEAN_AND_MEAN=1
        -DVC_EXTRALEAN=1
        -DNOMINMAX=1
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
        -DBUILD_TESTING=OFF)

add_dependencies (proj libsqlite3)

set (PROJ_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/proj/include)
set (PROJ_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/proj/lib)



# --- GOOGLE TEST ---

carla_string_option (CARLA_GTEST_TAG "Target googletest git tag." v1.14.0)

carla_declare_git_dependency (
    gtest
    https://github.com/google/googletest.git
    ${CARLA_GTEST_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DBUILD_GMOCK=OFF
        -DGTEST_HAS_ABSL=OFF)

set (GOOGLETEST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/gtest/include)
set (GOOGLETEST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/gtest/lib)



# --- XERCES-C ---

carla_string_option (CARLA_XERCESC_TAG "Target xerces-c git tag." v3.2.4)

carla_declare_git_dependency (
    xercesc
    https://github.com/apache/xerces-c.git
    ${CARLA_XERCESC_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DBUILD_SHARED_LIBS=OFF)

set (XERCESC_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/xercesc/include)
set (XERCESC_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/xercesc/lib)



# --- RPCLIB ---

carla_string_option (CARLA_RPCLIB_TAG "Target rpclib git tag." carla)

carla_declare_git_dependency (
    rpclib
    https://github.com/carla-simulator/rpclib.git
    ${CARLA_RPCLIB_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DRPCLIB_BUILD_TESTS=OFF
        -DRPCLIB_GENERATE_COMPDB=OFF
        -DRPCLIB_BUILD_EXAMPLES=OFF
        -DRPCLIB_ENABLE_LOGGING=OFF
        -DRPCLIB_ENABLE_COVERAGE=OFF
        -DRPCLIB_MSVC_STATIC_RUNTIME=OFF)

set (RPCLIB_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/rpclib/include)
set (RPCLIB_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/rpclib/lib)



# --- RECAST NAVIGATION ---

carla_string_option (CARLA_RECAST_TAG "Target recastnavigation git tag." carla)

carla_declare_git_dependency (
    recastnavigation
    https://github.com/carla-simulator/recastnavigation.git
    ${CARLA_RECAST_TAG}
    INSTALL_COMMAND ""
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DRECASTNAVIGATION_DEMO=OFF
        -DRECASTNAVIGATION_TESTS=OFF
        -DRECASTNAVIGATION_EXAMPLES=OFF)

set (RECAST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/recast/include)
set (RECAST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/recast/lib)



set (
    CARLA_COMMON_DEFINITIONS
    LIBCARLA_IMAGE_WITH_PNG_SUPPORT=true)

if (WIN32)
    # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
    list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # --- Windows 10 ---
    list (APPEND CARLA_COMMON_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
    list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
endif ()

set (CARLA_EXCEPTION_DEFINITIONS)

if (ENABLE_EXCEPTIONS)
else ()
    list (APPEND CARLA_EXCEPTION_DEFINITIONS ASIO_NO_EXCEPTIONS)
    list (APPEND CARLA_EXCEPTION_DEFINITIONS BOOST_NO_EXCEPTIONS)
    list (APPEND CARLA_EXCEPTION_DEFINITIONS LIBCARLA_NO_EXCEPTIONS)
    list (APPEND CARLA_EXCEPTION_DEFINITIONS PUGIXML_NO_EXCEPTIONS)
endif ()

set (CARLA_RTTI_DEFINITIONS)

if (ENABLE_RTTI)
else ()
    list (APPEND CARLA_RTTI_DEFINITIONS BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
endif ()

if (BUILD_CARLA_CLIENT OR BUILD_CARLA_SERVER)
  add_subdirectory (LibCarla)
endif ()

if (BUILD_OSM_WORLD_RENDERER)
  add_subdirectory (osm-world-renderer)
endif ()

if (BUILD_PYTHON_API)
  add_subdirectory (PythonAPI)
endif ()
