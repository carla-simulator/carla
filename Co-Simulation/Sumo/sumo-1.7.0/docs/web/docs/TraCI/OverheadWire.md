---
title: TraCI/OverheadWire
permalink: /TraCI/OverheadWire/
---
Asks for the value of a certain variable of the named OverheadWire.
The value returned is the state of the asked variable/value within the
last simulation step. Please note that for asking values from your
OverheadWire within an {{AdditionalFile}} and load them at the start of the simulation. The `freq` and `file`
attributes do not affect TraCI.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview OverheadWire Variables**
</center>

| Variable                                     | ValueType           | Description       |  [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)    |
| -------------------------------------------- | ------------------- | ----------------- | -------------------------------------------------------------- |
| end pos                                  | double          | The end position of the stop along the lane measured in m.  | [getEndPos](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getEndPos) |    
| lane ID                                  | string          | Returns the lane of this calibrator (if it applies to a single lane)  | [getLaneID](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getLaneID) |    
| name                                | string          | Returns the name of this stop  | [getName](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getName) |    
| start pos                                  | double          | The starting position of the stop along the lane measured in m.  | [getStartPos](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getStartPos) |    
| vehicle count                                 | integer          | Get the total number of vehicles stopped at the named overhead wire.  | [getVehicleCount](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getVehicleCount) |    
| vehicle ID                                 | stringList          |  Get the IDs of vehicles stopped at the named overhead wire.  | [getVehicleIDs](https://sumo.dlr.de/pydoc/traci._overheadwire.html#OverheadWireDomain-getVehicleIDs) |    
