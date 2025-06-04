project (
  libcarla
  LANGUAGES
    CXX
  VERSION
    ${CARLA_VERSION}
)

set (
  LIBCARLA_SOURCE_PATH
  ${CARLA_WORKSPACE_PATH}/LibCarla/source
)

carla_two_step_configure_file (
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
)

if (WIN32)

  set (
    WINNT_MACRO_PATH
    ${LIBCARLA_SOURCE_PATH}/util/winnt-macros.txt
  )

  file (
    STRINGS
    ${WINNT_MACRO_PATH}
    WINDOWS_MACRO_LIST
  )

  set (
    GUARD_BEGIN_FILE_TEXT
    ""
  )

  set (
    GUARD_END_FILE_TEXT
    ""
  )

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

  file (
    WRITE
    ${LIBCARLA_SOURCE_PATH}/util/ue-header-guard-begin.h
    ${GUARD_BEGIN_FILE_TEXT}
  )

  file (
    WRITE
    ${LIBCARLA_SOURCE_PATH}/util/ue-header-guard-end.h
    ${GUARD_END_FILE_TEXT}
  )

  set_directory_properties(
    PROPERTIES
    CMAKE_CONFIGURE_DEPENDS
    ${WINNT_MACRO_PATH}
  )

else ()

  file (
    WRITE
    ${LIBCARLA_SOURCE_PATH}/util/ue-header-guard-begin.h
    ""
  )

  file (
    WRITE
    ${LIBCARLA_SOURCE_PATH}/util/ue-header-guard-end.h
    ""
  )

endif ()



if (BUILD_CARLA_SERVER)

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

  if (ENABLE_ROS2)
    file (
      GLOB
      LIBCARLA_SERVER_SOURCES_ROS2
      ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.cpp
    )
    list (
      APPEND
      LIBCARLA_SERVER_SOURCES
      ${LIBCARLA_SERVER_SOURCES_ROS2}
    )
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
    ${LIBCARLA_SOURCE_PATH}/util/winnt-macros.txt
  )

  if (ENABLE_ROS2)
    file (
      GLOB
      LIBCARLA_SERVER_HEADERS_ROS2
      ${LIBCARLA_SOURCE_PATH}/carla/ros2/*.h
    )
    list (
      APPEND
      LIBCARLA_SERVER_HEADERS
      ${LIBCARLA_SERVER_HEADERS_ROS2}
    )
    set (CARLA_ROS2_DEFINITIONS WITH_ROS2)
  endif ()

  if (ENABLE_ROS2_DEMO)
    set (CARLA_ROS2_DEFINITIONS ${CARLA_ROS2_DEFINITIONS} WITH_ROS2_DEMO)
  endif ()

  file (
    GLOB
    LIBCARLA_SERVER_HEADERS_THIRD_PARTY
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/moodycamel/*.h
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.hpp
  )

  carla_add_library (
    carla-server
    "Build the CARLA server."
    ${LIBCARLA_SERVER_HEADERS}
    ${LIBCARLA_SERVER_SOURCES}
    ${LIBCARLA_SERVER_HEADERS_THIRD_PARTY}
    ${LIBCARLA_SERVER_SOURCES_THIRD_PARTY}
  )

  target_link_libraries (
    carla-server PUBLIC SYSTEM
    Boost::asio
    Boost::geometry
    Boost::algorithm
    Boost::assert
    Boost::gil
    RecastNavigation::Recast
    RecastNavigation::Detour
    RecastNavigation::DetourCrowd
    png_static
    zlibstatic
    rpc
  )
  
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

  if (ENABLE_ROS2)
    add_dependencies (carla-server carla-ros2-native)
  endif ()

endif ()



if (BUILD_CARLA_CLIENT)

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
    ${LIBCARLA_SOURCE_PATH}/util/winnt-macros.txt
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
    ${LIBCARLA_SOURCE_PATH}/third-party/odrSpiral/*.cpp
    ${LIBCARLA_SOURCE_PATH}/third-party/pugixml/*.cpp
  )

  carla_add_library (
    carla-client
    "Build the CARLA client."
    ${LIBCARLA_CLIENT_HEADERS}
    ${LIBCARLA_CLIENT_SOURCES}
    ${LIBCARLA_CLIENT_HEADERS_THIRD_PARTY}
    ${LIBCARLA_CLIENT_SOURCES_THIRD_PARTY}
  )

  target_link_libraries (
    carla-client PUBLIC
    Boost::asio
    Boost::python
    Boost::algorithm
    Boost::geometry
    Boost::assert
    Boost::gil
    RecastNavigation::Recast
    RecastNavigation::Detour
    RecastNavigation::DetourCrowd
    png_static
    zlibstatic
    rpc
  )
  
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

endif ()
