---
title: TraCI
permalink: /TraCI/
---

## Introduction to TraCI

TraCI is the short term for "**Tra**ffic **C**ontrol **I**nterface".
Giving access to a running road traffic simulation, it allows to
retrieve values of simulated objects and to manipulate their behavior
"on-line".

## Using TraCI

### SUMO startup

TraCI uses a TCP based client/server architecture to provide access to
[sumo](sumo.md). Thereby, [sumo](sumo.md) acts as server
that is started with additional command-line options: **--remote-port** {{DT_INT}} where {{DT_INT}} is the
port [sumo](sumo.md) will listen on for incoming connections.

When started with the **--remote-port** {{DT_INT}} option, [sumo](sumo.md) only prepares the
simulation and waits for all external applications to connect and take
over the control. Please note, that the **--end** {{DT_TIME}} option is ignored when
[sumo](sumo.md) runs as a TraCI server, [sumo](sumo.md)
runs until the client demands a simulation end.

When using [sumo-gui](sumo-gui.md) as a server, the simulation
must either be started by using the [*play*
button](sumo-gui.md#usage_description) or by setting the option **--start**
before TraCI commands will be processed.

### Multiple clients

The number of clients which can connect can be given as an additional
option **--num-clients** {{DT_INT}}, where 1 is the default. Please note that in multi client
scenarios you must explicitly specify the execution order of the clients
using the
[*SetOrder*-command](TraCI/Control-related_commands.md#command_0x03_setorder).

Each client must specify a unique (but otherwise arbitrary) integer value
and the client commands will be handled in the order from the lowest to
the highest value within each simulation step.

The clients are automatically synchronized after every simulation step. This means, the simulation does not advance to the next step until all clients have called the 'simulationStep'' command. Also, the simulationStep command only returns control to the client after the simulation has advanced.

!!! caution
    The simulation will only start once all clients have connected. 

### Protocol specification

Please see the [TraCI Protocol Specification](TraCI/Protocol.md)
(including [Basic Flow](TraCI/Protocol.md#basic_flow),
[Messages](TraCI/Protocol.md#messages), [Data
Types](TraCI/Protocol.md#data_types)).

### TraCI Commands

- [Control-related
  commands](TraCI/Control-related_commands.md): perform a
  simulation step, close the connection, reload the simulation.

For the following APIs, the ID is equal to the ID defined in
[sumo](sumo.md)'s input files. Here, you find their [general
structure](TraCI/SUMO_ID_Commands_Structure.md).

- Value Retrieval
  - [Induction Loop Value
    Retrieval](TraCI/Induction_Loop_Value_Retrieval.md)
    retrieve information about induction loops
  - [Lane Area Detector Value
    Retrieval](TraCI/Lane_Area_Detector_Value_Retrieval.md)
    retrieve information about lane area detectors
  - [Multi-Entry-Exit Detectors Value
    Retrieval](TraCI/Multi-Entry-Exit_Detectors_Value_Retrieval.md)
    retrieve information about multi-entry/multi-exit detectors
  - [Calibrator Value
    Retrieval](TraCI/Calibrator.md)
    retrieve information about calibrators
  - [Traffic Lights Value
    Retrieval](TraCI/Traffic_Lights_Value_Retrieval.md)
    retrieve information about traffic lights
  - [Lane Value Retrieval](TraCI/Lane_Value_Retrieval.md)
    retrieve information about lanes
  - [Vehicle Value
    Retrieval](TraCI/Vehicle_Value_Retrieval.md) retrieve
    information about vehicles
  - [Person Value
    Retrieval](TraCI/Person_Value_Retrieval.md) retrieve
    information about persons
  - [Vehicle Type Value
    Retrieval](TraCI/VehicleType_Value_Retrieval.md)
    retrieve information about vehicle types
  - [Route Value Retrieval](TraCI/Route_Value_Retrieval.md)
    retrieve information about routes
  - [PoI Value Retrieval](TraCI/POI_Value_Retrieval.md)
    retrieve information about points-of-interest
  - [Polygon Value
    Retrieval](TraCI/Polygon_Value_Retrieval.md) retrieve
    information about polygons
  - [BusStop Value
    Retrieval](TraCI/BusStop.md)
    retrieve information about BusStops
  - [Charging Station Value
    Retrieval](TraCI/ChargingStation.md)
    retrieve information about charging stations
  - [Parking Area Value
    Retrieval](TraCI/ParkingArea.md)
    retrieve information about parking areas
  - [Overhead Wire Value
    Retrieval](TraCI/OverheadWire.md)
    retrieve information about overhead wires
  - [Junction Value
    Retrieval](TraCI/Junction_Value_Retrieval.md) retrieve
    information about junctions
  - [Edge Value Retrieval](TraCI/Edge_Value_Retrieval.md)
    retrieve information about edges
  - [Simulation Value
    Retrieval](TraCI/Simulation_Value_Retrieval.md) retrieve
    information about the simulation
  - [GUI Value Retrieval](TraCI/GUI_Value_Retrieval.md)
    retrieve information about the simulation visualization
  - [Rerouter](TraCI/Rerouter.md)
    retrieve information about the rerouter
  - [RouteProbe](TraCI/RouteProbe.md)
    retrieve information about the RouteProbe
- State Changing
  - [Change Lane State](TraCI/Change_Lane_State.md) change a
    lane's state
  - [Change Traffic Lights
    State](TraCI/Change_Traffic_Lights_State.md) change a
    traffic lights' state
  - [Change Vehicle State](TraCI/Change_Vehicle_State.md)
    change a vehicle's state
  - [Change Person State](TraCI/Change_Person_State.md)
    change a persons's state
  - [Change Vehicle Type
    State](TraCI/Change_VehicleType_State.md) change a
    vehicle type's state
  - [Change Route State](TraCI/Change_Route_State.md) change
    a route's state
  - [Change PoI State](TraCI/Change_PoI_State.md) change a
    point-of-interest's state (or add/remove one)
  - [Change Polygon State](TraCI/Change_Polygon_State.md)
    change a polygon's state (or add/remove one)
  - [Change Edge State](TraCI/Change_Edge_State.md) change
    an edge's state
  - [Change Simulation
    State](TraCI/Change_Simulation_State.md) change the
    simulation
  - [Change GUI State](TraCI/Change_GUI_State.md) change the
    simulation visualization
- Subscriptions
  - [TraCI/Object Variable
    Subscription](TraCI/Object_Variable_Subscription.md)
  - [TraCI/Object Context
    Subscription](TraCI/Object_Context_Subscription.md)
- Accessing [Generic Parameters](TraCI/GenericParameters.md)

### Shutdown

When using TraCI, the **--end** option of [sumo](sumo.md) is ignored.
Instead the simulation is closed by issuing the [*close*
command](TraCI/Control-related_commands.md#command_0x7f_close).
To detect whether all route files have been exhausted and all vehicles
have left the simulation, one can check whether the command
[getMinExpectedNumber](TraCI/Simulation_Value_Retrieval.md)
returns 0. The simulation will end as soon as all clients have sent the
*close* command.

It is also possible to reload the simulation with a new list of
arguments by using the
[*load*-command](TraCI/Control-related_commands.md#command_0x01_load).

## Using SUMO as a library

Normally, TraCI is used to couple multiple processes: A SUMO server
process and one or more TraCI client processes. Alternatively,
[Libsumo](Libsumo.md) can be used to embed SUMO as a library
into the client process. This allows using the same method signatures as
in the client libraries but avoids the overhead of socket communication.
Libsumo supports generating client libraries using
[SWIG](https://en.wikipedia.org/wiki/SWIG) and can therefore be used
with a large number of programming languages. C++, Java and Python
bindings are included when downloading a sumo-build.

## Example use

- There is a [tutorial on using TraCI for adaptive traffic
  lights](Tutorials/TraCI4Traffic_Lights.md) (using Python).
- The [Tutorials/CityMobil](Tutorials/CityMobil.md) tutorial
  uses TraCI for assigning new routes to vehicles (using Python).
- The
  [Tutorials/TraCIPedCrossing](Tutorials/TraCIPedCrossing.md)
  tutorial uses TraCI for building a crossing with a pedestrian
  triggered traffic light.

## Resources

### Interfaces by Programming Language

- Python: [the package
  tools/traci](TraCI/Interfacing_TraCI_from_Python.md) allows
  to interact with [sumo](sumo.md) using Python (This library
  is tested daily and supports all TraCI commands).
- C++: [The C++ TraCIAPI](TraCI/C++TraCIAPI.md) is client
  library that is part of the [sumo](sumo.md)-source tree (API
  coverage is almost complete).
- C++: [The Veins project](http://veins.car2x.org) provides a
  middle-ware for coupling [sumo](sumo.md) with
  [OMNET++](https://omnetpp.org/). As part of the infrastructure it
  provides a C++ client library for the TraCI API (API completeness is
  a bit behind the python client).
- .NET:
  [TraCI.NET](https://github.com/CodingConnected/CodingConnected.Traci)
  is a client library with almost complete API coverage.
- Matlab
  [TraCI4Matlab](http://www.mathworks.com/matlabcentral/fileexchange/44805-traci4matlab).
  The client is included as part of each SUMO release in
  {{SUMO}}*/tools/contributed/traci4matlab*
  Not all TraCI commands have been
  implemented.
- Java: [TraaS](TraCI/TraaS.md#java_client) provides a client
  library that is part of the [sumo](sumo.md)-source tree (API
  coverage is almost complete).
- Others: Any language that can access webservices using
  [SOAP](https://en.wikipedia.org/wiki/SOAP) can access SUMO using the
  [TraaS Webservice](TraCI/TraaS.md#webservice). A [Java
  webservice client](TraCI/TraaS.md#webservice_client) is also
  included with TraaS. API lags
  behind the python client.

### V2X simulation

TraCI allows to use [sumo](sumo.md) in combination with
communication network simulators for simulating [vehicular
communication](Topics/V2X.md). See
[Topics/V2X](Topics/V2X.md) for a list of available solutions.

### Other Resources

- [sumo](sumo.md)'s TraCI Server is a part of the plain
  distribution. The source code is located in the folder
  `src/traci-server`.

## References

- Axel Wegener, Michal Piorkowski, Maxim Raya, Horst Hellbrück, Stefan
  Fischer and Jean-Pierre Hubaux. TraCI: An Interface for Coupling Road
  Traffic and Network Simulators. Proceedings of the 11th
  Communications and Networking Simulation Symposium, April 2008.
  [Available at ACM Digital Library](https://dl.acm.org/citation.cfm?doid=1400713.1400740)
- Axel Wegener, Horst Hellbrück, Christian Wewetzer and Andreas Lübke:
  VANET Simulation Environment with Feedback Loop and its Application
  to Traffic Light Assistance. Proceedings of the 3rd IEEE Workshop
  on Automotive Networking and Applications, New Orleans, LA, USA, 2008.
  [Available at IEEEXplore](https://doi.org/10.1109/GLOCOMW.2008.ECP.67)

## Performance

Using TraCI slows down the simulation speed. The amount of slow-down
depends on many factors:

- number of TraCI function calls per simulation step
- types of TraCI functions being called (some being more expensive
  than others)
- computation within the TraCI script
- client language

### Examples

As an example use-case consider retrieving the x,y position of each
vehicle during every simulation step (using the python client):

```
   while traci.simulation.getMinExpectedNumber() > 0:
       for veh_id in traci.vehicle.getIDList():
            position = traci.vehicle.getPosition(veh_id)
       traci.simulationStep()
```

- This script is able to process about 25000 vehicles per second.
- Using [embedded
  python](TraCI/Interfacing_TraCI_from_Python.md#embedded_python)
  increases this to about 50000 vehicles per second
- The same value retrieval can also be sped up to 50000 vehicles per
  second by using
  [subscriptions](TraCI/Object_Variable_Subscription.md):

```
   while traci.simulation.getMinExpectedNumber() > 0: 
       for veh_id in traci.simulation.getDepartedIDList():
           traci.vehicle.subscribe(veh_id, [traci.constants.VAR_POSITION])
       positions = traci.vehicle.getAllSubscriptionResults()
       traci.simulationStep()
```

When using this script on the [Bologna
scenario](Data/Scenarios.md#bologna) (9000 vehicles, 5000
simulation steps) the following running times were recorded:

- without TraCI 8s
- plain position retrieval 90s
- retrieval using subscriptions 42s
- retrieval using embedded python 46s
- retrieval using subscriptions and embedded python 34s

The C++ client performance is higher:

- plain position retrieval 80s
- retrieval using subscriptions 28s

## Current and Future Development

Historically TraCI used a different (single byte) command ID for every
domain (induction loops, vehicle etc.) where the more significant half
of the byte denotes the command (get, set, subscribe, ...) and the
lesser significant the domain itself. To allow more than the 16 domains
resulting from this split, the most significant bit (which was unused
until now because there were only 7 commands) is now used for the domain
as well (and only three for the command). This allows for 28 domains 
because four general commands (like SIMSTEP) block some available combinations.
Currently there are only four possible domains left.

Furthermore after the invention of libsumo
some parts of the TraCI interface are so generic that it may be not so
hard to invent a wrapper with Apache Kafka or Google protocol buffers
which could in the long run replace the need for all the byte fiddling 
and the different hand crafted clients.

## Troubleshooting

### Output files are not closed.

This problem occurs if the client tries to access the output while
the simulation is still closing down. This can be solved by letting
the client wait for the simulation to shut down. The bug report was #524

### Obsolete APIs

There used to be two "generations" of [TraCI](TraCI.md)
commands. The first one mainly uses an internal mapping between the
string-typed IDs used in [sumo](sumo.md) and an external
representation of these which is int-based. The mapping was done
internally (within [TraCI](TraCI.md)).
The second "generation", the current one uses string-IDs equal to
those [sumo](sumo.md) reads. If you are bound to the first
generation API (for instance if you want to use TraNS) you can only
use [sumo](sumo.md) up to version 0.12.3. See
[FAQ](FAQ.md) about obtaining an old version.
