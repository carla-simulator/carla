# CARLA Studio

CARLA Studio is the desktop GUI front-end for the CARLA simulator. One
window covers everything you'd otherwise wire up by hand: scenario and
map selection, sensor mounting, actor control, third-party-tool
integration (TeraSim, Autoware), Hugging Face dataset/model management,
local hardware fitness probes, and orchestrated launches across local /
Docker / SSH / NVIDIA Brev backends.

Studio is built with Qt 5/6 and links optionally against `libcarla-client`.
Source lives at [`Apps/CarlaStudio/`](https://github.com/carla-simulator/carla/tree/master/Apps/CarlaStudio).
Licence: **AGPL-3.0-or-later** (the simulator stays MIT — Studio is the
front-end, not the engine).

---

## Tabs

| Tab | Purpose |
|---|---|
| ⌂ Home | Map / View / Host / Weather / Port + START / STOP + live process table. The day-to-day launcher. |
| ⇄ Interfaces | **Sense** column (Vehicle + sensor categories: Camera, RADAR, LiDAR, NAV, GT, ros-bridge pseudo-sensors) + **Actuate** column (11-player table EGO + POV01..10, SAE L0–L5, LibCarla / Python backend). |
| 🩺 Health Check | Probes for CARLA root, engine version, Python / C++ APIs, disk, NVIDIA driver, ROS 2, ROS-bridge, Scenario Runner, Leaderboard, Autoware, Plugins. SDK ↔ Sim version compatibility row catches client/server mismatches before they crash. |
| 🛠 Scenario Builder | Quickstart examples, Map Browser, OpenDRIVE tools, 2D map preview, scenario JSON editor. |
| ◇ Scenario Re-Construction | NVIDIA reconstruction stack — NuRec renderer, Asset-Harvester, NCore, Lyra 2.0. Tile gating reflects local hardware fitness. |
| 🏆 Leaderboard *(roadmap)* | Submit and rank autonomous-driving agents on standard routes. See `CHANGELOG.md` → Roadmap. |
| 🛡 Testing *(roadmap)* | Euro NCAP and NHTSA protocols. See `CHANGELOG.md` → Roadmap. |
| 📋 Logging | CARLA recording / playback, debug visualisation, Python API distribution manager. |

The corner of the tab strip carries a **screenshot tool** (camera glyph)
that delays 3 seconds, then captures the whole desktop into a PNG +
clipboard so you can record open menus / tooltips before they close.

## Menus

- **Cfg** — Install / Update CARLA, Load Additional Maps, Community Maps,
  Cleanup / Uninstall, Docker Settings, Hugging Face Account, HF
  Datasets, HF Models, Third-Party Tools, Compute (HPC) Settings,
  Python Environments (uv), Remote / SSH Settings, render / driver /
  ROS-native flags.
- **Tools** — appears when supported tools are detected. Submenu **ROS
  Tools** for RViz / rqt / rqt_graph / rqt_console / rqt_plot /
  rqt_topic / topic-list. Sibling line items for **Gazebo** and
  **Foxglove Studio**.
- **Help** — CARLA documentation, Health Check toggle, License
  (Studio AGPL-3.0 + third-party manifest + citation).

## Build

Studio builds as part of the main CARLA tree behind `BUILD_CARLA_STUDIO`:

```bash
cmake -B Build -DBUILD_CARLA_STUDIO=ON
cmake --build Build --target carla-studio -j
./Build/Apps/CarlaStudio/carla-studio
```

Qt 6 is preferred; Qt 5 is detected as a fallback. With
`BUILD_CARLA_CLIENT=ON` the in-app driver path is enabled; without it,
Studio degrades to the Python `manual_control.py` backend.

## Configuration knobs (QSettings)

Studio persists everything to `QSettings` under the
`CARLA Simulator → CARLA Studio` organisation. The most-touched keys:

| Key | What it controls |
|---|---|
| `runtime/target` | Local / Docker / Remote / Brev backend (Host row) |
| `vehicle/blueprint` | Default ego CARLA blueprint ID |
| `vehicle/color` / `vehicle/finish` | Default ego paint colour + Substrate finish |
| `actuate/player_<NAME>` | Per-POV control method (Keyboard / Joystick N) |
| `actuate/sae_level` | Selected SAE J3016 level (0–5) |
| `actuate/backend` | LibCarla vs Python API |
| `addon/{terasim_enabled,autoware_enabled}` | Integrations toggles |
| `addon/{terasim_scenario,autoware_stack}` | Per-integration choice |
| `addon/renderer` | Default vs NuRec |
| `hf/token` | Hugging Face PAT (obfuscated) |
| `python/active_dist` | Active CARLA PythonAPI wheel/egg |
| `ros/pseudo/<id>` | Per-pseudo-sensor toggle |

## Crash protection

Studio installs a `std::set_terminate` handler at `main()`'s very first
line. The most common path it guards is a LibCarla **RPC version
mismatch** — a sim built from a different commit than the bundled
`libcarla-client` deserialises corrupt size fields and throws
`std::bad_array_new_length` from a thread that bypasses the outer
`try/catch`. The handler logs `type:` + `what:` + suggested remediation
to stderr and `_Exit(1)`s instead of `abort()`. The Health Check tab's
**SDK ↔ Sim version** row surfaces the same condition before the user
clicks something that would trip it.

## Roadmap

See `Apps/CarlaStudio/CHANGELOG.md` → Roadmap. Major upcoming items:

- **Leaderboard** integration with `leaderboard.carla.org` (ranking
  browser, local agent runner, submission helper).
- **Testing** runners for Euro NCAP and NHTSA protocol catalogues.
- Vehicle + Assembly fused page with Substrate finish per-POV.
- Live multi-terminal launcher driven by
  `resources/integrations/launch/terminal_launch_map.json`.

## Citation

If you publish work that uses or extends CARLA Studio, please cite via
`Apps/CarlaStudio/CITATION.cff`. GitHub renders a "Cite this
repository" button from it that emits BibTeX, APA, and Chicago forms
automatically.
