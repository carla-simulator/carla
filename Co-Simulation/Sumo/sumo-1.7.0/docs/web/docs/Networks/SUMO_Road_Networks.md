---
title: Networks/SUMO Road Networks
permalink: /Networks/SUMO_Road_Networks/
---

![eichstaett.net.png](../images/Eichstaett.net.png "Screenshot of a SUMO net file opened in sumo-gui. It shows the map of the German city Eichstätt.")
Screenshot of a SUMO net file opened in sumo-gui. It shows the map of the German city Eichstätt.

| SUMO road network  |              |
|--------------------|--------------|
| Filename extension | .net.xml     |
| Type of content    | Map          |
| Open format?       | Yes          |
| SUMO specific?     | Yes          |
| XML Schema         | [net_file.xsd](http://sumo.dlr.de/xsd/net_file.xsd) |

A **SUMO network file** describes the traffic-related part of a map, the
roads and intersections the simulated vehicles run along or across. At a
coarse scale, a SUMO network is a directed graph. Nodes, usually named
"junctions" in SUMO-context, represent intersections, and "edges" roads
or streets. Note that edges are unidirectional. Specifically, the SUMO
network contains the following information:

- every street (edge) as a collection of lanes, including the
  position, shape and speed limit of every lane,
- traffic light logics referenced by junctions,
- junctions, including their right of way regulation,
- connections between lanes at junctions (nodes).

Also, depending on the used input formats and set processing options,
one can also find

- districts,
- roundabout descriptions.

**Although being readable (XML) by human beings, a SUMO network file is
not meant to be edited by hand.** Rather you should use [SUMO XML
description
files](../Networks/PlainXML.md)
together with [netconvert](../netconvert.md). You can also convert
an existing map from various formats using
[netconvert](../netconvert.md) or generate geometrically simple,
abstract road maps with [netgenerate](../netgenerate.md). To modify
an existing *.net.xml*-file you may [load it with netconvert along with
patch files](../Networks/Import/SUMO_Road_Networks.md) You may also
use [netedit](../netedit.md) for building own road networks or for
reworking the ones obtained from [netconvert](../netconvert.md) or
[netgenerate](../netgenerate.md).

!!! caution
    When creating SUMO networks from custom input data, the recommended approach is to create/generate [plain-xml-files](../Networks/PlainXML.md) and use [netconvert](../netconvert.md) to turn these into a *.net.xml file.* Trying to [generate the .net.xml file directly is fraught with dangers](../FAQ.md#i_made_changes_to_the_netxml-file_but_it_did_not_work_as_expected_why).

## Network Format

SUMO road networks are encoded as XML files. The contents are grouped by
the instances in the following order:

- cartographic projection valid for this network
- edges; at first, internal edges are given, then plain edges; each
  edge contains the list of lanes that belong to it
- traffic light logics
- junctions, including their right-of-way definitions; plain junctions
  first, then internal junctions
- connections, plain first, then internal
- optionally roundabouts

### Coordinates and alignment

The networks are using cartesian, metric coordinates where the leftmost
node is at x=0 and the node being most at the bottom is at y=0. This
means that when being imported, [netconvert](../netconvert.md) and
[netgenerate](../netgenerate.md) are projecting the network, first,
if the original network was not using cartesian and/or metric
coordinates. Then, they move the road network to the origin at (0,0).

This process is documented within the generated network within the
element `location`. You may find here the
following attributes:

| Name          | Type                 | Description                                                                              |
| ------------- | -------------------- | ---------------------------------------------------------------------------------------- |
| netOffset     | offset ({{DT_2DPosition}})  | The offset applied for moving the network to (0,0)                                |
| convBoundary  | boundary ({{DT_2DBoundingBox}})        | The boundary spanning over the nodes of the converted network (the network as now given) |
| origBoundary  | boundary ({{DT_2DBoundingBox}})          | The network's initial boundary before projecting and translating it                      |
| projParameter | projection parameter | Information how the network was projected (see below)                                    |

`projParameter` may have one of the following
values:

| Encoding           | Description                                                                |
| ------------------ | -------------------------------------------------------------------------- |
| '\!'               | No projection was applied.                                                 |
| '-'                | "Simple" projection was applied                                            |
| '.'                | ?                                                                          |
| proj-definition ({{DT_PROJ_DEF}}) | The projection was done using proj, initialising it using these parameter. |

For obtaining the original coordinates from the x/y-coordinates SUMO
reports, one has to subtract the network offset first. Then, one has to
apply an inverse transformation of the initial projection, regarding
`projParameter`. For proj-projected networks,
one can use proj's function named pj_inv which performs the inverse
transformation. proj has to be initialised with the projection
parameters from `projParameter`, first. For
proj-projected networks this would look like:

```
Position2D cartesian = sumoXY(x, y);
projPJ myProjection = pj_init_plus(projParameter.c_str());
cartesian.sub(netOffset);
projUV p;
p.u = cartesian.x();
p.v = cartesian.y();
p = pj_inv(p, myProjection);
p.u *= RAD_TO_DEG;
p.v *= RAD_TO_DEG;
cartesian.set(p.u, p.v);
```

For networks with no projection
(`projParameter`='\!'), only the offset must
be applied. For networks with "simple" projection
(`projParameter`='-'), a back-projection
method is currently not known.

SUMO road networks are meant to be aligned to the north. Of course, it
is up to the user how he defines a road network, but when being imported
from sources as Open Street Map or shape files, the up-direction is
meant to correspond to north.

### Edges and Lanes

"plain" and "internal" edges are encoded almost the same way, but they
differ by the mandatory and used attributes.

#### Normal Edges

A "normal" edge is a connection between two nodes ("junctions").

```
<edge id="<ID>" from="<FROM_NODE_ID>" to="<TO_NODE_ID>" priority="<PRIORITY>">
    ... one or more lanes ...
</edge>
```

Here, the ID (*<ID\>* in the example above) is an ID as read during the
network import. Both the IDs of the begin and the end node are given
within the attributes `from` and
`to`, respectively. The priority is an
abstract ordinal number that determines right-of-way rules. The
`function` attribute was omitted in the
example above, as it defaults to "`normal`",
which is the example edge's `function` value.

The attributes of an edge are:

| Name     | Type                | Description                                                                           |
| -------- | ------------------- | ------------------------------------------------------------------------------------- |
| id       | id (string)         | The id of the edge                                                                    |
| from     | id (string)         | The id of the node it starts at                                                       |
| to       | id (string)         | The id of the node it ends at                                                         |
| priority | integer             | Indicates how important the road is (optional)                                        |
| function | enum ( "`normal`", "`internal`", "`connector`", "`crossing`", "`walkingarea`" ) | An abstract edge purpose (optional with default "`normal`") |

For the simulation, only the "function" attribute is of interest. It
describes how the edge is used, and whether it is an edge that can be
found within the real world or only a helper construct used for
assignment. The following purposes are defined:

- `normal`: The edge is a plain part of a
  road network, like a highway or a normal street which connects two
  roads
- `connector`: The edge is a macroscopic
  connector - not a part of the real world road network. Still, within
  the simulation, no distinction is made between "connector" roads and
  "normal" nodes. Only [sumo-gui](../sumo-gui.md) allows to hide
  connector edges.
- `internal`: The edge is a part of an
  intersection (is located within the intersection), see above.
- `crossing`:
- `walkingarea`:

#### Lanes

Each edge includes the definitions of lanes it consists of. The
following example shows a single edge with two lanes. Note, that
coordinates may be 2D as well as 3D.

```
    <edge id="<ID>" from="<FROM_NODE_ID>" to="<TO_NODE_ID>" priority="<PRIORITY>">
        <lane id="<ID>_0" index="0" speed="<SPEED>" length="<LENGTH>" shape="0.00,495.05 248.50,495.05"/>
        <lane id="<ID>_1" index="1" speed="<SPEED>" length="<LENGTH>" shape="0.00,498.35,2.00 248.50,498.35,3.00"/>
    </edge>
```

As one can see, the id is composed by the ID of the edge the lane
belongs to and a running number, both divided by an underline ('_').
The running number starts at zero for the right-most lane. The same
number is also given in the "`index`"
attribute. The lanes are sorted by this number, too. \@note: this is
highly duplicate

The attributes of a lane are:

| Name       | Type                          | Description                                                   |
| ---------- | ----------------------------- | ------------------------------------------------------------- |
| **id**     | id (string)                   | The id of the lane                                            |
| **index**  | running number (unsigned int) | A running number, starting with zero at the right-most lane   |
| **speed**  | float                         | The maximum speed allowed on this lane \[m/s\]                |
| **length** | float                         | The length of this lane \[m\]                                 |
| **shape**  | position vector               | The geometry of the lane, given by a polyline that describes the lane's center line; must not be empty or have less than two positions |

It should be noted, that currently all lanes of an edge have the same
length, even if it differs from the geometrical length of the shape.
[netconvert](../netconvert.md) even explicitly allows to override
the geometrical lengths. Additionally, even though the network is
shifted to start at (0,0), it is not guaranteed that all of the
network's parts have positive coordinates.

!!! caution
    The minimum edge length is 0.1m.

#### Internal Edges

An internal edge lies within an intersection and connects and incoming
*normal* edge with an outgoing *normal* edge. Internal edges are not
included if the network was built using the **--no-internal-links** option. An example of an
internal edge may look like this:

```
<edge id="<ID>" function="internal">
    ... one lane ...
</edge>
```

The id of an internal edge is composed as :*<NODE_ID\>*_*<EDGE_INDEX\>*
where *<NODE_ID\>* is the ID of the node the edge is located within and
*<EDGE_INDEX\>* is a running number running clockwise around the node
(starting in the north). Please note that the ID has a ':' as prefix. If
the incoming and outgoing edges connected by the internal edge have
multiple lanes, the internal edge has multiple lanes as well (according
to the number of connections between these two edges). In case of
multi-lane internal edges, the INDEX jumps by the number of of lanes so
that EDGE_INDEX + LANE_INDEX = CONNECTION_INDEX.

The attributes are given in the following table.

| Name         | Type                                   | Description                                                        |
| ------------ | -------------------------------------- | ------------------------------------------------------------------ |
| **id**       | id (string)                            | The id of the internal edge                                        |
| **function** | "`internal`" | Always "`internal`" for an internal edge |

#### Stop Offsets

Each edge or lane may carry a `stopOffset` child element to specify an additional
stopping offset for vehicles of certain classes:

```
<edge id="<ID>">
    <stopOffset value="<distance in m.>" vClasses="<space-separated list of vClasses>" />
    <lane id="<ID>" index="<INDEX>" ... >
        <stopOffset value="<distance in m.>" exceptions="<space-separated list of vClasses>" />
    </lane>
    ...
</edge>
```

Defining this element for an edge will affect all lanes of the edge that
do not hold an own `stopOffset` element. Note that there is the possibility to
define either all vehicle classes, that are affected by the stop offset
(attribute `vClasses`), or those, which are not affected (attribute `exceptions`). You may not
use both attributes in conjunction. The distance at which the specified
vehicle classes are required to stop from the lane end is specified by
the `value`-attribute.

| Name           | Type             | Description                                                         |
| -------------- | ---------------- | ------------------------------------------------------------------- |
| **value**      | value (double)   | The stop offset as positive value in meters.                        |
| **vClasses**   | list of vClasses | Specifies, for which vehicle classes the stopOffset applies.        |
| **exceptions** | list of vClasses | Specifies, for which vehicle classes the stopOffset does not apply. |

For specification of vehicle classes see
[here](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#abstract_vehicle_class).

### Traffic Light Programs

A traffic light program defines the phases of a traffic light.

```
    <tlLogic id="<ID>" type="<ALGORITHM_ID>" programID="<PROGRAM_ID>" offset="<TIME_OFFSET>">
        <phase duration="<DURATION#1>" state="<STATE#1>"/>
        <phase duration="<DURATION#1>" state="<STATE#1>"/>
        ... further states ...
        <phase duration="<DURATION#n>" state="<STATE#n>"/>
    </tlLogic>
```

For an explanation of the attributes and semantics, please consult
[Simulation/Traffic Lights](../Simulation/Traffic_Lights.md).
Please note that usually, the generated program has the ID "0".

### Junctions and Right-of-Way

#### Plain Junctions

Junctions represent the area where different streams cross, including
the right-of-way rules vehicles have to follow when crossing the
intersection. An example may be:

```
<junction id="<ID>" type="<JUNCTION_TYPE>" x="<X-POSITION>" y="<Y-POSITION>"
          incLanes="<INCOMING_LANES>" intLanes="<INTERNAL_LANES>"
          shape="<SHAPE>">
    ... requests ...
</junction>
```

The junction itself is described by the following attributes:

| Name         | Type              | Description              |
| ------------ | ----------------- | ------------------------ |
| **id**       | id (string)       | The id of the junction; please note, that a traffic light definition must use the same ID when controlling this intersection.                                          |
| **x**        | x-position (real) | The x-coordinate of the intersection                                                                                                                                   |
| **y**        | y-position (real) | The y-coordinate of the intersection                                                                                                                                   |
| z            | z-position (real) | The (optional) z-coordinate of the intersection                                                                                                                        |
| **incLanes** | id list           | The ids of the lanes that end at the intersection; sorted by direction, clockwise, with direction up = 0                                                               |
| **intLanes** | id list           | The IDs of the lanes within the intersection                                                                                                                           |
| **shape**    | position list     | A polygon describing the road boundaries of the intersection                                                                                                           |
| customShape  | bool              | Whether the shape was customized by the user (and should thus not be rebuilt by [netconvert](../netconvert.md) or [netedit](../netedit.md)), default *False* |

Please note, that the x/y-positions of the junction describe the given,
not the computed center of the junction. It is allowed for two nodes to
have the same position.

!!! caution
    The maximum number of connections (links) per junction is limited to 256 since version 0.25.0. In earlier versions of sumo the limit was 64.

#### Requests

What follows are "`request`"s, looking like:

```
  <request index="<INDEX>" response="<RELATIVE_MAJOR_LINKS>" foes="<FOE_LINKS>" cont="<MAY_ENTER>"/>
```

They describe, for each link (given as
"`index`") which streams have a higher
priority ("`response`") and force the vehicle
on link "`index`" to stop, and which streams
are in conflict ("foes"). "`foes`" is a
super-set of "`response`", adding streams that
have a lower priority than the stream referenced as
"`index`".

One can read the requests as following: the index describes a certain
connection, between an incoming and an outgoing lane, over the
intersection. The indices are computed connection-wise and the order is
as following: first, sort edges by their direction, starting with 0°
(up); second, use lane order (right-most-first); third, order by the
direction the connection follows, starting by the right-most.

The same order is applied in the "response" and the "foes" field,
starting from right. This means:

```
  <request index="2" response="001" foes="011" cont="0"/>
```

The a vehicle using the connection with the index "2" has to wait for
vehicles that use the connection "0". Connections "0" and "1" are its
foes. The attributes of a request element are:

| Name         | Type                 | Description                  |
| ------------ | -------------------- | ---------------------------- |
| **index**    | index (unsigned int) | The index of the described connection in the right-of-way matrix     |
| **response** | bitset (string)      | A bitstring that describes for each connection whether it prohibits un-decelerated passing of the intersection for vehicles at this connection. The rightmost bit corresponds to index 0.                           |
| **foes**     | bitset (string)      | A bitstring that describes for each connection whether it conflicts with this connection. The rightmost bit corresponds to index 0.                                                                               |
| **cont**     | bool                 | Whether a vehicle may pass the first stop line to wait within the intersection until there are no vehicles with higher priority. This is typically the case for left-moving streams from the prioritized direction. |

!!! caution
    The bitstrings in attributes `response` and `foes` must be read from right to left (response to link 0 is the rightmost entry) in contrast to `tlLogic`-`state` strings which are read from left to right (state of link 0 is the leftmost entry).

#### Internal Junctions

"internal" junctions are used to define a [waiting position within the
intersection](../Simulation/Intersections.md#waiting_within_the_intersection)
for some traffic relations. This is typically used for left-turning
vehicles from the prioritized direction or for right-turning vehicles
that have to wait for straight-going pedestrians.

Since version 0.25.0 the user can [customize the positioning (and
presence) of internal
junctions](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which)

These junctions do not need a right-of-way matrix but only following
information

- the lanes that are incoming into the junction the internal junction
  is located in and prohibit to pass the internal junction if a
  vehicle is approaching
- the internal lanes that must not be occupied for crossing the
  internal junction

An internal junction is encoded like this:

```
<junction id="<ID>" type="internal" x="<X-POSITION>" y="<Y-POSITION>"
          incLanes="<INCOMING_PROHIBITING_LANES>"
          intLanes="<INTERNAL_PROHIBITING_LANES>"/>
```

The ID is the same as the lane to use when crossing the internal
junction itself. The type is always "internal". Please note that an
internal junction is usually encoded in one line. The attributes of an
internal junction are:

| Name         | Type              | Description               |
| ------------ | ----------------- | ------------------------- |
| **id**       | id (string)       | The id of the junction; please note, that a traffic light definition must use the same ID when controlling this intersection.         |
| **x**        | x-position (real) | The x-coordinate of the intersection   |
| **y**        | y-position (real) | The y-coordinate of the intersection   |
| z            | z-position (real) | The (optional) z-coordinate of the intersection    |
| **incLanes** | id list           | The ids of the lanes that end at the intersection the internal junction is located within and prohibit to cross the internal junction |
| **intLanes** | id list           | The ids of the lanes within the intersection the internal junction is located within and prohibit to cross the internal junction      |

### Connections

#### Plain Connections

Plain connections or "links" describe which outgoing lanes can be
reached from an incoming lane. Additionally, the first lane to use for
passing the intersection is given. Additional information describe the
direction and the "state" of the connection. If the connection is
controlled by a traffic light, the name of the traffic light and the
index of the signal that controls this connection within the traffic
light's phase definition is given. A connection is encoded as:

```
<connection from="<FROM_EDGE_ID>" to="<TO_EDGE_ID>" fromLane="<FROM_LANE_INDEX>" toLane="<TO_LANE_INDEX>"
            via="<VIA_LANE_ID>" tl="<TRAFFIC_LIGHT_ID>" linkIndex="12" dir="r" state="o"/>
```

The attributes are:

| Name          | Type      | Description                   |
| ------------- | --------- | ----------------------------- |
| **from**      | edge id (string)       | The ID of the incoming edge at which the connection begins  |
| **to**        | edge id (string)       | The ID of the outgoing edge at which the connection ends    |
| **fromLane**  | index (unsigned int)   | The lane of the incoming edge at which the connection begins    |
| **toLane**    | index (unsigned int)   | The lane of the outgoing edge at which the connection ends      |
| **via**       | lane id (string)       | The id of the lane to use to pass this connection across the junction  |
| **tl**        | traffic light id (string)   | The id of the traffic light that controls this connection; the attribute is missing if the connection is not controlled by a traffic light      |
| **linkIndex** | index (unsigned int)       | The index of the signal responsible for the connection within the traffic light; the attribute is missing if the connection is not controlled by a traffic light |
| **dir**       | enum ("s" = straight, "t" = turn, "l" = left, "r" = right, "L" = partially left, R = partially right, "invalid" = no direction)     | The direction of the connection     |
| **state**     | enum ("-" = dead end, "=" = equal, "m" = minor link, "M" = major link, traffic light only: "O" = controller off, "o" = yellow flashing, "y" = yellow minor link, "Y" = yellow major link, "r" = red, "g" = green minor, "G" green major) | The state of the connection  |

When creating these connections without netconvert (not recommended) it
should be taken into account that a connection with an internal lane
follows a special pattern. For example when lane 1_f_0 needs to be
connected to 1_t_0 through 1_v_0, the following connection entries
exist: \[from=1_f_0, to=1_t_0 via=1_v_0\] and \[from=1_v_0
to=1_t_0\]. If the latter is omitted, SUMO will not load the network.
It will also not load if next to the necessary connections a connection
\[from=1_f_0 to=1_v_0\] exists.

#### Indices of a connection

Each connection has associated indices which can be shown in
[sumo-gui](../sumo-gui.md) by customizing gui settings and
activating *Junctions-\>Show ... index*. These indices are usually
identical but may be configured independently of each other.

- **Junction Index**: This index is described in section
  [\#Requests](#requests). It cannot be modified. The index
  corresponds to the order of the `<junctions>`s *incLanes*-attribute and also to
  the order of the `<connection>` elements. These indices run clockwise (in a
  right-hand network) around the junction starting at the top.

- **TLS Index**: This index is assigned to connections which are
  [controlled by a traffic
  light](../Simulation/Traffic_Lights.md#default_link_indices).
  For joined traffic lights (controlling multiple junctions) this
  index differs from the **junction index** because the latter is no
  longer unique. The TLS Index may be [configured with a *.con.xml*
  file](../Networks/PlainXML.md#controlled_connections).
  Custom indices may be used to create signal groups (multple
  connections that always have the same state and share the same
  index) and thus simplify the *state* definiton for the traffic light
  phases.

### Roundabouts

Roundabouts in a network influence the right of way (encoded in the
[Requests](#requests) `<request>` elements of a junction
during network building). Their presence in the network file has two
reasons:

- It facillitates re-importing a *.net.xml*-file with
  [netconvert](../netconvert.md)
- lane-changing models may take roundabouts into account

Each roundabout is defined (somewhat redundantly) by its nodes and
edges:

```
<roundabout nodes="nodeID1 nodeID2 ..." edges="edgeID1 edgeID2 ..."/>
```

## Software for Viewing, Editing and Processing

To create a SUMO network file, [netconvert](../netconvert.md) helps
to generate it from maps in other formats and
[netgenerate](../netgenerate.md) constructs a new map with simple
geometries. [sumo](../sumo.md) performs its simulation directly in
the map of this file. Most other SUMO tools read such files to generate
or import information that must be the mapped onto a road network.

Networks can be created and edited graphically using
[netedit](../netedit.md).

The SUMO net file is not meant for manual editing. Convert it to the
[SUMO native XML
descriptions](../Networks/PlainXML.md)
with [netconvert](../netconvert.md) instead. You can then process
these files by hand and rebuild the network with
[netconvert](../netconvert.md).

## Further Documentation

- Network Import
  -  - [Defining own networks using XML and netconvert](../Networks/PlainXML.md)
    - [Importing networks from other applications using netconvert](../Networks/Import.md)
  - [netconvert](../netconvert.md) manual
- Generation of abstract road networks
  - [Generating abstract networks using
      netgenerate](../Networks/Abstract_Network_Generation.md)
  - [netgenerate](../netgenerate.md) manual
- [netedit](../netedit.md) manual
- [Developer/Network Building
  Process](../Developer/Network_Building_Process.md)

!!! note
    Please see the [ChangeLog](../ChangeLog.md) when dealing with networks generated by old versions of [netconvert](../netconvert.md) / [netgenerate](../netgenerate.md).