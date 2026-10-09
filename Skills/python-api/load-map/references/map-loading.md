# Map loading — detail

Detail layer for the `load-map` skill: the exact Python API, what
`reset_settings` does, map layers, OpenDRIVE generation, and the gotchas that
bite when a load happens mid-pipeline.

## Contents

- The API surface
- reset_settings: reset vs keep
- Map layers (`_Opt` and large maps)
- OpenDRIVE worlds
- Verifying a load
- Gotchas

## Map names

`get_available_maps` on a 0.9.16 build returns 31 entries: the numbered towns
`Town01`–`Town07` each in plain and `_Opt` form, the HD town `Town10HD` /
`Town10HD_Opt`, the large maps `Town11`–`Town13` and `Town15`, plus non-town
levels (`BaseMap`, `LargeMap`, `Montague`, …). Pass names with the
`/Game/Carla/Maps/` prefix stripped.

Naming is **not uniform**, so `load_map.py load` resolves a friendly name to the
map actually used:

| You type | Resolves to | Why |
|---|---|---|
| `Town2`, `Town02`, `2` | `Town02` | numbered towns default to the **non-opt** map |
| `Town10`, `10` | `Town10HD_Opt` | Town10's canonical map is the layered HD one; plain `Town10HD` is effectively unused |
| `Town02_Opt`, `Town10HD` | exactly that | an exact/case-insensitive match always wins |

So a bare number gives the everyday map for that town, layers off — except
Town10, whose everyday map is the `_Opt` variant. Want layers on any other town?
Ask for its `_Opt` name explicitly (`Town03_Opt`).

## The API surface

All calls are on `carla.Client` except the layer toggles, which are on
`carla.World`. Signatures below are from CARLA 0.9.16.

| Call | Signature | Returns |
|---|---|---|
| list maps | `client.get_available_maps()` | `list[str]` of `/Game/Carla/Maps/<Name>` |
| load | `client.load_world(map_name, reset_settings=True, map_layers=MapLayer.All)` | `World` |
| reload | `client.reload_world(reset_settings=True)` | `World` |
| opendrive | `client.generate_opendrive_world(opendrive, parameters, reset_settings=True)` | `World` |
| load layer | `world.load_map_layer(map_layers)` | `None` |
| unload layer | `world.unload_map_layer(map_layers)` | `None` |

`get_available_maps` returns full asset paths; strip `/Game/Carla/Maps/` to get
the name you pass back to `load_world` (`load_map.py list` does this).

There is also `client.load_world_if_different(map_name, ...)` which skips the
reload when the requested map is already active — useful to avoid a costly no-op,
but it returns `None`, so re-fetch the world with `client.get_world()` after.

## reset_settings: reset vs keep

`reset_settings` decides what happens to `WorldSettings` when the new world comes
up. It is the single most consequential argument here.

| `reset_settings` | `--keep` | Effect on the new world |
|---|---|---|
| `True` (API default) | absent | Settings reset to server defaults: `synchronous_mode=False`, `fixed_delta_seconds=None`, `no_rendering_mode=False`, default substepping. |
| `False` | present | The current `WorldSettings` are re-applied to the new world: sync mode, fixed delta, rendering flag, and substepping all carry across. |

Why it matters: a data-collection or scenario pipeline runs in **synchronous
mode** with a fixed delta. A plain `load_world("Town03")` silently drops you back
to asynchronous, free-running rendering — the client and server stop being in
lockstep and captures desynchronise. `--keep` (`reset_settings=False`) is the fix.

## Map layers (`_Opt` and large maps)

`carla.MapLayer` is a bit-flag enum; combine values with `|`. Full set:

`NONE`, `Buildings`, `Decals`, `Foliage`, `Ground`, `ParkedVehicles`,
`Particles`, `Props`, `StreetLights`, `Walls`, `All`.

Two independent controls:

- **At load time** — `load_world(map, map_layers=...)` decides which layers exist
  when the map comes up. `MapLayer.NONE` gives the barest map (roads/ground only);
  `MapLayer.All` (default) loads everything.
- **After load** — `world.load_map_layer(...)` / `world.unload_map_layer(...)`
  stream layers in and out of the live world.

Layers only exist on **layered maps**: the `_Opt` variants (`Town01_Opt` …
`Town10HD_Opt`) and the large maps (`Town11`, `Town12`). On a fully-baked map
(plain `Town03`, etc.) every layer is part of the base geometry and the toggle
calls are silently ignored — not an error, just a no-op. `load_map.py` prints a
note to this effect rather than pretending something changed.

## OpenDRIVE worlds

`generate_opendrive_world(xodr_string, parameters, reset_settings=True)` builds a
drivable world from an OpenDRIVE road network — no buildings, props, or scenery,
just the road mesh. Pass the file **contents**, not the path.

`carla.OpendriveGenerationParameters` fields (metres unless noted), with the
stock `config.py` defaults that `load_map.py` uses:

| Field | Default | Meaning |
|---|---|---|
| `vertex_distance` | 2.0 | spacing of mesh vertices along roads; smaller = finer, heavier |
| `max_road_length` | 500.0 | roads longer than this are split into chunks |
| `wall_height` | 1.0 | height of the wall raised at road boundaries (0 = none) |
| `additional_width` | 0.6 | extra width added each side of every lane |
| `smooth_junctions` | `True` | smooth junction geometry to avoid artefacts |
| `enable_mesh_visibility` | `True` | render the generated mesh |

The generated map reports its name as `Carla/Maps/OpenDriveMap`. Converting OSM
first? `carla.Osm2Odr.convert(osm_string)` yields an xodr string you feed to the
same call (see stock `config.py`).

## Verifying a load

A returned `World` object is **not** proof the intended state is live — the map
could differ, or settings could have reset. After any mutating call, read back:

```python
s = world.get_settings()
print(world.get_map().name, s.synchronous_mode, s.fixed_delta_seconds, s.no_rendering_mode)
```

`load_map.py` prints exactly this as its `VERIFY` block. Confirm the map name is
the requested one and the settings are as intended (default after a plain load,
unchanged after `--keep`).

## Gotchas

- **Sync mode needs a tick after a kept load.** With `reset_settings=False` in
  synchronous mode, the new world will not advance until you call `world.tick()`.
  Nothing is "frozen" — it is waiting for the client tick, by design.
- **Traffic Manager and sync mode.** If a Traffic Manager was in sync mode, it is
  tied to the old world; after loading, re-assert TM sync (`tm.set_synchronous_mode(True)`)
  or vehicles may not move. (Owned by the traffic-manager skill — mentioned here
  only because it surfaces right after a map load.)
- **Actors do not survive a load.** `load_world` and `reload_world` both wipe all
  actors. Re-spawn afterwards; do not hold stale actor handles across a load.
- **Timeout on `generate_opendrive_world`.** Building geometry can take many
  seconds for a large network — a too-short client timeout raises during the call
  even though the server is fine. `env.sh` uses a longer working timeout (10s)
  than the 4s liveness probe in `check_env.sh`.
- **Version skew.** A client/server version mismatch (WARN in `check_env.sh`) can
  make map calls behave oddly; match the PythonAPI wheel to the server build.
