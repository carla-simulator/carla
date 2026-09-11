# Implementation notes — CARLA API behaviour observed during Phase 1

Evidence gathered against this worktree's build (ue58-dev @ 62f83f49a) running
Town10HD_Opt.  These complement the Phase 0 findings
(`.omc/research/cosmos/phase0_findings.md`).

## 1. `get_transform()` is the identity until the actor's first snapshot

Right after `world.try_spawn_actor(...)` (before the world observer publishes
a frame containing the new actor), `actor.get_transform()` returns the zero
transform while `get_vehicle_bone_world_transforms()` already returns world
poses:

```
fresh actor at Location(x=0, y=0, z=0)
  Wheel_Rear_Right world (107.2, -11.31, 0.87) local [107.2 -11.31 0.87]   # wrong frame!
after one tick: actor at (106.42, -12.71, 0.12)
  Wheel_Rear_Right local [-1.397  0.796  0.271]                            # correct
```

Consequence: computing the rear-axle offset from wheel bones without ticking
first silently yields world coordinates.  `carla_cosmos.rig.rear_axle_local_ue`
now detects the inconsistency (axle outside the bounding box + 1 m) and raises
with instructions; `Capture.run` ticks once before measuring.

## 2. `world.get_level_bbs(Car/...)` includes *spawned* vehicle actors

On an empty Town10HD_Opt, `get_level_bbs(CityObjectLabel.Car)` returns 47
boxes (the parked static vehicles).  After spawning 11 vehicles the demo's
dedup matched 10 level boxes to live actors (one spawned two-wheeler falls
under Motorcycle/Bicycle labels).  So level bbs are *not* purely static
scenery; `SceneExporter._export_static_obstacles` deduplicates against live
actors by centre distance (2 m) before emitting constant-pose obstacles.

## 3. Suspension settles for a few centimetres after spawn

Anchoring the FLU world frame one tick before the first captured frame left
the first ego pose at z = −0.054 m (the car still settling on its suspension).
The frame anchor therefore happens on the first *captured* tick, making the
first `egomotion_estimate` row exactly the origin.

## 4. Confirmations of Phase 0 findings (no change needed)

* `WheelPhysicsControl.location` is `(0,0,0)` for every wheel → wheel bones +
  bbox bottom used instead (`Wheel_Rear_Left/Right`, Lincoln MKZ axle at
  x = −1.397 m, ground z ≈ 0 in the actor frame).
* Iterating `blueprint_library.filter("vehicle.*")` and reading attributes can
  raise bare `RuntimeError` for some entries → guarded with `has_attribute`
  and try/except in the demo.
* Walker navmesh spawns need `z += 1.0`.
* Some junction lane connectors carry `Solid` markings → junction waypoints
  are skipped in `lane_line` (both whole segments and per-point).
* NVIDIA's rig translations put `front_wide` inside a CARLA sedan cabin →
  roofline mounting rule (documented in `client/carla_cosmos/rig.py`), actual
  extrinsics written to the calibration.  Verified: 1.443 m → 1.574 m for the
  Lincoln MKZ; NVIDIA's `generate_control_videos.py` render of the exported
  scene overlays the CARLA RGB correctly (cuboids on the parked/moving
  vehicles, lane lines on the paint).

## 5. Encoder choice for control videos

Controls are encoded as H.264 4:4:4 in lossless mode (`libx264 -qp 0
-pix_fmt yuv444p`).  Verified round-trip exact (unit test decodes flat palette
colours back bit-identically within ±1).  FFV1 would be mathematically
lossless but is not reliably decodable from `.mp4` by the readers the Cosmos
backends use (imageio-ffmpeg / PyAV / decord), so H.264 4:4:4 was chosen; RGB
uses `-crf 14 yuv420p`.

## 6. Depth normalisation and the ego hood

Depth controls use per-clip min–max normalised *inverse* depth (near = bright),
matching the Video-Depth-Anything distribution the Transfer 2.5 / Cosmos 3
depth branches were trained on.  With a roofline-mounted forward camera the
ego hood is the nearest surface and takes the top of the range — same as real
AV footage, but worth remembering when interpreting depth histograms
(demo clip: inverse-depth range 0.001–1.079 1/m, i.e. 0.93 m hood to 1 km sky).
