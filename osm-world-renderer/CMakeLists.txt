project (
  osm-map-renderer
  LANGUAGES CXX
  VERSION ${CARLA_VERSION}
)

set (
  OSM_RENDERER_PATH
  ${CARLA_WORKSPACE_PATH}/osm-world-renderer
)

add_library (
    lib-osm-map-renderer
    ${OSM_RENDERER_PATH}/OsmRenderer/src/OsmRenderer.cpp
    ${OSM_RENDERER_PATH}/OsmRenderer/src/MapDrawer.cpp
    ${OSM_RENDERER_PATH}/OsmRenderer/src/MapRasterizer.cpp
)

target_include_directories (
    lib-osm-map-renderer PUBLIC
    ${OSM_RENDERER_PATH}/OsmRenderer/include
)

add_executable (
    osm-world-renderer
    ${OSM_RENDERER_PATH}/OsmRenderer/main.cpp
)

target_link_libraries (
  osm-world-renderer PRIVATE
  lib-osm-map-renderer
  osmscout
  osmscout_map
  osmscout_map_svg
  lunasvg
)

target_compile_definitions (
    osm-world-renderer PRIVATE
    _USE_MATH_DEFINES
    ${CARLA_COMMON_DEFINITIONS}
)
