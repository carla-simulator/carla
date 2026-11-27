---
name: populate-world
description: Fills a running CARLA map with autopilot vehicles AND wandering pedestrians in one command, from one held client that owns the clock, the Traffic Manager and the walker controllers — no second process, no clock hand-off. Reuses the spawn-vehicles and spawn-walkers code, so their filters (--safe, --filter, --base-type) and walker options apply. Use when the user asks for both at once — "fill the map with 50 vehicles and 20 pedestrians", "populate the town", "add traffic and people", "bring the city to life" — or to remove both. For only vehicles (or a lane queue / ego) use spawn-vehicles; for only pedestrians use spawn-walkers.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server with a pedestrian navmesh. Needs the sibling skills spawn-vehicles and spawn-walkers present (their scripts are imported). Puts an asynchronous world in synchronous mode (--no-sync opts out) and restores it on release.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  requires: spawn-vehicles
---

# Populate the world with vehicles and pedestrians

> **Paths.** `scripts/…` below are relative to the directory holding this
> SKILL.md. Your working directory is the user's project, not that directory,
> so prefix them with its absolute path or the command is not found.

## Overview

Both the Traffic Manager and `controller.ai.walker` stop when the client that
created them exits, and only one client may tick a synchronous world. Using
[[spawn-vehicles]] and [[spawn-walkers]] separately therefore needs two
resident processes, started in order (vehicles first, owning the clock; walkers
second, observing). This skill does both in **one** process:

1. Claim the clock (async → sync at `--delta`, default 20 Hz; an already-sync
   world is observed, never ticked).
2. Open the TM in sync and batch-spawn `--vehicles` on autopilot at spawn points.
3. Spawn `--walkers` on the navmesh, attach controllers, start them, and verify
   by displacement that most are walking (re-issuing `start()` if not).
4. `--hold`: one tick loop keeps everything moving until Ctrl+C (or
   `--seconds N` of simulated time). On release: walkers (controllers first),
   then vehicles are destroyed, and the world settings restored.

The spawn code is imported from the two sibling skills; their gotchas sections
apply unchanged.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Spawn both with --hold, in the background
- [ ] Step 3: Read the "populated: N vehicles + M walkers" line and the walker confirmation
- [ ] Step 4: When done, Ctrl+C (SIGINT) the holder — never kill -9
```

### Step 1

```bash
bash scripts/check_env.sh
```

### Step 2

Run it as a background process: it does not return while holding.

```bash
source scripts/env.sh

# the usual request
python3 scripts/populate.py spawn --vehicles 50 --walkers 20 --hold

# "safe" vehicles: cars only (four wheels and base_type car; no bikes, trucks, vans or buses)
python3 scripts/populate.py spawn --vehicles 50 --walkers 20 --safe --hold

# reproducible, and ends by itself after 60 s simulated time
python3 scripts/populate.py spawn --vehicles 30 --walkers 30 --seed 42 --seconds 60

# leave the actors behind when the holder is released (they stop moving)
python3 scripts/populate.py spawn --vehicles 30 --walkers 10 --hold --keep

python3 scripts/populate.py destroy     # all walkers (safe order), then all vehicles
```

Walker options from spawn-walkers pass through: `--speed-min/--speed-max`,
`--cross-factor`, `--z-offset`, `--settle`, `--retries`, `--verify-frames`.
`--vehicles 0` or `--walkers 0` skips that population.

### Step 3: Verify

The output ends with `populated: N vehicles + M walkers` and a
`k/M confirmed walking` line. Fewer than requested is normal (spawn points
capped / occupied, navmesh collisions). Count live actors with [[world-data]].

## Using the scene afterwards

The holder **owns the clock**. Any other client must observe with
`wait_for_tick()`, never `tick()`: [[control-spectator]] follow and
[[bounding-boxes]] / [[debug-draw]] work alongside it. A lock-step sensor
capture that wants its own clock cannot share the world with a held TM (see
[[spawn-vehicles]] "The TM cannot be driven by another client's tick").

## Troubleshooting

**Vehicles or walkers frozen** — the holder exited or was killed. Re-run with
`--hold` and keep it alive.

**Server looks hung after a crash** — the world was left synchronous with no
ticker (holder `kill -9`ed). Set `synchronous_mode = False` with
[[set-world-settings]], then `destroy`.

**0 walkers** — no navmesh, or `--z-offset` too low; see [[spawn-walkers]] and
[[debug-navmesh]].

## Outputs

A live scene of autopilot traffic and wandering pedestrians, for as long as the
holder runs. No file.
