# Spawning pedestrians — detail

Detail layer for the `spawn-walkers` skill. Follows CARLA's own
`generate_traffic.py` walker flow.

## The two actors

A working pedestrian is **two** actors:

- `walker.pedestrian.*` — the body. Attributes: `is_invincible` (set `false` so it
  collides/can be hit), `speed` (recommended walk/run values; runtime speed is set
  on the controller instead).
- `controller.ai.walker` — the brain, spawned **parented to the walker**. Methods:
  `start()`, `stop()`, `go_to_location(carla.Location)`, `set_max_speed(m_s)`.

## Two-phase batch spawn

Spawning is done in phases with `client.apply_batch_sync(batch, True)`:

1. **Bodies** — for each walker pick `world.get_random_location_from_navigation()`
   (a point guaranteed on the navmesh), `SpawnActor(walker_bp, Transform(loc))`.
   Collect the ids that did not error.
2. Advance a frame (`world.tick()` in sync, else `world.wait_for_tick()`) so the
   walkers register.
3. **Controllers** — `SpawnActor(controller_bp, Transform(), parent=walker_id)`
   for each walker; advance a frame again.
4. **Start** — for each controller: `start()`, `go_to_location(random navmesh
   point)`, `set_max_speed(random speed)`.

Advancing between phases is required whether async or sync — it is one
`wait_for_tick` in async, not a demand for sync mode.

## Random wandering is infinite by default

You only set the destination **once**. CARLA's walker AI re-targets on its own:
on arrival it *"set[s] a new random target"* (LibCarla `nav/Navigation.cpp`
~L897-909 and `WalkerManager::SetWalkerRoute`), so a single
`start()` + `go_to_location(random)` makes the walker roam forever. CARLA's own
`generate_traffic.py` relies on exactly this — it calls `go_to_location` once and
never again. Hence there is no re-targeting loop in this skill: `spawn` starts
them and they wander indefinitely. `--no-wander` skips the `start()`, leaving a
stationary crowd. Speeds ~1.0-1.8 m/s read as normal walking; ~2.0-2.5 as hurried.

## Navmesh dependency

Everything hinges on the navmesh: `get_random_location_from_navigation()` returns
spawn points and destinations. If it returns `None`, there is no navmesh — walkers
cannot be placed or steered. Validate with the debug-navmesh skill before blaming
the controllers. `world.set_pedestrians_seed(n)` makes the sampling reproducible;
`world.set_pedestrians_cross_factor(p)` sets how often they cross roads (0-1).

## Destroy order (important)

Tear down in this order:

1. `controller.stop()` for every controller.
2. Destroy the controllers.
3. Destroy the walkers.

Destroying a walker while its controller lives strands the controller (it steers a
non-existent body) and can log errors or leave ghost actors. The skill's `destroy`
does controllers-then-walkers via `apply_batch_sync`.

## Sync mode (optional)

Pedestrians work in async mode. Use sync only for reproducible/deterministic runs
(set-world-settings): then you must tick the world for walkers to advance, and the
same tick drives their motion. The script adapts automatically (`tick` vs
`wait_for_tick`), so no code change is needed either way.
