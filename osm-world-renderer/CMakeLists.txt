cmake_minimum_required(VERSION 3.5.1)
project(OsmMapRenderer)

# Libosmscout Library
#add_library(libosmscout SHARED PUBLIC)
#set_target_properties(libosmscout PROPERTIES
#    IMPORTED_LOCATION "/home/aollero/Downloads/libosmcout/libosmscout-master/installation/lib"
#    INTERFACE_INCLUDE_DIRECTORIES "/home/aollero/Downloads/libosmcout/libosmscout-master/installation/include")
include_directories(/home/aollero/Downloads/libosmcout/libosmscout-master/installation/include)
link_directories(/home/aollero/Downloads/libosmcout/libosmscout-master/installation/lib)

# Luna SVG Library
include_directories(/home/aollero/Downloads/lunasvg-master/installation/include)
link_directories(/home/aollero/Downloads/lunasvg-master/installation/lib)

# Source Files
add_library(OsmRenderer OsmRenderer/src/OsmRenderer.cpp)
target_sources(OsmRenderer PRIVATE OsmRenderer/src/MapDrawer.cpp)
target_sources(OsmRenderer PRIVATE OsmRenderer/src/MapRasterizer.cpp)
target_include_directories( OsmRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/OsmRenderer/include)

add_executable(osm-world-renderer OsmRenderer/main.cpp)
target_link_libraries(osm-world-renderer OsmRenderer)
target_link_libraries(osm-world-renderer osmscout)
target_link_libraries(osm-world-renderer osmscout_map)
target_link_libraries(osm-world-renderer osmscout_map_svg)
target_link_libraries(osm-world-renderer lunasvg)