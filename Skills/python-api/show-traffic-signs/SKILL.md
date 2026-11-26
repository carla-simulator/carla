---
name: show-traffic-signs
description: Lists and draws the traffic signs of the loaded CARLA map — speed limits, stops, yields and unknown signs, optionally the traffic lights too — outlining each sign's trigger volume (the box a vehicle must enter to be affected) and drawing a line from that box to the sign itself, with a label, as a debug overlay. Filter by kind or by distance to a point. Use when the user asks to "show the speed limits", "show the traffic signs", "where do the stop signs apply", "draw the sign trigger boxes", or "which sign affects this lane".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Drawing needs a rendered (windowed or packaged) server; `list` works headless.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
---

# Show traffic signs and their trigger volumes

> **Paths.** `scripts/…` below are relative to the directory holding this
> SKILL.md. Your working directory is the user's project, not that directory,
> so prefix them with its absolute path or the command is not found.

## Overview

Every `traffic.*` actor except a traffic light is a sign: `traffic.speed_limit.<N>`,
`traffic.stop`, `traffic.yield`, and `traffic.unknown` (a sign the map did not
type). Each has a **trigger volume**: a vehicle inside it is affected by the
sign. `draw` outlines each trigger volume, draws a line from the box to the
sign, and labels the sign (`SPEED 30 #12`, `STOP #4`, …). Signs do not move, so
the drawing is stamped once and lasts `--life` seconds. `--lights` adds the
traffic lights, whose trigger volumes sit on their stop lines; for their live
state use [[control-traffic-lights]] `draw`.

Which signs exist depends on the map: Town10HD_Opt has 3 stops, 2 yields and
3 unknown signs, and no speed-limit actors.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: list the signs (kinds, positions, trigger sizes)
- [ ] Step 3: draw them on a rendered server; do not move the spectator
      (report where the signs are; control-spectator only if the user asks)
- [ ] Step 4: (optional) remove early with debug-draw clear
```

### Step 1

```bash
bash scripts/check_env.sh
```

### Step 2-3

```bash
source scripts/env.sh

python3 scripts/signs.py list                                  # every sign
python3 scripts/signs.py list --type speed_limit               # only speed limits
python3 scripts/signs.py draw                                  # all signs, 120 s
python3 scripts/signs.py draw --type stop,yield --life 120
python3 scripts/signs.py draw --near=-47,20,0 --radius 40 --lights   # around a junction, lights too
python3 scripts/signs.py draw --color 255,255,255 --no-labels  # one colour, no text
```

Colours by kind: speed limit orange, stop red, yield yellow, traffic light
cyan, unknown grey. `--color r,g,b` draws all of them in one colour.

### Verify

`list` prints each sign's position, the trigger centre and size, and the
distance between them. On Town10HD_Opt a stop or yield trigger sits about 5 m
from its sign, and a traffic light's 12-41 m (the pole stands across the
junction from the lane it controls). Light trigger boxes coincide with the stop
lines that control-traffic-lights `draw` marks.

## Gotchas

- **`trigger_volume` is in the sign's frame.** Its location is relative to the
  actor and its extent is along the actor's axes, so it is drawn at
  `transform.transform(location)` with the actor's rotation.
- **`carla.Transform.transform()` rewrites its argument in place** as well as
  returning the result. Transform a copy: transforming the live attribute twice
  moved a yield trigger 58 m away in testing, and a print of the "before" value
  after the call shows the transformed one.
- **Nothing shows on a headless `-nullrhi` server**, as for any debug drawing.

## Outputs

`list`: one line per sign. `draw`: a debug overlay for `--life` seconds; no
file, no actor. `debug-draw clear` removes it early, along with every other
debug drawing in the world.
