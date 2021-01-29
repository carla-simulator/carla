---
title: Simpla
permalink: /Simpla/
---

!!! note
    simpla is written for SUMO versions > 0.30.0. We strongly recommend regularly updating to the newest SUMO version if possible. [Nightly snapshots can be found here](Downloads.md#nightly_snapshots).
    <br><br>
    As simpla uses TraCI's [StepListener](TraCI/Interfacing_TraCI_from_Python.md#adding_a_steplistener) interface, it cannot be combined with controlling several sumo instances from the same client. 
    
# Intro

simpla is a configurable, platooning plugin for the [TraCI
Python client](TraCI/Interfacing_TraCI_from_Python.md). It
manages the spontaneous formation of vehicle platoons and allows you to
define specific behavior for vehicles that travel inside a platoon.

This is achieved by defining additional [vehicle
types](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
which represent platooning modes, and by providing information, which
type (mode) to use for normal, solitary travel, and which one to use
when platooning is activated. Further, 'catch-up' modes may be
specified, which are used when a potential platooning partner is farther
ahead downstream.

For platooning up to four operational modes may be specified apart from
the normal traveling mode (see section
[Configuration](#configuration) for the details):

- Platoon leader mode (parametrization of a vehicle driving at the
  front of a platoon)
- Platoon follower mode (parametrization of a vehicle driving behind
  another vehicle within a platoon)
- Platoon catch-up mode (parametrization of a vehicle in range of a
  platoon, which is feasible for joining)
- Platoon catch-up follower mode (parametrization of a vehicle
  traveling in a platoon, with a platoon leader in catchup mode)

!!! note
    The corresponding vTypes have to be known by SUMO at startup. This means, they should be placed at the beginning of the [demand definitions](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md). When using [duarouter](duarouter.md) for route generation, please use the option **--vtype-output** to generate a seperate file containing the vTypes, which can be loaded upfront.


It suffices to specify the platoon leader mode to use simpla. The
behavior in situations corresponding to the follower mode will use the
leader mode if the follower mode is not specified. The situations
corresponding to the catch-up mode will use the solitary mode if no
separate catch-up mode is specified. The situations corresponding to the
catch-up follower mode will use the follower mode if no separate
catch-up follower mode is specified.

!!! note
    Simpla currently does not support lane-changing to join a platoon. Vehicles that are already in a platoon will try to change their lane if the platoon leader changes it's lane though.

# Configuration

The configuration of simpla is done by providing an xml-file containing
obligatory and optional parameters for the application. It has a root
element `configuration` with children tagged by the parameter name and holding
attributes corresponding to the different parameter values. All possible
attributes are listed in the attribute table below. A very simple
configuration file could look like this (a more complete example, which
serves as a template can be found in the installation directory
`<SUMO_HOME>/tools/simpla`):

```
<configuration>
    <vehicleSelector value="pkw" />
    <vTypeMap original="simple_pkw" leader="simple_pkw_as_platoon_leader"  follower="simple_pkw_as_platoon_follower"/>
    ...
</configuration>
```

The following table summarizes all admissible configuration parameters:

| Parameter Name             | Value Type                                                                    | Description                           |
| -------------------------- | ----------------------------------------------------------------------------- | ------------------------------------- |
| **vehicleSelectors**       | string                                                                        | A vehicle is controlled by simpla if its vehicle type id has a given vehicle selector string as a substring. The given value may be a comma-separated list of selectors. Defaults to an empty string, which selects all vehicles for control.                                                                                                                                                                                                       |
| **controlRate**            | float                                                                         | The number of the updates (per seconds) for the platoon management logic. Defaults to 1.0 sec. <br>**Note:** The rate cannot be increased beyond 1/timestep.                                                                                                                                                                                                                                                                                                                                                      |
| **maxPlatoonGap**          | float                                                                         | Gap (in m.) below which vehicles are considered as a platoon (if their vType parameters allow safe traveling for the actual situation). Defaults to 15 m.                                                                                                                                                                                                                                                                                           |
| **platoonSplitTime**       | float                                                                         | Time (in secs.) until a vehicle which maintains a distance larger than **maxPlatoonGap** from its leader within the platoon (or travels on a different lane or behind a vehicle not belonging to its platoon) is split off. Defaults to 3.0 secs.                                                                                                                                                                                                   |
| **catchupDist**            | float                                                                         | If a catch-up mode was defined, vehicles switch their type to the corresponding catch-up vehicle type as soon as a platoon is ahead closer than the given value (in m.). Defaults to 50.0 m.                                                                                                                                                                                                                                                        |
| **switchImpatienceFactor** | float                                                                         | The switch impatience factor determines the magnitude of the effect that an increasing waiting time for a mode switch (due to safety constraints) has on the active speed factor of a vehicle. The active speed factor is calculated as `activeSpeedFactor = modeSpecificSpeedFactor/(1+impatienceFactor*waitingTime)`. The default value for the switch impatience factor is 0.1. Providing values <= 0 will deactivate the impatience mechanism. |
| **lcMode**                 | integers (binary code) {original, leader, follower, catchup, catchupFollower} | Specifies the [lane-change modes](TraCI/Change_Vehicle_State.md#lane_change_mode_0xb6) to be used during the corresponding operation modes (e.g. `<lcMode leader=597 follower=514 .../>`). The modes *leader* and *original* default to 594 and *follower* and *catch-up* modes default to 514.                                                                                                                                                                              |
| **speedFactor**            | floats {original, leader, follower, catchup, catchupFollower}                 | Specifies the [speed factors](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions) to be used during the corresponding operation modes (e.g. `<speedFactor follower="1.1" catchup="1.2" .../>`). All modes default to 1.0.                                                                                                                                                                                                                                              |
| **vTypeMap**               | strings {original, leader, follower, catchup, catchupFollower}                | Specifies the [vehicle types](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types) to be used during the corresponding operation modes (e.g. `<vTypeMap original="type1" leader="leader_type1" .../>`).                                                                                                                                                                                                                                                                              |
| **vTypeMapFile**           | string                                                                        | Specifies the name of a file containing the specification of vehicle type mappings. The referenced file should contain lines of the form `origTypeID:leaderTypeID:followerTypeID:catchupTypeID:catchupFollowerTypeID`. The given vehicle type IDs can be empty strings leading to the default behavior as described above.<br>**Note:** Mappings specified in such a file override mappings given as xml-elements of type `vTypeMap`.                                                                                                                          |
| **verbosity**              | int                                                                           | Specifies the level of output during the simulation. |


# Integrating simpla into your traci script

First of all you should ensure that the directory `<SUMO_HOME>/tools` is in your
`PYTHONPATH` ([See here for further
explanations](TraCI/Interfacing_TraCI_from_Python.md#importing_traci_in_a_script)).
To use simpla with your python script further requires the provision of
a configuration file [as described
above](#configuration). Assuming such a file named
`simpla.cfg` is located in your working directory, all you have to do is
importing simpla as a python module:

```
import simpla
```

and, after [establishing a
TraCI-connection](TraCI/Interfacing_TraCI_from_Python.md#first_steps)
to SUMO, load your configuration.

```
traci.start(...)
...
simpla.load("simpla.cfg")
```

The call to `simpla.load` automatically installs a
[TraCI-stepListener](TraCI/Interfacing_TraCI_from_Python.md#adding_a_steplistener)
and the platoon management is executed at the end of each call to
`traci.simulationStep()`.

!!! caution
    (1) simpla changes the vehicle types, speedfactors, and lane changemodes of all connected vehicles. If your application does so as well, this might have unintended consequences.

    (2) Currently, steps of lengths other than DeltaT are not supported (i.e. if traci.simulationStep() is called with argument when simpla is running this may yield undesired behaviour). 

    (3) simpla adds subscriptions to VAR_SPEED, VAR_ROAD_ID, VAR_LANE_ID, and VAR_LANE_INDEX and removes them when stopped (per `simpla.stop()`).

    (4) If you start several simulation runs from the same python script, be sure to call traci.close() or simpla.stop() in between to reset simpla's state.

# Utility Functions

It is planned to add utility functions for vehicle control in simpla.
They can be referenced by `simpla.<function_name>` Currently, only one
is implemented.

## openGap()

The function has the following syntax:

`openGap(vehID, desiredGap, desiredSpeedDiff, maximumDecel, duration)`

The arguments are:

| Parameter Name          | Value Type | Description                                                                         |
| ----------------------- | ---------- | ----------------------------------------------------------------------------------- |
| **vehID**            | string     | The ID of the vehicle to be controlled.                                             |
| **desiredGap**       | float\>0   | The gap that shall be established.                                                  |
| **desiredSpeedDiff** | float\>0   | The rate at which the gap is open if possible.                                      |
| **maximumDecel**     | float\>0   | The maximal deceleration at which the desiredSpeedDiff is tried to be approximated. |
| **duration**         | float\>0   | The period for which the gap control should be active.                              |

The methods adds a controller for the opening of a gap in front of the
given vehicle, which stays active for a period of the given duration. If
a leader is closer than the desiredGap, the controller tries to
establish the desiredGap by inducing the given speedDifference, while
not braking harder than maximumDecel. Internally, an object of the class
GapCreator is created to manage the vehicle state and is added to traci
as a
[stepListener](TraCI/Interfacing_TraCI_from_Python.md#adding_a_steplistener).
