file(GLOB carlaserver_SRC
    "*.h"
    "*.cpp"
    "*.pb.h"
    "*.pb.cc"
)
set(carlaserver_SRC ${carlaserver_SRC})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_library(carlaserverd SHARED ${carlaserver_SRC})
  # target_link_libraries(carlaserverd ${CarlaServer_LIBRARIES})
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_library(carlaserver SHARED ${carlaserver_SRC})
  # target_link_libraries(carlaserver ${CarlaServer_LIBRARIES})
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
