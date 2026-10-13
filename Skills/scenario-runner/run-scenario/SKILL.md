---
name: run-scenario
description: Runs a single ScenarioRunner Python scenario (or a whole group) against a running CARLA server — FollowLeadingVehicle, ControlLoss, DynamicObjectCrossing, SignalizedJunctionLeftTurn and the rest of srunner/examples — in sync or async mode, with result output, recording and manual/agent control of the ego. Lists what is runnable on the checked-out branch and on the loaded map. Use when the user asks to "run a scenario", "test FollowLeadingVehicle", "list scenarios", "run a CARLA traffic scenario", or reports a scenario that is "not supported".
license: MIT
compatibility: Any OS with a scenario_runner checkout, the CARLA PythonAPI importable, and a running CARLA server. No UE4/UE5 build needed. Scenario availability and town names depend on the branch (master = UE4/0.9.16, ue5-master = UE5/0.10.0, ue58-dev = UE5.8/0.10.0 with towns 1-5 + Town10HD as _Opt).
metadata:
  group: scenario-runner
  prerequisites: scripts/check_env.sh
  reference: references/scenarios.md
---

# Run a scenario

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A "scenario" here is a Python class in `srunner/scenarios/` plus an XML config in
`srunner/examples/` that gives it a town, an ego spawn point and parameters. You
select it by the **config name** (`FollowLeadingVehicle_1`), not the class name.

Two things decide whether a run works, and both are silent when wrong:

- **the branch** — which scenarios and which towns exist at all,
- **the map** — a scenario declares its town, and ScenarioRunner refuses to run if
  the server is on a different one unless you let it reload.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: List what this checkout can run
- [ ] Step 3: Run it, with --reloadWorld unless the map already matches
- [ ] Step 4: Give the ego a driver (manual_control or an agent) if it needs one
- [ ] Step 5: Read the result; reset the world to async when done
```

### Step 1-2: What can I run

```bash
source scripts/env.sh
bash scripts/check_env.sh

python3 scripts/list_scenarios.py                 # every config, its type and town
python3 scripts/list_scenarios.py --town Town04_Opt   # only what fits the loaded map
python3 scripts/list_scenarios.py --check             # configs whose type or town does not resolve
python3 scripts/list_scenarios.py --here          # only what fits the *running* server's map
python3 scripts/list_scenarios.py --types         # class -> configs, incl. route-only classes
```

`list_scenarios.py` parses the XML directly, so it works with no server and no
`carla` import — unlike `scenario_runner.py --list`, which constructs a client
first.

### Step 3: Run

```bash
source scripts/env.sh

# the canonical smoke test
bash scripts/run_scenario.sh FollowLeadingVehicle_1

# every config of one class, in sequence
bash scripts/run_scenario.sh group:FollowLeadingVehicle

# deterministic: 20 Hz synchronous, results printed and written as JSON
SYNC=1 FRAME_RATE=20 OUTPUT=1 JSON=1 OUTPUT_DIR=./results \
    bash scripts/run_scenario.sh ControlLoss_1

# repeat with randomised parameters, and record for later analysis
REPETITIONS=3 RANDOMIZE=1 RECORD=recordings bash scripts/run_scenario.sh DynamicObjectCrossing_1
```

Or call ScenarioRunner directly — the wrapper only adds the env, the map check and
the async reset:

```bash
python3 "$SCENARIO_RUNNER_ROOT/scenario_runner.py" --scenario FollowLeadingVehicle_1 --reloadWorld
```

Knobs (all env vars on `run_scenario.sh`):

| Var | Effect |
|---|---|
| `RELOAD=0` | do **not** reload the world (default reloads; needed when the map differs) |
| `SYNC=1` | synchronous mode at `FRAME_RATE` Hz — required for reproducibility |
| `FRAME_RATE` | sync tick rate, default 20 |
| `OUTPUT=1` / `FILE=1` / `JSON=1` / `JUNIT=1` | result to stdout / .txt / .json / .xml |
| `OUTPUT_DIR` | where those files land |
| `REPETITIONS` / `RANDOMIZE=1` | repeat count / randomise scenario parameters |
| `AGENT` / `AGENT_CONFIG` | drive the ego with an agent (**route mode only** — see below) |
| `RECORD=dir` | CARLA recorder log + a criteria JSON, relative to `$SCENARIO_RUNNER_ROOT` |
| `WAIT_FOR_EGO=1` | attach to an ego someone else spawned instead of spawning one |
| `DEBUG=1` | print the behaviour tree every tick |
| `TIMEOUT` | client timeout, default 120 s — an editor map switch needs it |
| `MAX_WALL` | wall-clock guard, default 1800 s; `0` disables. ScenarioRunner can print its verdict and never exit |

### Step 4: Somebody has to drive

Most scenarios spawn the ego and then **wait for it to move**. Nothing drives it
by itself. If you see

```
Preparing scenario: FollowLeadingVehicle_1
ScenarioManager: Running scenario FollowVehicle
No more scenarios .... Exiting
```

with nothing happening, that is the symptom. Options:

```bash
# keyboard control, in a second terminal
python3 "$SCENARIO_RUNNER_ROOT/manual_control.py"

# or attach to an existing ego by role name
python3 "$SCENARIO_RUNNER_ROOT/manual_control.py" -a --rolename=ego_vehicle
```

`--agent` is **not** accepted with `--scenario` — ScenarioRunner rejects the
combination ("Agents are currently only compatible with route scenarios"). To
have code drive the ego, use [[run-route-scenario]].

### Step 5: Aftermath

`SYNC=1` leaves the server in synchronous mode if the run is interrupted, and a
synchronous world with no ticking client looks **frozen** to every other tool.
`run_scenario.sh` resets it on exit, including on Ctrl-C; if you called
`scenario_runner.py` yourself and killed it, restore async with
[[set-world-settings]] `async`.

## Examples

**Example 1: "run a CARLA scenario to check my setup"**

`check_env.sh`, then `bash scripts/run_scenario.sh FollowLeadingVehicle_1` and
`manual_control.py` in another terminal. Drive forward; the leading car brakes;
the criteria report prints at the end.

**Example 2: "test how my controller handles a pedestrian running out"**

`DynamicObjectCrossing_*` (Town02_Opt–Town05_Opt on ue58-dev; unsuffixed on older
branches — `list_scenarios.py` prints the real town). Run with `SYNC=1 OUTPUT=1 JSON=1`, and
drive the ego with an agent through [[run-route-scenario]] so the run is
reproducible.

**Example 3: "run all the control-loss scenarios and give me the pass/fail"**

`OUTPUT=1 JSON=1 OUTPUT_DIR=./results bash scripts/run_scenario.sh group:ControlLoss`
— configs spread across several towns, each reloading its own map. Summarise with
[[analyze-scenario-results]].

## Troubleshooting

**Problem: `Scenario 'X' not supported ... Exiting`**
Cause: the config name is not in `srunner/examples/` for this branch, or
`SCENARIO_RUNNER_ROOT` is unset so the scenario glob resolves to `./`.
Solution: `python3 scripts/list_scenarios.py` for the real list; export
`SCENARIO_RUNNER_ROOT`. Scenario modules were removed on the UE5 branches
(`change_lane.py`, `cut_in.py`, `freeride.py`, `no_signal_junction_crossing.py`),
so a config naming one of those types cannot run wherever its XML survives.
`--check` is the fast way to see it.

**Problem: `The CARLA server uses the wrong map: TownXX / This scenario requires to use map: TownYY`**
Cause: `RELOAD=0` (or no `--reloadWorld`) with a mismatched map.
Solution: drop `RELOAD=0`, or load the map first with [[load-map]].

**Problem: nothing moves, run exits immediately**
Cause: no driver for the ego (see Step 4).
Solution: `manual_control.py`, or route mode with an agent.

**Problem: a config is listed but the run dies constructing the scenario**
Cause: the XML names a `type` with no scenario class behind it. The name may still
*resolve* to something that is not a scenario — an atomic behaviour of the same
name imported into a scenario module's namespace will be constructed and fail with
`<Type>.__init__() got an unexpected keyword argument 'world'`.
Solution: `python3 scripts/list_scenarios.py --check` lists every config whose
type or town does not resolve. It should report zero; a non-zero count means an
XML references a class that is not there.

**Problem: the world is frozen after a run; other clients time out**
Cause: interrupted `SYNC=1` run left synchronous mode on.
Solution: [[set-world-settings]] `async`. Prevent it by using `run_scenario.sh`,
which traps EXIT.

**Problem: `RuntimeError: Timeout occurred during scenario execution`**
Cause: the watchdog fired — the ego never reached a trigger, or the server
stalled.
Solution: raise `TIMEOUT`, check the ego is actually being driven, and confirm
the server is not rendering at 2 fps (`-nullrhi`/`-quality-level=Low` help).

**Problem: `AttributeError` in py_trees, or unicode errors with `DEBUG=1`**
Cause: py_trees 2.x, or non-UTF-8 stdout.
Solution: `pip install py-trees==0.8.3`; `env.sh` already exports
`PYTHONIOENCODING=utf-8`.

## Outputs

A scenario executed on the server, a criteria pass/fail report on stdout (with
`OUTPUT=1`) and optional `.txt`/`.json`/`.xml` result files in `OUTPUT_DIR`. With
`RECORD=dir`, a CARLA recorder log plus a criteria JSON under
`$SCENARIO_RUNNER_ROOT/dir/`, replayable with [[replay-recording]] and analysable
with [[analyze-scenario-results]].

The full inventory — every scenario type, its configs, its towns, and which
classes are route-only — is in [references/scenarios.md](references/scenarios.md).
