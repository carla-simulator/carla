---
name: control-lights
description: Controls the map's light sources through the CARLA LightManager — turn street lamps, building/facade lights, vehicle lights or all of them on/off, set their colour and intensity, and toggle the automatic lights-at-night cycle. Use when the user asks to "turn on the street lights", "light up the city at night", "turn off building lights", "change the street-light colour", or "make lights come on automatically at night". Distinct from hiding lamp meshes (toggle-env-objects) and the sun (set-weather).
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Light effects are visible on a rendered server. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/lights.md
---

# Control light sources

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Switch and tune the world's illumination — street lamps, building lights, vehicle
lights — via the `LightManager`. These are actual light **sources** (they cast
light): different from `toggle-env-objects` (which hides the lamp *geometry*) and
`set-weather` (the sun). Effects show on a rendered server.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: list a group to see counts/states
- [ ] Step 3: on / off / set a group, or toggle the day-night cycle
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Control

```bash
source scripts/env.sh

python3 scripts/lights.py list --group street              # counts + a sample

# light up the streets, warm colour
python3 scripts/lights.py on --group street --color 255,220,150 --intensity 2000
# turn building lights on for a night-city look
python3 scripts/lights.py on --group building
# turn everything off
python3 scripts/lights.py off --group all

# recolour without toggling
python3 scripts/lights.py set --group street --color 150,200,255

# let lights switch on/off automatically with the sun
python3 scripts/lights.py day-night on
```

Groups: `street`, `building`, `vehicle`, `other`, `all`.

## Examples

**Example 1: night city**

User says: "make it a lit-up city at night"

Set night with set-weather (`sun_altitude_angle` negative), then
`on --group street` and `on --group building`.

**Example 2: blackout**

User says: "kill all the lights"

`off --group all`.

**Example 3: automatic dusk lighting**

User says: "have the street lights come on by themselves at dusk"

`day-night on`.

## Troubleshooting

**Problem: no visible change**
Cause: headless `-nullrhi` server (no rendering), or it's daytime so lights wash out.
Solution: use a rendered server; set night (set-weather) to see them clearly.

**Problem: `list` shows 0 lights**
Cause: this map has few/no light sources of that group, or wrong group.
Solution: try `--group all`; some maps have sparse building lights.

**Problem: lights won't stay on**
Cause: the day-night cycle is auto-switching them.
Solution: `day-night off`, then set them manually.

## Outputs

Illumination state on the server (on/off, colour, intensity, auto-cycle). No file.
`list` reports per-group counts and a sample.

Detail (LightManager API, groups, colour/intensity units, day-night) in
[references/lights.md](references/lights.md).
