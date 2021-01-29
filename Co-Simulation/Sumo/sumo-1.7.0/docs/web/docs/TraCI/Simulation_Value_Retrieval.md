---
title: TraCI/Simulation Value Retrieval
permalink: /TraCI/Simulation_Value_Retrieval/
---

## Command 0xab: Get Simulation Variable

|  ubyte   |            string             |
| :------: | :---------------------------: |
| Variable | Simulation ID (always unused) |

Asks for the value of a certain simulation variable

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Simulation Variables**
</center>

<table>
<thead>
<tr class="header">
<th><p>Variable</p></th>
<th><p>ValueType</p></th>
<th><p>Description</p></th>
<th><p><a href="../TraCI/Interfacing_TraCI_from_Python.html" title="wikilink">Python Method</a></p></th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><p>current simulation time (0x66)</p></td>
<td><p>double</p></td>
<td><p>Returns the current simulation time (in s)</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getTime">getTime</a></p></td>
</tr>
<tr class="even">
<td><p>current simulation time (0x70) (deprecated)</p></td>
<td><p>int</p></td>
<td><p>Returns the current simulation time (in ms)</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getCurrentTime">getCurrentTime</a></p></td>
</tr>
<tr class="odd">
<td><p>number of loaded vehicles (id 0x71)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which were loaded in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getLoadedNumber">getLoadedNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of loaded vehicles (id 0x72)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles which were loaded in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getLoadedIDList">getLoadedIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of departed vehicles (id 0x73)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which departed (were inserted into the road network) in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getDepartedNumber">getDepartedNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of departed vehicles (id 0x74)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles which departed (were inserted into the road network) in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getDepartedIDList">getDepartedIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that start to teleport<br />
(id 0x75)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which started to teleport in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStartingTeleportNumber">getStartingTeleportNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that start to teleport (id 0x76)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles which started to teleport in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStartingTeleportIDList">getStartingTeleportIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that end being teleported<br />
(id 0x77)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which ended to be teleported in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getEndingTeleportNumber">getEndingTeleportNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that end being teleported<br />
(id 0x78)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles which ended to be teleported in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getEndingTeleportIDList">getEndingTeleportIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of arrived vehicles (id 0x79)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which arrived (have reached their destination and are removed from the road network) in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getArrivedNumber">getArrivedNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of arrived vehicles (id 0x7a)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles which arrived (have reached their destination and are removed from the road network) in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getArrivedIDList">getArrivedIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>network bounding box (id 0x7c)</p></td>
<td><p>2D polygon</p></td>
<td><p>The lower left and the upper right corner of the bounding box of the simulation network.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getNetBoundary">getNetBoundary</a></p></td>
</tr>
<tr class="even">
<td><p>minimum number of vehicles that are still expected to leave the net (id 0x7d)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles which are in the net plus the ones still waiting to start. This number may be smaller than the actual number of vehicles still to come because of delayed route file parsing.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getMinExpectedNumber">getMinExpectedNumber</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that begin a stop (id 0x68)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles that halted on a scheduled stop in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStopStartingVehiclesNumber">getStopStartingVehiclesNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that begin a stop (id 0x69)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles that halted on a scheduled stop in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStopStartingVehiclesIDList">getStopStartingVehiclesIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that end a stop (id 0x6a)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles that begin to continue their journey, leaving a scheduled stop in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStopEndingVehiclesNumber">getStopEndingVehiclesNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that end a stop (id 0x6b)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles that begin to continue their journey, leaving a scheduled stop in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getStopEndingVehiclesIDList">getStopEndingVehiclesIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles involved in a collision (id 0x80)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles that were involved in a collision in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getCollidingVehiclesNumber">getCollidingVehiclesNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles involved in a collision (id 0x81)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles that were involved in a collision in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getCollidingVehiclesIDList">getCollidingVehiclesIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that begin to be parked<br />
(id 0x6c)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles that enter a parking position in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getParkingStartingVehiclesNumber">getParkingStartingVehiclesNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that begin to be parked<br />
(id 0x6d)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles that enter a parking position in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getParkingStartingVehiclesIDList">getParkingStartingVehiclesIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>number of vehicles that end to be parked<br />
(id 0x6e)</p></td>
<td><p>int</p></td>
<td><p>The number of vehicles that begin to continue their journey, leaving a scheduled parking in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getParkingEndingVehiclesNumber">getParkingEndingVehiclesNumber</a></p></td>
</tr>
<tr class="even">
<td><p>ids of vehicles that end being parked (id 0x6f)</p></td>
<td><p>stringList</p></td>
<td><p>A list of ids of vehicles that begin to continue their journey, leaving a scheduled parking in this time step.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getParkingEndingVehiclesIDList">getParkingEndingVehiclesIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>bus stop waiting (id 0x67)</p></td>
<td><p>int</p></td>
<td><p>Get the total number of waiting persons at the named bus stop.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getBusStopWaiting">getBusStopWaiting</a></p></td>
</tr>
<tr class="even">
<td><p>bus stop waiting ids (id 0xef)</p></td>
<td><p>stringList</p></td>
<td><p>Get the ids of waiting persons at the named bus stop.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getBusStopWaitingIDList">getBusStopWaitingIDList</a></p></td>
</tr>
<tr class="odd">
<td><p>delta T (id 0x7b)</p></td>
<td><p>double</p></td>
<td><p>Returns the length of one simulation step in seconds.</p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getDeltaT">getDeltaT</a></p></td>
</tr>
<tr class="even">
<td><p>parameter (0x7e)</p></td>
<td><p>string</p></td>
<td><p><a href="#Generic_Parameter_Retrieval_0x7e" title="wikilink">Returns the value for the given string parameter</a></p></td>
<td><p><a href="http://sumo.dlr.de/daily/pydoc/traci._simulation.html#SimulationDomain-getParameter">getParameter</a></p></td>
</tr>
</tbody>
</table>

## Response 0xbb: Simulation Variable

|  ubyte   |  string   |            ubyte            |  <return_type\>   |
| :------: | :-------: | :-------------------------: | :--------------: |
| Variable | <invalid\> | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Simulation Variable"**.

## Extended retrieval messages

Some further messages require additional parameters.

<center>
**Overview Extended Variables Retrieval**
</center>

<table>
<thead>
<tr class="header">
<th><p>Variable</p></th>
<th><p>Request ValueType</p></th>
<th><p>Response ValueType</p></th>
<th><p>Description</p></th>
<th><p><a href="../TraCI/Interfacing_TraCI_from_Python.html" title="wikilink">Python Method</a></p></th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><p>position conversion (0x82)</p></td>
<td><p>compound, see below</p></td>
<td><p>Position, as wished</p></td>
<td><p>Reads a position information and returns it converted into the given representation.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-convert2D">convert2D</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-convert3D">convert3D</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-convertGeo">convertGeo</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-convertRoad">convertRoad</a></p></td>
</tr>
<tr class="even">
<td><p>distance request (0x83)</p></td>
<td><p>compound, see below</p></td>
<td><p>double</p></td>
<td><p>Reads two positions and an indicator whether the air or the driving distance shall be computed. Returns the according distance.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-getDistanceRoad">getDistanceRoad</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-getDistance2D">getDistance2D</a></p></td>
</tr>
<tr class="odd">
<td><p>find route (0x86)</p></td>
<td><p>compound, see below</p></td>
<td><p>compound, see below</p></td>
<td><p>Reads origin and destination edge together with some vehicle paramters and computes the currently fastest driving route for the vehicle (for pedestrians / passengers use find intermodal route).</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-findRoute">findRoute</a></p></td>
</tr>
<tr class="even">
<td><p>find intermodal route (0x87)</p></td>
<td><p>compound, see below</p></td>
<td><p>compound, see below</p></td>
<td><p>Reads origin and destination position together with usable modes and other person paramters and computes the currently fastest route for the person using the available modes.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._simulation.html#SimulationDomain-findIntermodalRoute">findIntermodalRoute</a></p></td>
</tr>
</tbody>
</table>

### Command 0x82: Position Conversion

The request needs the following parameter:

| byte                | integer                | *Position*            | byte             | ubyte                          |
|---------------------|------------------------|---------------------|------------------|--------------------------------|
| value type compound | item number (always 2) | position to convert | value type *ubyte* | Type of the Position to return |

Use this command to convert between the different position types used by
TraCI. The specified *Position* is converted into the requested format
given through *PositionId*. When converting from coordinates to road
positions, the result is the closest position that was found to match
the given one. In all other cases, the conversion will give an exact
match. The return values consists of the converted value only.

See [TraCI/Protocol\#Position
Representations](../TraCI/Protocol.md#position_representations) for
available position formats.

### Command 0x83: Distance Request

The request needs the following parameter:

|         byte          |        integer         |   *Position*   |  *Position*  |     ubyte     |
| :-------------------: | :--------------------: | :------------: | :----------: | :-----------: |
| value type *compound* | item number (always 3) | start position | end position | Distance Type |

where
  
- Distance Type==0: air distance
- Distance Type==1: driving distance

The resulting distance is returned.

### Command 0x86: Find Route

|         byte          |        integer         |        byte         |   string   |        byte         |      string      |        byte         |    string    |        byte         |       double       |         byte         |         integer          |
| :-------------------: | :--------------------: | :-----------------: | :--------: | :-----------------: | :--------------: | :-----------------: | :----------: | :-----------------: | :----------------: | :------------------: | :----------------------: |
| value type *compound* | item number (always 5) | value type *string* | start edge | value type *string* | destination edge | value type *string* | vehicle type | value type *double* | departure time (s) | value type *integer* | routing mode (see below) |

!!! caution
    Routing mode shall be used to specify which data source to use for estimation of edge travel times. This is not yet implemented and the same data sources as for command [vehicle.rerouteTraveltime is used](../Simulation/Routing.md#travel-time_values_for_routing).

The following response is returned:

|      type + string      |  type + string   | type + stringList |  type + double  |  type + double  |
| :---------------------: | :--------------: | :---------------: | :-------------: | :-------------: |
| type (always "driving") | line (always "") |       edges       | travel time (s) | travel time (s) |

!!! note
    The python and C++ client return a namedtuple / struct with field names *stageType, line, destStop, edges, travelTime, cost*.

!!! note
    The seemingly unused/redundant fields are reserved for route-by-cost and intermodal routing (see below).

### Command 0x87: Find Intermodal Route

|         byte          |         integer         |        byte         |   string   |        byte         |      string      |        byte         |     string      |        byte         |       double       |         byte         |         integer          |        byte         |       double        |        byte         |   double    |        byte         |       double        |        byte         |        double        |        byte         |      double      |        byte         |        string        |        byte         |         string          |
| :-------------------: | :---------------------: | :-----------------: | :--------: | :-----------------: | :--------------: | :-----------------: | :-------------: | :-----------------: | :----------------: | :------------------: | :----------------------: | :-----------------: | :-----------------: | :-----------------: | :---------: | :-----------------: | :-----------------: | :-----------------: | :------------------: | :-----------------: | :--------------: | :-----------------: | :------------------: | :-----------------: | :---------------------: |
| value type *compound* | item number (always 12) | value type *string* | start edge | value type *string* | destination edge | value type *string* | available modes | value type *double* | departure time (s) | value type *integer* | routing mode (see above) | value type *double* | walking speed (m/s) | value type *double* | walk factor | value type *double* | depart position (m) | value type *double* | arrival position (m) | value type *double* | departPosLat (m) | value type *string* | vtype of the vehicle | value type *string* | vtype of the pedestrian |

The available modes are "car", "public", "bicycle" or space separated
combination. Walking is always considered possible and is the default
(or used when giving an empty string). When "car" is an available mode
the vehicle type can be given using the vtype parameter. If this is
empty, the default vehicle type is used.

The response is a list of stages similar to the one returned by
findRoute.

## Generic Parameter Retrieval 0x7e

The simulation supports retrieval of additional object parameters using
the [generic parameter retrieval
call](../TraCI/GenericParameters.md#get_parameter). The *object id*
refers to the object (i.e. chargingStation) id. The following parameters
are supported:

- chargingStation.totalEnergyCharged
- chargingStation.name
- chargingStation.lane
- parkingArea.capacity
- parkingArea.occupancy
- parkingArea.name
- parkingArea.lane
- busStop.name
- busStop.lane