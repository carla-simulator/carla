---
name: control-spectator
description: Moves the CARLA spectator camera, frames an actor, or follows one live — third-person/chase, first-person/driver, top-down, or front views, resolving natural-language targets like "the ego" or "the Toyota Prius" to a specific actor. Use when the user asks to "move the camera/spectator", "watch/follow vehicle X", "3rd-person view of the ego", "1st-person view of the prius", "bird's-eye view", or "look at (x,y,z)".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Only meaningful on a rendered server (the spectator is the on-screen view). Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/spectator.md
---

# Control the CARLA spectator

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Aim the on-screen camera (the "spectator"). Move it to a pose, frame an actor in
a preset view, or follow a moving actor. The spectator **cannot be attached** to
an actor (the API has no reparent for it) — following is emulated by re-setting
its transform every world tick.

## Resolving the target from words

Like the other skills, this emits structured data (`actors`) and the agent
filters it:

- **"the ego"** → `--role hero` (CARLA's ego role_name is usually `hero`).
- **"the Toyota Prius"** → `--filter '*prius*'` (or `vehicle.toyota.prius`).
- **a specific one** → `--id N`.

"3rd-person view of the ego" → `follow --view chase --role hero`. "1st-person of
the prius" → `follow --view first --filter '*prius*'`. On multiple matches it
picks the nearest and says so — run `actors` first if you need to choose.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Resolve the target (actors) if the request names an actor
- [ ] Step 3: move / look (one-shot) / follow (live) in the requested view
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`. Only useful on a
rendered server (windowed or packaged) — the spectator is that window's camera.
**Negative coordinates:** pass `move --at` with an `=` (`--at=-24,-57,20`) so the
leading minus isn't parsed as a flag.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-3: Aim it

```bash
source scripts/env.sh

# list actors to resolve a name
python3 scripts/spectator.py actors
python3 scripts/spectator.py actors --filter '*prius*'

# move to an explicit pose (looking slightly down)
python3 scripts/spectator.py move --at 40,20,15 --pitch -30 --yaw 90

# one-shot framing (no loop): chase view of the ego
python3 scripts/spectator.py look --view chase --role hero

# live follow for 30s: 1st-person of the prius
python3 scripts/spectator.py follow --view first --filter '*prius*' --seconds 30

# bird's-eye follow of actor 87
python3 scripts/spectator.py follow --view top --id 87 --seconds 20
```

Views: `chase` (3rd-person behind+above), `first` (driver), `top` (bird's eye),
`front` (ahead, looking back). Tune with `--distance/--height/--pitch`.

## Examples

**Example 1: 3rd-person of the ego**

User says: "give me a 3rd-person view of the ego"

`follow --view chase --role hero --seconds 30`. The camera chases the hero from
behind and above.

**Example 2: 1st-person of a named car**

User says: "1st-person view of the Toyota Prius"

`follow --view first --filter '*prius*'`. If several Priuses exist it uses the
nearest and reports the id; run `actors --filter '*prius*'` to pick a specific one.

**Example 3: park the camera and look at a spot**

User says: "look at the crossroads at (42,24)"

`move --at 42,54,20 --pitch -35 --yaw -90` (offset back from the point and angle
down), or `look` at a nearby actor.

## Troubleshooting

**Problem: nothing changes on screen**
Cause: headless `-nullrhi` server (no window), or you're viewing a different
client's camera.
Solution: use a windowed/packaged server; the spectator is that view.

**Problem: `follow` doesn't track in sync mode**
Cause: it updates on world ticks; in sync mode nothing ticks unless a client does.
Solution: run the tick loop (set-world-settings), or follow in async mode.

**Problem: "can I attach the spectator to the car?"**
Answer: not directly — the API can't reparent the spectator. `follow` gives the
same effect by updating its transform each tick. For a *recorded/attached* camera,
spawn a camera sensor with SpringArm attachment (create-sensor skill).

**Problem: wrong actor chosen**
Cause: several actors matched the selector.
Solution: run `actors` and pass `--id`, or narrow `--filter`.

## Outputs

Server-side camera state (the on-screen view). `follow` runs for `--seconds` then
stops cleanly. No file produced.

Detail (view math, attachment vs following, selector semantics, sync-mode) in
[references/spectator.md](references/spectator.md).
