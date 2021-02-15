---
title: Z/Changes from Version 0.27.1 to Version 0.28.0
permalink: /Z/Changes_from_Version_0.27.1_to_Version_0.28.0/
---

## Version 0.28.0 (01.11.2016)

### Bugfixes

- Simulation
  - Fixed bug that was causing insufficient overtaking on a
    single-lane road when using sublane simulation (most noticeable
    with small step-lenghts). #2518
  - Fixed invalid count of insertion-backlog for flows. #2535
  - Added check for possibility to stop at scheduled stops during
    insertion. #2543
  - Fixed invalid timeLoss values in
    [tripinfo-output](../Simulation/Output/TripInfo.md) (last
    step was counted twice for some vehicles).
  - Fixed bug that would prevent a vehicle from reaching the end of
    it's route after rerouting while being teleported. #2554
  - Fixed erroneous detector output for a special situation. #2556
  - Fixed computation of energy loss for subsecond simulation. #2559
  - Fixed crash when using
    [closing-rerouters](../Simulation/Rerouter.md#closing_a_street)
    that prevent vehicles from departing after being loaded (related
    to #2561).
  - Fixed invalid permissions when using multiple
    [closing-rerouters](../Simulation/Rerouter.md#closing_a_street)
    that modify permissions of the same edge #2560.
  - Fixed crash when using the [sublane model](../Simulation/SublaneModel.md) with varying lane
    widths. #2585
  - The option **--ignore-errors** now properly ignores `vClass`-related errors. #2148
  - Vehicles are now properly angled during lane-changing in
    [sublane-simulation](../Simulation/SublaneModel.md). #2525

- MESO
  - Tripinfo-output now has appropriate `timeLoss` values. #2570

- netconvert
  - Fixed invalid lane permissions when setting lane-specific
    attributes without specifying lane-specific permissions. #2515
  - Fixed invalid elevation values at junctions when importing
    OpenDRIVE networks.
  - Fixed crash when importing OpenDRIVE networks.
  - Fixed invalid internal-lane geometries when exporting OpenDRIVE
    networks. #2562
  - Fixed various specification conformity issues when exporting
    OpenDRIVE networks.
  - Option **--street-sign-output** now only generates signs at *real* intersections.
  - Fixed error when loading a *.net.xml* file and patching lane
    numbers with an *.edg.xml* file. #2459

- sumo-gui
  - The visulization option *show lane borders* is now working.
  - Fixed crash when user loads an empty E3 detector #2249

- netedit
  - Geometry nodes are no longer drawn when set to full transparency
    (alpha = 0).
  - Geometry nodes are now colored for selected edges.
  - All textures are now loaded instead of a white square or a
    netedit crash. #2594 #2476 #2597

- dfrouter
  - Vehicles now always depart with the measured speeds. A vehicle
    type with non-zero `speedDev` is used to allow patching the
    [vehicle-specific speed factor](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
    in [sumo](../sumo.md). #2510

- TraCI
  - Vehicles added via method add() of the python client now have
    the correct departDelay value. #2540
  - Vehicle type value retrieval via the [C++ API](../TraCI/C++TraCIAPI.md) is now working.
  - Setting and retrieving edge travel times and efforts via the
    [C++ API](../TraCI/C++TraCIAPI.md) is now working.
  - It is now possible to specify routes that contain
    [TAZ-edges](../Demand/Importing_O/D_Matrices.md#describing_the_taz). #2586


### Enhancements

- Simulation
  - [Ballistic integration method](../Simulation/Basic_Definition.md#defining_the_time_step_length)
    can be used by giving the option **--step-method.ballistic**.
  - [Areal Detectors](../Simulation/Output/Lanearea_Detectors_(E2).md)
    now output vehicle counts. #2523
  - [Rerouters can now be used to close individual lanes](../Simulation/Rerouter.md#closing_a_lane) to
    simulate dynamic work zones and accidents. #2229
  - Electricity consumption is now included in emission outputs. #2211
  - Added option **--save-state.period** {{DT_TIME}} for periodically [saving simulation state](../Simulation/SaveAndLoad.md#saving). The new option
    **--save-state.suffix** {{DT_STR}} can be used to determine whether state should be saved in
    binary (*.sbx*) or XML (*.xml*). #2441
  - Tripinfo-output now includes the values `departPosLat,arrivalPosLat` when running a
    [sublane-simulation](../Simulation/SublaneModel.md).
  - Sublane simulation now supports the [vehicle parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes) `departPosLat,arrivalPosLat`. #2003
  - Added option **--tls.all-off** {{DT_BOOL}} for switching off all traffic lights in the
    simulation.

- netedit
  - Added new option **--new** {{DT_BOOL}} for starting with an empty network.
  - Added new option **--window-size** {{DT_INT}},{{DT_INT}} for specifying the initial window size. #2514
  - Junction attribute *tltype* is now supported.
  - Joined TLS can now be defined by setting the junction attribute
    *tl* following the same rules as in [*.nod.xml* files](../Networks/PlainXML.md#node_descriptions). #2389
  - Added support for restricted lanes (Sidewalks, buslanes and
    bikelanes). #1568
  - Now junction visualization can be toggled between bubbles and
    their real shape when in *Move*-mode.
  - Connections can now be shown in inspect mode (by activating a
    check-box) and their attributes can be edited. They can also be
    selected in select-mode and deleted in delete mode. When
    inspecting edges or lanes, controls for accessing their
    connections are now available. #2067
  - Now delete mode has a frame with information and options for
    deleting. #2432
  - Intervals of Rerouters can be edited using a dialog #2480

- sumo-gui
  - Added new option **--window-size** {{DT_INT}},{{DT_INT}} for specifying the initial window size. #2514
  - Added new visualization setting for drawing right-of-way rules
    (colored bars).
  - [Background images (decals)](../sumo-gui.md#using_decals_within_sumo-gui) now
    support the new Boolean attribute *screenRelative*. When set to
    *true*, position and size are relative to the drawing window
    instead of being relative to the network. #2438

- netconvert
  - Ferry routes are now imported from [OSM when using the *ships* typemap](../Networks/Import/OpenStreetMap.md#recommended_typemaps).
  - Railway crossings are now imported from
    [OSM](../Networks/Import/OpenStreetMap.md). #2059
  - Added new option **--junctions.scurve-stretch** {{DT_FLOAT}} which generates longer intersection shapes
    whenever the number of lanes changes. This allows for smoother
    trajectories and is recommended when [writing OpenDRIVE networks](../Networks/Further_Outputs.md#opendrive_road_networks). #2522
  - Added new option **--rectangular-lane-cut** {{DT_BOOL}} which prevents oblique angles between edges
    and intersections. This option is automatically enabled when
    exporting OpenDRIVE networks. #2562
  - The distance below which all approaching vehicles on foe lanes
    to a specific connection are visible can now be [customized with the new connection attribute `visibility`](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which).
    #2123 , #892
  - Elevation data is now interpolated when importing GeoTiff
    heightmaps. #1962
  - The geometry of [OpenDRIVE networks](../Networks/Further_Outputs.md#opendrive_road_networks)
    is now exported with parameterized curves according to
    specification version 1.4. #2041
  - Elevation data is now [imported from OpenDRIVE](../Networks/Import/OpenDRIVE.md) and [exported to OpenDRIVE](../Networks/Further_Outputs.md#opendrive_road_networks).
    #2239 , #2240
  - Added new option **--default.disallow** {{DT_STR}}**,...** for simplifying specification of [vehicle permissions](../Simulation/VehiclePermissions.md#network_definition) #2557
  - When converting an OSM network to [writingDlrNavteq or Amitran format](../Networks/Export.md), functional road class is
    now based on OSM highway types. #2602

- dfrouter
  - Vehicle types are now included in the
    [emitters-output](../Demand/Routes_from_Observation_Points.md#saving_flows_and_other_values)
    by default. The new option **--vtype-output** {{DT_FILE}} allows redirection the vTypes into a
    separate file.
  - If the average measured speeds are systematically above the
    speed limit of the respective roads, the written vTypes use
    attribute
    [speedFactor](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
    to reflect systematic speeding.
  - Added new option **--speeddev** {{DT_FLOAT}} to set a [speed deviation](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
    for the generate vTypes

- TraCI
  - Added new convenience function *traci.start* that automatically
    selects a port, starts sumo on that port and connects. #2236
  - Added new get/set command *height (0xbc)* to retrieve and modify
    vehicle height (applicable to vehicles and vehicle types). #2573
  - When adapting edge travel times and efforts, the (optional)
    parameters *begin* and *end* are now supported by the [python client](../TraCI/Interfacing_TraCI_from_Python.md) and the
    [C++ client](../TraCI/C++TraCIAPI.md). #2584

- Tools
  - The tool
    [route_departOffset.py](../Tools/Routes.md#route_departoffset)
    (previously undocumented) now supports additional options for
    departure time modification based on departure edge and for
    shifting departures within one time interval to another
    interval. #2568
  - [randomTrips.py](../Tools/Trip.md) supports the new alias **--edge-permission** for former option **--vclass**.

### Other

- Documentation
  - Added Tutorial for using [the OSM-Web-Wizard](../Tutorials/OSMWebWizard.md).
  - More details on [loading and inserting vehicles](../Simulation/VehicleInsertion.md)
  - Added new overview page on [vehicle permissions](../Simulation/VehiclePermissions.md)
  - The [netedit](../netedit.md) documentation on old and novel
    features has been expanded.
  - The tool
    [showDepartsAndArrivalsPerEdge.py](../Tools/Routes.md#showdepartsandarrivalsperedge)
    is now documented. It allows edge-coloring in
    [sumo-gui](../sumo-gui.md) according to traffic statistics #2263

- TraCI
  - TraCI version is now 13

- Miscellaneous
  - The full 24h-dataset for the
    [TAPASCologne](../Data/Scenarios/TAPASCologne.md) scenario
    is now available
  - The deprecated option **--lanechange.allow-swap** is no longer supported. #2116
  - The `laneChangeModel="JE2013"` has been removed due to strong similarity with *LC2013*.
    Using the model is deprecated and now defaults to *LC2013*
  - [SUMOPy](../Contributed/SUMOPy.md) has been added to the
    SUMO repository and to the relase. Many thanks to Joerg
    Schweizer.
  - The libraries for the Windows build and release were updated to
    Fox 1.6.52 and Xerces-C 3.1.4 (with an additional fix for
    [XERCESC-2052](https://issues.apache.org/jira/browse/XERCESC-2052))