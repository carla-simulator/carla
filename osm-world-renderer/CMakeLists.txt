project (
  osm-map-renderer
  LANGUAGES 
    CXX
  VERSION
    ${CARLA_VERSION}
)

set (
  OSM_WORLD_RENDERER_PATH
  ${CARLA_WORKSPACE_PATH}/osm-world-renderer/OsmRenderer
)

set (
  OSM_WORLD_RENDERER_INCLUDE_PATH
  ${OSM_WORLD_RENDERER_PATH}/include
)

set (
  OSM_WORLD_RENDERER_SOURCE_PATH
  ${OSM_WORLD_RENDERER_PATH}/src
)

add_library (
  lib-osm-map-renderer
  ${OSM_WORLD_RENDERER_INCLUDE_PATH}/MapDrawer.h
  ${OSM_WORLD_RENDERER_INCLUDE_PATH}/MapRasterizer.h
  ${OSM_WORLD_RENDERER_INCLUDE_PATH}/OsmRenderer.h
  ${OSM_WORLD_RENDERER_INCLUDE_PATH}/OsmRendererMacros.h
  ${OSM_WORLD_RENDERER_SOURCE_PATH}/OsmRenderer.cpp
  ${OSM_WORLD_RENDERER_SOURCE_PATH}/MapDrawer.cpp
  ${OSM_WORLD_RENDERER_SOURCE_PATH}/MapRasterizer.cpp
)

target_link_libraries (
  lib-osm-map-renderer PUBLIC
  carla-server
  OSMScout
  OSMScout::Map
  OSMScout::MapSVG
  lunasvg
)

target_include_directories (
  lib-osm-map-renderer PUBLIC
  ${OSM_WORLD_RENDERER_INCLUDE_PATH}
)

add_executable (
  osm-world-renderer
  ${OSM_WORLD_RENDERER_PATH}/main.cpp
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
