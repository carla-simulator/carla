---
title: TraCI/VehicleType Value Retrieval
permalink: /TraCI/VehicleType_Value_Retrieval/
---

## Command 0xa5: Get Vehicle Type Variable

|  ubyte   |     string      |
| :------: | :-------------: |
| Variable | Vehicle Type ID |

Asks for the value of a certain variable of the named vehicle type.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

| Variable                  | ValueType           | Description     | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md) |
| ------------------------- | ------------------- | --------------- | ---------------------------------------------------------- |
| id list (0x00)            | stringList          | Returns a list of ids of currently loaded vehicle types (the given vehicle type ID is ignored) | [getIDList](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getIDList)                     |
| count (0x01)              | int                 | Returns the number of currently loaded vehicle types (the given vehicle type ID is ignored)    | [getIDCount](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getIDCount)                   |
| length (0x44)             | double              | Returns the length of the vehicles of this type \[m\]                                          | [getLength](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getLength)                     |
| vmax (0x41)               | double              | Returns the maximum speed of vehicles of this type \[m/s\]                                     | [getMaxSpeed](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getMaxSpeed)                 |
| accel (0x46)              | double              | Returns the maximum acceleration possibility of vehicles of this type \[m/s^2\]                | [getAccel](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getAccel)                       |
| decel (0x47)              | double              | Returns the maximum deceleration possibility of vehicles of this type \[m/s^2\]                | [getDecel](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getDecel)                       |
| tau (0x48)                | double              | Returns the driver's desired time headway for vehicles of this type \[s\]                      | [getTau](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getTau)                           |
| sigma(0x5d)               | double              | Returns the driver's imperfection (dawdling) \[0,1\]                                           | [getImperfection](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getImperfection)         |
| speedFactor(0x5e)         | double              | Returns the road speed multiplier for drivers of this type \[double\]                          | [getSpeedFactor](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getSpeedFactor)           |
| speedDev(0x5f)            | double              | Returns the deviation of speedFactor for drivers of this type \[double\]                       | [getSpeedDeviation](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getSpeedDeviation)     |
| vclass (0x49)             | string              | Returns the class of vehicles of this type                                                     | [getVehicleClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getVehicleClass)         |
| emission_class (0x4a)    | string              | Returns the emission class of vehicles of this type                                            | [getEmissionClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getEmissionClass)       |
| shape (0x4b)              | string              | Returns the shape of vehicles of this type                                                     | [getShapeClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getShapeClass)             |
| minGap (0x4c)             | double              | Returns the offset (gap to front vehicle if halting) of vehicles of this type \[m\]            | [getMinGap](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getMinGap)                     |
| width (0x4d)              | double              | Returns the width of vehicles of this type \[m\]                                               | [getWidth](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getWidth)                       |
| height (0xbc)             | double              | Returns the height of vehicles of this type \[m\]                                              | [getHeight](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getHeight)                     |
| color (0x45)              | byte,byte,byte,byte | Returns the color of this type                                                                 | [getColor](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getColor)                       |
| max lateral speed (0xba)  | double              | Returns the maximum lateral speed in m/s of this type.                                         | [getMaxSpeedLat](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getMaxSpeedLat)           |
| lateral gap (0xbb)        | double              | Returns the desired lateral gap of this type at 50km/h in m.                                   | [getMinGapLat](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getMinGapLat)               |
| lateral alignment (0xb9)  | string              | Returns the preferred lateral alignment of the type.                                           | [getLateralAlignment](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getLateralAlignment) |
| action step length (0x7d) | double              | Returns the action step length for the vehicle type in s.                                      | [getActionStepLength](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getActionStepLength) |
| person capacity (0x38)    | int                 | Returns the total number of persons that can ride in a vehicle of this type.                   | [getPersonCapacity](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-getPersonCapacity)            |

**Overview Retrievable Vehicle Type Variables**

## Response 0xb5: Vehicle Type Variable

|  ubyte   |     string      |            ubyte            |  <return_type\>   |
| :------: | :-------------: | :-------------------------: | :--------------: |
| Variable | Vehicle Type ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Vehicle Type Variable"**.