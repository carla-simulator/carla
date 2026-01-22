cmake_minimum_required (
  VERSION
  3.28
)

if (CARLA_CONFIG_FILEPATH)
  include (${CARLA_CONFIG_FILEPATH})
else ()
  message (WARNING "Missing \"CARLA_CONFIG_FILEPATH\"")
endif ()

project (
  LibCarla
  LANGUAGES
    C CXX
  VERSION
    ${CARLA_VERSION}
)

option (BUILD_SERVER "Build the CARLA server." ON)
option (BUILD_CLIENT "Build the CARLA client." ON)
option (ENABLE_RSS "Enable RSS components (ad-rss-lib)" OFF)
option (ENABLE_ROS2 "Enable ROS2." OFF)
option (ENABLE_ROS2_DEMO "Enable ROS2 demo." OFF)

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
find_package (RPCLib REQUIRED)
find_package (PNG ${PNG_VERSION} EXACT REQUIRED)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (BUILD_SHARED_LIBS)
  set (CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

set (
  LIBCARLA_SOURCE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/Source
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
  set (LIBCARLA_IMAGE_SUPPORT_${FORMAT_UPPERCASE} TRUE CACHE BOOL "")
endforeach ()

if (WIN32)
  # Documentation: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # <- Windows 10
  list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
  list (APPEND CARLA_COMMON_DEFINITIONS _USE_MATH_DEFINES)
endif ()



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

configure_file (
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/carla/Version.h
)

configure_file (
  ${LIBCARLA_SOURCE_PATH}/carla/Config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/carla/Config.h
)

set (
  LIBCARLA_GENERATED_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/carla/Version.h
  ${CMAKE_CURRENT_BINARY_DIR}/carla/Config.h
)

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
    ${LIBCARLA_GENERATED_HEADERS}
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
    Server
    ${LIBCARLA_SERVER_HEADERS}
    ${LIBCARLA_SERVER_SOURCES}
    ${LIBCARLA_SERVER_HEADERS_THIRD_PARTY}
    ${LIBCARLA_SERVER_SOURCES_THIRD_PARTY}
  )

  add_library (LibCarla::Server ALIAS Server)
  
  target_link_libraries (
    Server PUBLIC
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
    target_link_libraries (Server PUBLIC PNG::PNG)
  endif ()
  
  target_include_directories (
    Server PUBLIC
    $<BUILD_INTERFACE:${LIBCARLA_SOURCE_PATH}>
    $<INSTALL_INTERFACE:include>
  )
  
  target_include_directories (
    Server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  
  target_compile_definitions (
    Server PUBLIC
    ${CARLA_COMMON_DEFINITIONS}
    ${CARLA_EXCEPTION_DEFINITIONS}
    ${CARLA_RTTI_DEFINITIONS}
    ${CARLA_ROS2_DEFINITIONS}
  )
  
  foreach (HEADER ${LIBCARLA_SERVER_HEADERS})
    cmake_path (
      GET
        HEADER
      PARENT_PATH
        HEADER_PARENT
    )
    cmake_path (
      RELATIVE_PATH
        HEADER_PARENT
      BASE_DIRECTORY
        ${LIBCARLA_SOURCE_PATH}
      OUTPUT_VARIABLE
        HEADER_PARENT_RELATIVE
    )
    install (
      FILES
        ${HEADER}
      DESTINATION
        include/${HEADER_PARENT_RELATIVE}
    )
  endforeach ()

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
    ${LIBCARLA_GENERATED_HEADERS}
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
    Client
    ${LIBCARLA_CLIENT_HEADERS}
    ${LIBCARLA_CLIENT_SOURCES}
    ${LIBCARLA_CLIENT_HEADERS_THIRD_PARTY}
    ${LIBCARLA_CLIENT_SOURCES_THIRD_PARTY}
  )
  
  add_library (LibCarla::Client ALIAS Client)

  target_link_libraries (
    Client PUBLIC
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
    target_link_libraries (Client PUBLIC PNG::PNG)
  endif ()

  target_include_directories (
    Client PUBLIC
    $<BUILD_INTERFACE:${LIBCARLA_SOURCE_PATH}>
    $<INSTALL_INTERFACE:include>
  )
  
  target_include_directories (
    Client PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  
  target_compile_definitions (
    Client PUBLIC
    ${CARLA_COMMON_DEFINITIONS}
    ${CARLA_EXCEPTION_DEFINITIONS}
    ${CARLA_RTTI_DEFINITIONS}
  )

  foreach (HEADER ${LIBCARLA_CLIENT_HEADERS})
    cmake_path (
      GET
        HEADER
      PARENT_PATH
        HEADER_PARENT
    )
    cmake_path (IS_PREFIX LIBCARLA_SOURCE_PATH ${HEADER} FROM_SOURCE)
    cmake_path (IS_PREFIX CMAKE_CURRENT_BINARY_DIR ${HEADER} FROM_BINARY)
    if (FROM_SOURCE)
      set (PARENT_PATH ${LIBCARLA_SOURCE_PATH})
    elseif (FROM_BINARY)
      set (PARENT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    else ()
      message (FATAL_ERROR "Could not determine where to install ${HEADER}.")
    endif ()
    cmake_path (
      RELATIVE_PATH
        HEADER_PARENT
      BASE_DIRECTORY
        ${PARENT_PATH}
      OUTPUT_VARIABLE
        HEADER_PARENT_RELATIVE
    )
    install (
      FILES
        ${HEADER}
      DESTINATION
        include/${HEADER_PARENT_RELATIVE}
    )
  endforeach ()

endif ()

set (LIBCARLA_TARGETS)

if (BUILD_CLIENT)
  list (APPEND LIBCARLA_TARGETS Client)
endif ()

if (BUILD_SERVER)
  list (APPEND LIBCARLA_TARGETS Server)
endif ()

install (
  TARGETS
    ${LIBCARLA_TARGETS}
  EXPORT LibCarlaTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install (
  EXPORT LibCarlaTargets
  FILE LibCarlaTargets.cmake
  NAMESPACE LibCarla::
  DESTINATION lib/cmake/LibCarla
)

include (CMakePackageConfigHelpers)

configure_package_config_file (
  ${CMAKE_CURRENT_LIST_DIR}/Cmake/LibCarlaConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LibCarlaConfig.cmake
  INSTALL_DESTINATION lib/cmake/LibCarla
)

install (
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LibCarlaConfig.cmake
  DESTINATION
    lib/cmake/LibCarla
)

write_basic_package_version_file (
  ${CMAKE_CURRENT_BINARY_DIR}/LibCarlaConfigVersion.cmake
  VERSION
    ${CARLA_VERSION}
  COMPATIBILITY
    SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LibCarlaConfigVersion.cmake
  DESTINATION
    lib/cmake/LibCarla
)

if (BUILD_EXAMPLES)
  add_subdirectory (Examples)
endif ()
