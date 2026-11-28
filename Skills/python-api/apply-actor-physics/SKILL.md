---
name: apply-actor-physics
description: Acts on an actor's physics body on a running CARLA server — push it with a force or an impulse (at its centre of mass or at a point on it), spin it with a torque or an angular impulse, set its velocity or a vehicle's constant velocity, switch gravity or physics simulation off and on, and teleport it — then watches the actor and reports how far it moved, its speed and its angular velocity, so the effect is measured rather than assumed. Use when the user asks to "push the car", "launch it into the air", "make it spin", "freeze it in mid-air", "turn off gravity", "teleport the vehicle", "give it a kick sideways", or to set up a crash or a rollover.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Verified against ue58-dev ccc52d24d (CARLA 1.0) on Town10HD_Opt.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/physics.md
---

# Apply physics to an actor

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

Every command acts on one actor and then watches it for `--observe` seconds (default 2) of simulation time, printing the distance moved, the height change, the speed and peak speed, and the angular velocity. Use that line to confirm the effect: several of these calls do nothing on some actors, without an error.

Driving a vehicle (throttle, steer, brake) belongs to `control-vehicle`; walking a pedestrian belongs to `spawn-walkers`. Resolve the actor id with `world-data` when the request is ambiguous.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Resolve the actor id (world-data) if it is not the only hero
- [ ] Step 3: Apply the command and read the observe line
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Apply

```bash
source scripts/env.sh
P="python3 scripts/physics.py"

$P impulse  --role hero --vec 20000,0,0 --local          # kick forward, N*s
$P impulse  --role hero --vec 0,0,6000 --at 2,0,0        # lift the nose: impulse 2 m ahead of the origin
$P force    --role hero --vec 30000,0,0 --local --seconds 1   # push for 1 s, N
$P velocity --role hero --vec 0,0,0 --angular 0,0,0      # stop it dead
$P constant --role hero --vec 10,0,0                     # hold 10 m/s forward (vehicles)
$P constant --role hero --off
$P gravity  off --role hero                              # then impulses carry it like in space
$P physics  off --role hero                              # freeze it where it is, even in mid-air
$P teleport --role hero --to=105.5,-45.6,0.5 --yaw=-90
$P spin     --role hero --vec 0,0,2000000000             # yaw, Unreal units (see below)
```

`--local` reads `--vec` in the actor's frame: x forward, y right, z up. `--at` is always an offset in the actor's frame, in metres. Use the `=` form for values starting with a minus sign.

In synchronous mode, pass `--tick` if no other client is ticking; without it the script waits for another client's ticks.

## What works on what

Measured on a `vehicle.lincoln.mkz` (about 1700 kg), a pedestrian and a traffic cone:

| Call | Vehicle | Walker | Prop (`static.prop.*`) |
|---|---|---|---|
| `impulse`, `force`, `velocity` | yes | **no effect** | only after `physics on` |
| `spin`, `torque`, `--angular` | in the air only | no effect | after `physics on` |
| `gravity off` | yes; the suspension slowly lifts it | not tested | not tested |
| `physics off` | freezes it in place; impulses are then ignored | not tested | the default |
| `constant` | yes | refused | refused |

## Gotchas

- **One `add_force` does nothing.** Unreal applies a force for one physics step only. One call moved the car 0 m; reapplied for 0.3 s it moved 1.43 m. `force` and `torque` therefore reapply every frame for `--seconds`. Use `impulse` for a one-off kick.
- **Angular calls take Unreal units.** The server converts forces and impulses from SI to centimetres, but passes `add_angular_impulse` and `add_torque` through unchanged, in kg·cm²·degrees. On the mkz in the air, an angular impulse of 3,000,000 did nothing visible, and 2,000,000,000 gave 102 deg/s. A torque of 2,000,000,000 for 1 s reached 3600 deg/s, Unreal's angular speed cap.
- **Wheels on the ground cancel rotation.** On the ground, `--angular 0,0,90` fell to 7.5 deg/s within 0.1 s, and a spin that turned the car at 102 deg/s in the air did nothing. Lift the car first (teleport up and `gravity off`) or use `--at` with a vertical impulse to tip it over.
- **Walkers ignore physics calls.** Walkers are moved by their controller. An impulse and `set_target_velocity` both left the walker where it was. Use `WalkerControl` (`spawn-walkers`).
- **Props start with physics off.** `static.prop.*` actors are spawned without simulated physics, so every call is silently ignored until `physics on`.
- **`constant` is in the vehicle's frame.** `--vec 10,0,0` is 10 m/s forward, whatever the heading. Only the component along the vector is held, so the vehicle can still turn. It overrides the Traffic Manager.

## Examples

**Example 1: a car on its roof**

User says: "flip the ego onto its roof"

Do not try to flip it with an impulse: upward impulses of 8000 to 40000 N·s at 1 m to the right tipped the mkz, and every time it landed back on its wheels. Place it upside down instead, in Python: `actor.set_transform(carla.Transform(loc_2m_up, carla.Rotation(yaw=current_yaw, roll=180)))`. It stays on its roof (roll about -175° after it settles).

**Example 2: freeze a car in mid-air for a screenshot**

`teleport --role hero --to=X,Y,10`, then `physics off --role hero`.

**Example 3: stop a runaway vehicle**

`velocity --id 137 --vec 0,0,0 --angular 0,0,0`.

## Troubleshooting

**Problem: the observe line shows no movement**
Cause: a walker, a prop with physics off, a single `add_force`, an angular call too small in Unreal units, or a car on its wheels.
Solution: see the table and the gotchas above.

**Problem: the actor shoots off much faster than the impulse explains**
Cause: the impulse spun it into the ground and the collision threw it.
Solution: apply it at the centre of mass (no `--at`), or lift the actor first.

## Outputs

Changes one actor's physics state. The script destroys nothing and restores nothing: turn gravity or physics back on yourself.

The API table, units and every measurement are in [references/physics.md](references/physics.md).
