# Leaderboard sensors, data shapes and agent base classes

## The seven allowed sensor types

`ALLOWED_SENSORS = SENSORS_LIMITS.keys()` in
`leaderboard/autoagents/agent_wrapper.py`. Anything else is rejected:

```
sensor.camera.rgb      sensor.lidar.ray_cast   sensor.other.radar
sensor.other.gnss      sensor.other.imu        sensor.opendrive_map
sensor.speedometer
```

Note what is **not** there: depth and semantic cameras, DVS, instance
segmentation, optical flow, collision and lane-invasion sensors, V2X. Those exist
in CARLA ([[create-sensor]]) but are not admissible in a leaderboard submission —
because they are privileged information or not sensor-realistic.

## Budgets

| Sensor | SENSORS / MAP (2.x) | SENSORS_QUALIFIER / MAP_QUALIFIER (2.x) | LB 1.0 |
|---|---|---|---|
| `sensor.camera.rgb` | 8 | 4 | 4 |
| `sensor.lidar.ray_cast` | 2 | 1 | 1 |
| `sensor.other.radar` | 4 | 2 | 2 |
| `sensor.other.gnss` | 1 | 1 | 1 |
| `sensor.other.imu` | 1 | 1 | 1 |
| `sensor.opendrive_map` | 1 (MAP only) | 1 (MAP only) | 1 (MAP only) |
| `sensor.speedometer` | 1 | 1 | 1 |

Plus: unique `id`s, and `sqrt(x²+y²+z²) <= MAX_ALLOWED_RADIUS_SENSOR` = **3.0 m**.

## Fixed attributes

`AgentWrapper._preprocess_sensor_spec` overrides everything except camera
resolution/fov and the mount transform. What you actually get:

**`sensor.camera.rgb`** — yours: `width`, `height`, `fov`, `x/y/z/roll/pitch/yaw`.

**`sensor.lidar.ray_cast`** — all fixed:
```
range 85          rotation_frequency 10       channels 64
upper_fov 10      lower_fov -30               points_per_second 600000
atmosphere_attenuation_rate 0.004
dropoff_general_rate 0.45   dropoff_intensity_limit 0.8   dropoff_zero_intensity 0.4
```
Only the mount is yours. Note `rotation_frequency 10` against a 20 Hz simulation:
each tick delivers **half** a revolution.

**`sensor.other.radar`** — yours: `horizontal_fov`, `vertical_fov` and the mount.
Fixed: `points_per_second 1500`, `range 100`.

**`sensor.other.gnss`** — mount only (rotation ignored). Fixed noise:
`noise_{alt,lat,lon}_stddev 5e-6`, biases 0.

**`sensor.other.imu`** — mount only. Fixed noise:
`accel_stddev x/y 0.001, z 0.015`; `gyro_stddev 0.001` on all axes.

**Pseudo-sensors** — `sensor.opendrive_map` and `sensor.speedometer` are not CARLA
actors. They are `BaseReader` threads in `leaderboard/envs/sensor_interface.py`,
given `reading_frequency = 1/fixed_delta_seconds` (i.e. every tick at 20 Hz), and
they ignore any transform you give them.

## `input_data` shapes

`run_step(self, input_data, timestamp)` receives `{id: (frame, data)}` where `data`
comes from the parse callbacks in `sensor_interface.py`:

| Sensor | `data` |
|---|---|
| `camera.rgb` | numpy `(height, width, 4)` uint8, **BGRA** |
| `lidar.ray_cast` | numpy `(N, 4)` float32 — x, y, z, intensity |
| `other.radar` | numpy `(N, 4)` float32 — depth, azimuth, altitude, velocity |
| `other.gnss` | numpy `[lat, lon, alt]` |
| `other.imu` | numpy `[ax, ay, az, gx, gy, gz, compass]` |
| `speedometer` | `{'speed': <m/s>}` — forward speed, not the velocity magnitude |
| `opendrive_map` | `{'opendrive': <the .xodr as a string>}` |

The camera is BGRA because it is the raw CARLA buffer: `img[:, :, :3][:, :, ::-1]`
gives RGB.

`speedometer` is computed as the projection of velocity on the vehicle's forward
vector (`SpeedometerReader._get_forward_speed`), so it is signed and reverse reads
negative.

`opendrive_map` returns the **whole map** every tick. Parse it once in the first
`run_step` and cache it.

If a sensor produces nothing within the timeout, the harness raises
`SensorReceivedNoData` and the route is marked crashed.

## The route plan

Set before the first `run_step` by `set_global_plan`:

```python
self._global_plan             # [({'lat':…, 'lon':…, 'z':…}, RoadOption), …]  downsampled
self._global_plan_world_coord # [(carla.Transform, RoadOption), …]            downsampled
```

`RoadOption` is `LANEFOLLOW`, `LEFT`, `RIGHT`, `STRAIGHT`, `CHANGELANELEFT`,
`CHANGELANERIGHT` or `VOID`. The plan is downsampled by
`leaderboard/utils/route_manipulation.downsample_route`, so it marks decision
points, not a dense trajectory. The GPS form is in the same frame as the `gnss`
sensor, which is what makes localisation-free following possible on the SENSORS
track.

## Agent lifecycle

```
__init__(carla_host, carla_port, debug=False)   # 2.x; (path_to_conf_file) in 1.0
setup(path_to_conf_file)                        # called by the evaluator (2.x)
sensors()                                       # once, before the route; validated
set_global_plan(...)                            # before the first tick
run_step(input_data, timestamp)   x N           # every tick, must return VehicleControl
destroy()                                       # between routes
```

`setup()` must set `self.track` to the `Track` enum member matching `--track`, or
validation rejects the submission. Do heavy loading here, never in `run_step`: the
setup watchdog is separate and more generous than the per-tick one.

`destroy()` runs between routes in the same process. Anything not released there
accumulates across a 90-route run.

## Reference agents

| File | What it is |
|---|---|
| `leaderboard/autoagents/dummy_agent.py` | minimal template, zero control |
| `leaderboard/autoagents/npc_agent.py` | Traffic-Manager autopilot following the route — the "setup works" baseline |
| `leaderboard/autoagents/human_agent.py` | pygame window, keyboard, optional side mirrors |
| `leaderboard/autoagents/log_agent.py` | replays a recorded control log (master only) |
| `leaderboard/autoagents/ros1_agent.py`, `ros2_agent.py` | thin subclasses of `ROSBaseAgent` |

## ROS agents

`leaderboard/autoagents/ros_base_agent.py` provides `ROSBaseAgent`, subclassed by
`ROS1Agent` and `ROS2Agent`. You implement:

```python
def get_ros_entrypoint(self):
    return {"package": "my_stack", "launch_file": "my_stack.launch.py", "parameters": {...}}
def sensors(self):
    return [...]
```

The base class:

- launches your package with `ROSLauncher` and monitors it (`is_alive`,
  `terminate`), piping its output through `ROSLogger`,
- bridges each declared sensor onto ROS topics, converting CARLA's left-handed
  coordinates with `BridgeHelper.carla2ros_pose`,
- subscribes to a vehicle control command topic
  (`_vehicle_control_cmd_callback`) and returns that as the agent's control,
- so `run_step` ignores `input_data` entirely — the data goes to ROS, the control
  comes back from ROS.

`AgentWrapperFactory.get_wrapper()` returns a `ROSAgentWrapper` for these instead
of the plain `AgentWrapper`.

The ROS bridge is a separate repo, branch-matched:
`git clone --recurse-submodules -b leaderboard-2.1 https://github.com/carla-simulator/ros-bridge`.
Build the docker image with `scripts/make_docker.sh -r <melodic|noetic|foxy>`,
which needs `CARLA_ROS_BRIDGE_ROOT` set as well.

ScenarioRunner's `Docs/ros_agent.md` and `srunner/autoagents/ros_agent.py` describe
the **old** `RosAgent`, which the leaderboard deleted. Ignore them.
