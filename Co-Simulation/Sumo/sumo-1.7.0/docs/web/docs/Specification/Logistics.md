---
title: Specification/Logistics
permalink: /Specification/Logistics/
---

The concept of logistics is mostly realised by
[containers](../Specification/Containers.md) and [container
stops](../Specification/Logistics.md#container_stops).

# Container stops

Container stops can be used to simulate transhipment stations, harbours
and other places for transhipping and storing containers/goods.
Similarly to the concept of [bus
stops](../Simulation/Public_Transport.md), container stops are
[stops](../Specification.md#stops) at which containers can be
loaded onto or unloaded from a vehicle. Vehicles use the same advanced
approaching behavior at container stops as at bus stops. Definitions of
container stop locations in SUMO have the following format:
`<containerStop id="<CONTAINER_STOP_ID>" lane="<LANE_ID>" startPos="<STARTING_POSITION>" endPos="<ENDING_POSITION>" [line="<LINE_ID>[ <LINE_ID>]*"]/>`. That means that a container stop is an area
on a lane. The parameters have the following meanings:

| Attribute Name | Value Type  | Value Range                                         | Default     | Description     |
| -------------- | ----------- | --------------------------------------------------- | ----------- | --------------- |
| **id**         | string      | id                                                  |             | The name of the container stop; must be unique    |
| **lane**       | string      | valid lane id                                       |             | The name of the lane the container stop shall be located at     |
| **startPos**   | float       | \-lane.length < x < lane.length (negative values count backwards from the end of the lane) | 0           | The begin position on the lane (the lower position on the lane) in meters                                                   |
| endPos         | float       | \-lane.length < x < lane.length (negative values count backwards from the end of the lane) | lane.length | The end position on the lane (the higher position on the lane) in meters, must be larger than *startPos* by more than 0.1m  |
| friendlyPos    | bool        | *true,false*                                                                                 | *false*     | whether invalid stop positions should be corrected automatically (default *false*)                                          |
| name           | string      | simple String                                                                                |             | Container stop name. This is only used for visualization purposes.                                                          |
| lines          | string list | A list of names separated by spaces (' ')                                                    |             | meant to be the names of the logistic lines that stop at this container stop. This is only used for visualization purposes. |

!!! caution
    Please note that container stops must be added to a config via the *--additional-files* parameter

Vehicles must be informed that they must stop at a container stop. The
following example shows how this should be done:

```
<vtype id="truck" accel="2.6" decel="4.5" sigma="0.5" length="15" maxspeed="70" color="1,1,0"/>
    <vehicle id="0" type="truck" depart="0" color="1,1,0">
        <route edges="2/0to2/1 2/1to1/1 1/1to1/2 1/2to0/2 0/2to0/1 0/1to0/0 0/0to1/0 1/0to2/0 2/0to2/1"/>
        <stop containerStop="containerStop1" duration="20"/>
        <stop containerStop="containerStop2" duration="20"/>
        <stop containerStop="containerStop3" duration="20"/>
        <stop containerStop="containerStop4" duration="20"/>
    </vehicle>
```

What is defined here is a vehicle named "0" being a "truck". "truck" is
a referenced type declared earlier. The vehicle has an embedded route
(written by hand in this case) and a list of stop places. Each stop
place is described by two attributes, "containerStop" and "duration"
where "containerStop" is the name of the container stop the vehicle
shall halt at and "duration" is the time the vehicle shall wait at the
container stop in seconds. Please note that the order of container stops
the vehicle shall halt at must be correct.

You may also let a vehicle stop at another position than a container
stop. The short definition of a vehicle's stop is:
`<stop containerStop="<CONTAINER_STOP_ID>" | ( lane="<LANE_ID>" endPos="<POSITION_AT_LANE>" ) duration="<HALTING_DURATION>"/>`. This means you can either use a
container stop or a lane position to define where a vehicle has to stop.
For a complete list of attributes for the "stop"-element of a vehicle
see [Specification\#Stops](../Specification.md#stops).