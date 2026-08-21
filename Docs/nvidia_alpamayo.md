# Run NVIDIA Alpamayo 2 Super in CARLA

CARLA UE5 provides a synchronous, closed-loop example for NVIDIA Alpamayo 2
Super. The integration captures the model's calibrated camera and ego-motion
history, runs trajectory inference, displays the predicted trajectory and
Chain of Causation reasoning, and converts the numerical trajectory into CARLA
vehicle controls.

The example supports two sensor backends:

* **CARLA cameras** attach to a world that is already running. This mode works
  with named CARLA maps, independently initialized traffic, and a spawned or
  externally managed ego vehicle.
* **NuRec cameras** load the OpenDRIVE world embedded in a NuRec recording and
  render its calibrated camera views with NVIDIA NRE. CARLA still owns vehicle
  physics and the simulation clock.

The user-facing launchers are in
[`Examples/av_stacks/alpamayo`](../Examples/av_stacks/alpamayo/):

| Command | Purpose |
|---|---|
| `install_alpamayo.sh` | Install the control-side dependencies and isolated model environment. |
| `run_alpamayo.sh` | Run closed-loop Alpamayo with CARLA-rendered cameras. |
| `run_alpamayo_model_server.sh` | Keep the model loaded between multiple demo runs. |
| `run_alpamayo_nurec.sh` | Run the unified NuRec and Alpamayo loop. |

The shell scripts are the public interface. The Python files in the same
directory are implementation modules and can still be imported for research or
testing.

For implementation details, see [Architecture and control](nvidia_alpamayo_architecture.md).
For operational problems, see [Troubleshooting](nvidia_alpamayo_troubleshooting.md).

---

## Requirements

Before installing the integration, prepare:

* Linux and a built CARLA UE5 server with its Python wheel.
* Python 3.10 or later for the CARLA control process.
* CUDA Toolkit 12.x, including `nvcc`, for the Alpamayo model environment.
* NVIDIA drivers compatible with the selected CUDA toolkit.
* Access to the gated
  [NVIDIA Alpamayo 2 Super model](https://huggingface.co/nvidia/Alpamayo2-Super)
  after accepting its license.
* Approximately 67 GB for model weights, in addition to the Python environment
  and download caches.

NuRec mode additionally requires Docker, NVIDIA Container Toolkit, access to
the gated
[PhysicalAI Autonomous Vehicles NuRec dataset](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec),
and at least 40 GB for one sample, the pinned NRE image, its expanded layers,
and temporary data.

Multi-GPU systems are strongly recommended. The examples below reserve GPU 0
for CARLA and NRE, and GPUs 1, 2, and 3 for Alpamayo. Adjust the assignments to
match the available hardware, but do not expose the CARLA rendering GPU to an
automatically started model process unless its remaining memory is known to be
sufficient.

---

## Install Alpamayo

Set `CARLA_ROOT` to the source checkout and authenticate with Hugging Face:

```sh
export CARLA_ROOT=/path/to/carla
hf auth login
```

After accepting the model license, run:

```sh
cd ${CARLA_ROOT}/Examples/av_stacks/alpamayo
./install_alpamayo.sh
```

The installer performs the following actions:

1. Installs the lightweight CARLA-side UI, NumPy, and video dependencies.
2. Finds and installs the matching built CARLA Python wheel when necessary.
3. Installs `uv` when it is unavailable.
4. Clones or updates the official `NVlabs/alpamayo2` source.
5. Creates an isolated Python 3.12 model environment from the upstream lockfile.
6. Downloads `nvidia/Alpamayo2-Super` through the authenticated Hugging Face
   session.

For non-interactive environments, set `HF_TOKEN` before running the installer.
To prepare the environment without downloading model weights, use:

```sh
./install_alpamayo.sh --no-model-download
```

Run `./install_alpamayo.sh --help` to override the control interpreter, source
checkout, model environment, or model identifier.

### Install the optional NuRec backend

The NuRec example remains under the NVIDIA integration directory because it is
also usable independently of Alpamayo:

```sh
cd ${CARLA_ROOT}/PythonAPI/examples/nvidia/nurec
./install_nurec.sh
```

The installer creates a local `.venv`, installs the CARLA wheel and NuRec
dependencies, pulls the pinned `nvcr.io/nvidia/nre/nre-ga:26.04.01` image, and
downloads one sample scene. Use `--scene <uuid>` to select a different sample.
Do not use `--full-dataset` unless a download of approximately 1.5 TB is
intentional.

---

## Start CARLA

Start a packaged development build on GPU 0:

```sh
cd ${CARLA_ROOT}
CUDA_VISIBLE_DEVICES=0 \
./Build/Package/Carla-0.10.0-Linux-Development/Linux/CarlaUnreal.sh \
    -carla-rpc-port=2000
```

Equivalent packaged or locally built `CarlaUnreal` executables can be used.
Add `-RenderOffScreen` for headless automation; an X11 or Wayland display is
still required when the Alpamayo dashboard itself is enabled.

---

## Run with CARLA cameras

The CARLA-camera backend never loads a map. Load the desired map before the
demo starts. For Town10:

```sh
python3 ${CARLA_ROOT}/PythonAPI/util/config.py \
    --host localhost \
    --port 2000 \
    --map Town10HD_Opt
```

Then launch Alpamayo:

```sh
cd ${CARLA_ROOT}/Examples/av_stacks/alpamayo
./run_alpamayo.sh \
    --expected-map Town10HD_Opt \
    --ego-mode auto \
    --model-gpus 1,2,3
```

`--expected-map` validates the current world but does not change it. Ego modes
are:

| Mode | Behavior |
|---|---|
| `auto` | Attach to the oldest vehicle with the requested role, or spawn an MKZ. |
| `attach` | Wait for an externally created hero and never spawn one. |
| `spawn` | Always spawn and later destroy a demo-owned MKZ. |

The default role is `hero`. The default vehicle is `vehicle.lincoln.mkz`, which
best matches the reference sensor geometry.

Press **ESC** or **Q** to stop. `--max-cycles N` is useful for bounded runs.

### Keep the model loaded

Loading the model for every client run is expensive. Start a persistent model
service in one terminal:

```sh
cd ${CARLA_ROOT}/Examples/av_stacks/alpamayo
ALPAMAYO2_CUDA_DEVICES=1,2,3 ./run_alpamayo_model_server.sh
```

Connect one or more sequential clients from another terminal:

```sh
./run_alpamayo.sh --external-model-server --expected-map Town10HD_Opt
```

The service listens on `127.0.0.1:8767` by default. Use matching
`--model-host` and `--model-port` arguments when changing that endpoint.

### Compose with traffic initialization

Only one client may advance synchronous simulation. A traffic or scenario
initializer may load the map and create actors, but it must not continue
calling `world.tick()` while Alpamayo is active.

For CARLA's traffic generator:

```sh
python3 ${CARLA_ROOT}/PythonAPI/examples/generate_traffic.py \
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

Use `--no-manage-traffic-manager` only when another orchestrator owns Traffic
Manager synchronization. That orchestrator must wait for ticks instead of
producing them.

---

## Run with NuRec cameras

NuRec mode deliberately replaces the current world with the OpenDRIVE network
embedded in the selected `.usdz`. Do not load Town10 or start the standalone
`nurec_runner.py` beside this mode.

With CARLA and the optional persistent model server running:

```sh
cd ${CARLA_ROOT}/Examples/av_stacks/alpamayo
export NUREC_USDZ=/absolute/path/to/scene.usdz

./run_alpamayo_nurec.sh \
    --external-model-server \
    --nurec-gpu 0 \
    --nurec-harmonizer \
    --nurec-resolution-ratio 0.25 \
    --nurec-disable-traffic
```

Omit `--external-model-server` and add `--model-gpus 1,2,3` to let the launcher
start and stop the model service for this run.

NuRec traffic policies are:

| Policy | Option | Intended use |
|---|---|---|
| Recorded replay | Default, or `--nurec-actors replay` | Faithful dataset playback; unsafe after the ego diverges from the recorded trajectory. |
| CARLA traffic | `--nurec-no-replay-traffic` | Hand usable vehicles to collision-aware Traffic Manager and hold route-less proxies. |
| Disabled traffic | `--nurec-disable-traffic` | Hold non-ego proxies collision-free; recommended for isolated policy evaluation. |

`--nurec-harmonizer` enables NRE appearance harmonization. It improves visual
consistency when actors diverge from the recorded state, but increases neural
rendering latency. NRE containers remain warm and reusable after exit by
default; pass `--stop-nurec-container` to remove the active container.

---

## Record the dashboard and telemetry

Add `--record-video` to either driving launcher:

```sh
./run_alpamayo.sh \
    --external-model-server \
    --record-video outputs/alpamayo-town10.mp4 \
    --record-fps 20
```

```sh
./run_alpamayo_nurec.sh \
    --external-model-server \
    --nurec-disable-traffic \
    --record-video outputs/alpamayo-nurec.mp4 \
    --record-fps 20
```

Recording uses simulation time. Model inference pauses do not become frozen
wall-clock sections in the MP4. At the default 20 FPS, each 10 Hz simulation
state is encoded twice. A sidecar with the same basename and the suffix
`.telemetry.jsonl` records predictions, control commands, and collisions.

Use `--telemetry-log PATH` to write telemetry without video or to select a
different sidecar path. Video recording requires the dashboard and therefore
cannot be combined with `--no-display`.

---

## Validation modes

Use the mock model to validate CARLA, sensors, synchronization, and the UI
without downloading or loading model weights:

```sh
./run_alpamayo.sh --mock-model --max-cycles 2
```

Other useful modes are:

```sh
# Inference and visualization without vehicle control.
./run_alpamayo.sh --observe-only

# Headless bounded automation.
./run_alpamayo.sh --mock-model --no-display --max-cycles 1

# Save the first dashboard frame containing a prediction.
./run_alpamayo.sh --mock-model --screenshot dashboard.png --max-cycles 1
```

Run a launcher with `--help` for the complete command-line reference.

---

## Important behavior

Alpamayo's reasoning text is displayed and recorded, but it does not directly
control the vehicle. CARLA control is derived from the numerical trajectory.
If the text says “accelerate” while the numerical trajectory remains
stationary, the controller correctly requests zero target speed. Inspect the
telemetry sidecar to distinguish model-output disagreement from control
conversion errors.

This integration is a research example, not a safety controller. It adds no
independent collision avoidance beyond behavior represented in the predicted
trajectory.
