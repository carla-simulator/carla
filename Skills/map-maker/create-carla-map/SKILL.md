---
name: create-carla-map
description: Creates, compiles, inspects, and installs standalone, editor-free CARLA map bundles (.carlamap.tar.gz) completely offline using the carla-map CLI without Unreal Engine source or Editor access. Supports OpenStreetMap (.osm) conversion to OpenDRIVE (.xodr), 3D building extrusion with ear-clipping triangulation, custom 3D mesh import (.obj, .gltf, .glb) with semantic tagging and automatic Z-fighting elimination, offline Recast pedestrian NavMesh compilation, Traffic Manager cache cooking, and installation into packaged CARLA servers. Use when the user asks to "create a map without unreal editor", "import custom FBX/OBJ roads", "convert OSM to CARLA", "build a carlamap bundle", or "install a custom map into packaged CARLA".
license: MIT
compatibility: macOS (Apple Silicon arm64) and Linux (x86_64, aarch64). Works on both source checkouts and packaged standalone CARLA simulators (e.g. Carla-0.10.0-Mac-Shipping) with zero Unreal Editor requirements.
metadata:
  group: map-maker
---

# Create & Package Standalone CARLA Maps (Zero Unreal Editor)

This skill provides the complete end-to-end procedure for building, inspecting, verifying, and installing standalone `.carlamap.tar.gz` map packages using CARLA's offline **`carla-map`** toolsuite.

---

## 1. Quick Start Workflows

### Option A: OpenStreetMap (.osm) $\rightarrow$ Full City Simulation
Given only an OSM file (e.g., exported from openstreetmap.org):

```bash
# 1. Initialize project
./carla-map init MyCity
cd MyCity

# 2. Place OSM file
cp /path/to/city.osm osm/city.osm

# 3. Build standalone map bundle (offline)
./carla-map build --osm osm/city.osm --name MyCity

# 4. Verify bundle structure and checksums
./carla-map verify MyCity.carlamap.tar.gz

# 5. Install into packaged CARLA server
./carla-map install MyCity.carlamap.tar.gz --server /path/to/Carla-0.10.0-Mac-Shipping
```

### Option B: Custom 3D Road / Sidewalk / Building Meshes (.obj / .gltf) + OpenDRIVE (.xodr)
When supplying custom-modeled road geometry:

```bash
# 1. Initialize project
./carla-map init CustomTown
cd CustomTown

# 2. Place OpenDRIVE and 3D meshes
cp /path/to/town.xodr opendrive/town.xodr
cp /path/to/town.obj meshes/town.obj

# 3. Build bundle (automatically sets generate_road_mesh: false to eliminate Z-fighting)
./carla-map build --xodr opendrive/town.xodr --mesh meshes/town.obj --name CustomTown

# 4. Inspect bundle metadata
./carla-map inspect CustomTown.carlamap.tar.gz
```

---

## 2. Command Reference

| Subcommand | Description |
|---|---|
| `carla-map init <MapName>` | Scaffolds directory layout and default `map_recipe.json`. |
| `carla-map build [--recipe <file>] [--osm <file>] [--xodr <file>] [--mesh <file>]` | Compiles OpenDRIVE, geometry layers (.cmesh), Recast NavMesh (.bin), TM cache (.bin), 3D building placements, terrain, clutter, street lighting, traffic signals, and crosswalks into `<MapName>.carlamap.tar.gz`. |
| `carla-map inspect <bundle.carlamap.tar.gz>` | Displays map metadata, road counts, 3D geometry layers, clutter, street lamps, traffic lights, and crosswalk metrics. |
| `carla-map verify <bundle.carlamap.tar.gz>` | Runs 19 structural checks verifying JSON schema, OpenDRIVE XML, `.cmesh` magic, NavMesh, TM headers, clutter, lighting, signals, and crosswalks. |
| `carla-map install <bundle> --server <dir>` | Deploys bundle to `<Server>/Mac/CarlaUnreal-Mac-Shipping.app/Contents/UE/CarlaUnreal/Maps/<MapName>` or `<Server>/Linux/CarlaUnreal/Maps/<MapName>`. |

---

## 3. Loading the Map in Python

Once installed or packaged, load the map via standard Python API:

```python
import carla

client = carla.Client('127.0.0.1', 2000)
client.set_timeout(30.0)

# Method 1: Load installed map by name
world = client.load_world('MyCity')

# Method 2: Load remote bundle archive directly over RPC
world = client.load_map_bundle('/path/to/MyCity.carlamap.tar.gz')

print(f"Loaded {world.get_map().name} successfully!")
```

---

## 4. Key Gotchas & Best Practices

1. **Z-Fighting Avoidance:** When custom road meshes are supplied, ensure `"generate_road_mesh": false` is set (automated by `carla-map build`).
2. **Sub-Mesh Naming:** Name 3D subparts with semantic keywords like `*Road*`, `*Sidewalk*`, `*Building*`, `*Vegetation*`, `*Wall*` so the parser tags ground-truth semantics automatically.
3. **Coordinate Systems:** In OpenStreetMap builds, projection alignment uses `carla.Map.geolocation_to_transform()` with centroid offset adjustment to ensure building facades align with road edges down to millimeter precision.

