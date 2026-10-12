---
name: load-map
description: Loads, reloads, and reshapes the map on a running CARLA server via the Python API — get_available_maps, load_world and reload_world (with or without resetting world settings), generate a world from an OpenDRIVE .xodr, and load/unload map layers. Use when the user asks to "load a map", "change the town", "reload the world", "list available maps", "load an xodr/opendrive map", or "load/unload map layers", and when settings (sync mode, fixed delta) must survive the switch.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT or a source checkout. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/map-loading.md
---

# Load a CARLA map

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Change what map a **running** server hosts, from a client. The deliverable is a
verified world: after any load the map name and the sync/rendering settings are
read back, because a load can quietly land on a different map or reset settings.

The one decision that governs every load: **reset settings, or keep them?**
`reset_settings=True` (the API default) drops the new world to async/default
rendering; `reset_settings=False` carries your current `synchronous_mode`,
`fixed_delta_seconds`, and `no_rendering_mode` across. In this skill that is the
`--keep` flag. Full semantics: [references/map-loading.md](references/map-loading.md).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: List maps if the target name is unknown
- [ ] Step 3: Do the operation (load / reload / opendrive / layer), keep-settings if a sync pipeline is live
- [ ] Step 4: Verify the printed map name + settings match what was asked
```

All commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh` (defaults
`127.0.0.1:2000`). Prefix any command with `source scripts/env.sh` or export them.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

FAILs only on a missing `carla` module or an unreachable server — both are hard
blockers for a client operation. Start a server first if it FAILs (no server is
launched by this skill).

### Step 2: List available maps

```bash
source scripts/env.sh
python3 scripts/load_map.py list
```

Names are returned stripped of the `/Game/Carla/Maps/` prefix, so `Town03`,
`Town10HD_Opt`, etc. — pass those straight to `load`.

**Friendly names resolve, but not uniformly** — `load` maps a bare town name to
the map actually used: `Town2` → `Town02` (non-opt, like every numbered town
except one), but **`Town10` → `Town10HD_Opt`** (Town10's canonical map is the
layered HD one; plain `Town10HD` exists but is effectively unused). An explicit
exact name (`Town02_Opt`, `Town10HD`) is always honoured as typed; `load` prints
what it resolved to.

### Step 3: Do the operation

```bash
# load a map, resetting settings to default (the plain case)
python3 scripts/load_map.py load --map Town03

# load a map but KEEP current settings — use inside a sync-mode pipeline
python3 scripts/load_map.py load --map Town03 --keep

# reload the current map (fresh actors), reset vs keep settings
python3 scripts/load_map.py reload
python3 scripts/load_map.py reload --keep

# build a world from an OpenDRIVE file (road network only, no props)
python3 scripts/load_map.py opendrive --xodr /path/to/road.xodr

# build a world from an OpenStreetMap export (.osm -> xodr -> world)
python3 scripts/load_map.py osm --osm /path/to/city.osm

# stream layers in/out on a layered ('_Opt') map already loaded
python3 scripts/load_map.py load --map Town10HD_Opt --layers Ground,Buildings
python3 scripts/load_map.py layer --load Foliage,ParkedVehicles
python3 scripts/load_map.py layer --unload ParkedVehicles
```

Low freedom on the two fragile points — `--keep` when a sync pipeline is live,
and layers only on `_Opt`/large maps; otherwise choose maps/params freely.

### Step 4: Verify

Every mutating command prints a `VERIFY` block (map name + `synchronous_mode`,
`fixed_delta_seconds`, `no_rendering_mode`). Do **not** trust that a call
returned — confirm the block: the map name is the one you asked for, and the
settings are default (plain) or unchanged (`--keep`). An OpenDRIVE world reports
its name as `Carla/Maps/OpenDriveMap`.

### On a ROS 2 server

A map switch is an **episode** switch, so on a server started with `--ros2`
([[run-carla-server]] `ROS2=1`):

- **`rt/carla/map` re-publishes automatically** — the new map's full OpenDRIVE,
  as a latched (`transient_local`) `std_msgs/String`. Verified: the sample content
  changes with the map. It carries no header, so there is no stamp or episode id to
  correlate with; reading it needs an explicit `--qos-durability transient_local`
  request (plus `--full-length`, or it truncates at 128 chars).
- **Every actor is destroyed — but its topics do NOT go away.** Verified: after a
  switch the old sensor topic is still listed with `Publisher count: 1` and
  publishes nothing, because the ROS 2 layer does not unregister publishers on
  episode teardown. Re-spawning with the same `ros_name` then gives
  **`Publisher count: 2`** — one live, one zombie — and it accumulates per switch.
  A subscriber can match the dead endpoint and wait forever.
- **Re-spawn and re-enable after the switch** ([[spawn-vehicles]],
  [[create-sensor]] `--ros`): `ros_name`s and `enable_for_ros()` state are gone
  with the actors even though the topics linger.
- `rt/clock` keeps ticking. Restart the server if the zombie publishers matter for
  what you are measuring.

Verifying it needs a ROS 2 consumer ([[visualize-ros-rviz]]); from the RPC side,
[[world-data]] `ros-topics` shows what the new episode should be publishing — and
disagreement with `ros2 topic list` after a map change is expected, for the reason
above.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

**Map layers are a silent no-op on 0.10.0.** The calls succeed, return without
error, and change nothing. Measured on Town10HD_Opt with a 40-ray sweep at 3 m
height: `Buildings` geometry was hit 9 times before `unload_map_layer(
MapLayer.Buildings)`, 9 times after, and 9 times again after reloading — and the
server log records no streaming activity at all.

The cause is content, not the API. `ACarlaGameModeBase::UnLoadMapLayer` still
streams sublevels, and `ConvertMapLayerMaskToMapNames` matches layer names
against `World->GetStreamingLevels()` — but the UE5 conversion **flattened the
layers into the persistent level**. `Town10HD_Opt.umap` is 32.9 MB with **zero**
`LevelStreaming` references on 0.10.0, against 158 KB with two on 0.9.x, where it
streamed `T10HD_Buildings`, `T10HD_Props`, `T10HD_Foliage` and friends. The
sublevel `.umap` files still sit in `Content/Carla/Maps/Sublevels/Town10HD_Opt/`
as orphans, so nothing matches and nothing happens.

To hide geometry on 0.10.0 use `enable_environment_objects` instead
([[toggle-env-objects]]). `load_world`, `reload_world`,
`get_available_maps`, `generate_opendrive_world` and the settings-preservation
behaviour are all unchanged.

`get_ego_spawn_points()` is new, and **UE 5.8 only** — spawn points the map author
marked for the hero vehicle, a subset of `get_spawn_points()`. On 5.5 it raises
`AttributeError` ([[check-ue5-limitations]]).

## Examples

**Example 1: just switch town**

User says: "load Town05"

`python3 scripts/load_map.py load --map Town05`. VERIFY shows `map = Carla/Maps/Town05`,
`synchronous_mode = False` (reset to default). Done.

**Example 2: switch town without breaking a sync-mode data run**

User says: "I'm recording in sync mode at 20 fps — switch to Town02 but keep my settings"

`python3 scripts/load_map.py load --map Town02 --keep`. VERIFY must still show
`synchronous_mode = True`, `fixed_delta_seconds = 0.05`. Because sync mode is
preserved, tick the world to advance it (the command prints this note).

**Example 3: load a custom road**

User says: "load this opendrive file as the world"

`python3 scripts/load_map.py opendrive --xodr ./my_road.xodr`. VERIFY shows
`Carla/Maps/OpenDriveMap`. It is a road network only — no buildings or props.

**Example 4: minimal map, then add buildings**

User says: "load Town10 with just the ground, then bring buildings in"

`load --map Town10HD_Opt --layers Ground` then `layer --load Buildings`.

## Troubleshooting

**Error: `FAIL  no CARLA server at 127.0.0.1:2000`**
Cause: no server running, or wrong host/port.
Solution: start a CARLA server, or set `CARLA_HOST`/`CARLA_PORT` to the right one.

**Error: `FAIL  cannot import carla`**
Cause: the active interpreter has no `carla` module.
Solution: activate the env with the PythonAPI wheel, or set `PYTHON` to it.

**Problem: sync mode "lost" after loading a new map**
Cause: `load_world`/`reload_world` default to `reset_settings=True`.
Solution: pass `--keep` (`reset_settings=False`). See the reference.

**Problem: `layer --load/--unload` does nothing**
Cause: the current map is fully baked (not an `_Opt`/large map); it has no
toggleable layers.
Solution: load an `_Opt` map first (`Town01_Opt` … `Town10HD_Opt`) or a large map.

## Outputs

This skill produces **server state**, not a file: the running server now hosts
the requested map, with settings reset or preserved as asked. The `VERIFY` block
is the confirmation of that state.

Deeper detail — the exact `reset_settings` behaviour, the full `MapLayer` list,
OpenDRIVE generation parameters, and the sync/Traffic-Manager gotchas — is in
[references/map-loading.md](references/map-loading.md).
