---
name: install-scenario-runner
description: Installs CARLA's ScenarioRunner (the official scenario engine) and picks the branch that matches the CARLA in use, defaulting to ue58-dev (CARLA on Unreal Engine 5.8, the 1.0 line): master for 0.9.14-0.9.16 (UE4), ue5-master for the 0.10.0 release on UE 5.5, leaderboard-1.0/2.0/2.1 when driving the Leaderboard. Clones or checks out the branch, installs requirements, wires PYTHONPATH (including the `agents` package the carla wheel does not ship), and verifies the import. Use when the user asks to "install scenario runner", "set up SR", "run CARLA scenarios", "use OpenSCENARIO", or has a scenario_runner that fails to import.
license: MIT
compatibility: Linux/Windows, Python 3.7-3.10 for master and the leaderboard branches (py_trees 0.8 pins the ceiling); ue58-dev uses py-trees 2.6.0 and ran on Python 3.13. Needs an extracted CARLA release or source checkout for the `agents` package; does NOT need UE4_ROOT. Verified against scenario_runner master @0.9.16, ue5-master @2616d21 and ue58-dev @5fd2880.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/versions.md
---

# Install ScenarioRunner

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

ScenarioRunner is pure Python — **there is nothing to build**. Installing it is
three decisions and one `pip install`:

1. **which branch** matches your CARLA (this is the whole problem),
2. where the checkout lives (`SCENARIO_RUNNER_ROOT`),
3. what goes on `PYTHONPATH`.

Get the branch wrong and the failure is not a version warning — it is
`Scenario 'X' not supported`, a missing map, or a scenario that spawns nothing,
because the scenario classes, the town names and the vehicle blueprints all
changed between branches.

## Instructions

```
Progress:
- [ ] Step 1: Identify the CARLA version in play
- [ ] Step 2: Pick the branch from the matrix
- [ ] Step 3: Clone or check out that branch
- [ ] Step 4: Install requirements into the interpreter that has `carla`
- [ ] Step 5: Record `SCENARIO_RUNNER_ROOT` with `set_config`, then verify (bash scripts/check_env.sh)
```

### Step 1-2: Which branch

```bash
python3 scripts/install_scenario_runner.py detect     # what CARLA is here?
python3 scripts/install_scenario_runner.py plan       # -> branch + exact commands
```

`detect` reads the installed `carla` client, the running server if there is one,
and any `SCENARIO_RUNNER_ROOT` already present. `plan` turns that into a branch
recommendation and prints the commands rather than running them.

`ue58-dev` is **the default**: it is the branch under active development and the
one to assume unless the CARLA in use says otherwise. The rest are still
supported and still used, but they are not the default.

| Your CARLA | ScenarioRunner branch | Notes |
|---|---|---|
| 1.x (CARLA 1.0, UE 5.8) | **`ue58-dev`** (default) | Town01_Opt to Town05_Opt and Town10HD_Opt; `--list` shows 96 configs; UE5 map-name and blueprint-name translation; route-mode fixes; py-trees 2.6; `srunner/scenic/` included |
| 0.9.14 / 0.9.15 / 0.9.16 (UE4) | `master` | the UE4 line; 0.9.16 released Sep 2025 |
| 0.10.x (UE 5.5) | `ue5-master` | Town10HD_Opt **only**; 11 of 101 configs ported; forked Jun 2024 |
| leaderboard build (0.9.14+large maps) | `leaderboard-2.1` *or* `leaderboard-2.0` | **byte-identical branches** — same commit |
| 0.9.10.1 | `leaderboard-1.0` | Leaderboard 1.0 only |
| 0.9.13 and older | the matching `0.9.x` tag/branch | frozen, unsupported |

**`ue58-dev` is the CARLA 1.0 branch**, and a 1.0 server resolves straight to it.
`0.10.x` is the UE 5.5 release and resolves to `ue5-master`. An unreadable version
gives `ue58-dev`, the default.
An unreadable version gives `ue58-dev` too.

The minimum enforced in code is a hard `ImportError` at startup, not a warning:
`MIN_CARLA_VERSION = '1.0'` on `ue58-dev` and `'0.9.14'` on `master`. A `0.10.0`
server is rejected by `ue58-dev` outright — that is the UE 5.5 release, and
`ue5-master` is its branch.

### Step 3: Clone

```bash
# CARLA on UE 5.8 (the 1.0 line) — the default
git clone -b ue58-dev --single-branch https://github.com/carla-simulator/scenario_runner.git

# UE4 / CARLA 0.9.16
git clone -b master --single-branch https://github.com/carla-simulator/scenario_runner.git

# CARLA 0.10.0 release on UE 5.5
git clone -b ue5-master --single-branch https://github.com/carla-simulator/scenario_runner.git

# driving the Leaderboard (pick the branch that matches your leaderboard checkout)
git clone -b leaderboard-2.1 --single-branch https://github.com/carla-simulator/scenario_runner.git
```

`--single-branch` is worth it: the repo carries the OpenSCENARIO 2.0 test corpus
and, on `ue5-master` and `ue58-dev`, the Scenic assets under `srunner/scenic/`.

Already cloned? Switch instead of re-cloning:

```bash
python3 scripts/install_scenario_runner.py install --root ~/scenario_runner --branch ue58-dev
```

### Step 4: Requirements

```bash
# into the SAME interpreter that imports carla — not a fresh venv unless carla is there too
python3 -m pip install -r ~/scenario_runner/requirements.txt
```

The `py-trees` pin is **not optional** and differs by branch: `ue58-dev` pins
`2.6.0`, master and the leaderboard branches pin `0.8.3`. The behaviour-tree API
changed in 2.x, so a scenario written for one major version fails with attribute
or type errors on the other. Always install from the checkout's own
`requirements.txt` rather than a remembered version. With a CARLA client
before 1.0, `numpy` must stay `<2`: those bindings fail under numpy 2.
CARLA 1.0 and later support numpy 2 (Boost 1.90.0, #9737).

### Step 5: Environment

```bash
export SCENARIO_RUNNER_ROOT=~/scenario_runner
export CARLA_ROOT=/path/to/CARLA_0.9.16          # the extracted release or checkout
export PYTHONPATH="${SCENARIO_RUNNER_ROOT}:${CARLA_ROOT}/PythonAPI/carla:${PYTHONPATH}"

bash scripts/check_env.sh
python3 scripts/install_scenario_runner.py verify
```

**`${CARLA_ROOT}/PythonAPI/carla` is required even when `carla` is pip-installed.**
`scenario_runner.py` imports `agents.navigation.global_route_planner` at module
load, and the `agents` package ships only inside a CARLA tree — never in the
wheel. A pip-only setup dies with `ModuleNotFoundError: No module named 'agents'`
before any scenario runs. This is the single most common install failure.

`SCENARIO_RUNNER_ROOT` is also read *at runtime* by ScenarioRunner itself, to
locate `srunner/scenarios/*.py` and to resolve `--record` paths. Exporting it is
not just for the shell's benefit.

### Recording the path

An `export` lasts until the shell exits. Persist `SCENARIO_RUNNER_ROOT` instead, so the
next session — and `list_skills` — still knows where this went:

```
set_config({"SCENARIO_RUNNER_ROOT": "<the checkout path>"})
```

Without it the group this just enabled keeps reporting `available: false`,
and the next skill re-detects from scratch. `CARLA_ROOT` is the only CARLA
path to record: `set_config` derives the engine-specific variable itself.

## Examples

**Example 1: "install scenario runner for my CARLA 0.9.16"**

`detect` → client 0.9.16 → `plan` says `master`. Clone `-b master`, pip install
requirements, export the three variables, `check_env.sh` passes, then run
[[run-scenario]] `--list`.

**Example 2: "I have a UE5 server and scenarios don't work"**

Almost certainly `master` checked out against a UE5 server. The version says
which one: `1.x` is CARLA 1.0 on UE 5.8, `0.10.x` is the UE 5.5 release.

- CARLA 1.0: `install --branch ue58-dev`. Its configs use Town01_Opt to Town05_Opt
  and Town10HD_Opt, and it translates UE5 map and blueprint names.
- UE 5.5 release: `install --branch ue5-master`. That release ships
  `Town10HD_Opt` only and just **11 of the 101 configs** were retargeted to it;
  the ego blueprint is `vehicle.lincoln.mkz` (**not** `mkz_2017`).

On both branches the weather call in `basic_scenario.py` is commented out.
`list_scenarios.py --town Town10HD_Opt` prints the runnable set.

**Example 3: "set up SR to run the leaderboard"**

Do it the other way round — start from [[install-leaderboard]], which detects the
leaderboard version and names the SR branch to pair with it. Installing SR first
means guessing.

## Troubleshooting

**Problem: `ModuleNotFoundError: No module named 'agents'`**
Cause: `carla` came from a wheel; `agents` only exists in a CARLA tree.
Solution: set `CARLA_ROOT` and add `${CARLA_ROOT}/PythonAPI/carla` to `PYTHONPATH`.

**Problem: `ImportError: CARLA version 0.9.14 or newer required`**
Cause: `master` against an older client. The check reads the *client* package
metadata, not the server.
Solution: upgrade the client ([[install-python-api]]), or check out the branch for
your version.

**Problem: `Scenario 'FollowLeadingVehicle_1' not supported ... Exiting`**
Cause: usually the wrong branch, e.g. `master` against a UE5 server, or
`ue5-master` against a UE 5.8 one (use `ue58-dev` there). `ue5-master` deleted `change_lane.py`,
`cut_in.py`, `freeride.py` and `no_signal_junction_crossing.py`. Can also be
`SCENARIO_RUNNER_ROOT` unset, which makes the scenario glob resolve to `./`.
Solution: export `SCENARIO_RUNNER_ROOT`; confirm the branch with
`git -C $SCENARIO_RUNNER_ROOT rev-parse --abbrev-ref HEAD`.

**Problem: a CARLA 1.0 tree, but `plan` recommends `ue5-master`**
Cause: the server it read reports `0.10.x`, which is the UE 5.5 release.
Solution: point it at the 1.0 server, or force it with
`plan --branch ue58-dev` / `install --branch ue58-dev`.

**Problem: scenario starts, then `AttributeError` or `TypeError` inside py_trees**
Cause: the installed py_trees does not match the branch's pin — 2.6.0 on
`ue58-dev`, 0.8.3 on the others.
Solution: `pip install -r $SCENARIO_RUNNER_ROOT/requirements.txt`.

**Problem: `UnicodeEncodeError: 'ascii' codec can't encode character '✓'`**
Cause: `--debug` prints py_trees tick marks to a non-UTF-8 stdout (docker, cron).
Solution: `export PYTHONIOENCODING=utf-8` — `scripts/env.sh` does this for you.

**Problem: two CARLA versions on one machine, SR picks the wrong one**
Cause: `PYTHONPATH` order, or a stale egg earlier in the path.
Solution: one venv per CARLA version; `python3 -c "import carla; print(carla.__file__)"`
tells you which one actually won.

## Outputs

A `scenario_runner` checkout on a known branch, its requirements installed in the
interpreter that has `carla`, and the three exports needed to run it. `verify`
prints the branch, the client/server versions and the scenario count it can see.

Version details — what each branch contains, what UE5 dropped, which branch the
Leaderboard needs — in [references/versions.md](references/versions.md).
