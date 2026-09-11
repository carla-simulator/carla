# Tools

## extract_blueprint_sizes.py

Regenerates the fallback blueprint dimension tables
(`blueprint_sizes_vehicle.json`, `blueprint_sizes_walker.json`) from a running
CARLA server. The integration normally probes the live server by itself
(`BlueprintLibrary(world=...)`) and caches per CARLA version; these JSONs are
only used when constructing `BlueprintLibrary()` without a world.

```sh
python tools/extract_blueprint_sizes.py [--host 127.0.0.1] [-p 2000] [--output-dir DIR]
```

| Option | Default | Description |
|---|---|---|
| `--host` | 127.0.0.1 | CARLA server address |
| `-p`, `--port` | 2000 | CARLA RPC port |
| `--output-dir` | module dir | Where to write the JSON files |

The measurement works by spawning each `vehicle.*` and `walker.pedestrian.*`
blueprint high above the map, reading `actor.bounding_box` (extent = half
dimensions, location = bounding-box center offset from the actor origin), and
destroying the actor.
