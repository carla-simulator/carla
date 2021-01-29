---
title: Simulation/Routing
permalink: /Simulation/Routing/
---

# Features that cause rerouting

There are multiple simulation features that allow routing at simulation
time. They are described in the following:

## Routing triggered by the vehicle

This type of routing works by assigning a *rerouting device* to some or
all vehicles. Details are given at
[Demand/Automatic_Routing](../Demand/Automatic_Routing.md).

## Incomplete Trips and Flows

This is a special case of the above method. Vehicles with incomplete
routes automatically receive a *rerouting device* and are rerouted once
when entering the network. In some scenarios this is a practical
[*one-shot*-approach to route assignment](../Demand/Dynamic_User_Assignment.md#oneshot-assignment)
that avoids time-consuming [iterative assignment](../Demand/Dynamic_User_Assignment.md#general_behavior).

## Alternative Route Signage

This is a location based method for triggering rerouting and is
described at [Simulation/Rerouter](../Simulation/Rerouter.md).

## TraCI

Using the methods [*traci.vehicle.changeTarget* or *traci.vehicle.rerouteTraveltime*](../TraCI/Change_Vehicle_State.md)
rerouting is triggered for the specified vehicle.

Alternatively, routes can be computed using
[''traci.simulation.findRoute](../TraCI/Simulation_Value_Retrieval.md)
and applied using
[*traci.vehicle.setRoute*](../TraCI/Change_Vehicle_State.md).

For persons, the function
[''traci.simulation.findIntermodalRoute](../TraCI/Simulation_Value_Retrieval.md)
can be used to compute simple walks as well as [itineraries for public transport](../IntermodalRouting.md).

# Alternative Destinations

By using [`<rerouter>`-definitions](../Simulation/Rerouter.md), vehicles can be routed
to alternative destinations. A different method is to use [traffic assignment zones (TAZ)](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#traffic_assignement_zones_taz).
This allows vehicles to change their destination to the best alternative
from a list of potential destinations.

# Travel-time values for routing

By default, the route with the least travel time is chosen. The travel
time depends on the current routing mode (configurable via
*traci.vehicle.setRoutingMode*) or via the explicit *routingMode*
argument to *traci.simulation.findRoute*.

## Routing Mode *traci.constants.ROUTING_MODE_DEFAULT*

The following order of steps is taken to retrieve the travel time for each edge. If a step provides data, this is used, otherwise the next step is attempted: 

1.  The vehicle retrieves it's individual data storage. This can be set
    and retrieved using the TraCI vehicle methods [*change edge travel time information*](../TraCI/Change_Vehicle_State.md#change_edge_travel_time_information_0x58)
    and [*edge travel time information*](../TraCI/Vehicle_Value_Retrieval.md#edge_travel_time_information_0x58).
2.  The [global edge weights](../Demand/Shortest_or_Optimal_Path_Routing.md#custom_edge_weights)
    loaded using option **--weight-files** are retrieved.
3.  The global edge weights (set and retrieved via TraCI) using the
    TraCI edge methods [*change edge travel time information*](../TraCI/Change_Edge_State.md) and [*edge travel time information*](../TraCI/Edge_Value_Retrieval.md).
4.  The minimum travel time (length/allowedSpeed) is used.

!!! note
    The edge weights for cases 1-3 support modeling time-dependent variations in edge travel time. This means future changes in travel time along a vehicles route can be taking into account when routing.

## Routing Mode *traci.constants.ROUTING_MODE_AGGREGATED*

The [smoothed travel times](../Demand/Automatic_Routing.md#edge_weights) computed for
the *rerouting device* are used.

## Special cases

- When rerouting with the *rerouting device* the travel time always
  comes from another data storage which is updated continuously with a
  configurable averaging procedure. The [parameters for this updating strategy are user definable](../Demand/Automatic_Routing.md).
  It is also possible to [set the device travel time directly via TraCI](../TraCI/Change_Vehicle_State.md#supported_device_parameters).
- When using the TraCI method rerouteTraveltime from the [python TraCI library](../TraCI/Interfacing_TraCI_from_Python.md), the
  command supports an additional boolean parameter *currentTravelTime*
  (default *True*). When this parameter is set to *True*, the global
  edge weights are replaced by the currently measured travel times
  before rerouting. To replicate this behavior with other TraCI
  clients, all edges in the network must be called with *change global
  travel time information* using the value of *current travel time*.
  Note that the travel time values which are set in this way are used
  for the full duration of the simulation unless updated again.

# Routing by Traveltime and Edge Priority
Sometimes it is useful to guide route search with additional information while still taking travel times into account.
For this use case the option **--weights.priority-factor FLOAT** can be used with [sumo](../sumo.md) and [duarouter](../duarouter.md).

When this option is set, the priority value of each edge is factored into the routing decision so that low-priority edges receive a penalty (they appear to be slower) whereas high-priority edges receive little or no penalty. For the option value `PriorityFactor`, the penalty is computed thus:
```
  MinEdgePriority : minimum priority value of all edges
  MaxEdgePriority : maximum priority value of all edges
  EdgePriorityRange = MaxEdgePriority - MinEdgePriority

  relativeInversePrio = 1 - ((edgePriority - MinEdgePriority) / EdgePriorityRange)
  effort =  traveltime * (1 + relativeInversePrio * PriorityFactor)
```
As a consequence:

- the highest priority edge will get no penalty
- the traveltime of the lowest priority edge is multiplied with 1+PriorityFactor, 
- edges with in-between priorities will get a scaled penalty

# Routing by *effort*

By default, the objective of the routing algorithms is to minimize the travel time between origin and destination.
The traveltime can either be computed from the speed limits and vehicle maximum speed, it can be estimated at runtime from the simulation state or it can be loaded from a data file. The latter option allows defining travel times for the future.
An example for the relevance of future travel times would be this:
- a vehicle departs for a long trip at a time where there is no jamming
- it is known that parts of the network will be jammed later
- the route of the vehicle computed at departure time can circumvent the jam because the routing algorithm is aware that by the time those edges are reached they will be jammed.

It may be useful to compute routes which minimize some other criteria (called **effort**) besides travel time (distance, emissions, price, ...).
When these quantities are meant to change over time, the routing algorithm needs two kinds of values for each edge:

- the **effort** that shall be minimized
- the **travel time** for the edge.

The travel time is needed to compute at which time a certain edge is reached so that effors which change over time can be used correctly.

!!! note
    When the effort values do not change over time, routing by effort can be achieved by loading weight-files with a modified `traveltime` attribute (the effort value is written into the traveltime attribute) and the option **--weight-attribute** can be omitted.

When setting the options **--weight-file** and **--weight-attribute**, additional routing efforts are read
according to the specified attribute. These are only used when calling
the TraCI function [reroute by effort](../TraCI/Change_Vehicle_State.md). The assumed efforts
along a vehicles route are are time-based values and the time is
computed based on the travel time values described above. The effort can
also be set using *traci.edge.setEffort*.

!!! caution
    The default effort value is 0 which causes detour routes to be preferred when not loading sensible effort values.
    
The applications [duarouter](../duarouter.md) and [marouter](../marouter.md) also support the options **--weight-file** and **--weight-attribute** but they can only be used with one of the weight attributes "CO", "CO2", "PMx", "HC", "NOx", "fuel", "electricity", "noise". However, the will still work as expected when the user loads custom effort values for these attributes.


# Routing by *distance*

Finding the shortest route rather than the fastests can be achieved by
loading suitable *effort*-data (see
[\#Routing_by_effort](#routing_by_effort)) or by setting
the speed for all network edges to the same value.

A simpler solution is to define a vehicle type that travels with the
same speed on all edges:

```
<vType id="routeByDistance" maxSpeed="1"/>
```

and then using that type to find the fastest route:

```
stageResult = traci.simulation.findRoute(fromEdge, toEdge, "routeByDistance")
shortestDistance = stage.length
```

!!! note
    Older versions of SUMO do not supply the value *stage.length*. In this case **shortestDistance = stage.travelTime** is also correct due to the speed of 1m/s.

# Routing Algorithms

Applications that perform routing ([sumo](../sumo.md),
[sumo-gui](../sumo-gui.md), [duarouter](../duarouter.md),
[marouter](../marouter.md)) support the option **--routing.algorithm** for selecting among
the following values:

- *dijkstra*: (default)
[Dijkstras](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm)
algorithm is the simplest and slowest of routing algorithms. It is
well suited to routing in time-dependent networks (i.e. when the
travel time on an edge depends on the time of day)
- *astar*: The
[A\*](https://en.wikipedia.org/wiki/A*_search_algorithm) routing
algorithm uses a metric for bounding travel time to direct the
search and is often faster than dijkstra. Here, the metric *euclidean distance / maximumVehicleSpeed*) is used.
  - by using *astar* together with the option **--astar.landmark-distance** {{DT_FILE}} the ALT-Algorithm is
    activated (A\*, Landmarks, triangle inequality). It uses a
    precomputed distance table to selected network edges (so-called
    landmarks) to speed up the search, often by a significant
    factor. A lookup table can be generated by creating a file with one landmark edge id per line (e.g. landmarks.txt)
    and then setting the options **-astar.landmark-distances landmarks.txt --astar.save-landmark-distances lookuptable.txt**
  - by using *astar* together with the option **--astar.all-distances** {{DT_FILE}} the A\* algorithm is
    used together with a complete (and often huge) distance table to
    allow for blazing fast search.
- *CH*: [Contraction Hierarchies](https://en.wikipedia.org/wiki/Contraction_hierarchies)
is preprocessing-based routing algorithm. This is very efficient
when a large number of queries is expected. The algorithm does not
consider time-dependent weights. Instead, new preprocessing can be
performed for time-slices of fixed size by setting the option **--weight-period** {{DT_TIME}}. The
preprocessing is done without restrictions on vehicle class which
reduces efficiency in multi-modal networks.
- *CHWrapper*: This works like *CH* but performs separate
preprocessing for every vehicle class that is encountered, thereby
increasing routing efficiency.
