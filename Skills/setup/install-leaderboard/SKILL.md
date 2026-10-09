---
name: install-leaderboard
description: Installs the CARLA Autonomous Driving Leaderboard (leaderboard.carla.org) at a chosen version — 1.0, 2.0 or 2.1 — together with the exact scenario_runner branch and CARLA build each one requires, then wires the four-root PYTHONPATH and verifies the whole stack imports. Detects which version an existing checkout is by reading its code, not its branch name. Use when the user asks to "install the leaderboard", "set up the CARLA challenge", "evaluate my driving agent", "reproduce my leaderboard score", or names a leaderboard version.
license: MIT
compatibility: Linux, Python 3.7-3.10. Needs a CARLA build matching the leaderboard version (0.9.10.1 for 1.0; the leaderboard 0.9.14+large-maps build for 2.x) plus a matching scenario_runner checkout. No UE4/UE5 build required.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/versions.md
---

# Install the CARLA Leaderboard

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The Leaderboard is an evaluation harness layered on ScenarioRunner: it supplies
routes, a fixed sensor budget, infraction criteria and a scoring formula, and it
imports `srunner` for the scenarios themselves. So a working install is **four
roots that agree on one version**:

```
CARLA build  ──▶  scenario_runner branch  ──▶  leaderboard branch  ──▶  your agent
```

Get any pair out of step and nothing tells you so directly — routes load and then
scenarios never trigger, or the score comes out different from the online one.

## Instructions

```
Progress:
- [ ] Step 1: Decide the leaderboard version (1.0 / 2.0 / 2.1)
- [ ] Step 2: Get the CARLA build that version needs
- [ ] Step 3: Clone leaderboard + the matching scenario_runner branch
- [ ] Step 4: Install both requirement sets
- [ ] Step 5: Record the roots with `set_config`, then verify (bash scripts/check_env.sh)
```

### Step 1: Which version

| | LB 1.0 | LB 2.0 | LB 2.1 |
|---|---|---|---|
| Status | legacy | superseded | **current** (since Mar 2025) |
| CARLA | 0.9.10.1 | leaderboard build (0.9.14 + large maps) | same as 2.0 |
| `leaderboard` branch | `leaderboard-1.0` | `leaderboard-2.0` | `leaderboard-2.1` |
| `scenario_runner` branch | `leaderboard-1.0` | `leaderboard-2.0` | `leaderboard-2.1` |
| Towns | Town01–Town06 | Town12 / Town13 | Town12 / Town13 |
| Routes | 50 train / 26 test / 4 devtest | 90 train / 20 val / 2 devtest | same as 2.0 |
| Tracks | SENSORS, MAP | + SENSORS_QUALIFIER, MAP_QUALIFIER | same as 2.0 |
| Sensor budget | 4 cam / 1 lidar / 2 radar | 8 cam / 2 lidar / 4 radar | same as 2.0 |
| Infraction penalty | multiplicative | multiplicative | **additive** |

**2.0 and 2.1 differ in exactly one file.** `leaderboard/utils/statistics_manager.py`
— 38 lines. Same routes, same scenarios, same CARLA, same sensors. Only the
maths that turns infractions into a penalty changed. Detail in
[references/versions.md](references/versions.md).

**`master` is the 2.0 line, not 2.1.** Its penalty table still has
`PENALTY_PERC_DICT` and the 0.5/0.6/0.65 multiplicative coefficients. If you
want to reproduce a score from the live leaderboard, check out `leaderboard-2.1`.

```bash
python3 scripts/install_leaderboard.py detect   # what is here, and which version is it?
python3 scripts/install_leaderboard.py plan --version 2.1
```

`detect` identifies the version of any existing checkout **from its code** —
`SENSORS_QUALIFIER` in `autonomous_agent.py` separates 2.x from 1.0, and
`PENALTY_PERC_DICT` in `statistics_manager.py` separates 2.0 from 2.1 — so it is
right even for a tarball, a docker image or a detached HEAD.

### Step 2: CARLA

LB 2.x does **not** run on a stock release. It needs the leaderboard build, whose
Python client reports the literal version string `leaderboard` (the evaluator
special-cases it and skips its own minimum-version check). Get it from
[leaderboard.carla.org](https://leaderboard.carla.org/get_started_v2_1/).

A stock 0.9.15/0.9.16 with **AdditionalMaps** installed gets you Town12/Town13
and will run routes, which is fine for developing an agent — but it is not the
evaluation environment, so treat any score from it as indicative only.

LB 1.0 needs 0.9.10.1 exactly; [[download-carla]] can fetch it.

### Step 3-4: Clone and install

```bash
# LB 2.1 (current)
git clone -b leaderboard-2.1 --single-branch https://github.com/carla-simulator/leaderboard.git
git clone -b leaderboard-2.1 --single-branch https://github.com/carla-simulator/scenario_runner.git

# LB 1.0
git clone -b leaderboard-1.0 --single-branch https://github.com/carla-simulator/leaderboard.git
git clone -b leaderboard-1.0 --single-branch https://github.com/carla-simulator/scenario_runner.git

python3 -m pip install -r leaderboard/requirements.txt -r scenario_runner/requirements.txt
```

Or in one step, which also switches an existing pair of checkouts:

```bash
python3 scripts/install_leaderboard.py install --version 2.1 --dir ~/carla-lb
```

`leaderboard/requirements.txt` pins `opencv-python==4.2.0.32`, which has no
wheels for Python 3.9+. If pip tries to build it from source and fails, install a
newer OpenCV instead — nothing in the leaderboard depends on that exact version
(it is used only by `human_agent.py` for the HUD).

### Step 5: Environment

```bash
export CARLA_ROOT=~/CARLA_Leaderboard_2.0
export SCENARIO_RUNNER_ROOT=~/carla-lb/scenario_runner
export LEADERBOARD_ROOT=~/carla-lb/leaderboard
export PYTHONPATH="${CARLA_ROOT}/PythonAPI/carla":"${SCENARIO_RUNNER_ROOT}":"${LEADERBOARD_ROOT}":"${CARLA_ROOT}/PythonAPI/carla/dist/carla-0.9.14-py3.7-linux-x86_64.egg":${PYTHONPATH}

bash scripts/check_env.sh
```

Order is the official one from `scripts/Dockerfile.master`. Two traps:

- `${CARLA_ROOT}/PythonAPI/carla` is mandatory — `srunner` imports
  `agents.navigation.global_route_planner`, and `agents` exists only in a CARLA
  tree, never in the `carla` wheel.
- The `.egg` filename embeds the version and Python minor. Glob it rather than
  copying the line: `ls ${CARLA_ROOT}/PythonAPI/carla/dist/`.

`scripts/env.sh` builds all of this and prints the detected version, so
`source scripts/env.sh` is the shortcut.

### Recording the path

An `export` lasts until the shell exits. Persist `LEADERBOARD_ROOT` and `SCENARIO_RUNNER_ROOT` instead, so the
next session — and `list_skills` — still knows where this went:

```
set_config({"LEADERBOARD_ROOT": "<the leaderboard path>",
            "SCENARIO_RUNNER_ROOT": "<the scenario_runner path>"})
```

Without it the group this just enabled keeps reporting `available: false`,
and the next skill re-detects from scratch. `CARLA_ROOT` is the only CARLA
path to record: `set_config` derives the engine-specific variable itself.

## Examples

**Example 1: "set up the leaderboard so I can evaluate my agent"**

`detect` → nothing installed. `install --version 2.1` clones both repos at
`leaderboard-2.1`, installs requirements, prints the exports. Then
[[write-leaderboard-agent]] for the agent, [[run-leaderboard-evaluation]] to run
`routes_devtest.xml`.

**Example 2: "my scores don't match what the leaderboard gave me"**

Check the version first: `detect`. A `master` or `leaderboard-2.0` checkout scores
multiplicatively, the live 2.1 leaderboard scores additively — the same run yields
different numbers. Switch with `install --version 2.1`, or recompute from the
existing `results.json` with [[read-leaderboard-results]] `--as 2.1`.

**Example 3: "I want to run the old 1.0 challenge routes"**

`install --version 1.0`, plus CARLA 0.9.10.1 via [[download-carla]]. Expect API
breaks in your agent: in 1.0 `AutonomousAgent.__init__` takes
`path_to_conf_file` and calls `setup()` itself; in 2.x it takes
`(carla_host, carla_port, debug)` and the evaluator calls `setup()`.

## Troubleshooting

**Problem: `ModuleNotFoundError: No module named 'srunner'`**
Cause: `SCENARIO_RUNNER_ROOT` not on `PYTHONPATH`.
Solution: `source scripts/env.sh`, or add it by hand.

**Problem: `ModuleNotFoundError: No module named 'agents'`**
Cause: `carla` from a wheel; `agents` lives in the CARLA tree.
Solution: add `${CARLA_ROOT}/PythonAPI/carla`.

**Problem: routes load but no scenario ever triggers**
Cause: `scenario_runner` on `master` instead of a `leaderboard-*` branch. The
route-scenario classes and their parameters differ.
Solution: `install --version <yours>` fixes the pairing; `check_env.sh` fails
loudly on it.

**Problem: `Exception: The CARLA server uses the wrong map!`**
Cause: Town12/Town13 missing — stock release without AdditionalMaps.
Solution: install AdditionalMaps ([[download-carla]]) or use the leaderboard build.

**Problem: `pip install opencv-python==4.2.0.32` fails to build**
Cause: no wheel for your Python; pip falls back to a source build.
Solution: install a current `opencv-python` instead; only the human agent's HUD
uses it.

**Problem: `ImportError: CARLA version 0.9.14 or newer required`**
Cause: an older client than the evaluator's minimum.
Solution: match the client to the leaderboard build ([[install-python-api]]).

## Outputs

A `leaderboard` checkout at a known version, a `scenario_runner` checkout on the
paired branch, requirements installed, and the four exports. `check_env.sh` prints
the detected version, the pairing verdict and whether Town12/Town13 exist on the
running server.

Version-by-version differences — scoring maths, tracks, sensor budgets, route
formats, agent API breaks — in [references/versions.md](references/versions.md).
