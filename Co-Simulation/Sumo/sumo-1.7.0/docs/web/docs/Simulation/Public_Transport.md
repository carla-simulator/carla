---
title: Simulation/Public Transport
permalink: /Simulation/Public_Transport/
---
# Introduction
Most public transport uses fixed routes (busses, trams and trains) and this page describes how to model this type of transport. For demand responsive transport that is more flexible see [Taxi](Taxi.md).

The descriptions below describe how to model busses but the same rules apply for trams, subways and trains.

# Bus Stops

It is possible to define positions of bus stops and let vehicles
("busses") stop at these positions for a pre-given time. Definitions of
bus stop locations in SUMO have the following format: `<busStop id="<BUS_STOP_ID>" lane="<LANE_ID>" startPos="<STARTING_POSITION>" endPos="<ENDING_POSITION>" [lines="<LINE_ID>[ <LINE_ID>]*"]/>`. That means that a bus
stop is an area on a lane. The parameters have the following meanings:

| Attribute Name | Value Type  | Value Range                                                                                  | Default     | Description      |
| -------------- | ----------- | ----------------------------------------------------- | ----------- | --------------------------------------------------------------- |
| **id**         | string      | id                                                                                           |             | The ID of the bus stop; must be unique                                                                                     |
| **lane**       | string      | valid lane id                                                                                |             | The name of the lane the bus stop shall be located at                                                                      |
| startPos       | float       | \-lane.length < x < lane.length (negative values count backwards from the end of the lane) | 0           | The begin position on the lane (the lower position on the lane) in meters                                                  |
| endPos         | float       | \-lane.length < x < lane.length (negative values count backwards from the end of the lane) | lane.length | The end position on the lane (the higher position on the lane) in meters, must be larger than *startPos* by more than 0.1m |
| friendlyPos    | bool        | *true,false*                                                                                 | *false*     | whether invalid stop positions should be corrected automatically (default *false*)                                         |
| name           | string      | simple String                                                                                |             | Bus stop name. This is only used for visualization purposes.                                                               |
| lines          | string list | A list of names separated by spaces (' ')                                                    |             | meant to be the names of the bus lines that stop at this bus stop. This is only used for visualization purposes.           |

!!! caution
    Please note that bus stops must be added to a config via the *--additional-files* parameter (see {{AdditionalFile}}).

!!! note
    The alias `<trainStop>` may be used anywhere instead of `<busStop>` and has the exact same functionality.

## Access Lanes

Each named  may have additional child elements to model access from
other parts of the network (i.e. road access to a stop on the rail
network. This takes the following form:

```
<busStop id="myStop" lane="A_0" startPos="230" endPos="250">
   <access lane="B_0" pos="150"/>
</busStop>
```

Access information is added automatically for railway stops when
[importing public transport information from
OSM](../Tutorials/PT_from_OpenStreetMap.md#initial_network_and_public_transit_information_extraction)

| Attribute Name | Value Type | Value Range         | Description                                                    |
| -------------- | ---------- | ------------------- | -------------------------------------------------------------- |
| **lane**       | string     | id                  | The name of the lane from which this stop may also be accessed |
| **pos**        | float      | position along lane | The position along the lane from which the stop is reached     |

# Letting Vehicles stop at a bus stop

Vehicles must be informed that they must stop at a bus stop. The
following example shows how this can be done (taken from {{SUMO}}*/examples/sumo/busses*):

```
<additional xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/additional_file.xsd">
    <busStop id="busstop1" lane="2/1to1/1_0" startPos="20" endPos="40" lines="100 101 102"/>
    <busStop id="busstop2" lane="1/2to0/2_0" startPos="20" endPos="40" lines="100 101"/>
    <busStop id="busstop3" lane="0/1to0/0_0" startPos="20" endPos="40" lines="100 101 102"/>
    <busStop id="busstop4" lane="1/0to2/0_0" startPos="20" endPos="40" lines="100 101"/>

    <vType id="BUS" accel="2.6" decel="4.5" sigma="0" length="12" minGap="3" maxSpeed="70" color="1,1,0" guiShape="bus"/>

    <vehicle id="0" type="BUS" depart="0" color="1,1,0">
        <route edges="2/0to2/1 2/1to1/1 1/1to1/2 1/2to0/2 0/2to0/1 0/1to0/0 0/0to1/0 1/0to2/0 2/0to2/1"/>
        <stop busStop="busstop1" duration="20"/>
        <stop busStop="busstop2" duration="20"/>
        <stop busStop="busstop3" duration="20"/>
        <stop busStop="busstop4" duration="20"/>
    </vehicle>
</additional>
```

What is defined here is a vehicle named "0" being a "BUS". "BUS" is a
referenced type declared earlier. The vehicle has an embedded route
(written by hand in this case) and a list of stop places. Each stop
place is described by two attributes, "bus_stop" and "duration" where
"bus_stop" is the name of the bus stop the vehicle shall halt at and
"duration" is the time the vehicle shall wait at the bus stop in
seconds. Please note that the order of bus stops the vehicle shall halt
at must be correct.

For a complete list of attributes for the "stop"-element of a vehicle
see
[Definition_of_Vehicles,_Vehicle_Types,_and_Routes\#Stops](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops).

## Incomplete Route Definition

It is also possible to define a bus route without defining all
intermediate edges:

```
   <trip id="0" type="BUS" depart="0" color="1,1,0" from="2/0to2/1" to="2/0to2/1">
       <stop busStop="busstop1" duration="20"/>
       <stop busStop="busstop2" duration="20"/>
       <stop busStop="busstop3" duration="20"/>
       <stop busStop="busstop4" duration="20"/>
   </trip>
```
The vehicle will take the fastest path between *from*-edge and *to*-edge
that visits all stops in their correct order.

Even the trip attributes 'from' and 'to' can be omitted to let the bus start at the first stop and end at the last. Using `departPos="stop"`, the vehicle will be inserted directly at the first stop in it's route:
```
   <trip id="0" type="BUS" depart="0" color="1,1,0" departPos="stop">
       <stop busStop="busstop1" duration="20"/>
       <stop busStop="busstop2" duration="20"/>
       <stop busStop="busstop3" duration="20"/>
       <stop busStop="busstop4" duration="20"/>
   </trip>
```

# Public Transport Schedules
In most cases, public transport runs according to a fixed schedule. Such a schedule must be defined using the `until` attribute in the stops. Vehicles in the simulation will adhere to the schedule as they cannot leave a stop before the until-time is reached but they may still be delayed due to traffic. When `until` and `duration` are used, duration defines the minimum stopping time which can also cause delayed departures when already arriving late at a stop.

!!! note
    Defining a public transport schedule is necessary for [intermodal routing](../IntermodalRouting.md).

## Single vehicles and trips
When defining until values for a vehicle and trip, the values denote absolute simulation. Times. Note, that seconds or human-readable times may be used.

```
<trip="bus" from = "beg" to ="end" line="bus" depart="6:0:0">
     <stop busStop="busStopA" until="6:30:00"/>
     <stop busStop="busStopB" until="6:32:30"/>
     <stop busStop="busStopC" until="23700"/>
</trip>
```

## Flows
When defining a vehicle flow the until-times are absolute times for the first vehicle in the flow. For all later vehicles, the times will be shifted according to later departure times (period * vehicleIndex).

The example below defines a flow that inserts two vehicles. The first vehicle will stop until 10,110,210 and the second vehicle will stop until 310,410,510:
```xml
    <flow id="bus" from = "beg" to ="end" line="bus" begin="0" end="301" period="300">
                <stop busStop="busStopA" until="10"/>
                <stop busStop="busStopB" until="110"/>
                <stop busStop="busStopC" until="210"/>
    </flow>
```

## Stops in a stand-alone route
Stops may also be part of a route that is defined by itself (with an id) and referenced by vehicles or flows. In that case the until times will be interpreted as 'times after vehicle departure'.

The example below defines a flow that inserts two vehicles. The first vehicle will depart at 500 and stop until 510,610,710. The second vehicle will depart at 800 and stop until 810,910,1010:

```xml
<route id="busRoute" edges="A B C D E">
                <stop busStop="busStopA" until="10"/>
                <stop busStop="busStopB" until="110"/>
                <stop busStop="busStopC" until="210"/>
</route>

<flow id="bus" route="busRoute" line="bus" begin="500" end="801" period="300"/>

```

# Stopping without defining a bus stop

You may also let a vehicle stop at another position than a bus stop. The
short definition of a vehicle's stop is: `<stop bus_stop="<BUS_STOP_ID>" | ( lane="<LANE_ID>" endPos="<POSITION_AT_LANE>" ) duration="<HALTING_DURATION>"/>`.
This means you can either use a bus stop or a lane position to define
where a vehicle has to stop. For a complete list of attributes for the
"stop"-element of a vehicle see
[Definition_of_Vehicles,_Vehicle_Types,_and_Routes\#Stops](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops).

# Transport Schedules
To define a schedule where buses 

# Passengers

[Additional definitions are necessary if the persons using the public
transport lines shall be modeled as
well](../Specification/Persons.md#rides).

To make use of [intermodal routing, additional constrains on public
transport definition must be met.](../IntermodalRouting.md)

# Importing Public Transport

The [osmWebWizard tool](../Tutorials/OSMWebWizard.md) can be used
to import public transport data from OpenStreetMap for a selected area.

This import uses an intermediate data format to describe public
transport lines. The usage of this line data is explained at
[Tutorials/PT_from_OpenStreetMap](../Tutorials/PT_from_OpenStreetMap.md)