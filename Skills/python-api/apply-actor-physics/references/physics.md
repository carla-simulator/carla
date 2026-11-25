# Actor physics reference

## API and units

| Python call | Unit the client passes | What the server does |
|---|---|---|
| `add_force(v)` | N | ×100 to Unreal units, `AddForce` for one physics step |
| `add_force_at_location(v, loc)` | N, world metres | both ×100, `AddForceAtLocation` |
| `add_impulse(v)` | N·s | ×100, `AddImpulse` |
| `add_impulse_at_location(v, loc)` | N·s, world metres | both ×100, `AddImpulseAtLocation` |
| `add_torque(v)` | none: passed through | `AddTorqueInDegrees`, kg·cm²·deg/s² |
| `add_angular_impulse(v)` | none: passed through | `AddAngularImpulseInDegrees`, kg·cm²·deg/s |
| `set_target_velocity(v)` | m/s, world axes | sets the body's linear velocity before the step |
| `set_target_angular_velocity(v)` | deg/s, world axes | sets the body's angular velocity before the step |
| `enable_constant_velocity(v)` | m/s, vehicle frame | holds the component along `v` every tick; vehicles only |
| `set_enable_gravity(b)` / `set_simulate_physics(b)` | | on the root primitive |
| `set_location(l)` / `set_transform(t)` | m, deg | teleport |

Source: `Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Actor/CarlaActor.cpp` and the `add_actor_*` binds in `Server/CarlaServer.cpp`. Constant velocity is `Vehicle/VehicleVelocityControl.cpp`.

## Measurements

ue58-dev ccc52d24d, Town10HD_Opt, asynchronous mode unless stated; `vehicle.lincoln.mkz`, so impulse ÷ speed gives about 1700 kg.

| Test | Result |
|---|---|
| one `add_force(30000 N)` forward | moved 0.00 m in 1 s |
| `add_force(30000 N)` reapplied for 0.3 s | moved 1.43 m, peak 4.79 m/s |
| `add_impulse(0,0,6000)`, gravity off, in the air | 3.53 m/s upward |
| same impulse at 2 m ahead of the origin | 3.53 m/s upward, pitch rate 237 deg/s |
| `add_angular_impulse` z = 3,000,000, in the air | no visible rotation |
| `add_angular_impulse` z = 2,000,000,000, in the air | 102 deg/s yaw |
| `add_torque` z = 2,000,000,000 for 1 s, in the air | 3600 deg/s (the cap) |
| `set_target_angular_velocity(0,0,90)` on the ground | 7.5 deg/s after 0.1 s |
| same in the air | 90.0 deg/s |
| `set_enable_gravity(False)` on the ground | rises at about 0.24 m/s, pushed by the suspension |
| `set_simulate_physics(False)` in mid-air | stays in place; an impulse then moves it 0 m |
| `enable_constant_velocity(0,8,0)` | 8 m/s sideways on top of the existing forward speed: 16 m/s total |
| upward impulse 8000–40000 N·s at 1 m to the right | the car tips and lands back on its wheels every time |
| `set_transform` with `roll=180`, 2 m up | stays on its roof at roll ≈ -175° |
| walker: `add_impulse(0,300,200)`, then `set_target_velocity(0,3,0)` | no movement |
| `static.prop.trafficcone02`: impulse | no movement until `set_simulate_physics(True)`, then it moves |
| synchronous mode, 0.05 s steps, `--tick` | same behaviour as asynchronous |

## Why `force` reapplies every frame

Unreal clears a body's accumulated force and torque after each physics step, so `add_force` and `add_torque` act for one step only. To push for a duration, call them once per frame. In asynchronous mode the script follows `wait_for_tick()`, so a call can occasionally miss a frame. In synchronous mode with `--tick`, each frame gets exactly one call.
