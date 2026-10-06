---
name: create-scenic-scenario
description: Writes a new Scenic scenario (.scenic) from a natural-language description — "a car cuts in from the right lane", "a pedestrian crosses in front of the ego", "negotiate an unsignalized four-way" — by turning the request into a spec, generating the file from idioms proven on this build, and validating it by compiling and sampling before any simulator is involved. Use when the user asks to "write a Scenic scenario", "create a .scenic file", "turn this description into Scenic", "generate a scenario from text", or wants a Scenic scenario authored or repaired.
license: MIT
compatibility: Any OS with Scenic installed. Generating and validating a scenario needs no CARLA server; only simulating it does. Blueprint checks read a CARLA checkout's content JSONs, so CARLA_ROOT is needed for those.
metadata:
  group: scenic
  requires: run-scenic-scenario
  prerequisites: scripts/check_env.sh
  reference: references/language.md
---

# Create a Scenic scenario from a description

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Scenic describes a **distribution over scenes**, not one scene. So a good
scenario is not a script of positions — it is a set of constraints loose enough
to sample and tight enough to be the situation asked for.

Translating a request has three parts, and only the first is a judgement call:

1. **request → spec** (yours): pick the map, the placement, the actors and their
   relations.
2. **spec → file** ([scripts/scaffold_scenic.py](scripts/scaffold_scenic.py)):
   emits boilerplate and placement idioms lifted from scenarios that run on this
   build.
3. **validate** (same script): compile, then sample. No server needed.

Do not hand-write the boilerplate. Three things bite every time and the generator
already handles them:

- **`assert len(x) > 0` is illegal on a random value.** Anything derived from a
  `Uniform(...)` raises `RandomControlFlowError`. Only assert on lists built
  deterministically from `network`.
- **Pedestrians and props need `with regionContainedIn None`** when placed on the
  road. Containment is re-checked every sample, so without it generation never
  converges — it looks like an unsatisfiable `require`.
- **`param map` and `param carla_map` must name the same town.** `map` builds the
  road network; `carla_map` tells CARLA what to load. Disagree and the scene is
  sampled on one map and simulated on another.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Pick a map that can express the situation
- [ ] Step 3: Pick blueprints that exist in THIS build
- [ ] Step 4: Write the spec, generate, and let it validate
- [ ] Step 5: Loop on the verdict until it samples; then simulate
```

### Step 2: The map has to support the situation

A scenario selects road features. If the map has none of them, no wording will
save it — the generated `filter` matches nothing and compilation fails with
`tried to make discrete distribution over empty domain`.

```bash
python3 ../run-scenic-scenario/scripts/list_scenic.py --check-maps
```

Read the `4way=N(uns M)` / `3way=N(uns M)` columns. Junction-negotiation
scenarios need `uns > 0`; dense downtown maps are usually fully signalized.

### Step 3: Blueprints that exist here

```bash
python3 scripts/blueprint_table.py                      # inventory + gaps
python3 scripts/blueprint_table.py --category bicycle   # ids Scenic offers
python3 scripts/blueprint_table.py --list vehicles      # what the build has
python3 scripts/blueprint_table.py --check vehicle.lincoln.mkz vehicle.lincoln.mkz_2017
```

Two lists, and they disagree. `blueprint_table.py` reads the build's own
`Content/Carla/Config/*Parameters.json` — the definitive inventory — and diffs it
against Scenic's table for the installed client version.

- **A category Scenic reports empty** makes `new Bicycle` fail at *sample* time
  even when the build has bicycles. Name the id instead:
  `with blueprint "vehicle.bh.crossbike"`.
- **An id in the build but absent from Scenic's table** is still spawnable —
  reference it explicitly.
- **An id in neither** fails at spawn time, long after sampling passed. This is
  how the upstream scenarios still carrying `vehicle.lincoln.mkz_2017` behave.

### Step 4: Spec, generate, validate

```bash
python3 scripts/scaffold_scenic.py --example > spec.json   # start from this shape
# edit spec.json
python3 scripts/scaffold_scenic.py --spec spec.json --out my.scenic
```

Generating with `--out` validates automatically. To re-validate after a hand edit:

```bash
python3 scripts/scaffold_scenic.py --validate my.scenic
```

Spec keys:

| Key | Meaning |
|---|---|
| `map` | town name; needs a matching `.xodr` in the assets (required) |
| `model` | `srunner` (default) or `scenic` — see [[run-scenic-scenario]] |
| `placement` | `lane` for road situations, `intersection` for junction ones |
| `arms`, `signalized` | intersection filter; `signalized: false` for negotiation |
| `ego` | `{blueprint, speed, maneuver}`; `maneuver` is `straight`/`left`/`right` |
| `actors[]` | `{name, type, blueprint, relation, distance, speed, maneuver, offset, threshold}` |
| `requires[]` | extra `require` lines, verbatim |
| `terminate` | termination condition; defaults to leaving the spawn area |

`relation` decides the geometry, and each maps to a proven idiom:

| `relation` | Geometry |
|---|---|
| `ahead` / `behind` | `following roadDirection from ego for ±Range(lo, hi)` |
| `right_lane` / `left_lane` | selects lane **sections** with that neighbour, places there |
| `conflicting` | a conflicting maneuver through the same intersection |
| `crossing` | anchored crossing: ego starts back up the lane, actor crosses it |

### Step 5: Loop on the verdict

| Verdict | Meaning | Fix |
|---|---|---|
| `PASS sampled in N iterations` | done — simulate it | [[run-scenic-scenario]] |
| `COMPILE-FAIL ... empty domain` | map lacks the feature | change `map`, or relax `arms`/`signalized` |
| `COMPILE-FAIL RandomControlFlowError` | a comparison on a random value | hand-edited assert — remove it |
| `COMPILE-FAIL ... no 'X' blueprints recorded` | Scenic category empty | name a concrete blueprint |
| `SAMPLE-FAIL RejectionException` | requirements jointly unsatisfiable | loosen the tightest `require`, widen `distance` |

Simulating is a separate, explicitly-requested step — generating a validated
scenario is this skill's deliverable. Run it with [[run-scenic-scenario]].

## Examples

**Example 1: "a car cuts in front of me from the right lane"**

`placement: lane`, one actor with `relation: right_lane`. The generator selects
lane *sections* that have a right neighbour, because only sections know their
neighbours — `network.lanes` does not. Samples in single-digit iterations on
Town05.

**Example 2: "a pedestrian steps into the road ahead of the ego"**

`relation: crossing`, `type: Pedestrian`. Uses `CrossingBehavior(ego, speed,
threshold)` so the walker waits until the ego is `threshold` metres away, and
adds `regionContainedIn None`. `relation: ahead` with a Pedestrian would sample
forever instead.

**Example 3: "negotiate an unsignalized four-way with a car on a conflicting arm"**

`placement: intersection`, `arms: 4`, `signalized: false`, one actor with
`relation: conflicting`. Check `--check-maps` first: on a map whose only four-way
is signalized this cannot compile, which is exactly why the shipped
`carlaChallenge10` fails on Town10HD_Opt.

## Troubleshooting

**Problem: `RandomControlFlowError: random values cannot be compared`**
Cause: a comparison — usually `assert len(...) > 0` — on something derived from a
`Uniform(...)`. Scenic evaluates the scenario symbolically, so control flow cannot
depend on a sample.
Solution: assert only on lists built by a plain loop over `network.*`. The
generator never emits the illegal form.

**Problem: sampling never converges on a scenario with a pedestrian or a prop**
Cause: the default containment region. The object is being placed on the road but
must lie in the walkable region, so every sample is rejected.
Solution: `with regionContainedIn None`.

**Problem: `no <Town>.xodr found`**
Cause: the generator resolves the OpenDrive from `SCENARIO_RUNNER_ROOT/srunner/scenic/assets`
and a Scenic checkout's `assets/maps/CARLA`.
Solution: set those roots, or pass `"xodr"` in the spec explicitly.

**Problem: the scenario samples but nothing spawns when simulated**
Cause: a blueprint id that does not exist in this build. Ids are resolved at spawn
time, so sampling cannot catch it.
Solution: `scripts/blueprint_table.py --check <id> ...` on every id in the file.

**Problem: a hand-written `terminate when` never fires**
Cause: it references an object that has moved, or a distance that never grows.
Solution: keep the generated form — distance from the spawn point, which always
grows once the ego drives — and always bound the run with `--time` anyway.

## Outputs

A `.scenic` file that compiles and produces at least one sampled scene, with the
blueprints of that scene printed so they can be checked against the build. The
file is ready for `scenic FILE --simulate --2d` via [[run-scenic-scenario]].

The language constructs this model actually supports — object classes, behaviors,
spatial operators, requirement forms — are in
[references/language.md](references/language.md).
