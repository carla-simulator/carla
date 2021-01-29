---
title: TraCI/Vehicle Value Retrieval
permalink: /TraCI/Vehicle_Value_Retrieval/
---

## Command 0xa4: Get Vehicle Variable

|  ubyte   |   string   |
| :------: | :--------: |
| Variable | Vehicle ID |

Asks for the value of a certain variable of the named vehicle. The value
returned is the state of the asked variable/value within the last
simulation step. In the case the vehicle is loaded, but outside the
network - due not being yet inserted into the network or being
teleported within the current time step - a default "error" value is
returned.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Vehicle Variables**
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
<td><p>id list (0x00)</p></td>
<td><p>stringList</p></td>
<td><p>Returns a list of ids of all vehicles currently running within the scenario (the given vehicle ID is ignored)</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getIDList">getIDList</a></p></td>
</tr>
<tr class="even">
<td><p>count (0x01)</p></td>
<td><p>int</p></td>
<td><p>Returns the number of vehicles currently running within the scenario (the given vehicle ID is ignored)</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getIDCount">getIDCount</a></p></td>
</tr>
<tr class="odd">
<td><p>speed (0x40)</p></td>
<td><p>double</p></td>
<td><p>Returns the speed of the named vehicle within the last step [m/s]; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSpeed">getSpeed</a></p></td>
</tr>
<tr class="even">
<td><p>lateral speed (0x32)</p></td>
<td><p>double</p></td>
<td><p>Returns the lateral speed of the named vehicle within the last step [m/s]; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLateralSpeed">getLateralSpeed</a></p></td>
</tr>
<tr class="odd">
<td><p>acceleration (0x72)</p></td>
<td><p>double</p></td>
<td><p>Returns the acceleration in the previous time step [m/s^2]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAcceleration">getAcceleration</a></p></td>
</tr>
<tr class="even">
<td><p>position (0x42)</p></td>
<td><p>position</p></td>
<td><p>Returns the position(two doubles) of the named vehicle (center of the front bumper) within the last step [m,m]; error value: [-2^30, -2^30].</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPosition">getPosition</a></p></td>
</tr>
<tr class="odd">
<td><p>position 3D (0x39)</p></td>
<td><p>position</p></td>
<td><p>Returns the 3D-position(three doubles) of the named vehicle (center of the front bumper) within the last step [m,m,m]; error value: [-2^30, -2^30, -2^30].</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPosition3D">getPosition3D</a></p></td>
</tr>
<tr class="even">
<td><p>angle (0x43)</p></td>
<td><p>double</p></td>
<td><p>Returns the angle of the named vehicle within the last step [°]; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAngle">getAngle</a></p></td>
</tr>
<tr class="odd">
<td><p>road id (0x50)</p></td>
<td><p>string</p></td>
<td><p>Returns the id of the edge the named vehicle was at within the last step; error value: ""</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getRoadID">getRoadID</a></p></td>
</tr>
<tr class="even">
<td><p>lane id (0x51)</p></td>
<td><p>string</p></td>
<td><p>Returns the id of the lane the named vehicle was at within the last step; error value: ""</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLaneID">getLaneID</a></p></td>
</tr>
<tr class="odd">
<td><p>lane index (0x52)</p></td>
<td><p>int</p></td>
<td><p>Returns the index of the lane the named vehicle was at within the last step; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLaneIndex">getLaneIndex</a></p></td>
</tr>
<tr class="even">
<td><p>type id (0x4f)</p></td>
<td><p>string</p></td>
<td><p>Returns the id of the type of the named vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getTypeID">getTypeID</a></p></td>
</tr>
<tr class="odd">
<td><p>route id (0x53)</p></td>
<td><p>string</p></td>
<td><p>Returns the id of the route of the named vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getRouteID">getRouteID</a></p></td>
</tr>
<tr class="even">
<td><p>route index (0x69)</p></td>
<td><p>int</p></td>
<td><p>Returns the index of the current edge within the vehicles route or -1 if the vehicle has not yet departed</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getRouteIndex">getRouteIndex</a></p></td>
</tr>
<tr class="odd">
<td><p>edges (0x54)</p></td>
<td><p>stringList</p></td>
<td><p>Returns the ids of the edges the vehicle's route is made of</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getRoute">getRoute</a></p></td>
</tr>
<tr class="even">
<td><p>color (0x45)</p></td>
<td><p>ubyte,ubyte,ubyte,ubyte</p></td>
<td><p>Returns the vehicle's color (RGBA). This does not return the currently visible color in the GUI but the color value set in the XML file or via TraCI.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getColor">getColor</a></p></td>
</tr>
<tr class="odd">
<td><p>lane position (0x56)</p></td>
<td><p>double</p></td>
<td><p>The position of the vehicle along the lane (the distance from the front bumper to the start of the lane in [m]); error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLanePosition">getLanePosition</a></p></td>
</tr>
<tr class="even">
<td><p>distance (0x84)</p></td>
<td><p>double</p></td>
<td><p>The distance, the vehicle has already driven [m]); error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getDistance">getDistance</a></p></td>
</tr>
<tr class="odd">
<td><p>signal states (0x5b)</p></td>
<td><p>int</p></td>
<td><p>An integer encoding the state of a vehicle's signals, see <a href="../TraCI/Vehicle_Signalling.html" title="wikilink">TraCI/Vehicle Signalling</a> for more information.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSignals">getSignals</a></p></td>
</tr>
<tr class="even">
<td><p>routing mode (0x89)</p></td>
<td><p>int</p></td>
<td><p>An integer encoding the <a href="../Simulation/Routing.html#travel-time_values_for_routing" title="wikilink">current routing mode</a> (0: default, 1: aggregated)</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getRoutingMode">getRoutingMode</a></p></td>
</tr>
<tr class="even">
<td><p>TaxiFleet (0x20)</p></td>
<td><p>stringList</p></td>
<td><p>Return the list of all taxis with the given mode:(-1: all, 0 : empty, 1 : pickup,2 : occupied, 3: pickup+occupied). Note: vehicles that are in state pickup+occupied (due to ride sharing) will also be returned when requesting mode 1 or 2</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getTaxiFleet">getTaxiFleet</a></p></td>
</tr>
<tr class="odd">
<td><p>CO2 emissions<br />
(id 0x60)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's CO2 emissions in mg/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getCO2Emission">getCO2Emission</a></p></td>
</tr>
<tr class="even">
<td><p>CO emissions<br />
(id 0x61)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's CO emissions in mg/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getCOEmission">getCOEmission</a></p></td>
</tr>
<tr class="odd">
<td><p>HC emissions<br />
(id 0x62)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's HC emissions in mg/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getHCEmission">getHCEmission</a></p></td>
</tr>
<tr class="even">
<td><p>PMx emissions<br />
(id 0x63)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's PMx emissions in mg/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPMxEmission">getPMxEmission</a></p></td>
</tr>
<tr class="odd">
<td><p>NOx emissions<br />
(id 0x64)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's NOx emissions in mg/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getNOxEmission">getNOxEmission</a></p></td>
</tr>
<tr class="even">
<td><p>fuel consumption<br />
(id 0x65)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's fuel consumption in ml/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getFuelConsumption">getFuelConsumption</a></p></td>
</tr>
<tr class="odd">
<td><p>noise emission<br />
(id 0x66)</p></td>
<td><p>double</p></td>
<td><p>Noise generated by the vehicle in dBA; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getNoiseEmission">getNoiseEmission</a></p></td>
</tr>
<tr class="even">
<td><p>electricity consumption (id 0x71)</p></td>
<td><p>double</p></td>
<td><p>Vehicle's electricity consumption in Wh/s during this time step, to get the value for one step multiply with the step length; error value: -2^30</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getElectricityConsumption">getElectricityConsumption</a></p></td>
</tr>
<tr class="odd">
<td><p>best lanes (id 0xb2)</p></td>
<td><p>complex</p></td>
<td><p>For each lane on the current edge, the sequences of lanes that would be followed from that lane without lane-change as well as information regarding lane-change desirability are returned (see below).</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getBestLanes">getBestLanes</a></p></td>
</tr>
<tr class="even">
<td><p>stop state (id 0xb5)</p></td>
<td><p>ubyte</p></td>
<td><p>value = 1 * stopped + 2 * parking + 4 * triggered + 8 * containerTriggered + 16 * atBusStop + 32 * atContainerStop + 64 * atChargingStation + 128 * atParkingArea</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getStopState">getStopState</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isAtBusStop">isAtBusStop</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isAtContainerStop">isAtContainerStop</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isStopped">isStopped</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isStoppedParking">isStoppedParking</a><br />
<a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isStoppedTriggered">isStoppedTriggered</a></p></td>
</tr>
<tr class="odd">
<td><p>length (0x44)</p></td>
<td><p>double</p></td>
<td><p>Returns the length of the vehicles [m]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLength">getLength</a></p></td>
</tr>
<tr class="even">
<td><p>vmax (0x41)</p></td>
<td><p>double</p></td>
<td><p>Returns the maximum speed of the vehicle [m/s]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getMaxSpeed">getMaxSpeed</a></p></td>
</tr>
<tr class="odd">
<td><p>accel (0x46)</p></td>
<td><p>double</p></td>
<td><p>Returns the maximum acceleration possibility of this vehicle [m/s^2]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAccel">getAccel</a></p></td>
</tr>
<tr class="even">
<td><p>decel (0x47)</p></td>
<td><p>double</p></td>
<td><p>Returns the maximum deceleration possibility of this vehicle [m/s^2]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getDecel">getDecel</a></p></td>
</tr>
<tr class="odd">
<td><p>tau (0x48)</p></td>
<td><p>double</p></td>
<td><p>Returns the driver's desired time headway for this vehicle [s]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getTau">getTau</a></p></td>
</tr>
<tr class="even">
<td><p>sigma (0x5d)</p></td>
<td><p>double</p></td>
<td><p>Returns the driver's imperfection (dawdling) [0,1]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getImperfection">getImperfection</a></p></td>
</tr>
<tr class="odd">
<td><p>speedFactor (0x5e)</p></td>
<td><p>double</p></td>
<td><p>Returns the road speed multiplier for this vehicle [double]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSpeedFactor">getSpeedFactor</a></p></td>
</tr>
<tr class="even">
<td><p>speedDev (0x5f)</p></td>
<td><p>double</p></td>
<td><p>Returns the deviation of speedFactor for this vehicle [double]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSpeedDeviation">getSpeedDeviation</a></p></td>
</tr>
<tr class="odd">
<td><p>vClass (0x49)</p></td>
<td><p>string</p></td>
<td><p>Returns the permission class of this vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getVehicleClass">getVehicleClass</a></p></td>
</tr>
<tr class="even">
<td><p>emission_class (0x4a)</p></td>
<td><p>string</p></td>
<td><p>Returns the emission class of this vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getEmissionClass">getEmissionClass</a></p></td>
</tr>
<tr class="odd">
<td><p>shape (0x4b)</p></td>
<td><p>string</p></td>
<td><p>Returns the shape class of this vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getShapeClass">getShapeClass</a></p></td>
</tr>
<tr class="even">
<td><p>minGap (0x4c)</p></td>
<td><p>double</p></td>
<td><p>Returns the offset (gap to front vehicle if halting) of this vehicle [m]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getMinGap">getMinGap</a></p></td>
</tr>
<tr class="odd">
<td><p>width (0x4d)</p></td>
<td><p>double</p></td>
<td><p>Returns the width of this vehicle [m]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getWidth">getWidth</a></p></td>
</tr>
<tr class="even">
<td><p>height (0xbc)</p></td>
<td><p>double</p></td>
<td><p>Returns the height of this vehicle [m]</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getHeight">getHeight</a></p></td>
</tr>
<tr class="odd">
<td><p>person capacity (0x38)</p></td>
<td><p>int</p></td>
<td><p>Returns the total number of persons that can ride in this vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPersonCapacity">getPersonCapacity</a></p></td>
</tr>
<tr class="odd">
<td><p>waiting time (0x7a)</p></td>
<td><p>double</p></td>
<td><p>Returns the consecutive time in where this vehicle was standing [s] (voluntary stopping is excluded) </p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getWaitingTime">getWaitingTime</a></p></td>
</tr>
<tr class="even">
<td><p>accumulated waiting time (0x87)</p></td>
<td><p>double</p></td>
<td><p>Returns the accumulated waiting time [s] within the previous time interval of default length 100 s. (length is configurable per option <b>--waiting-time-memory</b> given to the main application)</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAccumulatedWaitingTime">getAccumulatedWaitingTime</a></p></td>
</tr>
<tr class="odd">
<td><p>next TLS (0x70)</p></td>
<td><p>complex</p></td>
<td><p>Returns upcoming traffic lights, along with distance and state</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getNextTLS">getNextTLS</a></p></td>
</tr>
<tr class="even">
<td><p>next stops (0x73)</p></td>
<td><p>complex</p></td>
<td><p>Returns the list of upcoming stops, each as compound (laneID, endPos, ID, flags, duration, until). If flag 1 is set (stop reached), duration encodes the remaining duration. Negative values indicate being blocked from re-entering traffic after a parking stop.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getNextStops">getNextStops</a></p></td>
</tr>
<tr class="odd">
<td><p>person id list (0x1a)</p></td>
<td><p>stringList</p></td>
<td><p>Returns the list of persons which includes those defined using attribute 'personNumber' as well as <person>-objects which are riding in this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPersonIDList">getPersonIDList</a></p></td>
</tr>
<tr class="even">
<td><p>speed mode (0xb3)</p></td>
<td><p>int bitset (see below)</p></td>
<td><p>Retrieves how the values set by speed (0x40) and slowdown (0x14) shall be treated. See the <a href="../TraCI/Change_Vehicle_State.html#speed_mode_0xb3" title="wikilink">set speedmode command</a> for details.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSpeedMode">getSpeedMode</a></p></td>
</tr>
<tr class="odd">
<td><p>lane change mode (0xb6)</p></td>
<td><p>int bitset (see <a href="../TraCI/Change_Vehicle_State.html#lane_change_mode_0xb6" title="wikilink">below</a>)</p></td>
<td><p>Get information on how lane changing in general and lane changing requests by TraCI are performed.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLaneChangeMode">getLaneChangeMode</a></p></td>
</tr>
<tr class="even">
<td><p>slope (0x36)</p></td>
<td><p>double</p></td>
<td><p>Retrieves the slope at the current vehicle position in degrees</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSlope">getSlope</a></p></td>
</tr>
<tr class="odd">
<td><p>allowed speed (0xb7)</p></td>
<td><p>double</p></td>
<td><p>Returns the maximum allowed speed on the current lane regarding speed factor in m/s for this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAllowedSpeed">getAllowedSpeed</a></p></td>
</tr>
<tr class="even">
<td><p>line (0xbd)</p></td>
<td><p>string</p></td>
<td><p>Returns the line information of this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLine">getLine</a></p></td>
</tr>
<tr class="odd">
<td><p>Person Number(0x67)</p></td>
<td><p>int</p></td>
<td><p>Returns the total number of persons which includes those defined using attribute 'personNumber' as well as &lt;person&gt;-objects which are riding in this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getPersonNumber">getPersonNumber</a></p></td>
</tr>
<tr class="even">
<td><p>via edges(0xbe)</p></td>
<td><p>stringList</p></td>
<td><p>Returns the ids of via edges for this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getVia">getVia</a></p></td>
</tr>
<tr class="odd">
<td><p>speed without TraCI (0xb1)</p></td>
<td><p>double</p></td>
<td><p>Returns the speed that the vehicle would drive if not speed-influencing command such as setSpeed or slowDown was given.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSpeedWithoutTraCI">getSpeedWithoutTraCI</a></p></td>
</tr>
<tr class="even">
<td><p>valid route (0x92)</p></td>
<td><p>bool</p></td>
<td><p>Returns whether the current vehicle route is connected for the vehicle class of the given vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-isRouteValid">isRouteValid</a></p></td>
</tr>
<tr class="odd">
<td><p>lateral lane position (0xb8)</p></td>
<td><p>double</p></td>
<td><p>Returns the lateral position of the vehicle on its current lane measured in m.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLateralLanePosition">getLateralLanePosition</a></p></td>
</tr>
<tr class="even">
<td><p>max lateral speed (0xba)</p></td>
<td><p>double</p></td>
<td><p>Returns the maximum lateral speed in m/s of this vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getMaxSpeedLat">getMaxSpeedLat</a></p></td>
</tr>
<tr class="odd">
<td><p>lateral gap (0xbb)</p></td>
<td><p>double</p></td>
<td><p>Returns the desired lateral gap of this vehicle at 50km/h in m.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getMinGapLat">getMinGapLat</a></p></td>
</tr>
<tr class="even">
<td><p>lateral alignment (0xb9)</p></td>
<td><p>string</p></td>
<td><p>Returns the preferred lateral alignment of the vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLateralAlignment">getLateralAlignment</a></p></td>
</tr>
<tr class="odd">
<td><p>parameter (0x7e)</p></td>
<td><p>string</p></td>
<td><p><a href="#Device_and_LaneChangeModel_Parameter_Retrieval_0x7e" title="wikilink">Returns the value for the given string parameter</a></p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getParameter">getParameter</a></p></td>
</tr>
<tr class="even">
<td><p>action step length (0x7d)</p></td>
<td><p>double</p></td>
<td><p>Returns the current action step length for the vehicle in s.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getActionStepLength">getActionStepLength</a></p></td>
</tr>
<tr class="odd">
<td><p>last action time (0x7f)</p></td>
<td><p>double</p></td>
<td><p>Returns the time of the last action step in s.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLastActionTime">getLastActionTime</a></p></td>
</tr>
<tr class="even">
<td></td>
<td></td>
<td></td>
<td></td>
</tr>
</tbody>
</table>

## Response 0xb4: Vehicle Variable

|  ubyte   |   string   |            ubyte            |  <return_type\>   |
| :------: | :--------: | :-------------------------: | :--------------: |
| Variable | Vehicle ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Vehicle Variable"**.

**next stops (0x73)**

|         byte          |                 int                  |       ...        |
| :-------------------: | :----------------------------------: | :--------------: |
| value type *compound* | number of following stop information | stop information |

Each stop information is:

|        byte         | string  |        byte         | double |        byte         |     string      |         byte         |                                           integer                                           |        byte         |       double        |        byte         |      double      |
| :-----------------: | :-----: | :-----------------: | :----: | :-----------------: | :-------------: | :------------------: | :-----------------------------------------------------------------------------------------: | :-----------------: | :-----------------: | :-----------------: | :--------------: |
| value type *string* | lane-id | value type *double* | endPos | value type *string* | stoppingPlaceID | value type *integer* | stopFlags (bit set, see [setStop()](../TraCI/Change_Vehicle_State.md#stop_0x12)) | value type *double* | duration in seconds | value type *double* | until in seconds |

**best lanes (0xb2)**

|         byte          |                 int                  |       ...        |
| :-------------------: | :----------------------------------: | :--------------: |
| value type *compound* | number of following edge information | edge information |

Each edge information is:

|        byte         | string  |        byte         | double |        byte         |   double   |       byte        |    byte (signed)    |        byte        |                         byte (unsigned)                         |          byte           |          stringList           |
| :-----------------: | :-----: | :-----------------: | :----: | :-----------------: | :--------: | :---------------: | :-----------------: | :----------------: | :-------------------------------------------------------------: | :---------------------: | :---------------------------: |
| value type *string* | lane-id | value type *double* | length | value type *double* | occupation | value type *byte* | offset to best lane | value type *ubyte* | 0: lane may not be used for continuing drive, 1: it may be used | value type *stringlist* | list of best subsequent lanes |

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
<td><p>edge travel time information (0x58)</p></td>
<td><p>compound (time, edgeID), see below</p></td>
<td><p>double</p></td>
<td><p>Returns the edge travel time for the given time as stored in the vehicle's internal container. If such a value does not exist, -1 is returned.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getAdaptedTraveltime">getAdaptedTraveltime</a></p></td>
</tr>
<tr class="even">
<td><p>edge effort information (0x59)</p></td>
<td><p>compound (time, edgeID), see below</p></td>
<td><p>double</p></td>
<td><p>Returns the edge effort for the given time as stored in the vehicle's internal container. If such a value does not exist, -1 is returned.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getEffort">getEffort</a></p></td>
</tr>
<tr class="odd">
<td><p>leader (0x68)</p></td>
<td><p>double, see below</p></td>
<td><p>compound (string, double)</p></td>
<td><p>Returns the id of the leading vehicle and its distance, if the string is empty, no leader was found within the given range. Only vehicles ahead on the currently list of <em>best lanes</em> are considered (see above). This means, the leader is only valid until the next lane-change maneuver. The returned distance is measured from the ego vehicle front bumper + minGap to the back bumper of the leader vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLeader">getLeader</a></p></td>
</tr>
<tr class="even">
<td><p>distance request (0x83)</p></td>
<td><p>compound (see <a href="https://sumo.dlr.de/docs/TraCI/Simulation_Value_Retrieval.html#command_0x83_distance_request">Command 0x83: Distance Request</a>)</p></td>
<td><p>double, see below</p></td>
<td><p>Returns the distance between the current vehicle position and the specified position (for the given distance type)</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getDrivingDistance">getDrivingDistance</a> <a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getDrivingDistance2D">getDrivingDistance2D</a></p></td>
</tr>
<tr class="odd">
<td><p>change lane information (0x13)</p></td>
<td><p>compound , <a href="#change_lane_information_0x13" title="wikilink">see below</a></p></td>
<td><p>int, int <a href="#change_lane_information_0x13" title="wikilink">see below</a></p></td>
<td><p>Return whether the vehicle could change lanes in the specified direction (right: -1, left: 1. sublane-change within current lane: 0).<br />
Return the lane change state for the vehicle.</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getLaneChangeState">getLaneChangeState</a> <a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-couldChangeLane">couldChangeLane</a> <a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-wantsAndCouldChangeLane">wantsAndCouldChangeLane</a></p></td>
</tr>
<tr class="even">
<td><p>neighboring vehicles (0x13)</p></td>
<td><p>byte , <a href="#change_lane_information_0x13" title="wikilink">see below</a></p></td>
<td><p>stringList <a href="#change_lane_information_0x13" title="wikilink">see below</a></p></td>
<td><p>Returns a list of IDs for neighboring vehicle relevant to lane changing (&gt;1 elements only possible for <a href="../Simulation/SublaneModel.html" title="wikilink">sublane model</a>)</p></td>
<td><p><a href="https://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-getNeighbors">getNeighbors</a>, <a href="https://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-getLeftFollowers">getLeftFollowers</a>, <a href="https://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-getLeftLeaders">getLeftLeaders</a>, <a href="https://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-getRightFollowers">getRightFollowers</a>, <a href="https://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-getRightLeaders">getRightLeaders</a></p></td>
</tr>
<tr class="odd">
<td><p>followSpeed (0x1c)</p></td>
<td><p>compound, <a href="#followspeed_0x1c" title="wikilink">see below</a></p></td>
<td><p>double</p></td>
<td><p>Return the follow speed computed by the carFollowModel of vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getFollowSpeed">getFollowSpeed</a></p></td>
</tr>
<tr class="even">
<td><p>secureGap (0x1e)</p></td>
<td><p>compound, <a href="#secureGap_0x1c" title="wikilink">see below</a></p></td>
<td><p>double</p></td>
<td><p>Return the secure gap computed by the carFollowModel of vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getSecureGap">getSecureGap</a></p></td>
</tr>
<tr class="odd">
<td><p>stopSpeed (0x1e)</p></td>
<td><p>compound, <a href="#stopspeed_0x1c" title="wikilink">see below</a></p></td>
<td><p>double</p></td>
<td><p>Return the safe speed for stopping at gap computed by the carFollowModel of vehicle</p></td>
<td><p><a href="https://sumo.dlr.de/pydoc/traci._vehicle.html#VehicleDomain-getStopSpeed">getStopSpeed</a></p></td>
</tr>
</tbody>
</table>

The request message contents are as following:

### edge travel time information (0x58)

|         byte          |              int              |        byte         |        double         |        byte         | string  |
| :-------------------: | :---------------------------: | :-----------------: | :-------------------: | :-----------------: | :-----: |
| value type *compound* | number of elements (always=2) | value type *double* | requested time (in s) | value type *string* | edge id |

### edge effort information (0x59)

|         byte          |              int              |        byte         |        double         |        byte         | string  |
| :-------------------: | :---------------------------: | :-----------------: | :-------------------: | :-----------------: | :-----: |
| value type *compound* | number of elements (always=2) | value type *double* | requested time (in s) | value type *string* | edge id |

### leader (0x68)

|        byte         |               double               |
| :-----------------: | :--------------------------------: |
| value type *double* | minimum look ahead distance (in m) |

### next TLS (0x70)

|         byte          |        int         |         byte         |                 int                 |        byte         | string |       byte       |      int       |        byte         |     double      |       byte        |                                    byte                                     |
| :-------------------: | :----------------: | :------------------: | :---------------------------------: | :-----------------: | :----: | :--------------: | :------------: | :-----------------: | :-------------: | :---------------: | :-------------------------------------------------------------------------: |
| value type *compound* | number of elements | value type *integer* | number of traffic light links ahead | value type *string* | TLS id | value type *int* | TLS link index | value type *double* | distance to TLS | value type *byte* | [link state](../Simulation/Traffic_Lights.md#signal_state_definitions) |

The last 4 items occur multiple times according to the number of traffic
lights ahead (2nd return value)

### change lane information (0x13)

|         byte          |                                     int                                     |
| :-------------------: | :-------------------------------------------------------------------------: |
| value type *compound* | change direction (-1:right, 0:sublane-change within current lane or 1:left) |

The result consists of two integer values. The first value returns the
state as computed by the lane change model and the second value returns
the state after requests where incorporated (see
[changeLane](../TraCI/Change_Vehicle_State.md#change_lane_0x13),
[setLaneChangeMode](../TraCI/Change_Vehicle_State.md#lane_change_mode_0xb6)).
Each value is a bitset with the following meaning:

- 2^0: stay
- 2^1: left
- 2^2: right
- 2^3: strategic
- 2^4: cooperative
- 2^5: speedGain
- 2^6: keepRight
- 2^7: TraCI
- 2^8: urgent
- 2^9: blocked by left leader
- 2^10: blocked by left follower
- 2^11: blocked by right leader
- 2^12: blocked by right follower
- 2^13: overlapping
- 2^14: insufficient space
- 2^15: sublane

### neighboring vehicles (0xbf)

|        byte        |     ubyte     |
| :----------------: | :-----------: |
| value type *ubyte* | mode (bitset) |

The mode's bits specify which neighboring vehicles should be returned:

- 2^0: right neighbors (else: left)
- 2^1: neighbors ahead (else: behind)
- 2^2: only neighbors blocking a potential lane change (else: all)

The returned string list contains the IDs of all corresponding
neighboring vehicles.

### followSpeed (0x1c)

|         byte          | int   | double | double | double      | double         | string   |
| :-------------------: | :---: | :----: | :----: | :---------: | :------------: | :------: |
| value type *compound* | 5     | speed  | gap    | leaderSpeed | leaderMaxDecel | leaderID |

The leaderID is optional. Only a few car follow models such as CACC may require the identity of the leader to return the correct value
The returned value is the safe follow speed in m/s

### secureGap (0x1e)

|         byte          | int   | double | double      | double         | string   |
| :-------------------: | :---: | :----: | :---------: | :------------: | :------: |
| value type *compound* | 4     | speed  | leaderSpeed | leaderMaxDecel | leaderID |

The leaderID is optional. Only a few car follow models such as CACC may require the identity of the leader to return the correct value
The returned value is the safe gap for following the given leader in m.

### stopSpeed (0x1d)

|         byte          | int   | double | double |
| :-------------------: | :---: | :----: | :----: |
| value type *compound* | 2     | speed  | gap    |

The returned value is the safe speed in m/s for stopping after gap when braking
from the given speed.

## Device and LaneChangeModel Parameter Retrieval 0x7e

Vehicles supports retrieval of device parameters using the [generic
parameter retrieval
call](../TraCI/GenericParameters.md#get_parameter).

### Supported Device Parameters

- device.battery.energyConsumed
- device.battery.energyCharged
- device.battery.actualBatteryCapacity
- device.battery.maximumBatteryCapacity
- device.battery.chargingStationId
- device.battery.vehicleMass
- device.person.IDList
- device.container.IDList
- device.rerouting.period (returns individual rerouting period in
  seconds)
- device.rerouting.edge:EDGE_ID (returns assumed travel time for
  rerouting where EDGE_ID is the id of a network edge)
- device.driverstate.awareness
- device.driverstate.errorState
- device.driverstate.errorTimeScale
- device.driverstate.errorNoiseIntensity
- device.driverstate.minAwareness
- device.driverstate.initialAwareness
- device.driverstate.errorTimeScaleCoefficient
- device.driverstate.errorNoiseIntensityCoefficient
- device.driverstate.speedDifferenceErrorCoefficient
- device.driverstate.headwayErrorCoefficient
- device.driverstate.speedDifferenceChangePerceptionThreshold
- device.driverstate.headwayChangePerceptionThreshold
- device.driverstate.maximalReactionTime
- device.driverstate.originalReactionTime
- device.driverstate.actionStepLength
- device.example.customValue1 (return the value of option **--device.example.parameter**)
- device.example.customValue2 (return the value of vehicle parameter
  *example*)
- device.example.meaningOfLife (return 42)
- has.DEVICENAME.device (returns "true" or "false" depending on
  whether a device with DEVICENAME is equipped)

!!! caution
    If the vehicles does not carry the respective device an error is returned when retrieving parameters with the **device**. prefix. This happens when requesting *device.person.IDList* for a vehicle that has not carried any persons up to that point. Either check for existence or handle the error (i.e. by catching TraCIException in the python client).

### Supported LaneChangeModel Parameters

- laneChangeModel.<ATTRNAME\> (where <ATTRNAME\> is one of the
  [parameters supported by the
  laneChangeModel](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models)
  of the vehicle. i.e. *lcStrategic*)
