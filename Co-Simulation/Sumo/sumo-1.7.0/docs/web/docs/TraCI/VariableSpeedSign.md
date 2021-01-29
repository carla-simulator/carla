---
title: TraCI/VariableSpeedSign
permalink: /TraCI/VariableSpeedSign/
---

## Command 0x30: Get VariableSpeedSign Variable

|  ubyte   |  string  |
| :------: | :------: |
| Variable | Rerouter ID |

Asks for the value of a certain variable of the named VariableSpeedSign.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable VariableSpeedSign Variables**
</center>

| Variable       | ValueType  | Description               | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)              |
| -------------- | ---------- | ------------------------- | ----------------------------------------------------------------------- |
| id list        | stringList | Returns a list of ids of all current VariableSpeedSign | [getIDList](https://sumo.dlr.de/pydoc/traci._variablespeedsign.html#VariableSpeedSignDomain-getIDList)   |
| lanes          | stringList | Returns the ID of the lane the VariableSpeedSign is placed at.| [getIDCount](https://sumo.dlr.de/pydoc/traci._variablespeedsign.html#VariableSpeedSignDomain-getLanes) |
