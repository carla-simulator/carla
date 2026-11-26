---
name: control-traffic-lights
description: Controls the traffic-light actors on a running CARLA server — set a light green/red/yellow/off, freeze all lights (optionally all-green), set phase timing (green/yellow/red durations), reset to the normal cycle, and draw each light's live data in the world (state, time in state, phase times, frozen) as a debug overlay. Target all lights, one by actor id or OpenDRIVE signal id, all in a junction, or the nearest. Use when the user asks to "make the light red", "freeze all lights green", "change the light timing", "turn the traffic lights off", or "reset the lights", or to "show the traffic lights data / draw the traffic light states". This is the light actors, not the Traffic Manager (that's control-traffic).
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/traffic-lights.md
---

# Control traffic lights

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Drive the actual `traffic.traffic_light` actors: set states, freeze them, tune
phase timing. Not to be confused with [`control-traffic`](../control-traffic/SKILL.md)
— that is the Traffic Manager governing how autopilot *vehicles* drive; this is
the *lights themselves*.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: list to see lights/states; get a junction id from map-waypoints if needed
- [ ] Step 3: set / freeze / timing / reset (all, by id, by junction, or nearest)
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`. **Negative
coordinates:** use `--near=-24,-57,1`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Control

```bash
source scripts/env.sh

python3 scripts/traffic_lights.py list                      # ids, states, timing

# freeze the whole grid green (classic "green wave" / free-flow test)
python3 scripts/traffic_lights.py freeze on --state green
python3 scripts/traffic_lights.py freeze off                # back to normal cycling

# set one light, or all in a junction, or the nearest to a point
python3 scripts/traffic_lights.py set --state red --id 42
python3 scripts/traffic_lights.py set --state green --junction 838
python3 scripts/traffic_lights.py set --state red --near=-24,-57,1
python3 scripts/traffic_lights.py set --state red --opendrive-id 1067   # by the signal id in the .xodr

# phase timing (seconds) on all lights
python3 scripts/traffic_lights.py timing --green 20 --yellow 3 --red 10 --all

python3 scripts/traffic_lights.py reset                     # normal cycle again
```

### Show the lights' data in the world (`draw`)

```bash
python3 scripts/traffic_lights.py draw                      # every light, 120 s
python3 scripts/traffic_lights.py draw --junction 189 --seconds 0   # one junction, until Ctrl+C
```

Each light gets a bar across every lane it controls, on the stop line, in its
current colour, and a label above it: `TL <id> [<OpenDRIVE id>] <STATE>
<elapsed>/<duration>s  G<green> Y<yellow> R<red> [FROZEN]`. It is redrawn every
frame with a lifetime of about two frames, so it follows the cycle and is gone
as soon as the command stops. It only waits for ticks, never ticks, so it runs
alongside a traffic holder. Needs a rendered server, like any debug drawing.
`--color r,g,b` draws every light in that colour instead of its state colour.

The timer is shown for green and yellow only. Measured on Town10HD_Opt: a red
light waiting for its turn in the junction group reads `get_elapsed_time()`
0.0 the whole time, and `red_time` (2 s there) is only the all-red pause after
yellow, not how long the light stays red.

## Examples

**Example 1: let traffic flow**

User says: "make all the lights green so cars don't stop"

`freeze on --state green`. All lights hold green until `freeze off` / `reset`.

**Example 2: red light at one junction**

User says: "make the crossroads in the middle red"

`map-waypoints junctions --arms 4` → central junction id → `set --state red
--junction <id>`.

**Example 3: faster cycle**

User says: "shorten the light cycle"

`timing --green 10 --yellow 2 --red 6 --all`.

## Troubleshooting

**Problem: lights snap back after `set`**
Cause: they keep cycling; `set` changes the current state, the cycle continues.
Solution: `freeze on` to hold a state, or `timing` to change the cycle itself.

**Problem: `--junction` returns nothing**
Cause: wrong junction id, or that junction has no signalised lights.
Solution: get the id from map-waypoints `junctions`; not all junctions are lit.

**Problem: vehicles still stop at a green I set**
Cause: they may be on a different phase, or autopilot reacts to its own light.
Solution: `freeze on --state green` affects all; or let cars ignore lights via
control-traffic (`ignore-lights`).

## Outputs

Traffic-light state on the server (states/freeze/timing). No file. `list` reports
current states and timings.

Detail (states, freeze semantics, groups, per-junction/waypoint lookup) in
[references/traffic-lights.md](references/traffic-lights.md).
