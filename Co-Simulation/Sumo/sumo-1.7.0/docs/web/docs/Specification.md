---
title: Specification
permalink: /Specification/
---

This document focuses on the main parts of a traffic simulation, that is
streets (edges) including lanes, junctions (nodes), and vehicles with
their routes. It contains nothing about traffic lights, detectors,
visualization or stuff like that. On the other hand this document aims
at a precise description which serves as a template for implementation
as well as for an xml schema for the input files. Therefore there is a
table for all allowed attributes together with their type (and possibly
unit). If the attribute is not mandatory, a default value is defined as
well. The names of all elements and attributes consist of letters
(lowercase or camelCaps) and underscores only. Keep in mind that the
values of attributes which are ids should consist of letters, numbers,
and underscores, hyphens, points and colons only, starting with a letter
or an underscore (this is called "valid XML id" in the following). There
are "follow-ups" to this document on the [simulation of persons and
multi-modality](Specification/Persons.md),
[containers](Specification/Containers.md) and on
[routing](Specification/Duarouter.md).

## Vehicles

A single vehicle is not modeled, it is always the vehicle on a journey,
that means once a vehicle has reached its destination it is deleted from
the system and cannot be referenced any longer. The physical parameters
of the vehicle are defined with its type, which also defines its
membership in some vehicle categories. A vehicle is defined by the
combination of its type and its route, together with parameters
specifying the start and end behavior. Additional parameters such as
color only serve visualization purposes. A vehicle can have routes and
stops as child elements.

| Attribute    | Type              | Range                                                  | Default      | Remark    |
| ------------ | ----------------- | ------------------------------------------------------ | ------------ | ----------------------- |
| id           | string            | valid XML ids                                          | \-           |                         |
| route        | string            | route or routedist id                                  | \-           | either this id or a route child element are mandatory   |
| type         | string            | vType or vTypeDistribution id                          | default type |                       |
| depart       | float(s)/string   | ≥0;"triggered"                                         |              |                         |
| departLane   | int/string        | ≥0,"random","free"                                     | 0            | "free" is the least occupied lane (by sum of the vehicle lengths)  |
| departPos    | float(m)/string   | ≥0<sup>(2)</sup>,"random","free","random_free","base" | "base"       | "free" means the point closest to the start of the depart lane where it is possible to insert the vehicle. "random_free" tries forcefully to find a free random position and if that fails, places the vehicle at the next "free" position. "base" sets the vehicle's depart position to the vehicle's length + eps (eps=.1m), this means the vehicle is completely at the begin of the depart lane. |
| departSpeed  | float(m/s)/string | ≥0,"random","max"                                      | 0            | "max" refers to the maximum velocity the vehicle can achieve when being inserted                                                                                                                                                                                                                                                                                                                      |
| arrivalLane  | int/string        | ≥0,"current"                                           | "current"    |      |
| arrivalPos   | float(m)/string   | ≥0<sup>(2)</sup>,"random","max"                        | "max"        |      |
| arrivalSpeed | float(m/s)/string | ≥0,"current"                                           | "current"    |      |

- (2): in fact, negative positions are currently allowed, too. In this
case, this value is added to the lane's length. This means, the
position is counted from the end of the lane. Values lying beyond
the edge borders (positive and negative) are silently moved to the
closest edge border.

### Flows (repeated insertion)

A repeated vehicle insertion has the same attributes and child elements
as a single vehicle except for depart. The following additional
attributes are known:

| Attribute   | Type        | Range  | Default          | Remark |
| ----------- | ----------- | ------ | ---------------- | ------ |
| begin       | float(s)    | ≥0     | simulation begin |        |
| end         | float(s)    | ≥begin | simulation end   |        |
| vehsPerHour | float(\#/h) | \>0    | \-               |        |
| period      | float(s)    | ≥0     | \-               |        |
| number      | int(\#)     | \>0    | \-               |        |

At most one of "vehsPerHour" and "period" has to be given. If one of
them is given it is not allowed to define "period" and "end". The
vehicles are equally distributed in the time interval. The number of
inserted vehicles (if "no" is not given) is equal to
("end"-"begin")/"period" rounded to the nearest integer, thus if
"period" is small enough, there might be no vehicle at all. Furthermore
"period"=3600/"vehsPerHour". The first vehicle (if any) is always
created at time "begin". The id of the created vehicles is
"flowId.runningNumber".

### Types

Types define physical parameters such as length, acceleration,
deceleration and maximum speed and give a list of categories cars of
this type belong to. All theses parameters are identical among all
vehicles of a type.

<table>
<thead>
<tr class="header">
<th><p>Attribute</p></th>
<th><p>Type</p></th>
<th><p>Range</p></th>
<th><p>Default</p></th>
<th><p>Remark</p></th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><p>id</p></td>
<td><p>string</p></td>
<td><p>valid XML ids</p></td>
<td><p>-</p></td>
<td><p>This attribute is mandatory.</p></td>
</tr>
<tr class="even">
<td><p>vClass</p></td>
<td><p>string</p></td>
<td><p>list of category ids</p></td>
<td><p>empty list</p></td>
<td></td>
</tr>
<tr class="odd">
<td><p>accel</p></td>
<td><p>float(m/s<sup>2</sup>)</p></td>
<td><p>≥0</p></td>
<td><p>2.6</p></td>
<td></td>
</tr>
<tr class="even">
<td><p>decel</p></td>
<td><p>float(m/s<sup>2</sup>)</p></td>
<td><p>≥0</p></td>
<td><p>4.5</p></td>
<td></td>
</tr>
<tr class="odd">
<td><p>sigma</p></td>
<td><p>float</p></td>
<td><p>0≤sigma≤1</p></td>
<td><p>0.5</p></td>
<td></td>
</tr>
<tr class="even">
<td><p>length</p></td>
<td><p>float(m)</p></td>
<td><p>&gt;0</p></td>
<td><p>5</p></td>
<td><p>The physical length of the vehicle</p></td>
</tr>
<tr class="odd">
<td><p>minGap</p></td>
<td><p>float(m)</p></td>
<td><p>&gt;0</p></td>
<td><p>2.5</p></td>
<td><p>The minimum gap between this vehicle and the vehicle before it</p></td>
</tr>
<tr class="even">
<td><p>maxSpeed</p></td>
<td><p>float(m/s)</p></td>
<td><p>&gt;0</p></td>
<td><p>70</p></td>
<td></td>
</tr>
<tr class="odd">
<td><p>color</p></td>
<td>&lt;COLOR&gt;</td>
<td></td>
<td></td>
<td><p>the color to use for vehicles of this type.</p></td>
</tr>
<tr class="even">
<td><p>tau</p></td>
<td><p>float</p></td>
<td><p>&gt;0</p></td>
<td><p>1</p></td>
<td></td>
</tr>
<tr class="odd">
<td style="background:red;"><p>carFollowModel</p></td>
<td><p>string</p></td>
<td></td>
<td></td>
<td style="background:red;"><p>not yet implemented (see #663")</p></td>
</tr>
<tr class="even">
<td><p>laneChangeModel</p></td>
<td><p>string</p></td>
<td><p>"DK2008", "LC2013", "JE2013"</p></td>
<td><p>"LC2013"</p></td>
<td><p>model used for lane changing behavior</p></td>
</tr>
<tr class="odd">
<td><p>speedFactor</p></td>
<td><p>float/distribution</p></td>
<td><p>&gt;0</p></td>
<td><p>1</p></td>
<td><p>the factor by which the driver multiplies the speed read from street signs to estimate "real" maximum allowed speed</p></td>
</tr>
<tr class="even">
<td><p>speedDev</p></td>
<td><p>float</p></td>
<td><p>≥0</p></td>
<td><p>0</p></td>
<td><p>the standard deviation of the speed factor (deprecated in favor of a real distribution).</p></td>
</tr>
<tr class="odd">
<td><p>emissionClass</p></td>
<td><p>string</p></td>
<td></td>
<td><p>P_7_7</p></td>
<td><p>the emission class, see <a href="Models/Emissions.html" title="wikilink">Models/Emissions</a>.</p></td>
</tr>
<tr class="even">
<td><p>probability</p></td>
<td><p>float</p></td>
<td><p>&gt;0</p></td>
<td><p>1</p></td>
<td><p>this is only useful in distributions</p></td>
</tr>
<tr class="odd">
<td><p>guiShape</p></td>
<td><p>string (enum)</p></td>
<td></td>
<td><p>"unknown"</p></td>
<td><p>How this vehicle is rendered</p></td>
</tr>
<tr class="even">
<td><p>width</p></td>
<td><p>float</p></td>
<td><p>&gt;0</p></td>
<td><p>2.0</p></td>
<td><p>The vehicle's width [m] (only used for drawing);</p></td>
</tr>
<tr class="odd">
<td><p>imgFile</p></td>
<td><p>string</p></td>
<td></td>
<td></td>
<td><p>Image file for rendering vehicles of this type (should be grayscale to allow functional coloring);</p></td>
</tr>
<tr class="even">
<td style="background:yellow;"><p>osgFile</p></td>
<td><p>string</p></td>
<td></td>
<td></td>
<td><p>3D-Model file rendering vehicles of this type - internal experimental branch;</p></td>
</tr>
<tr class="odd">
<td><p>personCapacity</p></td>
<td><p>int</p></td>
<td><p>≥0</p></td>
<td><p>4</p></td>
<td><p>the number of persons (excluding an autonomous driver) the vehicle can transport</p></td>
</tr>
<tr class="even">
<td><p>containerCapacity</p></td>
<td><p>int</p></td>
<td><p>≥0</p></td>
<td><p>0</p></td>
<td><p>the number of containers the vehicle can transport</p></td>
</tr>
<tr class="odd">
<td><p>boardingDuration</p></td>
<td><p>float</p></td>
<td><p>≥0</p></td>
<td><p>0.5</p></td>
<td><p>the time required by a person to board the vehicle</p></td>
</tr>
<tr class="even">
<td><p>loadingDuration</p></td>
<td><p>float</p></td>
<td><p>≥0</p></td>
<td><p>30.0</p></td>
<td><p>the time required to load a container onto the vehicle</p></td>
</tr>
</tbody>
</table>

There is a default type defined with the id "DEFAULT_VEHTYPE", having
all the default parameters above, which may be redefined once but only
if it was not used beforehand (either by a vehicle or as a refId).
Redefining the default type does not change the defaults if defining a
new type, that means `<vType refId="DEFAULT_VEHTYPE"/>` may be different
from `<vType/>`.

The distribution for a speedFactor can currently only be given as
"norm(mean, dev)" or "normc(mean, dev, min, max)" which result in the
value for the vehicle being drawn from a standard normal distribution
with the given mean and standard deviation (first case) with optional
cutoff values (second case). In case of cutoff values the value will
still be drawn from the given normal distribution but if it is not in
the given range, the draw will be repeated (Attention: This may result
in long execution time for very narrow intervals).

### Type distributions

Type distributions define probability distributions of vehicle types.
They should have at least two vType childs, although defining only one
child is valid as well. All vehicle type childs maybe referenced outside
the distribution as well. The sum of the probabilities of the childs
should be larger than zero.

| Attribute | Type   | Range         | Default | Remark                      |
| --------- | ------ | ------------- | ------- | --------------------------- |
| id        | string | valid XML ids | \-      | This attribute is mandatory |

### Categories

Categories define vehicle classes such as cars, trucks, busses, but also
height, width and weight categories might (to some extent) be modeled
here. The sole purpose of the category is to determine whether a car is
allowed to (or prefers to) drive on a certain street or lane.

| Attribute   | Type   | Range                       | Default | Remark                                               |
| ----------- | ------ | --------------------------- | ------- | ---------------------------------------------------- |
| id          | string | valid XML ids               | \-      |                                                      |
| description | string | description of the category | \-      | serves only documentation and visualization purposes |

## Junctions

Strictly speaking, junctions or nodes need a unique id, which makes them
referenceable by streets which start or end there. Since our network is
always embedded into the plane, they also need x- and y-coordinates. A
type may be given if the one determined by
[netconvert](netconvert.md) is not correct.

| Attribute | Type     | Range                                         | Default | Remark       |
| --------- | -------- | --------------------------------------------- | ------- | ------------ |
| id        | string   | valid XML ids                                 | \-      |              |
| x         | float(m) | \-10^6<x<10^6                               | \-      |              |
| y         | float(m) | \-10^6<y<10^6                               | \-      |              |
| type      | string   | priority, right_before_left, traffic_light | \-      | if no value is given, [netconvert](netconvert.md) tries to determine the type heuristically |

## Streets

Streets need a unique id, a starting node and an ending node. Since the
nodes are embedded into the plane, the length is optional and (if not
given) is calculated as the euclidean distance between starting node and
end node. There may be further points in the plane (no junctions) given
to describe the shape of the street respectively calculating its length
as the cumulative distance. The length has to be strictly positive (not
zero). This means that if starting node and end node are identical, the
length has either to be given explicitly or there needs to be at least
one shape node at a position different from the start/end node.
Optionally a number of lanes may be given (defaulting to 1, respectively
to the number of lane elements in the definition of the street). If the
number of lanes given as an attribute is smaller than the number of lane
child elements this is an error. All parameters which can be given to
lanes can also be given to the street and serve as a default for the
corresponding lane parameter. The maximum speed allowed on the edge is
given in m/s.

| Attribute  | Type       | Range                  | Default | Remark                                                                         |
| ---------- | ---------- | ---------------------- | ------- | ------------------------------------------------------------------------------ |
| id         | string     | valid XML ids          | \-      |                                                                                |
| refId      | string     | another edge id        | \-      | all attributes and childs are copied from the given edge and maybe overwritten |
| from       | string     | node id                | \-      |                                                                                |
| to         | string     | node id                | \-      |                                                                                |
| function   | string     | normal,internal,ramp   | normal  | cannot be given as input, appears only in generated nets                       |
| length     | float(m)   | ≥0                     | \-      |                                                                                |
| numLanes   | int        | \>0                    | \-      | either this one or lane child elements are mandatory                           |
| speed      | float(m/s) | \>0                    | \-      |                                                                                |
| departLane | int        | 0≤departLane<numLanes  | 0       |                                                                                |

### Lanes

Lanes have a maximum allowed speed, and lists of allowed, disallowed and
preferred vehicle categories. The allowed list defaults to all vehicle
categories and the disallowed list to none. A vehicle is allowed to
drive on a lane if its category list contains no member of the
disallowed list and contains some member of the allowed list.

| Attribute | Type       | Range                    | Default                     | Remark |
| --------- | ---------- | ------------------------ | --------------------------- | ------ |
| index     | int        | 0≤index<edge.numLanes    | smallest non-explicit index |        |
| speed     | float(m/s) | ≥0                       | 13.9                        |        |
| allow     | string     | list of category ids,all | all                         |        |
| disallow  | string     | list of category ids     | empty list                  |        |
| prefer    | string     | list of category ids     | empty list                  |        |

## Routes

Routes give a description of the path a vehicle will follow, that is
they merely consist of a non-empty list of streets which are
consecutive. Optionally they can have a list of stops as child elements.

| Attribute | Type   | Range            | Default | Remark                                                                                   |
| --------- | ------ | ---------------- | ------- | ---------------------------------------------------------------------------------------- |
| id        | string | valid XML ids    | \-      | The attribute is disallowed for routes defined inside a vehicle or a route distribution. |
| refId     | string | another route id | \-      |                                                                                          |
| edges     | string | list of edge ids | \-      |                                                                                          |
| frequency | float  | \>0              | 1       | This only useful in connection with route distributions                                  |

### Route distributions

Route distributions define probability distributions of routes. They
should have at least two route childs.

| Attribute | Type   | Range         | Default | Remark |
| --------- | ------ | ------------- | ------- | ------ |
| id        | string | valid XML ids | \-      |        |

### Stops

Stops can be childs of vehicles, routes or persons.

| Attribute          | Type              | Range                                                                          | Default            | Remark                                                                                                 |
| ------------------ | ----------------- | ------------------------------------------------------------------------------ | ------------------ | ------------------------------------------------------------------------------------------------------ |
| busStop            | string            | valid [bus stop](Simulation/Public_Transport.md) ids                   | \-                 | if given, edge, lane, startPos and endPos are not allowed                                              |
| containerStop      | string            | valid [container stop](Specification/Logistics.md#container_stops) ids | \-                 | if given, edge, lane, startPos and endPos are not allowed                                              |
| lane               | string            | lane id                                                                        | \-                 | the lane id takes the form <edge_id\>_<lane_index\>. the edge has to be part of the corresponding route |
| endPos             | float(m)          | ε≤endPos≤edge.length                                                           | edge.length        |                                                                                                        |
| startPos           | float(m)          | 0≤startPos≤endPos-ε                                                            | endPos-ε           |                                                                                                        |
| friendlyPos        | bool              | true,false                                                                     | false              | whether invalid stop positions should be corrected automatically                                       |
| duration           | float(s)          | ≥0                                                                             | \-                 |                                                                                                        |
| until              | float(s)          | ≥0                                                                             | \-                 | the time step at which the route continues                                                             |
| index              | int, "end", "fit" | 0≤index≤number of stops in the route                                           | "end"              | where to insert the stop in the vehicle's list of stops                                                |
| triggered          | bool              | true,false                                                                     | false              | whether a person may end the stop                                                                      |
| containerTriggered | bool              | true,false                                                                     | false              | whether a container may end the stop                                                                   |
| parking            | bool              | true,false                                                                     | value of triggered | whether the vehicle stops on the road or beside                                                        |
| actType            | string            | arbitrary                                                                      | 'waiting'          | activity displayed for stopped person in GUI and output files (only applies to person simulation)      |

If "duration" *and* "until" are given, the vehicle will stop for at
least "duration" seconds. If "duration" is 0 the vehicle will decelerate
such that it may in principle reach velocity 0 but instead of braking to
full stop it will start to accelerate again. If "until" is given and
"duration" is not and the vehicle arrives at the stop at or after the
time step defined by "until" it will not even decelerate. If until is
defined in the context of a repeated vehicle insertion (flow) it will be
incremented by the difference of vehicle creation time and "begin" of
the flow. If neither "duration" nor "until" are given, "triggered"
defaults to true. If "triggered" is set to false explicitly the vehicle
will stop forever.

!!! caution
    If *triggered* is true then *parking* will also be set to true by default. If you then set *parking* to false you may create deadlocks which prevent the simulation from terminating
!!! note
    Bus stops must have a length of at least 10

## How the vehicle drives

### Before start

On route loading (which is not necessarily the same as application
startup) SUMO checks the route for connectivity and for proper placement
of stops. If speed deviation and/or speed factor are set, it calculates
a speed factor for the vehicle by drawing from a gaussian distribution.
There are some cutoffs applied to the finally chosen speed factor to
have it in a sensible range. It cannot be larger than speedFactor + two
standard deviations and not smaller 0.2 \* speedFactor. If the chosen
factor does not work with the chosen departure speed it is recalculated.

On vehicle loading SUMO also checks whether the vehicle is allowed to
drive on all edges of its route and whether all other parameters are in
the allowed ranges. If any violation of parameter ranges occurs, the
simulation stops immediately with an error message. (It is still in
discussion whether it should be possible to disable some of the checks
via a command-line option, resulting in a vehicle which stops at the end
of the last "valid" edge.)

### Departure

At the given departure time the simulation tries to insert the vehicle
with the given parameters. If this is not possible because it would
result in a collision, the simulation retries in the next simulation
step. If "free" or "random" are specified for startpos and/or startlane,
they are recalculated for the next try. The parameters are evaluated in
the following order:

1.  Determination of the lane
2.  Determination of the position (even if lane and position are both
    set to "free", we *do not* choose a different lane if we cannot find
    a free position on the lane chosen)
3.  Determination of the start speed (if position is "free" and speed is
    "max" we *do not* choose the position where we can reach the highest
    speed but choose the highest speed we can use at the position chosen
    before).

When determining the maximum speed and the possibility of insertion the
next vehicle(s) upstream and the next vehicle(s) downstream have to be
respected even if they are located on the next road section. Thus it may
be necessary to take all road sections into account which lead into or
follow the current section.

For filling the simulation as fast as possible with vehicles, use the
following parameters for the vehicle: `depart="0" departSpeed="0"
departLane="free" departPos="random_free" period="0" number="100"`.
Thus, 101 vehicles will be inserted at timestep 0 on the first edge of
the given route.

### Driving

At each step the model calculates a new target speed and accelerates
(decelerates) the vehicle accordingly. The acceleration is constant for
the step such that a car which is at position s with speed v and gets
during the step an acceleration of a has after a timestep of length t
the new position s' = s + v\*t + a\*t^2/2 and v' = v + a\*t.

### Stops

A vehicle tries to reach the most downstream position of a stop area
before it actually stops. It can only be forced to stop earlier by other
vehicles blocking the rest of the stop area. The duration of the stop
starts counting with the first step in which the vehicle's position is
in the stop area and its speed is 0. That means if a vehicle stops at
timestep 10 (reaches the area and speed 0) and has a stop duration of 2,
it stays there for step 11 and 12 and has a new speed and position in
step 13 (provided no other blocking occurs).

### End

If the vehicle reaches the point of final destination it is removed from
the simulation. That means if the s' as calculated above is larger or
equal to the destination point, the vehicle gets removed in this step.