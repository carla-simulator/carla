file(GLOB carla_server_SRC
    "*.h"
    "*.cpp"
    "*.pb.h"
    "*.pb.cc"
)

add_library(carla_server ${carla_server_SRC})
