---
title: TraCI/Edge Value Retrieval
permalink: /TraCI/Edge_Value_Retrieval/
---

## Command 0xaa: Get Edge Variable

|  ubyte   | string  |
| :------: | :-----: |
| Variable | Edge ID |

Asks for the value of a certain variable of the named edge.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>**Overview Retrievable Edge Variables**</center>

| Variable                             | ValueType  | Description                    | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)      |
| ------------------------------------ | ---------- | --------------------------------------------------------- | ------------------------------ |
| id list (0x00)                       | stringList | Returns a list of ids of all edges within the scenario (the given Edge ID is ignored)                                                                                   | [getIDList](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getIDList)                                 |
| count (0x01)                         | int        | Returns the number of edges within the scenario (the given Edge ID is ignored)                                                                                          | [getIDCount](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getIDCount)                               |
| lane number (0x52)                   | int        | Returns the number of lanes for the given edge ID                                                                                                                       | [getLaneNumber](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLaneNumber)                         |
| street name (0x1b)                   | string     | Returns the street name for the given edge ID                                                                                                                           | [getStreetName](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getStreetName)                         |
| current travel time (0x5a)           | double     | Returns the current travel time (length/mean speed).<br>**Note:** This definition of travel time has to be reverified.                                                                                                                    | [getTraveltime](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getTraveltime)                         |
| CO2 emissions (id 0x60)              | double     | Sum of CO2 emissions on this edge in mg during this time step.                                                                                                          | [getCO2Emission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getCO2Emission)                       |
| CO emissions (id 0x61)               | double     | Sum of CO emissions on this edge in mg during this time step.                                                                                                           | [getCOEmission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getCOEmission)                         |
| HC emissions (id 0x62)               | double     | Sum of HC emissions on this edge in mg during this time step.                                                                                                           | [getHCEmission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getHCEmission)                         |
| PMx emissions (id 0x63)              | double     | Sum of PMx emissions on this edge in mg during this time step.                                                                                                          | [getPMxEmission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getPMxEmission)                       |
| NOx emissions (id 0x64)              | double     | Sum of NOx emissions on this edge in mg during this time step.                                                                                                          | [getNOxEmission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getNOxEmission)                       |
| fuel consumption (id 0x65)           | double     | Sum of fuel consumption on this edge in ml during this time step.                                                                                                       | [getFuelConsumption](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getFuelConsumption)               |
| noise emission (id 0x66)             | double     | Sum of noise generated on this edge in dBA.                                                                                                                             | [getNoiseEmission](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getNoiseEmission)                   |
| electricity consumption (id 0x71)    | double     | Sum of electricity consumption on this edge in kWh during this time step.                                                                                               | [getElectricityConsumption](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getElectricityConsumption) |
| last step vehicle number (0x10)      | int        | The number of vehicles on this edge within the last time step.                                                                                                          | [getLastStepVehicleNumber](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepVehicleNumber)   |
| last step mean speed (0x11)          | double     | Returns the mean speed of vehicles that were on the named edge within the last simulation step \[m/s\]                                                                  | [getLastStepMeanSpeed](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepMeanSpeed)           |
| last step vehicle ids (0x12)         | stringList | Returns the list of ids of vehicles that were on the named edge in the last simulation step. The order is from rightmost to leftmost lane and downstream for each lane. | [getLastStepVehicleIDs](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepVehicleIDs)         |
| last step occupancy (0x13)           | double     | Returns the percentage of time the edge was occupied by a vehicle \[%\]                                                                                                 | [getLastStepOccupancy](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepOccupancy)           |
| last step mean vehicle length (0x15) | double     | The mean length of vehicles which were on the edge in the last step \[m\]                                                                                               | [getLastStepLength](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepLength)                 |
| waiting time (0x7a)                  | double     | Returns the sum of the waiting times for all vehicles on the edge \[s\]                                                                                                             | [getWaitingTime](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getWaitingTime)                       |
| last step person ids (0x1a)          | stringList | Returns the list of ids of persons that were on the named edge in the last simulation step                                                                              | [getLastStepPersonIDs](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepPersonIDs)           |
| last step halting number (0x14)      | int        | Returns the total number of halting vehicles for the last time step on the given edge. A speed of less than 0.1 m/s is considered a halt.                               | [getLastStepHaltingNumber](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getLastStepHaltingNumber)   |

### Response 0xba: Edge Variable

|  ubyte   | string  |            ubyte            |  <return_type\>   |
| :------: | :-----: | :-------------------------: | :--------------: |
| Variable | Edge ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Edge Variable"**.

### Extended retrieval messages

Some further messages require additional parameters.

<center>**Overview Extended Variables Retrieval**</center>

| Variable                            | Request ValueType | Response ValueType | Description       | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)             |
| ----------------------------------- | ----------------- | ------------------ | ---------------------- | --------------------------------- |
| edge travel time information (0x58) | time (double)     | double             | Returns the edge travel time for the given time as stored in the global container. If such a value does not exist, -1 is returned. | [getAdaptedTraveltime](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getAdaptedTraveltime) |
| edge effort information (0x59)      | time (double)     | double             | Returns the edge effort for the given time as stored in the global container. If such a value does not exist, -1 is returned.      | [getEffort](https://sumo.dlr.de/pydoc/traci._edge.html#EdgeDomain-getEffort)                       |