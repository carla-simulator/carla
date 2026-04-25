# CARLA Studio — Tabs

Studio's main surface is a tab strip. Each tab below maps to one
QWidget under `Apps/CarlaStudio/`. The corner of the tab strip carries
a **screenshot tool** (camera glyph) that delays 3 seconds, then
captures the whole desktop into a PNG + clipboard so you can record
open menus / tooltips before they close.

![Tab strip](img/cs_tabs_strip.png)

## Home

Day-to-day launcher. One row per concern; START/STOP at the bottom.

| Row | Controls |
|---|---|
| Map | Town picker, browse, reload from sim |
| View | Driver / Chase / Cockpit / Bird Eye View |
| Host | Local / Docker / Remote (SSH) / NVIDIA Brev |
| Weather | Humanized presets ("Clear · Noon", "Soft Rain · Night", …) |
| Port | RPC port + endpoint inline |
| START / STOP | Launches / kills the configured pipeline |
| Process Control | Per-PID CPU / Memory / GPU rows, total bars |

Process bars colour by threshold: blue < 75 %, yellow 75–94 %,
red >= 95 %. Status row shows `CARLA x.y.z with Unreal N`,
`Not Configured`, or a red mismatch badge.

## Interfaces

Two columns: **Sense** (what the EGO perceives) and **Actuate** (who
drives, and through which API). The cascade gates progressively —
nothing under POV.* / V2X.* is reachable until EGO is configured.

### Sense column

| Row | Notes |
|---|---|
| Vehicle | Blueprint + colour + Substrate finish |
| Camera × N | Per-sensor gear + preview button |
| RADAR × N | Per-sensor gear + preview button |
| LiDAR × N | Per-sensor gear + preview button |
| NAV × N | GNSS / IMU group; gear + preview |
| GT × N | Ground-truth taps; gear + preview |

Each `× N` count reflects how many of that category are mounted on
the EGO. The gear opens the per-sensor editor; preview pops a live
view.

### Actuate column

| Row | Notes |
|---|---|
| EGO | Root of the cascade — picks driver + control method |
| POV.01 .. POV.10 | Other-vehicle slots; chain rooted at EGO |
| V2X.01 .. V2X.06 | Connected-actor slots; chain rooted at EGO |
| Self Drive (SAE) | L0 / L1 / L2 / L3 / L4 / L5 buttons |
| Backend | LibCarla / Python API / ↻ refresh backend |

POV and V2X chains both gate behind EGO — un-configuring EGO greys
out the whole column. The SAE row picks the J3016 level for the
selected actor; the backend row decides whether driving goes through
`libcarla-client` (in-process) or the Python `manual_control.py`
shell-out.

![Interfaces tab](img/cs_tabs_interfaces.png)

## Health Check

Pre-flight probes for the runtime environment. Surfaces problems
before they crash the launcher.

| Probe | Catches |
|---|---|
| CARLA root | Path missing / unreadable |
| Engine | UE4 / UE5 + CARLA version |
| PythonAPI / C++ API | Paired — bindings present and importable |
| Disk free | Low-disk warning |
| NVIDIA driver | Driver / CUDA visibility |
| ROS 2 / ROS-bridge | Paired — distro detected, bridge resolves |
| Scenario Runner | Module importable |
| Leaderboard | Module importable |
| Autoware | Workspace + sourceable setup |
| Plugins | Studio plugin path scan |
| SDK ↔ Sim version | RPC compatibility — pre-empts the crash path |

Footer: `[Cleanup] [Re-check]  Last checked: …  ●OK  ●optional/not
found  ●blocks launch`.

## Scenario Builder

Workflow-numbered groups, top-to-bottom:

| # | Group | What it does |
|---|---|---|
| 1 | Quickstart Examples | `empty_flat.xodr`, `sample_osm_rural.xodr`, `scenario_town01_demo.json` |
| 2 | Map Browser | List / load / reload from sim, install-more shortcut |
| 3 | OpenDRIVE Tools | Browse, offline Validate, Import (`GenerateOpenDriveWorld`), Export (`map.GetOpenDrive()`), OSM → XODR |
| 4 | 2D Map Preview | Topology + spawn-points in a `QGraphicsView` |
| 5 | Scenario File | JSON save / load / Apply (weather + `TrySpawnActor` + `WalkerAIController`) |

## Scenario Re-Construction

NVIDIA reconstruction stack as tiles: NuRec renderer,
Asset-Harvester, NCore, Lyra 2.0. Tile gating reflects local
hardware fitness — tiles that won't run on the detected GPU/driver
combination are visibly disabled with a tooltip pointing at the
failing Health Check probe.

## Leaderboard *(roadmap)*

Source of truth: <https://leaderboard.carla.org/>.

| Control | Purpose |
|---|---|
| Ranking browser | Filter / search the live leaderboard |
| Local agent runner | Pick a Python agent + route, run locally |
| Track 1 / Track 2 | Sensors-only vs privileged selectors |
| Submission helper | Build agent container, push to autograder, monitor |

Driving Score surfaces as Route Completion × Infraction Penalty.

## Testing *(roadmap)*

Two regulator-defined regimes, each rendered as its own QGroupBox
card list. Studio loads the matching scenario, configures actors per
the protocol, runs, scores, and emits a PDF / JSON report.

| Regime | Examples |
|---|---|
| Euro NCAP | AEB CCR, AEB FTAP, AEB VRU, LSS (LKA / LDW / ELK), SAS, DSM |
| NHTSA | FMVSS-126, FMVSS-127, FMVSS-208, FMVSS-214, NCAP Frontal / Side / Rollover, AEB Pedestrian (NCAP 2024+) |

## Logging

CARLA recording / playback, debug visualisation, and the Python API
distribution manager (switch active wheel/egg without leaving the
GUI).

| Group | Notes |
|---|---|
| Recording & Playback | `start_recorder` / `replay_file` wrapper |
| Debug Visualization | World debug helpers |
| Display Settings | Render flags + driver overlays |
| Python distributions | Active CARLA `PythonAPI` wheel/egg picker |
