file(GLOB carlaserver_SRC
    "*.h"
    "*.cpp"
    "*.pb.h"
    "*.pb.cc"
)
set(carlaserver_SRC ${carlaserver_SRC})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_library(carlaserverd STATIC ${carlaserver_SRC})
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_library(carlaserver STATIC ${carlaserver_SRC})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
