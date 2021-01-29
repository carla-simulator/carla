---
title: Simulation/Output/SSM Device
permalink: /Simulation/Output/SSM_Device/
---

## Equipping vehicles
A vehicle can be equipped with an SSM Device which logs the conflicts of the vehicle and other traffic participants (currently only vehicles) and corresponding safety surrogate measures. To attach an SSM device to a vehicle, the [standard device-equipment procedures](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices) can be applied using `<device name>=ssm`.

For instance, a single vehicle can be equipped (with a device parametrized by default values) as in the following minimal example

```
<routes>
    ...
    <vehicle id="v0" route="route0" depart="0">
        <param key="has.ssm.device" value="true"/>
    </vehicle>
    ....
</routes>
```

The SSM device generates an output file (one for each vehicle named `ssm_<vehicleID>.xml` per default, but several vehicles may write to the same file). The top level elements of the generated file are 

`<conflict begin="<log-begin-time>" end="<log-end-time>" ego="<equipped-vehicleID>" foe="<opponent-vehicleID>"> ... </conflict>`.

The detail of information given for each conflict and the criteria to qualify an encounter as a conflict (i.e., produce a corresponding `conflict` element in the output) can be customized by a number of generic parameters to the vehicle or device, resp.. A full parametrization (redundantly assigning the default values, here) could look as follows:

```
<routes>
    ...
    <vehicle id="v0" route="route0" depart="0">
        <param key="has.ssm.device" value="true"/>
        <param key="device.ssm.measures" value="TTC DRAC PET"/>
        <param key="device.ssm.thresholds" value="3.0 3.0 2.0"/> 
        <param key="device.ssm.range" value="50.0" />
        <param key="device.ssm.extratime" value="5.0" />
        <param key="device.ssm.file" value="ssm_v0.xml" />
        <param key="device.ssm.trajectories" value="false" />
        <param key="device.ssm.geo" value="false" />
    </vehicle>
    ....
</routes>
```

## Parameters

The possible parameters are summarized in the following table

| Parameter  | Type  | Default  | Remark  |
|---|---|---|---|
| measures  | list of strings  | All available SSMs  | This space-separated list of SSM-identifiers determines, which encounter-specific SSMs are calculated for the equipped vehicle's encounters and which global measures are recorded (see [below](#available_ssms))   |
| thresholds  | list of floats  | default thresholds for specified measures  | This space-separated list of SSM-thresholds determines, which encounters are classified as conflicts (if their measurements exceed a threshold) and thus written to the output file as a `<conflict>`-element. The default thresholds for the available SSMs are: TTC<3.0[s], DRAC>3.0[m/s^2], PET<2.0[s], BR>0.0[m/s^2], SGAP<0.2[m], TGAP<[0.5s]. This list is required to have the same length as the list of measures if given.<br><br>**Note:** Currently the global measures are recorded as a single timeline for the whole simulation span and thresholds have only effect insofar a leader is looked for in the distance corresponding to the SGAP and, respectively, TGAP values.   |
| range  | double  | 50.0 [m]  | The devices detection range in meters. Other vehicles are tracked as soon as the are closer than `<range>` to the the equipped vehicle *along the road-network*. A tree search is performed to find all vehicles up to range upstream and downstream to the vehicle's current position. Further, for all downstream junctions in range, an upstream search for the given range is performed.  |
| extratime  | double  | 5.0 [s]  | The extra time that an encounter is tracked on after not being associated to a potential conflict (either after crossing the conflict area, deviating from a common route, changing lanes, or because vehicles leave the device range, etc.).  |
| file  | string  | "ssm_<equipped_vehicleID\>.xml"  | The filename for storing the conflict information of the equipped vehicle. Several vehicles may write to the same file. Conflicts of a single vehicle are written in the order of the log-begin of the encounter.   |
| trajectories  | bool  | false  | Whether the full time lines of the different measured values shall be written to the output. This includes logging the time values, encounter types, vehicle positions and velocities, values of the selected SSMs, and associated conflict point locations. If turned off (default) only the extremal values for the selected SSMs are written.  |
| geo  | bool  | false  | Whether the positions in the output file shall be given in the original coordinate reference system of the network (if available).  |

## Encounter types
Different types of encounters, e.g. crossing, merging, or lead/follow situations, may imply different calculation procedures for the safety measures. Therefore the SSM-device keeps track of these classifications and provides them in the output to allow the correct interpretation of the corresponding values.

The following table lists the different encounter types along with their codes, which will appear in the output file. 

| Code | Name                       | Description           | [Space Gap](#space_gap_definitions) | Speed Difference | [Conflict Entry Point](#conflict_definitions) |
|------|----------------------------|-----------------------|-------------------------------------|--------------------|-----------------------------------------------|
| 0    | NOCONFLICT_AHEAD           | Foe vehicle is closer than range, but not on a lane conflicting with the ego's route ahead.                                                                                                                               ||||
| <font color="lightgray">1    | <font color="lightgray">FOLLOWING                  | <font color="lightgray">General follow/lead situation (incomplete type, used only internally).                                                                            ||||
| 2    | FOLLOWING_FOLLOWER         | Ego vehicle is following the foe vehicle.                                                                                                                                                                                 | foeBack - egoFront | egoSpeed - foeSpeed | foeBack |
| 3    | FOLLOWING_LEADER           | Foe vehicle is following the ego vehicle.                                                                                                                                                                                 | egoBack - foeFront | foeSpeed - egoSpeed | egoBack |
| 4    | ON_ADJACENT_LANES          | Foe vehicle is on a neighboring lane of the ego vehicle's lane, driving in the same direction.                                                                                                                            ||||
| <font color="lightgray">5    | <font color="lightgray">MERGING                    | <font color="lightgray">Ego and foe share an upcoming edge of their routes while the merging point for the routes is still ahead (incomplete type, only used internally). ||||
| 6    | MERGING_LEADER             | As 5. The estimated arrival at the merge point is earlier for the foe than for the ego vehicle.                                                                                                                           | foeEntryDist | foeSpeed | mergePoint|
| 7    | MERGING_FOLLOWER           | As 5. The estimated arrival at the merge point is earlier for the ego than for the foe vehicle.                                                                                                                           | egoEntryDist | egoSpeed | mergePoint|
| 8    | MERGING_ADJACENT           | As 5. The vehicles' current routes lead to adjacent lanes on the same edge.                                                                                                                                               ||||
| <font color="lightgray">9    | <font color="lightgray">CROSSING                   | <font color="lightgray">Ego's and foe's routes have crossing edges (incomplete type, only used internally)                                                                ||||
| 10   | CROSSING_LEADER            | As 6. The estimated arrival of the ego at the conflict point is earlier than for the foe vehicle.                                                                                                                         | foeEntryDist | foeSpeed | foeCrossingPoint|
| 11   | CROSSING_FOLLOWER          | As 6. The estimated arrival of the foe at the conflict point is earlier than for the ego vehicle.                                                                                                                         | egoEntryDist | egoSpeed | egoCrossingPoint|
| <font color="lightgray">12   | <font color="lightgray">EGO_ENTERED_CONFLICT_AREA  | <font color="lightgray">The encounter is a possible crossing conflict, and the ego vehicle has entered the conflict area. (Is currently not logged -> TODO)               | foeEntryDist | foeSpeed | foeCrossingPoint|
| <font color="lightgray">13   | <font color="lightgray">FOE_ENTERED_CONFLICT_AREA  | <font color="lightgray">The encounter is a possible crossing conflict, and the foe vehicle has entered the conflict area. (Is currently not logged -> TODO)               | egoEntryDist | egoSpeed | egoCrossingPoint|
| 14   | EGO_LEFT_CONFLICT_AREA     | The encounter has been a possible crossing conflict, but the ego vehicle has left the conflict area.                                                                                                                      ||||
| 15   | FOE_LEFT_CONFLICT_AREA     | The encounter has been a possible crossing conflict, but the foe vehicle has left the conflict area.                                                                                                                      ||||
| <font color="lightgray">16   | <font color="lightgray">BOTH_ENTERED_CONFLICT_AREA | <font color="lightgray">The encounter has been a possible crossing conflict, and both vehicles have entered the conflict area (auxiliary type, only used internally, is evaluated to BOTH_LEFT_CONFLICT_AREA or to COLLISION). ||||
| 17   | BOTH_LEFT_CONFLICT_AREA    | The encounter has been a possible crossing conflict, but both vehicle have left the conflict area.                                                                                                                        ||||
| 18   | FOLLOWING_PASSED           | The encounter has been a following situation, but is not active any more.                                                                                                                                                 ||||
| 19   | MERGING_PASSED             | The encounter has been a merging situation, but is not active any more.                                                                                                                                                   ||||
| 20   | ONCOMING                   | The vehicles are driving [towards each other on the same lane](../../Simulation/OppositeDirectionDriving.md).                                                                                                             | foeFront - egoFront | egoSpeed + foeSpeed | midpoint between vehicles|
| <font color="lightgray">111  | <font color="lightgray">COLLISION                  | <font color="lightgray">Collision (currently not implemented, might be differentiated further).                                                                           ||||


## Space Gap Definitions
- front: position of front bumper of the vehicle as offset from the start of its lane
- back: position of back bumper of the vehicle as offset from the start of its lane

!!! note
    When ego and foe vehicles are on subsequent lanes, the gaps are computed by adding the distances of the intermediate lanes

## Conflict Definitions
For each conflict there is an entry point. For merging and crossing conflicts there is also an exit point.
The points may be slightly different for both ego and foe vehicle because the point is computed in reference to the vehicle front or rear bumper whereas a collision could happen with another part of the vehicle.

- entryDist: distance between the front bumper of the vehicle and the entry point along the lane sequence
- exitDist for merging and oncoming encounters: entryDist + vehicleLength
- exitDist for crossing encounters: entryDist + followerLength + leaderWidth
- mergePoint: start of the first common lane of ego and foe vehicle
- crossingPoint: point where the lane geometry sequence (center lines) of both vehicles intersects
- egoCrossingPoint: crossing point shifted upstream along the ego lanes by half of the foe vehicle width
- foeCrossingPoint: crossing point shifted upstream along the foe lanes by half of the ego vehicle width

## Available SSMs
Currently, the following safety surrogate measures are implemented:

- [TTC](#ttc) (time to collision)
- [DRAC](#drac) (deceleration rate to avoid a crash)
- [PET](#pet) (post encroachment time)

Further, the following additional safety-relevant output can be generated, which will not be linked to a specific encounter:

- [BR](#br) (brake rate)
- [SGAP](#sgap) (spacing)
- [TGAP](#tgap) (time headway)
For the selection in the device's output, the abbreviations have to be used.

Basically, we distinguish between three types of encounters for two vehicles:

- Lead/follow situation
- Crossing situation
- Merging Situation

Please note that some SSMs only apply to a specific encounter or are computed differently for different encounters.
For crossing and merging situations, we consider "expected" entry and exit times with respect to the conflict zone. 
For the calculation of those times for the approaching vehicles, we take into account the current deceleration of the 
vehicles, if the vehicle is not decelerating, the current speed is extrapolated as a constant (i.e., acceleration is only considered if it is negative).

For some reference to definitions of SSMs see for instance [Guido et al. (2011) "Safety performance measures: a comparison between microsimulation and observational data"] or [Mahmud et al. (2016) "Application of proximal surrogate indicators for safety evaluation: A review of recent developments and research needs"]

### TTC
The time-to-collision is defined for all follow-lead situations for which the follower is faster than the leader. It is given as

```
TTC = space_gap/speed-difference.
```

For a crossing or merging situation the TTC is only considered defined if for the case that the expected conflict area exit time of the vehicle A is larger than the expected conflict area entry time for vehicle B, where A is the vehicle with the smaller expected conflict area entry time. If this is the case the TTC is defined as

```
TTC = B's distance to conflict area entry / B's current speed.
```

### DRAC
For a lead/follow-situation the DRAC (deceleration to avoid a crash) where the follower vehicle's speed is larger than the leader vehicle's speed is defined as

```
DRAC = 0.5*speed_difference^2/space_gap.
```

For a crossing situation the DRAC is considered defined only if the expected conflict area exit time tA for the first vehicle (A) is larger than the *linearly extrapolated* conflict area entry time for the second vehicle (B). In that case, the DRAC is defined as follows:

```
DRAC = 2*(speedB - distConflictB/tA)/tA.
```

This value is chosen such that constant deceleration with the corresponding rate would imply that B enters the conflict area exactly at time tA, when vehicle A leaves it.

For a merging situation, both variants for the DRAC calculation must be tested and the minimal result should be applied as the appropriate value for the DRAC.

!!! note
    This has still to be implemented, currently only one variant is used.

### PET
For merging and crossing situations, the PET (post encroachment time) is defined as the difference of the leading vehicle's conflict area exit time tA and the following vehicle's conflict area entry time tB:

```
PET = tB - tA.
```

For lead/follow situations, no PET is calculated.

!!! note
    Some deficiencies of the current implementation state:

    1) Currently the geometry of the conflict area is considered simplified, considering crossing lanes to be orthogonal.

    2) The merging area is not treated (the "conflict area" for merging conflicts is actually considered as the cross section at the beginning of the common target lane.

    3) DRAC calculation for merging conflicts is still incomplete
    
    4) Lateral conflicts for opposite or neighboring lane's traffic are ignored


### BR
The brake rate is recorded at each simulation step. If the vehicle accelerates, a value of 0.0 is logged.
### SGAP
The spacing is measured as the bumper to bumper distance to the ego vehicle's leader minus the vehicle's [minGap](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
### TGAP
The time headway to the leading vehicle equals spacing/speed.

## Output
The output for an SSM device is written to a file specified by the parameter `device.ssm.file` in the routes definition file, see [above](#equipping_vehicles).
The extent of output can be controlled by the parameters `device.ssm.measures` (which SSMs to report) and `device.ssm.trajectories` (whether to report complete trajectories for vehicles, the tracked SSMs, and the encounter types).

The resulting file contains a root element `<SSMLog>`, which contains several `<conflict>` elements and one `<globalMeasures>` element. Each reported `<conflict>` corresponds to a tracked encounter, during which an SSM's criticality threshold was exceeded during the simulation. The `<globalMeasures>` Elementholds a timeline for all simulated times and the corresponding values for the selected measures, which are not associated to specific conflicts, see [Available_SSMs](#available_ssms).

An example for the contents of an output file:

```
<SSMLog>
     <conflict begin="6.50" end="13.90" ego="ego1" foe="foe1">
         <timeSpan values="6.50 6.60 6.70 6.80 6.90 7.00 7.10 ..."/>
         <typeSpan values="10 10 10 10 10 10 10 ..."/>
         <egoPosition values="98.35,61.20 98.35,60.20 98.35,59.25 ..."/>
         <egoVelocity values="0.00,-10.23 0.00,-9.78 0.00,-9.33 ..."/>
         <foePosition values="76.31,48.35 77.59,48.35 78.82,48.35 ..."/>
         <foeVelocity values="13.02,0.00 12.57,0.00 12.12,0.00 ..."/>
         <conflictPoint values="99.23,49.46 99.23,49.46 99.23,49.46 ..."/>
         <TTCSpan values="1.78 1.74 1.70 1.67 1.63 1.60 1.56 ..."/>
         <minTTC time="7.40" position="99.23,49.46" type="10" value="1.48"/>
         <DRACSpan values="3.66 3.61 3.56 3.50 3.44 3.37 3.30 ..."/>
         <maxDRAC time="6.50" position="99.23,49.46" type="10" 
value="3.66"/>
         <PET time="9.42" position="99.23,49.46" type="17" value="0.72"/>
     </conflict>
     <conflict begin="21.50" end="27.20" ego="ego1" foe="foe2">
         ...
     </conflict>
     ...
    <globalMeasures ego="ToC_veh">
        <timeSpan values="0.00 0.10 0.20 0.30 0.40 0.50 0.60 0.70 0.80 0.90 1.00 ..."/>
        <BRSpan values="0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00 ..."/>
        <maxBR time="363.20" position="1850.01,-4.95" value="5.53"/>
        <SGAPSpan values="27.50 27.50 27.49 27.45 27.38 27.29 27.17 27.02 26.85 26.65 ..."/>
        <minSGAP time="365.50" position="1863.41,-4.95" value="0.31" leader="leader"/>
        <TGAPSpan values="inf 105.77 52.86 35.19 26.33 20.99 17.41 14.85 12.91 11.39 10.16 ..."/>
        <minTGAP time="365.30" position="1863.14,-4.95" value="0.23" leader="leader"/>
    </globalMeasures>
</SSMLog>
```

Elements of type `<conflict>` hold the following information in their child elements:

| Element  | Attribute  | Type  | Description  |
|---|---|---|---|
| timeSpan  | values  | list of floats  | All simulation time points within the duration of the encounter. All other entries of elements with list-type are given with respect to the corresponding time points.   |
| typeSpan  | values  | list of integers (Encounter type codes)  | Timeseries of classifications for the tracked encounter.  |
| egoPosition  | values  | list of 2D-coordinates  | Timeseries of the ego vehicle's positions.  |
| egoVelocity  | values  | list of 2D-vectors  | Timeseries of the ego vehicle's velocity vectors.  |
| foePosition  | values  | list of 2D-coordinates  | Timeseries of the foe vehicle positions.  |
| foeVelocity  | values  | list of 2D-vectors  | Timeseries of the foe vehicle velocity vectors.  |
| conflictPoint  | values  | list of 2D-coordinates  | Timeseries of the (eventually extrapolated) coordinates of the conflict point. The *conflict* point is taken as the respective *entry point to the conflict area*.   |
| TTCSpan  | values  | list of floats  | Timeseries of the calculated TTC values. May contain entries 'NA' corresponding to times, where TTC is not defined.  |
| DRACSpan  | values  | list of floats  | Timeseries of the calculated DRAC values. May contain entries 'NA' corresponding to times, where DRAC is not defined.  |
| minTTC  | time  | float  | Time point of the minimal measured value for the TTC.  |
|   | position  | 2D-coordinate  | Coordinate of the corresponding conflict point.  |
|   | type  | integer (Encounter type code)  | [Type code](#encounter_types) of the corresponding encounter type. (Defines the variant of [TTC-calculation](#ttc).)  |
|   | value  | float >= 0  | The minimal measured TTC-value. |
| maxDRAC  | time  | float  | Time point of the maximal measured value for the DRAC.  |
|   | position  | 2D-coordinate  | Coordinate of the corresponding conflict point.  |
|   | type  | integer (Encounter type code)  | [Type code](#encounter_types) of the corresponding encounter type. (Defines the variant of [DRAC-calculation](#drac).)  |
|   | value  | float >= 0  | The maximal measured DRAC-value.  |
| PET  | time  | float  | Time point of the minimal measured value for the PET. (Usually the PET is only measured once, therefore no PETSpan is reported.)  |
|   | position  | 2D-coordinate  | Coordinate of the corresponding encroachment point.  |
|   | type  | integer (Encounter type code)  | [Type code](#encounter_types) of the corresponding encounter type.  |
|   | value  | float >= 0  | The measured PET-value.  |


The `<globalMeasures>` element has the following structure:

| Element  | Attribute | Type           | Description                                                               |
|----------|-----------|----------------|---------------------------------------------------------------------------|
| timeSpan | values    | list of floats | Simulation time points at which the reported measures are logged.         |
| BRSpan   | values    | list of floats | Values of the brake rate at the time points given in timeSpan.            |
| SGAPSpan | values    | list of floats | Values of the spacing at the time points given in timeSpan.               |
| TGAPSpan | values    | list of floats | Values of the time headway at the time points given in timeSpan.          |
| maxBR    | time      | float          | Time at which the maximal value of the brake rate was recorded.           |
|          | value     | float          | Maximal recorded value for the brake rate.                                |
|          | position  | 2D-coordinate  | Position of the ego vehicle, where the maximal brake rate was recorded.   |
| minSGAP  | time      | float          | Time at which the minimal spacing was recorded.                           |
|          | value     | float          | Minimal recorded value for the spacing.                                   |
|          | position  | 2D-coordinate  | Position of the ego vehicle, where the minimal spacing was recorded.      |
| minTGAP  | time      | float          | Time at which the minimal time headway was recorded.                      |
|          | value     | float          | Minimal recorded value for the time headway.                              |
|          | position  | 2D-coordinate  | Position of the ego vehicle, where the minimal time headway was recorded. |
