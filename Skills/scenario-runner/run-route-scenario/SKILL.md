---
name: run-route-scenario
description: Runs a route through ScenarioRunner's RouteScenario mode (--route / --route-id) with an autonomous agent driving the ego — the same mechanism the CARLA Leaderboard uses, and the only way to reach the ~34 route-only scenario classes (Accident, ParkedObstacle, HazardAtSideLane, InvadingTurn, HardBreakRoute, YieldToEmergencyVehicle, junction and actor-flow scenarios). Inspects route files, lists route ids and their scenarios, and runs one or all of them. Use when the user asks to "run a route", "drive a route with my agent", "run leaderboard scenarios without the leaderboard", or names a route-only scenario type.
license: MIT
compatibility: Any OS with a scenario_runner checkout, importable CARLA PythonAPI and a running server. Route mode forces --sync and --reloadWorld. Route files in the 2.x format need the towns they name (Town12/Town13 need AdditionalMaps or the leaderboard build).
metadata:
  group: scenario-runner
  prerequisites: scripts/check_env.sh
  reference: references/routes.md
---

# Run a route

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A **route** is a list of waypoints plus a set of scenarios pinned to trigger
points along it. `RouteScenario` builds one behaviour tree containing every
scenario on the route, plus background traffic, and evaluates route-level criteria
(completion, deviation, red lights, collisions, blocked).

This is the mode that matters for autonomous-driving work, for three reasons:

- **An agent drives.** `--agent` only works here; with `--scenario` it is refused.
- **Most of the scenario library is only reachable here.** ~34 classes have no
  standalone config.
- **It is what the Leaderboard runs.** Same `RouteScenario`, same criteria. The
  Leaderboard adds scoring, sensor limits and result bookkeeping on top.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Inspect the route file — ids, towns, scenarios
- [ ] Step 3: Pick or write the agent
- [ ] Step 4: Run one route id first, then the file
- [ ] Step 5: Read the criteria; reset the world to async
```

### Step 2: Inspect

```bash
source scripts/env.sh

bash scripts/run_route.sh list "$LEADERBOARD_ROOT/data/routes_devtest.xml"
bash scripts/run_route.sh show "$LEADERBOARD_ROOT/data/routes_devtest.xml" 0
```

`list` prints each route id, its town, waypoint count and scenario histogram, and
flags whether the towns it needs exist on the running server. `show` dumps one
route's scenarios in trigger order.

Route files come from two places: the Leaderboard's `data/*.xml`, or your own
(see [[create-leaderboard-route]] for the authoring tools). ScenarioRunner itself
ships no route files on `master` — the old `srunner/data/routes_*.xml` referenced
by the docs was removed.

### Step 3: The agent

Route mode needs something to drive. `srunner/autoagents/` has three:

| Agent | Use |
|---|---|
| `npc_agent.py` | Traffic-Manager autopilot following the route — the reference "it works" agent |
| `human_agent.py` | pygame window, keyboard control |
| `dummy_agent.py` | returns zero control; the template to copy |

ScenarioRunner derives the class name from the **file name**: `npc_agent.py` →
`NpcAgent`, `my_cool_agent.py` → `MyCoolAgent`
(`os.path.basename(agent).split('.')[0].title().replace('_','')`). A mismatch is an
`AttributeError` at load, not a helpful message. Writing one is
[[write-leaderboard-agent]] — the same `AutonomousAgent` interface applies here.

### Step 4: Run

```bash
source scripts/env.sh

# one route, the NPC agent — the canonical check that routes work at all
AGENT="$SCENARIO_RUNNER_ROOT/srunner/autoagents/npc_agent.py" \
    bash scripts/run_route.sh run "$LEADERBOARD_ROOT/data/routes_devtest.xml" 0

# every route in the file, results as JSON
AGENT=.../npc_agent.py OUTPUT=1 JSON=1 OUTPUT_DIR=./results \
    bash scripts/run_route.sh run routes.xml

# your own agent with a config file, recorded for later replay
AGENT=~/team_code/my_agent.py AGENT_CONFIG=~/team_code/config.json RECORD=recordings \
    bash scripts/run_route.sh run routes.xml 3
```

Direct equivalent:

```bash
python3 "$SCENARIO_RUNNER_ROOT/scenario_runner.py" \
    --route routes.xml --route-id 0 --agent .../npc_agent.py --agentConfig cfg.json
```

**`--route-id` is one id, not a range.** Unlike the Leaderboard's
`--routes-subset`, there is no `0-4` or `1,6` syntax here: omit it to run every
route in the file, or loop.

Route mode forces two things regardless of what you pass:
`--reloadWorld` (the route names its town) and `--sync` when `--agent` is given
(`arguments.sync = True`). So the world **will** be reloaded and **will** be in
synchronous mode — plan the aftermath accordingly.

### Step 5: Aftermath

Same trap as any sync run, worse here because it is unconditional: an interrupted
route leaves the world synchronous, which every other client sees as a hang.
`run_route.sh` resets it on exit including Ctrl-C. Otherwise:
[[set-world-settings]] `async`.

## Examples

**Example 1: "check that routes work"**

```bash
AGENT="$SCENARIO_RUNNER_ROOT/srunner/autoagents/npc_agent.py" \
    bash scripts/run_route.sh run "$LEADERBOARD_ROOT/data/routes_devtest.xml" 0
```

The NPC agent completes the route with the Traffic Manager; criteria print at the
end. If this fails, the problem is the setup, not your agent.

**Example 2: "test my agent on the Accident scenario"**

`Accident` is route-only in the Leaderboard 2.x sense. `list` a route file, find a
route whose histogram includes `Accident`, then run that id. Or write a
minimal one-scenario route with [[create-leaderboard-route]].

**Example 3: "I want the leaderboard scenarios but not the scoring"**

Exactly this skill. Point `--route` at `routes_training.xml`, drive with your
agent, and read the criteria. You lose the driving score and infraction penalty —
use [[run-leaderboard-evaluation]] when you want those numbers.

## Troubleshooting

**Problem: `Agents are currently only compatible with route scenarios`**
Cause: `--agent` passed together with `--scenario` or `--openscenario`.
Solution: use `--route`. There is no way to attach an agent to a standalone
scenario.

**Problem: `AttributeError: module 'my_agent' has no attribute 'MyAgent'`**
Cause: the class name must be the file name in TitleCase with underscores removed.
Solution: rename the class or the file — `my_agent.py` → `class MyAgent`.

**Problem: `Configuration for scenario ... cannot be found` / no routes parsed**
Cause: a 1.0-format route file (flat `<waypoint>` elements, scenarios in a
separate JSON) fed to a 2.x parser, or vice versa.
Solution: `list` reports which format it detected. LB 1.0 files need a
`leaderboard-1.0` checkout and a `--scenarios` JSON, which `scenario_runner.py`
on `master` does not accept.

**Problem: `The CARLA server uses the wrong map` even though reload is forced**
Cause: the town the route names is not installed — usually Town12/Town13 without
AdditionalMaps.
Solution: `list` flags missing towns; install AdditionalMaps ([[download-carla]])
or pick a route in a town you have.

**Problem: the route starts and the ego immediately fails `InRouteTest`**
Cause: the first waypoint is off the drivable surface, or the ego spawned facing
the wrong way.
Solution: `show` the route and check the first positions; visualise with
[[create-leaderboard-route]]'s `route_displayer`.

**Problem: scenarios never trigger, the ego just drives the route**
Cause: usually a branch mismatch — the route names scenario types that this
checkout does not implement. `RouteScenario` skips unknown types.
Solution: `list` cross-checks every scenario type in the file against the
classes in the checkout and names the ones that will be skipped.

**Problem: very slow, or the watchdog fires on large maps**
Cause: Town12/Town13 tile streaming with default distances.
Solution: raise `TIMEOUT`; the Leaderboard sets `tile_stream_distance` and
`actor_active_distance` to 650 — do the same with [[set-world-settings]].

## Outputs

The route driven by the agent, a criteria report (`OUTPUT=1`, `JSON=1`,
`OUTPUT_DIR`), and optionally a recorder log plus criteria JSON under
`$SCENARIO_RUNNER_ROOT/$RECORD/`. `list`/`show` print route structure without
touching the simulator.

Route file formats, the criteria route mode adds, and how this differs from the
Leaderboard are in [references/routes.md](references/routes.md).
