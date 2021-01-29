---
title: Simulation/Output/Traffic Lights
permalink: /Simulation/Output/Traffic_Lights/
---

SUMO offers some possibilities to save states of traffic lights during
the simulation, a feature mainly used to evaluate adaptive traffic light
algorithms. We will now describe these outputs.

## TLS States

### Instantiating within the Simulation

To enable writing tls state information you have to add the following
definition into one of your [*additional-files*](../../sumo.md#format_of_additional_files):

`<timedEvent type="SaveTLSStates" source="<TLS_ID>" dest="<OUTPUT_FILE>"/>`

The attributes have herein the following meanings:

| Attribute Name | Value Type        | Description                                               |
| -------------- | ----------------- | --------------------------------------------------------- |
| **type**       | enum (string)     | type of the event trigger; always "SaveTLSStates" herein. |
| **source**     | referenced tls id | The id of the traffic light which state shall be written. |
| **dest**       | file name         | The file to save the state into.                          |

### Generated Output

The output looks like this:

```
<tlsStates>
   <tlsState time="<SIM_STEP>" id="<TLS_ID>" programID="<TLS_SUBID>" phase="<PHASE_INDEX>" state="<STATE>"/>
   ... further states ...
</tlsStates>
```

The state is saved in each simulation step. The state itself is coded
as a list of the characters 'G', 'Y', and 'R', standing for "green",
"yellow", and "red", respectively, see [Simulation/Traffic
Lights](../../Simulation/Traffic_Lights.md). Each character describes
a link controlled by the traffic light. Only the state of the current
program is saved (see also "Adding new Programs"). The attributes have
the following meaning:

| Name      | Type                 | Description                                                         |
| --------- | -------------------- | ------------------------------------------------------------------- |
| time      | (simulation) seconds | The simulation time this entry was generated for                    |
| id        | id                   | The id of the tls that is responsible for the link.                 |
| programID | id                   | The sub-id of the tls that is (currently) responsible for the link. |
| phase     | uint                 | The index of the reported phase within the program                  |
| state     | id                   | The current state                                                   |

## TLS Switches

### Instantiating within the Simulation

This output contains information about the green light phases of links
(lane-to-lane connections). Each green light phase is describes by its
begin, end and duration. An entry is written into the file as soon a
green phase of a link ends. To enable writing tls switch information you
have to add the following definition into one of your [*additional-files*](../../sumo.md#format_of_additional_files):

`<timedEvent type="SaveTLSSwitchTimes" source="<TLS_ID>" dest="<OUTPUT_FILE>"/>`

The attributes have herein the following meanings:

| Attribute Name | Value Type        | Description                                                 |
| -------------- | ----------------- | ----------------------------------------------------------- |
| **type**       | enum (string)     | type of the event trigger; always "SaveTLSSwitches" herein. |
| **source**     | referenced tls id | The id of the traffic light which state shall be written.   |
| **dest**       | file name         | The file to save the state into.                            |

### Generated Output

The output looks like this:

```
<tlsSwitches>
   <tlsSwitch tls="<TLS_ID>" programID="<TLS_SUB_ID>" \
      fromLane="<LINKS_SOURCE_LANE>" toLane="<LINK_DESTINATION_LANE>" \
      begin="<BEGIN_OF_GREEN_PHASE>" end="<END_OF_GREEN_PHASE>" \
      duration="<DURATION_OF_GREEN_PHASE>"/>
   ... further switch points ...
</tlsSwitches>
```

Each entry is written into a single line. The values have the following
meanings:

| Name      | Type                 | Description                                                         |
| --------- | -------------------- | ------------------------------------------------------------------- |
| tls       | id                   | The id of the tls that is responsible for the link.                 |
| programID | id                   | The sub-id of the tls that is (currently) responsible for the link. |
| fromLane  | id                   | The id of the lane the link starts at.                              |
| toLane    | id                   | The id of the lane the link ends at.                                |
| begin     | (simulation) seconds | Begin of this link's last green phase.                              |
| end       | (simulation) seconds | End of this link's last green phase.                                |
| duration  | (simulation) seconds | Duration of this link's last green phase.                           |

## TLS Switch States

### Instantiating within the Simulation

This output saves tls-states as the TLS States - output does but not
every second but only at times the phases or the program (see also
"Adding new Programs") change. The output is instantiated by adding the
following definition into one of your [*additional-files*](../../sumo.md#format_of_additional_files):

`<timedEvent type="SaveTLSSwitchStates" source="<TLS_ID>" dest="<OUTPUT_FILE>"/>`

The attributes have herein the following meanings:

| Attribute Name | Value Type        | Description                                                     |
| -------------- | ----------------- | --------------------------------------------------------------- |
| **type**       | enum (string)     | type of the event trigger; always "SaveTLSSwitchStates" herein. |
| **source**     | referenced tls id | The id of the traffic light which state shall be written.       |
| **dest**       | file name         | The file to save the state into.                                |

### Generated Output

The output looks like this:

```
<tlsStates>
   <tlsState time="<SIM_STEP>" id="<TLS_ID>" programID="<TLS_SUBID>" phase="<PHASE_INDEX>" state="<STATE>"/>
   ... further states ...
</tlsStates>
```

Each entry is written into a single line. The values have the same
meaning as those in the
[SaveTLSStates-output](#generated_output).

## TLS Programs

### Instantiating within the Simulation

To enable writing tls state information you have to add the following
definition into one of your [*additional-files*](../../sumo.md#format_of_additional_files):

`<timedEvent type="SaveTLSProgram" source="<TLS_ID>" dest="<OUTPUT_FILE>"/>`

The attributes have herein the following meanings:

| Attribute Name | Value Type        | Description                                               |
| -------------- | ----------------- | --------------------------------------------------------- |
| **type**       | enum (string)     | type of the event trigger; always "SaveTLSStates" herein. |
| **source**     | referenced tls id | The id of the traffic light which state shall be written. |
| **dest**       | file name         | The file to save the state into.                          |

### Generated Output

The output is a loadable tlLogic element that records the complete state sequence of a traffic light:

```
<tlsStates>
   <tlLogic id="<TLS_ID>" programID="<TLS_SUBID>" type="static/>
      <phase state="..." duration="..."
      ...
   </tlLogic>
</tlsStates>
```

A new phase is saved whenever the state changes. A new tlLogic is written whenever the programID changes.

## Coupled Lane Area Detectors

### Instantiating within the Simulation

It is possible to add
[E2-detectors](../../Simulation/Output/Lanearea_Detectors_(E2).md)
which are coupled to a traffic light. Then, the tls is used to determine
the intervals (aggregation) time instead of giving a fixed aggregation
time. In this case, output will be generated every time the traffic
light switches. To use this feature set attribute `tl="<TL_ID>"` to the desired
traffic light id instead off attribute `<freq>`:

`<e2Detector id="<ID>" lane="<LANE_ID>" pos="<POSITION_ON_LANE>" length="<DETECTOR_LENGTH>" tl="<TL_ID>" file="<OUTPUT_FILE>" [timeThreshold="<FLOAT>"] [speedThreshold="<FLOAT>"] [jamThreshold="<FLOAT>"]/>`

A further feature allows you to collect measures only for the time the
light turns red for a certain link (connection between the incoming and
the outgoing lane). This allows measuring the maximum jam length in
front of a red traffic light for this link. To enable this, one has to
add the name of the following lane:
`to="<LANE_ID>"` to the list of attributes.
The incoming lane is already given by the
"`lane`"-attribute.