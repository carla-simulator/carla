project (
  osm-map-renderer
  LANGUAGES 
    CXX
  VERSION
    ${CARLA_VERSION}
)

set (
  OSM_RENDERER_SOURCE_PATH
  ${CARLA_WORKSPACE_PATH}/osm-world-renderer/OsmRenderer
)

carla_add_library (
  lib-osm-map-renderer
  "Build OSM-Map-Renderer."
  ${OSM_RENDERER_SOURCE_PATH}/src/OsmRenderer.cpp
  ${OSM_RENDERER_SOURCE_PATH}/src/MapDrawer.cpp
  ${OSM_RENDERER_SOURCE_PATH}/src/MapRasterizer.cpp
)

target_link_libraries (
  lib-osm-map-renderer PUBLIC
  OSMScout::OSMScout
  OSMScout::Map
  OSMScout2
  lunasvg
  carla-server
)

target_include_directories (
  lib-osm-map-renderer PUBLIC
  ${OSM_RENDERER_SOURCE_PATH}/include
)

carla_add_executable (
  osm-world-renderer
  "Build OSM-World-Renderer."
  ${OSM_RENDERER_SOURCE_PATH}/main.cpp
)

target_link_libraries (
  osm-world-renderer PRIVATE
  lib-osm-map-renderer
)

target_compile_definitions (
  osm-world-renderer PRIVATE
  _USE_MATH_DEFINES
  ${CARLA_COMMON_DEFINITIONS}
)
