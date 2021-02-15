---
title: Specification/Containers
permalink: /Specification/Containers/
---

Containers can be used to simulated freight and goods. A container can
represent, for example, an ISO container, a tank container, an arbitrary
amount of bulk material, an arbitrary amount of animals etc.

# Containers

A container moves through the net by being transported by a vehicle or
being transhiped between two stops. A container element has child
elements defining stages of its plan. The stages are a connected
sequence of [transport](../Specification/Containers.md#transports),
[tranship](../Specification/Containers.md#tranships) and
[stop](../Specification/Containers.md#stops) elements as described
below. Each container must have at least one stage in its plan.

```
<container id="foo" depart="0">
    <tranship edges="a b"/>
    <transport ../>
    <transport ../>
    <tranship ../>
    <stop ../>
</container>
```

| Attribute | Type      | Range               | Default | Remark |
| --------- | --------- | ------------------- | ------- | ------ |
| id        | string    | valid XML ids       | \-      |        |
| depart    | float(s)  | ≥0                  | \-      |        |
| type      | string    | any declared vType  |         |        |
| color     | rgb color |                     |         |        |

## Visualization

Containers are rendered in the GUI with the configured detail-level.
When assigning a type with attribute *imgFile*, the container may be
rendered with an image.

# Simulation input

## Transports

Transports define the start and end point of a movement with a single
mode of transport (e.g. a truck, train or a ship). They are child
elements of plan definitions.

| Attribute | Type   | Range                     | Default | Remark                                                 |
| --------- | ------ | ------------------------- | ------- | ------------------------------------------------------ |
| from      | string | valid edge ids            | \-      | id of the start edge                                   |
| to        | string | valid edge ids            | \-      | id of the destination edge                             |
| lines     | list   | valid line or vehicle ids | \-      | list of vehicle alternatives to take for the transport |

the route to take is defined by the vehicle.

## Tranships

Tranships define a direct transfer of containers between two points.
Thereby, containers do not move along edges. They move in a straight
line, no matter if the line is crossing buildings or anything else. They
are child elements of plan definitions.

| Attribute  | Type       | Range          | Default | Remark                                  |
| ---------- | ---------- | -------------- | ------- | --------------------------------------- |
| edges      | list       | valid edge ids | \-      | id of the edges to tranship             |
| from       | string     | valid edge ids | \-      | id of the start edge                    |
| to         | string     | valid edge ids | \-      | id of the destination edge              |
| speed      | float(m/s) | \>0            | \-      |                                         |
| departPos  | float(m)   |                | 0       | initial position on the starting edge   |
| arrivalPos | float(m)   |                | \-1     | arrival position o the destination edge |

You can define either a list of "edges" to travel or a "from" and a "to"
edge. In the former case, only the first and the last edge will be
considered. The container will move straight from the first edge to last
edge. In the latter case, the container will from straight from the edge
"from" to the edge "to".

## Stops

Stops define a delay until the next element of a plan is started. They
can be used to model containers beeing stored in a storage place,
harbour or anything else. A [container
stop](../Specification/Logistics.md#container_stops) can be used to
modal that storage place in the network (similar to a [bus stop for
public transport](../Simulation/Public_Transport.md). Stops for
containers follow the specification at
[Specification\#Stops](../Specification.md#stops). However, only
the attributes *lane*, *duration*, *until* and *startPos* are evaluated.
*startPos* defines the position on the lane where the container is
supposed to stop. Using these attributes it is possible to model
activities with a fixed duration as well as those with a fixed end time.

# Simulation behavior

A container is starting its life at its depart time on the source (resp.
first) edge of the first tranship, transport or stop. It tries to start
the next step of its plan.

## Transport

The container checks whether a vehicle with a line from the given list
is stopping at the given edge. If such a vehicle exists and the
container is positioned between the start and end position of the
vehicle's stop, the container will be loaded onto the vehicle. If such a
vehicle exists but the container is not positioned between the start and
end position of the vehicle's stop, the container will still be loaded
if the vehicle is triggered by the a container and the distance between
container and vehicle is at most 10 metres. It will not be checked
whether the vehicle has the aspired destination on the current route.
The first time the vehicle stops (on a well defined stop) at the
destination edge, the transport is finished and the container proceeds
with the next step in the plan.

## Tranship

The container moves from the "departPos" of the "from" edge to the
"arrivalPos" of the "to" edge. Thereby, the container is not following
edges. Instead, it makes a straight movement between these two points.
Any obstacles, like buildings, will be ignored.

## Stopping

The container stops for the maximum of *currentTime+duration* and
*until*.

# Example

The following is an example for a container which gets transhiped to a
train station, gets transported by a train, gets unloaded and transhiped
to a place, where it's stored (stopped) for some time and finally gets
transported again.

```
<routes>
    <container id="container0" depart="0">
        <tranship from="2/3to1/3" to="1/3to0/3" departPos="80" arrivalPos="55"/>
        <transport from="1/3to0/3" to="0/4to1/4" lines="train0"/>
        <tranship from="0/4to1/4" to="1/4to2/4" arrivalPos="30"/>
        <stop lane="1/4to2/4_0" duration="20" startPos="40"/>
        <transport from="1/4to2/4" to="3/4to4/4" lines="truck0"/>
    </container>

    <vehicle id="train0" depart="50">
        <route edges="1/4to1/3 1/3to0/3 0/3to0/4 0/4to1/4 1/4to1/3"/>
        <stop containerStop="containerStop0" until="120" duration="10"/>
        <stop containerStop="containerStop1" until="180" duration="10"/>
    </vehicle>

    <vehicle id="truck0" depart="containerTriggered">
        <route edges="1/4to2/4 2/4to3/4 3/4to4/4" departPos="30"/>
        <stop lane="1/4to2/4_0" duration="20" startPos="40" endPos="60"/>
    </vehicle>

</routes>
```

# Planned features

The following features are not yet implemented.

## Plan

A containers plan has the same role as a vehicles route: A container may
reference a plan or a distribution of plans using the attribute
<span class="inlxml">plan</span>. Likewise, a container may include a
plan or a distribution of plans as a child element. A plan is a sequence
of stages (transports, tranships and
[stops](../Specification.md#stops)).

| Attribute   | Type  | Range | Default | Remark                                                                                                                                                             |
| ----------- | ----- | ----- | ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| probability | float | ≥0    | 1       | this is only evaluated if a container has multiple plans, the probability values of all plans do not have to add to 1, they are scaled accordingly before choosing |

## ContainerFlow

A containerFlow has the same role as a flow. It allows to specify a
sequence of containers with the same plan (or distribution of plans)
with a single input element.

## Intermodal Routing

The router input is still not completely defined. There has to be a
possibility to define for the trips which vehicles / logistic lines may
be used at all and for which vehicles the route can be defined / changed
by the container.

### Trips

Trips define the start and end point of a movement with optional changes
in mode.

In definition they are identical to
[vehicles](../Specification.md#vehicles) except for the missing
route information (no route attribute and no route child is allowed).
Instead they have the following attributes

| Attribute | Type   | Range          | Default | Remark                     |
| --------- | ------ | -------------- | ------- | -------------------------- |
| from      | string | valid edge ids | \-      | id of the start edge       |
| to        | string | valid edge ids | \-      | id of the destination edge |

!!! note
    It is an error for subsequent trips to be unconnected.