---
title: TraCI/Induction Loop Value Retrieval
permalink: /TraCI/Induction_Loop_Value_Retrieval/
---

## Command 0xa0: Get Induction Loop Variable

|  ubyte   |      string       |
| :------: | :---------------: |
| Variable | Induction Loop ID |

Asks for the value of a certain variable of the named [induction loop
(e1
detector)](../Simulation/Output/Induction_Loops_Detectors_(E1).md).
The value returned is the state of the asked variable/value within the
last simulation step. Please note that for asking values from your
detectors [you have to define
them](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
within an {{AdditionalFile}} and load them at the start of the simulation. The `freq` and `file`
attributes do not matter for TraCI.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Induction Loop Variables**
</center>

| Variable                                     | ValueType           | Description       | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)     |
| -------------------------------------------- | ------------------- | ----------------- | -------------------------------------------------------------- |
| id list (0x00)                               | stringList          | Returns a list of ids of all induction loops within the scenario (the given Induction Loop ID is ignored)        | [getIDList](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getIDList)                               |
| position (0x42)                              | double              | Returns the position of the induction loop at it's lane, counted from the lane's begin, in meters.               | [getPosition](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getPosition)                           |
| lane ID (0x51)                               | string              | Returns the ID of the lane the induction loop is placed at.                                                      | [getLaneID](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLaneID)                               |
| count (0x01)                                 | int                 | Returns the number of induction loops within the scenario (the given Induction Loop ID is ignored)               | [getIDCount](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getIDCount)                             |
| last step vehicle number (0x10)              | int                 | Returns the number of vehicles that were on the named induction loop within the last simulation step \[\#\];<br><br>**Note:**This value corresponds to the *nVehEntered* measure of [induction loops](../Simulation/Output/Induction_Loops_Detectors_(E1).md).     | [getLastStepVehicleNumber](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLastStepVehicleNumber) |
| last step mean speed (0x11)                  | double              | Returns the mean speed of vehicles that were on the named induction loop within the last simulation step \[m/s\] | [getLastStepMeanSpeed](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLastStepMeanSpeed)         |
| last step vehicle ids (0x12)                 | stringList          | Returns the list of ids of vehicles that were on the named induction loop in the last simulation step            | [getLastStepVehicleIDs](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLastStepVehicleIDs)       |
| last step occupancy (0x13)                   | double              | Returns the percentage of time the detector was occupied by a vehicle \[%\]                                      | [getLastStepOccupancy](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLastStepOccupancy)         |
| last step mean vehicle length (0x15)         | double              | The mean length of vehicles which were on the detector in the last step \[m\]                                    | [getLastStepMeanLength](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getLastStepMeanLength)       |
| last step's time since last detection (0x16) | double              | The time since last detection \[s\]                                                                              | [getTimeSinceDetection](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getTimeSinceDetection)       |
| last step's vehicle data (0x17)              | complex (see below) | A complex structure containing several information about vehicles which passed the detector                      | [getVehicleData](https://sumo.dlr.de/pydoc/traci._inductionloop.html#InductionLoopDomain-getVehicleData)                     |


## Response 0xb0: Induction Loop Variable

|  ubyte   |      string       |            ubyte            |  <return_type\>   |
| :------: | :---------------: | :-------------------------: | :--------------: |
| Variable | Induction Loop ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Induction Loop Variable"**.

### Response to "last step's vehicle data" (0x17)

The return type is "complex", the following fields are:

| ubyte |              int              | <INFORMATION_PACKET\> | ... | <INFORMATION_PACKET\> |
| :---: | :---------------------------: | :------------------: | :-: | :------------------: |
| "int" | number of information packets | <INFORMATION_PACKET\> | ... | <INFORMATION_PACKET\> |

Where each information packet looks like:

|  ubyte   |   string   |  ubyte   |     double     |  ubyte   |      double      |  ubyte   |      double      |  ubyte   |     string      |
| :------: | :--------: | :------: | :------------: | :------: | :--------------: | :------: | :--------------: | :------: | :-------------: |
| "string" | Vehicle ID | "double" | Vehicle Length | "double" | Entry Time \[s\] | "double" | Leave Time \[s\] | "string" | Vehicle Type ID |

## Notes

- You can find [some further description on induction
  loops](../Simulation/Output/Induction_Loops_Detectors_(E1).md)