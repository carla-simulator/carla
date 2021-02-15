---
title: TraCI/Change Traffic Lights State
permalink: /TraCI/Change_Traffic_Lights_State/
---

# Command 0xc2: Change Traffic Light State

|  ubyte   |      string       |       ubyte       | <value_type\> |
| :------: | :---------------: | :---------------: | :----------: |
| Variable | Traffic Lights ID | Type of the value |  New Value   |

Changes the state of a traffic light. Because it is possible to change
different values of a traffic light, the given value may have different
types. The following values can be changed, the type of the value to
given is also shown in the table.

<center>**Overview Changeable Traffic Lights Variables**</center>

| Variable                            | ValueType            | Description   | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)     |
| ----------------------------------- | -------------------- | --------------------------------------------------------------------- | ----------------- |
| state (light/priority tuple) (0x20) | string               | Sets the phase definition to the given. Assumes the given string is a tuple of light definitions from rRgGyYoO, for red, green, yellow, off, where lower case letters mean that the stream has to decelerate. After this call the program-ID of the traffic light will be set to "online" and the state will be maintained until the next call of setRedYellowGreenState() or until setting another program with setProgram(). | [setRedYellowGreenState](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setRedYellowGreenState) [setLinkState](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setLinkState) |
| phase index (0x22)                  | integer              | Sets the phase of the traffic light to the given. The given index must be valid for the current program of the traffic light, this means it must be between 0 and the number of phases known to the current program of the tls - 1.                                                                                                                                                                                            | [setPhase](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setPhase)                                                                                                                                |
| program (0x23)                      | string               | Switches the traffic light to the given program. No WAUT algorithm is used, the program is directly instantiated. The index of the traffic light stays the same as before.                                                                                                                                                                                                                                                     | [setProgram](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setProgram)                                                                                                                            |
| phase duration (0x24)               | double               | Sets the remaining duration of the current phase in seconds.                                                                                                                                                                                                                                                                                                                                                                   | [setPhaseDuration](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setPhaseDuration)                                                                                                                |
| complete program definition (0x2c)  | compound (see below) | Inserts a completely new program.                                                                                                                                                                                                                                                                                                                                                                                              | [setCompleteRedYellowGreenDefinition](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-setCompleteRedYellowGreenDefinition)    |

The message contents are as following:

### state (light/priority tuple) (0x20)

|                 string                  |
| :-------------------------------------: |
| Phase Definition (light/priority tuple) |

### state (phase/brake/yellow) (0x21)

|    string    |    string    |    string     |
| :----------: | :----------: | :-----------: |
| Green bitset | Brake bitset | Yellow bitset |

### phase index (0x22)

|   integer   |
| :---------: |
| Phase Index |

### program (0x23)

|   string   |
| :--------: |
| Program ID |

### phase duration (0x24)

|        integer        |
| :-------------------: |
| Phase Duration \[ms\] |

### complete program definition (0x2c)

|         byte          |   integer   |        byte         |   string   |         byte         |     integer     |         byte          |           compound           |         byte         |   integer   |         byte         |   integer    | <phases\> |
| :-------------------: | :---------: | :-----------------: | :--------: | :------------------: | :-------------: | :-------------------: | :--------------------------: | :------------------: | :---------: | :------------------: | :----------: | :------: |
| value type *compound* | item number | value type *string* | program ID | value type *integer* | Type (always 0) | value type *compound* | Compound Length (always 0\!) | value type *integer* | Phase Index | value type *integer* | Phase Number |  Phases  |

with Phases:

|       byte        |       double        |        byte         | double |        byte         | double |        byte         |            string            |
| :---------------: | :-----------------: | :-----------------: | :----: | :-----------------: | :----: | :-----------------: | :--------------------------: |
| value type double | Duration in seconds | value type *double* | unused | value type *double* | unused | value type *string* | State (light/priority-tuple) |

!!! note
    For more information about what value should be put in each of the fields mentioned above, see [Tutorials/TraCI4Traffic_Lights#Further_Notes](../Tutorials/TraCI4Traffic_Lights.md#further_notes). Remember that you can check which integer value is associated with particular value type [here](../TraCI/Protocol.md#data_types).