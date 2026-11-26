# Vehicle control — detail

Detail layer for the `control-vehicle` skill.

## Manual vs autopilot

A vehicle is driven by **either** manual control **or** the Traffic Manager, never
both: applying control while autopilot is on does nothing useful. `control` and
`ackermann` call `set_autopilot(False)` first. To hand the car back to traffic,
use the control-traffic skill (or `set_autopilot(True, tm_port)`).

## VehicleControl (raw) vs Ackermann

- `apply_control(VehicleControl(throttle, steer, brake, hand_brake, reverse,
  manual_gear_shift, gear))` — direct actuator input. `throttle`/`brake` are
  0..1, `steer` is -1..1. It **persists**: the vehicle holds the last control
  every tick until you change it. Use `--hold` for a timed action.
- `apply_ackermann_control(VehicleAckermannControl(steer, steer_speed, speed,
  acceleration, jerk))` — a target-*speed* controller: you give a desired `speed`
  (m/s) and it works out throttle/brake to reach and hold it. Better for "drive at
  8 m/s" than hand-tuning throttle. `apply_ackermann_controller_settings(...)`
  tunes that controller's PID if needed.

## Lights

`set_light_state(VehicleLightState(bitmask))` / `get_light_state()`. Flags OR
together: `Position`, `LowBeam`, `HighBeam`, `Brake`, `LeftBlinker`,
`RightBlinker`, `Reverse`, `Fog`, `Interior`, `Special1`, `Special2` (plus `All`,
`NONE`). Hazards = both blinkers. The skill reads the current state and adds
(`--on`) / clears (`--off`) flags so other lights are preserved. Note: under
autopilot, `control-traffic`'s `update_vehicle_lights` can let the TM manage
lights automatically instead.

## Doors

`open_door(VehicleDoor)` / `close_door(VehicleDoor)`. 0.9.16 exposes `FL`, `FR`,
`RL`, `RR`, `All` — no hood/trunk. Not every vehicle model has animated doors;
some are no-ops.

## Physics control

`get_physics_control()` → `VehiclePhysicsControl`; mutate and `apply_physics_control(pc)`.
Common scalar fields (this skill exposes mass/drag/max_rpm; the rest are editable
in code):

- `mass` (kg), `drag_coefficient`, `max_rpm`, `center_of_mass`,
- **0.9.x (PhysX) only**: `moi`, `clutch_strength`, `gear_switch_time`,
  `forward_gears` (list of `GearPhysicsControl`), `use_gear_autobox`,
  `damping_rate_full_throttle`, `damping_rate_zero_throttle_clutch_engaged`,
- **0.10.0 (Chaos) instead**: `transmission_efficiency`, `gear_change_time`,
  `forward_gear_ratios` / `reverse_gear_ratios` (float lists, and **unreadable
  from Python** — no `std::vector<float>` converter), `use_automatic_gears`,
  `differential_type`, `front_rear_split`, `idle_rpm`, `rev_up_moi`,
  `rev_down_rate`, `brake_effect`, `chassis_height`, `chassis_width`, `drag_area`,
  `downforce_coefficient`, `inertia_tensor_scale`, `sleep_threshold`,
  `sleep_slope_limit`,
- `final_ratio`, `use_sweep_wheel_collision` (both versions),
- `torque_curve`, `steering_curve` (lists of `Vector2D`),
- `wheels` (list of `WheelPhysicsControl`: tyre friction, damping, steer angle,
  radius, suspension...).

Tuning wheels/curves is the `vehicle_physics.py` use case — read the current
control, edit specific entries, re-apply. `show_debug_telemetry(True)` overlays
live physics on a rendered server, handy while tuning.

## Sync mode

Under synchronous mode the vehicle only moves when the world is ticked, and
`--hold` counts wall-clock seconds — drive the tick loop (set-world-settings)
alongside, or control in async mode.

## Wheels, doors and light states (measured on ue58-dev ccc52d24d)

| Call | Behaviour |
|---|---|
| `set_wheel_animation_override(True)` + `set_wheel_steer_direction` / `set_wheel_pitch_angle` / `set_wheel_suspension_offset` | the getters return the values set (FL steer 30°, spin 90°, suspension 0.1 m) |
| `set_wheel_animation_override(False)` | steer and suspension return to the physics values; the spin angle keeps 90° until the wheel turns |
| `is_door_open(door)` | true for doors opened by any client; `VehicleDoor.All` is true once every door the model has is open |
| `open_door(Hood / Trunk)` on `vehicle.tesla.model3` | no effect: the model has only FL, FR, RL, RR |
| `world.get_vehicles_light_states()` | `{actor_id: VehicleLightState}` for every vehicle; the value is a bit mask, so `int(state) & int(VehicleLightState.LowBeam)` tests one light |
