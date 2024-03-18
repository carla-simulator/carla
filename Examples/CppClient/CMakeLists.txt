project (carla-example-cpp-client)

add_executable (
  carla-example-cpp-client
  main.cpp
)

target_link_libraries (
  carla-example-cpp-client PRIVATE
  carla-client
)
