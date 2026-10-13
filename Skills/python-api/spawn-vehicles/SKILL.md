---
name: spawn-vehicles
description: Spawns vehicles on a running CARLA server and destroys them — scattered across the map's spawn points on Traffic Manager autopilot, a row queued in a single lane at a fixed spacing, or a single hero/ego vehicle (optionally named for native ROS 2 publishing with --ros-name). Covers blueprint filtering, the atomic autopilot hand-off, and cleanup. Use when the user asks to "spawn vehicles/cars/traffic", "add N cars driving around", "put cars in a lane 15 m apart", "spawn the ego/hero vehicle", or "remove the vehicles".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT. Puts the world in synchronous mode by default (--no-sync opts out), and restores the previous settings on release. Tested against CARLA 0.9.16; the Traffic Manager and clock-ownership behaviour was measured on 0.10.0.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/vehicles.md
---

# Spawn autopilot vehicles

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Populate the roads with self-driving traffic. Vehicles are placed at the map's
predefined **spawn points** and, by default, handed to the **Traffic Manager**
autopilot at spawn time so they drive the road network autonomously. `destroy`
removes them.

**`spawn` and `line` make the world synchronous.** The Traffic Manager is not
reliable driving an asynchronous server: it runs its own loop against a server
advancing at whatever rate it likes. Measured on 0.10.0, async, 50 autopilot
vehicles with the spawning client alive and holding the TM: only 24 were
moving, and the actor count decayed on its own — 50 vehicles down to 39 and 15
walkers down to 5 over a couple of minutes. With a fixed timestep and the TM in
sync, 37 of 50 move and nothing decays. `--no-sync` keeps the old behaviour.

### The TM cannot be driven by another client's tick

The Traffic Manager steps only when the process that created it ticks the
world. Measured on 0.10.0: with a camera client owning a 20 Hz clock and this
skill attached as an observer, its 40 vehicles never moved at all — no error,
just a frozen scene. So a held traffic pool and a lock-step frame capture
cannot share a world; record the traffic here and replay it into the capture
instead ([[read-sensor]] has the recipe).

### The clock rule

The world settings are read before anything else happens:

* **asynchronous** — this client switches it to synchronous (`--delta`, default
  0.05 s = 20 Hz), puts the TM in sync, and becomes the ticker.
* **already synchronous** — another client owns the clock. This one never calls
  `tick()`; it observes with `wait_for_tick()`. Ticking a world you do not own
  silently drops commands — it is what stopped `spawn-walkers`' `start()` from
  landing.

The previous settings are restored on release, because a synchronous world with
nothing ticking does not advance and looks exactly like a hung server. Stop a
holder with Ctrl+C, never `kill -9`: the restore is in a `finally`.

**The Traffic Manager lives in the client process that created it.** So a
spawn that exits leaves the vehicles registered with a dead TM: they sit at
`throttle 0.00` for ever, in async *and* sync. Pass **`--hold`** to keep the TM
open (Ctrl+C releases it, and the vehicles then coast to a stop). Measured on
0.10.0: without `--hold`, 0/50 vehicles move; with it, 49/50 move.

The same is true of `controller.ai.walker` in [[spawn-walkers]] — its
controllers stop when their client exits — so a scene with both wants two
resident processes: this one owning the clock, the walker one held as an
observer.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Spawn N vehicles with --hold (the TM dies with the process without it)
- [ ] Step 3: Verify visually / via the world-data skill; spawn reports its count
- [ ] Step 4: Ctrl+C the held process, then destroy when done
```

Commands need `CARLA_HOST`/`CARLA_PORT`/`TM_PORT` from `scripts/env.sh`.
**Negative coordinates:** pass `--at` with an `=` (`--at=-24,-57,0.6`) so the
leading minus isn't parsed as a flag.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-4: Spawn / list / destroy

```bash
source scripts/env.sh

# 40 vehicles driving around on autopilot — --hold keeps the TM (and so the
# driving) alive; without it they spawn, then freeze when this command returns.
# The world is put in sync at 20 Hz and this client ticks it.
python3 scripts/vehicles.py spawn --count 40 --hold

# leave the world asynchronous (the old behaviour; the TM misbehaves)
python3 scripts/vehicles.py spawn --count 40 --hold --no-sync

# a different fixed timestep
python3 scripts/vehicles.py spawn --count 40 --hold --delta 0.033

# four-wheeled cars only, reproducible
python3 scripts/vehicles.py spawn --count 30 --safe --seed 42

# only Teslas, parked (no autopilot)
python3 scripts/vehicles.py spawn --count 10 --filter 'vehicle.tesla.*' --no-autopilot

# 5 vehicles in one lane, 15 m apart, starting near (x,y,z)
python3 scripts/vehicles.py line --at 30,20,0 --count 5 --gap 15
# ...as a static queue (no autopilot), laid behind the point
python3 scripts/vehicles.py line --at 30,20,0 --count 5 --gap 15 --no-autopilot --backward

# one hero/ego vehicle (autopilot off) — the anchor for control/sensors/telemetry
python3 scripts/vehicles.py ego --at 30,20,0        # prints its actor id + role=hero

# ...named for ROS 2: publishes TF as "hero" and accepts ROS control commands
python3 scripts/vehicles.py ego --at 30,20,0 --ros-name hero

python3 scripts/vehicles.py destroy                 # remove all vehicles
python3 scripts/vehicles.py destroy --filter 'vehicle.tesla.*'   # a subset (keeps the ego)
```

### Verify

The `spawn` command reports how many vehicles it created. Count is capped at the
number of spawn points (one vehicle per point); it reports if it capped or if some
spawns failed (occupied points) — both normal. Count/inspect live actors with the
world-data skill.

### ROS 2 (`ego` only)

On a server started with `--ros2` ([[run-carla-server]] `ROS2=1`), **only a
vehicle with `role_name = hero` is registered** with the ROS 2 layer — the server
checks that string explicitly. So `spawn`/`line` traffic is invisible to ROS and
cannot be driven from it, by design; there is no flag to change that here.

```bash
python3 scripts/vehicles.py ego --at=-24,-57,0.6 --ros-name hero
```

`--ros-name` / `--ros-frame-id` / `--no-ros-tf` set the `ros_name` /
`ros_frame_id` / `ros_publish_tf` blueprint attributes **before** spawn (they are
read once, at registration — no renaming afterwards). Registering the hero gives
(verified against a live server):

- `rt/carla/<ros_name>/vehicle_control_cmd` — `carla_msgs/CarlaEgoVehicleControl`.
- `rt/carla/<ros_name>/ackermann_control_cmd` — `ackermann_msgs/AckermannDriveStamped`.
  Both appear immediately, as **subscriptions** (the server is the subscriber).
- the topic prefix that sensors attached to it nest under ([[create-sensor]]).

**It does not publish its own transform.** `rt/tf` stays absent until a *sensor*
publishes — CARLA emits sensor→parent transforms only, so the `map`→vehicle
transform must come from outside (that is what the demo's `ego_tf_broadcaster.py`
does, [[visualize-ros-rviz]]). `ros_publish_tf=false` suppresses the sensor-side
transform for that actor; it does not disable a vehicle transform that was never
published.

Unlike sensors, a vehicle needs **no** `enable_for_ros()`. Driving it from those
topics: [[control-vehicle]].

## Examples

**Example 1: fill the roads**

User says: "spawn 50 cars driving around"

`spawn --count 50 --hold`. They drive on autopilot for as long as that command
runs; it reports how many spawned. Drop `--hold` only if you want them placed
and stationary.

**Example 2: reproducible car-only traffic**

User says: "same 30 cars every run, no motorbikes"

`spawn --count 30 --safe --seed 42`.

**Example 3: a queue in one lane**

User says: "put 5 cars in the same lane on this road, 15 m apart"

Find a point on the road (map-waypoints, or a known location), then
`line --at <x,y,z> --count 5 --gap 15`. They spawn in-lane 15 m apart and drive
off on autopilot; add `--no-autopilot` for a stationary queue.

**Example 4: clean up**

User says: "clear the traffic"

`destroy`.

## Troubleshooting

**Problem: fewer vehicles than requested**
Cause: count exceeds spawn points, or points were occupied.
Solution: expected; the map has a fixed number of spawn points (one car each).

**Problem: vehicles spawn but don't move (throttle stays 0.00)**
Cause: most often the spawning process exited, taking its Traffic Manager with
it — the vehicles are enrolled with a TM that no longer exists. Verified on
0.10.0 in both async and sync, and it is *not* a sync/TM mismatch: forcing the
TM async afterwards changes nothing, because the TM is gone.
Solution: spawn with `--hold`, or keep your own long-lived client holding
`client.get_trafficmanager(<port>)`. Re-registering `set_autopilot` from a new
process is not enough on its own. Other causes: `--no-autopilot`, or a sync
world with nothing calling `world.tick()` (set-world-settings).

**Problem: vehicles jitter / freeze in sync mode**
Cause: world sync but TM async (mismatch).
Solution: keep both in sync (set-world-settings couples them; `spawn` also sets
TM sync). Use the same `--tm-port` throughout.

**Problem: the server appears hung — every client blocks or times out**
Cause: the world was left synchronous with nothing ticking, usually because a
holder was `kill -9`ed instead of interrupted, so its restore never ran.
Solution: set `synchronous_mode = False` from any client
(set-world-settings), then start over. Note a frozen sync world also hands out
stale snapshots, so `get_actors()` can come back empty and `destroy` can report
0 actors — fix the clock before believing either.

## Outputs

Live autopilot traffic on the server. No file. `destroy` removes all vehicles.

Detail (blueprint filtering, spawn-point capping, the autopilot hand-off, TM/sync
interaction, determinism) in [references/vehicles.md](references/vehicles.md).
