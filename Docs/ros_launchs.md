<h1>CARLA launch</h1>

--------------
##carla_ackermann_control.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — ackermann_msgs.AckermannDrive
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control_parameter_descriptions</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — carla_ackermann_control.EgoVehicleControlInfo ???
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)

---------------
##carla_ros_bridge.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — visualization_msgs.MarkerArray

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — derived_object_msgs.ObjectArrayring
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)


--------------
##carla_ros_bridge_with_ackermann_control.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — visualization_msgs.MarkerArray

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — derived_object_msgs.ObjectArrayring
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ackermann_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/ackermann_cmd</b></font> — ackermann_msgs.AckermannDrive
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)


<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control_parameter_descriptions</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/control_info</b></font> — carla_ackermann_control.EgoVehicleControlInfo ???
* <font color="80ba10"><b>/carla/ego_vehicle/ackermann_control/parameter_updates</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)

--------------
##carla_ros_bridge_with_example_ego_vehicle.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — visualization_msgs.MarkerArray
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — geometry_msgs.Twist
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/camera_info</b></font> — sensor_msgs.CameraInfo
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/image_color</b></font> — sensor_msgs.Image
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/camera_info</b></font> — sensor_msgs.CameraInfo
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — sensor_msgs.Image
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — sensor_msgs.NavSatFix 
* <font color="80ba10"><b>/carla/ego_vehicle/imu</b></font> — sensor_msgs.Imu
* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — sensor_msgs.PointCloud2
* <font color="80ba10"><b>/carla/ego_vehicle/objects</b></font> — derived_object_msgs.ObjectArray
* <font color="80ba10"><b>/carla/ego_vehicle/odometry</b></font> — nav_msgs.Odometry
* <font color="80ba10"><b>/carla/ego_vehicle/radar/front/radar</b></font> — ainstein_radar_msgs.RadarTargetArray
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — derived_object_msgs.ObjectArrayring
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — sensor_msgs.Image
* <font color="80ba10"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — sensor_msgs.NavSatFix 
* <font color="80ba10"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — ???


---------------
##carla_ros_bridge_with_rviz.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/debug_marker</b></font> — visualization_msgs.MarkerArray

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p> 

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/objects</b></font> — derived_object_msgs.ObjectArrayring
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/rviz</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p> 

* <font color="f8815c"><b>/carla/vehicle_marker</b></font> — ???
* <font color="f8815c"><b>/carla/vehicle_marker_array</b></font> — ???
* <font color="f8815c"><b>/carla/ego_vehicle/lidar/front/point_cloud</b></font> — sensor_msgs.PointCloud2


--------------
##carla_manual_control.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — sensor_msgs.Image
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — sensor_msgs.NavSatFix 
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool

--------------
##carla_pcl_recorder.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>carla_ros_bridge</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/debug_marker</b></font> — visualization_msgs.MarkerArray
* <font color="f8815c"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???
* <font color="f8815c"><b>/carla/ego_vehicle/twist</b></font> — geometry_msgs.Twist
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd</b></font> —  [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/actor_list</b></font> — [carla_msgs.CarlaActorList](../ros_msgs#carlaactorlistmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/camera_info</b></font> — sensor_msgs.CameraInfo
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/front/image_color</b></font> — sensor_msgs.Image
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/camera_info</b></font> — sensor_msgs.CameraInfo
* <font color="80ba10"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — sensor_msgs.Image
* <font color="80ba10"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — sensor_msgs.NavSatFix 
* <font color="80ba10"><b>/carla/ego_vehicle/imu</b></font> — sensor_msgs.Imu
* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — sensor_msgs.PointCloud2
* <font color="80ba10"><b>/carla/ego_vehicle/objects</b></font> — derived_object_msgs.ObjectArray
* <font color="80ba10"><b>/carla/ego_vehicle/odometry</b></font> — nav_msgs.Odometry
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/radar/front/radar</b></font> — ainstein_radar_msgs.RadarTargetArray
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="80ba10"><b>/carla/marker</b></font> — visualization_msgs.Marker
* <font color="80ba10"><b>/carla/objects</b></font> — derived_object_msgs.ObjectArrayring
* <font color="80ba10"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
* <font color="80ba10"><b>/carla/traffic_lights</b></font> — [carla_msgs.CarlaTrafficLightStatusList](../ros_msgs#carlatrafficlightstatuslist)
* <font color="80ba10"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_manual_control_ego_vehicle</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/camera/rgb/view/image_color</b></font> — sensor_msgs.Image
* <font color="f8815c"><b>/carla/ego_vehicle/collision</b></font> — [carla_msgs.CarlaCollisionEvent](../ros_msgs#carlacollisioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/gnss/gnss1/fix</b></font> — sensor_msgs.NavSatFix 
* <font color="f8815c"><b>/carla/ego_vehicle/lane_invasion</b></font> — [carla_msgs.CarlaLaneInvasionEvent](../ros_msgs#carlalaneinvasioneventmsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_info</b></font> — [carla_msgs.CarlaEgoVehicleInfo](../ros_msgs#carlaegovehicleinfomsg)
* <font color="f8815c"><b>/carla/ego_vehicle/vehicle_status</b></font> — [carla_msgs.CarlaEgoVehicleStatus](../ros_msgs#carlaegovehiclestatusmsg)
* <font color="f8815c"><b>/carla/status</b></font> — [carla_msgs.CarlaStatus](../ros_msgs#carlastatusmsg)
 

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_cmd_manual</b></font> — [carla_msgs.CarlaEgoVehicleControl](../ros_msgs#carlaegovehiclecontrolmsg)
* <font color="80ba10"><b>/carla/ego_vehicle/vehicle_control_manual_override</b></font> — std_msgs.Bool

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_ego_vehicle_ego_vehicle</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/ego_vehicle/initialpose</b></font> — ???

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/enable_autopilot_rostopic</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-top:-10px;margin-bottom:-5px"> <b>Publishes in:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/enable_autopilot</b></font> — ???


<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/pcl_recorder_node</u> <small><i>(Node)</i></small> </h4>  

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="80ba10"><b>/carla/ego_vehicle/lidar/lidar1/point_cloud</b></font> — sensor_msgs.PointCloud2

--------------
##carla_waypoint_publisher.launch

<!---NODE-->
<h4 style="margin-bottom: 5px"> <u>/carla_waypoint_publisher</u> <small><i>(Node)</i></small> </h4>

<p style="margin-bottom:-5px"> <b>Subscribed to:</b> </p>  

* <font color="f8815c"><b>/carla/world_info</b></font> — [carla_msgs.CarlaWorldInfo](../ros_msgs#carlaworldinfomsg)
