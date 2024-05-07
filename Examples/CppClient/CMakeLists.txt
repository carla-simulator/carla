project (carla-example-cpp-client)

carla_add_executable (
  carla-example-cpp-client
  "Build the CARLA C++ client example."
  main.cpp
)

target_link_libraries (
  carla-example-cpp-client PRIVATE
  carla-client
)
