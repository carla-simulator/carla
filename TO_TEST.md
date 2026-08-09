# Migration Test Plans

Status legend: `[ ]` pending, `[x]` done, `[~]` partially done.

---

# DVS Sensor Migration — Test Plan

Goal: validate that `sensor.camera.dvs` on the UE5 migration branch
(`feat/dvs-sensor-migration`, worktree `carla-ue5-dvs`) behaves equivalently to
the UE4 reference implementation (`carla` repo, UE4.26).

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

---

# Light & Weather API Migration — Test Plan

Goal: validate `carla.LightManager`, `world.set_weather` /
`carla.WeatherParameters` on the UE5 migration branch
(`feat/light-weather-migration`, worktree `carla-ue5-lightweather`) against the
UE4 reference (`carla` repo, `ue4-dev`).

Branch contents: 9 night/weather commits cherry-picked from
`ue58-mapgen-features`, re-enabled `World.get_lightmanager` Python binding,
plus three new fixes (C++ enforcement of light on/off on components, day/night
flag sync, client cache dirty-marking on day/night changes). Smoke test:
`PythonAPI/examples/lightweather_test.py`.

## Branch: `feat/light-weather-migration` (UE5 — worktree `carla-ue5-lightweather`)

### Functional — lights

- [x] **L1. `get_lightmanager` exposed + enumeration.** Binding restored;
  LightManager lists lights per group (Town10: 164 Street, 119 Building;
  OpenDriveMap: 90 Street). (Passed 2026-08-09, worktree build, port 3010.)
- [x] **L2. Day/night cycle drives street lights.** `set_weather(ClearNight)`
  → all street lights on; `ClearNoon` → all off; `is_on` reflects it.
  (`lightweather_test.py` PASS: day 0/164, night 164/164.)
- [x] **L3. `turn_on` / `turn_off` from client.** State changes and Point/Spot
  light components visibly toggle (C++ enforcement; verified via difference
  heatmap `lw_diff_heat.png`).
- [x] **L4. Stale-client refresh.** A client connected before a server-side
  day/night change sees updated `is_on` without reconnecting (dirty-flag fix).
- [ ] **L5. Emissive lamp heads.** KNOWN LIMITATION: lamp-head emissive
  materials stay glowing when lights are off and feed Lumen GI (dominant on
  Town10 plaza). Needs a content/material pass driving `EmissiveIntensity`
  from the CarlaLight state — decide approach (dynamic material instances vs
  fixing the blueprint `UpdateLights` graphs).
- [ ] **L6. `set_intensity` / `set_color` / `set_light_group`.** State
  round-trips through the API, but visual effect depends on the blueprint
  `UpdateLights` graphs, which are dead in several lamp blueprints — C++ only
  enforces on/off. Verify per-map; extend C++ enforcement to intensity/color
  if needed.
- [ ] **L7. `set_day_night_cycle(False)`.** Street lights must stop following
  weather changes; manual control still works.
- [ ] **L8. Vehicle lights at night.** Headlight beams (cherry-picked
  fbcb51fa7): spawn vehicle, enable lights via
  `vehicle.set_light_state`, confirm beams illuminate road at night.
- [ ] **L9. Recorder/replayer.** Record a session with light changes, replay:
  light states restored (RecordLightChange path).
- [ ] **L10. Map change / streaming.** `is_on` and registration survive
  `load_world` and World Partition streaming (registration-flag fix
  f85c7fcee); light count stable across re-registration.

### Functional — weather

- [x] **W1. `set_weather` presets day/night.** ClearNoon / ClearNight change
  sun, sky, stars, ambient on Town10 and OpenDriveMap; weather read-back
  (`get_weather`) round-trips. (Screenshots `lw_day.png` / `lw_night.png`.)
- [ ] **W2. Individual parameter sweep.** cloudiness, precipitation +
  deposits, wind, fog (density/distance/falloff — remapped in 9d01b8583),
  wetness, scattering params, dust storm: each visibly changes the scene and
  none regresses the sky rig (UpdateNight sky-sphere fix 0253c7234).
- [ ] **W3. All presets render sanely.** Iterate every
  `WeatherParameters` preset (incl. Night variants): no black screen, no
  collapsed sky sphere, fog behaves.
- [ ] **W4. Weather + sensors.** RGB camera post-process effects respond
  (rain drops / dust wind screen materials); auto-exposure recovers after
  transitions (rt_lens exposure pin deliberately NOT cherry-picked — check
  interaction).
- [ ] **W5. Parity vs UE4.** Same preset on UE4 (`ue4-dev`) and UE5 side by
  side: sun position, fog character, precipitation look. Semantic parity,
  not pixel parity; document intentional differences (Lumen, volumetric
  clouds).

### Performance / robustness

- [ ] **PW1. Broadcast cost.** Day/night broadcast touches every registered
  light (283 on Town10) + dirty-marks all clients; measure `set_weather`
  RPC latency with many lights and >1 connected client.
- [ ] **PW2. Rapid weather changes.** 100 alternating ClearNoon/ClearNight
  calls: no leak, no light-state drift, `is_on` still consistent at the end.

## Build notes

- Wheel + editor built from the worktree
  (`cmake --preset Release -DPython3_EXECUTABLE=<anaconda python3.13>`, needs
  `CARLA_UNREAL_ENGINE_PATH` and anaconda python first on PATH for Boost).
- `StreetMap` plugin needed `EAllowShrinking` fixes for UE 5.8 — plugin is
  gitignored (vendored clone), fix is local-only in the worktree; re-apply
  after fresh setup.
- Latent unity-build include bug fixed on the branch (`CarlaEngine.cpp`,
  6fcd8abd6) — was masked on other branches by chunk layout.
