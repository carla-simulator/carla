---
name: toggle-env-objects
description: Lists and toggles a map's static environment objects via get_environment_objects / enable_environment_objects — hide or show buildings, vegetation, poles, fences, traffic signs and other baked assets by CityObjectLabel (and optional name filter). Use when the user asks to "hide/remove the buildings", "turn off the trees/vegetation", "show only the roads", "disable the fences", or "list the environment objects". Affects rendering and collision; resets on map reload.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Visual effect needs a rendered view; listing/toggling work headless. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/env-objects.md
---

# Toggle CARLA environment objects

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Show or hide the map's **static, baked** assets — buildings, vegetation, poles,
fences, walls, signs — grouped by `CityObjectLabel`. These are not spawned
actors: they come with the map. Toggling changes both **rendering and
collision**, applies to the whole matching set at once, and **resets on a map
reload**.

## Resolving words to a label

The agent maps the request to a `CityObjectLabel` (run `labels` for the list),
then acts:

- "hide the buildings" → `disable --label Buildings`
- "remove the trees" → `disable --label Vegetation`
- "turn off street poles" → `disable --label Poles`
- "show only the roads" → disable the other big labels (Buildings, Vegetation,
  Fences, Walls…), or re-`enable` selectively.

Narrow within a label by asset name with `--name` (substring).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: List to see labels/counts (labels, list --label X)
- [ ] Step 3: disable / enable the matching set (use --dry-run first if unsure)
- [ ] Step 4: Confirm on a rendered view (effect is visual + collision)
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-3: List and toggle

```bash
source scripts/env.sh

python3 scripts/toggle_env.py labels                      # valid CityObjectLabel names
python3 scripts/toggle_env.py list                        # whole-map summary by type
python3 scripts/toggle_env.py list --label Buildings      # count + sample of buildings

python3 scripts/toggle_env.py disable --label Buildings   # hide all buildings
python3 scripts/toggle_env.py disable --label Vegetation --dry-run   # preview first
python3 scripts/toggle_env.py disable --label Poles --name lamp      # subset by name
python3 scripts/toggle_env.py enable  --label Buildings   # bring them back
```

### Step 4: Confirm

The effect is visual + collision on a rendered server. `enable_environment_objects`
returns nothing to read back, so the command reports the **count toggled**;
confirm the change in a windowed/packaged view.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

`get_environment_objects()` and `enable_environment_objects()` — what this skill
uses — work normally. Measured on Town10HD_Opt: 59 102 objects, 48 369 of them
`Buildings`, then `Vegetation` 4 270, `Fences` 1 767, `Static` 1 223,
`Dynamic` 489, `Sidewalks` 269, `TrafficSigns` 147, `Poles` 120,
`TrafficLight` 62, `Bicycle` 56, plus 1 768 labelled `NONE`.

`CityObjectLabel` gained **`Rock` (= 29)**, which does not exist on 0.9.x
(`LibCarla/source/carla/rpc/ObjectLabel.h:48`). Code that iterates the enum by
name is fine; code that hard-codes label integers is not.

**Do not reach for map layers as an alternative here.** `load_map_layer` /
`unload_map_layer` are accepted and silently do nothing on 0.10.0 — see
[[load-map]] for the measurement and the cause. `enable_environment_objects` is
the mechanism that actually hides geometry on this version.

## Examples

**Example 1: clear the buildings**

User says: "hide all the buildings"

`disable --label Buildings` → reports N buildings hidden; the town goes bare.
`enable --label Buildings` restores them.

**Example 2: strip vegetation for a clean capture**

User says: "remove the trees and bushes"

`disable --label Vegetation`. Combine with a weather/settings setup for dataset
capture.

**Example 3: preview before a big toggle**

User says: "what would hiding the fences affect?"

`disable --label Fences --dry-run` lists the count and sample without changing
anything.

## Troubleshooting

**Problem: nothing changes visually**
Cause: headless `-nullrhi` server (no rendering), or wrong label.
Solution: use a rendered server; `list --label X` to confirm the set is non-empty.

**Problem: objects came back after loading a map**
Cause: toggles are per-world; a map load/reload rebuilds all objects.
Solution: re-apply the toggles after `load-map`.

**Problem: `list --label X` is empty but I see those assets**
Cause: the assets may be tagged under a different label (e.g. Static/Other).
Solution: `list` (no label) for the by-type summary, then target the right label.

## Outputs

Server-side visibility/collision state of static map assets. No file. The
reported count is the record of what was toggled.

Detail (the label taxonomy, name filtering, rendering/collision semantics) in
[references/env-objects.md](references/env-objects.md).
