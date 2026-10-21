# Use NVIDIA Neural Reconstruction with CARLA

[NVIDIA Omniverse NuRec](https://docs.nvidia.com/nurec/) is NVIDIA's family of neural reconstruction models and services: it turns recorded camera and LIDAR data into a 3D scene that can be rendered from new viewpoints, so a real drive becomes an interactive environment for training and testing Physical AI agents. The scene can then be modified — scenarios changed, synthetic objects added, randomization applied — and it reacts accordingly. [How NuRec works](https://docs.nvidia.com/nurec/basics/how-nurec-works.html) describes the networks behind it.

__NuRec__ is the technology; the __Neural Reconstruction Engine (NRE)__ is the piece you run: a [container published on NGC](https://catalog.ngc.nvidia.com/orgs/nvidia/teams/nre/containers/nre-ga) that loads a reconstructed scene and serves rendered frames over a [gRPC API](https://docs.nvidia.com/nurec/api/grpc_api_guide.html). CARLA UE5 integrates NuRec through that container. A NuRec scenario is a `.usdz` artifact containing the trained neural scene, the recorded actor tracks and the OpenDRIVE road network. When you run a replay:

1. The scenario's `map.xodr` is extracted from the `.usdz` and loaded with `client.generate_opendrive_world()` — no named CARLA town is used.
2. The recorded actors are spawned as CARLA actors (blueprint dimensions are probed from the live server, so the integration survives blueprint catalog changes) and CARLA hosts the actors, physics and the simulation loop.
3. The NRE container renders photoreal camera frames — and, from NRE 26.04, lidar sweeps — at the poses CARLA reports each tick, and the frames are fetched over gRPC. All cameras due in a tick are rendered in a single batched round trip.

Because CARLA owns the simulation loop, the replay can be perturbed with the full CARLA API: the ego can be re-driven with CARLA vehicle physics or handed to the Traffic Manager, other vehicles can be handed to the Traffic Manager, and scene assets can be swapped or inserted through the asset-editing API.

- [__Before you begin__](#before-you-begin)
- [__Setup__](#setup)
- [__Run the CARLA NuRec replays__](#run-the-carla-nurec-replays)
- [__Command line parameters__](#command-line-parameters)
- [__Hybrid rendering example__](#hybrid-rendering-example)
- [__See also__](#see-also)

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
| `--python <exe>`, `-i` | Use an existing Python interpreter instead of the local `.venv`. |
| `--scene <uuid>`, `-s` | Download a specific scene instead of the default sample scene. |
| `--full-dataset` | Download the whole dataset instead of one sample scene.<br>__It is about 1.5 terabytes__, so check the free space first. |

### Example datasets

By default the installer downloads one sample scene from the `26.04_release` sample set into `${CARLA_ROOT}/PythonAPI/examples/nvidia/nurec/PhysicalAI-Autonomous-Vehicles-NuRec/`. Browse the [dataset on Hugging Face](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec) to pick other scenes and pass their UUID with `--scene`.

!!! Note
    If a previous download was interrupted, delete (or move) the scene folder under *PhysicalAI-Autonomous-Vehicles-NuRec* to force a re-download — the installer skips the download if the folder exists.

Scenarios from older dataset releases (for example `25.07_release`) may need upgrading to the current artifact format before the NRE container can load them:

```sh
docker run --rm --gpus all -v $(pwd):$(pwd) nvcr.io/nvidia/nre/nre-ga:26.04.01 upgrade-artifact --help
```

### Set up your environment variables

The replay recognizes two environment variables:

*   `NUREC_IMAGE` overrides the pinned NRE container image (`nvcr.io/nvidia/nre/nre-ga:26.04.01`) for the current shell:

    ```sh
    export NUREC_IMAGE="nvcr.io/nvidia/nre/nre-ga:26.04.01"
    ```

*   [`CUDA_VISIBLE_DEVICES`](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#env-vars) is optional and designates the GPU that runs the neural rendering. If unset, GPU 0 is used.

---

## Run the CARLA NuRec replays

__1. Launch the CARLA server.__ From a packaged release:

```sh
cd <package root>
./CarlaUnreal.sh
```

A source build or the editor works just as well; the replay only needs a server to connect to on the RPC port. See [Running CARLA](start_quickstart.md#running-carla) for the packaged server and [Build and run CARLA UE5](build_linux_ue5.md#build-and-run-carla-ue5) for the build targets that launch it.

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

| Parameter | Default | Description |
|---|---|---|
| `--usdz-filename`, `-u` | required | Path to the `.usdz` holding the NuRec scenario |
| `--host` | 127.0.0.1 | Address of the CARLA server |
| `--port`, `-p` | 2000 | TCP port of the CARLA server |
| `--nurec-port`, `-np` | auto | TCP port of the NuRec gRPC connection; a free port is picked when unset |
| `--config`, `-c` | `nurec_demo_config.yaml` | Demo settings file, see [above](#demo-configuration-file) |
| `--saveimages` | off | Save the images generated by the NuRec and CARLA cameras |
| `--output-dir`, `-o` | `data` | Output directory for `--saveimages` |
| `--move-spectator` | off | Move the spectator camera to follow the ego vehicle |

---
## Hybrid rendering example

`PythonAPI/examples/nvidia/nurec/hybrid/` renders a NuRec scene with CARLA-driven vehicles and pedestrians composited into it: the NuRec frame supplies the background, CARLA's [ray-traced lens camera family](ref_sensors.md#ray-traced-lens-camera) supplies the added actors (colour, per-pixel distance and instance id, in the recorded camera's exact lens), occlusion against the neural scene's own geometry is resolved with the distance AOV, and lighting is matched with [`world.set_sky_light_map`](python_api.md#carla.World.set_sky_light_map) fed from the scene's own recorded environment. See `PythonAPI/examples/nvidia/nurec/hybrid/README.md` for the end-to-end command and output layout.

The same pipeline runs [Scenic](https://scenic-lang.org) scenarios inside the neural scene: `hybrid_run.py --scene <scene_uuid> --out <run dir> --scenic hwy_cutin.scenic` hands the scene's road network to Scenic, lets the scenario drive the ego (which carries the recorded camera) and every other agent, and composites the result frame by frame. Five scenarios ship with it — `hwy_cutin.scenic`, `hwy_overtake.scenic`, `hwy_brake_wave.scenic`, `urban_lead_brake.scenic` and `urban_pullout.scenic` — `PythonAPI/examples/nvidia/nurec/hybrid/scenic/README.md` describes the driver, the globals a scenario receives and how to write a new one.

---
## See also

NuRec scenes can also be driven by the Alpamayo 2 Super stack: see [Run with NuRec cameras](nvidia_alpamayo.md#run-with-nurec-cameras), which shares this installer. Do not run `nurec_runner.py` at the same time as the Alpamayo demo; both drive the same NRE container.
