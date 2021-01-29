---
title: TraCI/Traffic Lights Value Retrieval
permalink: /TraCI/Traffic_Lights_Value_Retrieval/
---

## Command 0xa2: Get Traffic Lights Variable

|  ubyte   |      string       |
| :------: | :---------------: |
| Variable | Traffic Lights ID |

Asks for the value of a certain variable of the named traffic light. The
value returned is the state of the asked variable/value within the last
simulation step. The following variable values can be retrieved, the
type of the return value is also shown in the table.

**Overview Retrievable Traffic Lights Variables**

| Variable               | ValueType       | Description     | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)   |
| ---------------------- | --------------- | --------------- | ------------------------------------------------------------ |
| id list (0x00)                                    | stringList      | Returns a list of ids of all traffic lights within the scenario (the given Traffic Lights ID is ignored)                                                                                                                                                                                                                                                                                                                                                  | [getIDList](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getIDList)                                                     |
| count (0x01)                                      | int             | Returns the number of traffic lights within the scenario (the given Traffic Lights ID is ignored)                                                                                                                                                                                                                                                                                                                                                         | [getIDCount](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getIDCount)                                                   |
| state (light/priority tuple) (0x20)               | string          | Returns the named tl's state as a tuple of light definitions from rRgGyYoO, for red, green, yellow, off, where lower case letters mean that the stream has to decelerate                                                                                                                                                                                                                                                                                  | [getRedYellowGreenState](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getRedYellowGreenState)                           |
| default current phase duration (0x24)             | double          | Returns the default total duration of the currently active phase in seconds; To obtain the remaining duration use *(getNextSwitch() - simulation.getTime())*; to obtain the spent duration subtract the remaining from the total duration                                                                                                                                                                                                                 | [getPhaseDuration](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getPhaseDuration)                                       |
| controlled lanes (0x26)                           | stringList      | Returns the list of lanes which are controlled by the named traffic light. Returns at least one entry for every element of the phase state (signal index)<sup>(1)(2)</sup>.                                                                                                                                                                                                                                                                               | [getControlledLanes](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getControlledLanes)                                   |
| controlled links (0x27)                           | compound object | Returns the links controlled by the traffic light, the index in the returned list corresponds to the tls link index of the connection. Each index maps to a list of link objects that share the same link index. Each link object is described by giving the incoming, outgoing, and via lane.                                                                                                                                                            | [getControlledLinks](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getControlledLinks)                                   |
| current phase (0x28)                              | int             | Returns the index of the current phase in the current program                                                                                                                                                                                                                                                                                                                                                                                             | [getPhase](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getPhase)                                                       |
| current program (0x29)                            | string          | Returns the id of the current program                                                                                                                                                                                                                                                                                                                                                                                                                     | [getProgram](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getProgram)                                                   |
| complete definition (light/priority tuple) (0x2b) | compound object | Returns the complete traffic light program, structure described under data types                                                                                                                                                                                                                                                                                                                                                                          | [getCompleteRedYellowGreenDefinition](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getCompleteRedYellowGreenDefinition) |
| assumed time of next switch (0x2d)                | double          | Returns the assumed time (in seconds) at which the tls changes the phase. Please note that the time to switch is not relative to current simulation step (the result returned by the query will be absolute time, counting from simulation start); to obtain relative time, one needs to subtract current simulation time from the result returned by this query. Please also note that the time may vary in the case of actuated/adaptive traffic lights | [getNextSwitch](https://sumo.dlr.de/pydoc/traci._trafficlight.html#TrafficLightDomain-getNextSwitch)                                             |


Please note:

1.  Each lane may be the start of multiple links (connections across the
    intersection). Thus a lane may occur multiple times in the output

## Response 0xb2: Traffic Lights Variable

|  ubyte   |      string      |            ubyte            | <return_type\>  |
| :------: | :--------------: | :-------------------------: | :------------: |
| Variable | Traffic Light ID | Return type of the variable | <RETURN_VALUE\> |

The respond to a **"Command Get Traffic Lights Variable"**.

### Structure of compound object traffic light program (both 0x25 and 0x2b)

If you request a complete traffic light program, the compound object is
structured as following. Attention, each part is preceded by a byte
which represents its data type, except "length".

| integer |  type + integer  |  logic  | ... |  logic  |
| :-----: | :--------------: | :-----: | :-: | :-----: |
| Length  | Number of logics | logic 1 | ... | logic n |

where **logic** is described by:

| type + string | type + integer | type + compound |   type + integer    |  type + integer  |  phase  | ... |  phase  |
| :-----------: | :------------: | :-------------: | :-----------------: | :--------------: | :-----: | :-: | :-----: |
|     SubID     |      Type      |  SubParameter   | Current phase index | Number of phases | Phase 1 | ... | Phase n |

Type and SubParameter aren't currently implemented therefore they are 0.

where **phase** is described by:

|     type + double     |      type + double       |      type + double       |  type + string   |
| :-------------------: | :----------------------: | :----------------------: | :--------------: |
| Duration (in seconds) | MinDuration (in seconds) | MaxDuration (in seconds) | Phase definition |

MinDuration and MaxDuration are only relevant in the context of actuated
traffic lights. The phase definition differs for 0x25 and 0x2b. The
first (0x25) contains the old, deprecated definition which uses three
strings that describe the tl's state, the first green y/n, the second
for brake y/n, the third for yellow y/n. The second (0x2b) contains a
single string representing the lights. Note, that the first (0x25) is
also reversed.

### Structure of compound object controlled links

If you request the list of links, the compound object is structured as
following. Attention, each part is preceded by a byte which represents
its data type, except "length".

|          integer           |       controlled links       | ... |        controlled links        |
| :------------------------: | :--------------------------: | :-: | :----------------------------: |
| Length (number of signals) | links controlled by signal 0 | ... | links controlled by signal n-1 |

**Controlled links:**

|            int             | stringlist | ... | stringlist |
| :------------------------: | :--------: | :-: | :--------: |
| number of controlled links |   link 0   | ... |  link n-1  |

Each link is described by a string list with three entries: the lane
which is incoming into the junction, the lane which is outgoing from the
junction, and the lane across the junction (junction-internal). Empty
strings indicate missing lanes.