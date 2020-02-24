<h1>Supported features</h1>

The ROS bridge enables communication between ROS and CARLA so that both softwares can reach full potential by combining their features. 

  * [__Listed features__](#listed-features)  
  * [__ROS packages__](#ros_packages)  
  * [__ROS topics__](#ros_topics)  

---------------
##Listed features

* __Sensors:__ 
	* Lidar.
	* Cameras (depth, RGB and segmentation).  
* __Messages:__ 
	* Transform publications.
	* Manual control using Ackerman msg.
	* Marker/bounding box messages for cars/pedestirans.
* __Other:__
	* Synchronous mode in CARLA. 
	* Handling ROS dependencies. 
	* Add traffic lights. 


---------------
##ROS packages

<h4> CARLA ego vehicle </h4>

`carla_ego_vehicle` provides a generic way to spawn an ego vehicle and attach sensors to it. You can find further documentation [here](carla_ego_vehicle/README.md).

<h4> CARLA infrastructure sensors </h4>

`carla_infrastructure` provides a generic way to spawn a set of infrastructure sensors defined in a config file. You can find further documentation [here](carla_infrastructure/README.md).

<h4> Waypoint calculation </h4>

To make use of the Carla waypoint calculation a ROS Node is available to get waypoints. You can find further documentation [here](carla_waypoint_publisher/README.md).


---------------
##ROS topics

<h4> Synchronous mode </h4>

It is possible to control the simulation execution:

-   Pause/Play
-   Execute single step

The following topic allows to control the stepping.

| Topic            | Type                                                       |
| ---------------- | ---------------------------------------------------------- |
| `/carla/control` | [carla_msgs.CarlaControl](carla_msgs/msg/CarlaControl.msg) |

A [CARLA Control rqt plugin](rqt_carla_control/README.md) is available to publish to the topic.


<h4> Ego Vehicle </h4>

<h5> Sensors </h5>

The ego vehicle sensors are provided via topics with prefix /carla/ego_vehicle/&lt;sensor_topic>

Currently the following sensors are supported:

Publish GNSS location, collision events... 

| Sensor            | Topic                                                          | Type                                                                                   |
|                   | -------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| __Camera__        | `/carla/<ROLE NAME>/camera/rgb/<SENSOR ROLE NAME>/image_color` | [sensor_msgs.Image](http://docs.ros.org/api/sensor_msgs/html/msg/Image.html)           |
| __Camera__        | `/carla/<ROLE NAME>/camera/rgb/<SENSOR ROLE NAME>/camera_info` | [sensor_msgs.CameraInfo](http://docs.ros.org/api/sensor_msgs/html/msg/CameraInfo.html) |
| __Lidar__         | `/carla/<ROLE NAME>/lidar/<SENSOR ROLE NAME>/point_cloud`      | [sensor_msgs.PointCloud2](http://docs.ros.org/api/sensor_msgs/html/msg/PointCloud2.html) |
| __Radar__         | `/carla/<ROLE NAME>/radar/<SENSOR ROLE NAME>/radar`            | [ainstein_radar_msgs.RadarTargetArray](https://github.com/AinsteinAI/ainstein_radar/blob/master/ainstein_radar_msgs/msg/RadarTargetArray.msg) |
| __IMU__           | `/carla/<ROLE NAME>/imu`                                       | [sensor_msgs.Imu](https://docs.ros.org/melodic/api/sensor_msgs/html/msg/Imu.html) |
| __GNSS__          | `/carla/<ROLE NAME>/gnss/<SENSOR ROLE NAME>/fix`               | [sensor_msgs.NavSatFix](http://docs.ros.org/api/sensor_msgs/html/msg/NavSatFix.html) |
| __Collision__     | `/carla/<ROLE NAME>/collision`                                 | [carla_msgs.CarlaCollisionEvent](carla_msgs/msg/CarlaCollisionEvent.msg) |
|__Lane invasion__  | `/carla/<ROLE NAME>/lane_invasion`                             | [carla_msgs.CarlaLaneInvasionEvent](carla_msgs/msg/CarlaLaneInvasionEvent.msg) |

!!! Note
    Radar data can be visualized on rviz using [ainstein_radar_rviz_plugins](https://wiki.ros.org/ainstein_radar_rviz_plugins). 


<h5> Object sensor </h5>

| Topic                        | Type                                                                                                     | Description                                      |
| ---------------------------- | -------------------------------------------------------------------------------------------------------- | ------------------------------------------------ |
| `/carla/<ROLE NAME>/objects` | [derived_object_msgs.ObjectArray](http://docs.ros.org/api/derived_object_msgs/html/msg/ObjectArray.html) | all vehicles and walkers, except the ego vehicle |

<h5> Control </h5>

| Topic                                                             | Type                                                                           |
| ----------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| `/carla/<ROLE NAME>/vehicle_control_cmd` (subscriber)             | [carla_msgs.CarlaEgoVehicleControl](carla_msgs/msg/CarlaEgoVehicleControl.msg) |
| `/carla/<ROLE NAME>/vehicle_control_cmd_manual` (subscriber)      | [carla_msgs.CarlaEgoVehicleControl](carla_msgs/msg/CarlaEgoVehicleControl.msg) |
| `/carla/<ROLE NAME>/vehicle_control_manual_override` (subscriber) | [std_msgs.Bool](http://docs.ros.org/api/std_msgs/html/msg/Bool.html)           |
| `/carla/<ROLE NAME>/vehicle_status`                               | [carla_msgs.CarlaEgoVehicleStatus](carla_msgs/msg/CarlaEgoVehicleStatus.msg)   |
| `/carla/<ROLE NAME>/vehicle_info`                                 | [carla_msgs.CarlaEgoVehicleInfo](carla_msgs/msg/CarlaEgoVehicleInfo.msg)       |

There are two modes to control the vehicle.

1.  Normal Mode (reading commands from `/carla/<ROLE NAME>/vehicle_control_cmd`)
2.  Manual Mode (reading commands from `/carla/<ROLE NAME>/vehicle_control_cmd_manual`)

This allows to manually override a Vehicle Control Commands published by a software stack. You can toggle between the two modes by publishing to `/carla/<ROLE NAME>/vehicle_control_manual_override`.

[carla_manual_control](carla_manual_control/) makes use of this feature.

For testing purposes, you can stear the ego vehicle from the commandline by publishing to the topic `/carla/<ROLE NAME>/vehicle_control_cmd`.

Examples for a ego vehicle with role_name 'ego_vehicle':

Max forward throttle:

     rostopic pub /carla/ego_vehicle/vehicle_control_cmd carla_msgs/CarlaEgoVehicleControl "{throttle: 1.0, steer: 0.0}" -r 10

Max forward throttle with max steering to the right:

     rostopic pub /carla/ego_vehicle/vehicle_control_cmd carla_msgs/CarlaEgoVehicleControl "{throttle: 1.0, steer: 1.0}" -r 10

The current status of the vehicle can be received via topic `/carla/<ROLE NAME>/vehicle_status`.
Static information about the vehicle can be received via `/carla/<ROLE NAME>/vehicle_info`

<h6> Sensors </h6>

| Topic                                       | Type                                                                             |
| ------------------------------------------- | -------------------------------------------------------------------------------- |
| `/carla/<ROLE NAME>/twist_cmd` (subscriber) | [geometry_msgs.Twist](http://docs.ros.org/api/geometry_msgs/html/msg/Twist.html) |

CAUTION: This control method does not respect the vehicle constraints. It allows movements impossible in the real world, like flying or rotating.

You can also control the vehicle via publishing linear and angular velocity within a Twist datatype.

Currently this method applies the complete linear vector, but only the yaw from angular vector.

<h6> Ackermann control </h6>

In certain cases, the [Carla Control Command](carla_msgs/msg/CarlaEgoVehicleControl.msg) is not ideal to connect to an AD stack.
Therefore a ROS-based node `carla_ackermann_control` is provided which reads [AckermannDrive](http://docs.ros.org/api/ackermann_msgs/html/msg/AckermannDrive.html) messages.
You can find further documentation [here](carla_ackermann_control/README.md).

<h4> Other actors </h4>

| Topic               | Type                                                                                                     | Description                           |
| ------------------- | -------------------------------------------------------------------------------------------------------- | ------------------------------------- |
| `/carla/objects`    | [derived_object_msgs.ObjectArray](http://docs.ros.org/api/derived_object_msgs/html/msg/ObjectArray.html) | all vehicles and walkers              |
| `/carla/marker`     | [visualization_msgs.Marker](http://docs.ros.org/api/visualization_msgs/html/msg/Marker.html)             | visualization of vehicles and walkers |
| `/carla/actor_list` | [carla_msgs.CarlaActorList](carla_msgs/msg/CarlaActorList.msg)                                           | list of all carla actors              |
| `/carla/traffic_lights` | [carla_msgs.CarlaTrafficLightStatusList](carla_msgs/msg/CarlaTrafficLightStatusList.msg)             | list of all traffic lights with their status |

<h4> Status of CARLA </h4>

| Topic               | Type                                                           | Description                                            |
| ------------------- | -------------------------------------------------------------- | ------------------------------------------------------ |
| `/carla/status`     | [carla_msgs.CarlaStatus](carla_msgs/msg/CarlaStatus.msg)       |                                                        |
| `/carla/world_info` | [carla_msgs.CarlaWorldInfo](carla_msgs/msg/CarlaWorldInfo.msg) | Info about the CARLA world/level (e.g. OPEN Drive map) |

<h4> Walkers </h4>

| Topic                                                | Type                                                                         | Description        |
| ---------------------------------------------------- | ---------------------------------------------------------------------------- | ------------------ |
| `/carla/walker/<ID>/walker_control_cmd` (subscriber) | [carla_msgs.CarlaWalkerControl](carla_msgs/msg/CarlaWalkerControl.msg)       | Control a walker   |
| `/carla/walker/<ID>/odometry`                        | [nav_msgs.Odometry](http://docs.ros.org/api/nav_msgs/html/msg/Odometry.html) | odometry of walker |

<h4> Other vehicles </h4>

| Topic                          | Type                                                                         | Description         |
| ------------------------------ | ---------------------------------------------------------------------------- | ------------------- |
| `/carla/vehicle/<ID>/odometry` | [nav_msgs.Odometry](http://docs.ros.org/api/nav_msgs/html/msg/Odometry.html) | odometry of vehicle |

<h4> Debug marker </h4>

It is possible to draw markers in CARLA.

Caution: Markers might affect the data published by sensors.

The following markers are supported in 'map'-frame:

-   Arrow (specified by two points)
-   Points
-   Cube
-   Line Strip

| Topic                              | Type                                                                                                   | Description                 |
| ---------------------------------- | ------------------------------------------------------------------------------------------------------ | --------------------------- |
| `/carla/debug_marker` (subscriber) | [visualization_msgs.MarkerArray](http://docs.ros.org/api/visualization_msgs/html/msg/MarkerArray.html) | draw markers in CARLA world |
