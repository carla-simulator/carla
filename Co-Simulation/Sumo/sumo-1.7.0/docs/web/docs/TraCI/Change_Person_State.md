---
title: TraCI/Change Person State
permalink: /TraCI/Change_Person_State/
---

## Command 0xce: Change Person State

|  ubyte   |  string   |
| :------: | :-------: |
| Variable | Person ID |

Changes the state of a Person...

<center>
**Overview Changeable Person Variables**
</center>

| Variable  | ValueType  | Description  | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)  |
|-----------|------------|--------------|-------------------------------------------------------------|
| add (0x80)  | complex see below  | Inserts a new person to the simulation at the given edge, position and time (in s). This function should be followed by appending Stages or the person will immediately vanish on departure.  | [add](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-add)  |
| append stage (0xc4)  | complex see below  | Appends a stage (stageObject, waiting, walking or driving) to the plan of the given person.  | [appendStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-appendStage)<br>[appendDrivingStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-appendDrivingStage)<br>[appendWaitingStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-appendWaitingStage)<br>[appendWalkingStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-appendWalkingStage)  |
| replace stage (0xcd)  | complex see below  | Replaces the nth next stage with the given stage object  | [replaceStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-replaceStage)  |
| remove stage (0xc5)  | int  | Removes the nth next stage. nextStageIndex must be lower then value of getRemainingStages(personID). nextStageIndex 0 immediately aborts the current stage and proceeds to the next stage. When removing all stages, stage 0 should be removed last (the python function removeStages does this automatically).  | [removeStage](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-removeStage)<br>[removeStages](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-removeStages)  |
| reroute (compute new route) by travel time (0x90)  | compound (<empty\>)  | Computes a new route to the current destination that minimizes travel time. The assumed values for each edge in the network can be customized in various ways. See [Simulation/Routing#Travel-time_values_for_routing](../Simulation/Routing.md#travel-time_values_for_routing). Replaces the current route by the found route.  | [rerouteTraveltime](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-rerouteTraveltime)  |
| move to XY (0xb4) | compound (edgeID, x, y, angle, keepRoute) (see below)  | Moves the person to a new position after normal movements have taken place. Also forces the angle of the person to the given value (navigational angle in degree). [See below for additional details](../TraCI/Change_Person_State.md#move_to_xy_0xb4) | [moveToXY](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-moveToXY) |
| color (0x45) | ubyte,ubyte,ubyte,ubyte (RGBA) | sets color for person with the given ID. i.e. (255,0,0,255) for the color red.| [setColor](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setColor)  |
| height (0xbc)  | double  | Sets the height in m for this person.  | [setHeight](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setHeight)  |
| length (0x44)  | double  | Sets the length in m for the given person.  | [setLength](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setLength)  |
| min gap (0x4c)  | double  | Sets the offset (gap to front person if halting) for this vehicle.  | [setMinGap](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setMinGap)  |
| speed (0x5e)  | double  | Sets the maximum speed in m/s for the named person for subsequent step.  | [setSpeed](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setSpeed)  |
| type (0x4f)  | string (id)  |  Sets the id of the type for the named person.  | [setType](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setType)  |
| width (0x4d)  | double  | Sets the width in m for this person.  | [setWidth](https://sumo.dlr.de/pydoc/traci._person.html#PersonDomain-setWidth)  |

# compound message details

The message contents are as following:

### add (0x80)


|         byte          |     integer     |        byte         | string  |        byte         |     string      |        byte         |         double         |        byte         |     double      |
| :------: | :--------: | :----------: | :-----: | :---------: | :-------: | :--------: | :-----------: | :-----------: | :-------: |
| value type *compound* | item number (4) | value type *string* | type ID | value type *string* | Initial Edge ID | value type *double* | depart time in seconds | value type *double* | depart position |

A depart time value of -3 is interpreted as immediate departure

### append stage (0xc4)

| byte                | int              | byte           | int        | byte              | string               | byte              | string              | byte              | string                  | byte                  | stringList | byte              | double     | byte              | double | byte              | double | byte              | string                  | byte              | double | byte              | double    | byte              | double     | byte              | string                     |
|---------------------|------------------|----------------|------------|-------------------|----------------------|-------------------|---------------------|-------------------|-------------------------|-----------------------|------------|-------------------|------------|-------------------|--------|-------------------|--------|-------------------|-------------------------|-------------------|--------|-------------------|-----------|-------------------|------------|-------------------|----------------------------|
| value type compound | item number (13) | value type int | stage type | value type string | vType (may be empty) | value type string | line (may be empty) | value type string | destStop (may be empty) | value type stringList | edges      | value type double | travelTime | value type double | cost   | value type double | length | value type string | intended (may be empty) | value type double | depart | value type double | departPos | value type double | arrivalPos | value type string | description (may be empty) |


### append waiting stage (0xc4)

|         byte          |       int       |       byte       |      int       |       byte       |      int       |        byte         |           string           |        byte         |        string         |
| :-------------------: | :-------------: | :--------------: | :------------: | :--------------: | :------------: | :-----------------: | :------------------------: | :-----------------: | :-------------------: |
| value type *compound* | item number (4) | value type *int* | stage type (1) | value type *double* | duration in s | value type *string* | description (may be empty) | value type *string* | stopID (may be empty) |

### append walking stage (0xc4)

|         byte          |       int       |       byte       |      int       |          byte           | stringList |        byte         |      double      |       byte       |                                                         int                                                          |        byte         |                                                    double                                                    |        byte         |        string         |
| :-------------------: | :-------------: | :--------------: | :------------: | :---------------------: | :--------: | :-----------------: | :--------------: | :--------------: | :------------------------------------------------------------------------------------------------------------------: | :-----------------: | :----------------------------------------------------------------------------------------------------------: | :-----------------: | :-------------------: |
| value type *compound* | item number (6) | value type *int* | stage type (2) | value type *stringlist* |   edges    | value type *double* | arrival position | value type *double* | duration in s (when a positive value is given, speed is computed from duration and length, otherwise speed is used) | value type *double* | speed (when a positive value is given this speed is used, otherwise the default speed of the person is used) | value type *string* | stopID (may be empty) |

### append driving stage (0xc4)

|         byte          |       int       |       byte       |      int       |        byte         |       string        |        byte         | string |        byte         |        string         |
| :-------------------: | :-------------: | :--------------: | :------------: | :-----------------: | :-----------------: | :-----------------: | :----: | :-----------------: | :-------------------: |
| value type *compound* | item number (4) | value type *int* | stage type (3) | value type *string* | destination edge ID | value type *string* | lines  | value type *string* | stopID (may be empty) |

### replace stage (0xcd)

| byte                | int             | byte           | int               | byte                | int              | byte           | int        | byte              | string               | byte              | string              | byte              | string                  | byte                  | stringList | byte              | double     | byte              | double | byte              | double | byte              | string                  | byte              | double | byte              | double    | byte              | double     | byte              | string                     |
|---------------------|-----------------|----------------|-------------------|---------------------|------------------|----------------|------------|-------------------|----------------------|-------------------|---------------------|-------------------|-------------------------|-----------------------|------------|-------------------|------------|-------------------|--------|-------------------|--------|-------------------|-------------------------|-------------------|--------|-------------------|-----------|-------------------|------------|-------------------|----------------------------|
| value type compound | item number (2) | value type int | replacement index | value type compound | item number (13) | value type int | stage type | value type string | vType (may be empty) | value type string | line (may be empty) | value type string | destStop (may be empty) | value type stringList | edges      | value type double | travelTime | value type double | cost   | value type double | length | value type string | intended (may be empty) | value type double | depart | value type double | departPos | value type double | arrivalPos | value type string | description (may be empty) |


### move to XY (0xb4)

The Person is moved to the network
position that best matches the given x,y network coordinates. 

The argument edgeID is optional and can be set to "" if not known.
It's use is to resolve ambiguities when there are multiple roads on top of each other (i.e. at bridges) or to provide additional guidance on intersections (where internal edges overlap). 

The optional keepRoute flag is a bitset that influences
mapping as follows:

- **bit0** (keepRoute = 1 when only this bit is set)
  - **1**: The person is mapped to the closest edge within it's existing route. 
           If no suitable position is found within 100m   mapping fails with an error.
  - **0**: The person is mapped to the closest edge within the network.
           If that edge does not belong to the original route, the current route is replaced by a new 
           route which consists of that edge only. 
           In the special case where the new edge is connected to the junction that the person was already walking towards
           (i.e. when crossing to the other side of the road),
           the current route edge is replaced with the new edge and the rest of the route remains unchanged. 
           If no suitable position is found within 100m mapping fails with an error.           
- **bit1** (keepRoute = 2 when only this bit is set)           
  - **1**: The person is mapped to the exact position in
  the network If that position lies outside the road network, the person stops moving on it's own
  accord until it is placed back into the network with another TraCI
  command. (if keeproute = 3, the position must still be within 100m of the vehicle route)
  - **0**: The person is always on a road
- **bit2** (keepRoute = 4 when only this bit is set)
  - **1**: lane permissions are ignored when mapping. This is needed for jaywalking.
  - **0**: The person is mapped only to lanes that allow pedestrians

The angle value is assumed to be in navigational degrees (between 0 and
360 with 0 at the top, going clockwise). The angle is used when scoring
candidate mapping lanes to resolve ambiguities. Upon mapping the person
is angle set to the given value for purposes of drawing and all outputs.
If the angle is set to the special TraCI constant
INVALID_DOUBLE_VALUE, the angle will not be used for scoring and the
person will assume the angle of the best found lane and guess the walking direction accordingly.
When mapping the person outside the road network, the angle will be computed from the
previous and the new position instead.

!!! note
    This function can also be used to force a person into the network that [has been loaded](../Simulation/VehicleInsertion.md#loading) but did not yet depart (i.e. immediately after calling person.add).
    
!!! caution
    In between calls of moveToXY, the person may still perform regular movement. The person position is reset afterwards but this can affect other pedestrians. To avoid this, person.setSpeed(personID, 0) can be used.

!!! caution
    When mapping a person to an edge that is not currently on it's route, the new route may consist of that edge only. Once the person reaches the arrivalPos on that edge it disappears unless receiving another moveToXY command in that simulation step. This means, persons may disappear when calling *traci.simulationStep* with arguments that cause SUMO to perform multiple steps.

|         byte          |       integer        |        byte         |                       string                       |       byte       |                        double                        | byte | double | byte | double | byte | integer ||
| :-------------------: | :------------------: | :------------------------------------------------: | :--------------: | :---------------------------------------------------: | :-: |:-: |:-: |:-: |:-: |:-: |:-: |:-: |
| value type *compound* | item number 5 | value type *string* | edge ID (to resolve ambiguities, may be arbitrary) | value type double | x Position (network coordinates) | value type double | y Position (network coordinates) | value type double | angle | value type byte | keepRoute (0 - 7) |

