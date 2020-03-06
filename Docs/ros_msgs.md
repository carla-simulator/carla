# CARLA messages reference

The following reference lists all the CARLA messages available in the ROS bridge. These can be used to enable communication in both ways.  
Any doubts regarding these messages or the CARLA-ROS bridge can be solved in the forum: 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/c/carla-ecosystem/ros-bridge" target="_blank" class="btn btn-neutral" title="Go to the CARLA Forum, ROS bridge section">
CARLA forum</a>
</p>
</div>

---
## CarlaActorInfo.msg

Comprises the information regarding an actor to be shared between ROS and CARLA.  
The [CarlaActorList.msg](#carlaactorlist) message is a list of these items. 

| Field   | Type   | Description |
| ----------- | ------ | ----------- |
| `id`        | uint32 | The ID of the actor. |
| `parent_id` | uint32 | The ID of the parent actor. `0` if no parent available. |
| `type`      | string | The identifier of the blueprint this actor was based on. |
| `rolename`  | string | Role assigned to the actor when spawned. |

---
## CarlaActorList.msg

Contains a list of messages with some basic information for CARLA actors.

| Field       | Type             | Description |
| ----------- | ---------------- | ----------- |
| `actors`    | [CarlaActorInfo](#carlaactorinfomsg)[] | List of messages with actors' information. |

---
## CarlaCollisionEvent.msg

Registers information regarding a collision event detected by the collision sensor of an actor.

| Field            | Type                  | Description |
| ---------------- | --------------------- | ----------- |
| `header`       | [Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published. |
| `other_actor_id` | uint32                | ID of the actor against whom the collision was detected.  |
| `normal_impulse` | geometry_msgs/Vector3 | Vector representing resulting impulse from the collision. |

---
## CarlaControl.msg

These messages are used to control the simulation while in synchronous mode. The constant defined is translated as stepping commands.  

| Field       | Type | Description |
| ----------- | ---- | ----------- |
| `command`   | int8 | __PLAY__=0 <br> __PAUSE__=1 <br> __STEP_ONCE__=2 |

<br>

!!! Important
    In synchronous mode, only the ROS bridge client is allowed to tick.

---
## CarlaEgoVehicleControl.msg

Messages sent to apply a control to a vehicle in both modes, autopilot and manual. These are published in a stack. 

| Field               | Type    | Description |
| ------------------- | ------- | ----------- |
| `header`            | [Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published. |
| `throttle`          | float32 | Scalar value to cotrol the vehicle throttle: __[0.0, 1.0]__ |
| `steer`             | float32 | Scalar value to control the vehicle steering direction: __[-1.0, 1.0]__ to control the vehicle steering   |
| `brake`             | float32 | Scalar value to control the vehicle brakes: __[0.0, 1.0]__       |
| `hand_brake`        | bool    | If __True__, the hand brake is enabled. |
| `reverse`           | bool    | If __True__, the vehicle will move reverse. |
| `gear`              | int32   | Changes between the available gears in a vehicle. |
| `manual_gear_shift` | bool    | If __True__, the gears will be shifted using `gear`. |

---
## CarlaEgoVehicleInfo.msg

Contains some static information regarding a vehicle, mostly the attributes used to define the vehicle's physics.  

| Field                                         | Type                                                    | Description |
| --------------------------------------------- | ------------------------------------------------------- | ----------- |
| `id`                                          | uint32                                                  | ID of the vehicle actor. |
| `type`                                        | string                                                  | The identifier of the blueprint this vehicle was based on. |
| `rolename`                                    | string                                                  | Role assigned to the vehicle. |
| `wheels`                                      | [CarlaEgoVehicleInfoWheel](#carlaegovehicleinfowheel)[] | List of messages with information regarding wheels. |
| `max_rpm`                                     | float32                                                 | Maximum RPM of the vehicle's engine. |
| `moi`                                         | float32                                                 | Moment of inertia of the vehicle's engine. |
| `damping_rate_full_throttle`                  | float32                                                 | Damping rate when the throttle is at maximum. |
| `damping_rate_zero_throttle_clutch_engaged`   | float32                                                 | Damping rate when the throttle is zero with clutch engaged. |
| `damping_rate_zero_throttle_clutch_disengaged`| float32                                                 | Damping rate when the throttle is zero with clutch disengaged. |
| `use_gear_autobox`                            | bool                                                    | If __True__, the vehicle will have an automatic transmission. |
| `gear_switch_time`                            | float32                                                 | Switching time between gears. |
| `clutch_strength`                             | float32                                                 | The clutch strength of the vehicle. Measured in __Kgm^2/s__. |
| `mass`                                        | float32                                                 | The mass of the vehicle measured in Kg. |
| `drag_coefficient`                            | float32                                                 | Drag coefficient of the vehicle's chassis. |
| `center_of_mass`                              | geometry_msgs/Vector3                                   | The center of mass of the vehicle. |

---
## CarlaEgoVehicleInfoWheel.msg

Contains some static information regarding a wheel that will be part of a [CarlaEgoVehicleInfo.msg](#carlaegovehicleinfo) message.

| Field                  | Type                  | Description |
| ---------------------- | --------------------- | ----------- |
| `tire_friction`        | float32               | A scalar value that indicates the friction of the wheel. |
| `damping_rate`         | float32               | The damping rate of the wheel. |
| `max_steer_angle`      | float32               | The maximum angle in degrees that the wheel can steer. |
| `radius`               | float32               | The radius of the wheel in centimeters. |
| `max_brake_torque`     | float32               | The maximum brake torque in Nm. |
| `max_handbrake_torque` | float32               | The maximum handbrake torque in Nm. |
| `position`             | gemoetry_msgs/Vector3 | World position of the wheel. |

---
## CarlaEgoVehicleStatus.msg

Details the current status of the vehicle as an object in the world.  

| Fied          | Type                                              | Description |
| ------------- | ------------------------------------------------- | ----------- |
| `header`      | [Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published. |
| `velocity`    | float32                                           | Current speed of the vehicle. |
| `acceleration`| geometry_msgs/Accel                               | Current acceleration of the vehicle. |
| `orientation` | geometry_msgs/Quaternion                          | Current orientation of the vehicle. |
| `control`     | [CarlaEgoVehicleControl](#carlaegovehiclecontrol) | Current control values as reported by CARLA. |

---
## CarlaLaneInvasionEvent.msg

These messages are used to publish lane invasions detected by a lane-invasion sensor attached to a vehicle. The invasions detected in the last step are passed as a list with a constant definition to identify the lane crossed.  

| Field                   | Type    | Description |
| ----------------------- | ------- | ----------- |
| `header`       | [Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published. |
| `crossed_lane_markings` | int32[] | __LANE_MARKING_OTHER__=0 <br> __LANE_MARKING_BROKEN__=1 <br> __LANE_MARKING_SOLID__=2 |

---
## CarlaScenario.msg

Details for a test scenario. 

| Field          | Type               | Description                                   |
| ---------------| ------------------ | --------------------------------------------- |
| `name`         | string             | Name of the scenario.                         |
| `scenario_file`| string             | Test file for the scenario.                   |
| `destination`  | geometry_msgs/Pose | Goal location of the scenario.                |
| `target_speed` | float64            | Desired speed to drive at during the scenario |

---
## CarlaScenarioList.msg

Contains a list of test scenarios to run in ScenarioRunner. 

| Field       | Type                                 | Description            |
| ----------- | ------------------------------------ | ---------------------- |
| `scenarios` | [CarlaScenario](#carlascenariomsg)[] | The list of scenarios. |

---
## CarlaScenarioRunnerStatus.msg

Represents the current state of the ScenarioRunner, that can be managed using an enum definition. 

| Field    | Type  | Description                                                                                                                                  |
| -------- | ----- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| `status` | uint8 | Current state of the scenario as an enum: <br> __STOPPED__=0 <br> __STARTING__=1 <br> __RUNNING__=2 <br> __SHUTTINGDOWN__=3 <br> __ERROR__=4 |

---
## CarlaStatus.msg

Details the current world settings of the simulation. 

| Field                      | Type    | Description                                                   |
| -------------------------- | ------- | ------------------------------------------------------------- |
| `frame`                    | uint64  | Current frame number.                                         |
| `fixed_delta_seconds`      | float32 | Simulation time between last and current step.                |
| `synchronous_mode`         | bool    | If __True__, synchronous mode is enabled.                         |
| `synchronous_mode_running` | bool    | __True__ when the simulation is running. __False__ when it is paused. |

---
## CarlaTrafficLightStatus.msg

Contains a constant definition regarding the state of a traffic light. 

| Field                      | Type    | Description                               |
| -------------------------- | ------- | ----------------------------------------- |
| `id`                       | uint32  | ID of the traffic light actor. |
| `state`                    | uint8   | __RED__=0 <br> __YELLOW__=1 <br> __GREEN__=2 <br> __OFF__=3 <br> __UNKNOWN__=4 |

---
## CarlaTrafficLightStatusList.msg

Comprises a list of all traffic lights with their status. 

| Field            | Type                                                  | Description      |
| ---------------- | ----------------------------------------------------- | ---------------- |
| `traffic_lights` | [CarlaTrafficLightStatus](#carlatrafficlightstatus)[] | A list of messages summarizing traffic light states. |

---
## CarlaWalkerControl.msg

Contains the information needed to apply a movement controller to a walker. 

| Field       | Type                   | Description                   |
| ----------- | ---------------------- | ----------------------------- |
| `direction` | geometry_msgs/Vector3  | Vector that controls the direction of the walker. |
| `speed`     | float32                | A scalar value to control the walker's speed. |
| `jump`      | bool                   | If __True__, the walker will jump. |

---
## CarlaWaypoint.msg

Summarizes data contained in a waypoint. 

| Field         | Type               | Description                   |
| ------------- | ------------------ | ----------------------------- |
| `road_id`     | int32              | OpenDRIVE road's id.  |
| `section_id`  | int32              | OpenDRIVE section's id, based on the order that they are originally defined. |
| `lane_id`     | int32              | OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road. |
| `is_junction` | bool               | __True__ if the current Waypoint is on a junction as defined by OpenDRIVE. |
| `pose`        | [geometry_msgs/Pose](http://docs.ros.org/api/geometry_msgs/html/msg/Pose.html) | Position and orientation of the waypoint. |

---
## CarlaWorldInfo.msg

Information about the current CARLA map (name and OpenDRIVE). 

| Field       | Type    | Description                   |
| ----------- | ------- | ----------------------------- |
| `map_name`  | string  | Name of the CARLA map loaded in the current world. |
| `opendrive` | string  | .xodr OpenDRIVE file of the current map as a string. |

---
## EgoVehicleControlCurrent.msg

Represents the current time, speed and acceleration values of the vehicle used by the controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field       | Type     | Description                   |
| ----------- | -------- | ----------------------------- |
| `time_sec`  | float32  | Current time when the controller is applied. |
| `speed`     | float32  | Current speed applied by the controller. |
| `speed_abs` | float32  | Speed as an absolute value. | 
| `accel`     | float32  | Current acceleration applied by the controller. |

---
## EgoVehicleControlInfo.msg

Contains all the current values used within an Ackermann controller. These messages are useful for debugging. 

| Field          | Type                                                                    | Description                    |
| -------------- | ----------------------------------------------------------------------- | ------------------------------ |
| `header`       | [Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published. |
| `restrictions` | [EgoVehicleControlMaxima](#egovehiclecontrolmaximamsg)                  | Limits to the controller values.             |
| `target`       | [EgoVehicleControlTarget](#egovehiclecontroltargetmsg)                  | Target values the controller aims for. |
| `current`      | [EgoVehicleControlCurrent](#egovehiclecontrolcurrentmsg)                | Currently measured values. |
| `status`       | [EgoVehicleControlStatus](#egovehiclecontrolstatusmsg)                  | Current control status.    |
| `output`       | [CarlaEgoVehicleControl](#carlaegovehiclecontrolmsg)                    | Output controller that will be applied in CARLA. |


---
## EgoVehicleControlMaxima.msg

Represents the restrictions of a controller (limit values). It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                | Type     | Description                   |
| -------------------- | -------- | ----------------------------- |
| `max_steering_angle` | float32  | Max. steering angle for a vehicle. |
| `max_speed`          | float32  | Max. speed for a vehicle. |
| `max_accel`          | float32  | Max. acceleration for a vehicle. |
| `max_decel`          | float32  | Max. deceleration for a vehicle. Default: __8 m/s^2__ |
| `min_accel`          | float32  | Min. acceleration for a vehicle. When the Ackermann taget accel. exceeds this value, the input accel. is controlled. |
| `max_pedal`          | float32  | Min. pedal. |<!---- TBF>

---
## EgoVehicleControlStatus.msg

Represents the current status of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                            | Type    | Description                   |
| -------------------------------- | ------- | ----------------------------- |
| `status`                         | string  | Current control status.       |<!---- TBF>
| `speed_control_activation_count` | uint8   | Speed controller. |<!---- TBF>
| `speed_control_accel_delta`      | float32 | Speed controller. |<!---- TBF>
| `speed_control_accel_target`     | float32 | Speed controller. |<!---- TBF>
| `accel_control_pedal_delta`      | float32 | Acceleration controller. |<!---- TBF>
| `accel_control_pedal_delta`      | float32 | Acceleration controller. |<!---- TBF>
| `brake_upper_border`             | float32 | Borders for lay off pedal. |<!---- TBF>
| `throttle_lower_border`          | float32 | Borders for lay off pedal. |<!---- TBF>

---
## EgoVehicleControlTarget.msg

Represents the target values for the variables of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field            | Type     | Description                   |
| ---------------- | -------- | ----------------------------- |
| `steering_angle` | float32  | Target steering angle for the controller. |
| `speed`          | float32  | Target speed for the controller. |
| `speed_abs`      | float32  | Speed as an absolute value. |
| `accel`          | float32  | Target acceleration for the controller. |
| `jerk`           | float32  | Target jerk for the controller. |