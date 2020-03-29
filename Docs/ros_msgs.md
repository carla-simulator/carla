# CARLA messages reference

The following reference lists all the CARLA messages available in the ROS bridge.  

Any doubts regarding these messages or the CARLA-ROS bridge can be solved in the forum.

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/c/carla-ecosystem/ros-bridge" target="_blank" class="btn btn-neutral" title="Go to the CARLA Forum, ROS bridge section">
CARLA forum</a>
</p>
</div>

---
## CarlaActorInfo.msg

Information shared between ROS and CARLA regarding an actor. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>id</code> </td>
<td>uint32</td>
<td>The ID of the actor.</td>
<tr>
<td><code>parent_id</code> </td>
<td>uint32</td>
<td>The ID of the parent actor. `0` if no parent available.</td>
<tr>
<td><code>type</code> </td>
<td>string</td>
<td>The identifier of the blueprint this actor was based on.</td>
<tr>
<td><code>rolename</code> </td>
<td>string</td>
<td>Role assigned to the actor when spawned.</td>
</tbody>
</table>


---
## CarlaActorList.msg

A list of messages with some basic information for CARLA actors.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>actors</code> </td>
<td><a href="#carlaactorinfomsg">CarlaActorInfo</a></td>
<td>List of messages with actors' information.</td>
</tobody>
</table>

---
## CarlaCollisionEvent.msg

Data retrieved on a collision event detected by the collision sensor of an actor.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>header</code> </td>
<td><a href="http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html">Header</a></td>
<td>Time stamp and frame ID when the message is published.</td>
<tr>
<td><code>other_actor_id</code> </td>
<td>uint32</td>
<td>ID of the actor against whom the collision was detected.</td>
<tr>
<td><code>normal_impulse</code> </td>
<td>geometry_msgs/Vector3</td>
<td>Vector representing resulting impulse from the collision.</td>
</tbody>
</table>


---
## CarlaControl.msg

These messages control the simulation while in synchronous mode. The constant defined is translated as stepping commands.  

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>command</code> </td>
<td>int8</td>
<td><b>PLAY</b>=0 <br> <b>PAUSE</b>=1 <br> <b>STEP_ONCE</b>=2</td>
</tbody>
</table>
<br>

!!! Important
    In synchronous mode, only the ROS bridge client is allowed to tick.

---
## CarlaEgoVehicleControl.msg

Messages sent to apply a control to a vehicle in both modes, autopilot and manual. These are published in a stack. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>header</code> </td>
<td><a href="http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html">Header</a></td>
<td>Time stamp and frame ID when the message is published.</td>
<tr>
<td><code>throttle</code> </td>
<td>float32</td>
<td>Scalar value to cotrol the vehicle throttle: <b>[0.0, 1.0]</b></td>
<tr>
<td><code>steer</code> </td>
<td>float32</td>
<td>Scalar value to control the vehicle steering direction: <b>[-1.0, 1.0]</b> to control the vehicle steering</td>
<tr>
<td><code>brake</code> </td>
<td>float32</td>
<td>Scalar value to control the vehicle brakes: <b>[0.0, 1.0]</b></td>
<tr>
<td><code>hand_brake</code> </td>
<td>bool</td>
<td>If <b>True</b>, the hand brake is enabled.</td>
<tr>
<td><code>reverse</code> </td>
<td>bool</td>
<td>If <b>True</b>, the vehicle will move reverse.</td>
<tr>
<td><code>gear</code> </td>
<td>int32</td>
<td>Changes between the available gears in a vehicle.</td>
<tr>
<td><code>manual_gear_shift</code> </td>
<td>bool</td>
<td>If <b>True</b>, the gears will be shifted using <code>gear</code>.</td>
</tbody>
</table>
<br>

---
## CarlaEgoVehicleInfo.msg

Static information regarding a vehicle, mostly the attributes used to define the vehicle's physics.  

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>id</code> </td>
<td>uint32</td>
<td>ID of the vehicle actor.</td>
<tr>
<td><code>type</code> </td>
<td>string</td>
<td>The identifier of the blueprint this vehicle was based on.</td>
<tr>
<td><code>type</code> </td>
<td>string</td>
<td>The identifier of the blueprint this vehicle was based on.</td>
<tr>
<td><code>rolename</code> </td>
<td>string</td>
<td>Role assigned to the vehicle.</td>
<tr>
<td><code>wheels</code> </td>
<td><a href="#carlaegovehicleinfowheelmsg">CarlaEgoVehicleInfoWheel</a></td>
<td>List of messages with information regarding wheels.</td>
<tr>
<td><code>max_rpm</code> </td>
<td>float32</td>
<td>Maximum RPM of the vehicle's engine.</td>
<tr>
<td><code>moi</code> </td>
<td>float32</td>
<td>Moment of inertia of the vehicle's engine.</td>
<tr>
<td><code>damping_rate_full_throttle</code> </td>
<td>float32</td>
<td>Damping rate when the throttle is at maximum.</td>
<tr>
<td><code>damping_rate_zero_throttle</code><br><code>_clutch_engaged</code> </td>
<td>float32</td>
<td>Damping rate when the throttle is zero with clutch engaged.</td>
<tr>
<td><code>damping_rate_zero_throttle</code><br><code>_clutch_disengaged</code> </td>
<td>float32</td>
<td>Damping rate when the throttle is zero with clutch disengaged.</td>
<tr>
<td><code>use_gear_autobox</code> </td>
<td>bool</td>
<td>If <b>True</b>, the vehicle will have an automatic transmission.</td>
<tr>
<td><code>gear_switch_time</code> </td>
<td>float32</td>
<td>Switching time between gears.</td>
<tr>
<td><code>clutch_strength</code> </td>
<td>float32</td>
<td>The clutch strength of the vehicle. Measured in <b>Kgm^2/s</b>.</td>
<tr>
<td><code>mass</code> </td>
<td>float32</td>
<td>The mass of the vehicle measured in Kg.</td>
<tr>
<td><code>drag_coefficient</code> </td>
<td>float32</td>
<td>Drag coefficient of the vehicle's chassis.</td>
<tr>
<td><code>center_of_mass</code> </td>
<td>geometry_msgs/Vector3</td>
<td>The center of mass of the vehicle.</td>
</tbody>
</table>
<br>


---
## CarlaEgoVehicleInfoWheel.msg

Static information regarding a wheel that will be part of a [CarlaEgoVehicleInfo.msg](#carlaegovehicleinfomsg) message.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>tire_friction</code> </td>
<td>float32</td>
<td>A scalar value that indicates the friction of the wheel.</td>
<tr>
<td><code>damping_rate</code> </td>
<td>float32</td>
<td>The damping rate of the wheel.</td>
<tr>
<td><code>max_steer_angle</code> </td>
<td>float32</td>
<td>The maximum angle in degrees that the wheel can steer.</td>
<tr>
<td><code>radius</code> </td>
<td>float32</td>
<td>The radius of the wheel in centimeters.</td>
<tr>
<td><code>max_brake_torque</code> </td>
<td>float32</td>
<td>The maximum brake torque in Nm.</td>
<tr>
<td><code>max_handbrake_torque</code> </td>
<td>float32</td>
<td>The maximum handbrake torque in Nm.</td>
<tr>
<td><code>position</code> </td>
<td>geometry_msgs/Vector3</td>
<td>World position of the wheel.</td>
</tbody>
</table>
<br>

---
## CarlaEgoVehicleStatus.msg

Current status of the vehicle as an object in the world.  

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>header</code> </td>
<td><a href="http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html">Header</a></td>
<td>Time stamp and frame ID when the message is published.</td>
<tr>
<td><code>velocity</code> </td>
<td>float32</td>
<td>Current speed of the vehicle.</td>
<tr>
<td><code>acceleration</code> </td>
<td>geometry_msgs/Accel</td>
<td>Current acceleration of the vehicle.</td>
<tr>
<td><code>orientation</code> </td>
<td>geometry_msgs/Quaternion</td>
<td>Current orientation of the vehicle.</td>
<tr>
<td><code>control</code> </td>
<td><a href="#carlaegovehiclecontrolmsg">CarlaEgoVehicleControl</a></td>
<td>Current control values as reported by CARLA.</td>
</tbody>
</table>
<br>

---
## CarlaLaneInvasionEvent.msg

These messages publish lane invasions detected by a lane-invasion sensor attached to a vehicle. The invasions detected in the last step are passed as a list with a constant definition to identify the lane crossed.  

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>header</code> </td>
<td><a href="http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html">header</a></td>
<td>Time stamp and frame ID when the message is published.</td>
<tr>
<td><code>crossed_lane_markings</code> </td>
<td>int32[]</td>
<td><b>LANE_MARKING_OTHER</b>=0 <br> <b>LANE_MARKING_BROKEN</b>=1 <br> <b>LANE_MARKING_SOLID</b>=2</td>
</tbody>
</table>
<br>


---
## CarlaScenario.msg

Details for a test scenario. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>name</code> </td>
<td>string</td>
<td>Name of the scenario.</td>
<tr>
<td><code>scenario_file</code> </td>
<td>string</td>
<td>Test file for the scenario.</td>
<tr>
<td><code>destination</code> </td>
<td>geometry_msgs/Pose</td>
<td>Goal location of the scenario.</td>
<tr>
<td><code>target_speed</code> </td>
<td>float64</td>
<td>Desired speed during the scenario.</td>
</tbody>
</table>
<br>

---
## CarlaScenarioList.msg

List of test scenarios to run in ScenarioRunner. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>scenarios</code> </td>
<td><a href="#carlascenariomsg">CarlaScenario[]</a></td>
<td>List of scenarios.</td>
</tbody>
</table>
<br>

---
## CarlaScenarioRunnerStatus.msg

Current state of the ScenarioRunner. It is managed using a constant. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>status</code> </td>
<td>uint8</td>
<td>Current state of the scenario as an enum: <br> <b>STOPPED</b>=0 <br> <b>STARTING</b>=1 <br> <b>RUNNING</b>=2 <br> <b>SHUTTINGDOWN</b>=3 <br> <b>ERROR</b>=4</td>
</tbody>
</table>
<br>

---
## CarlaStatus.msg

Current world settings of the simulation. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>frame</code> </td>
<td>uint64</td>
<td>Current frame number.</td>
<tr>
<td><code>fixed_delta_seconds</code> </td>
<td>float32</td>
<td>Simulation time between last and current step.</td>
<tr>
<td><code>synchronous_mode</code> </td>
<td>bool</td>
<td>If <b>True</b>, synchronous mode is enabled.</td>
<tr>
<td><code>synchronous_mode_running</code> </td>
<td>bool</td>
<td><b>True</b> when the simulation is running. <b>False</b> when it is paused.</td>
</tbody>
</table>
<br>

---
## CarlaTrafficLightStatus.msg

Constant definition regarding the state of a traffic light. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>id</code> </td>
<td>uint32</td>
<td>ID of the traffic light actor.</td>
<tr>
<td><code>state</code> </td>
<td>uint8</td>
<td><b>RED</b>=0 <br> <b>YELLOW</b>=1 <br> <b>GREEN</b>=2 <br> <b>OFF</b>=3 <br> <b>UNKNOWN</b>=4</td>
</tbody>
</table>
<br>

---
## CarlaTrafficLightStatusList.msg

List of traffic lights with their status. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>scenarios</code> </td>
<td><a href="#carlatrafficlightstatusmsg">CarlaTrafficLightStatus[]</a></td>
<td>A list of messages summarizing traffic light states.</td>
</tbody>
</table>
<br>

---
## CarlaWalkerControl.msg

Information needed to apply a movement controller to a walker. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>direction</code> </td>
<td>geometry_msgs/Vector3</td>
<td>Vector that controls the direction of the walker.</td>
<tr>
<td><code>speed</code> </td>
<td>float32</td>
<td>A scalar value to control the walker's speed.</td>
<tr>
<td><code>jump</code> </td>
<td>bool</td>
<td>If <b>True</b>, the walker will jump.</td>
</tbody>
</table>
<br>

---
## CarlaWaypoint.msg

Data contained in a waypoint object. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>road_id</code> </td>
<td>int32</td>
<td>OpenDRIVE road's id.</td>
<tr>
<td><code>section_id</code> </td>
<td>int32</td>
<td>OpenDRIVE section's id, based on the order that they are originally defined.</td>
<tr>
<td><code>lane_id</code> </td>
<td>int32</td>
<td>OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road.</td>
<tr>
<td><code>is_junction</code> </td>
<td>bool</td>
<td><b>True</b>, if the current Waypoint is on a junction as defined by OpenDRIVE.</td>
<tr>
<td><code>is_junction</code> </td>
<td><a href="http://docs.ros.org/api/geometry_msgs/html/msg/Pose.html">geometry_msgs/Pose</a></td>
<td><b>True</b> when the simulation is running. <b>False</b> when it is paused.</td>
</tbody>
</table>
<br>

---
## CarlaWorldInfo.msg

Information about the current CARLA map. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>map_name</code> </td>
<td>string</td>
<td>Name of the CARLA map loaded in the current world.</td>
<tr>
<td><code>opendrive</code> </td>
<td>string</td>
<td>.xodr OpenDRIVE file of the current map as a string.</td>
</tbody>
</table>
<br>

---
## EgoVehicleControlCurrent.msg

Current time, speed and acceleration values of the vehicle. Used by the controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>time_sec</code> </td>
<td>float32</td>
<td>Current time when the controller is applied.</td>
<tr>
<td><code>speed</code> </td>
<td>float32</td>
<td>Current speed applied by the controller.</td>
<tr>
<td><code>speed_abs</code> </td>
<td>float32</td>
<td>Speed as an absolute value.</td>
<tr>
<td><code>accel</code> </td>
<td>float32</td>
<td>Current acceleration applied by the controller.</td>
</tbody>
</table>
<br>

---
## EgoVehicleControlInfo.msg

Current values within an Ackermann controller. These messages are useful for debugging. 

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>header</code> </td>
<td><a href="http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html">header</a></td>
<td>Time stamp and frame ID when the message is published.</td>
<tr>
<td><code>restrictions</code> </td>
<td><a href="#egovehiclecontrolmaximamsg">EgoVehicleControlMaxima</a></td>
<td>Limits to the controller values.</td>
<tr>
<td><code>target</code> </td>
<td><a href="#egovehiclecontroltargetmsg">EgoVehicleControlTarget</a></td>
<td>Limits to the controller values.</td>
<tr>
<td><code>current</code> </td>
<td><a href="#egovehiclecontrolcurrentmsg">EgoVehicleControlCurrent</a></td>
<td>Limits to the controller values.</td>
<tr>
<td><code>status</code> </td>
<td><a href="#egovehiclecontrolstatusmsg">EgoVehicleControlStatus</a></td>
<td>Limits to the controller values.</td>
<tr>
<td><code>output</code> </td>
<td><a href="#carlaegovehiclecontrolmsg">CarlaEgoVehicleControl</a></td>
<td>Limits to the controller values.</td>
</tbody>
</table>
<br>

---
## EgoVehicleControlMaxima.msg

Controller restrictions (limit values). It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>max_steering_angle</code> </td>
<td>float32</td>
<td>Max. steering angle for a vehicle.</td>
<tr>
<td><code>max_speed</code> </td>
<td>float32</td>
<td>Max. speed for a vehicle.</td>
<tr>
<td><code>max_accel</code> </td>
<td>float32</td>
<td>Max. acceleration for a vehicle.</td>
<tr>
<td><code>max_decel</code> </td>
<td>float32</td>
<td>Max. deceleration for a vehicle. Default: <b>8m/s^2</b></td>
<tr>
<td><code>min_accel</code> </td>
<td>float32</td>
<td>Min. acceleration for a vehicle. When the Ackermann taget accel. exceeds this value, the input accel. is controlled.</td>
<tr>
<td><code>max_pedal</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Min. pedal.</td>
</tbody>
</table>
<br>

---
## EgoVehicleControlStatus.msg

Current status of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>status</code> </td><!---- TBF> <---->
<td>string</td>
<td>Current control status.</td>
<tr>
<td><code>speed_control_activation_count</code> </td><!---- TBF> <---->
<td>uint8</td>
<td>Speed controller.</td>
<tr>
<td><code>speed_control_accel_delta</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Speed controller.</td>
<tr>
<td><code>speed_control_accel_target</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Speed controller.</td>
<tr>
<td><code>accel_control_pedal_delta</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Acceleration controller.</td>
<tr>
<td><code>accel_control_pedal_target</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Acceleration controller.</td>
<tr>
<td><code>brake_upper_border</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Borders for lay off pedal.</td>
<tr>
<td><code>throttle_lower_border</code> </td><!---- TBF> <---->
<td>float32</td>
<td>Borders for lay off pedal.</td>
</tbody>
</table>
<br>

---
## EgoVehicleControlTarget.msg

Target values of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

<table class ="defTable">
<thead>
<th>Field</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>steering_angle</code> </td>
<td>float32</td>
<td>Target steering angle for the controller.</td>
<tr>
<td><code>speed</code> </td>
<td>float32</td>
<td>Target speed for the controller. </td>
<tr>
<td><code>speed_abs</code> </td>
<td>float32</td>
<td>Speed as an absolute value.</td>
<tr>
<td><code>accel</code> </td>
<td>float32</td>
<td>Target acceleration for the controller.</td>
<tr>
<td><code>jerk</code> </td>
<td>float32</td>
<td>Target jerk for the controller.</td>
</tbody>
</table>
<br>