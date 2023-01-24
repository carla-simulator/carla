add_library(OsmRenderer OsmRenderer/src/OsmRenderer.cpp)
target_include_directories( OsmRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/OsmRenderer/include)

add_executable(osm-world-renderer OsmRenderer/main.cpp)
target_link_libraries(osm-world-renderer OsmRenderer)