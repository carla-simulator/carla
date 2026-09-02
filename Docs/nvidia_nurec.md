# Use NVIDIA Neural Reconstruction with CARLA

NVIDIA Neural Reconstruction (NuRec) refers to the reconstruction and rendering models and services from NVIDIA that support the seamless ingestion of real-world data converted to a simulated environment suitable for training and testing Physical AI agents, including robotics and autonomous driving systems.

With NuRec, developers can convert recorded camera and LIDAR data into 3D scenes. NuRec uses multiple AI networks to create interactive 3D test environments where developers can modify the scene and see how the world reacts. Developers can change scenarios, add synthetic objects, and apply randomization — making the initial scenarios even more challenging.

CARLA UE5 integrates NuRec through NVIDIA's __Neural Reconstruction Engine (NRE)__ container and its gRPC API. A NuRec scenario is a `.usdz` artifact containing the trained neural scene, the recorded actor tracks and the OpenDRIVE road network. When you run a replay:

1. The scenario's `map.xodr` is extracted from the `.usdz` and loaded with `client.generate_opendrive_world()` — no named CARLA town is used.
2. The recorded actors are spawned as CARLA actors (blueprint dimensions are probed from the live server, so the integration survives blueprint catalog changes) and CARLA hosts the actors, physics and the simulation loop.
3. The NRE container renders photoreal camera frames — and, from NRE 26.04, lidar sweeps — at the poses CARLA reports each tick, and the frames are fetched over gRPC. All cameras due in a tick are rendered in a single batched round trip.

Because CARLA owns the simulation loop, the replay can be perturbed with the full CARLA API: the ego can be re-driven with CARLA vehicle physics or handed to the Traffic Manager, other vehicles can be handed to the Traffic Manager, and scene assets can be swapped or inserted through the asset-editing API.

* __[Before you begin](#before-you-begin)__
    * [Prerequisites](#prerequisites)
    * [Hugging Face account](#hugging-face-account)
* __[Setup](#setup)__
    * [Prerequisite installation](#prerequisite-installation)
    * [Run the installer script](#run-the-installer-script)
    * [Example datasets](#example-datasets)
    * [Environment variables](#set-up-your-environment-variables)
* __[Run the CARLA NuRec replays](#run-the-carla-nurec-replays)__
    * [Demo configuration file](#demo-configuration-file)
* __[Command line parameters](#command-line-parameters)__

---

## Before you begin

### Prerequisites

Before you get started, make sure you have satisfied the following prerequisites:

- Ubuntu 22.04 or later
- CARLA UE5 (0.10.x) with its Python wheel built or installed
- An NVIDIA GPU with a recent driver (the NRE container runs CUDA workloads)
- Python 3.10 or later
- Docker with the NVIDIA Container Toolkit (the installer script can install both)

### Hugging Face account

The installer downloads a sample scene from the gated [NVIDIA PhysicalAI-Autonomous-Vehicles-NuRec dataset](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec) on Hugging Face. Accept the dataset terms before installation and authenticate the CLI with the same account:

* If you don't already have a Hugging Face account, [create one](https://huggingface.co/join) and log in.
* [Create a token](https://huggingface.co/settings/tokens) with *Read* permissions and save it in a safe place.
* Run `hf auth login`, or set `HF_TOKEN` for non-interactive installation.

---

## Setup

In the following instructions, a `CARLA_ROOT` environment variable is used to locate the root directory of your CARLA installation (the source checkout or the extracted package). You should set this variable in your terminal or add the following line to your `.bashrc` profile:

```sh
export CARLA_ROOT=/path/to/carla
```

### Prerequisite installation

The installer script installs Docker and the NVIDIA Container Toolkit automatically if they are missing (it needs `sudo` for that). If you prefer to install them yourself first:

**Docker**: The NRE tool runs inside a Docker container. Follow the [Docker installation instructions](https://docs.docker.com/engine/install/ubuntu/) for Ubuntu, then verify with:

```sh
docker run hello-world
```

If this command produces a permission error, add your user to the `docker` group and log out and back in:

```sh
sudo usermod -aG docker $USER
```

**NVIDIA Container Toolkit**: required so the container can access the GPU. Follow [these instructions](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) to install it.

**Virtual environment**: the installer creates and uses `.venv` inside the
NuRec example directory by default. Pass `--python /path/to/python` only when
you intentionally want to use another compatible environment.

### Run the installer script

Navigate to the NuRec example directory and run the installation script:

```sh
cd ${CARLA_ROOT}/PythonAPI/examples/nvidia/nurec/
./install_nurec.sh
```

The script:

*   Installs Docker and the NVIDIA Container Toolkit if missing.
*   Pulls the pinned NRE container image (`nvcr.io/nvidia/nre/nre-ga:26.04.01`) from NVIDIA NGC.
*   Downloads a sample scene through the authenticated Hugging Face session.
*   Creates an isolated `.venv` without modifying `~/.bashrc`.
*   Installs the Python dependencies and CARLA wheel. The wheel is located automatically in `PythonAPI/carla/dist`, `Build/PythonAPI/dist`, or `Build/Release/PythonAPI/dist`; build the Python API first if none exists.

Optional installer arguments:

| Argument | Description |
|---|---|
| `-i`, `--python <exe>` | Use an existing Python interpreter instead of the default local `.venv`. |
| `-s`, `--scene <uuid>` | Download a specific scene from the dataset instead of the default sample scene. |
| `--full-dataset` | Download the entire dataset instead of a single sample scene. __The full dataset is about 1.5 terabytes__, so ensure you have adequate drive space. |

### Example datasets

By default the installer downloads one sample scene from the `26.04_release` sample set into `${CARLA_ROOT}/PythonAPI/examples/nvidia/nurec/PhysicalAI-Autonomous-Vehicles-NuRec/`. Browse the [dataset on Hugging Face](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec) to pick other scenes and pass their UUID with `--scene`.

!!! note
    If a previous download was interrupted, delete (or move) the scene folder under *PhysicalAI-Autonomous-Vehicles-NuRec* to force a re-download — the installer skips the download if the folder exists.

Scenarios from older dataset releases (for example `25.07_release`) may need upgrading to the current artifact format before the NRE container can load them:

```sh
docker run --rm --gpus all -v $(pwd):$(pwd) $NUREC_IMAGE upgrade-artifact --help
```

### Set up your environment variables

The replay recognizes two environment variables:

*   `NUREC_IMAGE` overrides the pinned NRE container image for the current shell:

    ```sh
    export NUREC_IMAGE="nvcr.io/nvidia/nre/nre-ga:26.04.01"
    ```

*   [`CUDA_VISIBLE_DEVICES`](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#env-vars) is optional and designates the GPU that runs the neural rendering. If unset, GPU 0 is used.

---

## Run the CARLA NuRec replays

__1. Launch the CARLA server.__ From a packaged release:

```sh
cd ${CARLA_ROOT}
./CarlaUnreal.sh
```

From a source build, launch the standalone binary (add `-RenderOffScreen` for headless operation) or the editor:

```sh
./Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal -carla-rpc-port=2000 -RenderOffScreen
# or the editor:
cmake --build Build --target launch-only
```

__2. Replay a NuRec scenario.__ In a second terminal, run the replay script with the path to a `.usdz` scenario:

```sh
cd ${CARLA_ROOT}/PythonAPI/examples/nvidia/nurec/
.venv/bin/python nurec_runner.py -u PhysicalAI-Autonomous-Vehicles-NuRec/sample_set/26.04_release/<scene_uuid>/<scenario>.usdz
```

The NRE container is started automatically; the gRPC port is auto-picked unless `-np/--nurec-port` is given. Containers are reused between runs to skip the scene-loading time (reuse is verified over gRPC — server version and scene id — before adoption).

A pygame window opens showing the scene's calibrated rig cameras (front, sides, rear) rendered by NuRec, a bird's-eye-view panel of the neural lidar, and a CARLA-rendered debug view of the reconstructed OpenDRIVE world with the proxy actors. Add `--saveimages` to also write every frame to disk (`--output-dir`, default `data`; lidar sweeps go to `<output-dir>/lidar_bev/`).

### Demo configuration file

Everything beyond the basic command line is controlled by a YAML settings file, `nurec_demo_config.yaml` by default (pass a different one with `-c/--config`; every key is optional). It exposes, among others:

*   __Renderer__: renderer backend selection (default: the backend the artifact was trained for; `nrend` and `gsplat` can be forced), the appearance harmonizer that visually blends inserted or CARLA-driven actors into the neural background, raw planar vs legacy JPEG image transfer, and pass-through of any extra `serve-grpc` flags.
*   __Cameras__: render the recording's own calibrated rig cameras (intrinsics and mounting extrinsics come from the scene itself), or hand-written camera definitions from a YAML file — see `carla_example_camera_config.yaml` for the format, including f-theta intrinsics with principal point and distortion polynomials, custom mounting transforms and rolling shutter parameters. Resolution scale and framerate are configurable, as is the CARLA debug view and its weather preset.
*   __Control__: after a configurable handoff time, the ego either keeps following the recorded track (`replay`), re-drives the recorded trajectory with CARLA vehicle physics (`trajectory`, the default), or drives freely with the CARLA Traffic Manager (`autopilot`). All other vehicles can likewise be handed to the Traffic Manager (`actors: carla`), with the resulting poses streamed into the neural render — this is the hook for authoring custom scenarios on top of a NuRec scene.
*   __Asset editing__ (NRE 26.04 and later): list the scene's dynamic objects and swappable assets, swap the rendered asset of one vehicle for another, or insert a rendered-only clone with a shifted trajectory.
*   __Lidar__ (NRE 26.04 and later): a neural lidar (`PANDAR128` or `AT128`), with configurable framerate, mount height and bird's-eye-view visualization.

For programmatic use, the underlying API in `nurec_integration.py` is compact:

```py
with NurecScenario(client, usdz_path) as scenario:
    scenario.add_camera("camera_front_wide_120fov", on_image)  # NuRec camera
    scenario.add_lidar(on_lidar, lidar_type="PANDAR128")       # NuRec lidar (NRE >= 26.04)
    scenario.start_replay()
    while not scenario.is_done():
        scenario.tick()
```

---

## Command line parameters

The following table explains the command-line parameters of `nurec_runner.py`:

| Parameter | Long form | Default | Description |
|-----------|-----------|---------|-------------|
| | `--host` | 127.0.0.1 | IP address of the CARLA host server |
| `-p` | `--port` | 2000 | TCP port of the CARLA server |
| `-u` | `--usdz-filename` | (required) | Path to the USDZ file containing the NuRec scenario |
| `-np` | `--nurec-port` | auto | TCP port for the NuRec gRPC connection (default: auto-pick a free port) |
| `-c` | `--config` | `nurec_demo_config.yaml` | Demo settings file (see [above](#demo-configuration-file)) |
| | `--saveimages` | Inactive | Save the images generated by the NuRec and CARLA cameras |
| `-o` | `--output-dir` | `data` | Output directory for `--saveimages` |
| | `--move-spectator` | Inactive | Move the spectator camera to follow the ego vehicle |
