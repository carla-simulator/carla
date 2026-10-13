---
name: install-scenario-runner
description: Installs CARLA's ScenarioRunner (the official scenario engine) and picks the branch that matches the CARLA in use — master for 0.9.14-0.9.16 (UE4), ue5-master for 0.10.0 (UE5), leaderboard-1.0/2.0/2.1 when driving the Leaderboard. Clones or checks out the branch, installs requirements, wires PYTHONPATH (including the `agents` package the carla wheel does not ship), and verifies the import. Use when the user asks to "install scenario runner", "set up SR", "run CARLA scenarios", "use OpenSCENARIO", or has a scenario_runner that fails to import.
license: MIT
compatibility: Linux/Windows, Python 3.7-3.10 (py_trees 0.8 pins the ceiling). Needs an extracted CARLA release or source checkout for the `agents` package; does NOT need UE4_ROOT. Verified against scenario_runner master @0.9.16 and ue5-master.
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

| Your CARLA | ScenarioRunner branch | Notes |
|---|---|---|
| 0.9.14 / 0.9.15 / 0.9.16 (UE4) | `master` | the live branch; 0.9.16 released Sep 2025 |
| 0.10.0 / 1.0 (UE5) | `ue5-master` | Town10HD_Opt **only**; 11 of 101 configs ported; forked Jun 2024. The UE5 line reports `0.10.0` before the 1.0 release and `1.x` after; the same branch is the match for both until scenario_runner publishes a 1.0 branch — check its branch list first |
| leaderboard build (0.9.14+large maps) | `leaderboard-2.1` *or* `leaderboard-2.0` | **byte-identical branches** — same commit |
| 0.9.10.1 | `leaderboard-1.0` | Leaderboard 1.0 only |
| 0.9.13 and older | the matching `0.9.x` tag/branch | frozen, unsupported |

The minimum enforced in code is `MIN_CARLA_VERSION = '0.9.14'` on `master`, and
it is a hard `ImportError` at startup, not a warning. `0.10.0` passes that check
because version comparison is numeric per component (`0.10 > 0.9`), and `1.x`
passes it on the major alone.

### Step 3: Clone

```bash
# UE4 / CARLA 0.9.16
git clone -b master --single-branch https://github.com/carla-simulator/scenario_runner.git

# UE5 / CARLA 0.10.0
git clone -b ue5-master --single-branch https://github.com/carla-simulator/scenario_runner.git

# driving the Leaderboard (pick the branch that matches your leaderboard checkout)
git clone -b leaderboard-2.1 --single-branch https://github.com/carla-simulator/scenario_runner.git
```

`--single-branch` is worth it: the repo carries the OpenSCENARIO 2.0 test corpus
and, on `ue5-master`, ~230k lines of Scenic OpenDRIVE assets.

Already cloned? Switch instead of re-cloning:

```bash
python3 scripts/install_scenario_runner.py install --root ~/scenario_runner --branch ue5-master
```

### Step 4: Requirements

```bash
# into the SAME interpreter that imports carla — not a fresh venv unless carla is there too
python3 -m pip install -r ~/scenario_runner/requirements.txt
```

`py_trees==0.8.3` is pinned and **not optional**: the behaviour-tree API changed
in 2.x and scenarios fail with attribute errors. `numpy` must stay `<2` for the
same reason the CARLA client does — the bindings are built against the 1.x C API.

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

**Example 2: "I have CARLA 0.10.0 and scenarios don't work"**

Almost certainly `master` checked out against a UE5 server. `install --branch
ue5-master` switches it. Note the knock-on effects: 0.10.0 ships `Town10HD_Opt`
only and just **11 of the 101 configs** were retargeted to it, the ego blueprint
is `vehicle.lincoln.mkz` (**not** `mkz_2017`), and weather behaviours are
commented out in `basic_scenario.py`. `list_scenarios.py --town Town10HD_Opt`
prints the runnable set.

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
Cause: usually the wrong branch — `ue5-master` deleted `change_lane.py`,
`cut_in.py`, `freeride.py` and `no_signal_junction_crossing.py`. Can also be
`SCENARIO_RUNNER_ROOT` unset, which makes the scenario glob resolve to `./`.
Solution: export `SCENARIO_RUNNER_ROOT`; confirm the branch with
`git -C $SCENARIO_RUNNER_ROOT rev-parse --abbrev-ref HEAD`.

**Problem: scenario starts, then `AttributeError` inside py_trees**
Cause: py_trees 2.x.
Solution: `pip install py-trees==0.8.3`.

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
