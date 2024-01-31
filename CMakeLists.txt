cmake_minimum_required (VERSION 3.20.0)
cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0140 NEW)

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 9)
set (CARLA_VERSION_PATCH 14)
set (CARLA_VERSION ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH})

set (CARLA_WORKSPACE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (CARLA_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR})

project (
    CARLA
    VERSION ${CARLA_VERSION}
    LANGUAGES CXX
    DESCRIPTION "Open-source simulator for autonomous driving research."
    HOMEPAGE_URL "https://carla.org"
)

macro (carla_string_option NAME DESCRIPTION VALUE)
    set (${NAME} ${VALUE} CACHE STRING ${DESCRIPTION})
endmacro ()

carla_string_option (
    CARLA_DEPENDENCIES_PATH
    "Where to build CARLA dependencies."
    ${CARLA_BUILD_PATH}/Dependencies)

carla_string_option (
    CARLA_DEPENDENCIES_INSTALL_PREFIX
    "Where to install CARLA dependencies."
    ${CMAKE_CURRENT_SOURCE_DIR}/Install)

set (
    CMAKE_INSTALL_PREFIX
    ${CARLA_DEPENDENCIES_INSTALL_PREFIX}
)

option (BUILD_CARLA_CLIENT "Build the LibCarla client." OFF)
option (BUILD_CARLA_SERVER "Build the LibCarla server." OFF)
option (ENABLE_LIBCARLA_PYTORCH "Enable LibCarla-PyTorch." OFF)
option (BUILD_PYTHON_API "Build the CARLA Python API." ON)
option (ENABLE_OSM2ODR "Enable OSM2ODR." ${BUILD_PYTHON_API})
option (ENABLE_RSS "Enable RSS components (ad-rss-lib)" OFF)
option (INSTALL_LIBCARLA "Install LibCarla and all of its dependencies." ON)
option (BUILD_OSM_WORLD_RENDERER "Build OSM World Renderer." ON)
option (ENABLE_RTTI "Enable C++ RTTI." OFF)
option (ENABLE_EXCEPTIONS "Enable C++ Exceptions." OFF)

set (
    CMAKE_MODULE_PATH
    ${CARLA_WORKSPACE_PATH}/cmake/modules
)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
include (ExternalProject)
include (FetchContent)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (LIBCARLA_SOURCE_PATH ${CARLA_WORKSPACE_PATH}/LibCarla/source)
set (LIBCARLA_THIRD_PARTY_SOURCE_PATH ${LIBCARLA_SOURCE_PATH}/third-party)
set (PYTHON_API_PATH ${CARLA_WORKSPACE_PATH}/PythonAPI)

set (EXTERNAL_DEPENDENCY_LIST)

macro (carla_declare_git_dependency NAME URL TAG)
    ExternalProject_Add (
        ${NAME}
        GIT_REPOSITORY ${URL}
        GIT_TAG ${TAG}
        GIT_SHALLOW TRUE
        ${ARGN}
    )
    list (APPEND EXTERNAL_DEPENDENCY_LIST ${NAME})
endmacro ()

macro (carla_declare_download_dependency NAME URL)
    FetchContent_Declare (
        ${NAME}
        URL ${URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        ${ARGN})
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
        ${PATTERN})
    message ("Found ${CANDIDATES}")
    list (GET CANDIDATES 0 FIRST_VALUE)
    message ("Selected ${FIRST_VALUE}")
    set (${OUT_RESULT} ${FIRST_VALUE})
    return (PROPAGATE ${OUT_RESULT})
endfunction ()

set (CARLA_DEFAULT_DEPENDENCY_ARGS)

if (CMAKE_C_COMPILER)
    list (APPEND CARLA_DEFAULT_DEPENDENCY_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER})
endif ()
if (CMAKE_INSTALL_PREFIX)
    list (APPEND CARLA_DEFAULT_DEPENDENCY_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
endif ()
if (CMAKE_CXX_COMPILER)
    list (APPEND CARLA_DEFAULT_DEPENDENCY_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})
endif ()
if (CMAKE_BUILD_TYPE)
    list (APPEND CARLA_DEFAULT_DEPENDENCY_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif ()

# --- BOOST ---

carla_string_option (CARLA_BOOST_TAG "Target boost git tag." boost-1.84.0)

set (
    REQUIRED_BOOST_LIBRARIES
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
        -DBOOST_INCLUDE_LIBRARIES="${REQUIRED_BOOST_LIBRARIES}"
        -DBOOST_ENABLE_PYTHON=ON)

set (BOOST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/boost/include)
set (BOOST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/boost/lib)



# --- ZLIB ---

carla_string_option (CARLA_ZLIB_TAG "Target zlib git tag." v1.3.1)

carla_declare_git_dependency (
    zlib
    https://github.com/madler/zlib.git
    ${CARLA_ZLIB_TAG}
    CMAKE_ARGS
        ${CARLA_DEFAULT_DEPENDENCY_ARGS}
        -DBUILD_SHARED_LIBS=OFF)

set (ZLIB_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/zlib/include)
set (ZLIB_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/zlib/lib)
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

set (LIBPNG_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/Install/libpng/include)
set (LIBPNG_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/Install/libpng/lib)



# --- SQLITE ---

carla_string_option (CARLA_SQLITE_VERSION_MAJOR "Target SQLite version" 3)
carla_string_option (CARLA_SQLITE_VERSION_MINOR "Target SQLite version" 45)
carla_string_option (CARLA_SQLITE_VERSION_PATCH "Target SQLite version" 0000)

set (SQLITE_TAG ${CARLA_SQLITE_VERSION_MAJOR}${CARLA_SQLITE_VERSION_MINOR}${CARLA_SQLITE_VERSION_PATCH})

carla_declare_download_dependency (
    sqlite3
    https://www.sqlite.org/2024/sqlite-amalgamation-${SQLITE_TAG}.zip
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

target_link_libraries (sqlite3 PRIVATE libsqlite3)



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

set (
    CARLA_COMMON_INCLUDE_PATHS
    ${LIBCARLA_SOURCE_PATH}
    ${BOOST_INCLUDE_PATH}
    ${CHRONO_INCLUDE_PATH}
    ${EIGEN_INCLUDE_PATH}
    ${GOOGLETEST_INCLUDE_PATH}
    ${LIBPNG_INCLUDE_PATH}
    ${PROJ_INCLUDE_PATH}
    ${RECAST_INCLUDE_PATH}
    ${RPCLIB_INCLUDE_PATH}
    ${SQLITE_INCLUDE_PATH}
    ${XERCESC_INCLUDE_PATH}
    ${ZLIB_INCLUDE_PATH})



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



if (BUILD_CARLA_SERVER OR BUILD_CARLA_CLIENT)
    configure_file (
        ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
        ${LIBCARLA_SOURCE_PATH}/carla/Version.h)
endif ()



if (BUILD_CARLA_SERVER)

    project (carla-server)

    file (
        GLOB
        LIBCARLA_SERVER_SOURCES
        ${LIBCARLA_SOURCE_PATH}/carla/Buffer.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/Exception.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/SensorHeaderSerializer.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.cpp
    )

    file (
        GLOB
        LIBCARLA_SERVER_SOURCES_THIRD_PARTY
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.cpp
    )

    file (
        GLOB
        LIBCARLA_SERVER_HEADERS
        ${LIBCARLA_SOURCE_PATH}/carla/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.h
        ${LIBCARLA_SOURCE_PATH}/compiler/*.h
    )

    file (
        GLOB
        LIBCARLA_SERVER_HEADERS_THIRD_PARTY
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.hpp
    )

    add_library (
        carla-server STATIC
        ${LIBCARLA_SERVER_HEADERS}
        ${LIBCARLA_SERVER_SOURCES}
        ${LIBCARLA_SERVER_HEADERS_THIRD_PARTY}
        ${LIBCARLA_SERVER_SOURCES_THIRD_PARTY}
    )

    if (EXTERNAL_DEPENDENCY_LIST)
        add_dependencies (carla-server ${EXTERNAL_DEPENDENCY_LIST})
    endif ()

    # target_compile_options (carla-server PRIVATE /EHsc)

    target_include_directories (
        carla-server PRIVATE
        ${CARLA_COMMON_INCLUDE_PATHS}
    )
    
    target_compile_definitions (
        carla-server PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
        ${CARLA_EXCEPTION_DEFINITIONS}
        ${CARLA_RTTI_DEFINITIONS}
    )

    install (TARGETS carla-server DESTINATION lib)

    foreach (HEADER ${LIBCARLA_SERVER_HEADERS})
        cmake_path (GET HEADER PARENT_PATH HEADER_PARENT)
        cmake_path (RELATIVE_PATH HEADER_PARENT BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla OUTPUT_VARIABLE HEADER_RELATIVE)
        install (FILES ${HEADER} DESTINATION include/carla/${HEADER_RELATIVE})
    endforeach ()

endif ()



if (BUILD_CARLA_CLIENT)

    project (carla-client)

    file (
        GLOB
        LIBCARLA_CLIENT_HEADERS
        ${LIBCARLA_SOURCE_PATH}/carla/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/client/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/client/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/image/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/nav/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/pointcloud/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/profiler/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/data/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/trafficmanager/*.h
        ${LIBCARLA_SOURCE_PATH}/compiler/*.h
    )

    if (ENABLE_RSS)
        file (
            GLOB
            LIBCARLA_CLIENT_HEADERS_RSS
            ${LIBCARLA_SOURCE_PATH}/carla/rss/*.h
        )
        list (
            APPEND
            LIBCARLA_CLIENT_HEADERS
            ${LIBCARLA_CLIENT_HEADERS_RSS}
        )
    endif ()

    file (
        GLOB
        LIBCARLA_CLIENT_HEADERS_THIRD_PARTY
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.hpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.h
    )

    file (
        GLOB
        LIBCARLA_CLIENT_SOURCES
        ${LIBCARLA_SOURCE_PATH}/carla/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/client/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/client/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/image/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/nav/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/pointcloud/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/data/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/trafficmanager/*.cpp
    )

    if (ENABLE_RSS)
        file (
            GLOB
            LIBCARLA_CLIENT_SOURCES_RSS
            ${LIBCARLA_SOURCE_PATH}/carla/rss/*.cpp
        )
        list (
            APPEND
            LIBCARLA_CLIENT_SOURCES
            ${LIBCARLA_CLIENT_SOURCES_RSS}
        )
    endif ()

    file (
        GLOB
        LIBCARLA_CLIENT_SOURCES_THIRD_PARTY
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.cpp
    )

    add_library (
        carla-client STATIC
        ${LIBCARLA_CLIENT_HEADERS}
        ${LIBCARLA_CLIENT_SOURCES}
        ${LIBCARLA_CLIENT_HEADERS_THIRD_PARTY}
        ${LIBCARLA_CLIENT_SOURCES_THIRD_PARTY}
    )

    if (EXTERNAL_DEPENDENCY_LIST)
        add_dependencies (carla-client ${EXTERNAL_DEPENDENCY_LIST})
    endif ()

    target_include_directories (
        carla-client PRIVATE
        ${CARLA_COMMON_INCLUDE_PATHS}
    )

    target_compile_definitions (
        carla-client PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
        ${CARLA_EXCEPTION_DEFINITIONS}
        ${CARLA_RTTI_DEFINITIONS}
    )
    
    install (TARGETS carla-client DESTINATION lib)
    
    foreach (HEADER ${LIBCARLA_CLIENT_HEADERS})
        cmake_path (GET HEADER PARENT_PATH HEADER_PARENT)
        cmake_path (RELATIVE_PATH HEADER_PARENT BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla OUTPUT_VARIABLE HEADER_RELATIVE)
        install (FILES ${HEADER} DESTINATION include/carla/${HEADER_RELATIVE})
    endforeach ()

endif ()



if (BUILD_OSM_WORLD_RENDERER)

    project (OsmMapRenderer)

    set (OSM_RENDERER_PATH ${CARLA_WORKSPACE_PATH}/osm-world-renderer)

    add_library (
        LibOSMRenderer
        ${OSM_RENDERER_PATH}/OsmRenderer/src/OsmRenderer.cpp
        ${OSM_RENDERER_PATH}/OsmRenderer/src/MapDrawer.cpp
        ${OSM_RENDERER_PATH}/OsmRenderer/src/MapRasterizer.cpp
    )

    if (EXTERNAL_DEPENDENCY_LIST)
        add_dependencies (LibOSMRenderer ${EXTERNAL_DEPENDENCY_LIST})
    endif ()

    target_include_directories (
        LibOSMRenderer PUBLIC
        ${OSM_RENDERER_PATH}/OsmRenderer/include
    )

    add_executable (
        OSMWorldRenderer
        ${OSM_RENDERER_PATH}/OsmRenderer/main.cpp
    )

    target_link_libraries (OSMWorldRenderer PRIVATE LibOSMRenderer)
    target_link_libraries (OSMWorldRenderer PRIVATE osmscout)
    target_link_libraries (OSMWorldRenderer PRIVATE osmscout_map)
    target_link_libraries (OSMWorldRenderer PRIVATE osmscout_map_svg)
    target_link_libraries (OSMWorldRenderer PRIVATE lunasvg)

    target_compile_definitions (
        OSMWorldRenderer PRIVATE
        _USE_MATH_DEFINES
        ${CARLA_COMMON_DEFINITIONS}
    )

endif ()



if (BUILD_PYTHON_API)

    project (carla-python-api)

    set (Python_USE_STATIC_LIBS ON)

    find_package (
        Python
        COMPONENTS
            Interpreter
            Development.Module
        REQUIRED)

    set (PYTHON_API_CARLA_PATH ${PYTHON_API_PATH}/carla)
    set (PYTHON_API_SOURCE_PATH ${PYTHON_API_CARLA_PATH}/source/libcarla)

    configure_file (
        ${PYTHON_API_PATH}/carla/pyproject.toml.in
        ${PYTHON_API_PATH}/carla/pyproject.toml)
    
    set (
        PYTHON_API_SOURCES
        ${PYTHON_API_SOURCE_PATH}/Actor.cpp
        ${PYTHON_API_SOURCE_PATH}/Blueprint.cpp
        ${PYTHON_API_SOURCE_PATH}/Client.cpp
        ${PYTHON_API_SOURCE_PATH}/Commands.cpp
        ${PYTHON_API_SOURCE_PATH}/Control.cpp
        ${PYTHON_API_SOURCE_PATH}/Exception.cpp
        ${PYTHON_API_SOURCE_PATH}/Geom.cpp
        ${PYTHON_API_SOURCE_PATH}/LightManager.cpp
        ${PYTHON_API_SOURCE_PATH}/Map.cpp
        ${PYTHON_API_SOURCE_PATH}/OSM2ODR.cpp
        ${PYTHON_API_SOURCE_PATH}/PythonAPI.cpp
        ${PYTHON_API_SOURCE_PATH}/PythonAPI.h
        ${PYTHON_API_SOURCE_PATH}/Sensor.cpp
        ${PYTHON_API_SOURCE_PATH}/SensorData.cpp
        ${PYTHON_API_SOURCE_PATH}/Snapshot.cpp
        ${PYTHON_API_SOURCE_PATH}/TrafficManager.cpp
        ${PYTHON_API_SOURCE_PATH}/Weather.cpp
        ${PYTHON_API_SOURCE_PATH}/World.cpp
    )

    if (ENABLE_RSS)
        list (APPEND PYTHON_API_SOURCES ${PYTHON_API_SOURCE_PATH}/AdRss.cpp)
    endif ()

    Python_add_library(
        carla-python-api
        MODULE
        ${PYTHON_API_SOURCES}
        WITH_SOABI)
    
    if (EXTERNAL_DEPENDENCY_LIST)
        add_dependencies (carla-python-api ${EXTERNAL_DEPENDENCY_LIST})
    endif ()

    target_compile_definitions (
        carla-python-api
        PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
        ${CARLA_RTTI_DEFINITIONS}
        # ${CARLA_EXCEPTION_DEFINITIONS}
        BOOST_ALL_NO_LIB
        BOOST_PYTHON_STATIC_LIB
        LIBCARLA_WITH_PYTHON_SUPPORT)

    target_include_directories (
        carla-python-api
        PRIVATE
        ${CARLA_COMMON_INCLUDE_PATHS}
        ${PYTHON_API_SOURCE_PATH}
        ${SUMO_INCLUDE_PATH})
    
    # carla_find_library (BOOST_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/boost/lib/*boost_python*${LIB_EXT})
    # carla_find_library (RPCLIB_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/rpclib/lib/*rpc*${LIB_EXT})
    # carla_find_library (LIBPNG_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/libpng/lib/*libpng*_static*${LIB_EXT})
    # carla_find_library (ZLIB_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/zlib/lib/*zlib${LIB_EXT})
    # carla_find_library (RECAST_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/recast/lib/*Recast${LIB_EXT})
    # carla_find_library (DETOUR_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/recast/lib/*Detour${LIB_EXT})
    # carla_find_library (DETOURCROWD_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/recast/lib/*DetourCrowd${LIB_EXT})
    # carla_find_library (XERCESC_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/xercesc/lib/*xerces-c*${LIB_EXT})
    # carla_find_library (OSM2ODR_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/sumo/lib/*osm2odr${LIB_EXT})
    # carla_find_library (SQLITE_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Build/sqlite/*sqlite*${LIB_EXT})
    # carla_find_library (PROJ_LIBRARY ${CARLA_DEPENDENCIES_PATH}/Install/proj/lib/*proj${LIB_EXT})

    set (
        PYTHON_API_LIBRARY_DEPENDENCIES
        ${BOOST_LIBRARY}
        ${RPCLIB_LIBRARY}
        ${LIBPNG_LIBRARY}
        ${ZLIB_LIBRARY}
        ${RECAST_LIBRARY}
        ${DETOUR_LIBRARY}
        ${DETOURCROWD_LIBRARY}
        ${XERCESC_LIBRARY}
        ${OSM2ODR_LIBRARY}
        ${SQLITE_LIBRARY}
        ${PROJ_LIBRARY})

    if (WIN32)
        list (APPEND PYTHON_API_LIBRARY_DEPENDENCIES Shlwapi.lib)
    endif ()

    target_link_libraries (
        carla-python-api
        PRIVATE
        carla-client
        ${PYTHON_API_LIBRARY_DEPENDENCIES})
    
    #[[
    install (
        TARGETS
        carla-python-api
        DESTINATION
        ${CARLA_BUILD_PATH}/PythonAPI/dist)
    ]]
endif ()
