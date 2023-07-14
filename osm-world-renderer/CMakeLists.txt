cmake_minimum_required(VERSION 3.5.1)
project(OsmMapRenderer)

# Libosmscout and Luna SVG Library
include_directories(ThirdParties/include)
include_directories(ThirdParties/include/lunasvg)
link_directories(ThirdParties/lib)

add_definitions(-D_USE_MATH_DEFINES)

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