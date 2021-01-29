---
title: TraCI/BusStop
permalink: /TraCI/BusStop/
---
Asks for the value of a certain variable of the named BusStop.
The value returned is the state of the asked variable/value within the
last simulation step. Please note that for asking values from your
BusStop [you have to define
them](../Simulation/Public_Transport.md#bus_stops)
within an {{AdditionalFile}} and load them at the start of the simulation. The `freq` and `file`
attributes do not affect TraCI.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview BusStop Variables**
</center>

| Variable                                     | ValueType           | Description       |  [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)    |
| -------------------------------------------- | ------------------- | ----------------- | -------------------------------------------------------------- |
| end pos                                  | double          | The end position of the stop along the lane measured in m.  | [getEndPos](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getEndPos) |    
| lane ID                                  | string          | Returns the lane of this calibrator (if it applies to a single lane)        | [getLaneID](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getLaneID) |    
| name                                  | string          | Returns the name of this stop        | [getName](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getName) |    
| person count                                  | integer          | Get the total number of waiting persons at the named bus stop.        | [getPersonCount](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getPersonCount) |    
| person ID                                  | stringList          | Get the IDs of waiting persons at the named bus stop.       | [getPersonIDs](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getPersonIDs) |    
| start pos                                  | double          | The starting position of the stop along the lane measured in m.        | [getStartPos](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getStartPos) |    
| vehicle count                                  | integer          | Get the total number of vehicles stopped at the named bus stop.        | [getVehicleCount](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getVehicleCount) |    
| vehicle ID                                  | stringList          | Get the IDs of vehicles stopped at the named bus stop.       |[getVehicleIDs](https://sumo.dlr.de/pydoc/traci._busstop.html#BusStopDomain-getVehicleIDs)  |    

