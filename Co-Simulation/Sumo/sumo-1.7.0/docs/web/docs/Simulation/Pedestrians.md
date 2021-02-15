---
title: Simulation/Pedestrians
permalink: /Simulation/Pedestrians/
---

# Pedestrian Simulation

This page describes simulations of pedestrians in SUMO. Pedestrians are
[persons that walk](../Specification/Persons.md). To build an
intermodal simulation scenario with proper interactions between road
vehicles and pedestrians, additional steps have to be take in comparison
to a plain vehicular simulation. When using multiple modes all edges and
lanes need to have the [correct permissions to separate the different modes of traffic](../Simulation/VehiclePermissions.md).

Edges that allow only pedestrian traffic are called **footpaths** in the
following. For edges that allow both modes, typically the left lanes
disallow pedestrians and the rightmost lane allows only pedestrian. The
rightmost lane is then called a **sidewalk**.

# Building a network for pedestrian simulation

When walking along an edge, pedestrians use sidewalks where available. A
sidewalks is a lane which allows only the sumo `vClass` *pedestrian*. When
crossing a road at an intersection, pedestrians use special lanes of
type *crossing*. The area that connects sidewalks with crossings is
modeled by special lanes of the type *walkingarea*. In the following, we
describe how to build a simulation network that *contains sidewalks*,
*crossings* and *walkingareas*.

When performing pedestrian routing, the router distinguishes between
networks that contain *walkingareas* and those which do not. In the
former, pedestrians may only cross a street whenever there is a
pedestrian crossing. In the latter they may "jump" between any two edges
which allow pedestrians at an intersection.

!!! note
    Almost all of the methods described below can be used for building a pedestrian network either based on an existing *.net.xml* file or while doing the initial import (i.e. from [OSM](../Networks/Import/OpenStreetMap.md)). The exception is [#Type-base_generation](#type-base_generation) which can only be done during import.

## Generating a network with sidewalks

A sidewalk is a lane which only permits the vClass *pedestrian*. There
are various different options for generating a network with sidewalks
which are explained below. All of these options recognize the presence
of an existing sidewalk and will not add another lane in that case.

!!! caution
    The current pedestrian models assume that each simulation edge has at most one sidewalk. In order to have sidewalks at both sides of a one-way street, a simulation edge in the reverse direction (which only allows pedestrians) must be added.

!!! note
    The methods described below, perform checks to prevent generating duplicate sidewalks. If the rightmost lane is recognized as a sidewalk (it only permits vClass *pedestrian*) no additional sidewalk will be added.

### Explicit specification of additional lanes

Sidewalks may be defined explicitly in plain XML input when describing
[edges (plain.edg.xml)](../Networks/PlainXML.md#lane-specific_definitions).
This is done by defining an additional lane which only permits the
vClass “pedestrian” and setting the appropriate width. In this case it
is important to disallow pedestrians on all other lanes. Also, any
pre-existing connection definitions must be modified to account for the
new sidewalk lane.

### Explicit specification of sidewalks

Alternatively to the above method, the `<edge>`-attribute
[sidewalkWidth may be used](../Networks/PlainXML.md#edge_descriptions).
It will cause a sidewalk of the specified width to be added to that
edge, connections to be remapped and pedestrian permissions to be
removed from all other lanes.

!!! note
    The heuristic methods described below, also perform automatic connection shifting and removal of pedestrian permissions from non-sidewalk lanes.

### Type-base generation

When importing edges with defined types, it is also possible to declare
that certain types should receive a sidewalk. This can be used to
automatically generate sidewalks for residential streets while omitting
them for motorways when importing OSM data. An example type file can be
found in [{{SUMO}}/data/typemap/osmNetconvertPedestrians.typ.xml]({{Source}}data/typemap/osmNetconvertPedestrians.typ.xml).

```
<types>
  <type id="highway.motorway" numLanes="3" speed="44.44" priority="13" oneway="true" disallow="pedestrian bicycle"/>
  <type id="highway.unclassified"   numLanes="1" speed="13.89" priority="5" sidewalkWidth="2" disallow="pedestrian"/>
  <type id="highway.residential"    numLanes="1" speed="13.89" priority="4" sidewalkWidth="2" disallow="pedestrian"/>
  <type id="highway.living_street"  numLanes="1" speed="2.78"  priority="3"/>
  <type id="highway.service"        numLanes="1" speed="5.56"  priority="2" allow="delivery pedestrian"/>
  ...
</types>
```

### Heuristic generation

A third option which can be used if no edge types are available is a
heuristic based on edge speed. It adds a sidewalk for all edges within a
given speed range. This is controlled by using the following [netconvert](../netconvert.md) options:

| Option                                 | Description                                                                       |
|----------------------------------------|-----------------------------------------------------------------------------------|
| **--sidewalks.guess** {{DT_BOOL}}               | Guess pedestrian sidewalks based on edge speed                                    |
| **--sidewalks.guess.max-speed** {{DT_FLOAT}}    | Add sidewalks for edges with a speed equal or below the given limit *default:13.89* |
| **--sidewalks.guess.min-speed** {{DT_FLOAT}}    | Add sidewalks for edges with a speed above the given limit *default:5.8*            |
| **--sidewalks.guess.exclude** {{DT_IDList}}[,{{DT_IDList}}\]\* | Specify a list of edges that shall not receive a sidewalk                         |

### Permission-based generation

Option **--sidewalks.guess.from-permissons** {{DT_BOOL}} is suitable for networks which specify their edge permissions
(such as [DlrNavteq](../Networks/Import/DlrNavteq.md)). It adds a
sidewalk for all edges which allow pedestrians on any of their lanes.
The option **--sidewalks.guess.exclude** {{DT_IDList}}[,{{DT_IDList}}\]\* applies here as well.

### Adding sidewalks with [netedit](../netedit.md)

To add sidewalks to a set of edges in [netedit](../netedit.md)
select these and right click on them. From the context-menu select *lane operations->add restricted lane->Sidewalks*.

### Non-exclusive sidewalks

The following rules are used to determine the sidewalk lane in case of
ambiguities:

- if there are multiple lanes with permissions *pedestrian* the
  rightmost one is used
- if there are lanes that allow only pedestrians and lanes that allow
  pedestrians along with other vehicle classes, the rightmost lane
  that **only** allows pedestrians is used
- if there are no lanes that allow only pedestrians but some lanes
  that allow pedestrians along with other vehicle classes the
  rightmost of those lanes is used

## Generating a network with crossings and walkingareas

In order to be able to cross a street at all, crossing places must
defined at junctions in the network. These places model the following
three situations:

- crossing the street with priority (zebra crossing)
- crossing the street without priority
- crossing the street at a traffic light

In order to be eligible for a crossing there must be sidewalks or
footpaths which are separated by one or more lanes which forbid
pedestrian traffic.

!!! note
    If crossing should be possible between two far-away junctions, additional junctions must be inserted by splitting the edge(s) between the junctions and defining a crossing at the inserted junctions.

Crossings may be defined explicitly in plain XML input when describing
[connections (plain.con.xml) using the XML element `crossings`](../Networks/PlainXML.md#pedestrian_crossings).
They can also be placed with [netedit](../netedit.md#crossings).

The second available method for adding crossing information to a network
is with the [netconvert](../netconvert.md) option **--crossings.guess** {{DT_BOOL}}. This enables a heuristic which adds crossings
wherever sidewalks with similar angle are separated by lanes which
forbid pedestrians. If the edges to be crossed have sufficient distance
between them or vary a by a sufficient angle, two crossings with an
intermediate walking area are generated. For roads with a speed limit
beyond 50km/h, crossings are only generated at tls-controlled
intersections. This threshold can be set with the option **crossings.guess.speed-threshold**. To
successfully generate crossings with this option [sidewalks should be defined](../Simulation/Pedestrians.md#generating_a_network_with_sidewalks)
for the network.

!!! caution
    To ensure proper generation of crossings, road lanes need to prohibit pedestrians either by setting `disallow="pedestrian"` or by explicitly specifying all other allowed classes using attribute `allow` When adding sidewalks via attribute `sidewalkWidth` or any of the heuristics above, pedestrians will be forbidden automatically on the remaining lanes.

### walkingareas

At every junction corner multiple sidewalks footpaths and pedestrian
crossings may meet. The connectivity between these elements is modeled
with a *walkingarea*. These walkingareas are generated automatically as
long as the [netconvert](../netconvert.md) option **--crossings.guess** is set or there is at least one user-defined
crossing in the network. If no crossings are wanted in the network it is
also possible to enable the creation of walkingareas by setting the
option **--walkingareas**.

!!! note
    It is not necessary to define connections between sidewalks or footpaths in a *.con.xml* file. Instead the connections to and from walkingareas are generated automatically along with the walkingareas.

Example: If multiple footpaths meet at an intersection they will be
automatically connected by a single walkingarea which allows
connectivity in all directions.

!!! caution
    If pedestrians are simulated in a network without walkingareas, they will assume full-connectivity at every junction.

# Generating pedestrian demand

## Explicit

Pedestrian demand may be specified explicitly as described at
[Specification/Persons\#Walks](../Specification/Persons.md#walks)
or it may be generated. Since version 1.2 [pedestrian flows](../Specification/Persons.md#repeated_persons_personflows)
are directly supported. In older versions, the tool
{{SUMO}}**/tools/pedestrianFlow.py** may also be used.

## Random

The tool
[Tools/Trip\#randomTrips.py](../Tools/Trip.md#randomtripspy)
supports generating random pedestrian demand using the option **--pedestrians**. The
option **--max-dist** {{DT_FLOAT}} may be used to limit the maximum air distance of pedestrian
walks.

Pedestrian demand may also arise from [intermodal routing](../IntermodalRouting.md). One way to obtain such a
scenario is with
[Tools/Trip\#randomTrips.py](../Tools/Trip.md#randomtripspy) using
the option **--persontrips**.

## OD-Based

The [od2trips](../od2trips.md) application generates pedestrian
demand when setting the option **--pedestrians**. Alternative it generates intermodal
demand by setting option **--persontrips**.

# Pedestrian Models

The pedestrian model to use can be selected by using the simulation
option **--pedestrian.model** {{DT_STR}} with the available paramters being *nonInteracting* and
*striping* (default is *striping*). The interface between the pedestrian
model and the rest of the simulation was designed with the aim of having
a high degree of freedom when implementing new models. It is planned to
implement models with a higher level of interaction detail in the
future.

## Model *nonInteraction*

This is a very basic walking model. Pedestrians walk bidirectionally
along normal edges and “jump” across intersections. They maybe either be
configured to complete a walk in a fixed amount of time or to move along
the edges with a fixed speed. No interaction between pedestrians and
vehicles or other pedestrians takes place. This model has a very high
execution speed and may be useful if the pedestrian dynamics are not
important.

## Model *striping*

This model assigns 2D-coordinates within a lane (of type sidewalk,
walkingarea or crossing) to each pedestrian. These coordinates which are
defined relative to the leftmost side of the start of the lane are
updated in every simulation step. This is in contrast to the coordinates
of vehicles, which (generally) only have 1D-coordinates within their
respective lane (except the case that the [sublane model](../Simulation/SublaneModel.md) is used). Pedestrians advance
along a lane towards the next node which may either correspond to the
natural direction of the lane (forward movement) or it may opposite to
the natural direction (backward movement). Thus, the x coordinate
monotonically increases or decreases while on a lane. Once the end of a
lane has been reached, the pedestrian is placed on the next lane (which
may either be unique or determined dynamically with a routing
algorithm).

The most important feature of pedestrian interactions is collision
avoidance. To achieve this, the “striping”-model divides the lateral
width of a lane into discrete stripes of fixed width. This width is user
configurable using the option **--pedestrian.striping.stripe-width** {{DT_FLOAT}} and defaults to 0.65 m. These stripes are
similar to lanes of a multi-lane road. Collision avoidance is thus
reduced to maintaining sufficient distance within the same stripe.
Whenever a pedestrian comes too close to another pedestrian within the
same stripe it moves in the y-direction (laterally) as well as in the
x-direction to change to a different stripe. The y-coordinate changes
continuously which leads to situations in which a pedestrian temporarily
occupies two stripes and thus needs to ensure sufficient distances in
both. The algorithm for selecting the preferred stripe is based on the
direction of movement (preferring evasion to the right for oncoming
pedestrians) and the expected distance the pedestrian will be able to
walk in that stripe without a collision.

During every simulation step, each pedestrian advances as fast as
possible while still avoiding collisions. The updates happen in a single
pass for each walking direction with the pedestrian in the front being
updated first and then its followers sorted by their x-coordinate. The
speed in the x-direction may be reduced by a random amount with the
maximum amount defined as a fraction of the maximum speed, using the
global option **--pedestrian.striping.dawdling** {{DT_FLOAT}} (defaulting to 0.2). As a consequence of the above
movement rules, pedestrians tend to walk side by side on sidewalks of
sufficient width. They wait in front of crossings in a wide queue and
they form a jam if the inflow into a lane is larger than its outflow.

More complicated movement rules apply when moving on a *walkingarea*.
Here, pedestrians paths cross in multiple directions. The actual path
taken across the walkingarea consists of 1-3 linear segments and is
unique for each pair of adjacent sidewalks or crossings. The pedestrians
on each path of these paths compute their movements as if they were on a
sidewalk. However, all other pedestrians are mapped into the coordinate
system of that path in order achieve collision avoidance.

# Pedestrian Routing

If the user does not specify a complete list of edges for walking,
pedestrian will be routed according to the fastest path along the road
edges. This is usually also the shortest path since edges typically have
no speed restraints that are relevant to pedestrians. This type of
routing may happen before the simulation starts (using
[duarouter](../duarouter.md)) or it may be done at insertion time
when loading only origin and destination into the simulation. Routes may
later be influenced via [TraCI](../TraCI/Change_Person_State.md).

When using the *striping*-model, pedestrians will also be routed within
intersections to selected a sequence of *walkingareas* and *crossings*
for moving onto the other side of the intersection. This type of routing
takes the shortest part but avoids red lights if possible. The routing
behavior within intersections cannot currently be influenced.

# Interaction between pedestrians and other modes

A pedestrian wishing to cross the street at an uncontrolled intersection
can only do so if its expected time slot for using the intersection does
not interfere with that of an approaching vehicle. It should be noted
that the dynamics at unprioritized crossings are conservative in
estimating the time required gap. In the simulation, pedestrians will
only use such a crossing if the whole length of the crossing is free of
vehicles for the whole time needed to cross. At priority crossings,
pedestrians cross without regard for vehicles.

Vehicles are prevented from driving across a pedestrian crossing which
is occupied by pedestrians. If a pedestrian is found which is not yet
past the intersection point (between the crossing and the vehicles
trajectory) but within a threshold distance to that point (configurable
via [junction parameter jmCrossingGap](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters))
the crossing is considered to be blocked.

When vehicles and pedestrians move on the same lane, they will do their
best to avoid a collision. If a vehicle occupies the whole width of the
lane and gets too close to a pedestrian, the pedestrian may briefly move
to the side of the lane in order to let the vehicle pass.

!!! note
    Using the model 'nonInteracting', no interactions between pedestrians and other modes take place.

# Pedestrian Outputs

Pedestrians are included in the following outputs:

- [tripinfo-output](../Simulation/Output/TripInfo.md)
- [vehroute-output](../Simulation/Output/VehRoutes.md)
- [fcd-output](../Simulation/Output/FCDOutput.md)
- [netstate-dump](../Simulation/Output/RawDump.md)
- [aggregated simulation statistics](../Simulation/Output.md#aggregated_traffic_measures)