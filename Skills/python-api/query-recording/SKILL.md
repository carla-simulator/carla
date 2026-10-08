---
name: query-recording
description: Inspects a recorded CARLA .log without replaying it — show_recorder_file_info for the header/actors/frames, collision queries between actor categories, and blocked-actor queries for vehicles that got stuck. Use when the user asks to "what's in this recording", "show recorder info", "which vehicles collided", "find collisions in the log", or "which actors were blocked/stuck".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Needs a .log on the server (from the record-simulation skill). Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  requires: record-simulation
  prerequisites: scripts/check_env.sh
  reference: references/queries.md
---

# Query a CARLA recording

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Read facts out of a `.log` without replaying it. The server parses the file and
returns a text report. Three questions:

- **What's in it?** — `info` (header: map, duration, frames, actor list).
- **What crashed?** — `collisions` between two actor categories.
- **What got stuck?** — `blocked` actors that barely moved.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Run the query that answers the question (info / collisions / blocked)
- [ ] Step 3: Read the report; use ids/times to drive a targeted replay if needed
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2: Query

```bash
source scripts/env.sh

# header + actors (add --all for every frame; large)
python3 scripts/query.py info --file /tmp/run.log

# collisions: category codes h=hero v=vehicle w=walker t=traffic-light o=other a=any
python3 scripts/query.py collisions --file /tmp/run.log --type1 v --type2 a   # any vehicle vs anything
python3 scripts/query.py collisions --file /tmp/run.log --type1 v --type2 w   # vehicle vs walker

# actors that moved < 1 m (100 cm) over >= 30 s (stuck)
python3 scripts/query.py blocked --file /tmp/run.log --min-time 30 --min-distance 100
```

**Units matter:** `--min-time` is seconds, `--min-distance` is **centimetres**
(100 = 1 m).

### Step 3: Act on it

The reports give frames, times and actor ids — feed them to
[`replay-recording`](../replay-recording/SKILL.md) (`--start`, `--follow`) to
jump to and watch the interesting moment.

## Examples

**Example 1: overview**

User says: "what's in /tmp/run.log?"

`info --file /tmp/run.log` → map, duration, frame count, actors.

**Example 2: find a crash**

User says: "did any car hit a pedestrian?"

`collisions --file /tmp/run.log --type1 v --type2 w`. Each line gives the frame,
time and the two actor ids; then `replay play --start <t> --follow <id>`.

**Example 3: find gridlock**

User says: "which cars got stuck?"

`blocked --file /tmp/run.log --min-time 20 --min-distance 50` (moved < 0.5 m in
20 s).

## Troubleshooting

**Problem: parse error / empty report**
Cause: the `.log` is not on the server at that path, or has no frames.
Solution: use the server-side path; confirm the recording with `info` first.

**Problem: `collisions` returns nothing but I saw a crash**
Cause: wrong categories, or collisions need `additional_data` context.
Solution: widen with `--type2 a` (any); ensure the recording captured the actors.

**Problem: `blocked` lists everything / nothing**
Cause: `--min-distance` unit confusion (centimetres, not metres).
Solution: 100 = 1 m; raise `--min-time` to ignore brief stops at lights.

## Outputs

Text reports printed to stdout (no world change, no file written). Use them to
target a [`replay-recording`](../replay-recording/SKILL.md).

Detail (report fields, category codes, blocked-query semantics) in
[references/queries.md](references/queries.md).
