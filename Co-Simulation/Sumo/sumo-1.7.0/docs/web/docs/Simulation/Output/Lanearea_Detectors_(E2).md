---
title: Simulation/Output/Lanearea Detectors (E2)
permalink: /Simulation/Output/Lanearea_Detectors_(E2)/
---

# Introduction

A laneAreaDetector is used to capture traffic on an area along a lane or
lanes. In reality this would be similar to a vehicle tracking cameras.
In contrast to an [induction
loop](../../Simulation/Output/Induction_Loops_Detectors_(E1).md), a
lane-area detector has a certain length which is specified by the `length`
attribute or by the attributes `pos,endPos`. The outputs of an E2 Detector are
tailored for measuring queues of standing/jammed vehicles and it keeps
track of all vehicles which currently are on its area, see attributes `timeThreshold`, `speedThreshold` and `jamThreshold` in the table below. Further it is possible to couple the E2
detector with a traffic light, see attributes `tl` and `to`.

# Instantiating within the Simulation

A lane area detector may be defined in one of the following ways within
an additional file.

## Explicit sequence of lanes

The first variant specifies the detector's location as a sequence of `lanes`,
on which the detector resides and, optionally, a start position on the
first lane and an end position on the last lane (these may be negative
values to indicate a distance from the lane's end).

```
<additional>
   <laneAreaDetector id="<ID>" lanes="<LANE_ID1> <LANE_ID2> ... <LANE_IDN>" 
   pos="<START_POSITION_ON_FIRST_LANE>" endPos="<END_POSITION_ON_LAST_LANE>" 
   friendlyPos="<BOOL>" freq="<AGGREGATION_TIME>" file="<OUTPUT_FILE>" 
   timeThreshold="<FLOAT>" speedThreshold="<FLOAT>" jamThreshold="<FLOAT>"
   tl="<TRAFFIC_LIGHT_ID>"  to="<LANE_ID>"/>  
</additional>
```

!!! note
    For this type of specification it is required that the lanes form a continuous sequence, i.e., a link from each lane in the sequence exists to the consecutive lane.

## Anchoring lane and length

Another possible specification allows to give a single anchoring lane
plus two of the attributes `pos`, `endPos`, and `length` like this:

```
<additional>
   <laneAreaDetector id="<ID>" lane="<LANE_ID>" 
   pos="<START_POSITION_ON_LANE>" endPos="<END_POSITION_ON_LANE>" length="<DETECTOR_LENGTH>"  // <- only two of those
   friendlyPos="<BOOL>" freq="<AGGREGATION_TIME>" file="<OUTPUT_FILE>" 
   timeThreshold="<FLOAT>" speedThreshold="<FLOAT>" jamThreshold="<FLOAT>" 
   tl="<TRAFFIC_LIGHT_ID>"  to="<LANE_ID>"/>  
</additional>
```

If start position and end position are given, the detector is assumed to
lie on a single lane. If one of these is given together with a length,
the detector is extended to one direction (upstream if the end position
was given and downstream, otherwise). If ambiguities for the lane's
continuation appear the sequence is heuristically extended to the most
natural predecessor / successor lane. The specified positions "snap" to
the lane's beginning and end, avoiding pieces of covered or uncovered
lanes shorter than 0.1m.

## Attributes

The complete list of attributes is:

| Attribute Name | Value Type              | Description             |
| -------------- | ----------------------- | -------------------------------------------------------------- |
| **id**         | id (string)             | A string holding the id of the detector   |
| **lane**       | referenced lane id      | The id of the lane the detector shall be laid on. The lane must be a part of the network used. This argument excludes the argument **lanes**.   |
| **lanes**      | referenced lane id list | A space-seperated list of lane-ids which are to be covered by the detector. All lanes must be a part of the network and form a continuous sequence. This argument excludes the arguments **lane** and **length**. |
| **pos**        | float                   | The position on the first lane covered by the detector. See information about the same attribute within the detector loop description for further information. Per default, the start position is placed at the first lane's begin.   |
| **endPos**     | float                   | The end position on the last lane covered by the detector. Per default the end position is placed at the last lane's end.    |
| **length**     | float                   | The length of the detector in meters. If the detector reaches over the lane's end, it is extended to preceding / consecutive lanes.   |
| **file**       | filename                | The path to the output file. The path may be relative.   |
| freq           | int                     | The aggregation period the values the detector collects shall be summed up. Either *freq* or *tl* must be specified      |
| tl             | id                      | The [traffic light that triggers aggregation when switching](../../Simulation/Output/Traffic_Lights.md#coupled_areal_detectors). Either *freq* or *tl* must be specified   |
| to             | id                      | The id of an outgoing lane that [triggers aggregation in conjunction with traffic light switching](../../Simulation/Output/Traffic_Lights.md#coupled_areal_detectors). This is only used together with *tl*.   |
| timeThreshold  | float                   | The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting; *in s, default: 1s*.      |
| speedThreshold | float                   | The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting; *in m/s, default: 5/3.6m/s*.   |
| jamThreshold   | float                   | The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam; *in m, default: 10m*.     |
| friendlyPos    | bool                    | If set, no error will be reported if the detector is placed behind the lane. Instead, the detector will be placed 0.1 meters from the lane's end or at position 0.1, if the position was negative and larger than the lane's length after multiplication with -1; *default: false*. |
| vTypes         | string                  | space separated list of vehicle type ids to consider, "" means all; default "".  |

# Generated Output

A single data line within the output of a simulated lane area detector
looks as following (the line is not broken within the output):

```
 <interval begin="<BEGIN_TIME>" end="<END_TIME>" id="<DETECTOR_ID>" sampledSeconds="<DATA_SAMPLES>" \
    nVehEntered="<VAL>" nVehLeft="<VAL>" nVehSeen="<VAL>" meanSpeed="<MEAN_SPEED>"  meanTimeLoss="<MEAN_TIMELOSS>" \
    meanOccupancy="<MEAN_OCCUPANCY>" maxOccupancy="<MAX_OCCUPANCY>" meanMaxJamLengthInVehicles="<VAL>" meanMaxJamLengthInMeters="<VAL>" \
    maxJamLengthInVehicles="<VAL>" maxJamLengthInMeters="<VAL>" jamLengthInVehiclesSum="<VAL>" jamLengthInMetersSum="<VAL>" \
    meanHaltingDuration="<VAL>" maxHaltingDuration="<VAL>" haltingDurationSum="<VAL>" meanIntervalHaltingDuration="<VAL>" \
    maxIntervalHaltingDuration="<VAL>" intervalHaltingDurationSum="<VAL>" startedHalts="<VAL>" meanVehicleNumber="<VAL>" maxVehicleNumber="<VAL>" />
```

To explain this vast amount of measures, a short note about how a lane
area detector works is needful. A lane area detector takes note about
each vehicle that enters the area. As long as the vehicle does not leave
the area completely, its state is collected in each time step during the
interval. Each vehicle state in this case is called a "data sample" and
the output of a lane area detector is made up from all data samples of
all vehicles within the are during the complete data collection ("freq")
interval.

As a lane area detector covers a lane and vehicles are sorted on these,
it is possible to recognize jams along the detector's area and measure
them. Because more than one jam may take place at the area at one time,
the values cover as well averaged measures of all jams
("jamLengthIn...Sum") as explicite measures of the longest (maximum)
jam. For the longest jam, both averaged ("meanMaxJamLengthIn...") and
maximum ("maxJamLengthIn...") values are written. \[Note\] Note

The jam length in meters may be more than a sum of the vehicles lengths,
because the place between vehicle is also taken into account.

Besides jam computation, the durations vehicles are halting are
collected. They are both collected over the whole time span a vehicle is
on the detector area ("...HaltingDuration" and "haltingDurationSum"),
and explicite for each interval ("...IntervalHaltingDuration" and
"intervalHaltingDurationSum").

The values are described in the following table.

| Name                        | Type                 | Description                                                                                                                                                                                                 |
| --------------------------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| begin                       | (simulation) seconds | The first time step the values were collected in                                                                                                                                                            |
| end                         | (simulation) seconds | The last time step + DELTA_T the values were collected in (may be equal to begin)                                                                                                                          |
| id                          | id                   | The id of the detector (needed if several detectors share an output file)                                                                                                                                   |
| sampledSeconds              | s                    | The total time all vehicles which contributed data were on the detector. this may be fractional even if the time step is one second, because the times when the vehicle enters and leaves are interpolated. |
| nVehEntered                 | \#                   | The number of vehicles that entered the detector in the corresponding interval.                                                                                                                             |
| nVehLeft                    | \#                   | The number of vehicles that left the detector in the corresponding interval.                                                                                                                                |
| nVehSeen                    | \#                   | The number of vehicles that were on the detector in the corresponding interval (were "seen" by the detector).                                                                                               |
| meanSpeed                   | m/s                  | The mean velocity over all collected data samples.                                                                                                                                                          |
| meanTimeLoss                | s                    | The average time loss per vehicle in the corresponding interval. The total time loss can be obtained by multiplying this value with nVehSeen.                                                               |
| meanOccupancy               | %                    | The percentage (0-100%) of the detector's place that was occupied by vehicles, summed up for each time step and averaged by the interval duration.                                                          |
| maxOccupancy                | %                    | The maximum percentage (0-100%) of the detector's place that was occupied by vehicles during the interval.                                                                                                  |
| meanMaxJamLengthInVehicles  | \#vehicles           | The length of the longest jams recognized during each step, averaged over the interval duration. In vehicles that have contributed to these jams.                                                           |
| meanMaxJamLengthInMeters    | m                    | As prior, but in meters (see notes)                                                                                                                                                                         |
| maxJamLengthInVehicles      | \#vehicles           | The length of the longest jam recognized during the interval duration. In vehicles that have contributed to this jams.                                                                                      |
| maxJamLengthInMeters        | m                    | As prior, but in meters (see notes)                                                                                                                                                                         |
| jamLengthInVehiclesSum      | \#vehicles           | The sum of all lengths of all jams recognized during the interval. In vehicles that have contributed to these jams.                                                                                         |
| jamLengthInMetersSum        | m                    | As prior, but in meters (see notes)                                                                                                                                                                         |
| meanHaltingDuration         | s                    | The mean halting duration of vehicles that entered the area and are still inside or have left the area within the reported interval.                                                                        |
| maxHaltingDuration          | s                    | The maximum halting duration of vehicles that entered the area and are still inside or have left the area within the reported interval.                                                                     |
| haltingDurationSum          | s                    | The sum of all halting durations of vehicles that entered the area and are still inside or have left the area within the reported interval.                                                                 |
| meanIntervalHaltingDuration | s                    | The mean halting duration of vehicles that entered the area and are still inside or have left the area within the reported interval, counted from the interval's begin.                                     |
| maxIntervalHaltingDuration  | s                    | The maximum halting duration of vehicles that entered the area and are still inside or have left the area within the reported interval, counted from the interval's begin.                                  |
| intervalHaltingDurationSum  | s                    | The sum of all halting durations of vehicles that entered the area and are still inside or have left the area within the reported interval, counted from the interval's begin.                              |
| startedHalts                | \#                   | The number of started halts. Please note that during an interval a vehicle may stop halting and enter a new halting state.                                                                                  |
| meanVehicleNumber           | \#                   | The mean number of vehicles that were on the detector (averaged over the interval duration).                                                                                                                |
| maxVehicleNumber            | \#                   | The maximum number of vehicles that were on the detector area during the interval.                                                                                                                          |

# Further notes

- You can generate detector definitions automatically. See [output
tools](../../Tools/Output.md) for more information.
- The mean speed given by the detector is rather the length divided by
the mean travel time, so even if all vehicles drive with constant
speed the result will differ from the measurements of an induction
loop.
- Unlike the [multi-entry/multi-exit
detectors](../../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md)
this detector
  - takes into account vehicles which start or end their route on
    the detector (or enter / leave by teleport)
  - counts also vehicles which only touch the starting position of
    the detector