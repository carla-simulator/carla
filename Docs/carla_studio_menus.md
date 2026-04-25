# CARLA Studio — Menus

Three top-level menus: **Cfg**, **Tools** (conditional), **Help**.
Everything that doesn't belong on a tab lives here.

![Menu bar](img/cs_menus_bar.png)

## Cfg

Configuration entry-points. Each item below opens its own modal
dialog and persists to `QSettings`.

| Item | Tooltip |
|---|---|
| Install / Update CARLA… | Pull / refresh the simulator binary against the configured CARLA root |
| Load Additional Maps… | Add maps from the official content packs |
| Community Maps… | Browse / install community-contributed maps |
| Cleanup / Uninstall… | Two-step Detect → Preview → Confirm wipe of CARLA artifacts |
| Docker Settings… | Image tag, runtime flags, NVIDIA toolkit toggle |
| Hugging Face Account | Sign in with a PAT (stored obfuscated under `hf/token`) |
| HF Datasets | Browse / pull datasets from the configured account |
| HF Models | Browse / pull models from the configured account |
| Third-Party Tools | Detection + path overrides for ROS / Gazebo / Foxglove |
| Compute (HPC) Settings… | Slurm / cluster targets for offloaded jobs |
| Python Environments (uv) | Manage `uv`-backed Python envs for the CARLA client |
| Remote / SSH Settings… | Hosts, keys, tunnel ports for the Remote backend |
| Render flags | Forward `-RenderOffScreen`, `-quality-level` etc. to the engine |
| Driver flags | NVIDIA-specific knobs (Vulkan / Optix / DLSS gating) |
| ROS-native flags | DDS impl, RMW, domain ID for the ros-bridge path |

## Tools

Appears **only when supported tools are detected** on `PATH` or via
the Cfg → Third-Party Tools overrides. Items grey out if their
binary disappears between launches.

### ROS Tools (submenu)

| Item | Launches |
|---|---|
| RViz | `rviz2` against the live ROS graph |
| rqt | `rqt` umbrella |
| rqt_graph | Node / topic graph visualiser |
| rqt_console | Log console |
| rqt_plot | Live numeric topic plotter |
| rqt_topic | Topic inspector |
| topic-list | `ros2 topic list` snapshot in a side panel |

### Siblings

| Item | Notes |
|---|---|
| Gazebo | Launches the detected `gz sim` / `gazebo` binary |
| Foxglove Studio | Opens Foxglove pointed at the live websocket bridge |

## Help

| Item | What it does |
|---|---|
| Documentation | Opens the CARLA docs (this site) |
| Health Check | Toggles visibility of the Health Check tab |
| License | Shows Studio's AGPL-3.0-or-later notice + third-party manifest + citation block |

The License dialog also surfaces the AGPL §13 network-use notice and
a "Source" link back to the upstream repository — that link is what
satisfies §13 for hosted deployments.

### Attribution

CARLA Studio is © 2026 Abdul, Hashim, distributed
under **AGPL-3.0-or-later**. The simulator itself remains MIT;
Studio is the front-end, not the engine.

![Help → License dialog](img/cs_menus_license.png)
