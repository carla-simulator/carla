# CARLA Studio — Build & Configuration

Studio builds as part of the main CARLA tree behind the
`BUILD_CARLA_STUDIO` CMake flag. Qt 6 is preferred; Qt 5 is detected
as a fallback.

## Build

```bash
cmake -B Build -DBUILD_CARLA_STUDIO=ON
cmake --build Build --target carla-studio -j
./Build/Apps/CarlaStudio/carla-studio
```

## Qt detection

`find_package(Qt6 …)` runs first. If it fails, the build falls back
to `find_package(Qt5 …)`. The same source tree compiles cleanly
against either.

## CMake flags

| Flag | Effect |
|---|---|
| `BUILD_CARLA_STUDIO` | Builds the GUI target `carla-studio` |
| `BUILD_CARLA_CLIENT` | Builds `libcarla-client`; enables the in-app driver path |

With `BUILD_CARLA_CLIENT=ON` the in-app driver path is enabled —
Studio drives the sim through `libcarla-client` directly.
Without it, Studio degrades to the Python `manual_control.py`
backend (Actuate column → Backend = Python API).

## Configuration knobs (QSettings)

Studio persists everything to `QSettings` under the
`CARLA Simulator → CARLA Studio` organisation / application pair.
The most-touched keys:

| Key | What it controls |
|---|---|
| `runtime/target` | Local / Docker / Remote / Brev backend (Host row) |
| `vehicle/blueprint` | Default ego CARLA blueprint ID |
| `vehicle/color` | Default ego paint colour |
| `vehicle/finish` | Substrate finish (matte / metallic / pearl / …) |
| `actuate/player_<NAME>` | Per-POV control method (Keyboard / Joystick N) |
| `actuate/sae_level` | Selected SAE J3016 level (0–5) |
| `actuate/backend` | LibCarla vs Python API |
| `addon/terasim_enabled` | TeraSim integration toggle |
| `addon/autoware_enabled` | Autoware integration toggle |
| `addon/terasim_scenario` | Active TeraSim scenario |
| `addon/autoware_stack` | Selected Autoware stack |
| `addon/renderer` | Default vs NuRec |
| `hf/token` | Hugging Face PAT (obfuscated) |
| `python/active_dist` | Active CARLA PythonAPI wheel/egg |
| `ros/pseudo/<id>` | Per-pseudo-sensor toggle |

### Storage paths

QSettings writes to the platform's native backend under
`CARLA Simulator/CARLA Studio`:

| Platform | Backend location |
|---|---|
| Linux | `~/.config/CARLA Simulator/CARLA Studio.conf` |
| macOS | `~/Library/Preferences/com.CARLA-Simulator.CARLA-Studio.plist` |
| Windows | `HKCU\Software\CARLA Simulator\CARLA Studio` (registry) |

Sensor mounts are larger and JSON-shaped, so they live next to the
workspace rather than in QSettings:

| Path | When used |
|---|---|
| `$CARLA_WORKSPACE_ROOT/.carla_studio/sensor_mounts.json` | When `CARLA_WORKSPACE_ROOT` is set |
| `~/.carla_studio/sensor_mounts.json` | Fallback when the env var is unset |

The fallback path is also where Studio writes if the workspace
directory is read-only.

## Plugins

Studio scans three locations on startup for shared libraries that
export `carla_studio_plugin_create`:

| Search location | Purpose |
|---|---|
| `<exe-dir>/plugins` | Bundled with the binary |
| `../lib/carla-studio/plugins` | Distro-installed plugins |
| `$CARLA_STUDIO_PLUGIN_PATH` | User override (colon-separated) |

See `Apps/CarlaStudio/Examples/QtClient` for a minimal plugin
template.

## Distribution / packaging

| Asset | Notes |
|---|---|
| Linux | `.desktop` entry + hicolor icons + install rules |
| macOS | `MACOSX_BUNDLE` + `.icns` |
| Windows | `.ico` + per-arch installer artifacts |
| Qt fallback | Qt 5 path triggered when Qt 6 absent |

## Versioning

`CARLA_STUDIO_VERSION` in `Apps/CarlaStudio/CMakeLists.txt` is the
single source of truth. The GUI reads it via the
`CARLA_STUDIO_VERSION_STRING` compile-time macro and renders it in
the top-right corner of the menu bar; hovering shows the changelog.

![Build output](img/cs_build_terminal.png)
