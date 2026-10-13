---
name: control-traffic
description: Tunes the CARLA Traffic Manager that drives autopilot vehicles — global and per-vehicle speed, following distance, running lights/signs, lane-change behaviour, lane offset, TM-managed lights, deterministic seed, hybrid physics, and TM sync mode. Use when the user asks to "make the traffic faster/slower", "let cars run red lights", "keep more distance", "stop lane changes", "make traffic deterministic", or otherwise change how autopilot cars behave. For manually driving one car, use control-vehicle.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server with autopilot vehicles (spawn-vehicles). Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/traffic-manager.md
---

# Control the Traffic Manager

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Shape how **autopilot** vehicles drive — globally (all cars on a TM port) or
per-vehicle. This is the counterpart to [`control-vehicle`](../control-vehicle/SKILL.md):
that skill drives one car manually; this one tunes the autonomous traffic. It
affects cars enrolled in autopilot (see [`spawn-vehicles`](../spawn-vehicles/SKILL.md)),
on the matching `--tm-port`.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Have autopilot vehicles on the TM port (spawn-vehicles)
- [ ] Step 3: Apply global and/or per-vehicle settings
- [ ] Step 4: Watch the change on a rendered server
```

Commands need `CARLA_HOST`/`CARLA_PORT`/`TM_PORT` from `scripts/env.sh`. Use the
**same `--tm-port`** the vehicles were spawned on (default 8000).

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Tune

```bash
source scripts/env.sh

# GLOBAL: all traffic 30% faster than the limit, keep 3 m, deterministic
python3 scripts/traffic.py global --speed-diff -30 --distance 3 --seed 42

# EVERY car: run 50% of red lights, allow lane changes
python3 scripts/traffic.py all --ignore-lights 50 --auto-lane-change on

# ONE car (the ego's neighbour): crawl, hug the right, TM manages its lights
python3 scripts/traffic.py vehicle --id 42 --speed-diff 60 --lane-offset 0.5 --lights on

# make the ego's autopilot reckless
python3 scripts/traffic.py vehicle --role hero --ignore-vehicles 40 --ignore-signs 100

# TM sync (only alongside a synchronous world — see set-world-settings)
python3 scripts/traffic.py sync on
```

### Step 4: Verify

Watch the traffic on a rendered server. Settings are write-only on the TM (no
read-back), so the command reports what it set and to how many vehicles.

## Examples

**Example 1: faster traffic**

User says: "make the traffic move faster"

`global --speed-diff -40` (40% above the limit). Negative = faster.

**Example 2: chaos for testing**

User says: "make cars run lights and ignore each other"

`all --ignore-lights 100 --ignore-vehicles 60` — expect collisions (that's the
point of the test).

**Example 3: deterministic traffic run**

User says: "same traffic behaviour every run"

`global --seed 42`, in synchronous mode (`sync on` + set-world-settings), spawn
with a seed. Seed + sync is what makes it reproducible.

## Troubleshooting

**Problem: nothing changes**
Cause: cars aren't on this TM port, or aren't on autopilot, or you set the ego
(which you're driving manually).
Solution: match `--tm-port` to the spawn; ensure autopilot is on (spawn-vehicles
default); target autopilot cars.

**Problem: cars froze after `sync on`**
Cause: TM sync without a synchronous, ticked world (or vice-versa).
Solution: keep TM sync and world sync in step and tick the world (set-world-settings).

**Problem: `--speed-diff 30` made them slower, not faster**
Cause: the value is % *below* the limit.
Solution: use a negative value to go faster (`-30` = 30% above the limit).

**Problem: collisions everywhere**
Cause: high `--ignore-vehicles` / `--ignore-lights`.
Solution: lower them; 0 restores full caution.

## Outputs

Traffic Manager behaviour state (global + per-vehicle). No file, no read-back; the
command reports what it applied.

Detail (every setting, the speed-difference sign, hybrid physics, determinism,
sync coupling) in [references/traffic-manager.md](references/traffic-manager.md).
