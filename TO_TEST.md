# DVS Sensor Migration — Test Plan

Goal: validate that `sensor.camera.dvs` on the UE5 migration branch
(`feat/dvs-sensor-migration`, worktree `carla-ue5-dvs`) behaves equivalently to
the UE4 reference implementation (`carla` repo, UE4.26).

Status legend: `[ ]` pending, `[x]` done, `[~]` partially done.

---

## Branch: `carla` (UE4 reference — baseline capture)

Purpose: produce reference data. Every test here generates a baseline artifact
that the UE5 branch is compared against.

- [ ] **B1. Baseline event stream.** Fixed scene (Town10, fixed weather, fixed
  spawn point), deterministic ego motion (scripted control, sync mode,
  `fixed_delta_seconds=0.05`), default DVS attributes, 640x480. Record all
  event packets for 100 ticks → `baseline_ue4_default.npz`.
- [ ] **B2. Grayscale input curve.** Same scene, spawn an RGB camera with
  identical pose/resolution alongside the DVS camera. Save RGB frames →
  establishes the luminance input the DVS simulator sees on UE4
  (tone curve / gamma reference for parity check T1).
- [ ] **B3. Attribute sweep baseline.** Repeat B1 for:
  - `positive_threshold` / `negative_threshold` ∈ {0.1, 0.3, 0.7}
  - `refractory_period_ns` ∈ {0, 1_000_000}
  - `use_log` ∈ {true, false}
  - `sigma_positive_threshold` = 0.1 (statistical comparison only, seeded RNG)
- [ ] **B4. Sensor tick.** `sensor_tick=0.1` → confirm packet cadence halves
  relative to 0.05 world tick; record cadence.

## Branch: `feat/dvs-sensor-migration` (UE5 — worktree `carla-ue5-dvs`)

### Functional (server can be any build sharing the DVS code)

- [x] **F1. Spawn + stream smoke test.** Blueprint present, sensor spawns,
  events received, coords in bounds, timestamps sorted, both polarities.
  (`dvs_test.py`, passed 2026-08-09 against running server, 42 packets /
  ~835k events.)
- [x] **F2. Python decode path.** numpy structured dtype
  `(x u2, y u2, t i8, pol ?)` decodes raw buffer; visualization sane.
  (`dvs_test_out.png`.)
- [ ] **F3. Build from migration branch.** Rebuild LibCarla + CarlaUnreal
  plugin from this exact branch; rerun F1 against that build.
- [ ] **F4. Sync mode.** Sync + `fixed_delta_seconds`: one packet per tick,
  event timestamps monotone across packets, `delta_t_ns` consistent with the
  fixed delta.
- [ ] **F5. Attribute effects.** Each attribute in B3 changes output in the
  expected direction (higher threshold → fewer events; refractory period →
  per-pixel event rate capped; `use_log=false` → different event distribution).
- [ ] **F6. `manual_control.py` interactive.** Launch example, cycle sensors to
  "Dynamic Vision Sensor", confirm live event rendering + no crash on sensor
  switch away/back (restored in `7d2f6c57c`).
- [ ] **F7. Resolution / FOV variations.** 1920x1080 and fov=120: no OOB
  events, no aspect distortion in event image.
- [ ] **F8. Lifecycle.** `stop()` → `listen()` again, destroy while streaming,
  respawn loop x10: no server crash, no stale state (prev_image reset).
- [ ] **F9. DVS + wide-angle/fisheye interplay.** DVS inherits lens distortion
  post-process material; verify distortion attributes (`lens_k`, etc.) apply
  and don't corrupt event coords.
- [ ] **F10. ROS2 path** (only if `WITH_ROS2` build available): DVS publisher
  emits on expected topic, message content matches stream data.

### Parity vs UE4 (needs both servers, same scene recipe)

- [ ] **T1. Luminance input parity.** Compare B2 RGB frames vs UE5 RGB frames
  (same scene recipe). If tone curves differ (UE5 `SCS_FinalToneCurveHDR`),
  quantify grayscale delta — this predicts event-statistics drift. Decide:
  acceptable, or needs capture-source adjustment in `DVSCamera`.
- [ ] **T2. Event statistics parity.** Same scenario as B1 on UE5: compare
  event count per tick, polarity ratio, spatial event density histogram
  against `baseline_ue4_default.npz`. Exact match impossible (different
  renderer); define tolerance (suggest: event count within ±20%, polarity
  ratio within ±10%).
- [ ] **T3. Threshold semantics parity.** B3 sweep on UE5: verify the
  *relative* effect of each attribute matches UE4 (monotonicity + rough
  magnitude), even if absolute counts differ.

### Performance

- [ ] **P1. Tick cost.** Measure `ADVSCamera::PostPhysTick` time (CPU trace
  scope exists) at 640x480 and 1920x1080. The per-pixel simulation loop is
  single-threaded on game thread — compare vs UE4 numbers, flag if 1080p
  blocks the frame > ~10 ms.
- [ ] **P2. Multi-DVS.** 3 concurrent DVS sensors: frame rate, no readback
  contention with other camera sensors (shared `FRHIGPUReadbackPool`).

---

## Test infrastructure notes

- UE5 client env: `.venv-dvs/` (python 3.13, carla 0.10.0 wheel, numpy,
  pillow). Smoke test: `dvs_test.py`.
- UE4 side needs its own client (0.9.x egg/wheel from `carla/PythonAPI/`) and
  a running UE4 server — not currently built/running on this machine.
- Deterministic scene recipe (shared by B1/T2): sync mode, fixed seed for TM,
  scripted ego (no autopilot randomness), fixed weather, same spawn index.
