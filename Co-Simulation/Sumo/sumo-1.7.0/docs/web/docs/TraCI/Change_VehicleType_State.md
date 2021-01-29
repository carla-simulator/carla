---
title: TraCI/Change VehicleType State
permalink: /TraCI/Change_VehicleType_State/
---

# Command 0xc5: Change Vehicle Type State

|  ubyte   |     string      |       ubyte       | <value_type\> |
| :------: | :-------------: | :---------------: | :----------: |
| Variable | Vehicle Type ID | Type of the value |  New Value   |

Changes a vehicle type. Because it is possible to change different
values of a vehicle type, the number of parameter to supply and their
types differ between commands. The following values can be changed, the
parameter which must be given are also shown in the table. If a vehicle
type is changed all vehicles sharing this type get the new value. If a
value mentioned below is changed via the vehicle (see
[TraCI/Change_Vehicle_State](../TraCI/Change_Vehicle_State.md)),
the vehicle gets a new type and won't be affected by further changes to
the original type.

<center>**Overview Changeable Vehicle Type Variables**</center>

| Variable                  | ValueType                                                      | Description                                                                                                                                                                          | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)                                                |
| ------------------------- | -------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------- |
| length (0x44)             | double                                                         | Sets the vehicle type's length to the given value                                                                                                                                    | [setLength](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setLength)                     |
| max speed (0x41)          | double                                                         | Sets the vehicle type's maximum speed to the given value                                                                                                                             | [setMaxSpeed](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setMaxSpeed)                 |
| vehicle class (0x49)      | string                                                         | Sets the vehicle type's vehicle class to the given value                                                                                                                             | [setVehicleClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setVehicleClass)         |
| speed factor (0x5e)       | double                                                         | Sets the vehicle type's speed factor to the given value                                                                                                                              | [setSpeedFactor](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setSpeedFactor)           |
| speed deviation (0x5f)    | double                                                         | Sets the vehicle type's speed deviation to the given value                                                                                                                           | [setSpeedDeviation](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setSpeedDeviation)     |
| emission class (0x4a)     | string                                                         | Sets the vehicle type's emission class to the given value                                                                                                                            | [setEmissionClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setEmissionClass)       |
| width (0x4d)              | double                                                         | Sets the vehicle type's width to the given value                                                                                                                                     | [setWidth](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setWidth)                       |
| height(0xbc)              | double                                                         | Sets the vehicle type's height to the given value                                                                                                                                    | [setHeight](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setHeight)                     |
| min gap (0x4c)            | double                                                         | Sets the vehicle type's minimum headway gap to the given value                                                                                                                       | [setMinGap](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setMinGap)                     |
| shape class (0x4b)        | string                                                         | Sets the vehicle type's shape class to the given value                                                                                                                               | [setShapeClass](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setShapeClass)             |
| acceleration (0x46)       | double                                                         | Sets the vehicle type's wished maximum acceleration to the given value                                                                                                               | [setAccel](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setAccel)                       |
| deceleration (0x47)       | double                                                         | Sets the vehicle type's wished maximum deceleration to the given value                                                                                                               | [setDecel](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setDecel)                       |
| imperfection (0x5d)       | double                                                         | Sets the vehicle type's driver imperfection (sigma) to the given value                                                                                                               | [setImperfection](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setImperfection)         |
| tau (0x48)                | double                                                         | Sets the vehicle type's wished headway time to the given value                                                                                                                       | [setTau](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setTau)                           |
| color (0x45)              | ubyte,ubyte,ubyte,ubyte (RGBA)                                 | Sets the vehicle type's color.                                                                                                                                                       | [setColor](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setColor)                       |
| max lateral speed (0xba)  | double                                                         | Sets the maximum lateral speed in m/s of this type.                                                                                                                                  | [setMaxSpeedLat](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setMaxSpeedLat)           |
| lateral gap (0xbb)        | double                                                         | Sets the minimal lateral gap of this type at 50km/h in m.                                                                                                                            | [setMinGapLat](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setMinGapLat)               |
| lateral alignment (0xb9)  | string                                                         | Sets the preferred lateral alignment of the type.                                                                                                                                    | [setLateralAlignment](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-setLateralAlignment) |
| copy (0x88)               | string                                                         | Creates a new vehicle type with the given ID as a duplicate of the original type.                                                                                                    | [copy](https://sumo.dlr.de/pydoc/traci._vehicletype.html#VehicleTypeDomain-copy)                               |
| action step length (0x7d) | double (new action step length), boolean (reset action offset) | Sets the current action step length for the vehicle type in s. If the boolean value resetActionOffset is true, an action step is scheduled immediately for all vehicles of the type. | [setActionStepLength](https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-setActionStepLength)         |



The message contents are as following:

### length (0x44)

|        byte         |   double   |
| :-----------------: | :--------: |
| value type *double* | New Length |

### max speed (0x41)

|        byte         |      double       |
| :-----------------: | :---------------: |
| value type *double* | New Maximum Speed |