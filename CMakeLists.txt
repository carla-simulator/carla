cmake_minimum_required (VERSION 3.12.0)

project (CARLA)

set (CARLA_VERSION)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

option (LIBCARLA_CLIENT "Whether to build the LibCarla client." ON)
option (LIBCARLA_SERVER "Whether to build the LibCarla server." ON)
option (LIBCARLA_PYTORCH_ENABLE "Whether to enable pytorch." OFF)
option (PYTHON_API "Whether to build the CARLA Python API." ON)
option (LIBCARLA_CLIENT_RSS "Whether to enable RSS components (ad-rss-lib)" OFF)
option (LIBCARLA_INSTALL "Whether to install LibCarla and all of its dependencies." ON)
option (OSM_ENABLE "" OFF)

set (LIBCARLA_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/LibCarla/source)
set (LIBCARLA_THIRD_PARTY_SOURCE_PATH ${LIBCARLA_SOURCE_PATH}/third-party)
set (PYTHONAPI_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/PythonAPI)

set (BOOST_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/boost-install/include)
set (BOOST_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/boost-install/lib)
set (CHRONO_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/chrono-install/include)
set (CHRONO_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/chrono-install/lib)
set (EIGEN_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/eigen-install/include)
set (EIGEN_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/eigen-install/lib)
set (GOOGLETEST_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/gtest-install/include)
set (GOOGLETEST_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/gtest-install/lib)
set (LIBPNG_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/libpng-install/include)
set (LIBPNG_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/libpng-install/lib)
set (PROJ_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/proj-install/include)
set (PROJ_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/proj-install/lib)
set (RECAST_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/recast-install/include)
set (RECAST_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/recast-install/lib)
set (RPCLIB_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/rpclib-install/include)
set (RPCLIB_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/rpclib-install/lib)
set (SQLITE_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/sqlite-install/include)
set (SQLITE_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/sqlite-install/lib)
set (XERCESC_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/xercesc-install/include)
set (XERCESC_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/xercesc-install/lib)
set (ZLIB_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/zlib-install/include)
set (ZLIB_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/zlib-install/lib)
set (ZLIB_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/zlib-source)
set (ZLIB_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/zlib-build)

add_compile_definitions (BOOST_ERROR_CODE_HEADER_ONLY)
add_compile_definitions (LIBCARLA_IMAGE_WITH_PNG_SUPPORT)

if (WIN32)
    # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
    add_compile_definitions (_WIN32_WINNT=0x0A00) # Windows 10
    add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
endif ()

if (LIBCARLA_SERVER)
    set (LIBCARLA_SERVER_INCLUDE_PATHS
        ${LIBCARLA_SOURCE_PATH}
        ${BOOST_INCLUDE_PATH}
        ${RPCLIB_INCLUDE_PATH})
    add_compile_definitions (BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
    add_compile_options (/EHsc)
    add_compile_definitions (ASIO_NO_EXCEPTIONS)
    add_compile_definitions (BOOST_NO_EXCEPTIONS)
    add_compile_definitions (LIBCARLA_NO_EXCEPTIONS)
    add_compile_definitions (PUGIXML_NO_EXCEPTIONS)
    file(GLOB LIBCARLA_SERVER_SOURCES
        ${LIBCARLA_SOURCE_PATH}/carla/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/Buffer.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/Exception.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/geom/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/SensorHeaderSerializer.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.hpp
    )
    add_library (LibCarla-Server STATIC ${LIBCARLA_SERVER_SOURCES})
    target_include_directories (LibCarla-Server PRIVATE ${LIBCARLA_SERVER_INCLUDE_PATHS})
endif ()

if (LIBCARLA_CLIENT)
    project (LibCarla-Client)
    set (LIBCARLA_CLIENT_INCLUDE_PATHS
        ${LIBCARLA_SOURCE_PATH}
        ${BOOST_INCLUDE_PATH}
        ${RPCLIB_INCLUDE_PATH}
        ${RECAST_INCLUDE_PATH}
        ${LIBPNG_INCLUDE_PATH})
    file (GLOB LIBCARLA_CLIENT_SOURCES
        ${LIBCARLA_SOURCE_PATH}/carla/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/*.cpp
        ${LIBCARLA_SOURCE_PATH}/carla/client/*.h
        ${LIBCARLA_SOURCE_PATH}/carla/client/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/client/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/client/detail/*.h
        ${LIBCARLA_SOURCE_PATH}carla/geom/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/geom/*.h
        ${LIBCARLA_SOURCE_PATH}carla/image/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/image/*.h
        ${LIBCARLA_SOURCE_PATH}carla/nav/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/nav/*.h
        ${LIBCARLA_SOURCE_PATH}carla/opendrive/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/opendrive/*.h
        ${LIBCARLA_SOURCE_PATH}carla/opendrive/parser/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/opendrive/parser/*.h
        ${LIBCARLA_SOURCE_PATH}carla/pointcloud/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/pointcloud/*.h
        ${LIBCARLA_SOURCE_PATH}carla/profiler/*.h
        ${LIBCARLA_SOURCE_PATH}carla/road/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/road/*.h
        ${LIBCARLA_SOURCE_PATH}carla/road/element/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/road/element/*.h
        ${LIBCARLA_SOURCE_PATH}carla/road/general/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/road/general/*.h
        ${LIBCARLA_SOURCE_PATH}carla/road/object/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/road/object/*.h
        ${LIBCARLA_SOURCE_PATH}carla/road/signal/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/road/signal/*.h
        ${LIBCARLA_SOURCE_PATH}carla/rpc/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/rpc/*.h
        ${LIBCARLA_SOURCE_PATH}carla/rss/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/rss/*.h
        ${LIBCARLA_SOURCE_PATH}carla/sensor/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/sensor/*.h
        ${LIBCARLA_SOURCE_PATH}carla/sensor/data/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/sensor/data/*.h
        ${LIBCARLA_SOURCE_PATH}carla/sensor/s11n/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/sensor/s11n/*.h
        ${LIBCARLA_SOURCE_PATH}carla/streaming/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/streaming/*.h
        ${LIBCARLA_SOURCE_PATH}carla/streaming/detail/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/streaming/detail/*.h
        ${LIBCARLA_SOURCE_PATH}carla/streaming/detail/tcp/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/streaming/detail/tcp/*.h
        ${LIBCARLA_SOURCE_PATH}carla/streaming/low_level/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/streaming/low_level/*.h
        ${LIBCARLA_SOURCE_PATH}carla/multigpu/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/multigpu/*.h
        ${LIBCARLA_SOURCE_PATH}carla/trafficmanager/*.cpp
        ${LIBCARLA_SOURCE_PATH}carla/trafficmanager/*.h

        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.h
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.cpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.hpp
        ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.h
    )
    add_library (LibCarla-Client STATIC ${LIBCARLA_CLIENT_SOURCES})
    target_include_directories (LibCarla-Client PRIVATE ${LIBCARLA_CLIENT_INCLUDE_PATHS})
endif ()

if (LIBCARLA_INSTALL)
    file (GLOB_RECURSE LIBCARLA_HEADERS ${LIBCARLA_SOURCE_PATH}/**/*.h,*.hpp)
    set (LIBCARLA_HEADERS_RELATIVE)
    foreach (HEADER LIBCARLA_HEADERS)
        cmake_path (RELATIVE_PATH HEADER BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH} OUTPUT_VARIABLE HEADER_RELATIVE)
        list (APPEND LIBCARLA_HEADERS_RELATIVE HEADER_RELATIVE)
    endforeach ()
    install (FILES LIBCARLA_HEADERS DESTINATION LIBCARLA_HEADERS_RELATIVE)
endif ()