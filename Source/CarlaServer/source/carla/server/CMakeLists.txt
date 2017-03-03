# add_library(carla_server
#     CarlaServer.h
#     CarlaServer.cpp
#     TCPServer.h
#     TCPServer.cpp
# )
file(GLOB carla_server_SRC
    "*.h"
    "*.cpp"
)

add_library(carla_server ${carla_server_SRC})
