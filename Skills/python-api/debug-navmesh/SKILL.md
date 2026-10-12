---
name: debug-navmesh
description: Visualises and validates a map's pedestrian navigation mesh by sampling walkable locations (get_random_location_from_navigation) and drawing them as debug points. Use when the user asks to "show/visualise the pedestrian navmesh", "check walkable areas", "validate pedestrian navigation after a map import", or "why won't my walkers move". Confirms the navmesh loaded and how far it covers.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Drawing needs a rendered view; validation works headless. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/navmesh.md
---

# Debug the pedestrian navmesh

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

CARLA does not expose navmesh geometry, but every location from
`get_random_location_from_navigation()` sits on the walkable mesh. Sampling many
of them **validates** the navmesh loaded and **visualises** its coverage — the
check to run after importing a map, or when walkers refuse to move.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Validate — confirm the navmesh exists and its extent
- [ ] Step 3: (rendered view) Sample + draw to see the walkable area
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2: Validate (works headless)

```bash
source scripts/env.sh
python3 scripts/navmesh.py validate --count 500
```

PASS reports the number of valid points, unique locations, and the walkable span.
FAIL (exit 1) means `get_random_location_from_navigation()` returned nothing — no
usable navmesh, so pedestrians with a `WalkerAIController` will not navigate.

### Step 3: Visualise (rendered server)

```bash
python3 scripts/navmesh.py sample --count 2000 --life 120
```

Draws the sampled points via `world.debug`; the dotted region is walkable. Needs a
windowed/packaged server (headless `-nullrhi` draws nothing — see debug-draw).

## Examples

**Example 1: after importing a map**

User says: "I imported a town — did the pedestrian nav build?"

`validate` → PASS with a sensible span means yes; FAIL means the nav mesh is
missing and must be rebuilt during import.

**Example 2: walkers won't move**

User says: "my pedestrians just stand still"

`validate`. If it FAILs, the navmesh is the cause; if it PASSes, look at the
WalkerAIController wiring (owned by the walker-spawning skill), not the map.

**Example 3: see the walkable area**

User says: "show me where pedestrians can walk"

`sample --count 3000 --life 180` on a rendered server; inspect the point cloud.

## Troubleshooting

**Problem: `validate` FAILs on a freshly imported map**
Cause: pedestrian navigation was not generated/exported for the map.
Solution: rebuild the map's nav mesh in the import pipeline; then re-validate.

**Problem: `sample` prints a count but nothing is visible**
Cause: headless `-nullrhi` server, or the camera is far from the coverage bounds.
Solution: use a rendered server and move the spectator over the reported bounds.

**Problem: very few unique locations**
Cause: navmesh is tiny or degenerate (a sliver of walkable area).
Solution: check the source geometry / sidewalks in the imported map.

## Outputs

- `validate`: a PASS/FAIL report with coverage bounds and walkable span.
- `sample`: a transient point overlay of the walkable area (rendered view).

Detail (navmesh basics, WalkerAIController link, import checklist) in
[references/navmesh.md](references/navmesh.md).
