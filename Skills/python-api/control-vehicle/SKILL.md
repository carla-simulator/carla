---
name: control-vehicle
description: Directly drives a specific vehicle on a running CARLA server — raw VehicleControl (throttle/steer/brake/reverse/hand-brake), Ackermann speed control, vehicle lights, opening/closing doors, and physics-control tuning; ros-info reports the native ROS 2 command topics (vehicle_control_cmd, ackermann_control_cmd) for the hero. Targets the ego by default. Use when the user asks to "drive/steer the car", "accelerate/brake/reverse", "set a target speed", "turn on the headlights/blinkers/brake lights", "open a door", "drive the car from ROS", or "change the vehicle's mass/drag/physics". This is manual control (autopilot off) — for TM-driven traffic use control-traffic.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server with at least one vehicle. Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/vehicle-control.md
---

# Control a vehicle directly

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Drive one vehicle yourself: control input, Ackermann speed target, lights, doors,
physics. It targets **the ego** (`role_name = hero`) by default, or any vehicle by
`--id` / `--role` / `--filter`. This is **manual** control — `control` and
`ackermann` turn autopilot off first; for autonomous traffic use the
[`control-traffic`](../control-traffic/SKILL.md) skill instead.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Ensure a target vehicle exists (spawn-vehicles ego)
- [ ] Step 3: Apply the command (control / ackermann / lights / door / physics)
- [ ] Step 4: Confirm on a rendered server; `stop` to halt manual driving
```

Commands need `CARLA_HOST`/`CARLA_PORT`/`TM_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Control

```bash
source scripts/env.sh

# drive the ego forward for 3 s, then it brakes
python3 scripts/vehicle_control.py control --throttle 0.5 --hold 3

# turn while creeping forward
python3 scripts/vehicle_control.py control --throttle 0.3 --steer -0.4 --hold 2

# hold a target speed (built-in controller does throttle/brake for you)
python3 scripts/vehicle_control.py ackermann --speed 8

# lights: headlights + brake on; position off
python3 scripts/vehicle_control.py lights --on LowBeam,Brake --off Position
# left blinker on a specific car
python3 scripts/vehicle_control.py lights --filter '*prius*' --on LeftBlinker

# doors
python3 scripts/vehicle_control.py door --open FL,FR
python3 scripts/vehicle_control.py door --close All

# physics: read, then make it heavier / draggier
python3 scripts/vehicle_control.py physics --show
python3 scripts/vehicle_control.py physics --mass 2200 --drag 0.4

# emergency stop
python3 scripts/vehicle_control.py stop

# what ROS 2 topics drive this vehicle (read-only)
python3 scripts/vehicle_control.py ros-info
```

### Driving it from ROS 2

On a server started with `--ros2` ([[run-carla-server]] `ROS2=1`), the **hero**
vehicle exposes two command topics — the server subscribes, so the commands come
from *outside* this skill:

| Topic (DDS name) | Message | Maps to |
|---|---|---|
| `rt/carla/<ros_name>/vehicle_control_cmd` | `carla_msgs/CarlaEgoVehicleControl` | `carla.VehicleControl` (throttle, steer, brake, hand_brake, reverse, gear, manual_gear_shift) |
| `rt/carla/<ros_name>/ackermann_control_cmd` | `ackermann_msgs/AckermannDriveStamped` | `carla.VehicleAckermannControl` (steering_angle, speed, acceleration, jerk) |

`ros-info` prints them for the resolved vehicle plus a ready-to-run
`ros2 topic pub` line, and says so plainly when the target **cannot** be driven
from ROS: only `role_name = hero` gets subscribers (the server tests that string
in `ActorDispatcher`), so ordinary traffic never does. Spawn the hero with
[[spawn-vehicles]] `ego --ros-name <name>`.

**You need the message definitions to publish.** Verified: neither type ships with
a stock ROS 2 install, so `ros2 topic pub carla_msgs/msg/...` fails with an unknown
type until you provide them:

| Type | Where it comes from |
|---|---|
| `ackermann_msgs/AckermannDriveStamped` | `sudo apt install ros-$ROS_DISTRO-ackermann-msgs` |
| `carla_msgs/CarlaEgoVehicleControl` | **not** an apt package — build `carla_msgs` from `carla-ros-bridge`, or hand-write a one-message package (below) |

A minimal `carla_msgs` is enough and takes a minute — this exact recipe was used to
drive the hero from ROS (0 → 64 km/h) on a packaged server:

```bash
mkdir -p ws/src/carla_msgs/msg
cat > ws/src/carla_msgs/msg/CarlaEgoVehicleControl.msg <<'EOF'
std_msgs/Header header
float32 throttle
float32 steer
float32 brake
bool hand_brake
bool reverse
int32 gear
bool manual_gear_shift
EOF
# + a standard ament_cmake package.xml / CMakeLists.txt calling
#   rosidl_generate_interfaces(${PROJECT_NAME} "msg/CarlaEgoVehicleControl.msg"
#                              DEPENDENCIES std_msgs)
colcon build --packages-select carla_msgs && source install/setup.bash
ros2 topic pub -r 20 /carla/hero/vehicle_control_cmd \
  carla_msgs/msg/CarlaEgoVehicleControl "{throttle: 0.8}"
```

**Field order is the contract**, not the field names: CDR is positional, so the
`.msg` must list exactly those eight fields in that order to match CARLA's POD.
The package name and message name must be `carla_msgs` / `CarlaEgoVehicleControl`
so the DDS type name (`carla_msgs::msg::dds_::CarlaEgoVehicleControl_`) matches.

Publishing from a container? Add `-e HOME=/tmp` — with `--user` and no writable
`HOME`, `rcl` aborts at startup with `Failed to create log directory: //.ros/log`.

The server side needs none of this: `ros-info` confirms the subscription exists
(`Subscription count: 1`) regardless, and the type name resolves from DDS.

Two things that bite:

- **Both paths write the same `VehicleControl`.** A ROS publisher sending at 20 Hz
  and this skill's `control` fight; last writer wins. Pick one driver.
- **Autopilot still overrides manual input.** ROS commands are manual input, so
  the TM must be off for them to have any effect (`control`/`ackermann` here
  disable it for you; a ROS-only workflow must not enable it).

A ROS 2 installation is needed only to *publish* — nothing in this skill requires
one ([[visualize-ros-rviz]] has containers if the host has no ROS 2).

### Step 4: Verify

Watch on a rendered server. A `VehicleControl` **persists** — the car keeps the
last input until you change it, so use `--hold` for a timed action or `stop` to
halt. `physics --show` reads back the applied values; `telemetry` overlays live
physics on screen.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

Physics control is the one part of this skill that is **engine-specific**. UE4
builds (0.9.x) use PhysX; 0.10.0 uses Chaos, and `VehiclePhysicsControl` was
replaced field-for-field. Verified live against a 0.10.0 server:

| 0.9.x (PhysX) | 0.10.0 (Chaos) |
|---|---|
| `forward_gears` — list of `GearPhysicsControl` | `forward_gear_ratios` / `reverse_gear_ratios` — float lists; **`carla.GearPhysicsControl` no longer exists** |
| `clutch_strength`, `gear_switch_time`, `use_gear_autobox` | `transmission_efficiency`, `gear_change_time`, `use_automatic_gears`, `differential_type`, `front_rear_split`, `final_ratio` |
| `moi`, `damping_rate_full_throttle`, `damping_rate_zero_throttle_clutch_engaged/disengaged` | `rev_up_moi`, `rev_down_rate`, `idle_rpm`, `brake_effect`, `inertia_tensor_scale` |
| — | `chassis_height`, `chassis_width`, `drag_area`, `downforce_coefficient`, `sleep_threshold`, `sleep_slope_limit` |

Wheels were renamed too: `radius`→`wheel_radius`, `tire_friction`→
`friction_force_multiplier`, `position`→`location`, `max_handbrake_torque`→
`max_hand_brake_torque`, `lat_stiff_*`/`long_stiff_value`→`cornering_stiffness` +
`spring_rate`/`suspension_*`, plus new `axle_type`, `abs_enabled`,
`traction_control_enabled`, `sweep_type`, `wheel_mass`, `wheel_width`.

**Two gear fields are unreadable from Python on 0.10.0.** `forward_gear_ratios`
and `reverse_gear_ratios` are declared with no `std::vector<float>` converter, so
reading either raises `TypeError: No to_python (by-value) converter found for C++
type: std::__1::vector<float, ...>`. Every other field on the struct reads fine,
and `physics --show` reports `gears=unreadable (0.10.0 converter gap)` rather than
dying. Writing and applying still works: a `mass` change round-tripped
1696 → 1750 kg.

`mass`, `drag_coefficient`, `max_rpm`, `center_of_mass`, `torque_curve`,
`steering_curve` and `wheels` are spelled the same on both, which is why the
`--mass` / `--drag` / `--max-rpm` flags need no version handling.

## Examples

**Example 1: nudge the ego forward**

User says: "drive the car forward a bit"

`control --throttle 0.4 --hold 2` — it accelerates for 2 s then brakes.

**Example 2: hazard lights**

User says: "put the hazards on"

`lights --on LeftBlinker,RightBlinker` (both blinkers = hazards).

**Example 3: a heavier truck feel**

User says: "make the ego handle like it's loaded"

`physics --mass 2500 --drag 0.45`, then drive and compare.

## Troubleshooting

**Problem: the car won't respond to control**
Cause: autopilot is still on (TM overrides manual input).
Solution: `control`/`ackermann` disable autopilot automatically; if you set it via
another tool, ensure autopilot is off. Sync mode also needs a world tick to move.

**Problem: it drives off and won't stop**
Cause: `VehicleControl` persists (throttle stays applied).
Solution: `stop`, or use `--hold` for timed inputs.

**Problem: "no vehicle with role hero"**
Cause: no ego spawned.
Solution: spawn one with the spawn-vehicles `ego` command, or target `--id`/`--filter`.

**Problem: no door named Hood/Trunk**
Cause: 0.9.16 exposes only FL, FR, RL, RR, All.
Solution: use those; not all vehicle models animate every door.

## Outputs

Vehicle state on the server (motion, lights, doors, physics). No file. Verify on a
rendered view; `physics --show` reads values back.

Detail (VehicleControl vs Ackermann, light flags, door set, the physics fields) in
[references/vehicle-control.md](references/vehicle-control.md).
