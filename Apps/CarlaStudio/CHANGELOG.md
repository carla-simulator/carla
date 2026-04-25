# CARLA Studio — Changelog

Versioning follows [Semantic Versioning](https://semver.org/):
- **MAJOR (X.0.0)** — incompatible API changes or significant, breaking overhauls.
- **MINOR (0.Y.0)** — new functionality in a backward-compatible manner.
- **PATCH (0.0.Z)** — backward-compatible bug fixes or small tweaks.

The single source of truth for the live version is the
`CARLA_STUDIO_VERSION` value in `Apps/CarlaStudio/CMakeLists.txt`.
The GUI reads it via the `CARLA_STUDIO_VERSION_STRING` compile-time macro
and renders it in the top-right corner of the menu bar; hovering over that
label shows this changelog.

---

## v1.0.0

Initial public release.

### Home tab
- **Pipeline** — CARLA root, Scenario, Runtime (Local / Docker / Remote),
  Target, Weather (humanized presets — "Clear · Noon", "Soft Rain · Night", …),
  Vehicles, Pedestrians, Random seed.
- **Connectivity** — port + endpoint, inline.
- **Process Control** — per-PID CPU / Memory / GPU rows, total bars with
  threshold colouring (blue < 75 %, yellow 75–94 %, red ≥ 95 %), pretty
  process names ("Simulator (UE4)" vs "Launcher (.sh)").
- **Status row** — `Status: …` + smart `CARLA x.y.z with Unreal N` /
  `Not Configured (click to configure)` / red mismatch badge.
- **View buttons** — Driver / Chase / Cockpit / Bird Eye View; enabled once
  the simulator is running.

### Actuate tab
- 17 player slots: EGO + POV.01..POV.10 + V2X.01..V2X.06.
- Each row has an editable name field, joystick / SUMO control selector,
  and pop-out preview button.
- Two cascade chains rooted at EGO (POV chain + V2X chain): downstream rows
  unlock as the upstream slot is assigned.
- **Self Drive** group — SAE L0–L5 selector; L1+ unlock once a self-drive
  integration is configured.
- Backend row — LibCarla / Python API toggle + reconnect button.

### Sense tab
- Per-sensor configuration with Assembly / Optics / Output panels.
- Multi-instance support: when a sensor's count > 1, an Instance picker +
  per-instance Name field appear and per-instance pose is editable while
  optics stay shared across instances of a kind.
- Apply / Save / Load / YAML footer.
- Schematic view in a blueprint palette (pale-blue paper, navy strokes,
  faint dotted grid, orange mount-point accent).

### Interfaces tab
- 12-protocol grid (LIN · FlexRay · MOST · Auto Ethernet · TSN · ROS ·
  SOME/IP · gRPC · DSRC · C-V2X · XCP · CAN).
- Hover tooltips describe each protocol; status indicators colour live for
  ROS, CAN, SOME/IP, TSN.

### Health Check tab (toggleable)
- 12 probes covering CARLA root, Engine (UE4/UE5 + CARLA version),
  PythonAPI / C++ API (paired), Disk free, NVIDIA driver, ROS 2 / ROS-bridge
  (paired), Scenario Runner, Leaderboard, Autoware, Plugins, with a
  `Third Party` subsection.
- Single-line footer: `[🗑 Cleanup] [↻ Re-check]  Last checked: …  ●OK
  ●optional/not found  ●blocks launch`.

### Scenario Builder tab (toggleable)
Workflow-numbered groups:
1. **Quickstart Examples** — bundled `empty_flat.xodr`,
   `sample_osm_rural.xodr`, `scenario_town01_demo.json`.
2. **Map Browser** — list / load / reload from sim, plus shortcut to install
   more maps.
3. **OpenDRIVE Tools** — Browse, offline Validate, Import to sim
   (`GenerateOpenDriveWorld`), Export current world (`map.GetOpenDrive()`).
   OSM → XODR via `carla.Osm2Odr.convert()`.
4. **2D Map Preview** — topology + spawn-points rendered in a
   `QGraphicsView`.
5. **Scenario File** — JSON save / load / `Apply to sim` (sets weather +
   spawns vehicles via `TrySpawnActor` + autopilot, walkers via
   `WalkerAIController` from navmesh sample points).

### Logging tab (toggleable)
- Recording & Playback, Debug Visualization, Display Settings.

### Cfg menu
- `Install / Update CARLA…`, `Load Additional Maps…`, `Community Maps…`,
  `Cleanup / Uninstall…`, `Health Check…`, `Remote / SSH Settings…`,
  render / driver options.

### Installer
- Pre-flight check skips re-download when the target directory already
  contains an extracted CARLA install (Yes / Re-download / Cancel).
- On install success, prompts to append `export CARLA_ROOT=…` to
  `~/.bashrc` / `~/.zshrc` / fish config, idempotently.

### Cleanup / Uninstall
- Two-step UX: Detect → Preview (no-op) → Confirm delete.
- Detects CARLA root, soft-cleanup of `Saved/`, community-map drops,
  `carlasim/carla:*` Docker images, app caches, PythonAPI eggs/wheels.

### Plugins
- `<exe-dir>/plugins`, `../lib/carla-studio/plugins`, and
  `$CARLA_STUDIO_PLUGIN_PATH` are scanned for shared libraries that export
  `carla_studio_plugin_create` (see `Examples/QtClient` for a template).

### Distribution / packaging
- Cross-platform CMake (Qt 5 fallback when Qt 6 absent).
- Per-platform icons: Linux hicolor, macOS `.icns`, Windows `.ico`.
- `.desktop` entry + install rules on Linux; `MACOSX_BUNDLE` on macOS.

---

## Roadmap

Tabs shipped in this release as deliberate skeletons — toggleable via
*Tools* (default off), with placeholder UI that points back here so
users see what's planned without misreading them as live features.

### Leaderboard tab
Source of truth: <https://leaderboard.carla.org/>.
- Ranking browser — filter / search the live leaderboard.
- Local agent runner — pick a Python agent + route, run it locally,
  surface the Driving Score (Route Completion × Infraction Penalty).
- Track 1 (sensors only) / Track 2 (privileged) selectors.
- Submission helper — build agent container, push to autograder,
  monitor evaluation status.

### Testing tab
Two regulator-defined regimes, each rendered as its own QGroupBox card
list. Studio's role at runtime: load the matching scenario, configure
CARLA actors per the protocol, run, score, emit a report (PDF / JSON).

#### Euro NCAP — <https://www.euroncap.com/en/for-engineers/protocols/>
- AEB Car-to-Car Rear (CCR)
- AEB Car-to-Car Front Turn-Across-Path (FTAP)
- AEB Vulnerable Road User (VRU) — Pedestrian / Cyclist / Powered
  Two-Wheeler
- Lane Support Systems (LSS) — LKA / LDW / ELK
- Speed Assist Systems (SAS)
- Driver State Monitoring (DSM)

#### NHTSA — <https://www.nhtsa.gov/laws-regulations/fmvss>
- FMVSS-126 (Electronic Stability Control)
- FMVSS-127 (Automatic Emergency Braking)
- FMVSS-208 (Occupant Crash Protection)
- FMVSS-214 (Side Impact Protection)
- NCAP Frontal / Side (MDB / Pole) / Rollover Resistance
- AEB — Pedestrian (NCAP 2024+)
