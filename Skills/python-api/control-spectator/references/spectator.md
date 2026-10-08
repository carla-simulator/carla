# Spectator control — detail

Detail layer for the `control-spectator` skill.

## The spectator

`world.get_spectator()` returns the spectator `carla.Actor` — the pose of the
on-screen camera on a rendered server. Aim it with `spectator.set_transform(
carla.Transform(location, rotation))`. It only matters on a rendered server
(windowed/packaged); on a headless `-nullrhi` server there is no view to move.

## Attaching vs following

There is **no way to attach/reparent the spectator** to another actor: `Actor`
has no `attach`, and `AttachmentType` (`Rigid`, `SpringArm`, `SpringArmGhost`)
applies only at **spawn time** to a newly spawned actor (e.g. a camera sensor).
The spectator already exists, so it cannot be re-spawned attached.

Two real options:

1. **Follow** (this skill): re-set the spectator's transform every world tick via
   `world.on_tick(cb)` so it tracks the target. `remove_on_tick(id)` stops it. In
   async mode the server ticks continuously (smooth); in sync mode it updates only
   when a client ticks the world.
2. **Attached camera sensor** (create-sensor skill): spawn a `sensor.camera.rgb`
   with `attach_to=actor, attachment_type=SpringArm` for a smooth chase camera you
   can also record. That is a real attachment — but a sensor, not the spectator.

## View math

Given the target's transform (`location`, `rotation`, and
`transform.get_forward_vector()` = unit vector along its yaw):

| View | Position | Orientation |
|---|---|---|
| `chase` (3rd) | `loc - forward*distance + up*height` | pitch `-12`, yaw = actor yaw |
| `first` (driver) | `loc + forward*0.5 + up*1.2` | pitch `0`, yaw = actor yaw |
| `top` (bird) | `loc + up*height` | pitch `-90`, yaw = actor yaw |
| `front` | `loc + forward*distance + up*height` | pitch, yaw = actor yaw + 180 |

Defaults: chase `distance=6, height=2.5`; first `distance=0.5, height=1.2`; top
`height=25`; front `distance=8, height=2`. Override with `--distance/--height/
--pitch`. `up` is world +Z. For a taller vehicle raise `first`'s height; the
defaults suit a sedan.

## Target resolution

`_resolve_actor` picks one actor:

- `--id N` → `world.get_actors().find(N)`.
- `--role R` → actors whose `attributes['role_name'] == R`. The ego vehicle is
  conventionally `hero`; scenario tools may use other roles.
- `--filter P` → `world.get_actors().filter(P)`, matching `type_id` with wildcards
  (`*prius*`, `vehicle.*.*`, `walker.pedestrian.*`).

On multiple matches it uses the one nearest the current spectator and prints the
chosen id — run `actors` to inspect and pick a specific `--id`.

## Sync-mode note

`follow` relies on world ticks. If the world is in synchronous mode and your
process is not the one ticking it, the camera will not update. Either follow in
async mode, or drive the tick loop (set-world-settings) alongside the follow.
