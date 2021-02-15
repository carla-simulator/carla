---
title: TraCI/Rerouter
permalink: /TraCI/Rerouter/
---

## Command 0x28: Get Rerouter Variable

|  ubyte   |  string  |
| :------: | :------: |
| Variable | Rerouter ID |

Asks for the value of a certain variable of the named Rerouter.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Rerouter Variables**
</center>

| Variable       | ValueType  | Description               | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)              |
| -------------- | ---------- | ------------------------- | ----------------------------------------------------------------------- |
| id list (0x00) | stringList | Returns a list of ids of all current RouteProbes (the given Rerouter ID is ignored) | [getIDList](https://sumo.dlr.de/pydoc/traci._routeprobe.html#RouteProbeDomain-getIDList)   |
| count (0x01)   | int        | Returns the number of current RouteProbes (the given Rerouter ID is ignored)        | [getIDCount](https://sumo.dlr.de/pydoc/traci._routeprobe.html#RouteProbeDomain-getIDCount) |

### Response 0x38: Rerouter Variable

|  ubyte   |      string       |            ubyte            |  <return_type\>   |
| :------: | :---------------: | :-------------------------: | :--------------: |
| Variable | Rerouter ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Rerouter Variable"**.