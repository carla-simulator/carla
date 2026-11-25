---
name: walker-pose-keypoints
description: Reads a pedestrian's skeleton on a running CARLA server — every bone's name and world or actor-relative transform — and projects it into a camera as 2D keypoints, writing one JSON per frame (bone, world position, pixel, in-image flag) and an annotated PNG with the skeleton drawn on it, for pose-estimation datasets; also draws the skeleton in the world, and poses a walker by hand (set_bones, blend between the pose and the animation, freeze the current animation frame, release back to the animation). Use when the user asks for "pedestrian keypoints", "2D skeleton annotations", "human pose ground truth", "list the walker's bones", "make the pedestrian raise its arms", or "freeze the pedestrian mid-stride".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. keypoints needs numpy, and Pillow for the annotated PNG (without it the raw frame is saved). Verified against ue58-dev ccc52d24d (CARLA 1.0) on Town10HD_Opt.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/skeleton.md
---

# Walker pose and keypoints

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

Spawning the walker belongs to `spawn-walkers`, the camera to `create-sensor`, and 2D boxes to `bounding-boxes`. This skill reads and drives the skeleton of a walker that already exists.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Get the walker id (world-data actors --filter 'walker.*') and a camera id (create-sensor)
- [ ] Step 3: Read bones, write keypoints, or pose the walker
- [ ] Step 4: Open one annotated PNG and check the skeleton sits on the body
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Run

```bash
source scripts/env.sh
P="python3 scripts/pose.py"

$P bones --id 26                                   # 66 bones, world transforms
$P bones --id 26 --relative                        # relative to the actor
$P keypoints --id 26 --camera 27 --frames 10 --out kp/      # kp/<frame>.json + kp/<frame>.png
$P keypoints --id 26,31,40 --camera 27 --frames 100 --tick  # several walkers, synchronous mode
$P draw --id 26 --seconds 10                       # skeleton as debug lines in the world

$P pose --id 26 --file pose.json                   # pose.json: {"crl_arm__R": {"rotation": [0, 0, 70]}}
$P blend --id 26 --value 0.5                       # half pose, half animation
$P freeze --id 26                                  # hold the current animation frame
$P release --id 26                                 # back to the animation
```

### Keypoint output

Each `<frame>.json` holds the camera id, image size and, per walker, one entry per bone: `bone`, `world` (x, y, z in metres), `uv` (pixel, or `null` behind the camera) and `in_image`. The PNG draws 22 links of the main skeleton (spine, head, arms, legs) in green and every bone in red. `in_image` is a frustum test, not an occlusion test: a keypoint behind a car still counts as in the image.

The camera pose is read from the image itself and the bones are read on the same frame. In synchronous mode pass `--tick`, or the script waits for another client's tick; it warns when the image and the bones come from different frames.

### Posing

`pose` reads each bone's current transform relative to its parent (`walker.get_bones()`) and adds the changes in the file, so a bone keeps its location unless the file moves it. With `--absolute`, the file holds the transforms themselves. It prints how many bones moved more than 1 cm. Moving a parent moves its children: rotating both upper arms by 70° moved 46 of the 66 bones.

### Step 4: Verify

Open one PNG. The green skeleton should sit on the rendered body. On the test run it matched at 6 m and at 3 m.

## Gotchas

- **Never send a bone with a zero location.** `set_bones` takes the transform relative to the parent, and a default `carla.Transform()` puts the bone on its parent's pivot. A first version of this script did that, and the pose did not change visibly. Leave out `location` and the bone keeps it.
- **An AI controller wins.** Per `Docs/tuto_walker_skeletons.md`, a walker driven by `controller.ai.walker` has its pose overwritten every tick. `pose` reports `0 bones moved` in that case.
- **`get_bone_relative_transforms()` is relative to the actor, not to the parent.** Only `walker.get_bones()` gives parent-relative transforms, which are what `set_bones` takes.
- **Bones work on vehicles too.** `bones --id <vehicle>` lists a vehicle's skeleton (wheels, doors); posing is for walkers only.

## Examples

**Example 1: a keypoint dataset**

User says: "record 2D pose ground truth for the pedestrians in front of the ego camera"

Switch to synchronous mode (set-world-settings), get the walker ids (world-data), then `keypoints --id <ids> --camera <ego cam> --frames 200 --tick --out dataset/`.

**Example 2: a pedestrian with raised arms**

`pose --id 26 --file arms.json` with `{"crl_arm__R": {"rotation": [0, 0, 70]}, "crl_arm__L": {"rotation": [0, 0, -70]}}`, then check it with `keypoints --frames 1`.

## Troubleshooting

**Problem: `0 bones moved`**
Cause: an AI walker controller, or `blend 0`.
Solution: use a walker spawned without `controller.ai.walker`, and `--blend 1`.

**Problem: keypoints are offset from the body**
Cause: image and bones from different frames (asynchronous mode under load).
Solution: synchronous mode with `--tick`.

## Outputs

`bones` and `draw` change nothing. `keypoints` writes JSON and PNG files under `--out`. `pose`, `blend`, `freeze` and `release` change the walker's pose until it is released or destroyed.

Bone names, the hierarchy and the projection maths are in [references/skeleton.md](references/skeleton.md).
