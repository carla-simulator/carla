cmake_minimum_required (
  VERSION
  3.30
)

set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 10)
set (CARLA_VERSION_PATCH 0)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

project (
  LibCarla
  LANGUAGES
    CXX
  VERSION
    ${CARLA_VERSION}
)

option (BUILD_SERVER "Build the CARLA server." ON)
option (BUILD_CLIENT "Build the CARLA client." ON)
option (ENABLE_RSS "Enable RSS components (ad-rss-lib)" OFF)
option (ENABLE_ROS2 "Enable ROS2." OFF)
option (ENABLE_ROS2_DEMO "Enable ROS2 demo." OFF)

include (CMake/Config.cmake)

find_package (
  Boost ${BOOST_VERSION} EXACT REQUIRED
  COMPONENTS
    asio geometry algorithm assert gil
)

find_package (
  RecastNavigation REQUIRED
  COMPONENTS
    Recast Detour DetourCrowd
)

find_package (Eigen3 ${EIGEN_VERSION} EXACT REQUIRED)
find_package (ZLIB ${ZLIB_VERSION} EXACT REQUIRED)
find_package (RPCLib ${RPC_VERSION} EXACT REQUIRED)
find_package (PNG ${PNG_VERSION} EXACT)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (BUILD_SHARED_LIBS)
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

set (
  LIBCARLA_SOURCE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set (LIBCARLA_IMAGE_SUPPORTED_FORMATS_DEFAULT)
if (TARGET PNG::PNG)
  list (APPEND LIBCARLA_IMAGE_SUPPORTED_FORMATS_DEFAULT "png")
endif ()

set (
  LIBCARLA_IMAGE_SUPPORTED_FORMATS
  "${LIBCARLA_IMAGE_SUPPORTED_FORMATS_DEFAULT}"
  CACHE STRING
  "Semicolon-separated list of supported image formats by LibCarla. Available formats: png, jpeg, tiff."
)

set (CARLA_COMMON_DEFINITIONS)

foreach (FORMAT ${LIBCARLA_IMAGE_SUPPORTED_FORMATS})
  message (STATUS "Enabling CARLA image support for \"${FORMAT}\".")
  string (TOUPPER "${FORMAT}" FORMAT_UPPERCASE)
  list (APPEND CARLA_COMMON_DEFINITIONS LIBCARLA_IMAGE_SUPPORT_${FORMAT_UPPERCASE}=1)
endforeach ()

if (WIN32)
  # Documentation: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # <- Windows 10
  list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
  list (APPEND CARLA_COMMON_DEFINITIONS _USE_MATH_DEFINES)
endif ()



file (
  GENERATE
  OUTPUT
    ${CMAKE_BINARY_DIR}/carla/Version.h # @TODO This should be in a binary dir
  INPUT
    ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
)

if (WIN32)

  set (WINNT_MACRO_PATH ${LIBCARLA_SOURCE_PATH}/util/winnt-macros.txt)
  file (STRINGS ${WINNT_MACRO_PATH} WINDOWS_MACRO_LIST)

  set (GUARD_BEGIN_FILE_TEXT "")
  set (GUARD_END_FILE_TEXT "")

  foreach (WINDOWS_MACRO ${WINDOWS_MACRO_LIST})
    string (
      APPEND
      GUARD_BEGIN_FILE_TEXT
      "#pragma push_macro(\"${WINDOWS_MACRO}\")\n"
      "#undef ${WINDOWS_MACRO}\n"
    )
    string (
      APPEND
      GUARD_END_FILE_TEXT
      "#pragma pop_macro(\"${WINDOWS_MACRO}\")\n"
    )
  endforeach ()

  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-begin.h ${GUARD_BEGIN_FILE_TEXT})
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-end.h ${GUARD_END_FILE_TEXT})

else ()

  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-begin.h "")
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-end.h "")

endif ()

if (BUILD_SERVER)

  file (
    GLOB
    LIBCARLA_SERVER_SOURCES
    ${LIBCARLA_SOURCE_PATH}/carla/geom/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/opendrive/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/opendrive/parser/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/road/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/road/element/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/road/general/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/road/object/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/road/signal/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/rpc/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/streaming/detail/tcp/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/multigpu/*.cpp
  )
  
  list (
    APPEND
    LIBCARLA_SERVER_SOURCES
    ${LIBCARLA_SOURCE_PATH}/carla/Buffer.cpp
    # ${LIBCARLA_SOURCE_PATH}/carla/Exception.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/sensor/s11n/SensorHeaderSerializer.cpp
  )
  
  if (ENABLE_ROS2)
    file (GLOB LIBCARLA_SERVER_SOURCES_ROS2 ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.cpp)
    list (APPEND LIBCARLA_SERVER_SOURCES ${LIBCARLA_SERVER_SOURCES_ROS2})
  endif ()
  
  file (
    GLOB
    LIBCARLA_SERVER_SOURCES_THIRD_PARTY
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.cpp
    ${LIBCARLA_SOURCE_PATH}/third-party/moodycamel/*.cpp
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.cpp
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
    ${LIBCARLA_SOURCE_PATH}/util/*.h
  )
  
  list (
    APPEND
    LIBCARLA_SERVER_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-begin.h
    ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-end.h
  )
  
  set (CARLA_ROS2_DEFINITIONS)

  if (ENABLE_ROS2)
    file (GLOB LIBCARLA_SERVER_HEADERS_ROS2 ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.h)
    list (APPEND LIBCARLA_SERVER_HEADERS ${LIBCARLA_SERVER_HEADERS_ROS2})
    list (APPEND CARLA_ROS2_DEFINITIONS WITH_ROS2)
  endif ()
  
  if (ENABLE_ROS2_DEMO)
    list (APPEND CARLA_ROS2_DEFINITIONS WITH_ROS2_DEMO)
  endif ()
  
  file (
    GLOB
    LIBCARLA_SERVER_HEADERS_THIRD_PARTY
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/moodycamel/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.hpp
  )
  
  add_library (
    carla-server
    ${LIBCARLA_SERVER_HEADERS}
    ${LIBCARLA_SERVER_SOURCES}
    ${LIBCARLA_SERVER_HEADERS_THIRD_PARTY}
    ${LIBCARLA_SERVER_SOURCES_THIRD_PARTY}
  )
  
  target_link_libraries (
    carla-server PUBLIC
    Boost::asio
    Boost::geometry
    Boost::algorithm
    Boost::assert
    Boost::gil
    RecastNavigation::Recast
    RecastNavigation::Detour
    RecastNavigation::DetourCrowd
    ZLIB::ZLIB
    rpclib::rpc
  )

  if (TARGET PNG::PNG)
    target_link_libraries (carla-server PUBLIC PNG::PNG)
  endif ()
  
  target_include_directories (
    carla-server PUBLIC
    ${LIBCARLA_SOURCE_PATH}
  )
  
  target_compile_definitions (
    carla-server PUBLIC
    ${CARLA_COMMON_DEFINITIONS}
    ${CARLA_EXCEPTION_DEFINITIONS}
    ${CARLA_RTTI_DEFINITIONS}
    ${CARLA_ROS2_DEFINITIONS}
  )

  install (
    TARGETS
      carla-server
    DESTINATION
      lib
  )
endif ()



if (BUILD_CLIENT)
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
    ${LIBCARLA_SOURCE_PATH}/util/*.h
  )
  
  list (
    APPEND
    LIBCARLA_CLIENT_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-begin.h
    ${CMAKE_CURRENT_BINARY_DIR}/ue-header-guard-end.h
  )
  
  if (ENABLE_RSS)
    file (GLOB LIBCARLA_CLIENT_HEADERS_RSS ${LIBCARLA_SOURCE_PATH}/carla/rss/*.h)
    list (APPEND LIBCARLA_CLIENT_HEADERS ${LIBCARLA_CLIENT_HEADERS_RSS})
  endif ()
  
  file (
    GLOB
    LIBCARLA_CLIENT_HEADERS_THIRD_PARTY
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/moodycamel/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.hpp
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.h
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
  
  if (ENABLE_RSS)
    file (GLOB LIBCARLA_CLIENT_SOURCES_RSS ${LIBCARLA_SOURCE_PATH}/carla/rss/*.cpp)
    list (APPEND LIBCARLA_CLIENT_SOURCES ${LIBCARLA_CLIENT_SOURCES_RSS})
  endif ()
  
  file (
    GLOB
    LIBCARLA_CLIENT_SOURCES_THIRD_PARTY
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.cpp
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.cpp
  )
  
  add_library (
    carla-client
    ${LIBCARLA_CLIENT_HEADERS}
    ${LIBCARLA_CLIENT_SOURCES}
    ${LIBCARLA_CLIENT_HEADERS_THIRD_PARTY}
    ${LIBCARLA_CLIENT_SOURCES_THIRD_PARTY}
  )
  
  target_link_libraries (
    carla-client PUBLIC
    Boost::asio
    Boost::geometry
    Boost::algorithm
    Boost::assert
    Boost::gil
    RecastNavigation::Recast
    RecastNavigation::Detour
    RecastNavigation::DetourCrowd
    ZLIB::ZLIB
    rpclib::rpc
  )
  
  if (TARGET PNG::PNG)
    target_link_libraries (carla-client PUBLIC PNG::PNG)
  endif ()

  target_include_directories (
    carla-client PUBLIC
    ${LIBCARLA_SOURCE_PATH}
  )
  
  target_compile_definitions (
    carla-client PUBLIC
    ${CARLA_COMMON_DEFINITIONS}
    ${CARLA_EXCEPTION_DEFINITIONS}
    ${CARLA_RTTI_DEFINITIONS}
  )

  install (
    TARGETS
      carla-client
    DESTINATION
      lib
  )
endif ()

if (BUILD_EXAMPLES)
  add_subdirectory (examples)
endif ()
