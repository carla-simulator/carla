---
name: create-leaderboard-route
description: Creates and edits CARLA Leaderboard route files — placing waypoints and scenario trigger points interactively from the spectator camera (route_creator, scenario_creator), visualising an existing route in the simulator (route_displayer), summarising it as a table (route_summarizer), capturing the current weather in route format (weather_creator), sorting scenarios by route position (scenario_orderer) and converting 1.0-format routes to 2.x (route_bridge). Use when the user asks to "make a new route", "add a scenario to a route", "visualise/debug a route", "see what's in this route file", or "convert old leaderboard routes".
license: MIT
compatibility: Needs a leaderboard checkout and a RUNNING CARLA server on the route's town — all of these tools drive the spectator camera and draw debug shapes, so they are interactive and useless headless. route_bridge exists on master only. Requires pygame for the interactive tools.
metadata:
  group: leaderboard
  prerequisites: scripts/check_env.sh
  reference: references/authoring.md
---

# Create a leaderboard route

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A route is waypoints plus scenarios pinned to trigger points, in one XML file
([[run-route-scenario]] documents the format). Writing the coordinates by hand does
not work in practice — Town12 is 10 km across and a trigger point a metre into the
wrong lane silently never fires. So the leaderboard ships **interactive** tools that
read positions off the spectator camera in a live simulator.

All of them live in `$LEADERBOARD_ROOT/scripts/` and all need a running server with
the right town loaded.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Load the town the route will live in (load-map)
- [ ] Step 3: Place waypoints (create), then scenarios (scenarios)
- [ ] Step 4: Inspect: display in the simulator, summarise as a table
- [ ] Step 5: Order the scenarios, then run it (run-leaderboard-evaluation)
```

### Step 2-3: Build it

```bash
source scripts/env.sh

# 1. load the town first — every tool works on the *currently loaded* map
python3 -c "import carla; carla.Client('127.0.0.1',2000).load_world('Town12')"

# 2. place the route waypoints by flying the spectator and clicking
bash scripts/route_tools.sh create --file ~/routes/my_routes.xml

# 3. add scenarios along it
bash scripts/route_tools.sh scenarios --file ~/routes/my_routes.xml

# 4. capture the current weather as a <weather> element
bash scripts/route_tools.sh weather --route 0
```

`route_creator` and `scenario_creator` are terminal + spectator loops: you move the
spectator in the CARLA window, and type commands in the terminal to record the
current position as a waypoint or a scenario trigger. `--file` accepts several
files (`nargs="+"`) so it can append into an existing set.

`scenario_creator -s/--show-only` shows the route without editing, which is the safe
way to look before touching.

### Step 4: Inspect

```bash
# draw an existing route in the simulator
bash scripts/route_tools.sh display --file ~/routes/my_routes.xml --route 0 --keypoints --scenarios

# a table of every route: length, towns, scenario histogram
bash scripts/route_tools.sh summary --file ~/routes/my_routes.xml

# offline structural check — no server needed
bash scripts/route_tools.sh check --file ~/routes/my_routes.xml
```

`check` is this skill's own addition and the one to run first: it validates the XML
against the 2.x schema shape, confirms every `<scenario type>` has a class in the
paired `scenario_runner` checkout, flags routes whose town is not installed, and
finds trigger points that are far from the route polyline — the failure that
otherwise shows up as "the scenario never triggers".

`route_displayer` flags: `-sr/--show-route`, `-sa/--show-all`,
`-sk/--show-keypoints`, `-ss/--show-scenarios`.

### Step 5: Tidy and run

```bash
# sort the <scenarios> block by position along the route (cosmetic but makes review sane)
bash scripts/route_tools.sh order --file ~/routes/my_routes.xml

cd ../run-leaderboard-evaluation
ROUTES=~/routes/my_routes.xml TEAM_AGENT=.../npc_agent.py \
    bash scripts/run_leaderboard.sh --routes-subset 0
```

Always validate a new route with `npc_agent.py` before using it to judge your own
agent: a route the NPC agent cannot complete is a broken route, not a hard one.

### Converting 1.0 routes

```bash
bash scripts/route_tools.sh bridge --routes old_routes.xml \
    --scenarios all_towns_traffic_scenarios_public.json --endpoint new_routes.xml
```

`scripts/route_bridge.py` merges a 1.0 geometry file and its separate scenario JSON
into a single 2.x route file. It exists on **`master` only** — it was removed from
the `leaderboard-2.0` and `2.1` branches, so run it from a master checkout and use
the output elsewhere.

## Examples

**Example 1: "make a short route in Town12 to test one scenario"**

Load Town12, `create` two or three waypoints along one road, `scenarios` to drop a
single `Accident` trigger, `check`, then run with the NPC agent. Faster to iterate
on than any of the shipped 90-route files.

**Example 2: "why doesn't my scenario trigger?"**

`check` first — it reports trigger points off the route polyline and scenario types
with no class. If both are clean, `display --scenarios` and watch where the ego
passes relative to the trigger.

**Example 3: "what is actually in routes_training.xml?"**

`summary --file $LEADERBOARD_ROOT/data/routes_training.xml`: 90 routes, all Town12,
4629 scenario instances across 38 types. `check` on the same file is a good sanity
test of your own checkout pairing.

**Example 4: "I want the same route in fog and at night"**

`weather` with the simulation set as you want it prints a `<weather>` element ready
to paste. Two entries with `route_percentage="0"` and `"100"` interpolate along the
route; more entries give a profile.

## Troubleshooting

**Problem: a tool exits immediately or draws nothing**
Cause: no server, or the wrong town loaded. These tools operate on the currently
loaded map and do not load one for you.
Solution: load the town first; confirm with `check_env.sh`.

**Problem: `ModuleNotFoundError: No module named 'pygame'`**
Cause: the interactive tools need pygame (in `leaderboard/requirements.txt`).
Solution: `pip install pygame`.

**Problem: scenarios in the file never trigger during a run**
Cause: the type has no class in this `scenario_runner` branch — `RouteScenario`
skips unknown types silently — or the trigger point is off the driven path.
Solution: `check`. Branch pairing is the usual culprit: `master` scenario_runner
with a `leaderboard-2.1` route set, or the reverse.

**Problem: the ego spawns facing backwards**
Cause: the first waypoint's lane direction. Routes take their heading from the road,
not from a yaw you set.
Solution: move the first waypoint, or reverse the waypoint order.

**Problem: `route_bridge.py: No such file`**
Cause: it only exists on `leaderboard` `master`.
Solution: run it from a master checkout; the output is a plain 2.x route file.

**Problem: edits to the route file did not take effect on `--resume`**
Cause: `RouteIndexer.validate_and_resume` compares the *total* route count and
restarts from scratch if it changed — and silently continues at the old index if it
did not.
Solution: never edit a route file mid-evaluation; use a fresh checkpoint.

**Problem: `--routes-subset 3` runs a different route than expected**
Cause: the subset is an index into the file, not a route id.
Solution: `summary` shows both; keep ids sequential from 0 so they coincide.

## Outputs

A route XML in the 2.x format, plus debug drawings in the simulator and a table on
stdout. `check` is read-only and exits non-zero on any problem that would make a
route silently misbehave. The interactive tools write to the `--file` you name.

Route element semantics, the interactive key/command sets and what each shipped
script does are in [references/authoring.md](references/authoring.md).
