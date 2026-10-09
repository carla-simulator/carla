# Spawning vehicles — detail

Detail layer for the `spawn-vehicles` skill. Follows CARLA's own
`generate_traffic.py` vehicle flow.

## Spawn points

`world.get_map().get_spawn_points()` returns a fixed list of recommended
`carla.Transform`s (on the road, correctly oriented). Vehicles are spawned **one
per point** — two cars at the same point collide, so the vehicle count is capped
at `len(spawn_points)`. Shuffle the points (seedable) to vary which are used when
spawning fewer than all. (CARLA's `extract_spawn_points.py` dumps these points if
you need to inspect them.)

## Blueprint filtering

`world.get_blueprint_library().filter(pattern)` selects vehicle blueprints
(`vehicle.*`, `vehicle.tesla.*`, `vehicle.audi.a2`, ...). `--safe` keeps only
`number_of_wheels == 4`, dropping motorbikes/bicycles and odd vehicles that can
misbehave in traffic. A random `color` (from the blueprint's recommended values)
and `role_name = "autopilot"` are set per vehicle.

## The autopilot hand-off

Each vehicle is spawned and enrolled in autopilot in one atomic batch command:

```python
SpawnActor(bp, transform).then(SetAutopilot(FutureActor, True, tm_port))
```

`.then(...)` chains a follow-up command onto the actor the `SpawnActor` creates
(`FutureActor` is its placeholder id), so the vehicle is handed to the Traffic
Manager the instant it exists — no separate loop, no race. `apply_batch_sync(batch,
True)` runs the whole batch and returns per-command responses; collect the
`actor_id`s whose response has no `.error`.

Equivalent single-actor form: `vehicle.set_autopilot(True, tm_port)`.

## Placing a row in one lane (`line`)

`spawn` uses the map's scattered spawn points; `line` places vehicles at chosen
positions **along a single lane**:

1. `map.get_waypoint(location, project_to_road=True, lane_type=Driving)` snaps the
   given `--at` point to the nearest driving lane.
2. Walk the lane in `--gap`-metre steps with `wp.next(gap)` (or `wp.previous(gap)`
   for `--backward`), taking the first branch and collecting one waypoint per
   vehicle. If the lane ends first, it places as many as fit and says so.
3. Spawn a vehicle at each waypoint's transform, lifted `--z-offset` m so it drops
   onto the road rather than clipping it.

Notes:
- Vehicles stay in the **same lane** on straight/curved road segments; at a
  junction or fork `next()` may branch (first branch taken) — keep the run within
  one road, or check with the map-waypoints skill first.
- Spacing is centre-to-centre; a `--gap` smaller than a car length will collide
  and some spawns will fail. ~10 m+ is safe for cars.
- `--no-autopilot` makes a static queue (e.g. a traffic jam); the default hands
  each to autopilot so the platoon drives off.

This is placement, and lives here; **exploring** the lane/topology to pick the
`--at` point or inspect the road is the map-waypoints skill.

## Traffic Manager and sync

Autopilot is the Traffic Manager (`client.get_trafficmanager(port)`, default
`8000`). Rules:

- **Async world** (default): nothing extra — vehicles drive immediately.
- **Sync world**: the TM must be sync too (`tm.set_synchronous_mode(True)`), or
  vehicles freeze/jitter. `spawn` sets this automatically when the world is sync;
  you then tick the world (set-world-settings) to advance them.
- Use the **same `--tm-port`** for spawning and for any later TM tuning.

## Determinism

Reproducible traffic needs: a fixed `--seed` (this skill seeds Python's RNG and
`tm.set_random_device_seed`), synchronous mode with a fixed step, and the same
spawn order each run. Seed alone fixes *which* cars spawn *where*; full
determinism of their driving also needs sync mode (set-world-settings).

## Cleanup

`destroy` batch-destroys everything matching `vehicle.*`. Autopilot detaches
automatically when an actor is destroyed, so (unlike walkers + controllers) there
is no separate controller to stop first. If you spawned in sync mode, tick once
after destroying so the removals apply.
