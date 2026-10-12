# Result artefacts and the metrics module

## What each output flag produces

| Flag | Output |
|---|---|
| `--output` | criteria table on stdout |
| `--file` | `<outputDir>/<config><timestamp>.txt` — the same table |
| `--json` | `<outputDir>/<config><timestamp>.json` |
| `--junit` | `<outputDir>/<config><timestamp>.xml`, JUnit format for CI |
| `--record DIR` | `$SCENARIO_RUNNER_ROOT/DIR/<config>.log` (recorder) **and** `<config>.json` (criteria dump) |

The timestamp is appended with **no separator** — `ControlLoss_12025-08-14-10-22-31.json`
is `ControlLoss_1` + `2025-08-14-10-22-31`. `--outputDir` only has an effect
together with `--file`/`--json`/`--junit`.

The `--record` criteria JSON is written by `ScenarioRunner._record_criteria`, which
dumps every JSON-serialisable attribute of every criterion. Non-serialisable
attributes (carla objects) are dropped silently, so the file is a projection of the
criterion, not the whole thing.

## The recorder log

`--record` calls `client.start_recorder(name, True)` — the `True` is
`additional_data`, so the log includes physics control, vehicle lights, traffic
light state and bounding boxes, not just transforms. That is what makes the metrics
module able to reconstruct a run.

Recorder logs are **version-specific**: a log from 0.9.16 does not replay on
0.10.0.

## Metrics module

```bash
python3 metrics_manager.py --metric <metric.py> --log <recording.log> [--criteria <criteria.json>]
                           [--host 127.0.0.1] [--port 2000]
```

It needs a **running server** despite computing offline: `metrics_manager.py` calls
`client.show_recorder_file_info()` to parse the log and loads the map the recording
names, so it can hand the metric a real `carla.Map` for the Waypoint API.

Flow: `metrics_manager.py` → `MetricsParser` → `MetricsLog` → your
`BasicMetric._create_metric(town_map, log, criteria)`.

### Writing a metric

```python
from srunner.metrics.examples.basic_metric import BasicMetric

class MyMetric(BasicMetric):
    def _create_metric(self, town_map, log, criteria):
        ego = log.get_ego_vehicle_id()
        start, end = log.get_actor_alive_frames(ego)
        for frame in range(start, end):
            t = log.get_actor_transform(ego, frame)
            v = log.get_actor_velocity(ego, frame)
            wp = town_map.get_waypoint(t.location)
            ...
```

The class name must match the file name in TitleCase, exactly as with agents and
scenarios (`my_metric.py` → `MyMetric`).

### `MetricsLog` API

*Simulation-level*
`get_total_frame_count()`, `get_elapsed_time(frame)`, `get_delta_time(frame)`,
`get_platform_time(frame)`

*Finding actors* — use these instead of hard-coding ids, which change between runs
`get_ego_vehicle_id()`, `get_actor_ids_with_role_name(role)`,
`get_actor_ids_with_type_id(pattern)` (fnmatch, e.g. `"vehicle.*"`),
`get_actor_attributes(id)`, `get_actor_bounding_box(id)`,
`get_actor_alive_frames(id)` → `(first, last)`

*Per-actor state* — each has three forms: one actor+frame, one actor across all
frames, all actors at one frame
`get_actor_transform` / `get_all_actor_transforms` / `get_actor_transforms_at_frame`
`get_actor_velocity` / `get_all_actor_velocities` / `get_actor_velocities_at_frame`
`get_actor_angular_velocity` / `…_at_frame`
`get_actor_acceleration` / `…_at_frame`

*Vehicles and walkers*
`get_vehicle_control(id, frame)`, `get_vehicle_physics_control(id, frame)`,
`get_walker_speed(id, frame)`, `get_vehicle_lights(id, frame)`,
`is_vehicle_light_active(light, id, frame)`

*Traffic lights and scene*
`get_traffic_light_state(id, frame)`, `is_traffic_light_frozen(id, frame)`,
`get_traffic_light_elapsed_time(id, frame)`,
`get_traffic_light_state_time(id, state, frame)`,
`get_traffic_light_trigger_volume(id)`, `get_scene_light_state(id, frame)`

*Collisions*
`get_actor_collisions(id)` → `{frame: [other ids]}`

Anything not present at a frame comes back `None`, so loops need a guard — that is
the usual cause of a metric that crashes halfway through.

### Bundled examples

| File | Demonstrates |
|---|---|
| `srunner/metrics/examples/basic_metric.py` | the base class (not runnable itself) |
| `criteria_filter.py` | reading the criteria dict, writing a table with `tabulate` |
| `distance_between_vehicles.py` | two-actor per-frame distance, matplotlib plot |
| `distance_to_lane_center.py` | `town_map.get_waypoint` per frame, lateral error plot |

Each has a matching pair in `srunner/metrics/data/` — a `.log` and a
`_criteria.json` — so they can be run with no simulation of your own:

```bash
python3 metrics_manager.py \
  --metric srunner/metrics/examples/distance_between_vehicles.py \
  --log    srunner/metrics/data/DistanceBetweenVehicles.log \
  --criteria srunner/metrics/data/DistanceBetweenVehicles_criteria.json
```

These sample logs were recorded on an old CARLA and may not replay on a current
server; if `show_recorder_file_info` returns nothing usable, record your own.

## OpenSCENARIO 2.0 traces

`srunner/metrics/tools/osc2_log.py` and `osc2_trace_parser.py` are the OSC2
equivalents of `metrics_log.py` / `metrics_parser.py`, used for traces produced by
`--openscenario2` runs. Same shape, different parser.

## Criteria → Leaderboard infractions

Criteria raise `TrafficEvent`s
(`srunner/scenariomanager/traffic_events.py`), and the Leaderboard's
`StatisticsManager` reads exactly those events to build infractions and a score.
The mapping is worth knowing when a scenario passes here but scores badly there:

| `TrafficEventType` | Leaderboard infraction key |
|---|---|
| `COLLISION_STATIC` | `collisions_layout` |
| `COLLISION_PEDESTRIAN` | `collisions_pedestrian` |
| `COLLISION_VEHICLE` | `collisions_vehicle` |
| `TRAFFIC_LIGHT_INFRACTION` | `red_light` |
| `STOP_INFRACTION` | `stop_infraction` |
| `OUTSIDE_ROUTE_LANES_INFRACTION` | `outside_route_lanes` |
| `MIN_SPEED_INFRACTION` | `min_speed_infractions` |
| `YIELD_TO_EMERGENCY_VEHICLE` | `yield_emergency_vehicle_infractions` |
| `SCENARIO_TIMEOUT` | `scenario_timeouts` |
| `ROUTE_DEVIATION` | `route_dev` |
| `VEHICLE_BLOCKED` | `vehicle_blocked` |
| `ROUTE_COMPLETION` | drives `score_route` |

So "all criteria passed" in ScenarioRunner and "driving score 100" in the
Leaderboard are the same statement, computed from the same events.
