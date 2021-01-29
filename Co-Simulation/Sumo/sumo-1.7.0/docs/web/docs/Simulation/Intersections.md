---
title: Simulation/Intersections
permalink: /Simulation/Intersections/
---

The vehicle dynamics at intersection are governed by the intersection
model described in
[\[Road Intersection Model in SUMO, Krajzewicz et al\]](http://elib.dlr.de/93669/1/LNCS_SUMOIntersections.pdf). Of course, the model in
the latest revision may deviate somewhat due to the ongoing evolution of
the model. This page gives an overview over the configuration options
governing the intersection model.

# Internal links

The most important configuration aspect is whether the dynamics within
an intersection shall be modelled or not. This is configured using the
following options. When simulating without internal links, 
Vehicles  be subject to right-of-way rules (waiting at traffic lights and at minor roads)
but they will appear instantly on the other side of the intersection after passing the stop line.
They cannot block the intersection, wait within the intersection for left turns nor collide on the intersection.

## [netconvert](../netconvert.md)-option **--no-internal-links**

When set to **true**, the network will not contain *internal lanes*,
that is lanes within an intersection. Instead, vehicles will *jump*
across the intersection. To avoid a systematic shortening of route lengths,
the length of all edges is [set artificially to the length](Distances.md)
between junction centers without changing their appearance. When set to
**false** (the default), The network will contain lanes within
intersections on which vehicles will drive just as on normal lanes,
albeit subject to some blocking constraints.

## [sumo](../sumo.md)-option **--no-internal-links**

When set to **true**, lanes within intersections are ignored. This
option is not needed when the network does not contain them in the first
place. Note, that route lengths will be wrong when ignoring internal
lanes in a network that includes them.

# Speed when approaching the intersection

Vehicles that approach an intersection distinguish 3 cases

- neither crossing nor merging takes place or the vehicle has
  right-of-way (linkState *M*,*G* or *O* ): vehicles drive across the
  intersection without slowdown
- a zipper merge must be performed (only at junction of type
  type="zipper", linkState *Z*). Speed depends directly on neighboring
  traffic (braking only takes place on demand)
- interference with other streams of traffic is possible
  (merging/crossing) and vehicle does not have right of way: vehicle
  prepares to break until reaching a specific distance (configurable
  for each location with connection attribute *visibilityDistance*).
  At that distance, the vehicle determines whether higher-prioritized
  traffic presents a conflict and then either brakes or accelerates
  across the intersection.

In each of these cases, the vehicle may also slow down if another
vehicle is still on the intersection and in the way.

# Waiting before the Intersection

Vehicles wait with a context dependent offset before the end of their
lane where it meets the intersection. The exact position depends on
several factors explained below.

## Lane Shape

Usually the lane ends exactly where the intersection shape starts. One
way to customize the exact position for each lane is to [edit the intersection shape](../netedit.md#junction). An alternative method
is to [set a custom endpoint for an edge](../netedit.md#specifying_the_complete_geometry_of_an_edge_including_endpoints).

Another possibility is the set the *endOffset* attribute for either the `<edge>`
or `<lane>` element. This will shorten the edge/lane by a set amount.

!!! note
    using *endOffset* currently does not affect junction shape so there will be a visible gap between the end of the lane and the start of the junction shape. The shape of internal lanes will be lengthened to compensate for this gap.

## Right-Of-Way

Vehicles from a minor road wait exactly at the end of the lane in order
to minimize the distance they have to cover when a suitable gap in
traffic is found. Vehicles waiting at at a traffic light wait with an
offset of 1m ahead of the lane end.

## Vehicle Class

!!! caution
    Vehicle class specific offsets can be created by using the `<stopOffset>` element but this is still experimental.

# Waiting Within the Intersection

In some situations, drivers are permitted to wait within the
intersection for a gap in foe traffic. Typical cases are left-turning
vehicles from the prioritized direction that wait for oncoming traffic
or right-turning vehicles that have to wait for straight-going
pedestrians. In SUMO this is modeled with [internal junctions](../Networks/SUMO_Road_Networks.md#internal_junctions).
An internal junction splits the internal lane in two parts. Upon
reaching the intersection, drivers may enter the intersection and drive
up to the end of the first part despite approaching foe vehicles. They
are not permitted to enter the intersection if it is blocked by vehicles
already on the intersection or if there is a red traffic light.

When building networks, [netconvert](../netconvert.md)
automatically recognizes common cases for waiting within the
intersection and creates internal junctions as necessary. At
intersections which are controlled by a traffic light, internal
junctions are built for every stream that has a *green minor* phase
(dark green). Thus, loading custom traffic light plans during network
building may influence the building of internal junctions.

Since version 0.25.0 the user also has the option for [customizing the presence and location of internal junctions](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which).

# Speed while passing the Intersection

All the [rules for vehicle speed](VehicleSpeed.md)
are applicable but there is an additioal feature to model speed
reductions while turning. Since SUMO version 1.0, lane speeds within the
intersection are reduced in accordance to their turning radius. This
means, that vehicles will reduce their speed while turning according to
their [individual speedFactor](VehicleSpeed.md#edge.2flane_speed_and_speedfactor).

The speedLimit is computed as

```
speedLimit = sqrt(radius * factor)
```

where *factor* is set by **--junctions.limit-turn-speed** {{DT_FLOAT}} (default 5.5) If factor is set to 0, the
turning limit is not applied.

Additionally, the computed speed will never be higher than the average
speed of source and destination lane for that particular connection.

The speed can be overruled by [setting the connection speed explicitly in the XML input](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which)
or via [netedit](../netedit.md#inspect).

# Junction Blocking

In most jurisdictions, drivers are forbidden to enter an intersection if
the outbound road is jammed to prevent them from blocking the
intersection. By default, vehicles in SUMO try to prevent blocking
intersections. This is accomplished by the **no-block-heuristic** which
prevents them from driving onto the intersection if they are likely to
become stuck there. This heuristic may be disabled by modifying the
simulation network ahead of the simulation.

!!! note
    The **no-block-heuristic** does not apply to *geometry-like* nodes (nodes without intersecting streams of traffic)

!!! note
    Junction blocking cannot occur when using option **--no-internal-links**

## [netconvert](../netconvert.md) options for allowing drivers to drive onto an intersections

- setting option **--default.junctions.keep-clear *false* ** (default *true*) will cause the
  **no-block-heuristic** to be disabled for all intersections.
- setting [`<node>`-attribute `keepClear="false"`](../Networks/PlainXML.md#node_descriptions)
- setting [`<connection>`-attribute `keepClear="false"\>`](../Networks/PlainXML.md#connection_descriptions)
    will cause the **no-block-heuristic** to be disabled for vehicles
  entering the intersection via that connection

## Junction model attributes for allowing drivers to drive onto an intersection

By setting the junction model parameter *jmIgnoreKeepClearTime* in a
`<vType>`-definition, drivers of that type will ignore the **no-block-heuristic**
after their accumulated waiting time exceeds the parameter value (in
seconds).

## Ignoring blocking vehicles after some time

When vehicles in SUMO are unable to move for some time they will be
[teleported](../Simulation/Why_Vehicles_are_teleporting.md#waiting_too_long.2c_aka_grid-locks)
to resolve dead-lock. If this is not desired,
[sumo](../sumo.md)-option **--ignore-junction-blocker** {{DT_TIME}} may be used to ignore vehicles which are
blocking the intersection on an intersecting lane after the specified
time. This can be used to model the real-life behavior of eventually
finding a way around the offending vehicle that is blocking the
intersection.

# Customizing Behavior at Junctions

## TraCI

The speed of a vehicle may be set using the [speed command](../TraCI/Change_Vehicle_State.md#command_0xc4:_change_vehicle_state).
When combined with [speed mode command](../TraCI/Change_Vehicle_State.md#speed_mode_.280xb3.29)
various safety-related rules can be disabled. Among them are flags for

- overriding save speed in regard to leader vehicles, or vehicles already on the intersection
- ignoring right-of-way rules
- ignore red lights

## Driver Parameters

The behavior at intersections can be configured with [junction model parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters).
The following aspects can be affected

- aggressiveness when merging from a unprioritized road (*impatience*)
- distance-keeping to pedestrians
- driving onto an intersection despite of
[\#Junction_Blocking](#junction_blocking) rules
- ignoring red lights
  - speed when ignoring red lights
