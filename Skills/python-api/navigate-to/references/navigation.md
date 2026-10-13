# Navigation agents — detail

Detail layer for the `navigate-to` skill. Follows `automatic_control.py`.

## The agents package

`agents.navigation` ships in the CARLA checkout at `PythonAPI/carla/agents` — it
is **not** in the installed `carla` wheel. env.sh puts `CARLA_ROOT/PythonAPI/carla`
on `PYTHONPATH` so `import agents...` works; check_env verifies it.

- **BasicAgent(vehicle, target_speed=20, opt_dict, map_inst, grp_inst)** — plans a
  route to a destination, follows it, brakes for vehicles/pedestrians, and obeys
  traffic lights. `set_destination(location)`, `run_step()` → `VehicleControl`,
  `done()`, `ignore_traffic_lights/vehicles/stop_signs(True)`, `set_target_speed`,
  `follow_speed_limits`, `set_global_plan`.
- **BehaviorAgent(vehicle, behavior='normal'|'cautious'|'aggressive')** — richer:
  car-following, overtaking, tailgating, speed adaptation per style.
- **ConstantVelocityAgent(vehicle, target_speed)** — holds a constant speed along
  the route (useful for reproducible scenarios).
- **GlobalRoutePlanner(map, sampling_resolution)** — `trace_route(origin, dest)` →
  list of `(waypoint, RoadOption)`; `RoadOption` ∈ {LANEFOLLOW, LEFT, RIGHT,
  STRAIGHT, CHANGELANELEFT, CHANGELANERIGHT, VOID} — the turn at each step.

## The drive loop (`go`)

Autonomous driving is a per-frame loop (agents are not fire-and-forget):

```
agent = BasicAgent(ego, speed); agent.set_destination(loc)
while not done and time left:
    world.tick()            # sync ...  or world.wait_for_tick() in async
    if agent.done(): break
    ego.apply_control(agent.run_step())
```

`go` runs exactly this for `--seconds`, turning autopilot off first (the agent is
the driver — manual/TM/agent control are mutually exclusive). On arrival it stops;
on timeout it brakes and reports.

## Sync vs async

`go` ticks the world itself in synchronous mode, so don't run a second ticker
concurrently (they fight over advancing frames). In asynchronous mode it just
reads the server's ticks and applies control — no extra ticking needed. For a
fully reproducible run, drive in sync with a fixed step (set-world-settings) and a
seeded TM (control-traffic) if other traffic is present.

## Route vs drive

`route` only calls the planner and reports the maneuver sequence (and optionally
draws the path) — nothing moves. `go` plans and executes. Use `route` to preview
or to feed a plan elsewhere; use `go` to actually get there.

## Interplay

- Destination often comes from a resolver: map-waypoints `junctions`/`waypoint`,
  or world-data `actors` (drive to where an actor is).
- Dense lights/traffic can stall `go`; `--ignore-lights`/`--ignore-vehicles`, or
  free the lights with the control-traffic-lights skill.
