---
name: map-waypoints
description: Explores a CARLA map's road network through the Map/Waypoint API — a natural-language rundown of the whole map ("give me the topology"), drawing the topology to confirm the modelled map matches the visible one, enumerating junctions to resolve phrases like "the 4-way junction in the middle", nearest-waypoint lane facts, and next/previous lane navigation. Use when the user asks about the map's layout, lanes, junctions, topology, or to locate/navigate specific map elements.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Drawing needs a rendered view; text queries work headless. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/waypoints.md
---

# Explore a CARLA map

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Query and visualise the road network of the loaded map. Three headline uses:

1. **Rundown** — `summary` turns the map into stats *and* a one-paragraph
   description ("a dense, city-like map … mostly 2-lane roads … 12 junctions").
2. **Show the topology** — `topology --draw` overlays the modelled road graph so
   you can confirm it matches the map you see rendered.
3. **Resolve a described element** — `junctions` lists every junction with its
   centre, arm count, distance-to-map-centre and bearing, so a phrase like
   "the 4-way junction in the middle" resolves to a specific junction id.

Plus per-point queries: `waypoint` (lane facts) and `navigate` (next/previous).

## How natural language maps to elements

This skill emits **structured data**; the agent does the matching (like the
weather skill). "The 4-way junction in the middle" →
`junctions --arms 4`, then pick the smallest `dist ... of centre`. "The northern
junction" → the one whose bearing is `N`. Draw the chosen one with
`junctions --arms 4 --draw` (or by id) to confirm. See
[references/waypoints.md](references/waypoints.md) for the vocabulary
(centre/near/far, bearings, "biggest" = size).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Rundown the map (summary) if asked about layout
- [ ] Step 3: For a described element, enumerate (junctions) then filter to one
- [ ] Step 4: Draw it (--draw) to confirm modelled == visible; navigate if needed
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`. **Negative
coordinates:** pass `--at` with an `=` (e.g. `--at=-24,-57,0.6`) so the leading
minus isn't parsed as a flag; the space form only works for all-positive coords.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-4: Explore

```bash
source scripts/env.sh

# a rundown of the map
python3 scripts/waypoints.py summary

# draw the whole road topology to compare with the visible map
python3 scripts/waypoints.py topology --draw --life 180

# every junction; or just the four-way ones, nearest the centre first
python3 scripts/waypoints.py junctions
python3 scripts/waypoints.py junctions --arms 4
python3 scripts/waypoints.py junctions --arms 4 --draw     # highlight + label them

# lane facts at a point, and a forward walk along the lane
python3 scripts/waypoints.py waypoint --at 30,20,0
python3 scripts/waypoints.py navigate --at 30,20,0 --dist 2 --steps 30 --draw
```

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

**`Landmark.waypoint` is always `None` on 0.10.0.** Measured on Town10HD_Opt:
68 landmarks, 68 of them with `waypoint is None`. Every other landmark field
populates normally (`id`, `name`, `type`, `road_id`, `s`, `t`, `distance`,
`orientation`, `transform`, `country`, `unit`, `value`, `sub_type`, `text`,
`h_offset`, `z_offset`, `height`, `width`, `pitch`, `roll`, `is_dynamic`,
`get_lane_validities`).

So any code shaped `landmark.waypoint.transform` or
`landmark.waypoint.lane_id` raises `AttributeError: 'NoneType'` there. Get the
road position from the landmark itself instead — `map.get_waypoint_xodr(
landmark.road_id, lane_id, landmark.s)`, or `map.get_waypoint(
landmark.transform.location)` when you only need the nearest lane.

`get_all_landmarks`, `get_all_landmarks_of_type`, `get_all_landmarks_from_id` and
`get_landmark_group` all work on both versions. The rest of the Map/Waypoint API
this skill uses — `get_topology`, `generate_waypoints`, `get_waypoint`,
`get_spawn_points`, `get_crosswalks` — is unchanged.

## Examples

**Example 1: describe the map**

User says: "give me a rundown of the topology"

`summary`. Relay the RUNDOWN paragraph (extent, road/lane counts, dominant lane
count, junction breakdown, density).

**Example 2: confirm the model matches what's rendered**

User says: "show me the topology so I can check the map"

`topology --draw` on a rendered server; the green arrows trace every road segment
— they should sit on the visible roads.

**Example 3: resolve a described junction**

User says: "highlight the 4-way junction in the middle"

`junctions --arms 4` → pick the row with the smallest "of centre" distance → note
its id → `junctions --arms 4 --draw` (or read its centre and `debug-draw box`
there). Confirm the label lands on the central crossroads.

**Example 4: navigate a lane**

User says: "follow the lane forward from (30,20) for 60 m"

`navigate --at 30,20,0 --dist 2 --steps 30 --draw`.

## Troubleshooting

**Problem: `summary` is slow on a large map**
Cause: Town11/12/13 have huge road networks; stats sample every 3 m.
Solution: expected; it still completes. Increase `STAT_STEP` in the script for a
faster, coarser pass.

**Problem: drawn topology/junctions not visible**
Cause: headless `-nullrhi` server, or the spectator is far away.
Solution: use a rendered server (see debug-draw) and move to the map centre.

**Problem: a "junction" has an odd arm count**
Cause: arms are counted as distinct entry road ids — merges/ramps can skew it.
Solution: treat arm count as approximate; confirm visually with `--draw`.

**Problem: `waypoint --at` says no driving lane**
Cause: the point is off-road or on a non-driving lane.
Solution: move the point onto a road; the query projects to the nearest driving
lane within range.

## Outputs

Text reports (rundown, junction list, lane facts, navigation path) and optional
`world.debug` overlays. No file, no world change.

Detail (the Map/Waypoint/Junction API, arm counting, bearings/centre semantics,
lane topology) in [references/waypoints.md](references/waypoints.md).
