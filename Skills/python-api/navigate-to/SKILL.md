---
name: navigate-to
description: Drives a vehicle to a destination autonomously using CARLA's navigation agents (BasicAgent/BehaviorAgent/ConstantVelocityAgent), or plans a route between two points with GlobalRoutePlanner. The agent handles routing, obstacle avoidance and traffic-light obedience. Use when the user asks to "drive the ego to (x,y,z)", "navigate to the junction", "go to that location", "plan a route from A to B", or "make the car drive itself somewhere specific". Manual control is control-vehicle; autopilot roaming is control-traffic.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed and the CARLA checkout's PythonAPI/carla/agents on PYTHONPATH (env.sh sets it from CARLA_ROOT), plus a reachable running server with a target vehicle. Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/navigation.md
---

# Navigate a vehicle to a destination

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Point-to-point autonomous driving. `go` drives the ego to a destination with a
navigation agent that plans a route and follows it while avoiding obstacles and
obeying (or ignoring, on request) traffic lights. `route` just plans the path.

This is distinct from the sibling driving skills: [`control-vehicle`](../control-vehicle/SKILL.md)
is manual actuator input; [`control-traffic`](../control-traffic/SKILL.md) is the
Traffic Manager making cars roam; this drives one car to a **specific place**.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs (incl. agents import)
- [ ] Step 2: Have an ego (spawn-vehicles ego) and a destination (world-data/map-waypoints)
- [ ] Step 3: go (drive) — or route (plan only). Runs a tick loop until arrival/timeout
```

Commands need `CARLA_HOST`/`CARLA_PORT` + `CARLA_ROOT` from `scripts/env.sh`.
**Negative coordinates:** use the `=` form (`--to=-24,-57,0.6`).

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

FAILs if the `agents` package isn't importable — set `CARLA_ROOT` to a carla
checkout so `PythonAPI/carla/agents` is on `PYTHONPATH`.

### Step 3: Navigate

```bash
source scripts/env.sh

# plan a route and draw it (no driving)
python3 scripts/navigate.py route --from=0,0,1 --to=-24,-57,1 --draw

# drive the ego there (BasicAgent, obeys lights)
python3 scripts/navigate.py go --to=-24,-57,1 --speed 25

# a cautious/aggressive style
python3 scripts/navigate.py go --to=-24,-57,1 --agent behavior --behavior aggressive

# ignore lights (e.g. an emergency run) and give it more time
python3 scripts/navigate.py go --to=100,20,1 --ignore-lights --seconds 120
```

`go` targets the ego (`role hero`) by default; `--id`/`--filter` for another
vehicle. It blocks until the car arrives (`agent.done()`) or `--seconds` elapses.

## Examples

**Example 1: drive to a spot**

User says: "drive the ego to (−24, −57)"

`go --to=-24,-57,1`. It plans, drives, and prints "arrived" on success.

**Example 2: go to a resolved element**

User says: "drive to the 4-way junction in the middle"

`map-waypoints junctions --arms 4` → pick its centre → `go --to=<centre>`.

**Example 3: just the route**

User says: "what's the route from here to the depot?"

`route --from=<here> --to=<depot> --draw` → maneuver list + drawn path.

## Troubleshooting

**Problem: `cannot import agents`**
Cause: the agents package (checkout-only) isn't on PYTHONPATH.
Solution: set `CARLA_ROOT` to a carla checkout; env.sh adds `PythonAPI/carla`.

**Problem: it never arrives / stops at timeout**
Cause: unreachable destination, too little time, or stuck in traffic.
Solution: verify the destination is on a road (map-waypoints `waypoint`), raise
`--seconds`, or `--ignore-vehicles` to push through.

**Problem: the car doesn't move in sync mode**
Cause: `go` ticks the world itself in sync mode; if another process also ticks,
they fight.
Solution: run one ticker. In async it just drives.

**Problem: it stops at every red light forever**
Cause: dense traffic / lights.
Solution: `--ignore-lights` (and/or the control-traffic-lights skill to free them).

## Outputs

Server state — the vehicle drives to the destination (or a planned route + drawn
overlay for `route`). No file. `go` reports arrival or timeout.

Detail (agent types, route planner, run-step loop, sync interaction) in
[references/navigation.md](references/navigation.md).
