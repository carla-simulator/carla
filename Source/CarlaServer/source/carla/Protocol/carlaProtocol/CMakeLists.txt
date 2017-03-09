# add_library(carla_server
#     carla_protocol.pb.h
#     carla_protocol.pb.cc
# )
file(GLOB carla_server_SRC
    "*.h"
    "*.cpp"
)

add_library(carla_server ${carla_server_SRC})
