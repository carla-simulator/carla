cmake_minimum_required (VERSION 3.12.0)

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 9)
set (CARLA_VERSION_PATCH 14)
set (CARLA_VERSION ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH})

project (
    CARLA
    VERSION ${CARLA_VERSION}
    LANGUAGES CXX
    DESCRIPTION "Open-source simulator for autonomous driving research."
    HOMEPAGE_URL "https://carla.org/"
)

include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

option (
    BUILD_LIBCARLA_CLIENT
    "Build the LibCarla client."
    ON)

option (
    BUILD_LIBCARLA_SERVER
    "Build the LibCarla server."
    ON)

option (
    ENABLE_LIBCARLA_PYTORCH
    "Enable LibCarla-PyTorch."
    OFF)

option (
    BUILD_PYTHON_API
    "Build the CARLA Python API."
    ON)

option (
    ENABLE_LIBCARLA_CLIENT_RSS
    "Enable RSS components (ad-rss-lib)"
    OFF)

option (
    INSTALL_LIBCARLA
    "Install LibCarla and all of its dependencies."
    ON)

option (
    BUILD_OSM_WORLD_RENDERER
    "Build OSM World Renderer."
    ON)

option (
    ENABLE_RTTI
    "Enable C++ RTTI."
    ON)

option (
    ENABLE_EXCEPTIONS
    "Enable C++ Exceptions."
    ON)

set (CARLA_WORKSPACE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (CARLA_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR})
if (NOT CARLA_DEPENDENCIES_PATH)
    set (CARLA_DEPENDENCIES_PATH ${CMAKE_CURRENT_BINARY_DIR}/Dependencies)
endif ()

configure_file (
    ${CARLA_WORKSPACE_PATH}/.clangd.in
    ${CARLA_WORKSPACE_PATH}/.clangd)

set (LIBCARLA_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/LibCarla/source)
set (LIBCARLA_THIRD_PARTY_SOURCE_PATH ${LIBCARLA_SOURCE_PATH}/third-party)
set (PYTHONAPI_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/PythonAPI)

set (BOOST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/boost-install/include/boost-1_83)
set (BOOST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/boost-install/lib)
set (CHRONO_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/chrono-install/include)
set (CHRONO_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/chrono-install/lib)
set (EIGEN_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/eigen-install/include)
set (EIGEN_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/eigen-install/lib)
set (GOOGLETEST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/gtest-install/include)
set (GOOGLETEST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/gtest-install/lib)
set (LIBPNG_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/libpng-install/include)
set (LIBPNG_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/libpng-install/lib)
set (PROJ_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/proj-install/include)
set (PROJ_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/proj-install/lib)
set (RECAST_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/recast-install/include)
set (RECAST_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/recast-install/lib)
set (RPCLIB_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/rpclib-install/include)
set (RPCLIB_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/rpclib-install/lib)
set (SQLITE_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/sqlite-install/include)
set (SQLITE_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/sqlite-install/lib)
set (XERCESC_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/xercesc-install/include)
set (XERCESC_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/xercesc-install/lib)
set (ZLIB_INCLUDE_PATH ${CARLA_DEPENDENCIES_PATH}/zlib-install/include)
set (ZLIB_LIBRARY_PATH ${CARLA_DEPENDENCIES_PATH}/zlib-install/lib)

set (
    CARLA_COMMON_DEFINITIONS
    BOOST_ERROR_CODE_HEADER_ONLY
    LIBCARLA_IMAGE_WITH_PNG_SUPPORT
)

if (WIN32)
    # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
    list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # Windows 10
    list (APPEND CARLA_COMMON_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
endif ()

if (ENABLE_EXCEPTIONS)
else ()
    list (APPEND CARLA_COMMON_DEFINITIONS ASIO_NO_EXCEPTIONS)
    list (APPEND CARLA_COMMON_DEFINITIONS BOOST_NO_EXCEPTIONS)
    list (APPEND CARLA_COMMON_DEFINITIONS LIBCARLA_NO_EXCEPTIONS)
    list (APPEND CARLA_COMMON_DEFINITIONS PUGIXML_NO_EXCEPTIONS)
endif ()

if (ENABLE_RTTI)
else ()
    list (APPEND CARLA_COMMON_DEFINITIONS BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
endif ()

if (BUILD_LIBCARLA_SERVER OR BUILD_LIBCARLA_CLIENT)
    configure_file (
        ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
        ${LIBCARLA_SOURCE_PATH}/carla/Version.h)
endif ()

if (BUILD_LIBCARLA_SERVER)

    project (carla-server)

    set (LIBCARLA_SERVER_INCLUDE_PATHS
        ${LIBCARLA_SOURCE_PATH}
        ${BOOST_INCLUDE_PATH}
        ${RPCLIB_INCLUDE_PATH}
    )

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
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.h
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

    target_compile_options (
        carla-server PRIVATE
        /EHsc
    )

    target_include_directories (
        carla-server PRIVATE
        ${LIBCARLA_SERVER_INCLUDE_PATHS}
    )
    
    target_compile_definitions (
        carla-server PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
    )

    install (TARGETS carla-server DESTINATION lib)
    
    foreach (HEADER ${LIBCARLA_SERVER_HEADERS})
        cmake_path (GET HEADER PARENT_PATH HEADER_PARENT)
        cmake_path (RELATIVE_PATH HEADER_PARENT BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla OUTPUT_VARIABLE HEADER_RELATIVE)
        install (FILES ${HEADER} DESTINATION include/carla/${HEADER_RELATIVE})
    endforeach ()

endif ()



if (BUILD_LIBCARLA_CLIENT)

    project (carla-client)

    set (
        LIBCARLA_CLIENT_INCLUDE_PATHS
        ${LIBCARLA_SOURCE_PATH}
        ${BOOST_INCLUDE_PATH}
        ${RPCLIB_INCLUDE_PATH}
        ${RECAST_INCLUDE_PATH}
        ${LIBPNG_INCLUDE_PATH}
    )

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
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/data/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/trafficmanager/*.h
    )

    if (ENABLE_LIBCARLA_CLIENT_RSS)
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

    if (ENABLE_LIBCARLA_CLIENT_RSS)
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

    target_include_directories (
        carla-client PRIVATE
        ${LIBCARLA_CLIENT_INCLUDE_PATHS}
    )

    target_compile_definitions (
        carla-client PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
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