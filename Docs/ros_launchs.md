# Launchfiles reference

---
## carla_ackermann_control.launch

Creates a node to manage a vehicle using Ackermann controls instead of the CARLA control messages. The node reads the vehicle info from CARLA and uses it to define the controller. A simple Python PID is used to adjust acceleration/velocity. This is a Python dependency that can be easily installed with _pip_. 
```sh
pip install --user simple-pid
```
It is possible to modify the parameters in runtime via ROS dynamic reconfigure. Initial parameters can be set using a `settings.yaml`. The path to it depends on the bridge installation.  

* __Deb repository installation__,  
`/opt/carla-ros-bridge/melodic/share/carla_ackermann_control/config/settings.yaml`.  
* __Source repository installation__,  
`/catkin_ws/src/ros-bridge/carla_ackermann_control/config/settings.yaml`.  

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  
Converts [AckermannDrive messages](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.html) to [CarlaEgoVehicleControl.msg](ros_msgs.md#carlaegovehiclecontrolmsg). Speed is in __m/s__, steering angle in __radians__ and refers to driving angle, not wheel angle.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — [ackermann_msgs.AckermannDrive](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_descriptions</b></font> — [dynamic_reconfigure/ConfigDescription](http://docs.ros.org/melodic/api/dynamic_reconfigure/html/msg/ConfigDescription.html)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — [carla_ackermann_control.EgoVehicleControlInfo](ros_msgs.md#egovehiclecontrolinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — [dynamic_reconfigure/Config](http://wiki.ros.org/dynamic_reconfigure)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)

---
## carla_ego_vehicle.launch

Spawns an ego vehicle (`role-name="ego_vehicle"`). The argument `sensor_definition_file` describes the sensors attached to the vehicle. It is the location of a __.json__ file. The format of this file is explained [here](https://github.com/carla-simulator/ros-bridge/tree/master/carla_ego_vehicle).  

To spawn the vehicle at a specific location, publish in `/carla/ego_vehicle/initialpose`, or use __RVIZ__ and select a position with __2D Pose estimate__. 

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Spawns an ego vehicle with sensors attached, and waits for world information.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)
* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

---
## carla_example_ego_vehicle.launch
Based on carla_ego_vehicle.launch, spawns an ego vehicle (`role-name="ego_vehicle"`). The file `sensors.json` describes the sensors attached. The path to it depends on the bridge installation.  

* __Deb repository installation__,  
`/opt/carla-ros-bridge/melodic/share/carla_ego_vehicle/config/sensors.json`.  
* __Source repository installation__,  
`/catkin_ws/src/ros-bridge/carla_ego_vehicle/config/sensors.json`.  

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Spawns an ego vehicle with sensors attached and waits for world information.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)
* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

---
## carla_infrastructure.launch
Spawns infrastructure sensors and requires the argument `infrastructure_sensor_definition_file` with the location of a __.json__ file describing these sensors.

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_infrastructure</u> <small><i>(Node)</i></small> </h4>
Spawns the infrastructure sensors passed as arguments.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

---
## carla_ros_bridge.launch
Creates a node with some basic communications between CARLA and ROS.  

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
Publishes the data regarding the current state of the simulation. Reads the debug shapes being drawn.

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](ros_msgs.md#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](ros_msgs.md#carlatrafficlightstatuslistmsg)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

---
## carla_ros_bridge_with_ackermann_control.launch

Launches two basic nodes. One retrieves simulation data, the other controls a vehicle using [AckermannDrive messages](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.html). 

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
Publishes data regarding the current state of the simulation. Reads the debug shapes being drawn.

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](ros_msgs.md#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](ros_msgs.md#carlatrafficlightstatuslistmsg)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  
Converts [AckermannDrive messages](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.html) to [CarlaEgoVehicleControl.msg](ros_msgs.md#carlaegovehiclemsg). Speed is in __m/s__, steering angle is in __radians__ and refers to driving angle, not wheel angle.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — [ackermann_msgs.AckermannDrive](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.html)  
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_descriptions</b></font> — [dynamic_reconfigure/ConfigDescription](http://docs.ros.org/melodic/api/dynamic_reconfigure/html/msg/ConfigDescription.html)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — [carla_ackermann_control.EgoVehicleControlInfo](ros_msgs.md#egovehiclecontrolinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — [dynamic_reconfigure/Config](http://wiki.ros.org/dynamic_reconfigure)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)

---
## carla_ros_bridge_with_example_ego_vehicle.launch

Spawns an ego vehicle with sensors attached, and starts communications between CARLA and ROS. Both share current simulation state, sensor and ego vehicle data. The ego vehicle is set ready to be used in manual control. 

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
In charge of the communications between CARLA and ROS. They share the current state of the simulation, traffic lights, vehicle controllers and sensor data. 
<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — [geometry_msgs.Twist](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/Twist.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](ros_msgs.md#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html)
* <font color="80ba10"><b>/carla/ego_vehicle/imu</b></font> — [sensor_msgs.Imu](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Imu.html)
* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)
* <font color="80ba10"><b>/carla/ego_vehicle/objects</b></font> — [derived_object_msgs.ObjectArray](http://docs.ros.org/kinetic/api/derived_object_msgs/html/msg/ObjectArray.html)
* <font color="80ba10"><b>/carla/ego_vehicle/odometry</b></font> — [nav_msgs.Odometry](http://docs.ros.org/melodic/api/nav_msgs/html/msg/Odometry.html)
* <font color="80ba10"><b>/carla/ego_vehicle/radar/front/radar</b></font> — [ainstein_radar_msgs.RadarTargetArray](http://wiki.ros.org/ainstein_radar_msgs)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](ros_msgs.md#carlatrafficlightstatuslistmsg)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Retrieves information from CARLA regarding the ego vehicle. Uses keyboard input to publish controller messages for the ego vehicle. The information retrieved includes static data, current state, sensor data, and simulation settings. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](ros_msgs.md#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html) 
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](ros_msgs.md#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  
Spawns an ego vehicle with sensors attached. Reads world information.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)

---
## carla_ros_bridge_with_rviz.launch

Starts communications between CARLA and ROS, and launches RVIZ to retrieve Lidar data. 
<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
Shares information between CARLA and ROS regarding the current simulation state. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](ros_msgs.md#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](ros_msgs.md#carlatrafficlightstatuslistmsg)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/rviz</u> <small><i>(Node)</i></small> </h4>
Runs an instance of RVIZ, and waits for Lidar data. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/vehicle_marker</b></font> — [visualization_msgs/Marker](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/Marker.html)
* <font color="f8815c"><b>/carla/vehicle_marker_array</b></font> — [visualization_msgs/MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/lidar/front/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)


---
## carla_manual_control.launch

A ROS version of the CARLA script `manual_control.py`. It has some prequisites.  

*  __To display an image__, a camera with role-name `view` and resolution 800x600.
*  __To display the position__, a gnss sensor with role-name `gnss1`.
*  __To detect other sensor data__, the corresponding sensor. 


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Retrieves information from CARLA regarding the ego vehicle. Uses keyboard input to publish controller messages for the ego vehicle. The information retrieved includes static data, current state, sensor data, and simulation settings. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](ros_msgs.md#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html) 
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](ros_msgs.md#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

---
## carla_pcl_recorder.launch
Creates a pointcloud map for the current CARLA level. An autopilot ego vehicle roams around the map with a LIDAR sensor attached.  

The point clouds are saved in the `/tmp/pcl_capture` directory. Once the capture is done, the overall size can be reduced.

```sh
#create one point cloud file
pcl_concatenate_points_pcd /tmp/pcl_capture/*.pcd

#filter duplicates
pcl_voxel_grid -leaf 0.1,0.1,0.1 output.pcd map.pcd

#verify the result
pcl_viewer map.pcd
```

The launch file requires some functionality that is not part of the python egg-file. The PYTHONPATH has to be extended. 

```sh
export PYTHONPATH=<path-to-carla>/PythonAPI/carla/dist/carla-<version_and_arch>.egg:<path-to-carla>/PythonAPI/carla/
```

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
In charge of most of the communications between CARLA and ROS. Both share the current state of the simulation, traffic lights, vehicle controllers and sensor data. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — [geometry_msgs.Twist](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/Twist.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](ros_msgs.md#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html)
* <font color="80ba10"><b>/carla/ego_vehicle/imu</b></font> — [sensor_msgs.Imu](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Imu.html)
* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)
* <font color="80ba10"><b>/carla/ego_vehicle/objects</b></font> — [derived_object_msgs.ObjectArray](http://docs.ros.org/kinetic/api/derived_object_msgs/html/msg/ObjectArray.html)
* <font color="80ba10"><b>/carla/ego_vehicle/odometry</b></font> — [nav_msgs.Odometry](http://docs.ros.org/melodic/api/nav_msgs/html/msg/Odometry.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/radar/front/radar</b></font> — [ainstein_radar_msgs.RadarTargetArray](http://wiki.ros.org/ainstein_radar_msgs)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/marker</b></font> — [visualization_msgs.Marker](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/Marker.html)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](ros_msgs.md#carlatrafficlightstatuslistmsg)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Retrieves information from CARLA regarding the ego vehicle. Uses keyboard input to publish controller messages for the ego vehicle. The information retrieved includes static data, current state, sensor data, and simulation settings. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](ros_msgs.md#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html)
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](ros_msgs.md#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](ros_msgs.md#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](ros_msgs.md#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](ros_msgs.md#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](ros_msgs.md#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  
Spawns an ego vehicle with sensors attached. Waits for world information.  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/enable_autopilot_rostopic</u> <small><i>(Node)</i></small> </h4>  
Changes between autopilot and manual control modes. 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/pcl_recorder_node</u> <small><i>(Node)</i></small> </h4>  
Receives the cloud point data. 

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)

---
## carla_waypoint_publisher.launch

Calculates a waypoint route for an ego vehicle. The route is published in `/carla/<ego vehicle name>/waypoints`. The goal is either read from the ROS topic `/carla/<ROLE NAME>/goal`, or a fixed spawnpoint is used. 
The prefered way of setting a goal is to click __2D Nav Goal__ in RVIZ.  

The launch file requires some functionality that is not part of the python egg-file. The PYTHONPATH has to be extended. 
```sh
export PYTHONPATH=$PYTHONPATH:<path-to-carla>/PythonAPI/carla-<carla_version_and_arch>.egg:<path-to-carla>/PythonAPI/carla/
```

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_waypoint_publisher</u> <small><i>(Node)</i></small> </h4>
Uses the current pose of the ego vehicle as starting point. If the vehicle is respawned or moved, the route is calculated again.

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](ros_msgs.md#carlaworldinfomsg)
