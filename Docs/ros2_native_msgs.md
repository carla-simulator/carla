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

| Field                                                     | Type                                                      | Description                                               |
| ---------------------------------------------------------- | ---------------------------------------------------------- | ---------------------------------------------------------- |
| `id`                                                      | uint32                                                    | The ID of the actor.                                      |
| `parent_id`                                               | uint32                                                    | The ID of the parent actor. \`0\` if no parent available. |
| `type`                                                    | string                                                    | The identifier of the blueprint this actor was based on.  |
| `rolename`                                                | string                                                    | Role assigned to the actor when spawned.                  |




---
## CarlaActorList.msg

A list of messages with some basic information for CARLA actors.

| Field                                      | Type                                       | Description                                |
| -------------------------------------------------------- | -------------------------------------------------------- | -------------------------------------------------------- |
| `actors`                                   | [CarlaActorInfo](<#carlaactorinfomsg>)     | List of messages with actors' information. |



---
## CarlaCollisionEvent.msg

Data retrieved on a collision event detected by the collision sensor of an actor.

| Field                                                                     | Type                                                                      | Description                                                               |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| `header`                                                                  | [Header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published.                    |
| `other_actor_id`                                                          | uint32                                                                    | ID of the actor against whom the collision was detected.                  |
| `normal_impulse`                                                          | geometry\_msgs/Vector3                                                    | Vector representing resulting impulse from the collision.                 |




---
## CarlaControl.msg

These messages control the simulation while in synchronous, non-passive mode. The constant defined is translated as stepping commands.  

| Field                                               | Type                                                | Description                                         |
| ----------------------------------------------------------- | ----------------------------------------------------------- | ----------------------------------------------------------- |
| `command`                                           | int8                                                | **PLAY**=0 <br>**PAUSE**=1 <br>**STEP\_ONCE**=2 |

---

## CarlaEgoVehicleControl.msg

Messages sent to apply a control to a vehicle in both modes, autopilot and manual. These are published in a stack. 

| Field                                                                                                   | Type                                                                                                    | Description                                                                                             |
| ------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| `header`                                                                                                | [Header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html)                               | Time stamp and frame ID when the message is published.                                                  |
| `throttle`                                                                                              | float32                                                                                                 | Scalar value to cotrol the vehicle throttle: **[0.0, 1.0]**                                             |
| `steer`                                                                                                 | float32                                                                                                 | Scalar value to control the vehicle steering direction: **[-1.0, 1.0]** to control the vehicle steering |
| `brake`                                                                                                 | float32                                                                                                 | Scalar value to control the vehicle brakes: **[0.0, 1.0]**                                              |
| `hand_brake`                                                                                            | bool                                                                                                    | If **True**, the hand brake is enabled.                                                                 |
| `reverse`                                                                                               | bool                                                                                                    | If **True**, the vehicle will move reverse.                                                             |
| `gear`                                                                                                  | int32                                                                                                   | Changes between the available gears in a vehicle.                                                       |
| `manual_gear_shift`                                                                                     | bool                                                                                                    | If **True**, the gears will be shifted using `gear`.                                                    |

---

## CarlaEgoVehicleInfo.msg

Static information regarding a vehicle, mostly the attributes used to define the vehicle's physics.  

| Field                                                          | Type                                                           | Description                                                    |
| -------------------------------------------------------------- | -------------------------------------------------------------- | -------------------------------------------------------------- |
| `id`                                                           | uint32                                                         | ID of the vehicle actor.                                       |
| `type`                                                         | string                                                         | The identifier of the blueprint this vehicle was based on.     |
| `type`                                                         | string                                                         | The identifier of the blueprint this vehicle was based on.     |
| `rolename`                                                     | string                                                         | Role assigned to the vehicle.                                  |
| `wheels`                                                       | [CarlaEgoVehicleInfoWheel](<#carlaegovehicleinfowheelmsg>)     | List of messages with information regarding wheels.            |
| `max_rpm`                                                      | float32                                                        | Maximum RPM of the vehicle's engine.                           |
| `moi`                                                          | float32                                                        | Moment of inertia of the vehicle's engine.                     |
| `damping_rate_full_throttle`                                   | float32                                                        | Damping rate when the throttle is at maximum.                  |
| `damping_rate_zero_throttle`<br>`_clutch_engaged`            | float32                                                        | Damping rate when the throttle is zero with clutch engaged.    |
| `damping_rate_zero_throttle`<br>`_clutch_disengaged`         | float32                                                        | Damping rate when the throttle is zero with clutch disengaged. |
| `use_gear_autobox`                                             | bool                                                           | If **True**, the vehicle will have an automatic transmission.  |
| `gear_switch_time`                                             | float32                                                        | Switching time between gears.                                  |
| `clutch_strength`                                              | float32                                                        | The clutch strength of the vehicle. Measured in **Kgm^2/s**.   |
| `mass`                                                         | float32                                                        | The mass of the vehicle measured in Kg.                        |
| `drag_coefficient`                                             | float32                                                        | Drag coefficient of the vehicle's chassis.                     |
| `center_of_mass`                                               | geometry\_msgs/Vector3                                         | The center of mass of the vehicle.                             |

---

## CarlaEgoVehicleInfoWheel.msg

Static information regarding a wheel that will be part of a [CarlaEgoVehicleInfo.msg](#carlaegovehicleinfomsg) message.

| Field                                                    | Type                                                     | Description                                              |
| -------------------------------------------------------- | -------------------------------------------------------- | -------------------------------------------------------- |
| `tire_friction`                                          | float32                                                  | A scalar value that indicates the friction of the wheel. |
| `damping_rate`                                           | float32                                                  | The damping rate of the wheel.                           |
| `max_steer_angle`                                        | float32                                                  | The maximum angle in degrees that the wheel can steer.   |
| `radius`                                                 | float32                                                  | The radius of the wheel in centimeters.                  |
| `max_brake_torque`                                       | float32                                                  | The maximum brake torque in Nm.                          |
| `max_handbrake_torque`                                   | float32                                                  | The maximum handbrake torque in Nm.                      |
| `position`                                               | geometry\_msgs/Vector3                                   | World position of the wheel.                             |

---

## CarlaEgoVehicleStatus.msg

Current status of the vehicle as an object in the world.  

| Field                                                                     | Type                                                                      | Description                                                               |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| `header`                                                                  | [Header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published.                    |
| `velocity`                                                                | float32                                                                   | Current speed of the vehicle.                                             |
| `acceleration`                                                            | geometry\_msgs/Accel                                                      | Current acceleration of the vehicle.                                      |
| `orientation`                                                             | geometry\_msgs/Quaternion                                                 | Current orientation of the vehicle.                                       |
| `control`                                                                 | [CarlaEgoVehicleControl](<#carlaegovehiclecontrolmsg>)                    | Current control values as reported by CARLA.                              |

---

## CarlaLaneInvasionEvent.msg

These messages publish lane invasions detected by a lane-invasion sensor attached to a vehicle. The invasions detected in the last step are passed as a list with a constant definition to identify the lane crossed.  



| Field                                                                                         | Type                                                                                          | Description                                                                                   |
| ---------------------------------------------------- | ---------------------------------------------------- | ---------------------------------------------------- |
| `header`                                                                                      | [header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html)                     | Time stamp and frame ID when the message is published.                                        |
| `crossed_lane_markings`                                                                       | int32[]                                                                                       | **LANE\_MARKING\_OTHER**=0 <br>**LANE\_MARKING\_BROKEN**=1 <br>**LANE\_MARKING\_SOLID**=2 |

---

## CarlaScenario.msg

Details for a test scenario. 


| Field                              | Type                               | Description                        |
| ---------------------------------- | ---------------------------------- | ---------------------------------- |
| `name`                             | string                             | Name of the scenario.              |
| `scenario_file`                    | string                             | Test file for the scenario.        |
| `destination`                      | geometry\_msgs/Pose                | Goal location of the scenario.     |
| `target_speed`                     | float64                            | Desired speed during the scenario. |

---

## CarlaScenarioList.msg

List of test scenarios to run in ScenarioRunner. 


| Field                                  | Type                                   | Description                            |
| -------------------------------------- | -------------------------------------- | -------------------------------------- |
| `scenarios`                            | [CarlaScenario[]](<#carlascenariomsg>) | List of scenarios.                     |

---

## CarlaScenarioRunnerStatus.msg

Current state of the ScenarioRunner. It is managed using a constant. 


| Field                                                                                                                                             | Type                                                                                                                                              | Description                                                                                                                                       |
| ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| `status`                                                                                                                                          | uint8                                                                                                                                             | Current state of the scenario as an enum: <br>**STOPPED**=0 <br>**STARTING**=1 <br>**RUNNING**=2 <br>**SHUTTINGDOWN**=3 <br>**ERROR**=4 |

---

## CarlaStatus.msg

Current world settings of the simulation. 


| Field   | Type    | Description    |
| -------------------------------------------------- | -------------------------------------------------- | -------------------------------------------------- |
| `frame`      | uint64        | Current frame number.    |
| `fixed_delta_seconds`    | float32    | Simulation time between last and current step.  |
| `synchronous_mode`   | bool  | If **True**, synchronous mode is enabled.   |
| `synchronous_mode_running`   | bool    | **True** when the simulation is running. **False** when it is paused. |

---

## CarlaTrafficLightStatus.msg

Constant definition regarding the state of a traffic light. 

| Field                                                                              | Type                                                                               | Description                                                                        |
| ---------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| `id`                                                                               | uint32                                                                             | ID of the traffic light actor.                                                     |
| `state`                                                                            | uint8                                                                              | **RED**=0 <br>**YELLOW**=1 <br>**GREEN**=2 <br>**OFF**=3 <br>**UNKNOWN**=4 |

---

## CarlaTrafficLightStatusList.msg

List of traffic lights with their status. 

| Field                                                      | Type                                                       | Description                                                |
| ---------------------------------------------------------- | ---------------------------------------------------------- | ---------------------------------------------------------- |
| `scenarios`                                                | [CarlaTrafficLightStatus[]](<#carlatrafficlightstatusmsg>) | A list of messages summarizing traffic light states.       |

---

## CarlaWalkerControl.msg

Information needed to apply a movement controller to a walker. 

| Field                                             | Type                                              | Description                                       |
| ------------------------------------------------- | ------------------------------------------------- | ------------------------------------------------- |
| `direction`                                       | geometry\_msgs/Vector3                            | Vector that controls the direction of the walker. |
| `speed`                                           | float32                                           | A scalar value to control the walker's speed.     |
| `jump`                                            | bool                                              | If **True**, the walker will jump.                |

---

## CarlaWaypoint.msg

Data contained in a waypoint object. 

| Field                                                                                                                                    | Type                                                                                                                                     | Description                                                                                                                              |
| ---------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| `road_id`                                                                                                                                | int32                                                                                                                                    | OpenDRIVE road's id.                                                                                                                     |
| `section_id`                                                                                                                             | int32                                                                                                                                    | OpenDRIVE section's id, based on the order that they are originally defined.                                                             |
| `lane_id`                                                                                                                                | int32                                                                                                                                    | OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road. |
| `is_junction`                                                                                                                            | bool                                                                                                                                     | **True**, if the current Waypoint is on a junction as defined by OpenDRIVE.                                                              |
| `is_junction`                                                                                                                            | [geometry\_msgs/Pose](https://docs.ros.org/en/api/geometry_msgs/html/msg/Pose.html)                                                        | **True** when the simulation is running. **False** when it is paused.                                                                    |

---

## CarlaWorldInfo.msg

Information about the current CARLA map. 

| Field                                                | Type                                                 | Description                                          |
| ---------------------------------------------------- | ---------------------------------------------------- | ---------------------------------------------------- |
| `map_name`                                           | string                                               | Name of the CARLA map loaded in the current world.   |
| `opendrive`                                          | string                                               | .xodr OpenDRIVE file of the current map as a string. |

---

## EgoVehicleControlCurrent.msg

Current time, speed and acceleration values of the vehicle. Used by the controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                                           | Type                                            | Description                                     |
| ----------------------------------------------- | ----------------------------------------------- | ----------------------------------------------- |
| `time_sec`                                      | float32                                         | Current time when the controller is applied.    |
| `speed`                                         | float32                                         | Current speed applied by the controller.        |
| `speed_abs`                                     | float32                                         | Speed as an absolute value.                     |
| `accel`                                         | float32                                         | Current acceleration applied by the controller. |

---

## EgoVehicleControlInfo.msg

Current values within an Ackermann controller. These messages are useful for debugging. 

| Field                                                                     | Type                                                                      | Description                                                               |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| `header`                                                                  | [header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html) | Time stamp and frame ID when the message is published.                    |
| `restrictions`                                                            | [EgoVehicleControlMaxima](<#egovehiclecontrolmaximamsg>)                  | Limits to the controller values.                                          |
| `target`                                                                  | [EgoVehicleControlTarget](<#egovehiclecontroltargetmsg>)                  | Limits to the controller values.                                          |
| `current`                                                                 | [EgoVehicleControlCurrent](<#egovehiclecontrolcurrentmsg>)                | Limits to the controller values.                                          |
| `status`                                                                  | [EgoVehicleControlStatus](<#egovehiclecontrolstatusmsg>)                  | Limits to the controller values.                                          |
| `output`                                                                  | [CarlaEgoVehicleControl](<#carlaegovehiclecontrolmsg>)                    | Limits to the controller values.                                          |

---

## EgoVehicleControlMaxima.msg

Controller restrictions (limit values). It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                                                                                                                | Type                                                                                                                 | Description                                                                                                          |
| -------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `max_steering_angle`                                                                                                 | float32                                                                                                              | Max. steering angle for a vehicle.                                                                                   |
| `max_speed`                                                                                                          | float32                                                                                                              | Max. speed for a vehicle.                                                                                            |
| `max_accel`                                                                                                          | float32                                                                                                              | Max. acceleration for a vehicle.                                                                                     |
| `max_decel`                                                                                                          | float32                                                                                                              | Max. deceleration for a vehicle. Default: **8m/s^2**                                                                 |
| `min_accel`                                                                                                          | float32                                                                                                              | Min. acceleration for a vehicle. When the Ackermann taget accel. exceeds this value, the input accel. is controlled. |
| `max_pedal`                                                                                                          | float32                                                                                                              | Min. pedal.                                                                                                          |

---

## EgoVehicleControlStatus.msg

Current status of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                            | Type                             | Description                      |
| -------------------------------- | -------------------------------- | -------------------------------- |
| `status`                         | string                           | Current control status.          |
| `speed_control_activation_count` | uint8                            | Speed controller.                |
| `speed_control_accel_delta`      | float32                          | Speed controller.                |
| `speed_control_accel_target`     | float32                          | Speed controller.                |
| `accel_control_pedal_delta`      | float32                          | Acceleration controller.         |
| `accel_control_pedal_target`     | float32                          | Acceleration controller.         |
| `brake_upper_border`             | float32                          | Borders for lay off pedal.       |
| `throttle_lower_border`          | float32                          | Borders for lay off pedal.       |

---

## EgoVehicleControlTarget.msg

Target values of the ego vehicle controller. It is part of a `Carla_Ackermann_Control.EgoVehicleControlInfo.msg` message.

| Field                                     | Type                                      | Description                               |
| ----------------------------------------- | ----------------------------------------- | ----------------------------------------- |
| `steering_angle`                          | float32                                   | Target steering angle for the controller. |
| `speed`                                   | float32                                   | Target speed for the controller.          |
| `speed_abs`                               | float32                                   | Speed as an absolute value.               |
| `accel`                                   | float32                                   | Target acceleration for the controller.   |
| `jerk`                                    | float32                                   | Target jerk for the controller.           |

<br>
