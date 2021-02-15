---
title: TraCI/Lane Value Retrieval
permalink: /TraCI/Lane_Value_Retrieval/
---

# Command 0xa3: Get Lane Variable

|  ubyte   | string  |
| :------: | :-----: |
| Variable | Lane ID |

Asks for the value of a certain variable of the named lane.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Lane Variables**
</center>

| Variable                             | ValueType  | Description       | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)                      |
| ------------------------------------ | ---------- | ----------------------------------------------------------------------------------------- | ------------------- |
| id list (0x00)                       | stringList | Returns a list of ids of all lanes within the scenario (the given Lane ID is ignored)                                                     | [getIDList](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getIDList)                                 |
| count (0x01)                         | int        | Returns the number of lanes within the scenario (the given Lane ID is ignored)                                                            | [getIDCount](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getIDCount)                               |
| link number (0x30)                   | ubyte      | Returns the number of links outgoing from this lane \[\#\]                                                                                | [getLinkNumber](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLinkNumber)                         |
| edge id (0x31)                       | string     | Returns the id of the edge this lane belongs to                                                                                           | [getEdgeID](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getEdgeID)                                 |
| links (0x33)                         | compound   | Returns descriptions of the links outgoing from this lane \[m\]                                                                           | [getLinks](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLinks)                                   |
| allowed vehicle classes (0x34)       | stringList | Returns the mml-definitions of vehicle classes allowed on this lane                                                                       | [getAllowed](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getAllowed)                               |
| disallowed vehicle classes (0x35)    | stringList | Returns the mml-definitions of vehicle classes not allowed on this lane                                                                   | [getDisallowed](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getDisallowed)                         |
| length (0x44)                        | double     | Returns the length of the named lane \[m\]                                                                                                | [getLength](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLength)                                 |
| vmax (0x41)                          | double     | Returns the maximum speed allowed on this lane \[m/s\]                                                                                    | [getMaxSpeed](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getMaxSpeed)                             |
| shape (0x4e)                         | shape      | Returns this lane's shape                                                                                                                 | [getShape](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getShape)                                   |
| width (0x4d)                         | double     | Returns the width of the named lane \[m\]                                                                                                 | [getWidth](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getWidth)                                   |
| CO2 emissions (id 0x60)              | double     | Sum of CO2 emissions on this lane in mg during this time step.                                                                            | [getCO2Emission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getCO2Emission)                       |
| CO emissions (id 0x61)               | double     | Sum of CO emissions on this lane in mg during this time step.                                                                             | [getCOEmission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getCOEmission)                         |
| HC emissions (id 0x62)               | double     | Sum of HC emissions on this lane in mg during this time step.                                                                             | [getHCEmission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getHCEmission)                         |
| PMx emissions (id 0x63)              | double     | Sum of PMx emissions on this lane in mg during this time step.                                                                            | [getPMxEmission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getPMxEmission)                       |
| NOx emissions (id 0x64)              | double     | Sum of NOx emissions on this lane in mg during this time step.                                                                            | [getNOxEmission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getNOxEmission)                       |
| fuel consumption (id 0x65)           | double     | Sum of fuel consumption on this lane in ml during this time step.                                                                         | [getFuelConsumption](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getFuelConsumption)               |
| noise emission (id 0x66)             | double     | Sum of noise generated on this lane in dBA.                                                                                               | [getNoiseEmission](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getNoiseEmission)                   |
| electricity consumption (id 0x71)    | double     | Sum of electricity consumption on this lane in kWh during this time step.                                                                 | [getElectricityConsumption](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getElectricityConsumption) |
| last step vehicle number (0x10)      | int        | The number of vehicles on this lane within the last time step.                                                                            | [getLastStepVehicleNumber](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepVehicleNumber)   |
| last step mean speed (0x11)          | double     | Returns the mean speed of vehicles that were on this lane within the last simulation step \[m/s\]                                         | [getLastStepMeanSpeed](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepMeanSpeed)           |
| last step vehicle ids (0x12)         | stringList | Returns the list of ids of vehicles that were on this lane in the last simulation step                                                    | [getLastStepVehicleIDs](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepVehicleIDs)         |
| last step occupancy (0x13)           | double     | Returns the total lengths of vehicles on this lane during the last simulation step divided by the length of this lane                     | [getLastStepOccupancy](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepOccupancy)           |
| last step mean vehicle length (0x15) | double     | The mean length of vehicles which were on this lane in the last step \[m\]                                                                | [getLastStepLength](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepLength)                 |
| waiting time (0x7a)                  | double     | Returns the waiting time for all vehicles on the lane \[s\]                                                                               | [getWaitingTime](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getWaitingTime)                       |
| traveltime (0x5a)                    | double     | Returns the estimated travel time for the last time step on the given lane \[s\]                                                          | [getTraveltime](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getTraveltime)                         |
| last step halting number (0x14)      | int        | Returns the total number of halting vehicles for the last time step on the given lane. A speed of less than 0.1 m/s is considered a halt. | [getLastStepHaltingNumber](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getLastStepHaltingNumber)   |

## Response 0xb3: Lane Variable

|  ubyte   | string  |            ubyte            |  <return_type\>   |
| :------: | :-----: | :-------------------------: | :--------------: |
| Variable | Lane ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Lane Variable"**.

### command *links* 0x33 compound response value

If you request the list of links, an compound object is returned,
structured as following. Attention, each part is forwarded by a byte
which represents its data type, except "length".

| integer | type + integer  |  link  | ... |  link  |
| :-----: | :-------------: | :----: | :-: | :----: |
| Length  | Number of links | Link 1 | ... | Link n |

where **length** is the total number of following elements -- counting
each element of **link** separately -- and **link** is described by:

|  type + string  | type + string  |  type + ubyte   |   type + ubyte    |  type + ubyte   |   type + string     |      type + string      |     type + double  |
| :----------: | :-------: | :---------------: | :------------------------: | :------------: | :-----------------------: | :----------------------------: | :----------------: |
| consecutive not internal lane | consecutive internal lane | has priority (=1) or not (=0) | is opened (=1) or not (=0) | has approaching foe (=1) or not (=0) | [(current) state](../Networks/SUMO_Road_Networks.md#plain_connections) | [direction](../Networks/SUMO_Road_Networks.md#plain_connections) | length \[m\] - only valid if not using internal lanes |

!!! caution
    Please note that the information "is opened" and "has approaching foe" currently refer to the current time step; this is rather inappropriate and will be probably changed in the next future.

## Extended retrieval messages

Some further messages require additional parameters.

<center>
**Overview Extended Variables Retrieval**
</center>

| Variable    | Request ValueType | Response ValueType | Description      | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)          |
| ----------- | ----------------- | ------------------ | ------------------------------------------- | ------------------------------------------------------------------------ |
| foes (0x37) | toLane (string)   | stringVector       | Returns the list of foe lanes. There are two modes for calling this method. If **toLane** is a normal road lane that is reachable from the laneID argument, the list contains all lanes that are the origin of a connection with right-of-way over the connection between **laneID** and **toLane**. If **toLane** is empty and **laneID** is an internal lane, the list contains all internal lanes that intersect with *laneID*. | [getFoes](https://sumo.dlr.de/pydoc/traci._lane.html#LaneDomain-getFoes) |