# ROS Bridge Sensors

---

The server publishes the simulation clock on `/clock`, updated every frame, and the transform tree on `/tf`, which carries one transform per actor and can be switched off with `world.set_publish_tf(False)`.

In the topics below, `<PARENT ROLE NAME>` is the role name of the actor the sensor is attached to, and `<SENSOR ROLE NAME>` is the sensor's `ros_name`. A sensor with `ros_topic_name` set publishes on that name instead.

## Available Sensors

###### RGB camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](https://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Depth camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](https://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Semantic segmentation camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](http://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Instance segmentation camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](http://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Normals camera 

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](http://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Optical flow camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](http://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### DVS camera

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/events` | [sensor_msgs/PointCloud2](https://docs.ros.org/en/api/sensor_msgs/html/msg/PointCloud2.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/image` | [sensor_msgs/Image](https://docs.ros.org/en/api/sensor_msgs/html/msg/Image.html) |
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs/CameraInfo](https://docs.ros.org/en/api/sensor_msgs/html/msg/CameraInfo.html) |

###### Lidar

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [sensor_msgs/PointCloud2](https://docs.ros.org/en/api/sensor_msgs/html/msg/PointCloud2.html) |

###### Semantic lidar

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [sensor_msgs/PointCloud2](https://docs.ros.org/en/api/sensor_msgs/html/msg/PointCloud2.html) |

###### Radar

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [sensor_msgs/PointCloud2](https://docs.ros.org/en/api/sensor_msgs/html/msg/PointCloud2.html) |

###### IMU

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [sensor_msgs/Imu](https://docs.ros.org/en/api/sensor_msgs/html/msg/Imu.html) |

###### GNSS

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [sensor_msgs/NavSatFix](https://docs.ros.org/en/api/sensor_msgs/html/msg/NavSatFix.html) |


###### Collision sensor

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [carla_msgs/CarlaCollisionEvent](ros2_native_msgs.md#carlacollisioneventmsg) |

###### Lane Invasion Sensor

| Topic | Type |
|-------|------|
| `/carla/[<PARENT ROLE NAME>]/<SENSOR ROLE NAME>` | [carla_msgs/CarlaLaneInvasionEvent](https://github.com/carla-simulator/ros-carla-msgs/blob/master/msg/CarlaLaneInvasionEvent.msg) |