# CARLA → Autoware map tooling

Produces the map directory Autoware expects:

```
<map_path>/
  pointcloud_map.pcd        # >= 0.2 m resolution point cloud
  lanelet2_map.osm          # lanelet2 vector map (local_x/local_y metric coords)
  map_projector_info.yaml   # projector_type: Local
```

used as `ros2 launch autoware_launch e2e_simulator.launch.xml map_path:=<map_path> ...`.

## Two ways to get a map

### 1. Prebuilt (Town01–Town07, Town10HD) — no server needed

```bash
./fetch_prebuilt_maps.sh Town10HD_Opt        # -> <this dir>/maps/Town10HD/
./fetch_prebuilt_maps.sh Town03 /custom/dir  # explicit output dir
```

Downloads the UE4-era pair from `carla-simulator/autoware-contents`
(TIER IV verified the Town10 pair against UE5 Town10HD_Opt). Known warts:
no traffic-light regulatory elements, small pcd/osm origin offsets.

### 2. Generated from a running server — any town

```bash
# converter deps live in a venv (see requirements.txt)
python3 -m venv ~/.venvs/carla-map-tools
~/.venvs/carla-map-tools/bin/pip install -r requirements.txt \
    <path-to>/PythonAPI/carla/dist/carla-*.whl

~/.venvs/carla-map-tools/bin/python generate_map_artifacts.py \
    --town Town12 --out ./maps/Town12 --tick
```

The driver orchestrates:

| Tool | Purpose |
|---|---|
| `generate_pointcloud_map.py` | ground-truth lidar sweep over the waypoint graph → voxelized binary PCD |
| `generate_lanelet2_map.py` | OpenDRIVE → lanelet2 (crdesigner), + ground-truth traffic-light regulatory elements when live |
| `write_projector_info.py` | `map_projector_info.yaml` (Local; MGRS/UTM options for digital twins) |
| `fetch_prebuilt_maps.sh` | download + arrange the prebuilt pair |
| `pcd_io.py` | numpy-only PCD write/read + voxel downsample (no open3d) |

`generate_lanelet2_map.py --xodr file.xodr` also works fully offline
(skips traffic-light injection, with a warning).

## Rules of engagement

* Tools **never launch** a CARLA server; they connect to `--host/--port`.
* Sync-mode worlds are **never ticked** unless you pass `--tick`, which makes
  the tool the *single* ticking client. Without `--tick`, exactly one other
  client must tick or sensor capture times out with a clear message.
* Coordinate convention: the map artifacts are right-handed,
  **y-inverted relative to CARLA** (same convention as the prebuilt
  autoware-contents maps). OpenDRIVE-derived data passes through unflipped;
  live actor/sensor data is flipped (`y := -y`). See `carla_common.py`.

## Tests

```bash
python3 tests/test_pcd_io.py      # PCD writer/reader/voxel unit tests (numpy only)
bash -n fetch_prebuilt_maps.sh    # syntax check
```
