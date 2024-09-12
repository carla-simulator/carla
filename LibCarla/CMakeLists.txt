set (
  LIBCARLA_SOURCE_PATH
  ${CARLA_WORKSPACE_PATH}/LibCarla/source
)

set (
  LIBCARLA_THIRD_PARTY_SOURCE_PATH
  ${LIBCARLA_SOURCE_PATH}/third-party
)

carla_two_step_configure_file (
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
)

if (BUILD_CARLA_SERVER)

  project (
    carla-server
    LANGUAGES
      CXX
    VERSION
      ${CARLA_VERSION}
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
    ${LIBCARLA_SOURCE_PATH}/compiler/*.h
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
    ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/odrSpiral/*.h
    ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/moodycamel/*.h
    ${LIBCARLA_THIRD_PARTY_SOURCE_PATH}/pugixml/*.hpp
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
    carla-server PUBLIC
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

  project (
    carla-client
    LANGUAGES
      CXX
    VERSION
      ${CARLA_VERSION}
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
    ${CARLA_RTTI_DEFINITIONS}
  )

endif ()



if (ENABLE_ROS2)

  file (
    GLOB
    LIBCARLA_ROS2_HEADERS
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.h
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.h
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/listeners/*.h
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/types/*.h
  )

  file (
    GLOB
    LIBCARLA_ROS2_SOURCES
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/publishers/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/subscribers/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/listeners/*.cpp
    ${LIBCARLA_SOURCE_PATH}/carla/ros2/types/*.cpp
  )

  carla_add_library (
    carla-ros2-native
    ${LIBCARLA_ROS2_HEADERS}
    ${LIBCARLA_ROS2_SOURCES}
  )

  target_compile_definitions (
    carla-ros2-native PUBLIC
    BOOST_ASIO_ENABLE_BUFFER_DEBUGGING
  )

  install (
    TARGETS
    carla-ros2-native
    DESTINATION
    lib
  )

endif ()
