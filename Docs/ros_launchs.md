<h1>CARLA launch</h1>

--------------
##carla_ackermann_control.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — [ackermann_msgs.AckermannDrive](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.htm)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_descriptions</b></font> — [dynamic_reconfigure/ConfigDescription](http://docs.ros.org/melodic/api/dynamic_reconfigure/html/msg/ConfigDescription.html)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — [carla_ackermann_control.EgoVehicleControlInfo](../ros_msgs#egovehiclecontrolinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — [dynamic_reconfigure/Config](http://wiki.ros.org/dynamic_reconfigure)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)

---------------
##carla_ros_bridge.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>
Publishes the data regarding the current state of the simulation: world, objects, trafic lights, actors... 
Receives the debug shapes being drawn, which include: arrows, points, cubes and line strips.

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)


--------------
##carla_ros_bridge_with_ackermann_control.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — [ackermann_msgs.AckermannDrive](http://docs.ros.org/jade/api/ackermann_msgs/html/msg/AckermannDrive.htm)  
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_descriptions</b></font> — [dynamic_reconfigure/ConfigDescription](http://docs.ros.org/melodic/api/dynamic_reconfigure/html/msg/ConfigDescription.html)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — [carla_ackermann_control.EgoVehicleControlInfo](../ros_msgs#egovehiclecontrolinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — [dynamic_reconfigure/Config](http://wiki.ros.org/dynamic_reconfigure)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)

--------------
##carla_ros_bridge_with_example_ego_vehicle.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — [geometry_msgs.Twist](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/Twist.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
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
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html) 
* <font color="80ba10"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)


---------------
##carla_ros_bridge_with_rviz.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/rviz</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/vehicle_marker</b></font> — [visualization_msgs/Marker](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/Marker.html)
* <font color="f8815c"><b>/carla/vehicle_marker_array</b></font> — [visualization_msgs/MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/lidar/front/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)


--------------
##carla_manual_control.launch

A ROS version of the CARLA script `manual_control.py` that receives and manages the information using ROS topics. It has some prerequisites:  

* To display an image: a camera with role-name 'view' and resolution 800x600
* To display the position: a gnss sensor with role-name 'gnss1'
* To detect other sensor data: the corresponding sensor. 


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>
Retrieves from CARLA information regarding the ego vehicle. Uses keyboard input to publish messages containing the controller data to manage the ego vehicle.  
The information retrieved includes both static and current state, the sensor data registered on every step and the general settings of the simulation.  


<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html) 
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

--------------
##carla_pcl_recorder.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — [visualization_msgs.MarkerArray](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/MarkerArray.html)
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — [geometry_msgs.Twist](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/Twist.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/camera_info</b></font> — [sensor_msgs.CameraInfo](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/CameraInfo.html)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html)
* <font color="80ba10"><b>/carla/ego_vehicle/imu</b></font> — [sensor_msgs.Imu](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Imu.html)
* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)
* <font color="80ba10"><b>/carla/ego_vehicle/objects</b></font> — [derived_object_msgs.ObjectArray](http://docs.ros.org/kinetic/api/derived_object_msgs/html/msg/ObjectArray.html)
* <font color="80ba10"><b>/carla/ego_vehicle/odometry</b></font> — [nav_msgs.Odometry](http://docs.ros.org/melodic/api/nav_msgs/html/msg/Odometry.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/radar/front/radar</b></font> — [ainstein_radar_msgs.RadarTargetArray](http://wiki.ros.org/ainstein_radar_msgs)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/marker</b></font> — [visualization_msgs.Marker](http://docs.ros.org/melodic/api/visualization_msgs/html/msg/Marker.html)
* <font color="80ba10"><b>/carla/objects</b></font> — [derived_object_msgs.ObjectArrayring](http://wiki.ros.org/derived_object_msgs)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — [sensor_msgs.Image](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html)
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — [sensor_msgs.NavSatFix](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/NavSatFix.html)
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — [geometry_msgs/PoseWithCovarianceStamped](http://docs.ros.org/melodic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/enable_autopilot_rostopic</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — [std_msgs.Bool](http://docs.ros.org/melodic/api/std_msgs/html/msg/Bool.html)


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/pcl_recorder_node</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — [sensor_msgs.PointCloud2](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/PointCloud2.html)

--------------
##carla_waypoint_publisher.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_waypoint_publisher</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)
