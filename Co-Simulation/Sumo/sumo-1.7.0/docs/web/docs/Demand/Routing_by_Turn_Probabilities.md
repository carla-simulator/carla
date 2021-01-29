---
title: Demand/Routing by Turn Probabilities
permalink: /Demand/Routing_by_Turn_Probabilities/
---

The [jtrrouter](../jtrrouter.md) is a routing applications which
uses flows and turning percentages at junctions as input. The following
parameter must be supplied:

- the network to route the vehicles through,
- the description of the turning ratios for the junctions (defaults
   may be used for this, too), and
- the descriptions of the flows.

A call may look like this:

```
jtrrouter --flow-files=<FLOW_DEFS> --turn-ratio-files=<TURN_DEFINITIONS> --net-file=<SUMO_NET> \
  --output-file=MySUMORoutes.rou.xml --begin <UINT> --end <UINT>
```

!!! note
    When turn-counts instead of turn-ratios are available, [other tools may be used](Routes_from_Observation_Points.md).

## Turn Definitions

| Turn and Sink Definitions | |
|--------------------|---------------------------|
| Filename extension | .turns.xml                |
| Type of content    | turn and sink definitions |
| Open format?       | Yes                       |
| SUMO specific?     | Yes                       |
| XML Schema         | [edgerelations_file.xsd](http://sumo.dlr.de/xsd/edgerelations_file.xsd)           |

To describe the turn definitions, one has to write an XML file. Within
this file, for each interval and each edge the list of percentages to
use a certain follower edge has to be given. An example:

```
<edgeRelations>
   <interval begin="0" end="3600">
      <edgeRelation from="myEdge0" to="myEdge1" probability="0.2"/>
      <edgeRelation from="myEdge0" to="myEdge1" probability="0.7"/>
      <edgeRelation from="myEdge0" to="myEdge1" probability="0.1"/>

      ... any other edges ...

   </interval>

   ... some further intervals ...

</edgeRelations>
```

!!! note
    If you provide turn definitions that overlap (for example in multiple files), the latter turn definition will override the former.

The snippet defines that vehicles coming at the end of edge
"`myEdge0`" within the time interval between
0s and 3600s will choose the edge "`myEdge1`"
with a probability of 20%, "`myEdge2`" with a
probability of 70% and "`myEdge3`" with a
probability of 10%. Another possibility to save time on preparing the
description is to use default values. The parameter **--turn-defaults (-T) <TURN_DEFAULTS\>** can be used to
describe the default ratios that will be used for all junctions for all
time steps. <TURN_DEFAULTS\> is a list of doubles, separated by a ','. To
achieve the same behaviour as in the example above, use **--turn-defaults 20,70,10**. The values
will be applied to an edge's following edges beginning at the right edge
(20%) and ending at the leftmost edge (10%). As the number of possible
followers changes for different edges, the values are resampled for
edges which number of following edges differs from the number of given
turning probability defaults. Given **--turn-defaults 20,70,10** a vehicle using an edge that has
two followers would use the follower to the right with 55% probability,
the one to the left with 45%.

### Automatic generation of turn definitions

For automatic, yet artificial, turn definitions generation based on the
network structure, see [generateTurnDefs.py](../Tools/Turns.md#generateturndefspy).

You can also generate turn definitions from an existing route file. see [genrateTurnRatios.py](../Tools/Turns.md#generateturnratiospy).

## Sinks Definitions

A vehicle leaves the network as soon as it comes to a sink edge. As not
all networks have sink edges defined, one can support a list of edges to
be declared as sinks using **--sinks <EDGE_ID\>[,<EDGE_ID\>\]***. You may also add your sink definitions to a
turn-file (XML only):

```
<turns>
   ... some further turning definitions as above ...

   <sink edges="<EDGE_ID>[ <EDGE_ID>]*"/>
   ... further sink definitions ...

</turns>
```

If you do not define sinks, the option **--accept-all-destinations** {{DT_BOOL}} may be used to declare all edge
as possible sinks.

## Flow Definitions

The [definitions of the
flow](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#repeated_vehicles_flows)
is the same as for the [duarouter](../duarouter.md) with just a
single difference: as it is not known where the vehicle will leave the
network as the route it uses is randomly computed, the route must be
specified using attribute *from* and attribute *to* must be omitted:

```
<flow id="0" from="A" begin="0" end="3600" probability="0.5"/>
```

### Random Flows

The tool [randomTrips.py](../Tools/Trip.md#randomtripspy) can be
used to generated jtrrouter input by setting options **--flows** {{DT_INT}} **--jtrrouter**.

## Overlapping Flows

By default JTRRouter will route flows through the network and split them at intersections until each part of the flow has reached a sink edge. When such a flow reaches the source edge of another flow, the total traffic is added.
The following options can change this behavior:

- **--sources-are-sinks** (shortcut **-S**). When this option is set, all flows will terminate upon reaching the from-edge of another flow. Issue #6601
- **--discount-sources** (shortcut **-D**). When this option is set, the flow departing at an edge will be reduced by the amount of traffic that has reached this edge from upstream flows. When combined with option **--sources-are-sinks**, upstream flows will terminate at a downstream edge when the source flow number has been reached.

## Additional Options

As theoretically a route may get infinitely long when a vehicle is
forced to take always the same direction, it is possible to limit the
route's size using option **--max-edges-factor** {{DT_FLOAT}}. This factor, multiplied with the number of
the used network's edges is the maximum number of edges a route may
have. With the default of 2.0, a route may contain twice as many edges
as the network has. Any route longer than this size will be marked as
invalid. We assume that for each network this number has to be chosen
again.