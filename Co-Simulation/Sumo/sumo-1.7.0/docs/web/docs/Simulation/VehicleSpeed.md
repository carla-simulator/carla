---
title: Simulation/VehicleSpeed
permalink: /Simulation/VehicleSpeed/
---

There are a wide range of influences on vehicle speed. They are
described in the following. Each of these influences sets an upper bound
on the vehicle speed. The actual speed in any given situation is the
minimum speed of all influences.

# maxSpeed

The
[`<vType>-attribute maxSpeed`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
models the maximum speed that a vehicle will travel. It can be thought
of as the maximum speed of the engine or the maximum speed desired by
the driver under any circumstances (possibly these two aspects will be
modelled with separate attributes in the future).

# edge/lane speed and speedFactor

The `speed`-attribute is usually [defined for edges](../Networks/PlainXML.md#edge_descriptions)
but may also [differ among the lanes](../Networks/PlainXML.md#lane-specific_definitions)
of the same edge. It models the legal speed limit.

When approaching an edge with a lower speed limit than the current one,
a vehicle will slow down so as to stay within the new limit at the time
of reaching the new edge.

Each vehicle can be [assigned an individual speed multiplier (speedFactor)](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
which then lets it exceed this limit. Since version 1.0.0, vehicles will
have a random speedFactor with a deviation of 0.1 and mean of 1.0 which
means there will be different desired speeds in the vehicle population
by default.

When vehicles are driving freely (unconstrained by other vehicles) they will accelerate until reaching the speed 
```
min(maxSpeed, speedFactor * speedLimit)
```

!!! note
    Legacy behavior can be achieved by setting option **--default.speeddev 0**

Since version 0.24.0 it is also possible to define [vClass-specific speed limits for every edge](../Networks/PlainXML.md#vehicle-class_specific_speed_limits).

# Car Following Model

The [car following model](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#car-following_models)
of a vehicle defines its speed in relation the vehicle ahead. The
default model always selects the maximum speed which is *safe* in the
sense of being able to stop in time to avoid a collision.

## Acceleration and Deceleration

All models are subject to constraints in their [acceleration an deceleration](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#car-following_models).
By default they will not accelerate stronger then the *accel* value. The
default model plans it's maneuvers so as to stay within the *decel*
value (per second) but other models may interpret this value
differently. All models will never brake harder than the
*emergencyDecel* value (which defaults to the same value as *decel* but
may be set independently).

!!! note
    This is just a convention which the available models obey but which could be ignored by custom models.

## Dawdling

Most car-following models support the `sigma`-attribute which models driver
imperfection. For values above **0**, drivers with the default
car-following model will drive *slower* than would be safe by a random
amount (between \[0, `accel`\]).

# Intersections

Vehicles approaching an intersection without the right-of-way have to
slow down. If the intersection is used by other vehicles which have the
right of way, stopping may be necessary until a safe time-window is
found. That time windows is based on the same safety assumptions as the
car-following model. For the default *Krauss*-model this means that each
vehicle must be able to stop safely even if it's lead vehicle brakes
hard to a full stop.

Even if a vehicle has the right-of-way it may need to slow down due to
[impatient](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#impatience)
drivers which drive across the intersection. The [right-of-way rules](../Networks/PlainXML.md#right-of-way)
at an intersection are [defined by the node type-attribute](../Networks/PlainXML.md#node_descriptions)
and by [traffic lights](../Simulation/Traffic_Lights.md).

The visibility of intersections can be controlled by the visibility
attribute for the corresponding connections, see [Connection Descriptions](../Networks/PlainXML.md#connection_descriptions).
Per default, a vehicle approaching from a minor road slows down until it
is 4.5m away from the intersection (even if no prioritized vehicle is
nearby). After that it may start to accelerate again if there is a safe
gap in traffic.

# Lane Changing

Vehicles may decide to slow down in order to execute a lane-change
maneuver. They may also slow down in order to aid other vehicles with
lane-changing. If the lane a vehicle is on does not have a connection to
the next edge on a vehicles route, the vehicle will decelerate and stop.

# Stops

Vehicles will decelerate when approaching the position of a
[`stop`-definition](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops).

# Acceleration / Deceleration constraints

Vehicles can only change their speed by a certain amount each time step.
This is defined by the
[`<vType>`-attributes `accel` and `decel`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)

# departSpeed / arrivalSpeed

Vehicles enter the network using their [defined `departSpeed`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes).
When approaching the end of their route they will adapt their speed to
match their [defined `arrivalSpeed`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes)

# Variable Speed Signs

[Variable speed signs](../Simulation/Variable_Speed_Signs.md) are
used to modify the speed limit of an edge for a defined time interval.

# Calibrators

[Calibrators](../Simulation/Calibrator.md) are used to adapt the
flow on an edge for a defined time interval but may also be used to
modify the speed limit of an edge.

# TraCI

Vehicles can forced to adapt their speed using [TraCI commands](../TraCI/Change_Vehicle_State.md). When using the command
*slow down* stochastic influences on speed are not applied. By using
[the *speed mode* command, various safety related influences can be selectively disabled](../TraCI/Change_Vehicle_State.md#speed_mode_0xb3).
