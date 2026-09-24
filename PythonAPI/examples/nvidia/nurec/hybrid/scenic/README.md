# Scenic scenarios in a NuRec scene

Scenic ([scenic-lang.org](https://scenic-lang.org), `pip install scenic`, tested with 3.1.1) controls the ego and every
other agent on the proxy road of a NuRec scene; the ego carries the recorded camera, the path-traced actor layer is
rendered by `sensor.camera.rt_lens` (+ instance / distance AOVs) attached to it, and every frame is composited into the
neural scene exactly like a `hybrid_video.py` run (`../README.md`). One command:

```sh
python hybrid_run.py --scene 7c2cf6cd --out /runs/cutin --scenic hwy_cutin.scenic
python hybrid_run.py --scene a2a4322c --out /runs/urban --scenic urban_lead_brake.scenic --scenic-param ego_speed=18 --seed 2
```

`hybrid_run.py` brings up the engine and a CARLA server, runs the illumination probe, then `hybrid_scenic.py` instead of
`hybrid_video.py`, then the composite, the harmonizers and the videos (`hybrid_final.mp4`, `hybrid_temporal.mp4`,
`compare_*.mp4`).

## What `hybrid_scenic.py` does

1. Proxy world from the artifact's OpenDRIVE, sun + sky light from the probe, exposure of the path-traced layer solved
   against the neural road on the recorded trajectory (`exposure_calib.py`) — as in `hybrid_video.py`.
2. The OpenDRIVE goes to Scenic as `OUT/map_scenic.xodr` after `scenic_xodr.py`: the exporter writes junction records
   whose incoming road is `-1` or a road inside a junction, and junctions left with no usable connection; CARLA ignores
   them, Scenic's parser divides by the number of roads. Invalid connections are dropped and empty junctions dissolved
   into plain roads; the geometry is untouched, so Scenic's lanes coincide with CARLA's proxy road.
3. The scenario is compiled with `mode2D` and the globals below, sampled with `--seed`, and simulated by a
   `CarlaSimulator` subclass that reuses the loaded world. The three `rt_lens` sensors are attached to the ego (rig origin
   on the ego's vertical axis at the recorded rig height, camera = rig × the scene's `rig_to_camera`), showing every
   other agent over the shadow catchers. After the warm-up (sensors delivering, actor set stable, cars settled) the
   declared initial speeds are applied, so the clip starts mid-drive. Each Scenic step ticks the world and saves
   `B_k.npz` with the camera pose the frame was rendered from.
4. Pass A (catchers alone) is replayed over the captured poses; the neural scene is rendered at each pose, at the
   recorded time closest to the camera along the recorded drive (the recorded actors are parked out of view).
5. `meta.json` carries the scenario, seed, parameters, termination reason and the ego track (speed, arc length along
   and lateral distance from the recorded drive per frame).

Globals the driver sets for the scenario: `ego_x`, `ego_y`, `ego_heading` (Scenic frame; lane centre of the recorded
start pose), `clip_end_x`, `clip_end_y` (recorded end pose), `clip_length_m`, `ego_speed_kmh` (recorded start speed),
`timestep`, `map`, `carla_map=None`, `use2DMap`, `snapToGroundDefault`, `weather=None` (lighting is the probe's).
`--scenic-param K=V` overrides any `param` of the scenario. A run is capped at twice the recorded drive plus 5 s
(`hybrid_scenic.py --max-seconds`), so a scenario whose ego never reaches its `terminate when` condition (say it ends up
stopped behind another car short of the route end, like `urban_pullout.scenic`, which therefore also carries a
`terminate after` time limit) does not fill the disk with frames.

## The scenarios

| file | scene | what happens |
|---|---|---|
| `hwy_cutin.scenic` | 7c2cf6cd, highway, 100 km/h, 567 m, 23 s | a faster car passes the ego on the left, cuts in a few car lengths ahead and slows; the ego brakes from 102 to 72 km/h and recovers; the lead car ahead moves left to pass a slow car that the ego then closes on; two more fast cars overtake the ego later |
| `urban_lead_brake.scenic` | a2a4322c, one-way street, 47 m, 11 s | the ego rolls behind a lead car at 20 km/h, the lead brakes to a stop, the ego stops, after a pause both pull away; a car crawls in the lane opening on the right and a car stands in the ego lane at the end, which the lead passes and the ego stops behind |
| `hwy_overtake.scenic` | 7c2cf6cd | the ego closes on a slow van, waits for a faster car coming up in the left lane to pass, pulls out behind it, overtakes the van and moves back into the right lane ahead of it; the camera leaves the recorded lane by one lane width for a few seconds |
| `hwy_brake_wave.scenic` | 7c2cf6cd | the ego follows a three-car platoon; the head car brakes hard to 40 km/h and holds it, the braking propagates back through the platoon to the ego, then everyone accelerates again while left-lane traffic keeps passing |
| `urban_pullout.scenic` | a2a4322c | a car waiting at the kerb where the street widens pulls out in front of the ego as it comes up; the ego brakes hard to its headway and follows it until both are queued behind the standing car at the end (time-limited clip) |
| `hwy_adversarial_merge.scenic` | 1370aa93, explicit main-road/ramp poses in the workspace scenario manifest | the white Model 3 follows a real right-hand on-ramp, merges ahead and slows; the Lincoln ego brakes and recovers; ten-second review; `ego_reacts=0` is a deliberately failing counterfactual |
| `hwy_construction.scenic` | 0fd2c051, parameters below | the new Model 3 yields to a graphite Model 3, moves left around a cone taper, barriers and parked Sprinter, and continues within the short reconstructed route |

The construction preset for the 173 m source route is:

```sh
python hybrid_run.py --scene 0fd2c051 --out /runs/construction --scenic hwy_construction.scenic \
  --scenic-param ego_speed=45 --scenic-param wait_speed=30 --scenic-param change_speed=35 \
  --scenic-param passer_speed=75 --scenic-param passer_start=-8 --scenic-param closure_distance=100
```

Construction props are included in the capture's show-only actor list. Their semantic labels are saved in
`meta.json: synthetic_tags` and used by the compositor; older captures retain the vehicle/pedestrian mask.
The metadata separates `cars`, `walkers`, and `props`.

The former A preset on 7c2cf6cd was a left adjacent-lane cut-in and is rejected for the right-ramp requirement.
The replacement starts ego on road 19/lane -4 and the merger on road 6/lane -1 (`OnRamp`) of 1370aa93.
Its 200-frame physics check has no collisions; ego slows from about 65 to 33 km/h. Disabling ego's response
causes vehicle contact. Use the explicit poses in `artifacts/nurec-scenarios/scenarios.json` via
`run_selected.py A`; the standalone scenario's placeholder poses are not a usable default source layout.
`carla_lane_path` follows connected CARLA waypoints selected in 3D, avoiding Scenic's ambiguity at overpasses.
Ramp transforms are read only after an initial world tick; newly spawned actors can otherwise report an origin pose.

For a verified fully overcast source, `hybrid_run.py --scenic ... --diffuse-only` uses the probe as environment
lighting with directional sun and moon disabled. This does not require inventing a sun direction when the
probe has no credible detection. The lower-level capture accepts `--sky-probe PATH.json --diffuse-only`.
Actual light intensities should be checked in the live renderer; A's review records sun=0, moon=0, sky=26000.
The probe is reconstructed, display-referred imagery, so this remains an approximate lighting match.
A uses +2.568 EV sensor exposure measured against a clear sky ROI: matching the generic proxy asphalt alone
made the visible sky 1.67 EV too bright and washed out roof reflections. The qualification harness supports
`--calibrate-sky X0,Y0,X1,Y1` for a visually verified overcast sky region, plus a diagnostic `--balance-road`
comparison. The selected exposure and reference are saved with the workspace scenario manifest.

The OpenDRIVE sanitizer reports missing lane links, zero-area drivable lanes, and bounded cubic width
undershoot corrections (maximum 2 cm; larger defects fail). It preserves CARLA's original proxy geometry;
only the copy supplied to Scenic is repaired. The 1370aa93 corrections are 3.20 and 10.83 mm.

`validate_behavior.py` runs the Scenic scenario on a dedicated CARLA server without rendering or NuRec GPU inference.
It **replaces that server's world** with the source OpenDRIVE. It accepts `--usdz`, `--scenario`, `--out`, `--port`,
`--param K=V`, and `--seed`; run with `python -O`. It saves per-frame actor poses, bounding-box footprints, speeds,
controls, lane IDs and collision events in `behavior.json`. Nonzero initial speeds are applied after settling, as in
the hybrid capture. `--sample-only` checks placement and prints rejection reasons without simulating behavior.

Scenarios end at or before the end of the recorded drive: the neural scene is only reconstructed along it — a few
metres past the end the render falls apart (`+0/+40/+80 m` past the highway end: 66/71/76 % of the pixels below 0.6
opacity). Lateral departures of a lane width or two are fine (the overtake takes the camera into the left lane); more
than that, and the reconstruction shows its holes.
Two scene facts shaped the casts: the highway's opposite carriageway lies behind a guardrail and median bushes the low
recorded camera cannot see past (composited oncoming cars get cut by the neural depth, so there are none), and the
urban reconstruction carries a red smear where the recorded car stood in traffic for most of the clip (the standing
CARLA car covers it; the temporal harmonizer otherwise turns the smear into a phantom car). Set every car's colour, and
use the UE5-native models (Lincoln MKZ, Dodge Charger, Nissan Patrol, Mini Cooper, Ford Crown taxi, Mercedes Sprinter);
the `ue4.*` blueprints are the low-detail legacy imports. Until 2026-09-08 the Lincoln and the Mini ignored the `color`
attribute (their paint slot was not named `Bodywork_Mat`, see `Util/ContentRepair/fix_vehicle_paint_slots.py`), which
made every car of the first runs black whatever the scenario asked for. Vehicle appearance also depends on the actual rendered lighting and transparent-surface masks. The September 14 qualification tests found that the sky rig could leave the directional light at its authored trajectory even though the weather API reported the requested angles. `Weather.cpp` now sets the sun component's world rotation explicitly. With the correct sun supplying direct illumination, `skymap.DEFAULT_INTENSITY` is 26000 again; the previous 160000 fill produced broad white reflections. Exposure is still solved per scene. The path-traced instance/distance AOVs now identify the first visible surface, including glazing, independently of the raster depth opacity clip; this prevents missing glass pixels in the composite.

Construction props use `prop_grounding.py`: road collision samples determine local height and slope, and measured mesh base offsets seat the qualified cones/barriers on that surface. This runs in both behavior validation and hybrid rendering and writes `prop_grounding.json`. It replaces origin-only waypoint snapping for those props. In source 0fd2c051 the latter left one barrier about 15 cm below the actual road surface.

Static qualification is not final video acceptance. Check the actor layer and composite at the actual camera distances and in motion before selecting the filming assets.

`make_reel.sh OUT.mp4 RUN:"caption" ...` cuts the harmonized clips of several runs into one captioned reel.

Do not name a behaviour `Follow`: Scenic then compiles every behaviour of the file with an empty parameter list
(`TypeError: too many positional arguments` at the first `with behavior`), an interaction with its `follow` specifier.

`nurec_lib.scenic` / `nurec_helpers.py` hold the behaviours: `LaneKeep(target_speed, leads=..., headway=..., lane=...)`
steers by pure pursuit on the lane centreline (the lane followed by its successors) with a feed-forward + PI throttle
and proportional brake, and keeps a time headway behind the nearest `leads` car ahead in its lane (within `lat_tol` m of its axis, 1.8 by default;
the pull-out scenario widens it to 3.0 so the ego reacts to the kerb car as it angles in); `Stop()`. Scenic's
own `FollowLaneBehavior` PID is tuned for ~10 m/s and spins a car at highway speed, and its `RegulatedControlAction`
never brakes (it zeroes the throttle before taking its magnitude), which is why the behaviours are local.

## Writing a scenario

### Traffic queue followed by pedestrian crossing

`urban_traffic_crossing.scenic` uses `a2a4322c`, the same source as the Mary
cinematic comparison. The ego approaches at 15 km/h and stops behind a stationary
Lincoln and Mini queue. A farther Nissan retains the existing reconstruction-ghost
cover placement. The pedestrian waits for a continuous one-second ego stop, then
crosses right to left between ego and lead vehicle at Mary's measured native
1.047794 m/s. The ego holds its brake throughout the crossing.

On a dedicated CARLA server at port 4690, from the workspace root:

```sh
.venv-rtaov/bin/python -O carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/scenic/validate_behavior.py \
  --usdz nurec_samples/sample_set/26.04_release/a2a4322c-3f99-40c3-94df-17a67f56f55d/a2a4322c-3f99-40c3-94df-17a67f56f55d.usdz \
  --scenario carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/scenic/urban_traffic_crossing.scenic \
  --out RUN --fps 24 --seconds 20
# Repeat into REPLAY, then validate both:
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/scenic/check_traffic_crossing.py RUN/behavior.json REPLAY/behavior.json REVIEW
```

The completed physics review is in
`artifacts/hybrid-cinematic-mvp/traffic_crossing/validation.json` and
`behavior-review.png`. Both runs contain 370 samples at 24 fps, with zero reported
collisions and identical actor positions/yaws. Ego travel is 10.70 m; the crossing
runs from 7.71 to 14.38 s. Minimum pedestrian-to-ego footprint clearance is 2.33 m.
The simulation uses `walker.pedestrian.german` as Mary's proxy. These measurements
validate behavior, not VFX appearance. The completed cinematic review is
`artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing/comp/review.mp4`: 370 frames
at 24 fps, 960×540. It includes the longer Mary cache, moving-camera NuRec plates,
and native vehicle cinematic tracks. See that shot's `REVIEW.md` for validation
and remaining source-reconstruction/color/lighting limitations.

### General placement

```scenic
model scenic.simulators.carla.model
from nurec_lib import *
param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param clip_end_x = 0.0
param clip_end_y = 0.0
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
ego = new Car at EGO_START, facing EGO_START.heading, with blueprint 'vehicle.lincoln.mkz', with speed 27, with behavior LaneKeep(27, leads=[lead])
lead = new Car at (follow roadDirection from EGO_START for 40), with speed 24, with behavior LaneKeep(24)
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=6.0)
```

Place agents relative to `EGO_START` (`follow roadDirection from EGO_START for D`, `offset by (lateral @ 0)`); the lateral
offsets of the neighbouring lanes are scene constants (measure them on the OpenDRIVE, both files list theirs). The ego is
`scene.objects[0]`; it is never shown to its own camera. Declared `speed`s are applied at the first frame. Run
`python -O`: Scenic's OpenDRIVE reader has parser asserts that some exported maps trip.
