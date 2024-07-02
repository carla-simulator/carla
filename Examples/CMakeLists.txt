set (
  CARLA_EXAMPLE_CPP_CLIENT_SOURCE_PATH
  ${CARLA_WORKSPACE_PATH}/Examples/CppClient
)

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
  ${CARLA_EXAMPLE_CPP_HEADERS}
)
  
target_link_libraries (
  carla-example-client PUBLIC
  carla-client
)

