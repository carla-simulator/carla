# Troubleshoot the CARLA Alpamayo integration

This page covers common installation, launch, rendering, synchronization,
control, NuRec, and recording failures. See the [Alpamayo user guide](nvidia_alpamayo.md)
for the supported workflows and [Architecture and control](nvidia_alpamayo_architecture.md)
for component semantics.

---

## Start with a bounded mock run

Before loading model weights, validate the CARLA connection, ego, cameras,
synchronization, dashboard, and controller:

```sh
cd ${CARLA_ROOT}/Examples/av_stacks/alpamayo
./run_alpamayo.sh --mock-model --max-cycles 2
```

For a machine without a display:

```sh
./run_alpamayo.sh --mock-model --no-display --max-cycles 1
```

If the mock succeeds but real inference does not, investigate the model
environment or GPU allocation rather than CARLA synchronization.

---

## Installation problems

### CARLA cannot be imported

The control interpreter must have a wheel matching its Python ABI. Build the
Python API and rerun the installer:

```sh
cd ${CARLA_ROOT}
cmake --build Build --target carla-python-api-install

cd Examples/av_stacks/alpamayo
./install_alpamayo.sh
```

Check which interpreter the launcher uses:

```sh
ALPAMAYO_CONTROL_PYTHON=/path/to/python ./run_alpamayo.sh --help
```

### Hugging Face returns 401 or 403

Both Alpamayo and the NuRec dataset are gated. Opening the repository page is
not sufficient: accept its terms with the same account used by the CLI, then
authenticate again:

```sh
hf auth whoami
hf auth login
```

For unattended installation, provide a read token through `HF_TOKEN`. Do not
put tokens in repository files or command examples committed to source control.

### `nvcc` is missing or `flash-attn` fails to build

The NVIDIA driver alone does not provide the CUDA compiler. Install CUDA
Toolkit 12.x and point `CUDA_HOME` to it:

```sh
export CUDA_HOME=/usr/local/cuda-12.8
export PATH=${CUDA_HOME}/bin:${PATH}
${CUDA_HOME}/bin/nvcc --version
```

Then rerun `install_alpamayo.sh`. The model environment is isolated from the
CARLA control Python; installing packages into only one environment does not
repair the other.

### Installation runs out of disk space

Allow approximately 67 GB for model weights. NuRec additionally needs roughly
13.3 GB of compressed image transfer, about 27.8 GB of local container layers,
one sample scene, and temporary space. Reserve at least 40 GB beyond Alpamayo.
Do not use `install_nurec.sh --full-dataset` unless approximately 1.5 TB is
available.

Inspect the major consumers before deleting anything:

```sh
docker system df
du -sh ~/.cache/huggingface ~/.cache/uv 2>/dev/null
```

---

## Launch and connection problems

### The client cannot connect to CARLA

Confirm the server and RPC port:

```sh
ss -ltn | grep ':2000'
./run_alpamayo.sh --host 127.0.0.1 --port 2000 --mock-model --max-cycles 1
```

The CARLA server must finish loading before the client connects.

### The expected map check fails

The CARLA-camera launcher never loads maps. Load the map first or correct the
validation name:

```sh
python3 ${CARLA_ROOT}/PythonAPI/util/config.py --map Town10HD_Opt
./run_alpamayo.sh --expected-map Town10HD_Opt
```

Do not pass `--expected-map` in NuRec mode. NuRec replaces the world with the
OpenDRIVE map embedded in the `.usdz`.

### The model server is unreachable

Start the persistent service and use the same endpoint on both sides:

```sh
ALPAMAYO2_CUDA_DEVICES=1,2,3 \
./run_alpamayo_model_server.sh --host 127.0.0.1 --port 8767

./run_alpamayo.sh \
    --external-model-server \
    --model-host 127.0.0.1 \
    --model-port 8767
```

Only one control client is served at a time. A disconnected client does not
unload the persistent model.

### The simulation freezes or advances twice

This usually indicates multiple synchronous tick owners. Stop standalone
`nurec_runner.py`, `manual_control.py`, or any scenario client that calls
`world.tick()`. Alpamayo must be the sole tick producer. Traffic and monitoring
clients should call `wait_for_tick()`.

---

## Camera and dashboard problems

### Camera images are noisy

The default ray-traced fisheye cameras use one sample per tick and rely on DLSS
Ray Reconstruction. Ensure DLSS is enabled and use the packaged standalone
CARLA server. Increase `--rt-lens-samples` when latency permits.

`--no-dlss` is a diagnostic option; calibrated projection remains active, but
one-sample path-traced output will contain visible noise.

### Cameras show the vehicle body or an incorrect view

Use the default `vehicle.lincoln.mkz`. For another vehicle, calibrate
`--rear-axle-x` and `--camera-body-clearance`. The source calibration is
defined in a rear-axle frame, while CARLA camera transforms are relative to the
actor origin.

The eighth tile is a CARLA-only third-person view and should show the ego. It is
not part of the Alpamayo request.

### The dashboard cannot open

Verify `DISPLAY` and Pygame access. For headless validation use `--no-display`.
Video recording cannot be combined with `--no-display` because the recorder
serializes the composited dashboard.

---

## Control diagnosis

### Target speed is zero and brake remains active

The controller derives target speed from numerical waypoint displacement, not
from reasoning text. First inspect the telemetry sidecar:

```sh
jq -c 'select(.event == "control") |
  {cycle, control_step, speed_mps_before_control, target_speed_mps,
   throttle, brake, hold_active}' outputs/run.telemetry.jsonl | head
```

Interpret the result as follows:

* A stationary numerical trajectory legitimately produces target speed zero.
* `hold_active=true` below 0.15 m/s is the intentional stationary brake hold.
* Text such as “accelerate” beside a stationary trajectory is disagreement
  between the model's language and numerical heads; the trajectory controls.
* A target that becomes zero only after a collision may indicate that the ego
  is physically stuck or the model is replanning from repeated stationary
  observations.

In NuRec, the ego should start with the recorded velocity. The integration uses
a forward pose difference at the recording's first timestamp, where a backward
difference has no interval.

### Reasoning says to keep distance, but the vehicle does not brake

Reasoning is explanatory output, not an input to the controller. Compare the
trajectory's time-aligned speed with actual speed. If the numerical trajectory
does not slow, the controller will not invent a stop that the policy did not
request.

### Steering is too weak or oscillates

Inspect `requested_steer` and `applied_steer` in telemetry. A persistent
difference indicates the steering-rate limit; a small requested value indicates
the trajectory or pure-pursuit geometry. Tune cautiously:

```sh
./run_alpamayo.sh \
    --steering-gain 1.2 \
    --max-steer-rate 1.5 \
    --lookahead 3.0
```

Changing the coordinate reflection is not a steering-tuning mechanism. A CARLA
right turn must remain positive normalized steer after conversion.

### Verify temporal control alignment

The speed index must match the control step:

```sh
jq -c 'select(.event == "control" and
  .speed_target_index != .control_step)' outputs/run.telemetry.jsonl
```

No output is expected. The steering target index may differ because lateral
control uses a spatial look-ahead.

---

## NuRec problems

### The wrong episode loads

Set an absolute path and verify it before launching:

```sh
export NUREC_USDZ=/absolute/path/to/<scene_uuid>.usdz
test -f "${NUREC_USDZ}"
./run_alpamayo_nurec.sh --external-model-server
```

### NRE repeatedly reloads or reuses the wrong configuration

Container reuse verifies the scene, render flags, service version, and GPU
selection. A change to Harmonizer, renderer, scene, or GPU should start a
matching container. Inspect active services with:

```sh
docker ps --filter name=NuRec
```

Use `--stop-nurec-container` when the service should be removed on normal exit.

### Background traffic hits the ego

Rigid recorded trajectories do not react when the policy-controlled ego slows
or leaves the source trajectory. Use one of:

```sh
# Isolated ego evaluation.
./run_alpamayo_nurec.sh --nurec-disable-traffic

# Reactive background vehicles where valid routes are available.
./run_alpamayo_nurec.sh --nurec-no-replay-traffic
```

The second mode holds close same-lane rear traffic until a safe gap opens and
uses collision-aware Traffic Manager. Some route-less proxies remain held.

### Harmonizer is slow

Harmonization adds neural rendering work. Reduce native NRE camera resolution
while preserving the dashboard/model resize:

```sh
./run_alpamayo_nurec.sh \
    --nurec-harmonizer \
    --nurec-resolution-ratio 0.25
```

Use `nvidia-smi` to confirm that NRE and CARLA fit on their assigned GPU.

---

## Recording problems

### The video contains long frozen inference pauses

Record with the built-in `--record-video` option. External screen recorders
capture wall time and therefore capture inference stalls; the built-in recorder
uses simulation time and omits those pauses.

### The MP4 is missing or cannot be decoded

Allow the process to handle ESC, Q, or Ctrl-C so the writer can finalize the
container. Validate the result with:

```sh
ffprobe -v error -select_streams v:0 \
    -show_entries stream=codec_name,width,height,r_frame_rate,nb_frames \
    -show_entries format=duration,size \
    outputs/run.mp4

ffmpeg -v error -i outputs/run.mp4 -f null -
```

The installer supplies `imageio-ffmpeg` when a system `ffmpeg` binary is not
available.

---

## Collect a useful bug report

Include:

* CARLA commit and server build type.
* Exact launcher command and episode UUID or map.
* GPU allocation and `nvidia-smi` output.
* Whether DLSS and NuRec Harmonizer were enabled.
* The run log and telemetry JSONL.
* A short MP4 or screenshot when the problem is visual.
* The first collision event, if any.

Do not include Hugging Face tokens, GitHub credentials, model weights, NuRec
artifacts, or other gated content.
