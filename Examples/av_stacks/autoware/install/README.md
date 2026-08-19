# Autoware install tooling for CARLA (UE5.8, native ROS2)

`install_autoware.sh` fetches/installs Autoware for use against CARLA's native
ROS2 interface. It has three modes and one add-on:

```bash
./install_autoware.sh --check                 # read-only prerequisite report
./install_autoware.sh                         # default: source build (see below)
./install_autoware.sh --source --with-vad     # source build + TensorRT-VAD e2e stack
./install_autoware.sh --docker                # pull the official prebuilt image
```

## Version policy

| Tag | Why |
|-----|-----|
| `0.45.1` | TIER IV-validated baseline for CARLA native ROS2 (Humble / Ubuntu 22.04 source build). Default on Humble. |
| `1.9.0` | Current Autoware release (`autoware_universe` 0.52.0 / `autoware_core` 1.9.0). Default on Jazzy and for `--docker`. |

Override with `--version <tag>`. Supported OS/ROS pairings are enforced:
Humble ↔ Ubuntu 22.04, Jazzy ↔ Ubuntu 24.04 — the script refuses a source
build on a mismatch (use `--docker` instead).

## Source mode

Mirrors the canonical sequence from docs.autoware.org source installation:
clone the meta-repo at the tag → `vcs import src < autoware.repos` (the file is
at `repositories/autoware.repos` on 1.x, repo root on 0.45.1; both handled) →
`rosdep install -y --from-paths src --ignore-src` → `colcon build
--symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release`.

Deliberate deviation: we use plain non-interactive `rosdep` instead of the
upstream `ansible-playbook autoware.dev_env.install_dev_env`, because the
ansible dev-env playbook prompts interactively (NVIDIA/CUDA/TensorRT,
artifact downloads) and mutates the base system beyond ROS dependencies.
CUDA/cuDNN/TensorRT must already be installed system-wide if you want GPU
perception or VAD; `--check` reports on this. The script header shows the
ansible commands if you prefer the upstream flow.

Build parallelism is **bounded** to `nproc/2` (both `MAKEFLAGS=-jN` and
`colcon --parallel-workers N`) because unbounded Autoware builds OOM on
this class of machine. Override with `--jobs N`.

## Docker mode

Pulls from GHCR (no local image build):

- `ghcr.io/autowarefoundation/autoware:universe-cuda-jazzy` — GPU (default when `nvidia-smi` is present)
- `ghcr.io/autowarefoundation/autoware:universe-jazzy` — CPU-only
- swap `jazzy`→`humble` via `--distro humble`; pin a release by appending
  `--version 1.9.0` (→ `...-jazzy-1.9.0`)

The script prints the full `docker run` pattern (host networking, CycloneDDS,
map/data volume mounts). CARLA's Fast-DDS default here is UDPv4-only, so
containers interoperate with the simulator without SHM configuration.

## `--with-vad`

1. Downloads the VAD model from HuggingFace
   `AutowareFoundation/tensorrt_vad` tag `v0.1` into
   `~/autoware_data/ml_models/vad/v0.1/`:
   `vad-carla-tiny_backbone.onnx`, `vad-carla-tiny_head_no_prev.onnx`,
   `vad-carla-tiny_head.onnx`, `vad-carla-tiny.param.json`,
   `deploy_metadata.yaml` (all resolve URLs verified). TensorRT engines are
   built on the first VAD run and cached.
2. In source mode, checks out the `autoware_launch` glue from **open PR
   [autowarefoundation/autoware_launch#1685](https://github.com/autowarefoundation/autoware_launch/pull/1685)**
   (head branch `feat/e2e-vad-carla-simulator` on the same repo — verified via
   the GitHub API, 2026-08-19). This PR is what adds
   `use_e2e_planning` / `e2e_planning_type:=vad` to `e2e_simulator.launch.xml`;
   it is **not merged**. If it merges later and the branch disappears, the
   script degrades gracefully and tells you to use `autoware_launch` main or
   launch `e2e/autoware_tensorrt_vad/launch/vad_carla_tiny.launch.xml` manually.

VAD also needs `/localization/kinematic_state` + `/localization/acceleration`
from ground truth (three plain ROS nodes from `autoware_carla_interface`) and
the six-camera CARLA rig — see the sibling `Examples/av_stacks/autoware`
runtime tooling for that side.

## Maps

Autoware needs per-town `pointcloud_map.pcd`, `lanelet2_map.osm`, and
`map_projector_info.yaml` (`projector_type: Local`) passed as `map_path:=<dir>`.
Prebuilt Town01–07 + Town10HD maps live in the
`carla-simulator/autoware-contents` Bitbucket repo; map tooling is handled by
the sibling map scripts in this Examples tree, not by this installer.
