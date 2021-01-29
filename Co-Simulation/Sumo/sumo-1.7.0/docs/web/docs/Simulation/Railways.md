---
title: Simulation/Railways
permalink: /Simulation/Railways/
---

# Train Simulation

This page describes simulations of trains in SUMO. To build an
intermodal simulation scenario with trains, additional steps have to be
taken in comparison to a plain vehicular simulation.

# Building a network for train simulation

## Railways

Railways can be imported from
[OSM](../Networks/Import/OpenStreetMap.md). They can also be
explicitly specified using the existing *vClasses*.

### OSM

#### Types

When importing from OSM the following railway types are distinguished by
default (via [{{SUMO}}/data/typemap/osmNetconvert.typ.xml]({{Source}}data/typemap/osmNetconvert.typ.xml)):

- tram
- subway
- light_rail
- rail
- [highspeed](https://en.wikipedia.org/wiki/High-speed_rail)

#### Electrification

Whenever a railway is electrified, the vClasses *rail*, *rail_electric*
and *rail_fast* are permitted to drive there. Otherwise, only vClass
*rail* is allowed.

#### Usage

By loading the additional typemap [{{SUMO}}/data/typemap/osmNetconvertRailUsage.typ.xml]({{Source}}data/typemap/osmNetconvertRailUsage.typ.xml), additional [usage
information](https://wiki.openstreetmap.org/wiki/Key:usage#With_railways)
is exported for the edge types:

- main
- branch
- industrial
- military
- test
- tourism

This will lead to compound edge type ids such as
*railway.rail|usage.main*.

#### Track numbers

Local track numbers (mostly in train stations) are exported as edge
parameter *track_ref*. These values are shown in the edge parameter
dialog and can also be used for coloring (*color by param, streetwise*).

## Bidirectional track usage

In reality all tracks can be used in either direction if the need arises
but many rails are used in only one direction most of the time. In SUMO,
bidirectional track usage must be enabled explicitly. This simplifies
routing as rails will only be used in their preferred direction most of
the time.

Bidirectional track usage is modeled by two edges that have their
geometries exactly reversed and using the attribute `spreadType="center"`. This will result
in lane geometries that are overlayed exactly. These edges are referred
to as *superposed* (alternatively as bidirectional rail edges). In the
.net.xml file these edges are marked with `bidi="<REVERSE_EDGE_ID>"` but this is a generated
attribute and not to be set by the user.

When Rail signals are placed at both ends of a bidirectional track they
will restrict it's usage to one direction at a time.

### Bidirectional rails in [sumo-gui](../sumo-gui.md)

sumo-gui automatically shows only one of both edges to avoid duplicate
drawing of cross-ties. The visualisation option *show lane direction*
can be used to identify superposed edges. (arrows in both directions
will be show).

### Working with bidirectional tracks in [netedit](../netedit.md)

- To show both edges that constitute a bidirectional track, activate
  edge visualisation option *spread superposed*. Both edges will be
  drawn narrower and with a side-offset to make them both visible
  without overlap.
- To find (and highlight) all bidirectional tracks, use [attribute
  selection](../netedit.md#match_attribute) and search for
  attribute *bidi* with a value of *1*
- Create bidirectional tracks [as explained
  here](../netedit.md#creating_bidirectional_railway_tracks)

### Importing bidirectional tracks from OSM

When importing networks from
[OSM](../Networks/Import/OpenStreetMap.md), rails tagged with `railway:preferred_direction:both` are
automatically imported as superposed edges.

### Handling Problems in bidirectional railway networks

Commonly, rail networks import from OSM are incomplete in regard to
bidirectional track usage. One example would be terminal tracks which a
train can only leave by reversing direction. A large number of these
issues can be fixed automatically be setting the
[netconvert](../netconvert.md)-option **--railway.topology.repair**. To analyze problems with
bidirectional tracks, the option **--railway.topology.output** {{DT_FILE}} can be used to identify problematic
tracks. The option **--railway.topology.all-bidi** can be used to make all tracks usable in both
directions.

## Rail Signals

The [node type](../Networks/PlainXML.md#node_descriptions)
`rail_signal` may be used to define signals which implement [Automatic Block
Signaling](https://en.wikipedia.org/wiki/Automatic_block_signaling).

By setting the [netconvert](../netconvert.md)-option **--railway.signals.discard** all signals
can be removed from a network.

## Rail Crossings

The [node type](../Networks/PlainXML.md#node_descriptions)
`rail_crossing` may be used to define railway crossings. At these nodes trains will
always have the right of way and road vehicles get a red light until
there is a safe gap between approaching trains.

When importing networks from
[OpenStreetMap](../Networks/Import/OpenStreetMap.md), rail
crossings will be imported automatically. For other input data sources
the crossings may have to be specified [via additional xml
files](../Tutorials/ScenarioGuide.md#modifying_an_imported_network_via_plainxml)
or set via [netedit](../netedit.md) after importing.

## Kilometrage (Mileage, Chainage)

Edges support the attribute *distance* to denote the distance at the
start of the edge relative to some point of reference for a [linear
referencing scheme](https://en.wikipedia.org/wiki/Linear_referencing).
When the distance metric decreases along the forward direction of the edge, this is indicated by using a negative sign for the distance value.

The distance value along an edge is computed as: 
```
  |edgeDistance + vehiclePos|
```

Edge distance is imported from OSM and can also be be set along a route in [netedit](../netedit.md#route)

!!! note
    Negative distance values are not currently supported (pending introduction of another attribute)

# Routing on Bidirectional Tracks
When train tracks can be used in both directions, there is considerable freedom for trains when search a path through the network. To reduce the number of conflicts (when two vehicles want to use the same track in opposite directions), the preferred direction for each track can be defined and factored into the routing decision.

When routes are computed in the simulation, this is done by setting the option **--device.rerouting.priority-factor FLOAT**. This causes the priority values of edges to be factored into the routing decision with higher values being preferred. 
At the default value of 0. Edge priority is ignored when routing. When setting a positive value, the edges with the lowest priority receive a penalty factor to their estimated travel time of 1 + FLOAT whereas the edges with the highest priority receive no penalty. Edges with medium priority will receive a penality of 1 + x * FLOAT where 

```
  x = (edgePriority - minPriority) / (maxPriority - minPriority)
```

The priority values can either be assigned by the user or computed heuristically by [netconvert](../netconvert.md) by setting the option **--railway.topology.direction-priority**. This requires that some of the tracks in the network are uni-directional (to unambiguously define the main direction). The assigned priority values are:

- 4: unidirectional track
- 3: main direction of bidirectional track
- 2: undetermined main direction (straight continuation from different directions of unidirectional track)
- 1: undetermined main direction (no continuation from unidirectional track)
- 0: reverse of main direction of bidirectional track

# Modelling Trains

There is a dedicated carFollowMode for trains which can be actived by
setting `carFollowModel="Rail" trainType="<TYPE>"` in the `<vType>` definition. Allowed values for trainType are

- Freight
- ICE1
- ICE3
- RB425
- RB628
- REDosto7
- NGT400
- NGT400_16

These types model traction and rolling resistance for particular trains.
Alternatively, any other car following model may be used and configured
with appropriate acceleration / deceleration parameters.

# Train Interaction

When simulation trains on a network with railway signals, trains will
only enter a block (a section of edges between signals) if it is free of
other trains. When there are no rail signals or multiple trains have
been inserted in the same block, they will automatically keep a safe
distance according to their car following model. When using `carFollowModel="Rail"`, trains
will always keep enough distance to the leading train to come to a safe
stop even if the lead train was to stop instantly.

# Reversing Direction

Trains will reverse direction if all of the following conditions are
met:

- The head of the train is on a normal edge (not on an intersection /
  railway switch)
- The whole length of the train is located on rail-edges that allow
  bidirectional use.
- The speed of the train is below 0.1m/s.
- The train does not have any further stops on the current edge
- The succeeding edges in the train's route are the reverse-direction
  edges of those it is currently on
- There is a "turn-around" connection from the current train edge to
  the reverse direction edge

  !!! note
      When importing public transport stops with option **--ptstop-output**, all bidirectional edges with a public transport stop will have the necessary turn-around connection and thus be eligible for reversing.

# Portion working
Trains can be split and joined (divided and coupled) at stops.

## Splitting a train
To split a train, the following input definition can be used. The rear half of the train is defined as a new vehicle which depart value **split**. The train train that is being split must define the 'split' attribute in its stop definition referencing the id of the rear half.
```
<vType id="train" vClass="rail"/>
    <vType id="splitTrain" vClass="rail" length="50"/>
    <trip id="t0" type="train" depart="0.00" from="a" to="c">
        <stop busStop="B" duration="60.00" split="t1"/>
    </trip>
    <trip id="t1" type="splitTrain" depart="split" departPos="last" from="b" to="e">
        <stop busStop="B" duration="60.00"/>
    </trip>
```
When defined this way, The rear part of the train will be created as a new simulation vehicle once the first part has reached the stop. After stopping, The front half of the train will continue with reduced length.

## Joining two trains
To join two trains, the following input definition can be used. The front half of the train must define a stop trigger with value **join**. The rear half othe train must define attribute 'join' referencing the id of the front half.

```
<vType id="train" vClass="rail"/>
    <vType id="splitTrain" vClass="rail" length="50"/>
    <trip id="t0" type="splitTrain" depart="0.00" from="a" to="c">
        <stop busStop="B" duration="60.00" triggered="join"/>
    </trip>
    <trip id="t1" type="splitTrain" depart="30" from="d" to="b">
        <stop busStop="B" duration="5.00" join="t0"/>
    </trip>
```
The rear part of the train will be joined to the front part if the followign conditions are met:
- the rear part has fulfilled its stopping duration
- the front part the train is present and it's back is on the same lane as the front of the rear part
- the gap between the trains is less than 5 meters
After being joined to the front part, the rear part will no longer be part of the simulation.
The front half of the train will stop until the rear part is joined to it. Afterwards it will continue with increased length. 

# TraCI

Rail signals and rail crossings can be controlled with function *traci.trafficlight.setRedYellowGreenState*. They can also be switched off with *traci.trafficlight.setProgram(tlsID, "off")*. In either case, normal operations can be resumed by reactivating the default program "0": *traci.trafficlight.setProgram(tlsID, "0")*.

Trains can be controlled just like cars by using the *traci.vehicle* functions. 
Furthermore the following functions are available for rail signals:

- traci.trafficlight.getBlockingVehicles(tlsID, linkIndex): Returns the list of vehicles that are blocking the subsequent block for the given tls-linkIndex from the perspective of the closest vehicle upstream of the signal
- traci.trafficlight.getRivalVehicles(tlsID, linkIndex): Returns the list of vehicles that also wish to enter the subsequent block for the given tls-linkIndex (regardless of priority) from the perspective of the closest vehicle upstream of the signal
- traci.trafficlight.getPriorityVehicles(tlsID, linkIndex): Returns the list of vehicles that also wish to enter the subsequent block for the given tls-linkIndex (only those with higher priority) from the perspective of the closest vehicle upstream of the signal

# Visualisation

The length of railway carriages, locomotive and the gap between the
carriages can be configured using the following [generic vType
parameters](../Simulation/GenericParameters.md):

- carriageLength
- locomotiveLength
- carriageGap

# Limitations

- Individual rail cars / coupling / uncoupling cannot currently be
  modeled
