# NVIDIA Alpamayo 2 Super integration for CARLA UE5

`run_alpamayo.sh` is a synchronous, closed-loop driving demo for NVIDIA
Alpamayo 2 Super. It captures the seven-camera PhysicalAI-AV sensor ring and
1.6 seconds of ego-motion history, runs trajectory inference, displays the six
trajectory cameras, predicted path, Chain of Causation reasoning, and control
telemetry, then tracks a one-second prefix of the prediction in CARLA.

The demo attaches to the world that is already running. It does **not** load a
map or remove actors, so a separate script can select the map and populate
traffic before the demo starts. The optional NuRec backend is the exception:
it deliberately loads the OpenDRIVE world embedded in a recording artifact.

Published CARLA documentation is available in the
[Alpamayo user guide](../../../../Docs/nvidia_alpamayo.md),
[architecture and control reference](../../../../Docs/nvidia_alpamayo_architecture.md),
and [troubleshooting guide](../../../../Docs/nvidia_alpamayo_troubleshooting.md).

## Requirements

- A built and running CARLA UE5 server and its Python wheel (Python 3.10+)
- Linux, NVIDIA drivers, and CUDA Toolkit 12.x including `nvcc`
- Enough NVIDIA GPU memory for Alpamayo 2 Super; keep the CARLA rendering GPU
  out of `--model-gpus`
- Access to the gated
  [`nvidia/Alpamayo2-Super`](https://huggingface.co/nvidia/Alpamayo2-Super)
  model after accepting its license
- About 67 GB for the model weights, plus the Python environment and caches

The CARLA process and control UI use the normal CARLA Python environment.
Inference runs in a separate Python 3.12 process because Alpamayo's locked
environment requires it. A small, pickle-free TCP protocol connects them.

## Install

From this directory:

```sh
./install_alpamayo.sh
```

The installer:

1. installs the lightweight UI/control and video-recording dependencies;
2. clones or updates the official
   [`NVlabs/alpamayo2`](https://github.com/NVlabs/alpamayo2) source;
3. installs its locked Python 3.12 environment with `uv`;
4. authenticates with Hugging Face if necessary; and
5. downloads the gated model weights.

Set `HF_TOKEN` for non-interactive authentication. Use
`./install_alpamayo.sh --no-model-download` to install the code without
downloading weights. Run `./install_alpamayo.sh --help` for path overrides.

## Run on the current map

Start CARLA and load the map first. For example, with Town10 already active:

```sh
./run_alpamayo.sh \
    --expected-map Town10HD_Opt \
    --ego-mode auto \
    --model-gpus 1,2,3
```

`--ego-mode auto` attaches to the oldest vehicle whose `role_name` is `hero`,
or spawns its own hero if none exists. `--ego-mode attach` waits for an
externally created hero and never spawns one. `--ego-mode spawn` always creates
and later destroys a demo-owned ego vehicle.

The default is continuous closed-loop operation. Press ESC or Q to stop, or
use `--max-cycles N` for a bounded run. Useful validation modes are:

```sh
# Exercise the complete camera/UI/control loop without loading model weights.
./run_alpamayo.sh --mock-model --max-cycles 2

# Bypass DLSS while retaining the calibrated path-traced camera model.
./run_alpamayo.sh --no-dlss

# Run inference and visualization but do not drive the vehicle.
./run_alpamayo.sh --observe-only

# Headless automation.
./run_alpamayo.sh --no-display --max-cycles 1

# Save the first dashboard frame that includes a prediction.
./run_alpamayo.sh --screenshot alpamayo-dashboard.png --max-cycles 1

# Record the complete interface without adding wall-clock inference pauses.
./run_alpamayo.sh \
    --record-video outputs/alpamayo-town10.mp4 \
    --record-fps 20
```

The dashboard uses two rows of four views: the seven canonical Alpamayo source
cameras plus a CARLA-only third-person chase camera. The chase camera is never
included in the model request. The lower half contains a dedicated trajectory
plot and one unified inference panel for reasoning, state, and controls.

Recording encodes the fully composited 1600×900 interface to H.264 MP4 in
**simulation time**. Each synchronized 0.1-second CARLA step contributes one
video state; dashboard redraws while Alpamayo inference has the simulation
paused are deliberately omitted. The result plays continuously without frozen
five-to-seven-second inference gaps. At 20 FPS, each 10 Hz simulation state is
encoded twice to preserve real-time speed. The installer supplies
`imageio-ffmpeg` as a fallback when a system `ffmpeg` binary is unavailable.
The file is finalized when the demo exits normally or is stopped with
ESC/Q/Ctrl-C.

Video recording also writes `outputs/alpamayo-town10.telemetry.jsonl`. The
sidecar contains every raw Alpamayo trajectory plus each requested and
rate-limited CARLA steering command. Use `--telemetry-log PATH` to record this
diagnostic stream without video or to choose another path.

## Use with map and traffic initialization scripts

Only one client may own synchronous ticks. Let the initializer load the map
and create actors, then leave those actors alive. Run long-lived traffic
clients in asynchronous/wait-for-tick mode while `run_alpamayo.sh` owns
`world.tick()`. For CARLA's `generate_traffic.py`, that looks like:

```sh
python3 ../generate_traffic.py \
    --asynch \
    --tm-port 8001 \
    --number-of-vehicles 40 \
    --number-of-walkers 20

./run_alpamayo.sh \
    --expected-map Town10HD_Opt \
    --ego-mode attach \
    --traffic-manager-port 8001 \
    --restore-autopilot
```

Use the same Traffic Manager port in both commands. While active, the demo sets
the existing world to synchronous mode with a 0.1-second fixed step and puts
that Traffic Manager into synchronous mode. On exit it restores the original
world settings. It destroys only its seven sensors and an ego vehicle that it
spawned itself; externally initialized traffic is left untouched.

If another orchestrator already manages the Traffic Manager, pass
`--no-manage-traffic-manager`. That orchestrator must still avoid competing
calls to `world.tick()`.

## Keep the model warm

Loading the model is expensive. Start it once and connect multiple demo runs:

```sh
ALPAMAYO2_CUDA_DEVICES=1,2,3 ./run_alpamayo_model_server.sh

./run_alpamayo.sh --external-model-server
```

The server defaults to `127.0.0.1:8767`. The control script auto-starts and
stops a server when `--external-model-server` is omitted; add
`--keep-model-server` to leave that subprocess warm after the client exits.

## Control conversion

Alpamayo predicts 64 ego-frame XYZ waypoints over 6.4 seconds in a
right-handed frame (`x` forward, `y` left). CARLA uses a left-handed frame, so
the bridge reflects the lateral coordinate. Lateral and longitudinal control
use independent trajectory indices: pure pursuit selects a spatial look-ahead
point only for steering, while control step `i` derives its target speed from
the displacement over Alpamayo's matching 0.1-second trajectory interval
`i`. A stateful longitudinal PID converts that time-aligned speed error into
throttle or brake with anti-windup and derivative-on-measurement. Predicted
speeds at or below `--stop-target-speed` engage a minimum stopping brake and
then `--hold-brake` below `--hold-speed`. The controller executes ten
0.1-second steps by default, captures fresh synchronized observations, and
replans.

The PID defaults are `--speed-kp 0.35`, `--speed-ki 0.04`, and
`--speed-kd 0.02`; the former `--speed-gain` spelling remains an alias for
`--speed-kp`. Raw telemetry distinguishes `steering_target_index` from
`speed_target_index` and records the pre-control speed, speed error, signed
longitudinal effort, and hold state for auditing.

The default `--steering-gain 1.0` maps the requested wheel angle directly to
the MKZ's normalized steering range. `--max-steer-rate 1.0` limits abrupt
changes to 0.1 normalized steering units per 10 Hz simulation step. Both can
be tuned per vehicle blueprint without changing the Alpamayo/CARLA coordinate
conversion.

This is a research demo, not a safety controller. It does not add collision
avoidance beyond behavior represented by the model's predicted trajectory.

## Sensor calibration

The default ego is `vehicle.lincoln.mkz`, whose length, height, and wheelbase
closely match the PhysicalAI-AV source vehicle. Camera translations are the
public calibration coordinates in the source rear-axle frame, converted to the
MKZ actor frame with the calibrated 1.3535 m rear-axle-to-box-center offset.
Full roll, pitch, and yaw are retained when converting the source optical frame
to CARLA's vehicle frame. The stored angles are NVIDIA's FLU sensor-to-rig
values (DriveWorks `nominalSensor2Rig_FLU`) named for the direction in which
they are positive: `pitch_up_deg` is positive up, `yaw_left_deg` positive left,
`roll_right_up_deg` positive with the right side rising. `CameraSpec.carla_pose`
turns them into CARLA's left-handed frame as `(pitch_up_deg, -yaw_left_deg,
-roll_right_up_deg)`; the pitch and roll signs were verified against the live
server on 2026-08-28 (see `tests/test_rig.py`).

The MKZ roof intersects the two calibrated forward-center camera locations.
Those cameras retain their calibrated horizontal position and orientation, but
their height is clamped to 8 cm above the actual vehicle bounding box. Adjust
this clearance with `--camera-body-clearance` when using another vehicle.

The source F-theta polynomials and sub-pixel Kannala-Brandt fits are retained in
`alpamayo_rig.py`. By default, the seven model views use the UE58
`sensor.camera.rt_lens` sensor with `camera_model=kannala_brandt`. This evaluates
the fitted lens per ray rather than warping six rasterized cubemap faces. Its
path-tracing denoiser is explicitly enabled; `DefaultEngine.ini` selects NVIDIA
DLSS Ray Reconstruction, and `--rt-lens-samples` controls the samples rendered
before DLSS-RR (default: 1 for closed-loop operation).

Pass `--no-dlss` to disable both the `rt_lens` denoiser and chase-camera
DLSS-SR. The seven cameras still use the same calibrated path-traced
Kannala-Brandt projection, but one-sample output will contain path-tracing
noise.

`--camera-projection cubemap` selects the older
`sensor.camera.rgb_fisheye`, while `--camera-projection pinhole` is the basic
diagnostic fallback. Seven simultaneous 1080p cubemap cameras triggered a
GPU-readback assertion in UnrealEditor, so multi-camera testing should use the
packaged standalone CARLA server. The eighth CARLA chase view always uses
`sensor.camera.rgb` with DLSS-SR and is not sent to Alpamayo.

When attaching to a non-MKZ hero, calibrate `--rear-axle-x` and verify that the
side cameras sit outside the body mesh; otherwise the vehicle can occlude part
of their images.

## Tests

```sh
python3 -m unittest discover -s tests -v
```

The tests cover the transport, request schema, coordinate handedness, camera
ring, and trajectory-to-control conversion without requiring a running model.

## NuRec camera backend

NuRec mode reconstructs the recorded scene and supplies Alpamayo's camera
history from NVIDIA NRE instead of CARLA camera actors. Install the NuRec
example first:

```sh
cd ../../nvidia/nurec
./install_nurec.sh
```

The installer uses a local `.venv`, pins
`nvcr.io/nvidia/nre/nre-ga:26.04.01`, and downloads one gated sample (about
1.86 GiB). Accept the PhysicalAI Autonomous Vehicles NuRec dataset terms and
run `hf auth login`, or set `HF_TOKEN`, before installation. The container is
about 13.3 GiB compressed and 27.8 GiB locally; reserve at least 40 GiB for the
image, sample, Python environment, and temporary data. Do not use
`--full-dataset` unless roughly 1.5 TB is intentional.

With CARLA running, launch the unified orchestrator from this directory:

```sh
./run_alpamayo_nurec.sh \
    --nurec-gpu 0 \
    --model-gpus 1,2,3
```

Set `NUREC_USDZ=/absolute/path/to/scene.usdz` to select another artifact. The
launcher uses NuRec's Python environment while Alpamayo inference remains in
the separate model service. If that service is already running, add
`--external-model-server --model-port <port>` instead of starting another one.
Add `--nurec-no-replay-traffic` when Alpamayo controls the ego. It disables
rigid non-ego trajectory replay and hands usable moving vehicles to CARLA's
Traffic Manager with vehicle avoidance and a speed-dependent two-second gap
(minimum 10 m). Background vehicles start with zero velocity; same-lane rear
traffic waits collision-free until the ego opens the required gap. This avoids
injecting a recorded vehicle behind an ego that has slowed or diverged from the
source take. `--nurec-actors carla` is an equivalent advanced spelling; the
default `replay` mode remains available for faithful dataset playback. NuRec
tracks that cannot accept pose overrides remain neural-replay-only and their
CARLA proxies are non-colliding. Controllable vehicles without a usable TM
route are held as stationary physics obstacles; walkers remain on recorded
motion. An ego collision sensor writes attributed `collision` events to the
telemetry JSONL. The handoff uses `--traffic-manager-port` (default `8001`),
just like the standard CARLA backend. `--stop-nurec-container` tears down NRE
on exit; otherwise the verified container stays warm and is reused.

Do not run `nurec_runner.py` alongside this mode. `NurecScenario` replaces the
current map and configures a 20 Hz synchronous world, while its neural cameras
run at 10 Hz. `run_alpamayo_nurec.sh` owns that scenario and is the only process
that calls `world.tick()`, preventing double ticks and temporal skew. Traffic
or scenario setup clients may attach after the generated world exists, but
must wait for ticks rather than advancing the world themselves.

The public recording provides exactly the six cameras used by Alpamayo's
trajectory profile. Alpamayo's canonical input validator still requires a
seventh rear-tele context slot, even though the profile discards it. The bridge
fills that slot with an explicitly identified duplicate of the synchronized
rear-left neural view. The eighth dashboard tile remains a standard CARLA
third-person camera and is never sent to the model. DLSS stays enabled for that
CARLA chase view; the six neural views are rendered by NRE and do not depend on
CARLA's DLSS path.
