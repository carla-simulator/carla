---
name: spawn-walkers
description: Spawns pedestrians that wander autonomously via WalkerAIController, and destroys them in the correct order. Pairs each walker.pedestrian.* with a controller.ai.walker, places them on the navigation mesh, and sends them to random navmesh points at walking speed. Use when the user asks to "spawn pedestrians/walkers", "add people/crowds", "make pedestrians walk around", or "remove the walkers". Reads the world settings first: if the world is already synchronous it never ticks it, and if it is asynchronous it makes it synchronous and drives the clock itself.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server WITH a pedestrian navmesh for the loaded map. Does NOT need UE4_ROOT. Tested against CARLA 0.9.16; the client-lifetime and clock-ownership behaviour below was measured on 0.10.0.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/walkers.md
---

# Spawn wandering pedestrians

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Populate the map with pedestrians that walk around on their own. Each is a
`walker.pedestrian.*` body paired with a `controller.ai.walker` that steers it to
random points on the **navigation mesh**. The deliverable is a live, moving crowd;
`destroy` tears it down cleanly.

By default they wander **indefinitely**: on spawn every controller is `start()`ed,
given one random navmesh destination, and a random walking speed. That single
target is enough — CARLA's walker AI automatically picks a fresh random
destination each time a walker arrives (confirmed in LibCarla's nav code), so the
crowd roams forever with no re-targeting loop. Pass `--no-wander` for a stationary
crowd (controllers left unstarted).

This needs a navmesh — validate it first with the
[`debug-navmesh`](../debug-navmesh/SKILL.md) skill if walkers won't move.

**The controllers stop when the client that started them exits.** Same rule as
the Traffic Manager, and it is the reason a fire-and-forget spawn leaves the
crowd standing still. Pass **`--hold`**: the process stays alive and the crowd
keeps walking.

It is *not* primarily about pumping ticks, which is what this skill used to say.
The two were confounded, because `--hold` both keeps the client alive and pumps.
Separating them needed a second client owning a synchronous clock; measured on
0.10.0, walkers spawned against a world ticked by a traffic client:

| setup | result |
|---|---|
| spawn, process exits (another client ticking) | 0/15 move |
| a separate client issues `start()` and stays alive | 15/15 move, then stop when it exits |
| spawn `--hold`, observing only, never ticking | **10/10 move, 9.94 m in 8 s** |

The first row has a second symptom worth knowing: a later `start()` on those
controllers is *accepted* rather than rejected as already-started, which proves
the original call never landed. `_ensure_walking` therefore verifies by
displacement and re-issues (`--retries`) instead of trusting `start()`.

Ticks still have to come from somewhere, of course — that is the clock rule
below, not the walkers' own business.

### The clock rule

Read the world settings before doing anything:

* **already synchronous** — another client owns the clock. Never call `tick()`;
  observe with `wait_for_tick()`. Ticking a world you do not own is what
  silently drops commands like `start()`.
* **asynchronous** — this client may switch it to sync (`--delta`, default
  20 Hz) and become the ticker. `--no-sync` opts out.

Either way the previous settings are restored on release: a synchronous world
with nothing ticking does not advance at all, and is indistinguishable from a
hung server. So stop a holder with Ctrl+C (SIGINT), never `kill -9` — the
restore runs in a `finally`.

Walkers are also spawned `--z-offset` (default 2.0 m) above the navmesh point:
spawning at the navmesh z is rejected on 0.10.0 with *"Spawn failed because of
collision at spawn position"* (+0.5 still fails, +1.0 is the first that works).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: (if walkers won't move) validate the navmesh — debug-navmesh
- [ ] Step 3: Spawn N walkers with --hold (the controllers die with the client)
- [ ] Step 4: Verify visually / via the world-data skill; spawn reports its count
- [ ] Step 5: Ctrl+C the held process, then destroy (controllers first, then walkers)
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3-5: Spawn / roam / destroy

```bash
source scripts/env.sh

# 30 pedestrians wandering at 1.0-1.8 m/s — --hold keeps this client alive, and
# the controllers stop the moment it exits
python3 scripts/walkers.py spawn --count 30 --hold

# reproducible placement (still wanders forever)
python3 scripts/walkers.py spawn --count 50 --seed 42

# more road-crossing, faster walkers
python3 scripts/walkers.py spawn --count 20 --cross-factor 0.4 --speed-min 1.4 --speed-max 2.2

# a stationary crowd (no autonomous movement)
python3 scripts/walkers.py spawn --count 20 --no-wander

python3 scripts/walkers.py destroy        # correct-order teardown
```

### Verify

The `spawn` command reports how many walkers + controllers it created (they
should be equal). Watch them move on a rendered server; count/inspect live actors
with the world-data skill. Fewer than requested is normal at high counts (navmesh
points collide) — the spawn command reports the shortfall.

## Examples

**Example 1: add a crowd**

User says: "spawn 40 pedestrians walking around"

`spawn --count 40 --hold`. They head to random navmesh points and keep walking
for as long as that command runs; it reports how many spawned.

**Example 2: a reproducible pedestrian scene**

User says: "same 25 pedestrians every run"

`spawn --count 25 --seed 7`. They wander indefinitely; the seed fixes placement.

**Example 3: clean up**

User says: "remove all the pedestrians"

`destroy` — stops and removes controllers first, then the walkers.

## Troubleshooting

**Problem: walkers spawn but stand still (walk animation plays, no movement)**
Cause: in order of likelihood — (1) the spawning client exited, so its
controllers stopped; (2) `start()` was dropped, which happens when the client
ticked a world it did not own; (3) nothing is advancing the world at all;
(4) the navmesh. It looks identical in all four cases.
Solution: spawn with `--hold`. Obey the clock rule above — never `tick()` a
world that is already synchronous. `_ensure_walking` re-issues `start()`
automatically and reports how many are confirmed walking, so a silent failure
is now a printed number. Only if that still says 0, validate the navmesh with
debug-navmesh — a PASS there rules the map out.

**Problem: a crowd looks frozen but `get_velocity()` is the evidence**
Cause: on this build nav-driven walkers report `get_velocity()` ≈ 0 the entire
time they are walking — measured 10/10 walking by displacement while every one
of them read below 0.1 m/s.
Solution: judge movement by displacement between two samples, never by
velocity. This applies to your own checks as much as to this skill's.

**Problem: far fewer spawned than requested (or zero)**
Cause: `Spawn failed because of collision at spawn position`. At high counts
random navmesh points collide with each other; *all* of them fail if the
vertical offset is too small (0.10.0 rejects the bare navmesh z).
Solution: keep `--z-offset` at its 2.0 m default — a count of 10 returning 0 is
an offset problem, not a crowding one. Then retry, lower the count, or batch.

**Problem: errors / ghost actors after cleanup**
Cause: walkers destroyed before their controllers (wrong order).
Solution: always use `destroy` here — it stops controllers and removes them
first, then the walkers.

**Problem: pedestrians constantly walk into the road**
Cause: high `--cross-factor`.
Solution: lower it (default 0.1); 0 keeps them on sidewalks.

## Outputs

A live crowd of wandering pedestrians (walker + controller pairs) on the server.
No file. `destroy` removes them in the safe order.

Detail (the two-phase batch spawn, controller API, navmesh dependency, sync
optionality, destroy order) in [references/walkers.md](references/walkers.md).
