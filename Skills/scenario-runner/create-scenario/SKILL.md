---
name: create-scenario
description: Writes a new ScenarioRunner scenario — a Python class deriving from BasicScenario with its behaviour tree assembled from srunner atomic behaviours and criteria, plus the XML config that gives it a town, ego spawn and parameters — and registers it so `--scenario MyScenario_1` works. Also covers adding a scenario to a route via --additionalScenario. Generates a working skeleton and validates it without a simulator. Use when the user asks to "create/write a new scenario", "add a custom scenario", "make my own traffic situation", or wants to extend the scenario library.
license: MIT
compatibility: Any OS with a scenario_runner checkout; a running CARLA server only to execute the result. The behaviour/criteria APIs are the same across branches, but the py_trees version is not: ue58-dev pins 2.6.0, where Sequence takes an explicit memory argument and Parallel a policy object, while master, ue5-master and the leaderboard branches pin 0.8.3. The scaffold reads the pin and emits the right form; module paths differ on the leaderboard-1.0 branch.
metadata:
  group: scenario-runner
  prerequisites: scripts/check_env.sh
  reference: references/atomics.md
---

# Create a scenario

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A scenario is two files:

```
srunner/scenarios/my_scenario.py    the class: actors + behaviour tree + criteria
srunner/examples/MyScenario.xml     the config: town, ego spawn, parameters
```

ScenarioRunner finds the class by globbing `$SCENARIO_RUNNER_ROOT/srunner/scenarios/*.py`
plus whatever `--additionalScenario` points at, and matching the config's `type`
attribute against the `BasicScenario` subclasses in those modules. There is **no
registration list** — dropping the two files in, or passing their folder, is the
whole install. The corollary is that the `type` in the XML must match the class
name exactly, and a typo produces `Scenario 'X' not supported`.

On `ue58-dev`, `--additionalScenario` and `--configFile` each take a file, a
folder, a comma-separated list, or a repeated flag, and the classes they find are
also available to `--route`. Older checkouts take one file each, and route mode
sees only `srunner/scenarios/`, so a custom scenario named by a route needs the
file inside the checkout there.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Generate the skeleton (scaffold), or copy the closest existing scenario
- [ ] Step 3: Fill in _initialize_actors, _create_behavior, _create_test_criteria
- [ ] Step 4: Validate offline (validate) — imports, class/type match, tree shape
- [ ] Step 5: Run it (run-scenario), iterate with DEBUG=1
```

### Step 2: Scaffold

```bash
source scripts/env.sh

# a standalone scenario with a config, in the checkout
python3 scripts/scaffold_scenario.py --name MyCutIn --town Town04 \
    --template follow_leading_vehicle

# somewhere else, to keep the checkout clean (use --additionalScenario to run it)
python3 scripts/scaffold_scenario.py --name MyCutIn --town Town04 --out ~/my_scenarios

python3 scripts/scaffold_scenario.py --list-templates
```

The scaffold writes both files with the four methods stubbed, the imports that
actually exist in this branch, and a `CollisionTest` criterion so the run reports
something. Then edit.

### Step 3: The four methods

```python
class MyCutIn(BasicScenario):
    def __init__(self, ego_vehicles, config, randomize=False,
                 debug_mode=False, criteria_enable=True, timeout=60):
        self.timeout = timeout
        # scenario parameters come from the XML, via config.other_parameters
        self._distance = 50
        if "distance" in config.other_parameters:
            self._distance = float(config.other_parameters["distance"]["value"])
        super().__init__("MyCutIn", ego_vehicles, config, debug_mode, criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """Spawn everything the scenario needs. Use request_new_actor, never world.spawn_actor."""
        wp = self._reference_waypoint.next(self._distance)[0]
        actor = request_new_actor("vehicle.tesla.model3", wp.transform)
        self.other_actors.append(actor)

    def _create_behavior(self):
        """Return the py_trees tree. Composites: Sequence, Parallel; leaves: atomics."""
        root = py_trees.composites.Sequence("MyCutIn", memory=True)   # py_trees 2.x
        root.add_child(InTriggerDistanceToVehicle(self.other_actors[0], self.ego_vehicles[0], 30))
        root.add_child(LaneChange(self.other_actors[0], speed=10, distance_other_lane=30))
        root.add_child(ActorDestroy(self.other_actors[0]))
        return root

    def _create_test_criteria(self):
        """Return a list of criteria. These decide pass/fail."""
        return [CollisionTest(self.ego_vehicles[0])]
```

Rules that are not obvious and cost the most time:

- **Spawn through `request_new_actor`** from `srunner.tools.actor_factory`, not
  `world.spawn_actor`: it resolves the blueprint against this CARLA version and
  waits for the actor to exist before returning it.
- **Append to `self.other_actors`** or the actor is never destroyed. Nothing else
  owns it, so a forgotten append leaks the actor for the rest of the session.
- `_create_behavior` must return a tree that **terminates**. A `Sequence` whose
  last child never succeeds hangs until the timeout — which is what an apparently
  stuck scenario usually is.
- `config.other_parameters` is a dict of `{name: {attr: value}}` built from the XML
  child elements, and every value is a **string**. Cast it.
- `self.timeout` must be set *before* `super().__init__`, because the base class
  reads it while building the tree.
- Criteria are attached in parallel with the behaviour and run to the end. An
  `optional=True` criterion reports but does not fail the scenario.

### Step 4: Validate offline

```bash
python3 scripts/scaffold_scenario.py validate --name MyCutIn
```

Checks, without a simulator: the module imports, the class exists and derives from
`BasicScenario`, the XML `type` matches the class name, the XML parses and names a
real map for this branch, the four methods are present, and `_create_behavior`
returns something tree-shaped. Most authoring mistakes surface here rather than
after a 30 s world reload.

### Step 5: Run

```bash
cd ../run-scenario
DEBUG=1 OUTPUT=1 bash scripts/run_scenario.sh MyCutIn_1

# scenario file outside the checkout
python3 "$SCENARIO_RUNNER_ROOT/scenario_runner.py" --scenario MyCutIn_1 \
    --additionalScenario ~/my_scenarios/my_cut_in.py \
    --configFile ~/my_scenarios/MyCutIn.xml --reloadWorld
```

`DEBUG=1` prints the behaviour tree every tick with each node's status — the only
practical way to see which child is blocking.

## Examples

**Example 1: "make a scenario where a car brakes hard in front of me"**

`scaffold --name HardBrakeAhead --town Town04 --template follow_leading_vehicle`,
then in `_create_behavior`: `InTriggerDistanceToVehicle` →
`ChangeActorTargetSpeed(0)` or `HardBreak` → `Idle` → `ActorDestroy`. Criteria:
`CollisionTest` plus `DrivenDistanceTest`.

**Example 2: "add a parameter so I can sweep the distance"**

Add `<distance value="50"/>` to the XML config and read
`config.other_parameters["distance"]["value"]` in `__init__`. Duplicate the
`<scenario>` element with different values and names to get `MyCutIn_1`,
`MyCutIn_2`, … then run `group:MyCutIn`.

**Example 3: "put my scenario in a leaderboard route"**

Write the class, then reference it by `type` in a route's `<scenarios>` block and
run with [[run-route-scenario]]. Route scenarios take their position from
`trigger_point` rather than from the config's ego spawn, so read
`config.trigger_points[0]` and derive everything relative to it — that is what
makes a scenario reusable across routes.

## Troubleshooting

**Problem: `Scenario 'MyCutIn_1' not supported`**
Cause: the XML `type` does not match a class name, the class does not derive
from `BasicScenario`, the file is neither in `srunner/scenarios/` nor passed to
`--additionalScenario`, or `SCENARIO_RUNNER_ROOT` is unset so the glob is `./`.
Solution: `validate`; use `--additionalScenario` for files outside the checkout.

**Problem: the scenario starts and hangs until the timeout**
Cause: a behaviour that never returns SUCCESS — a trigger condition that cannot
be met, or a `Sequence` missing a terminating child.
Solution: `DEBUG=1` and read the tree; add `Idle(duration)` or a
`TimeOut` in parallel.

**Problem: `RuntimeError: trying to operate on a destroyed actor`**
Cause: a behaviour kept a handle to an actor another behaviour already destroyed.
Solution: destroy an actor in exactly one place, and use `destroy_actor` from
`srunner.tools.actor_state`, which returns False instead of raising on a dead one.

**Problem: actors from the last run are still there**
Cause: the actor was never appended to `self.other_actors`, so `remove_all_actors()`
does not know about it. There is no global pool backing it up any more.
Solution: append every actor to `self.other_actors`, or destroy it in the
`terminate()` of whatever created it.

**Problem: `TypeError: unsupported operand` on a parameter**
Cause: `config.other_parameters` values are strings.
Solution: `float(...)` / `int(...)`.

**Problem: `AttributeError: 'NoneType' object has no attribute 'transform'`**
Cause: `waypoint.next(d)` returned an empty list — the distance runs off the end
of the road, or past a junction.
Solution: guard the result; `generate_target_waypoint` /
`get_waypoint_in_distance` in `srunner/tools/scenario_helper.py` handle junctions.

## Outputs

`srunner/scenarios/<snake_name>.py` and `srunner/examples/<Name>.xml` (or the same
pair under `--out`), immediately runnable by name. `validate` prints a per-check
report and exits non-zero on anything that would stop the scenario loading.

The atomic behaviours, trigger conditions and criteria available — with the
signatures that matter — are catalogued in
[references/atomics.md](references/atomics.md).
