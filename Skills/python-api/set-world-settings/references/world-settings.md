# World settings — detail

Detail layer for the `set-world-settings` skill: every `WorldSettings` field, the
substepping math, and the synchronous/asynchronous + Traffic-Manager rules that
make or break a deterministic run.

## Contents

- WorldSettings fields
- Synchronous vs asynchronous
- Traffic Manager coupling
- Physics substepping
- Determinism checklist
- Gotchas

## WorldSettings fields

Applied with `world.apply_settings(settings, seconds=0.0)`, which returns the
frame id the change lands on (not a success flag — read the settings back). Fields
and their defaults on a fresh `carla.WorldSettings()` (0.9.16):

| Field | Default | Meaning |
|---|---|---|
| `synchronous_mode` | `False` | server advances only on `world.tick()` when `True` |
| `fixed_delta_seconds` | `None` | fixed time step in seconds; `None` = variable (server clock) |
| `no_rendering_mode` | `False` | skip rendering (physics/logic still run) — faster headless |
| `substepping` | `True` | split each frame into physics substeps |
| `max_substep_delta_time` | `0.01` | largest substep length (s) |
| `max_substeps` | `10` | max substeps per frame |
| `max_culling_distance` | `0.0` | actor render culling distance (0 = off) |
| `deterministic_ragdolls` | `False` | reproducible ragdoll physics |
| `tile_stream_distance` | `3000.0` | large-map tile streaming distance (m) |
| `actor_active_distance` | `2000.0` | large-map actor activation distance (m) |
| `spectator_as_ego` | `True` | treat the spectator as the ego for large-map streaming |

`fps` is derived: `fps = 1 / fixed_delta_seconds` (0.05 s ⇒ 20 fps).

## Synchronous vs asynchronous

- **Asynchronous** (default): the server runs on its own clock as fast as it can;
  the client reads whatever frame is current. Fine for free driving and casual
  visualisation. Sensor callbacks arrive unaligned to any client step.
- **Synchronous**: the server blocks until the client calls `world.tick()`, then
  advances exactly `fixed_delta_seconds`. Required for frame-aligned multi-sensor
  capture and for reproducibility. **Set `fixed_delta_seconds` whenever you enable
  sync** — variable-step sync is not meaningful.

In sync mode nothing advances until you tick. A "frozen" server or client
time-outs right after enabling sync almost always means no one is ticking.

## Traffic Manager coupling

The Traffic Manager (`client.get_trafficmanager(port)`, default port `8000`) has
its own sync flag, `tm.set_synchronous_mode(bool)`. It must match the world:

- **World sync + TM sync**: the TM computes its update inside your tick — orderly,
  deterministic traffic.
- **World sync + TM async**: the TM races the tick — vehicles jitter, stutter, or
  teleport.
- **World async + TM sync**: the TM waits for ticks that never come — traffic
  stalls.

`sync`/`async` in this skill set both together so they cannot drift. If you manage
the TM yourself, flip its mode in lockstep with the world's, and do it on the same
TM port you drive traffic on.

## Physics substepping

With `substepping = True`, each frame is integrated in up to `max_substeps`
substeps of at most `max_substep_delta_time` seconds. The invariant CARLA
enforces:

```
max_substep_delta_time * max_substeps >= fixed_delta_seconds
```

Defaults give `0.01 * 10 = 0.1 s`, so `fixed_delta_seconds` up to `0.1` is
covered. A larger step leaves physics under-integrated and unstable. Keep the step
`<= 0.1` (i.e. `>= 10` fps); the skill auto-raises `max_substeps` to satisfy the
invariant and warns when the requested step is above `0.1`.

## Determinism checklist

Reproducible runs need all of:

1. `synchronous_mode = True` with a fixed `fixed_delta_seconds`.
2. Traffic Manager in sync mode (this skill) **and** a fixed TM random seed
   (`tm.set_random_device_seed(n)` — owned by the traffic-manager skill).
3. The same client tick loop and spawn order each run.

Sync mode alone is necessary but not sufficient — an unseeded TM still randomises.

## Gotchas

- **Restore async on exit.** Leaving the server in sync mode strands the next
  client (it will look hung). Ending a sync task with `async` hands the clock back.
- **`apply_settings` is not confirmation.** It returns a frame id; always read
  `world.get_settings()` back (the skill's `VERIFY` block does).
- **Keeping settings across a load.** `load_world`/`reload_world` reset settings
  unless `reset_settings=False` — pair this skill with the `load-map` skill's
  `--keep`. Re-assert TM sync after the load; a load detaches the old world.
- **`no_rendering_mode` ≠ off-screen.** It disables the render pipeline entirely
  (no camera/lidar images), but physics and the RPC world keep running.
