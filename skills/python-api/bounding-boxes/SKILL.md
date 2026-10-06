---
name: bounding-boxes
description: Gets actor and level 3D bounding boxes and projects them into a camera image as 2D boxes — the dataset-annotation workflow. Lists box geometry, draws 3D boxes in the world via debug, or captures one camera frame and writes an annotated PNG plus a JSON of 2D boxes for matching actors. Use when the user asks to "get bounding boxes", "draw boxes around the cars", "project boxes onto the camera", or "generate a detection dataset / annotations".
license: MIT
compatibility: Any OS with the CARLA PythonAPI + numpy (cv2 for drawing) installed for the active interpreter and a reachable, already-running CARLA server. `project` needs an RGB camera (create-sensor). Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  requires: create-sensor
  prerequisites: scripts/check_env.sh
  reference: references/bounding-boxes.md
---

# Bounding boxes

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Three things: read actors' 3D boxes, draw them in the world, or **project** them
into a camera image as 2D boxes with a JSON sidecar — the standard CARLA
dataset-annotation pipeline (`client_bounding_boxes.py`).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: For `project`, have a camera on the ego (create-sensor) + actors around
- [ ] Step 3: list / draw / project
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Boxes

```bash
source scripts/env.sh

# 3D box geometry of every vehicle
python3 scripts/bounding_boxes.py list --filter vehicle.*

# draw 3D boxes around vehicles — they track moving cars, stay 30 s (default)
python3 scripts/bounding_boxes.py draw --filter vehicle.*
python3 scripts/bounding_boxes.py draw --filter vehicle.* --seconds 60

# project vehicle boxes into a camera frame -> annotated PNG + JSON
python3 scripts/bounding_boxes.py project --camera <camera_id> --filter vehicle.* --out boxes.png
```

## How the projection works (brief)

Capture one frame → build the camera intrinsics `K` from its `fov` + resolution →
for each actor, take its 3D box's 8 world vertices (`get_world_vertices`),
transform to camera space with the camera's inverse matrix, keep those in front,
apply `K` → 2D points → the min/max is the 2D box. Off-frame and too-far
(`--max-dist`) actors are skipped. Full math in the reference.

## Examples

**Example 1: annotate the ego camera**

User says: "put boxes on the cars in the camera view"

Spawn a camera on the ego (create-sensor), then `project --camera <id>
--filter vehicle.*` → `boxes.png` with green 2D boxes + `boxes.json`.

**Example 2: how big is that actor**

User says: "what are the dimensions of vehicle 137?"

`list --filter vehicle.*` (find 137) → its LxWxH from the extent.

**Example 3: visualise in 3D**

User says: "outline all the vehicles in the world"

`draw --filter vehicle.*` — green 3D boxes on a rendered server.

## Troubleshooting

**Problem: `project` finds no boxes**
Cause: no actors in front of / within `--max-dist` of the camera, or wrong filter.
Solution: raise `--max-dist`, widen `--filter`, ensure vehicles are in view.

**Problem: boxes misaligned**
Cause: the frame and the actor poses drifted (fast motion, async).
Solution: capture in synchronous mode (set-world-settings) so the frame and poses
are from the same tick.

**Problem: `draw` shows nothing**
Cause: headless `-nullrhi` server.
Solution: use a rendered server (see debug-draw).

**Note: boxes always track moving actors**
`draw` re-stamps each frame for `--seconds` (default 30), so boxes ride moving
cars automatically — the command blocks for that duration.

## Outputs

- `list`: box dimensions per actor.
- `draw`: 3D box overlay (rendered view).
- `project`: `<out>.png` (annotated) + `<out>.json` (2D boxes: id, type, xyxy).

Detail (intrinsics, world→camera transform, level boxes, lidar-to-camera) in
[references/bounding-boxes.md](references/bounding-boxes.md).
