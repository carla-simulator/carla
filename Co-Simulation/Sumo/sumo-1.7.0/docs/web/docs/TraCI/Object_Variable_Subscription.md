---
title: TraCI/Object Variable Subscription
permalink: /TraCI/Object_Variable_Subscription/
---

# Introduction

Variable subscriptions allow you to ask once for a set of a structure's
variables and retrieve them periodically. In accordance to plain value
retrieval commands (0xa0-0xae) the subscription commands are split by
the "domain" - the type of the structure:

- command 0xd0: retrieve information from an induction loop (for a
  list of retrievable variables, see [Induction Loop Value
  Retrieval](../TraCI/Induction_Loop_Value_Retrieval.md))
- command 0xd1: retrieve information from a multi-entry/multi-exit
  detector (see [Multi-Entry-Exit Detectors Value
  Retrieval](../TraCI/Multi-Entry-Exit_Detectors_Value_Retrieval.md))
- command 0xd2: retrieve information from a traffic light (see
  [Traffic Lights Value
  Retrieval](../TraCI/Traffic_Lights_Value_Retrieval.md))
- command 0xd3: retrieve information from a lane (see [Lane Value
  Retrieval](../TraCI/Lane_Value_Retrieval.md))
- command 0xd4: retrieve information from a vehicle (see [Vehicle
  Value Retrieval](../TraCI/Vehicle_Value_Retrieval.md))
- command 0xd5: retrieve information from a vehicle type (see [Vehicle
  Type Value Retrieval](../TraCI/VehicleType_Value_Retrieval.md))
- command 0xd6: retrieve information from a route (see [Route Value
  Retrieval](../TraCI/Route_Value_Retrieval.md))
- command 0xd7: retrieve information from a point-of-interest (see
  [PoI Value Retrieval](../TraCI/POI_Value_Retrieval.md))
- command 0xd8: retrieve information from a polygon (see [Polygon
  Value Retrieval](../TraCI/Polygon_Value_Retrieval.md))
- command 0xd9: retrieve information from a junction (see [Junction
  Value Retrieval](../TraCI/Junction_Value_Retrieval.md))
- command 0xda: retrieve information from an edge (see [Edge Value
  Retrieval](../TraCI/Edge_Value_Retrieval.md))
- command 0xdb: retrieve information from the simulation (see
  [Simulation Value
  Retrieval](../TraCI/Simulation_Value_Retrieval.md))

The subscription for a structure's variables is initiated using a
"Subscribe ... Variable" command (0xd0-0xde). The command is evaluated
immediately on request, verifying it this way. It returns a "Subscribe
... Variable" response (0xe0-0xee). In the case of vehicles, the
subscription is descheduled as soon as the vehicle leaves the
simulation.

As soon as the subscription was accepted, it is executed after each call
of [Simulation
Step(2)](../TraCI/Control-related_commands.md#command_0x02_simulation_step)
command.

# Command 0xd*X*: Subscribe ... Variable

|    time    |   time   |  string   |      ubyte      |           ubyte\[n\]            |
| :--------: | :------: | :-------: | :-------------: | :-----------------------------: |
| begin Time | end Time | Object ID | Variable Number | The list of variables to return |

Some notes:

- begin Time: the subscription is executed only in time steps \>= this
  value; in ms
- end Time: the subscription is executed in time steps <= this value;
  the subscription is removed if the simulation has reached a higher
  time step; in ms
- The size of the variables list must be equal to the field "Variable
  Number".

# Response 0xe*X*: ... Subscription Response

|  string   |     ubyte      |      ubyte      |        ubyte        |              ubyte              |     <return_type\>      | ... |      ubyte      |        ubyte        |              ubyte              |     <return_type\>      |
| :-------: | :------------: | :-------------: | :-----------------: | :-----------------------------: | :--------------------: | :-: | :-------------: | :-----------------: | :-----------------------------: | :--------------------: |
| Object ID | Variable Count | Variable \#1 Id | Variable \#1 status | Return type of the variable \#1 | <VARIABLE_VALUE\#1\> | ... | Variable \#n Id | Variable \#n status | Return type of the variable \#n | <VARIABLE_VALUE\#n\> |

The respond to a **"Subscribe ... Variable"**.

The status is 0x00 (RTYPE_OK) if the variable could be retrieved
successfully. If not, the status is 0xff (RTYPE_ERR). In the second
case, the variable type is set to string and the variable value contains
the error message.

***Variable Count*** is the number of subscriptions which are returned.

# Client library methods

- In the [python
  library](../TraCI/Interfacing_TraCI_from_Python.md#subscriptions),
  all domains support the methods [*subscribe* and
  *unsubscribe*](https://sumo.dlr.de/pydoc/traci.domain.html#Domain)
- In the [C++ library](../TraCI/C++TraCIAPI.md), the method
  *simulation.subscribe* takes an additional argument that encodes the
  domain

# Special Cases

- The VAR_LEADER variable is special because the *getLeader (0x68)*
  command requires an additional distance argument. To subscribe to
  VAR_LEADER, the function *subscribeLeader* is provided by the
  [python client](../TraCI/Interfacing_TraCI_from_Python.md).
- The VAR_PARAMETER_WITH_KEY generic variable is special because the *getParameterWithKey (0x3e)*
  command requires an additional key argument. To subscribe to
  VAR_PARAMETER_WITH_KEY, the function *subscribeParameterWithKey* is provided by the
  [python client](../TraCI/Interfacing_TraCI_from_Python.md).