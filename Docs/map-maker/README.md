# CARLA Editor-Free Map Maker (`Docs/map-maker`)

This directory documents CARLA 0.10.0's **Editor-Free, Cross-Platform Map Creation & Packaging** pipeline.

The goal of Map Maker is to allow autonomous driving developers to build, customize, package, and load rich 3D simulation maps on **both Linux and macOS (Apple Silicon)** using a shipped CARLA package and Python API—**without** requiring access to the private Unreal Engine 5.8 source repository or opening the Unreal Editor.

> [!TIP]
> **Hands-on End-to-End Showcase:** Check out the complete [Munich, Germany Step-by-Step Tutorial](tutorial_munich_osm_e2e.md) showing real-world OpenStreetMap fetching, headless JOSM rendering, zero-editor map compilation, Traffic Manager traffic simulation, and manual driving.

---

## Feature & Milestone Overview

| Milestone | Status | Summary |
| :--- | :---: | :--- |
| **M1: `OSM2ODR` Build & Python Wheel Integration** | **Completed** | Native OpenStreetMap (`.osm`) $\rightarrow$ OpenDRIVE (`.xodr`) conversion statically bundled into the `carla` Python wheel (`carla.Osm2Odr`, `carla.Osm2OdrSettings`) and Unreal plugins (`WITH_OSM2ODR`). |
| **M2: Enhanced Runtime Mesh, Material & Semantic Engine** | **Completed** | Extended custom mesh spawning (`spawn_custom_mesh_ex` / batch API) with transforms, normals, UVs, vertex colors, `CityObjectLabel` ground-truth semantic segmentation tags, `EnvironmentObject` registration, 35+ pre-cooked material presets, and hybrid road mode (`generate_road_mesh` toggle). |
| **M3: Server-Side `.carlamap` Bundle Discovery & Loader** | **Completed** | Native discovery of `<ProjectDir>/Maps/<MapName>/carla-map.json` in `client.get_available_maps()`, seamless loading via `client.load_world("<MapName>")`, pre-compiled NavMesh staging, and live streaming via `client.load_map_bundle()`. |
| **M4: Offline `carla-map` CLI & OSM Building Generator** | **Completed** | Standalone `carla-map` CLI (`init`, `build`, `install`, `inspect`, `verify`) compiling `.obj`/`.gltf` meshes, `.osm`/`.xodr` road networks, 2.5D procedural OSM buildings, offline Recast pedestrian navmeshes (`Nav/*.bin`), and Traffic Manager caches (`TM/*.bin`) into portable `.carlamap.tar.gz` archives. |
| **M5: Shipped Package Integration & E2E Scenarios** | **Completed** | Standalone simulator package completeness (`CarlaUnreal.sh`, `carla-map`, `RecastBuilder`, wheel), Scenario A (OSM 3D city), Scenario B (custom 3D road replacement with zero Z-fighting), and complete E2E regression verification across M1–M5. |
| **M6: Urban Terrain, Landuse Ground Meshes & Sub-Base Floor** | **Completed** | OSM landuse polygon triangulation (`grass`, `park`, `water`, `plaza`), road clipping, continuous sub-base ground plane with foundation skirts, and 0 void abysses. |
| **M7: Vegetation, Roadside Furniture & Curbside Parked Cars** | **Completed** | OSM tree/furniture parsing + procedural sidewalk synthesis; static curbside parked cars (`CityObjectLabel.Vehicles`) with realistic collision hulls and 0 road obstruction. |
| **M8: Street Lighting, Dynamic Lumen Emitters & Day/Night** | **Completed** | Physical 3D street lamp poles (`SM_StreetLight*`); photometric lux engine complying with European EN 13201 ($E_{\text{avg}} \ge 10\text{ lx}$, $E_{\text{min}} \ge 2.5\text{ lx}$); 4-phase day/sunset/night/wet simulation. |
| **M9: Dynamic Traffic Lights & OpenDRIVE Signal Integration** | **Completed** | OpenDRIVE `<signal>` XML generation; dynamic 3-phase `ATrafficLightGroup` actors; Traffic Manager stop-line compliance; Ego perception API (`vehicle.get_traffic_light()`). |
| **M10: Photorealistic PBR Materials, Multi-Slot Shading & UE5 Matching** | **Completed** | Multi-slot building meshes (facade vs roof); tag-driven architectural PBR materials (`brick`, `plaster`, `glass`, `stone`, `concrete`); modular per-story floor UV mapping; multi-texture roads; matching [`Docs/img/carla_ue5_readme_img.webp`](../img/carla_ue5_readme_img.webp). |
| **M11: Pedestrian Crosswalks, Zebra Markings & Tactile Paving** | **Completed** | OSM `highway=crossing` extraction, zebra stripe mesh generator, yellow tactile paving blister tiles, pedestrian stop lines, and CityObjectLabel registration. |
| **M12: High-Resolution Satellite & Aerial Ground Infill** | **Completed** | GeoTIFF / XYZ ortho satellite imagery fetcher and projective texture mapping across ground sub-base meshes for photo-accurate real-world backdrops. |
| **M13: Protected Cycleways, Green Lanes & Bike Infrastructure** | **Completed** | Ingests `cycleway=*` and `highway=cycleway`; extruded colored lane beds (Munich red / green), dashed white demarcations, bike stencils, advance bike boxes, and bicycle racks. |
| **M14: Continuous Barriers, Perimeter Walls, Fences & Guard Rails** | **Completed** | Watertight 3D extruded polyline mesh generator with mitered corners and endcaps for OSM `barrier=*` (walls, fences, guardrails, hedges); collision hulls and Recast NavMesh obstacle registration. |

> [!NOTE]
> For the comprehensive technical architecture, visual feedback mechanisms, and Definition of Done (DoD) criteria for Milestones 6–14, see the [Master Implementation Plan](PLAN.md).


---

## Milestone 1: OpenStreetMap to OpenDRIVE (`OSM2ODR`)

### 1. Architecture & Build System Integration

CARLA's OpenStreetMap-to-OpenDRIVE converter (`libosm2odr`) is built from CARLA's fork of SUMO (`netconvert` / `osm2odr`) and depends on **PROJ 9.7.0**, **Xerces-C 3.3.0**, and **SQLite 3**.

- **Enabled by Default:** `ENABLE_OSM2ODR` is `ON` by default in `CMake/Options.cmake`.
- **Zero External Runtime Dependencies:**
  - `CMake/Dependencies.cmake` builds `libosm2odr.a`, `libproj.a`, `libxerces-c.a`, and `libsqlite3.a` as position-independent static archives (`-fPIC`).
  - `PROJ 9.7.0` is built with `EMBED_RESOURCE_FILES=ON` and `USE_ONLY_EMBEDDED_RESOURCE_FILES=ON`, embedding `proj.db` directly into `libproj.a` so no external `PROJ_DATA` directory is required at runtime on user machines.
  - `Xerces-C 3.3.0` is built with `network=OFF`, `transcoder="iconv"`, and `message-loader="inmemory"`, avoiding any external `libcurl` or `ICU` dynamic library dependencies.
- **Targets Linked:**
  1. **Python Wheel (`carla` module):** Linked into `carla.cpython-*.so` with `CARLA_PYTHON_API_HAS_OSM2ODR` defined.
  2. **Unreal Plugins (`Carla` & `CarlaTools`):** Linked via `Unreal/CMakeLists.txt` (`Includes.def`, `Libraries.def`, `Options.def`) with `WITH_OSM2ODR` defined for `UCustomFileDownloader::ConvertOSMInOpenDrive`.

#### Building from Source

```bash
# Configure CMake (ENABLE_OSM2ODR=ON is enabled by default)
cmake -B Build -DENABLE_OSM2ODR=ON

# Build the osm2odr static archive and its dependencies
cmake --build Build --target osm2odr

# Build and install the CARLA Python API wheel into the active Python environment
cmake --build Build --target carla-python-api-install
```

---

### 2. Python API Reference: `carla.Osm2Odr` & `carla.Osm2OdrSettings`

#### `carla.Osm2OdrSettings`

Configures how OpenStreetMap ways, lanes, junctions, sidewalks, and coordinate projections are translated into OpenDRIVE 1.4 XML.

| Property / Method | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `use_offsets` | `bool` | `False` | Whether to apply custom `(offset_x, offset_y)` translation to all projected coordinates. |
| `offset_x` | `float` | `0.0` | Custom X offset in meters (used when `use_offsets=True`). |
| `offset_y` | `float` | `0.0` | Custom Y offset in meters (used when `use_offsets=True`). |
| `default_lane_width` | `float` | `3.35` | Default driving lane width in meters when an OSM way does not specify a `width` tag. |
| `default_sidewalk_width` | `float` | `2.8` | Width in meters of generated sidewalks (`sidewalk` lane type). Set `> 0.0` to generate sidewalks along roads. |
| `elevation_layer_height` | `float` | `0.0` | Vertical separation in meters per OSM `layer` tag (`layer=1`, `layer=-1`, etc.) for bridges/overpasses. |
| `proj_string` | `str` | `"+proj=tmerc"` | PROJ coordinate projection string used to project WGS84 `(lat, lon)` into local Cartesian `(x, y)` meters. |
| `center_map` | `bool` | `True` | Automatically center the projected road network around `(0, 0)`. |
| `generate_traffic_lights` | `bool` | `False` | Generate OpenDRIVE `<signal>` definitions from OSM `highway=traffic_signals` nodes and controlled junctions. |
| `all_junctions_with_traffic_lights` | `bool` | `False` | Force all junctions to be signalized with traffic lights. |
| `set_osm_way_types(way_types)` | `list[str]` | *(all)* | Whitelist of OSM `highway=*` values to import as drivable roads (e.g. `["motorway", "primary", "secondary", "tertiary", "residential"]`). |
| `set_traffic_light_excluded_way_types(way_types)` | `list[str]` | `[]` | List of OSM `highway=*` values that should never receive generated traffic lights. |

#### `carla.Osm2Odr.convert(osm_file, settings=carla.Osm2OdrSettings())`

- **Parameters:**
  - `osm_file` (`str`): Raw UTF-8 XML content of the `.osm` file (not a file path).
  - `settings` (`carla.Osm2OdrSettings`, optional): Conversion settings instance.
- **Returns:** `str` — Complete OpenDRIVE (`.xodr`) XML string.

---

### 3. Usage Guide

#### Workflow A: Offline `.osm` $\rightarrow$ `.xodr` Conversion & Map Inspection (No Server Required)

You can convert an OpenStreetMap `.osm` file into an OpenDRIVE `.xodr` file and inspect its road topology, waypoints, landmarks, and Traffic Manager cache completely offline using only the `carla` Python package:

```python
from pathlib import Path
import carla

# 1. Read the input .osm XML file
osm_xml = Path("e2e_tests/m1_osm2odr/fixtures/sample_network.osm").read_text(encoding="utf-8")

# 2. Configure Osm2OdrSettings
settings = carla.Osm2OdrSettings()
settings.set_osm_way_types([
    "motorway", "motorway_link",
    "trunk", "trunk_link",
    "primary", "primary_link",
    "secondary", "secondary_link",
    "tertiary", "tertiary_link",
    "unclassified", "residential",
])
settings.default_lane_width = 3.5
settings.default_sidewalk_width = 3.0
settings.elevation_layer_height = 4.5
settings.generate_traffic_lights = True
settings.center_map = True

# 3. Convert OSM XML -> OpenDRIVE (.xodr) XML
xodr_xml = carla.Osm2Odr.convert(osm_xml, settings)
Path("sample_network.xodr").write_text(xodr_xml, encoding="utf-8")

# 4. Parse and validate the OpenDRIVE map offline with carla.Map
offline_map = carla.Map("SampleOSMMap", xodr_xml)
waypoints = offline_map.generate_waypoints(2.0)
topology = offline_map.get_topology()
landmarks = offline_map.get_all_landmarks()

print(f"Waypoints (2.0m spacing): {len(waypoints)}")
print(f"Topology segments:        {len(topology)}")
print(f"Traffic landmarks/signs:  {len(landmarks)}")
```

---

## Milestone 2: Enhanced Runtime Custom Mesh, Material & Semantic Tagging Engine

Milestone 2 upgrades CARLA's runtime mesh engine so custom 3D geometry (roads, sidewalks, buildings, walls, vegetation, bridges, and imported `.fbx`/`.obj`/`.gltf` meshes) can be spawned at runtime with full **Chaos physics collision**, **ground-truth semantic/instance segmentation tags**, **`EnvironmentObject` & 3D bounding box registration**, and **30+ pre-cooked PBR materials**—all without the Unreal Editor.

### 1. Python API Reference

#### `carla.CustomMaterialParameters`

Configures dynamic PBR overrides or runtime texture maps when spawning custom meshes.

| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `base_color` | `carla.Color` | `Color(200, 200, 200, 255)` | Base albedo tint color `(r, g, b)`. |
| `roughness` | `float` | `0.8` | Surface roughness in `[0.0, 1.0]`. |
| `metallic` | `float` | `0.0` | Metallic factor in `[0.0, 1.0]`. |
| `specular` | `float` | `0.5` | Specular reflectance in `[0.0, 1.0]`. |
| `uv_scale` | `carla.Vector2D` | `Vector2D(1.0, 1.0)` | UV tiling multiplier `(u_scale, v_scale)`. |
| `albedo_texture_path` | `str` | `""` | Optional local `.png`/`.jpg` path for runtime base color texture. |
| `normal_texture_path` | `str` | `""` | Optional local `.png`/`.jpg` path for runtime normal map texture. |
| `roughness_texture_path` | `str` | `""` | Optional local `.png`/`.jpg` path for runtime roughness texture. |

#### `carla.CustomMeshDescription`

Describes a single 3D mesh section to spawn in the CARLA world.

| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `name` | `str` | `"CustomMesh"` | Human-readable object name registered in `world.get_environment_objects()`. |
| `transform` | `carla.Transform` | `Transform()` | World transform (location in meters, rotation in degrees). |
| `scale` | `carla.Vector3D` | `Vector3D(1, 1, 1)` | Per-axis scale applied to local vertex positions. |
| `vertices` | `list[carla.Vector3D]` | `[]` | Vertex positions in meters (accepts `Vector3D`, `Location`, `(x, y, z)` tuples, or a flat `float` list). |
| `triangles` | `list[int]` | `[]` | Triangle vertex indices (accepts a flat `int` list of length $3N$ or a list of `(i0, i1, i2)` tuples). |
| `normals` | `list[carla.Vector3D]` | `[]` | Optional per-vertex normals. Auto-computed (area-weighted) when omitted. |
| `uvs` | `list[carla.Vector2D]` | `[]` | Optional per-vertex UV coordinates. Auto-computed via triplanar world projection (5m tile) when omitted. |
| `vertex_colors` | `list[carla.Color]` | `[]` | Optional per-vertex colors. |
| `semantic_label` | `carla.CityObjectLabel` | `CityObjectLabel.Buildings` | Ground-truth semantic tag for `sensor.camera.semantic_segmentation`, `sensor.camera.instance_segmentation`, `sensor.lidar.ray_cast_semantic`, `world.cast_ray()`, `world.get_level_bbs()`, and `world.get_environment_objects()`. |
| `enable_collision` | `bool` | `True` | Enables synchronous Chaos `BlockAll` complex-as-simple triangle collision for vehicles, walkers, and raycasts. |
| `material_preset` | `str` | `"building_concrete"` | Pre-cooked material preset name (see catalog below), `"custom_pbr"`, or a direct `/Game/...` Unreal asset path. |
| `material_params` | `carla.CustomMaterialParameters` | `CustomMaterialParameters()` | Optional PBR scalar/color/UV/texture overrides. |

#### World Methods

- **`world.spawn_custom_mesh_ex(description: carla.CustomMeshDescription) -> int`**
  Spawns a single custom mesh and returns its unique 64-bit `EnvironmentObject.id`.
- **`world.spawn_custom_meshes(descriptions: list[carla.CustomMeshDescription]) -> list[int]`**
  Spawns a batch of custom meshes in a single RPC round-trip and returns their `EnvironmentObject.id` list in matching order.
- **`world.spawn_custom_mesh(vertices: list[float], triangles: list[int], material: str = "grass") -> None`**
  Legacy M0/M1 helper preserved for backward compatibility.

#### Pre-Cooked Material Preset Catalog (34 Presets)

All presets below are cooked into the CARLA package and can be passed directly as `desc.material_preset`:

| Category | Preset Names |
| :--- | :--- |
| **Roads & Markings** | `road`, `road_asphalt`, `road_worn`, `road_wet`, `road_rural`, `crosswalk`, `road_line_white`, `road_line_yellow` |
| **Sidewalks & Curbs** | `sidewalk`, `sidewalk_concrete`, `sidewalk_tiles`, `sidewalk_residential`, `curb`, `gutter` |
| **Terrain & Water** | `grass`, `grass_yard`, `dirt`, `gravel`, `rock`, `water` |
| **Buildings & Roofs** | `building_concrete`, `building_brick`, `building_plaster`, `building_stone`, `building_glass`, `roof_tiles`, `roof_metal`, `roof_bitumen` |
| **Walls, Fences & Bridges** | `wall_concrete`, `wall_brick`, `fence_wood`, `fence_metal`, `guardrail`, `bridge`, `bridge_steel`, `custom_pbr` |

#### Hybrid OpenDRIVE Generation (`carla.OpendriveGenerationParameters`)

`carla.OpendriveGenerationParameters` now includes four boolean flags that allow decoupling the **logical OpenDRIVE road network** (waypoints, topology, junctions, Traffic Manager routing, and traffic signals) from **visual/collision mesh generation**:

| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `generate_road_mesh` | `bool` | `True` | Set to `False` when providing custom 3D road/sidewalk meshes via `spawn_custom_meshes()` so procedural OpenDRIVE road meshes do not z-fight with custom road geometry. |
| `generate_ground_mesh` | `bool` | `True` | Set to `False` when providing custom terrain/ground meshes instead of the procedural B-spline terrain heightfield. |
| `generate_street_furniture` | `bool` | `True` | Controls whether procedural street furniture anchors / PCG scatter run along OpenDRIVE roads. |
| `generate_traffic_signals` | `bool` | `True` | Controls whether OpenDRIVE `<signal>` traffic lights and signs are automatically spawned. |

---

### 2. Usage Guide: Hybrid OpenDRIVE + Custom 3D Road & Building Meshes

```python
import carla

client = carla.Client("127.0.0.1", 2000)
client.set_timeout(30.0)

# 1. Load logical OpenDRIVE road network WITHOUT procedural road/ground meshes
od_params = carla.OpendriveGenerationParameters(
    vertex_distance=2.0,
    max_road_length=50.0,
    wall_height=0.0,
    additional_width=0.0,
    smooth_junctions=True,
    enable_mesh_visibility=True,
    enable_pedestrian_navigation=False,
    generate_road_mesh=False,      # Custom road mesh will be spawned below
    generate_ground_mesh=False,    # Custom ground/building meshes will be spawned below
    generate_street_furniture=False,
    generate_traffic_signals=True,
)
world = client.generate_opendrive_world(xodr_xml, od_params)

# 2. Spawn custom road mesh + custom building mesh in a single batch RPC
road = carla.CustomMeshDescription()
road.name = "CustomRoad_Main"
road.vertices = [
    carla.Vector3D(0.0, -10.0, 0.0),
    carla.Vector3D(0.0, 10.0, 0.0),
    carla.Vector3D(100.0, 10.0, 0.0),
    carla.Vector3D(100.0, -10.0, 0.0),
]
road.triangles = [0, 1, 2, 0, 2, 3]
road.semantic_label = carla.CityObjectLabel.Roads
road.enable_collision = True
road.material_preset = "road_asphalt"

road_id = world.spawn_custom_mesh_ex(road)

# 3. Query & toggle spawned custom meshes via CARLA's EnvironmentObject API
roads = world.get_environment_objects(carla.CityObjectLabel.Roads)
road_bbs = world.get_level_bbs(carla.CityObjectLabel.Roads)
world.enable_environment_objects([road_id], True)
```

---

## Milestone 3: Server-Side `.carlamap` Bundle Discovery & Runtime Loader

Milestone 3 implements the server-side runtime discovery, staging, instantiation, and live streaming of editor-free `.carlamap` bundles.

A `.carlamap` bundle is a portable directory structure (or `.tar.gz` archive) containing:
1. `carla-map.json` — Bundle metadata, layer declarations, material presets, hybrid OpenDRIVE parameters, and HISM asset placements.
2. `OpenDrive/<MapName>.xodr` — Logical OpenDRIVE driving network.
3. `Nav/<MapName>.bin` — Pre-compiled Recast pedestrian navigation mesh.
4. `Meshes/*.cmesh` — Binary packed procedural mesh layers (roads, sidewalks, buildings, terrain) with physics collision and semantic labels.
5. `Placements/` — Instanced static mesh placements referencing internal CARLA asset blueprints.

---

### 1. Bundle Directory Structure & `carla-map.json` Specification

```text
MyTown.carlamap/
├── carla-map.json
├── OpenDrive/
│   └── MyTown.xodr
├── Nav/
│   └── MyTown.bin
├── Meshes/
│   ├── roads.cmesh
│   ├── sidewalks.cmesh
│   └── buildings.cmesh
└── Placements/
    └── street_props.json
```

#### `carla-map.json` Schema

```json
{
  "format_version": "1.0",
  "map_name": "MyTown",
  "xodr_file": "OpenDrive/MyTown.xodr",
  "nav_file": "Nav/MyTown.bin",
  "opendrive_parameters": {
    "generate_road_mesh": false,
    "generate_ground_mesh": false,
    "generate_traffic_signals": true,
    "generate_street_furniture": false
  },
  "layers": [
    {
      "name": "CustomRoads",
      "mesh_file": "Meshes/roads.cmesh",
      "semantic_label": "Roads",
      "material_preset": "road_asphalt",
      "enable_collision": true
    },
    {
      "name": "CustomSidewalks",
      "mesh_file": "Meshes/sidewalks.cmesh",
      "semantic_label": "Sidewalks",
      "material_preset": "sidewalk_concrete",
      "enable_collision": true
    },
    {
      "name": "CustomBuildings",
      "mesh_file": "Meshes/buildings.cmesh",
      "semantic_label": "Buildings",
      "material_preset": "building_brick",
      "enable_collision": true
    }
  ],
  "placements": [
    {
      "asset_path": "/Game/Carla/Static/Buildings/Building_01.Building_01",
      "transforms": [
        {
          "location": [50.0, 50.0, 0.0],
          "rotation": [0.0, 0.0, 0.0],
          "scale": [1.0, 1.0, 1.0]
        }
      ]
    }
  ],
  "spawn_points": [
    {
      "location": [0.0, 0.0, 0.5],
      "rotation": [0.0, 0.0, 0.0]
    }
  ]
}
```

---

### 2. Binary `.cmesh` Format Specification

To ensure fast streaming and minimal disk overhead across platforms, geometry layers are serialized into packed binary `.cmesh` files (`CMES` magic `0x434D4553`, little-endian):

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32` | `Magic` | `0x434D4553` (`'CMES'`). |
| `0x04` | `uint32` | `Version` | File format version (`1`). |
| `0x08` | `uint32` | `Flags` | Bit 0: Has Normals (`1`), Bit 1: Has UV0 (`2`), Bit 2: Has Colors (`4`). |
| `0x0C` | `uint32` | `VertexCount` | Number of 3D vertices ($N_v$). |
| `0x10` | `uint32` | `TriangleIndexCount`| Number of triangle vertex indices ($N_t$, multiple of 3). |
| `0x14` | `uint32` | `Reserved` | Reserved (padding). |
| Data | `float32[Nv * 3]` | Vertices | Flat `[x0, y0, z0, x1, y1, z1, ...]` in meters (CARLA coordinates: X right, Y forward, Z up). |
| Data | `int32[Nt]` | Triangles | 0-based triangle indices `[i0, i1, i2, ...]`. |
| Data | `float32[Nv * 3]` | Normals | *(Optional, if Flags & 1)* Vertex normal vectors. |
| Data | `float32[Nv * 2]` | UV0 | *(Optional, if Flags & 2)* Texture coordinates `[u0, v0, u1, v1, ...]`. |
| Data | `uint8[Nv * 4]` | Colors | *(Optional, if Flags & 4)* RGBA 8-bit vertex colors `[r0, g0, b0, a0, ...]`. |

---

### 3. Server Discovery Rules & Python API Usage

#### Discovery Paths

The CARLA server automatically scans the following locations for `.carlamap` directories (or directories containing `carla-map.json`):
1. `<CARLA_ROOT>/Unreal/CarlaUnreal/Maps/<MapName>/` (or `<PackageRoot>/Carla/Maps/<MapName>/`).
2. `<CARLA_ROOT>/Unreal/CarlaUnreal/Saved/Maps/<MapName>/`.
3. Directories specified via the server command-line argument `-carla-maps=<dir>[;<dir>]`.
4. Directories specified via the `CARLA_MAPS_PATH` environment variable.

#### Workflow A: Native Map Discovery & Seamless Loading

```python
import carla

client = carla.Client("127.0.0.1", 2000)
client.set_timeout(60.0)

# 1. Discover all native cooked maps AND custom .carlamap bundles
available_maps = client.get_available_maps()
print("Available maps:", available_maps)
assert "MyTown" in available_maps

# 2. Seamlessly load the bundle
# The server stages OpenDRIVE & NavMesh, opens OpenDriveMap, instantiates
# .cmesh layers with Chaos collision, and hooks walker navigation instantly.
world = client.load_world("MyTown")

# 3. Query map and spawn actors
carla_map = world.get_map()
print(f"Loaded map: {carla_map.name}")

# Pedestrian navigation uses the bundle's pre-compiled NavMesh
ped_loc = world.get_random_location_from_navigation()
walker_bp = world.get_blueprint_library().filter("walker.pedestrian.*")[0]
walker = world.spawn_actor(walker_bp, carla.Transform(ped_loc))
```

#### Workflow B: Live Remote Bundle Loading via RPC (`client.load_map_bundle`)

When running a client against a remote CARLA server where local filesystem staging is not feasible, client applications can stream a `.carlamap.tar.gz` bundle directly over the RPC connection:

```python
import carla

client = carla.Client("remote-server-ip", 2000)
client.set_timeout(120.0)

# Uploads the compressed tarball over RPC, unpacks into server's Saved/Maps/<MapName>,
# and loads the world seamlessly.
world = client.load_map_bundle("path/to/MyTown.carlamap.tar.gz", reset_settings=True)
print("Remote map bundle loaded successfully!")
```

---

## Milestone 4: Offline `carla-map` Builder CLI & Procedural Generators

Milestone 4 delivers the standalone `carla-map` CLI and Python SDK (`Util/MapBuilder/carla_map.py` and `Util/MapBuilder/carla-map`). It enables autonomous driving engineers to scaffold, import, extrude, build, inspect, verify, and install complete `.carlamap` bundles entirely offline—**with zero Unreal Editor or running CARLA server required**.

### 1. CLI Subcommands Reference

```text
usage: carla-map [-h] {init,build,inspect,verify,install} ...
```

#### `carla-map init <MapName>`
Scaffolds a clean map creation project directory with `map_recipe.json` and asset folders.
```bash
# Scaffold a new map project
carla-map init SunnyTown --osm input_network.osm
```

#### `carla-map build [<target>]`
Compiles all input OpenStreetMap/OpenDRIVE sources, custom 3D models (`.obj`, `.gltf`, `.glb`), extruded 3D buildings, pre-compiled Recast walker NavMesh (`Nav/*.bin`), and Traffic Manager cache (`TM/*.bin`) into a `.carlamap` folder and portable `.tar.gz` bundle:
```bash
# Build from project directory or recipe
carla-map build SunnyTown

# Or build directly from CLI flags without a project folder:
carla-map build --osm city.osm --name CityTown -o CityTown.carlamap.tar.gz

# Or build from OpenDRIVE and custom 3D road/building models:
carla-map build --xodr road.xodr --meshes meshes/ -n CustomTown
```

#### `carla-map inspect <bundle>`
Displays a human-readable diagnostic report of the bundle:
```bash
carla-map inspect SunnyTown.carlamap.tar.gz
```
Outputs:
- Total file and geometry size.
- OpenDRIVE details: number of road segments, junctions, waypoints, and traffic lights/signals.
- Geometry layers: layer name, semantic label, material preset, vertex count, triangle count, and bounding box.
- NavMesh and Traffic Manager cache status.

#### `carla-map verify <bundle>`
Deep structural, mathematical, and cryptographic verification of the bundle:
```bash
carla-map verify SunnyTown.carlamap.tar.gz
```
Validates:
1. Archive and JSON manifest syntax.
2. OpenDRIVE XML road graph validity with `carla.Map`.
3. Every binary `.cmesh` layer: checks magic `0x53454D43`, version 1, indices divisible by 3, and non-empty vertex buffers.
4. Non-empty pre-compiled NavMesh binary.

#### `carla-map install <bundle> --server <server_dir>`
Installs the bundle into a target CARLA simulator installation (Linux or macOS package, or source checkout):
```bash
carla-map install SunnyTown.carlamap.tar.gz --server /opt/carla-simulator
```

---

### 2. Multi-Format 3D Mesh Import & Z-Fighting Prevention

`carla-map` parses `.obj`, `.gltf`, and `.glb` files and splits sub-meshes into discrete `.cmesh` layers based on naming or material keywords:

| Name / Material Keyword | Semantic Label | Default Material Preset | Collision |
| :--- | :--- | :--- | :---: |
| `*road*`, `*street*`, `*highway*`, `*asphalt*` | `CityObjectLabel.Roads` | `road_asphalt` | Yes |
| `*sidewalk*`, `*walkway*`, `*curb*`, `*pavement*` | `CityObjectLabel.Sidewalks` | `sidewalk_concrete` | Yes |
| `*building*`, `*house*`, `*facade*` | `CityObjectLabel.Buildings` | `building_brick` | Yes |
| `*wall*`, `*fence*` | `CityObjectLabel.Walls` | `wall_concrete` | Yes |
| `*barrier*`, `*guardrail*` | `CityObjectLabel.GuardRail` | `metal` | Yes |
| `*pole*`, `*lamp*` | `CityObjectLabel.Poles` | `metal` | Yes |
| `*tree*`, `*foliage*`, `*veg*` | `CityObjectLabel.Vegetation` | `wood` / `grass` | Optional |
| `*terrain*`, `*ground*`, `*grass*`, `*dirt*` | `CityObjectLabel.Terrain` | `grass` / `dirt` | Yes |

> [!TIP]
> **Automatic Z-Fighting Prevention**: Whenever user road meshes are imported, `carla-map build` automatically sets `"generate_road_mesh": false` in `carla-map.json`. This ensures the custom road geometry serves as the sole visual and physical road surface, preventing Z-fighting flicker with procedural OpenDRIVE meshes.

---

### 3. OpenStreetMap 3D Building Extrusion Engine

When an `.osm` file is supplied, `carla-map` extracts all `building=*` footprints and automatically extrudes watertight 3D building geometry:
1. **Height Parsing**: Reads `height` (e.g. `14.5m`) or `building:levels` (e.g. `4` $\rightarrow$ 4 $\times$ 3.5m = 14m), falling back to configurable defaults.
2. **Sub-Millimeter Alignment**: Calculates the exact translation offset between the centered OpenDRIVE road waypoints and raw geographic PROJ projection coordinates, ensuring 3D building facades align with sidewalks to within $10^{-9}$ meters.
3. **Watertight Extrusion**:
   - Vertical walls extruded with outward-facing normals and continuous UV coordinates along the perimeter.
   - 2D polygon ear-clipping triangulator generates solid horizontal roofs for arbitrary non-convex building footprints.
   - Saves extruded buildings as `Meshes/osm_buildings.cmesh` tagged with `CityObjectLabel.Buildings`.

---

## Milestone 5: Shipped Package Integration & End-to-End Scenarios

Milestone 5 brings together the entire editor-free pipeline into the distributed standalone CARLA simulator package (`Carla-<version>-<platform>-Shipping`).

### 1. Shipped Package Layout & Completeness

When extracting a distributed CARLA simulator package, the root directory provides all necessary tools out-of-the-box:

```text
Carla-0.10.0-Mac-Shipping/
├── CarlaUnreal.sh                          # Headless or offscreen simulator launcher
├── carla-map                               # Standalone Map Maker executable CLI wrapper
├── Mac/
│   ├── CarlaUnreal-Mac-Shipping.app        # Pre-compiled, codesigned CARLA server
│   └── Tools/
│       └── RecastBuilder                   # Bundled Recast pedestrian navmesh builder
├── PythonAPI/
│   ├── carla/dist/carla-*.whl              # Self-contained Python wheel (with Osm2Odr)
│   ├── python_api.md                       # Comprehensive API reference
│   └── util/
│       └── carla_map.py                    # Map Maker entrypoint
└── Util/
    └── MapBuilder/                         # Serializers, parsers, and extrusion engine
```

### 2. Standalone End-to-End Workflow

Users can create and install maps directly without cloning or building CARLA source:

```bash
# 1. Run carla-map directly from package root
./carla-map init MyCity
./carla-map build --osm /path/to/city.osm --name MyCity

# 2. Verify bundle structure
./carla-map verify MyCity.carlamap.tar.gz

# 3. Install map into the simulator
./carla-map install MyCity.carlamap.tar.gz

# 4. Start simulator and load map
./CarlaUnreal.sh -RenderOffScreen &
python3 -c "import carla; client = carla.Client('127.0.0.1', 2000); client.load_world('MyCity')"
```

---

## Milestone 14: Continuous Barriers, Perimeter Walls, Fences & Guard Rails (`barrier=*`)

Milestone 14 enables the extraction, watertight 3D extrusion, and physical collision registration of urban boundary structures directly from OpenStreetMap data.

### 1. Architectural Scope & Semantic Classification

OpenStreetMap `barrier=*` ways and nodes are parsed and classified into dedicated semantic simulation layers:

| Layer Name | Semantic Label (`CityObjectLabel`) | Material Preset | Default Dimensions | Filter Criteria |
| :--- | :--- | :--- | :--- | :--- |
| `OSM_Barriers_Walls` | `CityObjectLabel.Walls` | `wall_concrete` / `wall_stone` | Height: 1.5m, Width: 0.35m | `barrier=wall`, `retaining_wall`, `city_wall` |
| `OSM_Barriers_Fences` | `CityObjectLabel.Fences` | `fence_metal` / `fence_wood` | Height: 1.2m, Width: 0.08m | `barrier=fence`, `railing`, `handrail` |
| `OSM_Barriers_GuardRails` | `CityObjectLabel.GuardRail` | `fence_metal` / `metal` | Height: 0.85m, Width: 0.15m | `barrier=guard_rail`, `crash_barrier`, `jersey_barrier` |
| `OSM_Barriers_Hedges` | `CityObjectLabel.Vegetation` | `hedge` / `foliage` | Height: 1.2m, Width: 0.60m | `barrier=hedge` |

- **Underground Structure Filtering:** Features marked with `level < 0`, `layer < 0`, `indoor=yes`, or `tunnel=yes` (such as subway station barriers) are automatically excluded from the above-ground visual and collision geometry.

### 2. Watertight 3D Polyline Extrusion Engine

The barrier mesh generator produces continuous, solid geometry:
1. **Vertical Wall Faces:** Front and back vertical side walls extruded from ground level ($z = 0$) up to the designated barrier height with outward-pointing normals.
2. **Top Capping Surface:** Continuous horizontal cap polygon spanning the upper boundary between left and right extrusion profiles.
3. **Closed Endcaps:** Solid vertical quad endcaps sealing the start and end of open polylines.
4. **Corner Mitering:** Mitering at sharp polyline intersections with scale factor clamped ($\le 2.5$) to prevent unnatural geometric spikes.
5. **Continuous Texture Coordinates:** UV unwrapping parameterized along the length and height of the barrier for seamless texture tiling.

### 3. Physics Collision & Recast NavMesh Obstacles

- All barrier meshes are baked into the CARLA environment with full physics collision bodies.
- Barrier footprints are passed to the bundled Recast NavMesh compiler as solid obstacles (`block`), ensuring pedestrians automatically navigate around perimeter walls, fences, and guardrails without walking through them.

### 4. CLI Tooling & Manifest

```bash
# Inspect barrier breakdown, lengths, and vertex counts for a map
python3.10 PythonAPI/util/inspect_barriers.py Docs/map-maker/output/MunichStachus.carlamap.tar.gz

# Verify bundle integrity (including Check 12: Barrier manifest & layer validation)
./carla-map verify Docs/map-maker/output/MunichStachus.carlamap.tar.gz

# Build map without barriers (optional flag)
./carla-map build --osm city.osm --no-barriers
```

---

## Running End-to-End Verification Tests

All end-to-end verification tests and their input fixtures live in [`e2e_tests/`](../../e2e_tests/README.md):

```bash
# Run all 14 milestone E2E tests (offline checks)
python3.10 e2e_tests/run_all.py --offline-only

# Run full live E2E test suite (spawns CARLA simulator if not already running)
python3.10 e2e_tests/run_all.py

# Run specific milestone suites
python3.10 e2e_tests/m11_crosswalks/test_m11_crosswalks.py
python3.10 e2e_tests/m12_satellite/test_m12_satellite.py
python3.10 e2e_tests/m13_cycleways/test_m13_cycleways.py
python3.10 e2e_tests/m14_barriers/test_m14_barriers.py
```


