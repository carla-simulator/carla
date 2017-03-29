file(GLOB carlaserver_SRC
    "*.h"
    "*.cpp"
    "*.pb.h"
    "*.pb.cc"
)
set(carlaserver_SRC ${carlaserver_SRC} ../CarlaServer.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_library(carlaserverd STATIC ${carlaserver_SRC})
elseif (CMAKE_BUILD_TYPE MATCHES Release)
  add_library(carlaserver STATIC ${carlaserver_SRC})
endif (CMAKE_BUILD_TYPE MATCHES Debug)
