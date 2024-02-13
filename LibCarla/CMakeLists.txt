set (LIBCARLA_SOURCE_PATH ${CARLA_WORKSPACE_PATH}/LibCarla/source)
set (LIBCARLA_THIRD_PARTY_SOURCE_PATH ${LIBCARLA_SOURCE_PATH}/third-party)

set (CARLA_PYTHON_VERSION_MAJOR 3)
set (CARLA_PYTHON_VERSION_MINOR 8)

configure_file (
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
  ${LIBCARLA_SOURCE_PATH}/carla/Version.h
)

if (BUILD_CARLA_SERVER OR BUILD_CARLA_CLIENT)
  find_package (
    Boost
    ${CARLA_BOOST_VERSION} EXACT
    COMPONENTS
      system
      python${CARLA_PYTHON_VERSION_MAJOR}${CARLA_PYTHON_VERSION_MINOR}
  )

  if (NOT ${Boost_FOUND})
    include (${CARLA_WORKSPACE_PATH}/CMake/BuildBoost.cmake)
  endif ()
  
  include (${CARLA_WORKSPACE_PATH}/CMake/BuildRPCLib.cmake)
endif ()



if (BUILD_CARLA_SERVER)

    project (
      carla-server
      LANGUAGES CXX
      VERSION ${CARLA_VERSION}
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

    target_link_libraries (
      carla-server
      Boost::asio
      Boost::algorithm
      Boost::assert
      rpc
    )
    
    target_include_directories (
      carla-server PRIVATE
      ${LIBCARLA_SOURCE_PATH}
    )

    target_compile_definitions (
        carla-server PRIVATE
        ${CARLA_COMMON_DEFINITIONS}
        ${CARLA_EXCEPTION_DEFINITIONS}
        ${CARLA_RTTI_DEFINITIONS}
    )

    install (
      TARGETS
      carla-server
      DESTINATION lib
    )

    foreach (HEADER ${LIBCARLA_SERVER_HEADERS})
        cmake_path (GET HEADER PARENT_PATH HEADER_PARENT)
        cmake_path (
          RELATIVE_PATH HEADER_PARENT
          BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla
          OUTPUT_VARIABLE HEADER_RELATIVE
        )
        install (
          FILES ${HEADER}
          DESTINATION include/carla/${HEADER_RELATIVE}
        )
    endforeach ()

endif ()



if (BUILD_CARLA_CLIENT)

    project (
      carla-client
      LANGUAGES CXX
      VERSION ${CARLA_VERSION}
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
    
    target_link_libraries (
      carla-client
      Boost::asio
      Boost::algorithm
      Boost::assert
      rpc
    )
    
    target_include_directories (
      carla-client PRIVATE
      ${LIBCARLA_SOURCE_PATH}
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
        cmake_path (
          RELATIVE_PATH HEADER_PARENT
          BASE_DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla
          OUTPUT_VARIABLE HEADER_RELATIVE
        )
        install (
          FILES ${HEADER}
          DESTINATION include/carla/${HEADER_RELATIVE}
        )
    endforeach ()

endif ()
