---
name: create-sensor
description: Spawns and attaches sensors on a running CARLA server — cameras (rgb/depth/semantic/instance/optical-flow/normals), lidar, radar, IMU, GNSS, collision/lane-invasion/obstacle — to the ego or any actor, with a mount transform and blueprint attributes (resolution, fov, sensor_tick, lidar range/points, …). Also sets up native ROS 2 publishing (--ros to enable_for_ros, --ros-name/--ros-frame-id topic and TF naming) and prints the topics the sensor will publish. Use when the user asks to "add a camera/lidar/sensor", "put a dashcam on the ego", "attach a depth camera", "mount a sensor", or "publish a sensor to ROS". Prints the sensor id; read its data with the read-sensor skill.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server (and an actor to attach to, for attached sensors). Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/sensors.md
---

# Create a sensor

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Spawn a sensor and (usually) attach it to a vehicle so it moves with it. The
sensor persists as an actor; its **id** is printed so the
[`read-sensor`](../read-sensor/SKILL.md) skill can save or display its data. This
skill is placement/lifecycle only — reading is the other skill.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Have a parent actor if attaching (spawn-vehicles ego)
- [ ] Step 3: Spawn the sensor with a type, mount transform, and attributes
- [ ] Step 4: Note the printed id; read it with read-sensor; destroy when done
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Spawn

```bash
source scripts/env.sh

python3 scripts/sensors.py types                     # list sensor blueprints

# an 800x600 dashcam on the ego (default mount x=1.5, z=2.4)
python3 scripts/sensors.py spawn --type camera.rgb --attach-to hero \
    --attr image_size_x=800 --attr image_size_y=600 --attr fov=90

# a lidar on the ego roof
python3 scripts/sensors.py spawn --type lidar.ray_cast --attach-to hero --z 2.5 \
    --attr range=50 --attr points_per_second=200000 --attr rotation_frequency=20

# a depth camera; a GNSS; a collision sensor
python3 scripts/sensors.py spawn --type camera.depth --attach-to hero
python3 scripts/sensors.py spawn --type other.gnss --attach-to hero
python3 scripts/sensors.py spawn --type other.collision --attach-to hero

python3 scripts/sensors.py destroy                   # remove all sensors

# turn ROS publishing on for sensors that already exist
python3 scripts/sensors.py ros --id 123
python3 scripts/sensors.py ros --filter 'sensor.camera.*' --disable
```

`--attach-to <role>` attaches to that vehicle (ego = `hero`); `--parent-id N`
attaches to any actor; omit both for a world-fixed sensor. `--attachment SpringArm`
gives a smooth chase mount. Repeat `--attr key=value` for blueprint attributes.

### Publishing to ROS 2

Only meaningful on a server started with `--ros2` ([[run-carla-server]] `ROS2=1`).

```bash
# a lidar that publishes on rt/carla/hero/lidar/point_cloud
python3 scripts/sensors.py spawn --type lidar.ray_cast --attach-to hero --z 2.4 \
    --ros --ros-name lidar
# a camera with its own TF frame and no transform broadcast
python3 scripts/sensors.py spawn --type camera.rgb --attach-to hero \
    --ros --ros-name front --ros-frame-id front_cam --no-ros-tf
```

**`--ros` is not optional decoration — without it the sensor publishes nothing.**
A CARLA sensor is only ticked while something listens to its stream
(`ASensor::Tick` returns early otherwise), and `enable_for_ros()` is what marks
the stream as listened-to without a Python client. `--ros-name`/`--ros-frame-id`/
`--no-ros-tf` set the `ros_name` / `ros_frame_id` / `ros_publish_tf` blueprint
attributes, which the server reads **once at registration**: they cannot be
changed after spawn.

Topic layout (`spawn` prints the exact list):

| Sensor | Topics under the base name | Type |
|---|---|---|
| any camera | `/image`, `/camera_info` | `sensor_msgs/Image`, `CameraInfo` |
| `camera.dvs` | `/image`, `/camera_info`, `/point_cloud` | + `PointCloud2` |
| `lidar.*`, `other.radar` | `/point_cloud` | `sensor_msgs/PointCloud2` |
| `other.imu` | *(the base name itself)* | `sensor_msgs/Imu` |
| `other.gnss` | *(the base name itself)* | `sensor_msgs/NavSatFix` |
| `other.collision` | *(the base name itself)* | `carla_msgs/CarlaCollisionEvent` |
| `other.lane_invasion`, `other.obstacle`, `other.rss` | **none** — no native publisher exists | — |

Base name is `rt/carla/<ros_name>`, nested as
`rt/carla/<parent ros_name>/<ros_name>` when attached; unnamed sensors become
`actor<id>`. Transforms go to the single `rt/tf` topic, parented to the attach
parent's frame (or `map`).

All of this is verified against a live server: a lidar spawned as
`--attach-to hero --ros-name lidar` **without** `--ros` produced no topic at all;
`ros --id N` made `/carla/hero/lidar/point_cloud` **and** `/tf` appear at once —
`/tf` exists only because a sensor publishes, never from the vehicle alone. Two
practical notes:

- **Rates follow the tick.** With `sensor_tick` unset on an async `-nullrhi`
  server that lidar published at ~2.7 kHz. Set `--attr sensor_tick=0.05` or use
  sync mode unless you want tick-rate floods.
- **Durability is `transient_local` on fastdds for every topic**, not just the
  latched map — the middleware only raises durability and Fast DDS already
  defaults there. Reliability is as expected (best-effort for image/point cloud).

Read them with [[read-sensor]] `ros-info`, list everything with [[world-data]]
`ros-topics`, and echo them with [[visualize-ros-rviz]].

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

The sensor catalogue is **larger** on 0.10.0. Live from a 0.10.0 server, the
blueprints that do not exist on 0.9.x:

| New blueprint | What it is |
|---|---|
| `sensor.camera.rgb_fisheye`, `.depth_fisheye`, `.semantic_segmentation_fisheye`, `.instance_segmentation_fisheye` | fisheye projections of the four cameras |
| `sensor.camera.rt_lens` | ray-traced lens camera — **UE 5.8 only** |
| `sensor.lidar.hss_lidar` | high-solid-state lidar model |
| `sensor.other.autoware_gnss`, `sensor.other.vehicle_status` | Autoware-shaped GNSS and vehicle-status publishers — **UE 5.8 only** |

**The RGB camera's attribute set is much smaller.** On 0.10.0 `sensor.camera.rgb`
exposes 18 attributes: `image_size_x/y`, `fov`, `sensor_tick`, `role_name`,
`ros_name`, `ros_topic_name`, `enable_postprocess_effects`, `post_process_profile`,
`enable_dlss`, `dlss_screen_percentage` (both **UE 5.8 only**), `use_ray_tracing`,
and the six `lens_*` distortion knobs. The per-shot photographic controls 0.9.x exposes —
`bloom_intensity`, `fstop`, `iso`, `gamma`, `shutter_speed`,
`motion_blur_intensity`, `chromatic_aberration_intensity`, `exposure_mode`,
`lens_flare_intensity`, `blur_amount`, `slope`, `toe`, `tint` — are **not on the
blueprint**, even though the definitions still exist in
`ActorBlueprintFunctionLibrary.cpp`. Post-processing is selected wholesale
instead, by naming a profile: `post_process_profile` (default `Default`) resolves
to a JSON under `Content/Carla/Config/PostProcess/` — shipped are `Default.json`,
`GoPro.json`, `Town10HD_Opt.json`, `Town_C.json`. So `--attr bloom_intensity=…`
fails on 0.10.0; author or pick a profile.

**ROS 2 enabling moved class, not concept.** `enable_for_ros` /
`disable_for_ros` / `is_enabled_for_ros` live on `carla.Actor` in 0.9.x and on
`carla.ServerSideSensor` in 0.10.0 (`PythonAPI/carla/src/Sensor.cpp:37-39`). A
spawned server-side sensor *is* a `ServerSideSensor` on both, so `--ros` needs no
version handling — but note the methods are absent from the `carla.Sensor` base
class, which is what you get if you introspect the wrong type.

`ros_topic_name` is new alongside `ros_name`: it overrides the generated topic
exactly, rather than contributing a segment. `World.set_publish_tf()` /
`get_publish_tf()` are new too, controlling `rt/tf` globally instead of per
sensor — those two are **UE 5.8 only** ([[check-ue5-limitations]]).

## Examples

**Example 1: dashcam on the ego**

User says: "put a camera on the ego"

Spawn an ego (spawn-vehicles `ego`), then `spawn --type camera.rgb --attach-to hero`.
Note the id, then `read-sensor show --id <id>`.

**Example 2: a lidar dataset rig**

User says: "add a lidar for capturing point clouds"

`spawn --type lidar.ray_cast --attach-to hero --attr range=100 --attr points_per_second=500000`,
then `read-sensor save --id <id> --out ./lidar`.

## Troubleshooting

**Problem: "no vehicle with role hero"**
Cause: nothing to attach to.
Solution: spawn an ego (spawn-vehicles), or use `--parent-id`, or omit to place a
world-fixed sensor.

**Problem: "<bp> has no attribute X"**
Cause: wrong attribute for that sensor type.
Solution: `types` then check the blueprint; cameras use image_size_x/y/fov,
lidar uses range/points_per_second/channels/rotation_frequency.

**Problem: server runs with `--ros2`, `/clock` is there, but the sensor's topic never appears**
Cause: the sensor was spawned without `--ros` — with no listener it is never
ticked, so no data and no topic (the topic is created on first publish).
Solution: re-spawn with `--ros`, or call `enable_for_ros()` on the actor.

**Problem: the ROS topic name is wrong / says `actor42`**
Cause: `ros_name` is read once at registration; no `--ros-name` was given.
Solution: destroy and re-spawn with `--ros-name`. Renaming a live sensor is not
possible.

**Problem: attached camera clips through the car / bad angle**
Cause: mount transform is relative to the parent.
Solution: adjust `--x/--y/--z/--pitch`; a windshield cam is ~x=1.5, z=1.3.

## Outputs

A sensor actor on the server (id printed), attached or world-fixed. No data yet —
attach a listener with the read-sensor skill. `destroy` removes sensors.

Detail (sensor families, attributes, attachment types, sensor_tick) in
[references/sensors.md](references/sensors.md).
