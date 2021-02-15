---
title: TraCI/Change Edge State
permalink: /TraCI/Change_Edge_State/
---

# Command 0xca: Change Edge State

|  ubyte   | string  |       ubyte       | <value_type\> |
| :------: | :-----: | :---------------: | :----------: |
| Variable | Edge ID | Type of the value |  New Value   |

Changes the state of an edge. Because it is possible to change different
values of a edge, the number of parameter to supply and their types
differ between commands. The following values can be changed, the
parameter which must be given are also shown in the table.


<center>**Overview Changeable Edge Variables**</center>

| Variable                                     | ValueType              | Description  | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)   |
| -------------------------------------------- | ------------------------------------------------- | -------------------------- | ------------------------------------ |
| change global travel time information (0x58) | compound (begin time, end time, value), see below | Inserts the information about the travel time of the named edge valid from begin time to end time into the global edge weights times container. | [adaptTraveltime](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-adaptTraveltime) |
| change global effort information (0x59)      | compound (begin time, end time, value), see below | Inserts the information about the effort of the named edge valid from begin time to end time into the global edge weights container.            | [setEffort](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-setEffort)             |
| change max speed (0x41)                      |                                                   | Set a new maximum speed (in m/s) for all lanes of the edge.                                                                                     | [setMaxSpeed](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-setMaxSpeed)         |


The message contents are as following:

### change edge travel time information (0x58)

|         byte          |              int              |        byte         |      double       |        byte         |     double      |        byte         |          double          |
| :-------------------: | :----------------: | :-----------------: | :---------------: | :-----------------: | :-------------: | :-----------------: | :----------------------: |
| value type *compound* | number of elements (always=3) | value type *double* | begin time (in s) | value type *double* | end time (in s) | value type *double* | travel time value (in s) |

### change edge effort information (0x59)

|         byte          |              int              |        byte         |      double       |        byte         |     double   |        byte      |     double          |
| :-------------------: | :------------------: | :-----------------: | :---------------: | :-----------------: | :-------------: | :-----------------: | :-------------: |
| value type *compound* | number of elements (always=3) | value type *double* | begin time (in s) | value type *double* | end time (in s) | value type *double* | effort value (abstract) |