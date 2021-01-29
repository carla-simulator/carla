---
title: Simulation/Bluetooth
permalink: /Simulation/Bluetooth/
---

# Overview

[sumo](../sumo.md) supports the simulation of wireless onboard
devices to facilitate a sensor mechanism which relies on the detection
of radio signals (usually Bluetooth or WLAN) emitted by the vehicle.
Every vehicle can act as a sender and/or a receiving device. The
parameters of the detection (e.g. the range and detection probability)
can be configured and a log of the detection events can be written. It
is currently not possible to retrieve the detection events via
[TraCI](../TraCI.md) or react directly in the simulation to such an
event. The relevant parameters for configuring the sender and recevier
devices are described at
[sumo#communication](../sumo.md#communication). Details on the
detection process can be found in the following publication: [Behrisch,
Michael and Gurczik, Gaby (2014) Modelling Bluetooth Inquiry for SUMO.
In: SUMO2014 Modeling Mobility with Open Data. SUMO2014, 15.-16.
Mai 2014, Berlin](http://elib.dlr.de/95237/)

# Enabling Sender / Receiver devices

The Bluetooth functionality is enabled using two devices:

- **btsender**: enables this vehicle to be detected
- **btreceiver**: enables this vehicle to detector senders

Activation of these devices is explained [here](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices).

The behavior of the **btreceiver** device can be further customized using the following [sumo](../sumo.md)-Options:

- **--device.btreceiver-range** {{DT_FLOAT}} (defines the detection range in meters)
- **--device.btreceiver-offtime** {{DT_FLOAT}} (defines the minimum time between connections in seconds. This can be used to model effects of communication traffic load).

# Output

The option **--bt-output** generates an XML output file of the following form:

```
<bt-output>
    <bt id="<VEHICLE_ID>">
        <seen id="<VEHICLE_ID>" tBeg="<TIME>" observerPosBeg="<COORD>" observerSpeedBeg="<SPEED>" observerLaneIDBeg="<LANE_ID>" observerLanePosBeg="<LANE_POS>"
                                                seenPosBeg="<COORD>" seenSpeedBeg="<SPEED>" seenLaneIDBeg="<LANE_ID>" seenLanePosBeg="<LANE_POS>"
                                tEnd="<TIME>" observerPosEnd="<COORD>" observerSpeedEnd="<SPEED>" observerLaneIDEnd="<LANE_ID>" observerLanePosEnd="<LANE_POS>"
                                                seenPosEnd="<COORD>" seenSpeedEnd="<SPEED>" seenLaneIDEnd="<LANE_ID>" seenLanePosEnd="<LANE_POS>"
                                observerRoute="<EDGE_ID>+" seenRoute="<EDGE_ID>+">
            <recognitionPoint t="<TIME>" observerPos="<COORD>" observerSpeed="<SPEED>" observerLaneID="<LANE_ID>" observerLanePos="<LANE_POS>"
                                            seenPos="<COORD>" seenSpeed="<SPEED>" seenLaneID="<LANE_ID>" seenLanePos="<LANE_POS>"/>

            ... more recognition points ...

        </seen>

        ... more contacts of the same vehicle ...
    </bt>

    ... more data of of sensor vehicles ...
</bt-output>
```

If the option **--device.btreceiver.all-recognitions** is not given only the first recognition point will be
printed for each encounter. The meanings of the written values are given
in the following table.

| Name               | Type        | Description                                                                         |
| ------------------ | ----------- | ----------------------------------------------------------------------------------- |
| id\@bt              | ID          | The id of the observing vehicle (receiver)                                          |
| id\@seen            | ID          | The id of the detected vehicle (sender)                                             |
| tBeg               | s           | The time the sender entered the detection range                                     |
| observerPosBeg     | x,y in m    | Cartesian coordinates of the observer when the sender entered the range             |
| observerSpeedBeg   | m/s         | Speed of the observer when the sender entered the range                             |
| observerLaneIDBeg  | ID          | Lane id of the observer when the sender entered the range                           |
| observerLanePosBeg | m           | Longitudinal position on the lane of the observer when the sender entered the range |
| seenPosBeg         | x,y in m    | Cartesian coordinates of the sender when it entered the range                       |
| seenSpeedBeg       | m/s         | Speed of the sender when it entered the range                                       |
| seenLaneIDBeg      | ID          | Lane id of the sender when it entered the range                                     |
| seenLanePosBeg     | m           | Longitudinal position on the lane of the sender when it entered the range           |
| tEnd               | s           | The time the sender left the detection range                                        |
| observerPosEnd     | x,y in m    | Cartesian coordinates of the observer when the sender left the range                |
| observerSpeedEnd   | m/s         | Speed of the observer when the sender left the range                                |
| observerLaneIDEnd  | ID          | Lane id of the observer when the sender left the range                              |
| observerLanePosEnd | m           | Longitudinal position on the lane of the observer when the sender left the range    |
| seenPosEnd         | x,y in m    | Cartesian coordinates of the sender when it left the range                          |
| seenSpeedEnd       | m/s         | Speed of the sender when it left the range                                          |
| seenLaneIDEnd      | ID          | Lane id of the sender when it left the range                                        |
| seenLanePosEnd     | m           | Longitudinal position on the lane of the sender when it left the range              |
| observerRoute      | list of IDs | the route of the observing vehicle                                                  |
| seenRoute          | list of IDs | the route of the sender vehicle                                                     |
| t                  | s           | The time the observer detected the sender                                           |
| observerPos        | x,y in m    | Cartesian coordinates of the observer when it detected the sender                   |
| observerSpeed      | m/s         | Speed of the observer when it detected the sender                                   |
| observerLaneID     | ID          | Lane id of the observer when it detected the sender                                 |
| observerLanePos    | m           | Longitudinal position on the lane of the observer when it detected the sender       |
| seenPos            | x,y in m    | Cartesian coordinates of the sender when it was detected                            |
| seenSpeed          | m/s         | Speed of the sender when it was detected                                            |
| seenLaneID         | ID          | Lane id of the sender when it left the range                                        |
| seenLanePos        | m           | Longitudinal position on the lane of the sender when it was detected                |

# Simulating Vehicle-to-Infrastructure (V2I)

To simulate detection events between vehicles and a road-side-unit
(RSU), it is convenient to place a stopped vehicle at the side of the
road. This can be accomplished by by setting [attribute
*parking="true"*](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops).