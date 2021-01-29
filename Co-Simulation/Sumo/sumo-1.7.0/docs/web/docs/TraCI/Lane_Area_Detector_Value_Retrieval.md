---
title: TraCI/Lane Area Detector Value Retrieval
permalink: /TraCI/Lane_Area_Detector_Value_Retrieval/
---

## Command 0xad: Get LaneAreaDetector Variable

|  ubyte   |          string          |
| :------: | :----------------------: |
| Variable | Get LaneArea Detector ID |

Asks for the value of a certain variable of the named [LaneArea (e2)
detector](../Simulation/Output/Lanearea_Detectors_(E2).md). The
value returned is the state of the asked variable/value within the last
simulation step. Please note that for asking values from your detectors
[you have to define
them](../Simulation/Output/Lanearea_Detectors_(E2).md) within an {{AdditionalFile}}
and load them at the start of the simulation. The `freq` and `file` attributes do
not matter for TraCI.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Lane Area Detector Variables**
</center>

| Variable                                          | ValueType  | Description      | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)         |
| ------------------------------------------------- | ---------- | ---------------- | ------------------------------------------------------------------ |
| id list (0x00)                                    | stringList | Returns a list of ids of all lane area detectors within the scenario (the given DetectorID is ignored)                   | [getIDList](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getIDList)                               |
| count (0x01)                                      | int        | Returns the number of lane area detectors within the scenario (the given DetectorID is ignored)                          | [getIDCount](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getIDCount)                             |
| position (0x42)                                   | double     | Returns the starting position of the detector at it's lane, counted from the lane's begin, in meters.                    | [getPosition](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getPosition)                           |
| length(0x44)                                      | double     | Returns the length of the detector in meters.                                                                            | [getLength](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLength)                               |
| lane ID (0x51)                                    | string     | Returns the ID of the lane the detector is placed at.                                                                    | [getLaneID](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLaneID)                               |
| last step vehicle number (0x10)                   | int        | Returns the number of vehicles that have been within the area detector within the last simulation step \[\#\];           | [getLastStepVehicleNumber](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLastStepVehicleNumber) |
| last step mean speed (0x11)                       | double     | Returns the mean speed of vehicles that have been within the named area detector within the last simulation step \[m/s\] | [getLastStepMeanSpeed](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLastStepMeanSpeed)         |
| last step vehicle ids (0x12)                      | stringList | Returns the list of ids of vehicles that have been within the detector in the last simulation step                       | [getLastStepVehicleIDs](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLastStepVehicleIDs)       |
| last step occupancy (0x13)                        | int        | Returns the percentage of space the detector was occupied by a vehicle \[%\]                                             | [getLastStepOccupancy](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getLastStepMeanSpeed)         |
| last step halting vehicles number (0x14)          | int        | Returns the number of vehicles which were halting during the last time step                                              | [getJamLengthVehicle](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getJamLengthVehicle)           |
| last step jam length in number of vehicles (0x18) | int        | Returns the number of vehicles which were halting on the loop during the last time step                                  | [getJamLengthVehicle](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getJamLengthVehicle)           |
| last step jam length in meters (0x19)             | int        | Returns the length of the jam in meters                                                                                  | [getJamLengthMeters](https://sumo.dlr.de/pydoc/traci._lanearea.html#LaneAreaDomain-getJamLengthMeters)             |

## Response 0xbd: LaneAreaDetector Variable

|  ubyte   |        string         |            ubyte            |  <return_type\>   |
| :------: | :-------------------: | :-------------------------: | :---------------: |
| Variable | Lane Area Detector ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Get LaneAreaDetector Variable"**.

## Notes

- You can find [some further description on multi-entry/multi-exit
  detectors](../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md)