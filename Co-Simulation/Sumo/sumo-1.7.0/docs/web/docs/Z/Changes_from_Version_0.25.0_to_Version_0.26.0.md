---
title: Z/Changes from Version 0.25.0 to Version 0.26.0
permalink: /Z/Changes_from_Version_0.25.0_to_Version_0.26.0/
---

## Version 0.26.0 (19.04.2016)

### Bugfixes

- Simulation
  - Fixed crash when rerouting a large number of vehicles in
    parallel. #2169, #2180
  - Fixes related to [*zipper* nodes](../Networks/PlainXML.md#node_types).
    - Fixed deadlock #2075
    - Fixed collision #2165
    - Fixed undesired non-determinism.
  - Fixed collision of a vehicle with itself when departing at an
    edge that forms a tight circle. #2087
  - Fixed bug that was causing wrong vehicle counts in [summary
    output](../Simulation/Output/Summary.md) and was preventing
    the simulation from terminating automatically when [continuing
    from a loaded state](../Simulation/SaveAndLoad.md). #1494
  - Fixed route errors and crashing when [continuing from a loaded
    state](../Simulation/SaveAndLoad.md) and using [routing
    devices](../Demand/Automatic_Routing.md). #2102
  - Fixed bug that was causing false positives when calling
    *traci.inductionloop.getLastStepVehicleIds* and
    *traci.inductionloop.getLastStepVehicleNumber*. #2104
  - [Induction loop
    detectors](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
    now count vehicles which occupy the detector position during
    insertion.
  - Fixed collisions when using using continuous lane-changing.
  - Fixed bug that was causing erratic emission behavior for stopped
    vehicles when using the [PHEMlight emission
    model](../Models/Emissions/PHEMlight.md). #2109
  - Fixed unsafe traffic light plans when building networks without
    exclusive left-green phase. #2113
  - When using the [*striping*
    model](../Simulation/Pedestrians.md#model_striping),
    pedestrians now avoid moving with near-zero speed. #2143
  - Fixed pedestrian collisions. #2145
  - Intersections with more than 64 connections can now be loaded.
  - When approaching a
    [double-connection](../Networks/PlainXML.md#multiple_connections_from_the_same_edge_to_the_same_target_lane)
    vehicles now prefer the lane with the prioritized connection. #2160
  - Fixed collision at
    [double-connection](../Networks/PlainXML.md#multiple_connections_from_the_same_edge_to_the_same_target_lane). #2170
  - The default vehicle class is now `passenger` when using the default vehicle
    type *DEFAULT_VEHTYPE*. #2181
  - Fixed stuck vehicles when teleporting past an closed edge with a
    [rerouter](../Simulation/Rerouter.md). #2194
  - Fixed invalid route lengths in
    [tripinfo-output](../Simulation/Output/TripInfo.md). #2201
  - [Rerouters](../Simulation/Rerouter.md) rerouters that
    combine `closingReroute` and `destProbReroute` now only apply to vehicles that are affected by
    the closed edges. #2208
  - [Rerouting devices](../Demand/Automatic_Routing.md) can now
    be specified with [generic
    parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices). #2209
  - Fixed invalid waiting position of pedestrians after walking. #2220
  - Fixed detector data for teleporting vehicles. #1452
  - Fixed meandata for circular networks. #1032

- sumo-gui
  - Fixed bug that was causing slightly exaggerated exit times to be
    shown when activating *Show Link Items* from the vehicle context
    menu. #2022
  - Fixed flickering brake lights due to small random decelerations. #2177
  - [Areal
    detectors](../Simulation/Output/Lanearea_Detectors_(E2).md)
    can now be hidden by setting their size exaggeration to 0.
  - Fixed invalid occupancy value for [lane area
    detectors](../Simulation/Output/Lanearea_Detectors_(E2).md)
    (was exaggerated by a factor of 100). #2217
  - Fixed crashing related to showing and tracking parameters of
    arrived vehicles. #2226, #2227
  - Fixed glitch when drawing rail carriages on edges with
    customized length. #2192
  - Fixed coordinate update without mouse movement. #2076
  - Fixed time display switch in initial view. #2069
  - Vehicle shape and size are now correctly updated when set via
    TraCI. #2791

- MESO
  - Fixed bug that broke **--meso-multi-queue** behavior (regression in 0.25.0). #2150
  - vClass-specific speed limits are now used. #2205
  - tripinfo-output now contains valid *timeLoss* values. #2204
  - Fixed invalid travel time computation during simulation routing
    (was averaging segments instead of vehicles).

- MESO-GUI
  - Fixed crash. #2187
  - Coloring vehicles *by selection* is now working. #2149

- netedit
  - Fixed bug that made it impossible to modify numerical attributes
    (lane numbers, phase duration etc.) on some computers. #1966
  - Fixed error when modifying signal plans for joined traffic
    lights. #2185
  - Fixed invalid edge length attribute when inspecting networks
    without internal links. #2210
  - Fixed bug where junctions with uncommon shapes could not be
    selected.

- netconvert
  - Fixed bug that was causing unsafe [internal
    junctions](../Simulation/Intersections.md#waiting_within_the_intersection)
    to be built. #2086, #2097
  - Fixed bug that was causing z-information to become corrupted.
    Thanks to Mirco Sturari for the patch.
  - Fixed bug where pedestrians never got the green light when
    loading a *.net.xml* file and adding pedestrian crossings.
  - Fixed bug where pedestrian *walkingarea* edges were missing.
    (This could cause invalid routes to be generated). #2060
  - Multiple connections from the same edge to the same target lane
    can now be set in post-processing (i.e. after removal of
    geometry-like nodes). #2066
  - Option **--tls.guess-signals** now respects option **--tls.unset**. #2093
  - Fixed invalid traffic light plans for networks with pedestrian
    crossings. #2095
  - Loading [custom traffic light
    plans](../Networks/PlainXML.md#traffic_light_program_definition)
    now correctly affects the [building of internal
    junctions](../Simulation/Intersections.md#waiting_within_the_intersection). #2098
  - Several fixes in regard to OpenDrive networks:
    - Added missing `contactPoint` attribute when writing
      [OpenDrive](../Networks/Export.md#opendrive) networks.
    - Fixed geometry of lanes within intersections when writing
      [OpenDrive](../Networks/Export.md#opendrive) networks.
    - Fixed geometry of lanes when importing imprecise
      [OpenDrive](../Networks/Export.md#opendrive) networks #2085.
  - Option **--street-sign-output** now writes [xsd-conforming](../XMLValidation.md)
    output.
  - Fixed bugs that were causing invalid TLS plans to be generated
    for joined traffic lights. #2106, #2107
  - Fixed crash when importing OSM networks related to self-looping
    edges. #2135
  - Fixed bug that was causing invalid junction shapes and extremely
    large network boundaries. #2141
  - Fixed crashing (on Windows) and invalid traffic lights (Linux)
    when loading a *.net.xml* file and adding splits. #2147
  - Fixed invalid connections at edges with vClass-exclusive lanes. #2157, #2158
  - Fixed invalid traffic light plans for node type `traffic_light_right_on_red`. #2162
  - Fixed unsafe junction logic when using custom tls plans with
    node type `traffic_light_right_on_red`. #2163
  - Connections from lanes that are added during ramp guessing can
    now be specified. #2175
  - User-defined connections are no longer discarded at guessed
    ramps. #2196
  - Fixed error when guessing overlapping off-ramps. #2213
  - Fixed error when computing edge shapes with unusual input
    geometries. #2218

- TraCI
  - Multiple fixes to the [C++ TraCI
    library](../TraCI/C++TraCIAPI.md)
    - commands *gui.setScheme, gui.getScheme*,
      *inductionloop.getVehicleData*
    - various *set*-commands were not consuming all result bytes
      and thus corrupted the message stream. Thanks to Alexander
      Weidinger for the patch. #2007
  - Fixes to TraaS functions *Simulation_getDistance2D* and
    *Simulation_getDistanceRoad*. #2127
  - Fixed crash when using vehicle command *move to VTD*. #2129
  - vehicle command *move to VTD* can now position vehicles on
    internal lanes. #2130
  - Commands that return the road distance no longer return an
    exaggerated value in networks without internal links (distances
    across intersections were counted twice).
  - vehicle command *distance* now takes the depart position into
    account (was using 0 before). #2203

- Tools
  - Fixed error that prevented
    [Tools/Visualization\#plot_net_dump.py](../Tools/Visualization.md#plot_net_dumppy)
    from running.
  - Fixed import errors in
    [vehrouteDiff.py](../Tools/Output.md#vehroutediffpy)
    (formerly undocumented tool).
  - [netdiff.py](../Tools/Net.md#netdiffpy) now correctly
    handles non-ascii characters (i.e. international street names). #2112
  - [netdiff.py](../Tools/Net.md#netdiffpy) now handles
    pedestrian crossings. #1893

### Enhancements

- Simulation
  - [MESO](../Simulation/Meso.md) and
    [MESO\#MESO-GUI](../Simulation/Meso.md#meso-gui) are now open.
  - Can now simulated rail road crossings (see netconvert
    enhancement below). Vehicles will get a red light whenever a
    train is approaching. #1744
  - Added option **--emission-output.precision** {{DT_INT}} for configuring the numerical precision of vehicle
    emissions. #2108
  - Added new
    [departPos](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#departpos)
    value `last` which can be used to maximize flow and still maintain
    vehicle ordering. #2025
  - [Rerouters](../Simulation/Rerouter.md) now apply to
    vehicles that are already on the rerouter edge at the start of
    the active interval. #2207
  - Vehicles that are equipped with a [rerouting
    device](../Demand/Automatic_Routing.md) now incorporate
    knowledge about the current traffic state when computing a new
    route due to encountering a [rerouter
    object](../Simulation/Rerouter.md#assigning_a_new_destination). #2197
  - [Parking](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops)
    vehicles are no included in the
    [FCD-Output](../Simulation/Output/FCDOutput.md).
  - Riding [persons](../Specification/Persons.md#rides) and
    [containers](../Specification/Containers.md#transports) are
    now included in
    [FCD-Output](../Simulation/Output/FCDOutput.md) and
    [Netstate-Output](../Simulation/Output/RawDump.md). #1726, #1727
  - The [car following
    model](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#car-following_models)
    can now be defined using the new `vType`-attribute
    [carFollowModel](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
    (as alternative to using a child XML-element). #1968
  - Added new [PHEMlight](../Models/Emissions/PHEMlight.md)
    version. #2206

- sumo-gui
  - Adjusted zooming distance when centering on simulation objects
    to make objects easier to find. #2146
  - Added option **--waiting-time-memory** {{DT_TIME}} for tracking accumulated waiting time of vehicles.
    The accumulated waiting time (seconds of waiting time within the
    configured interval, default 100s) can be inspected in the
    vehicle's parameter window and the vehicles can be colored
    according to this value. #999
  - Lanes can now be colored *by routing device assumed speed*. This
    is an exponential moving average of mean travel speeds used for
    [dynamic rerouting](../Demand/Automatic_Routing.md). #2189

- MESO
  - Added option **--meso-tls-penalty** {{DT_FLOAT}} as an alternative way to model the delay effects
    of traffic lights. When this option is set to a positive value,
    the expected delay time for each controlled link (based on red
    duration and cycle duration) is added to the travel time,
    multiplied with the {{DT_FLOAT}} argument. By calibrating the parameter, the
    quality of TLS coordination can be modeled. #2199

- MESO-GUI
  - Coloring vehicles *by depart delay* is now working. Added
    *depart delay* to the vehicle parameter dialog.
  - Added *event time*, *entry time* and *block time* to the vehicle
    parameter dialog. These values record when a vehicle leaves,
    entered and was blocked on an edge segment.

- netconvert
  - Added new [node type
    *rail_crossing*](../Networks/PlainXML.md#node_types)
    to model behavior at a rail road crossings. #1744

- polyconvert
  - Added default typemaps similar to netconvert. #1853

- duarouter
  - Added person trips and
    [IntermodalRouting](../IntermodalRouting.md)
  - When a flow has a stop definition with attribute `until`, the time is
    shifted for each successive vehicle in the flow. #1514

- marouter
  - Added bulk routing and better OD cell handling for speed
    improvements. #2167

- TraCI
  - The [python
    client](../TraCI/Interfacing_TraCI_from_Python.md) is now
    thread safe when using multiple connections in parallel. Each
    opened connection returns an independent TraCI instance. #2091
  - Added support for vehicle commands to the [C++ TraCI
    library](../TraCI/C++TraCIAPI.md) Thanks to Alexander
    Weidinger for the patch. #2008
  - Added new [TraaS
    commands](../TraCI.md#interfaces_by_programming_language)
    *Edge.getLastStepPersonIDs, Person.getNextEdge,
    Vehicle.getRouteIndex, Vehicle.getStopState, Vehicle.isStopped*
    and some more stop-related vehicle commands.
  - The angle argument of vehicle command [*move to
    VTD*](../TraCI/Change_Vehicle_State.md#command_0xc4_change_vehicle_state)
    now overrides the vehicle angle for drawing and
    [fcd-output](../Simulation/Output/FCDOutput.md). #2131
  - Added new simulation command [*save state
    0x95*](../TraCI/Change_Simulation_State.md#command_0xcc_change_simulation_state)
    for saving the current simulation state. #2191

- Tools
  - [tls_csv2SUMO.py](../Tools/tls.md#tls_csv2sumopy) now
    handles controlled edges within a joined traffic light
    definition automatically.
  - Added option **--ignore-connections** to
    [netcheck.py](../Tools/Net.md#netcheckpy). This can be
    used to compute all components in the node graph without
    considering lane-to-lane connections.
  - Added option **--symmetrical** to
    [generateBidiDistricts.py](../Tools/District.md#generatebididistrictspy)
    (previously undocumented tool). This can improve
    opposite-finding in conjunction with option **--radius**.
  - Added option **--geo** to
    [route2poly.py](../Tools/Routes.md#route2polypy). This can
    be used to visualize routes from one network within another
    network.

### Other

- Miscellaneous
  - Visual Studio project files have been updated MSVC12. While it
    is still possible to build SUMO with MSVC10, this support may be
    dropped in the future in favor of C++11.

- Simulation
  - When [saving simulation
    state](../Simulation/SaveAndLoad.md) as *XML*, lane
    elements now contain their id for easier inspection.
  - The
    [departPos](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#departpos)
    values `maxSpeedGap`, `pwagSimple` and `pwagGeneric` were removed since they never worked as intended.
  - Option **lanechange.allow-swap** is now deprecated.

- sumo-gui
  - Vehicle type parameters were moved to a separate dialog box
    (*Show Type Parameter*). #2133

- TraCI
  - TraCI version is now 11.

- Documentation
  - Added page on [saving and loading simulation
    state](../Simulation/SaveAndLoad.md)
  - [Arrival parameters are now
    documented](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#a_vehicles_depart_and_arrival_parameter)
  - Extended [netedit](../netedit.md) documentation.
  - Described [netdiff.py](../Tools/Net.md#netdiffpy), a tool
    for comparing networks which was undocumented for a long time.
  - Added page on [modelling networks for motorway simulation. In
    particular
    on-off-ramps](../Simulation/Motorways.md#building_a_network_for_motorway_simulation)
  - Added new [overview page for usage of elevation
    data](../Networks/Elevation.md)
  - Added documentation [on influencing the simulation via
    sumo-gui](../sumo-gui.md#influencing_the_simulation)
  - Added detailed [License](../Libraries_Licenses.md) information.
  - All applications report some build configuration when called
    without options. #2118