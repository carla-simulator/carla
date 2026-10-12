---
name: run-scenic-scenario
description: Runs Scenic probabilistic scenarios against a running CARLA server with the `scenic FILE --simulate --2d` CLI — the carlaChallenge1-10 traffic scenarios shipped by ScenarioRunner (`model srunner.scenic.models.model`) and by Scenic itself (`model scenic.simulators.carla.model`) — bounding the run, sampling scenes headlessly first to tell a map problem from a syntax problem, and restoring the world afterwards. Use when the user asks to "run a Scenic scenario", "run carlaChallenge", "test a .scenic file", "scenic --simulate", or reports a Scenic scenario that rejects every sample or cannot find blueprints.
license: MIT
compatibility: Any OS with Scenic and a matching CARLA client in ONE interpreter, plus a running CARLA server. No UE4/UE5 build needed. Scenic keys its blueprint tables off the client version, so client and server must match exactly.
metadata:
  group: scenic
  prerequisites: scripts/check_env.sh
  reference: references/scenic-cli.md
---

# Run a Scenic scenario

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A Scenic scenario is a `.scenic` file describing a *distribution* over scenes.
Running it has two distinct phases, and almost every confusing failure comes from
not knowing which one broke:

1. **sample** — Scenic reads the map's OpenDrive, then rejection-samples until a
   scene satisfies every `require`. No simulator involved.
2. **simulate** — the sampled scene is spawned in CARLA and behaviours tick.

A scenario that cannot sample will never reach CARLA, so a server-side
explanation for it is always wrong. [scripts/sample_scenic.py](scripts/sample_scenic.py)
separates the two in about a second per file.

Three things decide whether a run works:

- **the model** — `srunner.scenic.models.model` needs `SCENARIO_RUNNER_ROOT` on
  `PYTHONPATH`; `scenic.simulators.carla.model` ships in the wheel.
- **the map** — scenarios filter road features (`is4Way and not isSignalized`).
  A filter that matches nothing is an empty domain, not a bug.
- **the client version** — Scenic's blueprint table is keyed on it, so a
  client/server mismatch silently offers ids the server does not have.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Inventory what is runnable, and on which map
- [ ] Step 3: Sample headlessly to separate map problems from syntax problems
- [ ] Step 4: Run it bounded, with --simulate --2d
- [ ] Step 5: Confirm a simulation actually terminated; leave the world async
```

### Step 1-2: What can I run

```bash
source scripts/env.sh
bash scripts/check_env.sh

python3 scripts/list_scenic.py                # every scenario, its model, its map
python3 scripts/list_scenic.py --check-maps   # + each road network's features
python3 scripts/list_scenic.py --map Town05   # only scenarios for one map
```

`list_scenic.py` parses the headers as text, so it needs neither Scenic nor a
server and cannot be broken by a failing model import.

### Step 3: Sample before you simulate

```bash
python3 scripts/sample_scenic.py "$SCENARIO_RUNNER_ROOT"/srunner/scenic/carlaChallenge*.scenic
```

Verdicts map to causes with no overlap:

| Verdict | Meaning | Where to look |
|---|---|---|
| `PASS` | scene constructible; only runtime issues remain | go to Step 4 |
| `COMPILE-FAIL` | syntax, model import, missing `.xodr`, or an empty blueprint category | `check_env.sh`, `PYTHONPATH` |
| `SAMPLE-FAIL` | `require`s unsatisfiable on this map | `--check-maps` |

### Step 4: Run

**Always bound the run.** `--count` and `--time` default to infinity, so a bare
`scenic --simulate` never returns.

```bash
source scripts/env.sh

# the canonical smoke test
bash scripts/run_scenic.sh carlaChallenge1

# reproducible: one simulation, fixed seed, 300 steps
COUNT=1 TIME=300 SEED=7 bash scripts/run_scenic.sh carlaChallenge7

# retarget a scenario at another map (both params, or the .xodr and the server disagree)
PARAMS="carla_map Town05" PARAMS2="map $SCENARIO_RUNNER_ROOT/srunner/scenic/assets/Town05.xodr" \
    bash scripts/run_scenic.sh carlaChallenge10
```

Or call the CLI directly — the wrapper only adds resolution, bounding, logging,
artifact confirmation and the async reset:

```bash
scenic "$SCENARIO_RUNNER_ROOT/srunner/scenic/carlaChallenge1.scenic" --simulate --2d --count 1 --time 300
```

| Var | Effect |
|---|---|
| `COUNT` | simulations to run, default 1; empty means unbounded |
| `TIME` | step bound per simulation, default 300; empty means unbounded |
| `SEED` | fixes the sampled scene, so a run is reproducible |
| `MODE2D=0` | drop `--2d`; 3D mode needs meshes the CARLA model does not define |
| `PARAMS`/`PARAMS2`/`PARAMS3` | `--param` pairs, e.g. `PARAMS="carla_map Town05"` |
| `VERBOSITY` | `scenic -v`, default 2 — 2 prints per-sample rejections |
| `LOG_DIR` | where the run log lands, default `./scenic-runs` |
| `EXTRA` | raw extra CLI arguments |

`--2d` is the mode these scenarios were written for. Drop it only deliberately:
in 3D mode Scenic wants real object meshes and the CARLA world model does not
supply them.

### Step 5: Aftermath

Scenic switches the world to synchronous mode for a run and restores async on a
clean exit. An interrupted run leaves it synchronous with nobody ticking, and
every other client then appears frozen. `run_scenic.sh` traps EXIT/INT/TERM and
restores it; if you called `scenic` yourself and killed it, reset with
[[set-world-settings]] `async`.

## Examples

**Example 1: "run a Scenic scenario to check my setup"**

`bash scripts/check_env.sh`, then `bash scripts/run_scenic.sh carlaChallenge1` —
control loss with debris, the least constrained of the set. Expect
`Simulation 1 ended successfully at time step 300 because: reached time limit`.

**Example 2: "carlaChallenge10 rejects everything"**

It filters for an unsignalized four-way. Run
`python3 scripts/list_scenic.py --check-maps` and read the `4way=N(uns M)`
column: where `uns` is 0 the scenario cannot sample on that map at all. Retarget
it with `PARAMS`/`PARAMS2` to a map that has one.

**Example 3: "run the whole challenge set and tell me what works"**

`python3 scripts/sample_scenic.py <dir>/*.scenic` first — one second per file —
then `run_scenic.sh` only the ones that sampled. Simulating a scenario that
cannot sample just spends a minute reaching the same conclusion.

## Troubleshooting

**Problem: `ModuleNotFoundError: No module named 'srunner'`**
Cause: the scenario says `model srunner.scenic.models.model` and the checkout is
not importable.
Solution: `source scripts/env.sh` with `SCENARIO_RUNNER_ROOT` set; it puts the
checkout first on `PYTHONPATH`.

**Problem: `InvalidScenarioError: tried to make discrete distribution over empty domain!`**
Cause: a `filter(...)`/`Uniform(*...)` over road features matched nothing on this
map. Not a syntax error and not a server problem.
Solution: `list_scenic.py --check-maps`, then retarget the map with `PARAMS`.

**Problem: `Scenic has no 'bicycle' blueprints recorded for CARLA <version>`**
Cause: Scenic's table for that client version has an empty category. The build
may well contain such vehicles — the table just does not list them.
Solution: `check_env.sh` prints the empty categories. Name a concrete blueprint
with `with blueprint "..."` instead of relying on the category.

**Problem: `RejectionException: failed to generate scenario in N iterations`**
Cause: the `require`s are jointly too tight for this map — often two distance
requirements that no road geometry satisfies.
Solution: raise `--max-sims-per-scene`, loosen a `require`, or move maps. Confirm
with `sample_scenic.py --iterations 5000` before blaming the simulator.

**Problem: the run never returns**
Cause: `--count`/`--time` default to infinity.
Solution: always pass both; `run_scenic.sh` defaults them to 1 and 300.

**Problem: every other client times out after a Scenic run**
Cause: an interrupted run left synchronous mode on.
Solution: `run_scenic.sh` restores it on exit; otherwise [[set-world-settings]] `async`.

**Problem: `RuntimeError: std::exception` from `createObjectInSimulator`**
Cause: a blueprint id that does not exist on this server, resolved at spawn time
long after sampling passed. `blueprintLib.find()` raises a bare `std::exception`
and never names the id, so the traceback is useless on its own. The upstream
scenarios still carrying `vehicle.lincoln.mkz_2017` fail exactly here.
Solution: `list_scenic.py` prints every hardcoded id per scenario; check each with
`python3 ../create-scenic-scenario/scripts/blueprint_table.py --check <id> ...`

**Problem: `CARLA could not load world 'X'` for a map that then appears loaded**
Cause: Scenic's `param timeout` defaults to 10 s. An editor loading a large map
takes minutes; the client gives up and the load completes anyway.
Solution: raise it — `TIMEOUT=180` on the wrapper, or `--param timeout 180`. A
re-run once the map is loaded also succeeds.

**Problem: `CARLA could not load world 'Town05'`**
Cause: on this build towns 1-9 ship as `TownXX_Opt`; the plain names do not exist.
The OpenDrive assets, however, are named *without* the suffix.
Solution: set them separately — `PARAMS="carla_map Town05_Opt"` with
`PARAMS2="map .../assets/Town05.xodr"`.

## Outputs

One or more simulations executed on the server, a per-run log under `LOG_DIR`
holding the sampling trace and the termination reason, and a verify block that
confirms a simulation actually terminated — a Scenic run can exit 0 having
simulated nothing.

CLI flags, the two world models, and the map/feature matrix are in
[references/scenic-cli.md](references/scenic-cli.md).
