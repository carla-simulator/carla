set (
  CARLA_EXAMPLE_CPP_CLIENT_SOURCE_PATH
  ${CARLA_WORKSPACE_PATH}/Examples/CppClient
)

set (
  CARLA_EXAMPLE_CPP_DEPENDENCIES
  carla-client
  rpc
  png_static
  zlibstatic
  Boost::asio
  Boost::python
  Boost::geometry
  Boost::gil
  RecastNavigation::Recast
  RecastNavigation::Detour
  RecastNavigation::DetourCrowd
)

if (BUILD_EXAMPLES)
  carla_message ("CARLA Building C++ EXAMPLE CLIENT")

  project (
    carla-example-cpp-client
    LANGUAGES
      CXX
    VERSION
      ${CARLA_VERSION}
  )

  file (
    GLOB_RECURSE 
    CARLA_EXAMPLE_CPP_SOURCES
    CONFIGURE_DEPENDS 
    ${CARLA_EXAMPLE_CPP_CLIENT_SOURCE_PATH}/*.cpp
  )

  file (
    GLOB_RECURSE 
    CARLA_EXAMPLE_CPP_HEADERS
    CONFIGURE_DEPENDS 
    ${CARLA_EXAMPLE_CPP_CLIENT_SOURCE_PATH}/*.h
  )
  
  carla_add_executable (
    carla-example-client
    "Build the CARLA C++ client example."
    ${CARLA_EXAMPLE_CPP_SOURCES}
  )
    
  add_dependencies (
    carla-example-client
    ${CARLA_EXAMPLE_CPP_DEPENDENCIES}
  )

  target_link_libraries (
    carla-example-client PUBLIC
    carla-client
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
endif ()
