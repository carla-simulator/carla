---
title: TraCI/Route Value Retrieval
permalink: /TraCI/Route_Value_Retrieval/
---

## Command 0xa6: Get Route Variable

|  ubyte   |  string  |
| :------: | :------: |
| Variable | Route ID |

Asks for the value of a certain variable of the named route.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Route Variables**
</center>

| Variable       | ValueType  | Description               | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)              |
| -------------- | ---------- | ------------------------- | ----------------------------------------------------------------------- |
| id list (0x00) | stringList | Returns a list of ids of all currently loaded routes (the given route ID is ignored) | [getIDList](https://sumo.dlr.de/pydoc/traci._route.html#RouteDomain-getIDList)   |
| count (0x01)   | int        | Returns the number of currently loaded routes (the given route ID is ignored)        | [getIDCount](https://sumo.dlr.de/pydoc/traci._route.html#RouteDomain-getIDCount) |
| edges (0x54)   | stringList | Returns the ids of the edges this route covers                                       | [getEdges](https://sumo.dlr.de/pydoc/traci._route.html#RouteDomain-getEdges)     |

### Response 0xb6: Route Variable

|  ubyte   |      string       |            ubyte            |  <return_type\>   |
| :------: | :---------------: | :-------------------------: | :--------------: |
| Variable | Induction Loop ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Route Variable"**.