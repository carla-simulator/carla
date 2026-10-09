# Consuming CARLA's native ROS 2 topics

Detail layer for `visualize-ros-rviz`. Source: `LibCarla/source/carla/ros2/`
(publishers, `PublisherQos.h`, `middleware/`) and
`PythonAPI/examples/ros2/` (`README.md`, `run_*.sh`, `map_and_lidar_demo/`).
Verified live (2026-08) against a ROS-2-enabled packaged server with ROS 2 Humble
as the consumer, except where marked.

## Names: `rt/...` vs `/...`

The publishers are created with DDS topic names that start with `rt/`
(`rt/clock`, `rt/tf`, `rt/carla/map`, `rt/carla/<actor>/...`). `rt/` is the ROS 2
DDS name-mangling prefix for topics, so a ROS 2 node sees the same endpoints as
`/clock`, `/tf`, `/carla/map`, `/carla/<actor>/...`. Use the slash form with
`ros2 topic ...`; expect the `rt/` form in CARLA's own logs and code.

## What exists without any actor

| Topic | Type | QoS (observed, fastdds) | Note |
|---|---|---|---|
| `/clock` | `rosgraph_msgs/Clock` | reliable, transient_local, depth 1 | one sample per tick; **~4 kHz** on an async `-nullrhi` server |
| `/carla/map` | `std_msgs/String` | reliable, transient_local, depth 1 | full OpenDRIVE; overwritten on each map load |

`/tf` is **not** in that list — it only appears once a **sensor** publishes
(verified: a hero vehicle alone produces no `/tf`). CARLA emits sensor→parent
transforms; `map`→vehicle comes from the demo's `ego_tf_broadcaster.py`.

`/carla/map` is the *intentionally* latched topic — but note the QoS column:
**every** topic reports `transient_local` on Fast DDS, because the middleware only
ever raises durability and eProsima's default writer QoS is already
transient_local. So "volatile" is not a property you can rely on here, and a
`transient_local` subscription can hand you a stale sample on any topic. Reading
the map needs the matching request:

```bash
ros2 topic echo --once --qos-durability transient_local --qos-reliability reliable \
                --full-length /carla/map
```

Without `--qos-durability transient_local` the subscription is volatile and waits
forever, because the map is published only on episode start. Without
`--full-length` the OpenDRIVE string is truncated at 128 characters.

`/carla/map` has no `Header`, matching `carla-ros-bridge`'s `/carla/map`, so
there is no stamp or episode id — correctness comes from the single cached sample
being replaced on every load.

## Local ROS 2 needs the RMW profile (verified, silent otherwise)

The containers here mount `PythonAPI/examples/ros2/config` at `/config` and set
`FASTRTPS_DEFAULT_PROFILES_FILE` / `CYCLONEDDS_URI`. That is **not** cosmetic:
those files force **UDP-only** transport, and CARLA's Fast DDS is built with
shared memory that a stock local ROS 2 install cannot attach to. Skip the profile
in a local shell and you get the worst failure mode available — `ros2 topic list`
shows every topic, `ros2 topic hz` prints nothing, and no error appears on either
side. `ros_view.sh local-env` emits the right exports for your RMW.

## Per-actor topics

Base name `/carla/<ros_name>`, nested under the parent when attached
(`/carla/hero/lidar`), `actor<id>` when `ros_name` is unset.

| Producer | Topics | Type | QoS |
|---|---|---|---|
| any camera | `…/image`, `…/camera_info` | `sensor_msgs/Image`, `CameraInfo` | **best_effort**, transient_local, depth 1 |
| `camera.dvs` | + `…/point_cloud` | `sensor_msgs/PointCloud2` | best_effort |
| `lidar.*`, `other.radar` | `…/point_cloud` | `sensor_msgs/PointCloud2` | best_effort |
| `other.imu` | the base name | `sensor_msgs/Imu` | reliable |
| `other.gnss` | the base name | `sensor_msgs/NavSatFix` | reliable |
| `other.collision` | the base name | `carla_msgs/CarlaCollisionEvent` | reliable |
| hero vehicle (**subscribes**) | `…/vehicle_control_cmd`, `…/ackermann_control_cmd` | `carla_msgs/CarlaEgoVehicleControl`, `ackermann_msgs/AckermannDriveStamped` | reliable |

High-rate streams use `PublisherQos::SensorData()` — best-effort so a slow
subscriber can never stall the simulation thread. Consequences for consumers:

- A **reliable** subscriber on `/…/image` or `/…/point_cloud` will not match a
  best-effort publisher. RViz's default sensor profiles are best-effort; custom
  nodes must ask for best-effort explicitly (`rclpy.qos.qos_profile_sensor_data`).
- Depth is 1 everywhere. Slow consumers drop frames, they do not queue them.
- **Durability is transient_local on every topic** with fastdds, not just
  `/carla/map` (verified with `ros2 topic info -v`). The reliability column is the
  part that actually varies, and it is what governs matching.
- Rates follow the simulation tick, not the sensor: a lidar with `sensor_tick`
  unset measured ~2.7 kHz on async `-nullrhi`, while a camera with
  `sensor_tick=0.1` measured ~7 Hz on the packaged renderer.

## Simulation time

Stamps come from the simulator clock, published on `/clock`. Consumers must run
with `use_sim_time` (`ros2 run … --ros-args -p use_sim_time:=true`) or every
timestamp looks wrong and TF lookups fail on "extrapolation into the future".
The bundled RViz preset and demo helpers already do this.

## The demo stack

`run_map_and_lidar_demo.sh` runs one container (`--net=host`, fixed name,
`--init`) whose launcher starts three helpers together:

| Helper | Role |
|---|---|
| `ros2_native.py` | spawns the `stack.json` hero (camera, lidar, GNSS, IMU), sets `ros_name`/`role_name` per sensor, calls `enable_for_ros()` on each, drives on autopilot |
| `map_to_markers.py` | subscribes to the latched `/carla/map`, parses the OpenDRIVE **client-side** with the carla wheel (no simulator connection) and publishes latched `visualization_msgs/MarkerArray` on `/carla/map_markers` |
| `ego_tf_broadcaster.py` | publishes `map->hero` every tick, stamped with simulation time, completing the TF chain `map -> hero -> <sensor>` |

`cleanup.py` runs at both start and stop, so an unclean exit cannot leave a second
hero publishing on the same topics. The lane markers are **not** a CARLA topic —
they are computed by the demo from the OpenDRIVE.

`stack.json` is the sensor set; edit it to change the demo rig. The wheel baked
into the image must match the distro's Python (humble → cp310, jazzy → cp312).

## RMW notes

- `fastdds` and `cyclonedds` **do** interoperate (verified: a `rmw_fastrtps_cpp`
  subscriber read a `--rmw=cyclonedds` server at an identical ~213 Hz on `/clock`).
  Both are RTPS. `zenoh` is a different protocol and must match on both sides.
- `fastdds` and `cyclonedds` get an XML profile mounted at `/config`
  (`FASTRTPS_DEFAULT_PROFILES_FILE`, `CYCLONEDDS_URI`) — discovery can fail
  silently without them, which is why the scripts always mount that directory.
- `zenoh` needs a router process (`rmw_zenohd`) reachable by both sides; it is the
  only RMW here with an out-of-band dependency.
- `--net=host` is required: DDS discovery uses multicast/shared memory that a
  bridged container network does not carry.

## Where each failure belongs

| Layer | Owner |
|---|---|
| support not compiled in | [[build-carla-ue4]] `ROS2=1` |
| cooked package lost it | [[package-carla-ue4]] `ROS2=1` |
| server not started with `--ros2`, wrong domain/RMW | [[run-carla-server]] `ROS2=1` |
| actor exists but is silent (`enable_for_ros`) | [[create-sensor]], [[world-data]] `ros-topics` |
| topic on the wire but nothing consumes it | this skill |
