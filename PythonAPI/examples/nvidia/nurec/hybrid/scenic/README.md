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

All end when the ego reaches the end of the recorded drive: the neural scene is only reconstructed along it — a few
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
made every car of the first runs black whatever the scenario asked for. Two more things decide how the cars look under
the path tracer (both landed the same day): the car-paint master (`M_CarPaintMaster`, a clear-coat metal) carries
roughness values up to 1.6 on its instances, which the raster path clamps but the path tracer integrates literally
(a metal that reflects nothing), so the master now clamps the roughness to 0.2 through a `PathTracingQualitySwitch`
(`Util/ContentRepair/fix_carpaint_pathtracing.py`; raster untouched); and the sky light's default intensity moved from
26000 to 160000 (`skymap.DEFAULT_INTENSITY`): under the exposure that matches the road to the neural road, the neural
sky sits at ~6x the road's luminance and the old sky light at ~1x, so car panels — mirrors of the sky — came out 3-6x too
dark. A silver car's side went from 0.01 to 0.21 of the road luminance (median; 0.54 at p90) with both, the cast
shadows kept their depth. Sweep with `hybrid_run.py --skymap-intensity`.

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
