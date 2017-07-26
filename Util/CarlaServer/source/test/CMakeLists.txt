file(GLOB test_carla_server_SRC
    "*.h"
    "*.cpp"
)
set(test_carla_server_SRC ${test_carla_server_SRC})

add_executable(test_carla_server ${test_carla_server_SRC})
target_link_libraries(test_carla_server ${CarlaServer_LIBRARIES})

add_test(carla_server test_carla_server)
