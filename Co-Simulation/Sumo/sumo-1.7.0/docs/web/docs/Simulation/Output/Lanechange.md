---
title: Simulation/Output/Lanechange
permalink: /Simulation/Output/Lanechange/
---

The lanechange-output tracks all events where a vehicle changes
laterally from one lane to another (define as the moment where the
center line of the vehicle enters the new lane). It also contains the
dominant reason for that change maneuver.

## Instantiating within the Simulation

The simulation is forced to generate this output using the option **--lanechange-output** {{DT_FILE}}. {{DT_FILE}} is
the name of the file the output will be written to. Any other file with
this name will be overwritten, the destination folder must exist.

## Generated Output

The generated XML file looks like this:

```
<lanechanges>
  <change id="<VEHICLE_ID>" time="<TIME_STAMP>" from="<SOURCE_LANE>" to="<DESTINATION_LANE>" reason="<CHANGE_REASON>" ... />
  ...

</lanechanges>
```

| Name                | Type    | Description                                    |
| ------------------- | ------- | ---------------------------------------------------------------------------------------- |
| id                  | id      | The id of the vehicle          |
| type                | id      | The type id of the vehicle      |
| time                | seconds | The time at which the change took place            |
| from                | id      | The id of the source lane                                      |
| to                  | id      | The id of the destination lane                               |
| pos                 | float m | The position where the lane-change took place (offset from lane start)    |
| reason              | string  | The reason for changing (see below)          |
| dir                 | int     | The direction of the change (difference in lane indices when staying within one edge)               |
| speed               | double  | The current speed of the vehicle                                                                    |
| leaderGap           | double  | The longitudinal gap to the nearest leader on the target lane (bumper to bumper) or 'None' if there was no leader.    |
| leaderSecureGap     | double  | The required longitudinal gap to the nearest leader to fulfill deceleration constraints or 'None' if there was no leader.      |
| followerGap         | double  | The longitudinal gap to the nearest follower on the target lane (bumper to bumper) or 'None' if there was no follower.      |
| followerSecureGap   | double  | The required longitudinal gap to the nearest follower to fulfill deceleration constraints or 'None' if there was no follower.     |
| origLeaderGap       | double  | The longitudinal gap to the nearest leader on the vehicle's original lane (bumper to bumper) or 'None' if there was no leader.    |
| origLeaderSecureGap | double  | The required longitudinal gap to the nearest leader to fulfill deceleration constraints or 'None' if there was no leader.      |
| latGap              | double  | The lateral gap to the nearest neighbor on the target lane or 'None' if there was no neighbor. (only when using the [sublane-model](../../Simulation/SublaneModel.md)) |

### Lane Changing Reasons

The reason may be one of

- **speedGain**
- **strategic**
- **cooperative**
- **keepRight**
- **sublane**
- **traci**

A reason may be further qualified by the addition of the string
**|urgent**.

### Continuous Lane Changes

If the simulation is performed with the
[sublane-model](../../Simulation/SublaneModel.md), the options **--lanechange-output.ended** and **--lanechange-output.started**
can be used to obtain additional output elements:

```
<lanechanges>
  <changeStarted id="<VEHICLE_ID>" time="<TIME_STAMP>" from="<SOURCE_LANE>" to="<DESTINATION_LANE>" reason="<CHANGE_REASON>" ... />
  <change id="<VEHICLE_ID>" time="<TIME_STAMP>" from="<SOURCE_LANE>" to="<DESTINATION_LANE>" reason="<CHANGE_REASON>" ... />
  <changeEnded id="<VEHICLE_ID>" time="<TIME_STAMP>" from="<SOURCE_LANE>" to="<DESTINATION_LANE>" reason="<CHANGE_REASON>" ... />
  ...
</lanechanges>
```

- A `changeStarted`-element is recorded when the vehicle starts to
  perform lateral movements for reasons other than *sublane*.
- A `change`-element is recorded when the primary lane of the vehicle
  has changed (the center of its front bumper crossed the border
  between original and target lane).
- A `changeEnded`-element is recorded when the lateral movement of the
  vehicle completes the planned maneuver distance and the lateral
  movement is for a reason other than *sublane*.

In case of activating the option **--lanechange.duration**, the change-elements of the output
report the gaps recorded at the initialization of the corresponding
lanechange.