# ROS 2 native messages

Every message the [native ROS 2 interface](ros2_native.md) publishes or subscribes to, and what each field means.
Most of them are standard ROS 2 messages, so a stack built on `sensor_msgs` and `geometry_msgs` reads CARLA without a translation layer.

The message types are defined in `LibCarla/source/carla/ros2/types/msg/` and serialized to the DDS type names the ROS 2 tooling expects, so `ros2 topic info -v` reports the same type a native ROS 2 publisher would.

- [__Published__](#published)
- [__Subscribed__](#subscribed)
- [__CARLA messages__](#carla-messages)
- [__Autoware messages__](#autoware-messages)
- [__Quality of service__](#quality-of-service)

!!! Note
    The [ROS bridge](ros_bridge.md) defines a much larger set of `carla_msgs` for the Unreal Engine 4 line — actor lists, world info, traffic light status, scenario control. The native interface does not use them; they are documented in the [bridge's own messages reference](https://carla.readthedocs.io/projects/ros-bridge/en/latest/ros_msgs/).

---
## Published

| Message | Published on | Source |
|---|---|---|
| [`rosgraph_msgs/Clock`](https://docs.ros.org/en/jazzy/p/rosgraph_msgs/interfaces/msg/Clock.html) | `/clock` | The simulation clock, every frame |
| [`tf2_msgs/TFMessage`](https://docs.ros.org/en/jazzy/p/tf2_msgs/interfaces/msg/TFMessage.html) | `/tf` | One transform per actor |
| [`sensor_msgs/Image`](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/Image.html) | `<sensor>/image` | RGB, depth, semantic, instance, normals, optical flow and DVS cameras |
| [`sensor_msgs/CameraInfo`](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/CameraInfo.html) | `<sensor>/camera_info` | The same cameras |
| [`sensor_msgs/PointCloud2`](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/PointCloud2.html) | `<sensor>/point_cloud` | LIDAR, semantic LIDAR, HSS LIDAR, radar, and the DVS camera's events |
| [`sensor_msgs/Imu`](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/Imu.html) | `<sensor>` | `sensor.other.imu` |
| [`sensor_msgs/NavSatFix`](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/NavSatFix.html) | `<sensor>` | `sensor.other.gnss` |
| [`geometry_msgs/PoseStamped`](https://docs.ros.org/en/jazzy/p/geometry_msgs/interfaces/msg/PoseStamped.html) | `<sensor>/pose` | `sensor.other.autoware_gnss` |
| [`geometry_msgs/PoseWithCovarianceStamped`](https://docs.ros.org/en/jazzy/p/geometry_msgs/interfaces/msg/PoseWithCovarianceStamped.html) | `<sensor>/pose_with_covariance` | `sensor.other.autoware_gnss` |
| [`carla_msgs/CarlaCollisionEvent`](#carlacollisionevent) | `<sensor>` | `sensor.other.collision` |
| [`autoware_vehicle_msgs/VelocityReport`](#velocityreport) | `/vehicle/status/velocity_status` | `sensor.other.vehicle_status` |
| [`autoware_vehicle_msgs/SteeringReport`](#steeringreport) | `/vehicle/status/steering_status` | `sensor.other.vehicle_status` |
| [`autoware_vehicle_msgs/ControlModeReport`](#controlmodereport) | `/vehicle/status/control_mode` | `sensor.other.vehicle_status` |
| [`autoware_vehicle_msgs/GearReport`](#gearreport) | `/vehicle/status/gear_status` | `sensor.other.vehicle_status` |
| [`autoware_vehicle_msgs/TurnIndicatorsReport`](#turnindicatorsreport-and-hazardlightsreport) | `/vehicle/status/turn_indicators_status` | `sensor.other.vehicle_status` |
| [`autoware_vehicle_msgs/HazardLightsReport`](#turnindicatorsreport-and-hazardlightsreport) | `/vehicle/status/hazard_lights_status` | `sensor.other.vehicle_status` |

`<sensor>` is the topic built from the sensor's `ros_name` and its parent's, as [ROS 2 sensors](ros2_native_sensors.md) describes.

---
## Subscribed

A vehicle is subscribed to only when its `role_name` is `hero` or `ego`, and the three command surfaces are mutually exclusive: it listens on exactly one of them.
Autoware control wins when `ros2_autoware_control` is set, then Ackermann when `ros2_ackermann_control` is set, and the direct control topic otherwise.

| Message | Subscribed on | Effect |
|---|---|---|
| [`carla_msgs/CarlaEgoVehicleControl`](#carlaegovehiclecontrol) | `<vehicle>/vehicle_control_cmd` | Throttle, steer and brake applied directly |
| [`ackermann_msgs/AckermannDriveStamped`](#ackermanndrivestamped) | `<vehicle>/ackermann_control_cmd` | Target speed and steering angle, converted by the server |
| [`autoware_control_msgs/Control`](#control) | `/control/command/control_cmd` | Drives the vehicle in Autoware mode |
| [`autoware_vehicle_msgs/GearCommand`](#gearcommand) | `/control/command/gear_cmd` | Received, not applied |
| [`autoware_vehicle_msgs/TurnIndicatorsCommand`](#turnindicatorscommand-and-hazardlightscommand) | `/control/command/turn_indicators_cmd` | Received, not applied |
| [`autoware_vehicle_msgs/HazardLightsCommand`](#turnindicatorscommand-and-hazardlightscommand) | `/control/command/hazard_lights_cmd` | Received, not applied |
| [`tier4_vehicle_msgs/VehicleEmergencyStamped`](#vehicleemergencystamped-and-engage) | `/control/command/emergency_cmd` | Received, not applied |
| [`autoware_vehicle_msgs/Engage`](#vehicleemergencystamped-and-engage) | `/vehicle/engage` | Received, not applied |

The Autoware topics are fixed names, so one Autoware vehicle runs per ROS domain. See [Autoware](autoware.md).

---
## CARLA messages

Two messages are CARLA's own. Install them from the [ros-carla-msgs package](https://github.com/carla-simulator/ros-carla-msgs).

### CarlaEgoVehicleControl

The direct control command, equivalent to [carla.VehicleControl](python_api.md#carla.VehicleControl).

| Field | Type | Description |
|---|---|---|
| `header` | [Header](https://docs.ros.org/en/jazzy/p/std_msgs/interfaces/msg/Header.html) | Time stamp and frame id of the command. |
| `throttle` | float32 | Throttle, `[0.0, 1.0]`. |
| `steer` | float32 | Steering, `[-1.0, 1.0]`, positive to the right. |
| `brake` | float32 | Brake, `[0.0, 1.0]`. |
| `hand_brake` | bool | Engages the hand brake. |
| `reverse` | bool | Puts the vehicle in reverse. |
| `gear` | int32 | Gear to select when `manual_gear_shift` is set. |
| `manual_gear_shift` | bool | Shifts with `gear` instead of the automatic gearbox. |

### CarlaCollisionEvent

Published by `sensor.other.collision` on every impact.

| Field | Type | Description |
|---|---|---|
| `header` | [Header](https://docs.ros.org/en/jazzy/p/std_msgs/interfaces/msg/Header.html) | Time stamp and frame id of the event. |
| `other_actor_id` | uint32 | Id of the actor that was hit. |
| `normal_impulse` | [geometry_msgs/Vector3](https://docs.ros.org/en/jazzy/p/geometry_msgs/interfaces/msg/Vector3.html) | Impulse resulting from the collision. |

### AckermannDriveStamped

The standard [ackermann_msgs](https://docs.ros.org/en/api/ackermann_msgs/html/msg/AckermannDriveStamped.html) command: a `header` and a `drive` field whose contents are below. All five fields map onto [carla.VehicleAckermannControl](python_api.md#carla.VehicleAckermannControl), which CARLA's own Ackermann controller turns into throttle, brake and steering.

| Field of `drive` | Type | Description |
|---|---|---|
| `steering_angle` | float32 | Target steering angle, radians. |
| `steering_angle_velocity` | float32 | Rate of change of the steering angle, rad/s. |
| `speed` | float32 | Target speed, m/s. |
| `acceleration` | float32 | Target acceleration, m/s². |
| `jerk` | float32 | Target jerk, m/s³. |

---
## Autoware messages

These come from Autoware's own interface packages. CARLA implements the subset below, which is what the vehicle interface needs; every field is as Autoware defines it.

### Control

`autoware_control_msgs/Control`, the only command that moves the vehicle in Autoware mode.

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time the command was produced. |
| `control_time` | builtin_interfaces/Time | Time the command applies to. |
| `lateral` | Lateral | Steering part of the command. |
| `longitudinal` | Longitudinal | Speed part of the command. |

__`Lateral`__ carries `stamp`, `control_time`, `steering_tire_angle` (float32, radians), `steering_tire_rotation_rate` (float32, rad/s) and `is_defined_steering_tire_rotation_rate` (bool).

__`Longitudinal`__ carries `stamp`, `control_time`, `velocity` (float32, m/s), `acceleration` (float32, m/s²), `jerk` (float32, m/s³), and the `is_defined_acceleration` and `is_defined_jerk` flags.

### VelocityReport

| Field | Type | Description |
|---|---|---|
| `header` | Header | Time stamp and frame id. |
| `longitudinal_velocity` | float32 | Forward speed, m/s. |
| `lateral_velocity` | float32 | Sideways speed, m/s. |
| `heading_rate` | float32 | Yaw rate, rad/s. |

### SteeringReport

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the report. |
| `steering_tire_angle` | float32 | Current steering angle at the tire, radians. |

### ControlModeReport

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the report. |
| `mode` | uint8 | `NO_COMMAND`=0, `AUTONOMOUS`=1, `AUTONOMOUS_STEER_ONLY`=2, `AUTONOMOUS_VELOCITY_ONLY`=3, `MANUAL`=4, `DISENGAGED`=5, `NOT_READY`=6. CARLA always reports `AUTONOMOUS`. |

### GearReport

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the report. |
| `report` | uint8 | `NONE`=0, `NEUTRAL`=1, `DRIVE`=2 through `DRIVE_18`=19, `REVERSE`=20, `REVERSE_2`=21, `PARK`=22, `LOW`=23, `LOW_2`=24. |

### GearCommand

Same constants as `GearReport`, in a `command` field.

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the command. |
| `command` | uint8 | Requested gear. Received, not applied. |

### TurnIndicatorsReport and HazardLightsReport

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the report. |
| `report` | uint8 | Indicators: `DISABLE`=1, `ENABLE_LEFT`=2, `ENABLE_RIGHT`=3. Hazards: `DISABLE`=1, `ENABLE`=2. |

### TurnIndicatorsCommand and HazardLightsCommand

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the command. |
| `command` | uint8 | The same constants as the reports, plus `NO_COMMAND`=0. Received, not applied. |

### VehicleEmergencyStamped and Engage

| Field | Type | Description |
|---|---|---|
| `stamp` | builtin_interfaces/Time | Time of the command. |
| `emergency` / `engage` | bool | Emergency stop request, engage request. Received, not applied. |

---
## Quality of service

Everything is published __reliable__, __volatile__, with a __keep-last__ history of depth 1 — the Fast-DDS writer defaults, and what an Autoware-style stack expects.
A subscriber created with a best-effort policy still matches a reliable publisher; the opposite does not.

The Autoware command subscribers are the one exception: they are __transient local__, so a command published before the vehicle was spawned still reaches it.
