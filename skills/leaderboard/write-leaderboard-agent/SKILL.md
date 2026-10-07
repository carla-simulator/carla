---
name: write-leaderboard-agent
description: Writes an autonomous agent for the CARLA Leaderboard — an AutonomousAgent subclass with setup/sensors/run_step/destroy, a sensor suite inside the track's budget, and the right constructor signature for the leaderboard version (1.0 and 2.x differ). Generates a working skeleton, validates the sensor configuration against the real per-track limits offline, and covers the ROS1/ROS2 agent base classes. Use when the user asks to "write a leaderboard agent", "make my agent work with the leaderboard", "add sensors to my agent", "port my agent to 2.1", or hits SensorConfigurationInvalid.
license: MIT
compatibility: Any OS with a leaderboard + scenario_runner checkout. Validation runs offline. Running the agent needs a matching CARLA. Sensor budgets and the constructor signature depend on the leaderboard version — 1.0 differs from 2.0/2.1.
metadata:
  group: leaderboard
  prerequisites: scripts/check_env.sh
  reference: references/sensors.md
---

# Write a Leaderboard agent

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

An agent is one Python file with one class. The Leaderboard imports it by path and
instantiates it **by a name derived from the file name**:

```python
module_name = os.path.basename(args.agent).split('.')[0]   # my_agent.py -> my_agent
# ... the evaluator then looks for the TitleCase, underscore-stripped class
```

so `my_agent.py` must contain `class MyAgent`. That is the first thing that goes
wrong, and the error is a bare `AttributeError`.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh) — confirms the LB version
- [ ] Step 2: Generate the skeleton for your track and version
- [ ] Step 3: Declare sensors inside the budget; validate offline
- [ ] Step 4: Implement run_step
- [ ] Step 5: Smoke-test on routes_devtest.xml
```

### Step 2: Skeleton

```bash
source scripts/env.sh

python3 scripts/agent_tools.py scaffold --name MyAgent --track SENSORS --out ~/team_code
python3 scripts/agent_tools.py scaffold --name MyMapAgent --track MAP --out ~/team_code
```

The scaffold reads the detected leaderboard version and emits the **correct
constructor** for it — this is the one hard API break between 1.0 and 2.x:

```python
# 1.0 — the base class calls setup() from __init__
def __init__(self, path_to_conf_file): ...

# 2.0 / 2.1 — the evaluator constructs, then calls setup() itself
def __init__(self, carla_host, carla_port, debug=False): ...
```

A 1.0 agent run under a 2.x evaluator receives a host string where it expects a
config path. If you override `__init__` at all, match the version; if you do not
override it, both work.

### Step 3: Sensors

```python
def sensors(self):
    return [
        {'type': 'sensor.camera.rgb', 'id': 'Center',
         'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
         'width': 800, 'height': 600, 'fov': 100},
        {'type': 'sensor.lidar.ray_cast', 'id': 'LIDAR',
         'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0},
        {'type': 'sensor.other.imu',  'id': 'IMU', 'x': 0.7, 'y': 0.0, 'z': 1.60,
         'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0},
        {'type': 'sensor.other.gnss', 'id': 'GPS', 'x': 0.7, 'y': -0.4, 'z': 1.60},
        {'type': 'sensor.speedometer', 'id': 'Speed'},
    ]
```

```bash
python3 scripts/agent_tools.py validate --agent ~/team_code/my_agent.py --track SENSORS
```

`validate` runs the **actual** `validate_sensor_configuration()` from your
checkout, so it enforces the real budget for your version and track rather than a
copy of it. Budgets (2.0/2.1; qualifier tracks halve them, and match 1.0's):

| Sensor | SENSORS / MAP | *_QUALIFIER and LB 1.0 |
|---|---|---|
| `sensor.camera.rgb` | 8 | 4 |
| `sensor.lidar.ray_cast` | 2 | 1 |
| `sensor.other.radar` | 4 | 2 |
| `sensor.other.gnss` | 1 | 1 |
| `sensor.other.imu` | 1 | 1 |
| `sensor.speedometer` | 1 | 1 |
| `sensor.opendrive_map` | 1 (**MAP tracks only**) | 1 |

Three constraints beyond the counts:

- **Every `id` must be unique** — duplicates are rejected outright.
- **Mount radius ≤ 3.0 m** from the ego origin: `sqrt(x²+y²+z²) > 3.0` is rejected.
- **`sensor.opendrive_map` on a SENSORS track is rejected**, which is the entire
  difference between the SENSORS and MAP tracks.

**You do not control sensor attributes except camera resolution/fov and the mount.**
`agent_wrapper.py` hard-codes the rest: lidar is always 64 channels, 85 m range,
10 Hz, 600k points/s with fixed dropoff; radar 1500 points, 100 m; GNSS and IMU
noise are fixed. Setting `range` or `channels` in your sensor dict is silently
ignored. That is deliberate — it is what makes submissions comparable.

### Step 4: `run_step`

```python
def run_step(self, input_data, timestamp):
    # input_data is {id: (frame, data)} for every sensor in sensors()
    frame, img = input_data['Center']          # numpy BGRA, (height, width, 4)
    _, gnss    = input_data['GPS']            # [lat, lon, alt]
    _, imu     = input_data['IMU']            # [ax, ay, az, gx, gy, gz, compass]
    _, speed   = input_data['Speed']          # {'speed': m/s}

    control = carla.VehicleControl()
    control.steer, control.throttle, control.brake = 0.0, 0.5, 0.0
    return control
```

- Return a `carla.VehicleControl` **every tick**. Returning `None` is a crash.
- `self._global_plan` (GPS waypoints + `RoadOption`) and
  `self._global_plan_world_coord` (world transforms) are set before the first
  `run_step`. On the `MAP` track you also get the OpenDRIVE string through
  `sensor.opendrive_map`.
- Camera images are **BGRA**, not RGB. Slice `[:, :, :3]` and reverse if you need RGB.
- The agent runs inside a watchdog. An overrun is `Agent crashed` /
  `Agent took longer than Xs`, not a warning — the route is scored as a failure.
- `destroy()` is called between routes; release models and close windows there or
  the next route starts with the memory still held.

### Step 5: Smoke test

```bash
cd ../run-leaderboard-evaluation
TEAM_AGENT=~/team_code/my_agent.py bash scripts/run_leaderboard.sh --routes-subset 0
```

Compare against `leaderboard/autoagents/npc_agent.py` first: if the NPC agent
completes and yours does not, the problem is your agent, not the setup.

## Examples

**Example 1: "write me a leaderboard agent with a front camera and lidar"**

`scaffold --name MyAgent --track SENSORS`, add the two sensors, `validate`, then
smoke-test on route 0 of `routes_devtest.xml`.

**Example 2: "my agent is rejected: Too many sensor.camera.rgb used"**

You are on a qualifier track (4 cameras) or LB 1.0 (4 cameras), not the 8-camera
main track. `validate --track SENSORS_QUALIFIER` shows the budget being applied.

**Example 3: "port my 1.0 agent to 2.1"**

Change `__init__(self, path_to_conf_file)` to
`__init__(self, carla_host, carla_port, debug=False)` and move the setup work into
`setup(path_to_conf_file)` — the evaluator calls it with `--agent-config`. Then
re-check the sensor budget: 2.x is more generous, so nothing breaks there, but the
qualifier tracks are not the main tracks.

**Example 4: "I want to drive with ROS"**

Subclass `ROS1Agent` or `ROS2Agent` from `leaderboard/autoagents/ros_base_agent.py`
instead of `AutonomousAgent`, implement `sensors()` and `get_ros_entrypoint()`. The
harness starts your stack and bridges the sensors as ROS topics. The old
`RosAgent` from ScenarioRunner is deleted — do not follow `Docs/ros_agent.md`.

## Troubleshooting

**Problem: `AttributeError: module 'my_agent' has no attribute 'MyAgent'`**
Cause: class name must be the file name TitleCased with underscores removed.
Solution: rename one of them.

**Problem: `SensorConfigurationInvalid: Duplicated sensor tag [X]`**
Cause: two sensors share an `id`.
Solution: unique ids; they are also the `input_data` keys.

**Problem: `SensorConfigurationInvalid: Illegal sensor extrinsics ... Max allowed radius is 3.0m`**
Cause: the mount is more than 3 m from the ego origin.
Solution: bring it in. A roof mount is about `x=0.7, z=1.6`.

**Problem: `SensorConfigurationInvalid: You are submitting to the wrong track`**
Cause: `self.track` in `setup()` disagrees with `--track` / `CHALLENGE_TRACK_CODENAME`.
Solution: set `self.track = Track.SENSORS` (or MAP/…) to match.

**Problem: `Illegal sensor 'sensor.opendrive_map' used for Track [Track.SENSORS]`**
Cause: map sensor on a sensors-only track.
Solution: switch to `MAP`, or drop the sensor.

**Problem: my lidar settings are ignored**
Cause: attributes are fixed by `agent_wrapper._preprocess_sensor_spec`.
Solution: expected — only camera `width`/`height`/`fov` and the mount transform
are yours.

**Problem: `Agent took longer than Xs to setup` / `Agent crashed`**
Cause: watchdog. Model loading in `run_step` instead of `setup`, or a per-tick
overrun.
Solution: load in `setup`; keep `run_step` bounded; raise `--timeout` while
debugging (but the submitted limit is fixed).

**Problem: black or empty images**
Cause: reading `input_data` for a sensor id that is not in `sensors()`, or
expecting RGB.
Solution: keys come from `sensors()`; data is BGRA.

## Outputs

An agent `.py` (plus an optional config file) that the evaluator can load, verified
against the real sensor validator for your version and track. `validate` exits
non-zero and names the violated rule when the configuration would be rejected.

Sensor semantics, data shapes, pseudo-sensors and the ROS agents are detailed in
[references/sensors.md](references/sensors.md).
