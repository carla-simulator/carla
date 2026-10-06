# ScenarioRunner branches, and what actually differs

Everything here was read off the repository (`carla-simulator/scenario_runner`)
rather than the docs, because the published docs are years behind the code.

## The map

```
                       0.9.x tags (frozen)
                              |
  master ────────────────────────────────────────────── 0.9.16 (Sep 2025)   <- live
     |
     ├── ue5-master  (forked 2024-06-26, 5 commits)      CARLA 0.10.0 / UE5
     |
     ├── leaderboard-2.0 ─┐ same commit d7bcaf0 (May 2024)
     ├── leaderboard-2.1 ─┘  Leaderboard 2.0 and 2.1
     |
     ├── leaderboard-1.0  (Jun 2023)                     Leaderboard 1.0
     └── leaderboard      (Sep 2020)                     legacy, superseded
```

| Branch | Tip | CARLA | Purpose |
|---|---|---|---|
| `master` | `94ff3b8` 2025-09-29 "0.9.16 release" | 0.9.14 – 0.9.16 | the active branch |
| `ue5-master` | `2616d21` 2024-10-23 | 0.10.0 | UE5 port + Scenic |
| `leaderboard-2.0` | `d7bcaf0` 2024-05-03 | leaderboard build | LB 2.0 |
| `leaderboard-2.1` | `d7bcaf0` 2024-05-03 | leaderboard build | LB 2.1 — **identical to 2.0** |
| `leaderboard-1.0` | `b9c342e` 2023-06-15 | 0.9.10.1 | LB 1.0 |
| `leaderboard` | `95366ad` 2020-09-30 | 0.9.10 | LB 1.0, one watchdog fix behind |

`leaderboard-2.0` and `leaderboard-2.1` resolve to the same commit —
`git diff origin/leaderboard-2.0 origin/leaderboard-2.1` is empty. The
leaderboard *repo* is what differs between 2.0 and 2.1; ScenarioRunner is not.
The official 2.1 instructions say to clone `-b leaderboard-2.1`, so do that for
clarity, but a `leaderboard-2.0` checkout is not a real mismatch.

`leaderboard` vs `leaderboard-1.0` differ by **two lines** in
`srunner/scenariomanager/watchdog.py`. Prefer `leaderboard-1.0`; the official
1.0 page still says `-b leaderboard`, which predates that fix by three years.

## master (CARLA 0.9.14 – 0.9.16)

- `MIN_CARLA_VERSION = '0.9.14'`, enforced as an `ImportError` in
  `ScenarioRunner.__init__` against the **client package metadata**.
- Modes: `--scenario` (Python), `--openscenario` (.xosc), `--openscenario2`
  (.osc / OpenSCENARIO 2.0), `--route` (route + embedded scenarios).
- ~55 scenario classes in `srunner/scenarios/`, of which **21 types have example
  configs** in `srunner/examples/*.xml`. The other ~34 are route-only: they take
  their placement from a route file's `<scenarios>` block and cannot be launched
  with `--scenario`.
- `_cleanup()` has `self.manager.cleanup()` and `CarlaDataProvider.cleanup()`
  **commented out** (they run on `ue5-master`). Actors from a crashed run can
  therefore survive into the next one; reload the world between runs.
- Requirements pin `py-trees==0.8.3`, `numpy==1.24.4`, `networkx==3.4.2`,
  `Shapely==2.1.1`, `xmlschema==1.0.18`, `opencv-python==4.7.0.72`,
  `antlr4-python3-runtime==4.10`, plus `simple-watchdog-timer`.

### Known stale/broken bits in master

- `CARLA_VER` still says `RELEASE=CARLA_0.9.13` (used by the Jenkinsfile) while
  the branch is 0.9.16.
- `srunner/examples/HighwayCutIn.xml` has `town="Highway"` — no such map exists,
  so `--scenario HighwayCutIn_1` cannot load. Use the route-based `HighwayCutIn`
  instead.
- `--reloadWorld`'s help text says "(default=True)"; it is `store_true`, so the
  default is **False**. It is forced to True only in `--route` mode.

## ue5-master (CARLA 0.10.0 / UE5)

Forked from `master` at `7758d06` (2024-06-26) and never merged back, so it is
missing ~15 months of master fixes. Five commits on top:

| Commit | Effect |
|---|---|
| `207d7e1` | requirements unpinned; `simple-watchdog-timer` dropped |
| `fa2ffa0` | **weather control disabled** — `_create_weather_behavior()` and `world.set_weather()` commented out in `basic_scenario.py` |
| `00eaa7b` | scenario configs retargeted to 0.10.0 |
| `6884200`, `2616d21` | `srunner/scenic/` — Scenic models, CARLA interface and the 10 `carlaChallenge*.scenic` files |

What changes for a user:

- **The port is partial.** CARLA 0.10.0 ships `Town10HD_Opt` only, and of the
  **101** configs in `srunner/examples/` just **11** were retargeted to it:
  `BlockedIntersection_{1,2}`, `ConstructionObstacle_{1,2}`, `ControlLoss_1`,
  `OppositeVehicleRunningRedLight_1`, `ParkingExit_{1,2}`,
  `SignalizedJunctionLeftTurn_1`, `StaticCutIn_1`, `VehicleOpensDoorTwoWays_1`.
  The other 90 still name Town01–Town05 and **cannot load** — the map does not
  exist. `list_scenarios.py --town Town10HD_Opt` prints the usable set.
- **Blueprint renames.** `vehicle.lincoln.mkz` replaces `vehicle.lincoln.mkz_2017`;
  `vehicle.ambulance.ford` is new. The retargeted configs use the new name; the
  stale ones still say `mkz_2017`.
- **Weather is inert.** `_create_weather_behavior()` and `world.set_weather()` are
  commented out, so a scenario's `<weather>` block has no effect. Set weather
  yourself ([[set-weather]]) if a run depends on it.
- **Deleted scenarios**: `change_lane.py`, `cut_in.py`, `freeride.py`,
  `no_signal_junction_crossing.py`.
- **Dangling configs**: `ChangeLane.xml` and `CutIn.xml` survive although their
  classes were deleted, so `ChangeLane_1`, `ChangeLane_2`, `CutInFrom_left_Lane`
  and `CutInFrom_right_Lane` are listed and fail with "not supported". Not a
  setup error on your side; `list_scenarios.py --check` flags all five broken
  configs (those four plus `HighwayCutIn_1`).
- `srunner/examples/` was split: `.xosc`/`.osc` moved to `srunner/osc_examples/`,
  so OpenSCENARIO paths from master's docs are wrong here.
- Cleanup is enabled (`manager.cleanup()`, `CarlaDataProvider.cleanup()` active).

## The leaderboard branches

They are frozen snapshots whose scenario library matches a leaderboard release.
Do not use `master` with the leaderboard: the route-scenario classes, their
parameter names and the criteria differ, and the mismatch surfaces as scenarios
that never trigger rather than as an error.

`leaderboard-2.x` route scenarios (Town12/Town13) that exist there and on master
but **not** as standalone configs include `Accident*`, `ParkedObstacle*`,
`HazardAtSideLane*`, `ConstructionObstacle*`, `InvadingTurn`, `HardBreakRoute`,
`YieldToEmergencyVehicle`, `VehicleTurningRoute*`, `PriorityAtJunction`,
`NonSignalizedJunction*`, `Interurban*ActorFlow`, `MergerIntoSlowTraffic*`,
`HighwayExit`, `CrossingBicycleFlow`, `ParkingExit`, `ParkingCutIn`,
`ParkingCrossingPedestrian`, `StaticCutIn`, `BlockedIntersection`,
`PedestrianCrossing`, `OppositeVehicleTakingPriority`.

## Python and OS

- `py_trees==0.8.3` has no wheels for 3.11+ metadata in some environments and the
  API is the constraint anyway: **3.7 – 3.10** is the safe window, matching the
  CARLA client wheels (0.9.16 ships cp310/311/312; 0.9.15 cp37–cp310).
- The docs' Ubuntu 16.04 / Python 2.7 instructions are obsolete. Python 2 is dead
  in this codebase in practice — the `from distutils.version import LooseVersion`
  fallback is the only remnant.
