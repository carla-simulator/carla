# Debug drawing — detail

Detail layer for the `debug-draw` skill. All calls are on `world.debug`
(`carla.DebugHelper`).

## Primitives

| Call | Signature (after location args) | Notes |
|---|---|---|
| `draw_point` | `(location, size=0.1, color, life_time=-1.0)` | a dot |
| `draw_line` | `(begin, end, thickness=0.1, color, life_time)` | segment |
| `draw_arrow` | `(begin, end, thickness=0.1, arrow_size=0.1, color, life_time)` | direction |
| `draw_box` | `(BoundingBox, Rotation, thickness=0.1, color, life_time)` | wireframe box |
| `draw_string` | `(location, text, draw_shadow=False, color, life_time)` | floating text |

On **0.9.x only** there are `draw_hud_*` variants (`draw_hud_line/point/box`) that
draw in screen/HUD space rather than world space — same arguments. They do **not
exist on 0.10.0**, where `carla.DebugHelper` has exactly seven methods
(`draw_point/line/arrow/box/string`, `clear_debug_shape`, `clear_debug_string`).
Use the world-space ones for marking map locations: they work on both.

`carla.Color(r, g, b)` is 0-255 per channel. `carla.BoundingBox(location, extent)`
takes a centre `Location` and a half-size `Vector3D` (extent is half-width, so a
`extent=(2,4,1.5)` box is 4 x 8 x 3 m).

## Lifetime and rendering

- `life_time` is in **seconds** and is consumed as the **simulation** advances.
- Shapes are drawn during a world tick. In **async** mode the server ticks on its
  own, so a shape appears immediately and fades after `life_time`. In **sync**
  mode the shape is not rendered until the client `world.tick()`s, and its life is
  measured in ticked sim-time.
- For a persistent overlay while stepping in sync mode, redraw the shape every
  frame, or give a `life_time` comfortably longer than one tick.
- A **headless `-nullrhi`** server has no render thread — debug shapes are issued
  but never displayed. Use a windowed (`WINDOW=1`) or packaged (`PACKAGED=1`)
  server (see the run-carla-server skill) to see them.

## Not actors

Debug shapes are a render overlay, not world actors: `get_actors()` never returns
them, and there is no handle to delete one. Control them purely through
`life_time`. A world reload clears everything.

## Moving objects: shapes do NOT follow

A shape is **stamped at a fixed world pose** when you draw it — it does not track a
moving actor. A box drawn around a car stays put while the car drives out of it.
To annotate something that moves, **redraw every frame** with a short `life_time`
(e.g. 0.15 s) in a loop, re-reading the actor's transform each time; the rapid
re-stamping reads as a box that rides the actor. The bounding-boxes skill's `draw`
does exactly this automatically (re-stamps for its `--seconds`). Same applies to
text/labels on moving actors.

## Common uses

- Mark a computed location or spawn point (`draw_point` + `draw_string`).
- Visualise a planned path (`draw_line`/`draw_arrow` between successive waypoints —
  the map-waypoints skill draws topology this way).
- Outline an actor or region (`draw_box`).
