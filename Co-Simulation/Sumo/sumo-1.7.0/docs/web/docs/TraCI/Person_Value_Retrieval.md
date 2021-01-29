---
title: TraCI/Person Value Retrieval
permalink: /TraCI/Person_Value_Retrieval/
---

## Command 0xae: Get Person Variable

|  ubyte   |  string   |
| :------: | :-------: |
| Variable | Person ID |

Asks for the value of a certain variable of the named person. The value
returned is the state of the asked variable/value within the last
simulation step. In the case the person is loaded, but outside the
network - due not being yet inserted into the network or being
teleported within the current time step - a default "error" value is
returned.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Person Variables**
</center>

| Variable                | ValueType               | Description      | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)   |
| ----------------------- | ----------------------- | ---------------- | ------------------------------------------------------------ |
| id list (0x00)          | stringList              | Returns a list of ids of all persons currently running within the scenario (the given person ID is ignored)                                                    | [getIDList](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getIDList)             |
| count (0x01)            | int                     | Returns the number of persons currently running within the scenario (the given person ID is ignored)                                                           | [getIDCount](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getIDCount)           |
| speed (0x40)            | double                  | Returns the speed of the named person within the last step \[m/s\]; error value: -2^30                                                                         | [getSpeed](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getSpeed)               |
| position (0x42)         | position                | Returns the position(two doubles) of the named person within the last step \[m,m\]; error value: \[-2^30, -2^30\].                                             | [getPosition](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getPosition)         |
| position 3D (0x39)      | position                | Returns the 3D-position(three doubles) of the named vehicle (center of the front bumper) within the last step \[m,m,m\]; error value: \[-2^30, -2^30, -2^30\]. | [getPosition3D](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getPosition3D)     |
| angle (0x43)            | double                  | Returns the angle of the named person within the last step \[°\]; error value: -2^30                                                                           | [getAngle](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getAngle)               |
| slope (0x36)            | double                  | Retrieves the slope at the current person position in degrees                                                                                                  | [getSlope](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getSlope)               |
| road id (0x50)          | string                  | Returns the id of the edge the named person was at within the last step; error value: ""                                                                       | [getRoadID](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getRoadID)             |
| type id (0x4f)          | string                  | Returns the id of the type of the named person                                                                                                                 | [getTypeID](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getTypeID)             |
| color (0x45)            | ubyte,ubyte,ubyte,ubyte | Returns the person's color (RGBA).                                                                                                                             | [getColor](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getColor)               |
| edge position (0x56)    | double                  | The position of the person along the edge (in \[m\]); error value: -2^30                                                                                       | [getLanePosition](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getLanePosition) |
| length (0x44)           | double                  | Returns the length of the persons \[m\]                                                                                                                        | [getLength](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getLength)             |
| minGap (0x4c)           | double                  | Returns the offset (gap to front person if halting) of this person \[m\]                                                                                       | [getMinGap](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getMinGap)             |
| width (0x4d)            | double                  | Returns the width of this person \[m\]                                                                                                                         | [getWidth](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getWidth)               |
| waiting time (0x7a)     | double                  | Returns the waiting time \[s\]                                                                                                                                 | [getWaitingTime](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getWaitingTime)   |
| next edge (0xc1)        | string                  | Returns the next edge on the persons route while it is walking. If there is no further edge or the person is in another stage, returns the empty string.       | [getNextEdge](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getNextEdge)         |
| remaining stages (0xc2) | int                     | Returns the number of remaining stages for the given person including the current stage.                                                                       | [getRemainingStages](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getStage)     |
| vehicle (0xc3)          | string                  | Returns the id of the vehicle if the person is in stage driving and has entered a vehicle.                                                                     | [getVehicle](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getVehicle)           |
| taxi reservations (0xc6)| list(Stage) (see below)     | Returns all reservations. If onlyNew is 1, each reservation is returned only once.                                                                     | [getTaxiReservations](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getTaxiReservations)           |

## Response 0xb4: Person Variable

|  ubyte   |  string   |            ubyte            |  <return_type\>   |
| :------: | :-------: | :-------------------------: | :--------------: |
| Variable | Person ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Person Variable"**.

## Response 0xc6: taxi reservations

Returns all reservations. If onlyNew is 1, each reservation is returned only once.

| string  | string   |  string  | string   | string   |  double   |  double    |  double |  double         |
| :-----: | :------: | :------: | :------: | :------: | :-------: | :--------: | :-----: | :-------------: |
|    id   | persons  | group    | fromEdge | toEdge   | departPos | arrivalPos | depart  | reservationTime | 

## Extended retrieval messages

Some further messages require additional parameters.

<center>
**Overview Extended Variables Retrieval**
</center>

| Variable   | Request ValueType   | Response ValueType | Description    | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md) |
| ---------- | ------------------- | ------------------ | -------------- | ---------------------------------------------------------- |
| stage (0xc0) | next stage index (int) | [TraCIStage](../TraCI/Change_Person_State.md#append_stage_280xc429)                | Returns the a compound object that describes nth next stage. Index 0 retrieves the value for the current stage. The given index must be lower than the value of 'remaining stages'.  | [getStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getStage) |
| edges (0x54) | next stage index (int) | stringList         | Returns the edges of the nth next stage. Index 0 retrieves the value for the current stage. The given index must be lower than the value of 'remaining stages'. For driving stages only origin and destination edge are returned.                              | [getEdges](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-getEdges) |
