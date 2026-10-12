# Telemetry — detail

Detail layer for the `telemetry` skill.

## Frame-consistent reads

The per-actor getters (`actor.get_transform()`, `get_velocity()`,
`get_acceleration()`, `get_angular_velocity()`) each hit the server independently
and can land on different frames, so a fast-moving actor's position and velocity
may not correspond. Instead take **one** `world.get_snapshot()` and read the
actor's `ActorSnapshot` (`snapshot.find(actor_id)`): its `get_transform`,
`get_velocity`, `get_acceleration`, `get_angular_velocity` all come from that
single frame. The skill does this for both `show` and each tick of `watch`.

## Fields

- **location** (m, world), **rotation** (pitch/yaw/roll degrees).
- **velocity** vector (m/s) and **speed** = magnitude in km/h.
- **acceleration** magnitude (m/s²), **angular velocity** magnitude (rad/s).
- Vehicles additionally:
  - **control** — `get_control()`: throttle, steer, brake, gear, reverse (the
    input currently driving it, whether from you or the Traffic Manager).
  - **wheel steer** — `get_wheel_steer_angle(VehicleWheelLocation.FL_Wheel/FR_Wheel)`
    in degrees (wheel locations: FL/FR/BL/BR, plus Front/Back aliases for bikes).
  - **mass** — from `get_physics_control()` (see control-vehicle for full physics).

## Selection

`--id` is unambiguous. `--role` matches `attributes['role_name']` (ego = `hero`).
`--filter` matches `type_id`, `--color` a vehicle's colour; `--nearest` (with
`--near`/`--near-id`) picks the single closest. If several still match, resolve
with the world-data skill rather than guessing — this skill refuses an ambiguous
selection on purpose so you never read the wrong actor. There is no positional
"Nth": peer actors have no order.

## Sync mode

In synchronous mode values update only when the world is ticked; the snapshot
still keeps a single reading self-consistent. `watch` sleeps in wall-clock time,
so in sync mode drive the tick loop (set-world-settings) alongside to see motion.
