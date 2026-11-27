# Traffic Manager — detail

Detail layer for the `control-traffic` skill. The TM (`client.get_trafficmanager(
port)`, default 8000) drives every vehicle enrolled in autopilot on that port.

## Scope: global vs per-vehicle

- **Global** calls affect all vehicles on the TM port and are the TM's defaults
  for new ones.
- **Per-vehicle** calls take the actor and override the global value for that car.

Always use the **same port** the vehicles were spawned on; multiple TMs can run on
different ports (2000/8000, 2002/8001, ...).

## Settings

Global (`global` command):

| Call | Meaning |
|---|---|
| `global_percentage_speed_difference(p)` | all cars p%% below the limit (negative = faster) |
| `set_global_distance_to_leading_vehicle(m)` | following distance, metres |
| `set_random_device_seed(n)` | deterministic TM decisions (needs sync too) |
| `set_hybrid_physics_mode(bool)` + `set_hybrid_physics_radius(m)` | full physics only near the hero; far cars are teleported cheaply |
| `set_osm_mode(bool)` | OpenStreetMap-imported road behaviour |
| `set_synchronous_mode(bool)` | TM steps with the world's tick |

Per-vehicle (`vehicle` / `all` commands), each takes the actor:

| Call | Meaning |
|---|---|
| `vehicle_percentage_speed_difference(v,p)` | this car's speed (sign as above) |
| `distance_to_leading_vehicle(v,m)` | this car's following distance |
| `ignore_lights_percentage(v,p)` | %% of red lights it runs |
| `ignore_signs_percentage(v,p)` | %% of stop/yield signs it ignores |
| `ignore_vehicles_percentage(v,p)` | %% of other vehicles it ignores (→ collisions) |
| `ignore_walkers_percentage(v,p)` | %% of pedestrians it ignores |
| `auto_lane_change(v,bool)` | allow/forbid lane changes |
| `keep_slow_lane_rule_percentage(v,p)` | adherence to keep-right/slow-lane rule |
| `vehicle_lane_offset(v,m)` | lateral offset from lane centre (+ = right) |
| `update_vehicle_lights(v,bool)` | TM auto-manages this car's blinkers/brake lights |
| `force_lane_change(v,bool)` | one-off lane change (True = right) |
| `collision_detection(ref,other,bool)` | toggle collision awareness between two actors |

## The speed-difference sign (common gotcha)

`percentage_speed_difference` is a percentage **below** the speed limit:

- `+30` → 30% **slower** than the limit,
- `0` → at the limit,
- `-30` → 30% **faster** than the limit.

So "make traffic faster" = a **negative** value.

## Determinism

Reproducible traffic needs all of: `set_random_device_seed(n)`, synchronous mode
on both the world (set-world-settings) and the TM (`sync on`), a fixed world step,
and a seeded spawn (spawn-vehicles `--seed`). Seed alone in async mode is not
deterministic.

## Sync coupling

TM sync must match the world's sync mode. The set-world-settings skill couples
them when you switch modes there; this skill's `sync` sets only the TM side. In
sync mode the world must be ticked for the TM to advance the cars.

## Relationship to other skills

- **spawn-vehicles** enrols cars in autopilot (this TM); **control-vehicle** takes
  a car OUT of autopilot for manual driving — don't tune a manually-driven car
  here.
- Hybrid physics matters mainly on the large maps (Town12, Town13, Town15).

## Routes, paths and plans (measured on ue58-dev ccc52d24d, Town10HD_Opt)

| Call | Result |
|---|---|
| `set_desired_speed(v, 15)` | 14.9–15.0 km/h held: the unit is km/h |
| `set_path(v, locations)` | 15 points along a route with turns, each passed within 2 m |
| `set_route(v, ['Right'] * 4)` | straight through three junctions, and `get_all_actions` never showed `Right`: route options are matched to the TM's road graph, whose branch points can differ from the OpenDRIVE junctions |
| `get_next_action` / `get_all_actions` from another process | `[]` before the TM RPC fix (the client dropped the reply); with it, the same `[option, carla.Waypoint]` pairs as in the TM's own process |
| TM on port 3656 next to a server on 3654 | `bind error`: 3656 is taken by the server, so pick a TM port away from RPC + 1 and + 2 |
