# Native ROS 2 topic reference

This page lists every topic the native ROS 2 interface publishes and
subscribes to when CARLA runs with `--ros2` (middleware selectable with
`--rmw=fastdds|cyclonedds|zenoh`). The interface follows one scope rule:
it exposes the data a real drive-by-wire vehicle would produce and accepts
the control commands such a vehicle would accept. Simulation control
(spawning, weather, synchronous ticking) stays in the Python API.

Standard message types ship with every ROS 2 distribution. The
`carla_msgs/*` types are the ones published from the bridge era in
[ros-carla-msgs](https://github.com/carla-simulator/ros-carla-msgs);
install that package on the ROS side to deserialize them.

Topic names below are the ROS names. On the DDS wire each topic carries
the standard `rt/` prefix (for example `rt/carla/map`), which ROS 2 tools
hide.

## Simulation-wide topics

Published as soon as the simulator starts; no actor opt-in needed.

| Topic | Type | QoS | Cadence |
|-------|------|-----|---------|
| `/clock` | `rosgraph_msgs/Clock` | volatile | every frame |
| `/carla/map` | `std_msgs/String` | transient_local | once per episode |
| `/carla/traffic_lights/info` | `carla_msgs/CarlaTrafficLightInfoList` | transient_local | once per episode |
| `/carla/traffic_lights/status` | `carla_msgs/CarlaTrafficLightStatusList` | transient_local | on state change |
| `/tf_static` | `tf2_msgs/TFMessage` | transient_local | on hero registration |
| `/tf` | `tf2_msgs/TFMessage` | volatile | every frame |

- `/carla/map` carries the full OpenDRIVE description of the current map,
  the same string `carla.Map.to_opendrive()` returns. Latched, so late
  joiners receive it without waiting for a map change.
- `/carla/traffic_lights/info` describes every traffic light once per
  episode: actor id, world pose, and the trigger volume as a center
  relative to that pose plus a full box size, matching what the Python API
  exposes as `actor.trigger_volume`.
- `/carla/traffic_lights/status` carries the state of every light
  (`RED=0, YELLOW=1, GREEN=2, OFF=3, UNKNOWN=4`). The full list is
  republished only when at least one light changed state; the latched
  sample keeps the current state available to late joiners between
  changes.

## Ego vehicle topics

Created when a vehicle with `role_name` `hero` is spawned. `<ros_name>`
is the vehicle's `ros_name` attribute (defaults to `actor<id>` when
unset). Destroying the vehicle removes the topics.

| Topic | Direction | Type | QoS | Cadence |
|-------|-----------|------|-----|---------|
| `/carla/<ros_name>/odometry` | publish | `nav_msgs/Odometry` | volatile | every frame |
| `/carla/<ros_name>/vehicle_status` | publish | `carla_msgs/CarlaEgoVehicleStatus` | volatile | every frame |
| `/carla/<ros_name>/vehicle_info` | publish | `carla_msgs/CarlaEgoVehicleInfo` | transient_local | once at spawn |
| `/carla/<ros_name>/vehicle_control_cmd` | subscribe | `carla_msgs/CarlaEgoVehicleControl` | volatile | consumer-driven |
| `/carla/<ros_name>/ackermann_control_cmd` | subscribe | `ackermann_msgs/AckermannDriveStamped` | volatile | consumer-driven |

- Odometry pose is ground truth in ROS coordinates with
  `frame_id: odom` and `child_frame_id: <ros_frame_id>`; the twist is
  expressed in the vehicle body frame.
- Vehicle status carries speed, acceleration (computed from the velocity
  history), world orientation, and an echo of the last applied control.
- Vehicle info is the latched static description from the vehicle physics
  control: mass, wheels, gearbox, center of mass.

## TF tree

The transform topics compose a REP-105 tree:

```
map -> odom -> <vehicle frame> -> <sensor frames>
```

- `map -> odom`: identity, latched on `/tf_static` when the hero vehicle
  registers. CARLA publishes ground truth, so the `odom` frame never
  drifts from `map`.
- `odom -> <vehicle frame>`: published on `/tf` every frame for the hero
  vehicle.
- `<parent frame> -> <sensor frame>`: published on `/tf` every frame for
  each sensor with ROS 2 enabled, where the parent is the actor the
  sensor is attached to (or `map` for unattached sensors).

A sensor's frame defaults to its `ros_name` and can be overridden with
the `ros_frame_id` attribute; per-actor TF publishing can be disabled
with the `ros_publish_tf` attribute.

## Sensor topics

Sensor publishing is opt-in per actor: call `enable_for_ros()` on the
sensor from the Python API (`ros2_native.py` in `PythonAPI/examples/ros2`
does this for a whole sensor stack). The base name composes the
attachment hierarchy: `/carla/<parent ros_name>/<sensor ros_name>` for an
attached sensor, `/carla/<sensor ros_name>` otherwise. All sensor topics
are volatile and publish at the sensor's own tick rate.

| Sensor | Topics | Types |
|--------|--------|-------|
| RGB camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| Depth camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| Semantic segmentation camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| Instance segmentation camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| Normals camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| Optical flow camera | `<base>/image`, `<base>/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` |
| DVS camera | `<base>/image`, `<base>/camera_info`, `<base>/point_cloud` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo`, `sensor_msgs/PointCloud2` |
| LiDAR (ray cast) | `<base>/point_cloud` | `sensor_msgs/PointCloud2` |
| Semantic LiDAR | `<base>/point_cloud` | `sensor_msgs/PointCloud2` |
| Hybrid solid-state LiDAR | `<base>/point_cloud` | `sensor_msgs/PointCloud2` |
| Radar | `<base>/point_cloud` | `sensor_msgs/PointCloud2` |
| GNSS | `<base>` | `sensor_msgs/NavSatFix` |
| IMU | `<base>` | `sensor_msgs/Imu` |
| Collision | `<base>` | `carla_msgs/CarlaCollisionEvent` |

Lane invasion, obstacle detection and RSS sensors have no native ROS 2
publisher; read them through the Python API.

## QoS summary

Every publisher is reliable with keep-last history. Topics marked
transient_local above are latched with depth 1: a subscriber that joins
late immediately receives the last published sample. Match the
durability when echoing them, for example:

```sh
ros2 topic echo --qos-durability transient_local --qos-reliability reliable /carla/map
```
