# ROS 2 native sensors

What each CARLA sensor publishes over the [native ROS 2 interface](ros2_native.md), and on which topic.

- [__Topic names__](#topic-names)
- [__Published sensors__](#published-sensors)
- [__Quality of service__](#quality-of-service)

---
## Topic names

A sensor's topic is built from the `ros_name` of its parent chain and its own `ros_name`, with a per-sensor suffix:

```
/carla/<parent ros_name>/<sensor ros_name><suffix>
```

A sensor with no parent drops the parent segment, and one whose `ros_name` is not set falls back to its type and stream id, such as `rgb123`.

`ros_topic_name` replaces the whole `/carla/<parent>/<sensor>` part, and the suffix is then appended under it: a camera with `ros_topic_name` set to `/sensing/camera/front` publishes on `/sensing/camera/front/image` and `/sensing/camera/front/camera_info`.
The single-topic point-cloud sensors are the exception — LIDAR, semantic LIDAR, HSS LIDAR and radar publish on the overridden name verbatim, with no `/point_cloud` suffix.

Two topics are global rather than per sensor:

| Topic | Type | Notes |
| --- | --- | --- |
| `/clock` | `rosgraph_msgs/Clock` | The simulation clock, updated every frame. |
| `/tf` | `tf2_msgs/TFMessage` | One transform per actor. Turn it off with `world.set_publish_tf(False)`. |

---
## Published sensors

| Blueprint | Topic suffix | Message |
| --- | --- | --- |
| `sensor.camera.rgb` | `/image`, `/camera_info` | [sensor_msgs/Image](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/Image.html), [sensor_msgs/CameraInfo](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/CameraInfo.html) |
| `sensor.camera.depth` | `/image`, `/camera_info` | sensor_msgs/Image, sensor_msgs/CameraInfo |
| `sensor.camera.semantic_segmentation` | `/image`, `/camera_info` | sensor_msgs/Image, sensor_msgs/CameraInfo |
| `sensor.camera.instance_segmentation` | `/image`, `/camera_info` | sensor_msgs/Image, sensor_msgs/CameraInfo |
| `sensor.camera.normals` | `/image`, `/camera_info` | sensor_msgs/Image, sensor_msgs/CameraInfo |
| `sensor.camera.optical_flow` | `/image`, `/camera_info` | sensor_msgs/Image, sensor_msgs/CameraInfo |
| `sensor.camera.dvs` | `/image`, `/camera_info`, `/point_cloud` | sensor_msgs/Image, sensor_msgs/CameraInfo, [sensor_msgs/PointCloud2](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/PointCloud2.html) for the raw events |
| `sensor.lidar.ray_cast` | `/point_cloud` | sensor_msgs/PointCloud2 |
| `sensor.lidar.ray_cast_semantic` | `/point_cloud` | sensor_msgs/PointCloud2 |
| `sensor.lidar.hss_lidar` | `/point_cloud` | sensor_msgs/PointCloud2 |
| `sensor.other.radar` | `/point_cloud` | sensor_msgs/PointCloud2 |
| `sensor.other.imu` | — | [sensor_msgs/Imu](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/Imu.html) |
| `sensor.other.gnss` | — | [sensor_msgs/NavSatFix](https://docs.ros.org/en/jazzy/p/sensor_msgs/interfaces/msg/NavSatFix.html) |
| `sensor.other.collision` | — | `carla_msgs/CarlaCollisionEvent` |
| `sensor.other.autoware_gnss` | `/pose`, `/pose_with_covariance` | [geometry_msgs/PoseStamped](https://docs.ros.org/en/jazzy/p/geometry_msgs/interfaces/msg/PoseStamped.html), geometry_msgs/PoseWithCovarianceStamped |
| `sensor.other.vehicle_status` | the six `/vehicle/status/*` topics | The Autoware vehicle report messages. Only publishes when its parent vehicle was spawned with `ros2_autoware_control`; otherwise it logs a warning and sends nothing. |

The sensors that have no publisher — `sensor.other.lane_invasion`, `sensor.other.obstacle`, `sensor.other.v2x`, `sensor.other.v2x_custom`, `sensor.other.rss`, and the ray-traced lens and fisheye cameras — are read from the Python API instead.

---
## Quality of service

Sensor topics are published as __reliable__ and __volatile__, with a __keep-last__ history of depth 1, which is what an Autoware-style stack expects. A subscriber created with a best-effort policy still matches a reliable publisher; the opposite does not.

The message each sensor publishes is defined in [ROS 2 messages](ros2_native_msgs.md).
