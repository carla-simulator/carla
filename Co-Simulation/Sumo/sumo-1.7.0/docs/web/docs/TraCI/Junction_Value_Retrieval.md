---
title: TraCI/Junction Value Retrieval
permalink: /TraCI/Junction_Value_Retrieval/
---

## Command 0xa9: Get Junction Variable

|  ubyte   |   string    |
| :------: | :---------: |
| Variable | Junction ID |

Asks for the value of a certain variable of the named junction.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Junction Variables**
</center>

| Variable        | ValueType  | Description                                                        | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)        |
| --------------- | ---------- | --------------------------------------------------------------------------------------------- | ----------------------------------- |
| id list (0x00)  | stringList | Returns a list of ids of all junctions within the scenario (the given Junction ID is ignored) | [getIDList](https://sumo.dlr.de/pydoc/traci._junction.html#JunctionDomain-getIDList)     |
| count (0x01)    | int        | Returns the number of junctions within the scenario (the given Junction ID is ignored)        | [getIDCount](https://sumo.dlr.de/pydoc/traci._junction.html#JunctionDomain-getIDCount)   |
| position (0x42) | position   | Returns the position of the named junction \[m,m\]                                            | [getPosition](https://sumo.dlr.de/pydoc/traci._junction.html#JunctionDomain-getPosition) |
| shape (0x4e)    | 2D-polygon | Returns the shape (list of 2D-positions) of the named junction                                | [getShape](https://sumo.dlr.de/pydoc/traci._junction.html#JunctionDomain-getShape)       |

### Response 0xb9: Junction Variable

|  ubyte   |   string    |            ubyte            |  <return_type\>   |
| :------: | :---------: | :-------------------------: | :---------------: |
| Variable | Junction ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Junction Variable"**.