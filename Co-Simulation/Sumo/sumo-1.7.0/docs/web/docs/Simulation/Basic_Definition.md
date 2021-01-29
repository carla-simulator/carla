---
title: Simulation/Basic Definition
permalink: /Simulation/Basic_Definition/
---

In the following, the inputs needed by the simulation modules
[sumo](../sumo.md) and [sumo-gui](../sumo-gui.md) are
described.

# Input Files

## Road Network

For a simulation, a [SUMO Road
Network](../Networks/SUMO_Road_Networks.md) must be given using the
option **--net-file** {{DT_NET}} (or **-n** {{DT_NET}}). The network is normally built using
[netconvert](../netconvert.md) or
[netgenerate](../netgenerate.md).

## Traffic Demand (Routes)

The vehicles to simulate must be given. Their description normally
includes [vehicle types, vehicles, and vehicle
routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md).
Routes are normally given to the simulation modules using the option **--route-files** {{DT_Routes}}\[,{{DT_Routes}}]\*
(or **-r** {{DT_Routes}}\[,{{DT_Routes}}]\*). As you can see, you can use more than one route file within a
single simulation run.

The routes MUST be sorted. The reason is that we want to simulate large
road networks with up to millions of routes. Using a plain PC this is
only possible if you do not keep all routes in memory. All files given
as parameter to **--route-files** {{DT_Routes}}\[,{{DT_Routes}}]\* are read step-wise. Starting at the begin time step,
new routes are loaded every n time steps for the next n time steps. n
may be controlled using the **--route-steps** {{DT_INT}} where <=0 forces
[sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) to load the file
completely. Fetching routes for the next steps only implies that the
vehicle types - or maybe "global" routes - must be given in prior to the
routes that use them.

You may also give routes including vehicle definitions as {{AdditionalFile}} to
[sumo](../sumo.md)/[sumo-gui](../sumo-gui.md).

## Additional Files

One ore more {{AdditionalFile}}(s) are used to load additional entities:

- infrastructure related things: [traffic light
  programs](../Simulation/Traffic_Lights.md), [induction
  loops](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
  and [bus stops](../Simulation/Public_Transport.md)
- additional visualization:
  [POIs](../Simulation/Shapes.md#poi_point_of_interest_definitions)
  and [polygons](../Simulation/Shapes.md#polygon_definitions)
  (i.e. rivers and houses)
- dynamic simulation control structures: [variable speed
  signs](../Simulation/Variable_Speed_Signs.md) and
  [rerouters](../Simulation/Rerouter.md)
- demand related entities: [vehicle
  types](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
  and
  [routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes)

All these additional structures / definitions are given to the
simulation using the **--additional-files** {{DT_FILE}}\[,{{DT_FILE}}]\*. The list of given files is processed
incrementally, this means each file is read completely from top to
bottom and the list is processed starting with the first file of the
list.

## Parsing Order

To ensure the correct resolution of references, it is important to know
what is loaded when. The order is as follows:

1.  the network is read
2.  the additional files are read (completely from top to bottom) in the
    order in which they are given in the option
3.  the route files are opened and the first n steps are read
4.  each n time steps, the routes for the next n time steps are read

# Defining the Time Period to Simulate

Each simulation requires the definition about the time period to be
simulated. This is given to [sumo](../sumo.md) or
[sumo-gui](../sumo-gui.md) using the options **--begin** {{DT_TIME}} (or **-b** {{DT_TIME}} for short) and **--end** {{DT_TIME}}
(**-e** {{DT_TIME}}). Please note that whether the option **--end** was given influences the
simulation's behavior. The details are described below.

The simulation starts at the time given in **--begin**, which defaults to 0. All
vehicles with a departure time (`depart`)
lower than the begin time are discarded.

The simulation performs each time step one-by-one.

The simulation ends in the following cases:

- The final time step was given using **--end** and this time step was reached
  (time after a step is \>= end).
- No value for **--end** has been given and all vehicles have been simulated.
  The state of the simulation is the one in which the last vehicle has
  left the simulated area. If a [TraCI](../TraCI.md) connection
  is active, the simulation will continue even after the last vehicle
  (potentially "forever").
- A close command has been received via [TraCI](../TraCI.md)

# Defining the Time Step Length

[sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) use a time step
of one second per default. You may override this using the **--step-length** {{DT_TIME}} option. {{DT_TIME}} is
here given in seconds, but you may enter a real number. In conclusion
giving **--step-length 0.01** will run the simulation using time steps of 10ms.

Specifying the option **--default.action-step-length** {{DT_TIME}} implies that vehicles perform calculations for
the adaption of accelerations or lane-change maneuvers only at intervals
of the given length and not within every simulations step (which is the
default). Using an action-step-length different from the simulation step
length automatically switches the [integration
method](#defining_the_integration_method) to 'ballistic'.

Choosing the action step length larger than the simulation step length
may speed up the simulation, but note that a value larger than the
assumed reaction time tau may induce collisions. See the description of
[vehicle
type](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
and
[car-following](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#car-following_models)
parameters for details.

!!! note
    Please note that the simulation step-length sets a lower bound on driver reaction times. Thus using values > 1 is generally discouraged.

# Defining the Integration Method

There are two numerical integration methods available, which control the
dynamical update of the simulation. Currently, the default is an Euler
update, which considers the vehicle's speed constant during one time
step. As an alternative, SUMO offers the ballistic update, which
considers the acceleration constant during one time step. It can be
activated by giving the option **--step-method.ballistic** or including the following into the
configuration file:

```
<processing>
   <step-method.ballistic value="true"/>
</processing>
```

Especially for larger time-steps (e.g. 1sec.) ballistic update yields
more realistic dynamics for car-following models based on continuous
dynamics. See also [*Martin Treiber, Venkatesan Kanagaraj*, Comparing
Numerical Integration Schemes for Time-Continuous Car-Following
Models](http://arxiv.org/abs/1403.4881).

Ballistic update causes positions to be updated with the average speed
between time steps instead of the speed of the current time step.
Consider this example with step-length 1s:

Vehicle at position x drives with speed 4.5m/s in step t and brakes to
speed 0 in step t+1.

- Euler-update: Vehicle is stopped at position x in step t+1 (instant
  deceleration)
- Ballistic-update: Vehicle is stopped at position x + 2.25 in step
  t+1 (continuous deceleration)

!!! caution
    The ballistic update is currently not working well in combination with the sublane-model. See #2581