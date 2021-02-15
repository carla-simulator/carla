---
title: TraCI/Change Route State
permalink: /TraCI/Change_Route_State/
---

# Command 0xc6: Change Route State

|  ubyte   |  string  |       ubyte       | <value_type\> |
| :------: | :------: | :---------------: | :----------: |
| Variable | Route ID | Type of the value |  New Value   |

Changes the state of a route. The following values can be changed, the
type of the value to given is also shown in the table.

**Overview Changeable Route Variables**

| Variable   | ValueType  | Description                                                                | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)    |
| ---------- | ---------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| add (0x80) | stringList | Adds a new route; the route gets the given id and follows the given edges. | [add](https://sumo.dlr.de/pydoc/traci._route.html#RouteDomain-add) |