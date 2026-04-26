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

# Standalone/ and AgentDemo/ are independently buildable demos (their own
# entry points + CMakeLists for out-of-tree consumption); fold them out
# of the parent target's source list to avoid duplicate `main` symbols.
list (
  FILTER CARLA_EXAMPLE_CPP_SOURCES EXCLUDE REGEX
  ".*/Examples/CppClient/(Standalone|AgentDemo)/.*"
)
list (
  FILTER CARLA_EXAMPLE_CPP_HEADERS EXCLUDE REGEX
  ".*/Examples/CppClient/(Standalone|AgentDemo)/.*"
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

# AgentDemo — driveability test for the C++ navigation agents (#9554).
carla_add_executable (
  carla-agent-demo
  "Build the C++ navigation-agents driveability demo."
  ${CARLA_EXAMPLE_CPP_CLIENT_SOURCE_PATH}/AgentDemo/main.cpp
)

target_link_libraries (
  carla-agent-demo PUBLIC
  carla-client
)

# Qt-based community tool example (builds a dockable plugin + standalone exe).
# Skips itself cleanly when Qt is not installed.
add_subdirectory (${CARLA_WORKSPACE_PATH}/Examples/QtClient QtClient)

