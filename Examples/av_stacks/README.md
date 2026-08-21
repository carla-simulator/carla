# av_stacks — Reference AV-Stack Integrations for CARLA

This directory hosts **reference integrations of complete autonomous-driving
stacks with CARLA**. ROS2-based stacks talk to the simulator over CARLA's
native ROS2 interface — the CARLA server itself publishes sensor data and
vehicle status and subscribes to control commands over DDS, with no Python or
C++ bridge process in the hot path. Non-ROS stacks integrate through the
Python API.

Each subdirectory is one stack. ROS2 stacks are packaged the same way:

| Subdir | Contents |
|---|---|
| `<stack>/install/` | Installer scripts that fetch, pin, and build the stack (source or container) without touching the CARLA tree |
| `<stack>/map_tools/` | Tools to obtain or generate the map artifacts the stack needs (point clouds, lanelet2/vector maps, projector configs) |
| `<stack>/run/` | Launch orchestration: start CARLA with the right ROS2 flags, spawn the ego and sensor rig, launch the stack, keep sync-mode ticking correct |
| `<stack>/README.md` | Full walkthrough for that stack |

## Stacks

- **[`autoware/`](autoware/README.md)** — [Autoware](https://autoware.org/)
  (Core/Universe), the first reference integration. Supports the classical
  full-stack pipeline (NDT localization, perception, planning, control) and an
  end-to-end camera-only mode using the TensorRT VAD model. Native ROS2.
- **[`alpamayo/`](alpamayo/README.md)** — NVIDIA
  [Alpamayo 2 Super](https://github.com/NVlabs/alpamayo2), closed-loop
  vision-language-action driving: seven-camera model input, predicted
  trajectory + Chain-of-Causation display, and trajectory tracking through
  CARLA vehicle controls. Python API based; composes with the NuRec neural
  renderer (`PythonAPI/examples/nvidia/nurec/`). See the
  [user guide](../../Docs/nvidia_alpamayo.md).

## Quickstart

The short version, all detailed in [`autoware/README.md`](autoware/README.md):
install Autoware with `autoware/install/install_autoware.sh`, obtain map
artifacts for your town with `autoware/map_tools/fetch_prebuilt_maps.sh` (or
regenerate them from a live simulator with
`autoware/map_tools/generate_map_artifacts.py`), then start everything with
`autoware/run/run_carla_autoware.sh --mode classical` (or `--mode e2e`). The
run script launches the CARLA server with native ROS2 enabled (`-ros2`, plus
`-rmw=`/`-ros-domain-id=` as needed), runs the ego/sensor spawner that also
acts as the single sync-mode ticking client, and brings up Autoware pointed at
your map directory.
