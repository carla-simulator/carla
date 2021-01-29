---
title: Z/Changes from Version 0.28.0 to Version 0.29.0
permalink: /Z/Changes_from_Version_0.28.0_to_Version_0.29.0/
---

## Version 0.29.0 (16.02.2017)

### Bugfixes

- Simulation
  - Fixed emergency braking and collisions related to
    [opposite-direction driving](../Simulation/OppositeDirectionDriving.md). #2652
  - Fixed crashing related to [opposite-direction driving](../Simulation/OppositeDirectionDriving.md). #2664
  - Fixed implausible behavior related to [opposite-direction driving](../Simulation/OppositeDirectionDriving.md). #2665
  - Fixed error where vehicles could not stop at the end of a lane
    for numerical reasons. #2670
  - [Generic parameters](../Simulation/GenericParameters.md) of
    vehicles and vehicle types are now handled when [saving and loading simulation state](../Simulation/SaveAndLoad.md). #2690
  - Stopped and parking vehicles are now handled when [saving and loading simulation state](../Simulation/SaveAndLoad.md). #1301
  - All vehicle parameters and vehicle stops are now handled when
    [saving and loading simulation state](../Simulation/SaveAndLoad.md). #2720
  - Fixed extreme vehicle angles during lane changing when using the
    [sublane model](../Simulation/SublaneModel.md). #2741
  - Fixed error when defining flows without vehicles. #2823
  - Fixed issue where the heuristic to prevent junction blocking
    caused a vehicle to block itself. #2834
  - Fixed bug that was causing pedestrian collisions. #2840
  - The last vehicle from a flow is no longer missing if the flow
    duration is not a multiple of the *period* parameter. #2864

- netconvert
  - Fixed errors when exporting elevation data to OpenDRIVE. #2641
  - Fixed format conformity issues when exporting OpenDRIVE
    networks. #2673, #2674, #2682
  - Networks exported in the [DlrNavteq format](../Networks/Export.md#dlrnavteq) are now written
    with **--numerical-ids** by default.
  - Fixed crash when importing OpenDrive and using [edge-removal options](../netconvert.md#edge_removal). #2685
  - Improved connection guessing at roads with a lane reduction.
  - Fixed crash when specifying a
    [`<split>`](../Networks/PlainXML.md#road_segment_refining)-element
    for a roundabout-edge. #2737
  - The option **--geometry.max-segment-length** is now working when importing OSM data. #2779
  - Improved heuristic for option **osm.layer-elevation** to reduce unrealistic up-and-down
    geometries.
  - Fixed steep grades near intersections in networks with
    3D-geometry. #2782
  - Geometry fixes for output in [DlrNavteq format](../Networks/Export.md#dlrnavteq). #2785, #2786

- netedit
  - Fixed crash when doing undo/redo connection changes at a newly
    created junction. #2662
  - Fixed slow operation when switching between move-mode and other
    modes in a large network.
  - Fixed slow operation when move junctions in large networks. #2699
  - Selecting objects by [matching against attributes](../netedit.md#match_attribute) now works on
    windows. #2675
  - Fixed crash when setting custom geometry endpoints. #2693
  - Fixed shortcuts #2694
  - Fixed crash when using option **--numerical-ids**.
  - Fixed visualization errors when using **--offset.disable-normalization *false* **
  - When adding sidewalks to selected lanes, pedestrians are now
    automatically forbidden on the other lanes. #2708
  - Fixed an inconsistency with the definition of SUMO Time in
    Inspector mode. #2625

- sumo-gui
  - Fixed bug that was causing interface lag when right-clicking in
    networks with detailed geometry.
  - Configuring the visualisation of generated induction loops for
    [actuated traffic lights](../Simulation/Traffic_Lights.md#actuated_traffic_lights)
    is now working. #2639
  - Fixed crash when coloring lanes by occupancy. #2850

- MESO
  - when using option **--meso-tls-penalty** the maximum flow at controlled intersections
    is now scaled according to fraction of available green time. #2753
  - Fixed various issues related to inconsistent handling of net and
    gross time gaps. This was causing exaggerated speeds. The option
    **--meso-taujf** {{DT_TIME}} now corresponds to the net time-gap (as do **--meso-taufj**, **--meso-tauff**). It's default
    value has been reduced from *2.0* to *1.73* correspondingly
    (reproducing the old behavior at 100km/h).

- MESO-GUI
  - Fixed invalid coloring of the gap between lanes. #1428

- duarouter
  - The default vehicle class is now *passenger* (as in the
    simulation). Note, that non-passenger classes such as trains now
    need an explicit type definition to be able to use rail edges. #2829
  - The last vehicle from a flow is no longer missing if the flow
    duration is not a multiple of the *period* parameter. #2864
  - Various fixes to [intermodal routing functionality](../IntermodalRouting.md). #2852, #2857

- TraCI
  - Fixed crash when trying to reroute vehicles before their
    departure.
  - Fixed inefficiency when calling *vehicle.rerouteTravelTime()*
    multiple times per simulation step (edge weights are now updated
    at most once per step).
  - Various fixes to *vehicle.moveToXY* mapping behavior.
  - unsubscribing now works in the python client. #2704
  - The C++ client now properly closes the simulation when calling
    *close()*
  - Subscriptions to 2D-Positions are now working in the C++ client
  - Deleting vehicles that have not yet departed is now working. #2803
  - [Brake lights and blinkers](../TraCI/Vehicle_Signalling.md)
    can now be set for the current simulation step. #2804
  - Vehicle function *move to XY* now works for edges with custom
    length. #2809
  - Vehicle function *move to XY* now actually uses the 'edgeID' and
    'lane index' parameters to resolve ambiguities. It uses the
    'origID' parameter if set and the lane id otherwise.
  - Function *edge.adaptTravelTime(begin=... end=...)* is now
    working as expected with times in seconds. #2808

- Tools
  - [netdiff.py](../Tools/Net.md#netdiffpy) now correctly
    handles changes to `spreadType` and other optional attributes. #2722
  - [sumolib.net](../Tools/Sumolib.md) now returns consistent
    values for *edge.getShape()*: the center-line of all lanes.
    Previously, the raw edge shape used in netconvert was returned.
  - [randomTrips.py](../Tools/Trip.md#randomtripspy) now works
    correctly when giving the options **--via, --validate** at the same time. #2821

- Miscellaneous
  - The [special sub-string *TIME*](../Basics/Using_the_Command_Line_Applications.md#writing_files)
    in output file names is now working on Windows.
  - Various fixes concerning the binary XML format #2650, #2651

### Enhancements

- Simulation
  - Added definition of [parking areas](../Simulation/ParkingArea.md) and functionality for
    [rerouting in search of a free parking space](../Simulation/Rerouter.md#rerouting_to_an_alternative_parking_area).
    Many thanks to Mirco Sturari for this contribution.
  - Added new output option **--stop-output** {{DT_FILE}} for generating [output on vehicle stops (for public transport, logistics, etc.)](../Simulation/Output/StopOutput.md)
  - Vehicles that are inserted with `departPos="last"` now depart at the end of the
    lane when there are no leader vehicles. This reduces the overall
    variance in depart positions. #2571
  - The **--step-log-output** now includes statistics on time spend in TraCI functions. #2698
  - When a [vClass is specified for a vehicle type](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_emission_classes),
    this information is used to [assign a default emissionClass](../Vehicle_Type_Parameter_Defaults.md) from
    the [HBEFA3 model](../Models/Emissions/HBEFA3-based.md)
    (Before all vehicles had class
    ["HBEFA2/P_7_7"](../Models/Emissions/HBEFA-based.md) by
    default, even bicycles). The default class for passenger
    vehicles is now "HBEFA3/PC_G_EU4".
  - Output files of the
    [MultiEntryExitDetector](../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md)
    now include *timeLoss* information. #2703
  - Output files of the
    [LaneAreaDetector](../Simulation/Output/Lanearea_Detectors_(E2).md)
    now include *timeLoss* information. #2703
  - Added option **--load-state.remove-vehicles ID1\[,ID2\]\*** for removing vehicles from a loaded state. #2774
  - Pedestrians now use the *speedDev* attribute of their type to
    vary their walking speed. #2792
  - [tripinfo-output](../Simulation/Output/TripInfo.md#generated_output)
    now includes the attribute *speedFactor* which may have been
    chosen randomly when loading the vehicle.
  - Output precision is automatically increased whenever simulating
    with step-lengths below 10ms. #2816
  - E1 detectors now support the attribute *vTypes* for collecting
    type specific measurements. The attribute *splitByType* is no
    longer supported.

- sumo-gui
  - Added new option **--window-pos** {{DT_INT}},{{DT_INT}} for specifying the initial window placement.
  - Added new button to the object locator dialog that allows
    toggling selection status. #2687
  - Parking vehicles are now listed in the vehicle locator dialog.
    This can be disabled using a new menu option. Teleporting
    vehicles can optionally be listed as well #2750
  - Vehicles can now be colored *by time loss* (accumulated over the
    whole route). The time loss is also shown in the vehicle
    parameter dialog.

- MESO
  - Added new option **--meso-minor-penalty** {{DT_TIME}} for applying a time penalty when passing a
    minor link. This may be used to model slow down on approach or
    even intersection delays when running without **--meso-junction-control**. #2640
  - When using the option **--meso-tls-penalty** {{DT_FLOAT}} to model mesoscopic traffic lights, the
    maximum capacity of edges is now reduced according to the
    proportion of green-time and cycle time at controlled
    intersection. #2753

- MESO-GUI
  - The number of queues for the current segment is now shown in the
    edge parameter dialog
  - The latest headway for the current segment is now shown in the
    edge parameter dialog

- netconvert
  - Pedestrian rail crossings are now working. #2654
  - Added new option **--geometry.check-overlap** {{DT_FLOAT}} for finding edges with overlapping lanes. This
    is typically a sign of faulty inputs. The accompanying option **--geometry.check-overlap.vertical-threshold** {{DT_FLOAT}}
    filters out edges that are separated vertically by at least the
    given value (default *4.0*). #2648
  - The options **--keep-edges.explicit, --keep-edges.input-file** now allow for white-listing in combination with
    other removal options (edges are kept if listed explicitly or if
    any other keep-condition is met). #2680
  - Added new option **--osm.oneway-spread-right** {{DT_BOOL}} for setting the default
    [spreadType](../Networks/PlainXML.md#edge_descriptions)
    to *right* for one-way edges (i.e. motorways).
  - The
    [`<split>`](../Networks/PlainXML.md#road_segment_refining)-element
    now supports the new attributes *idBefore, idAfter* to determine
    the ids of the newly created edges. #2731
  - Added new option **--reserved-ids** {{DT_FILE}} for loading a selecting of node and edge ids
    that shall be avoided in the output network.
  - When importing networks from
    [DlrNavteq](../Networks/Import/DlrNavteq.md) format, the
    new option **--construction-date YYYY-MM-DD** can now be used to interpret the readiness of roads
    under construction.
  - Networks exported to
    [DlrNavteq](../Networks/Export.md#dlrnavteq) format now
    contain additional information: bridge/tunnel information, *form
    of way*, ZIP code, prohibited_manoeuvres, connected_lanes
  - Warnings are now issued when steep grades are present in the
    road network. The warning threshold can be configured using the
    new option **geometry.max-grade** {{DT_FLOAT}}.
  - <laneOffset\> data is now imported from OpenDrive networks

- netedit
  - Added new option **--window-pos** {{DT_INT}},{{DT_INT}} for specifying the initial window placement.
  - new [hotkeys](../netedit.md#hotkeys) implemented. #2694
  - New icons for edit modes. #2612
  - Added new button to the object locator dialog that allows
    toggling selection status. #2687

- od2trips
  - Added new option **--flow-output.probability** {{DT_BOOL}} for generating probabilistic flows instead of
    evenly spaced flows. Thanks to Dominik Buse for the patch.

- duarouter
  - Added option **persontrip.walkfactor** {{DT_FLOAT}} to account for pedestrian delays in [intermodal routing](../IntermodalRouting.md#intermodal_cost_function). #2856


- TraCI
  - Added functions *person.getStage, person.getRemainingStages*,
    *person.getVehicle* and *person.getEdges* to the API, the python
    client and the C++ client. #1595
  - Added functions *person.add*, *person.appendWalkingStage*,
    *person.appendDrivingStage*, *person.appendWaitingStage*,
    person.removeStage'', *person.removeStages, person.setColor,
    person.setLength, person.setHeight, person.setWidth,
    person.setMinGap, person.setType* and *person.setSpeed* to the
    API, the python client and the C++ client. #2688
  - The python client now supports *vehicle.getPosition3D*.
  - Added the functions *vehicle.getLine* and *vehicle.setLine* to
    the python client and the C++ client [(to be used for public transport)](../Specification/Persons.md#riding) #2719
  - Added the functions *vehicle.getVia* and *vehicle.setVia* to the
    python client and the C++ client (affects subsequent rerouting
    calls) #2729
  - Added the functions *polygon.getFilled* and *polygon.setFilled*
    to the python client

- Tools
  - [route_departOffset.py](../Tools/Routes.md#route_departoffset)
    now supports additional options for departure time modification
    based on arrival edge. #2568
  - [sumolib.net](../Tools/Sumolib.md) now supports the new
    function *edge.getRawShape()* to retrieve the shape used by
    netconvert. #2742

- Miscellaneous
  - All applications now support the option **--precision** {{DT_INT}} to specify the output
    precision as number of decimal places for floating point output.
    For lon/lat values this is configured separately using option **--precision.geo** {{DT_INT}}.

### Other

- Documentation
  - Added description of the [Object Locator](../sumo-gui.md#selecting_objects)-menu
  - Added documentation of [visualizing road access permissions](../sumo-gui.md#road_access_permissions)
  - Added documentation of [crossings](../netedit.md#crossings) in netedit
  - Added documentation for the [MESO-model](../Simulation/Meso.md#model_description)
  - Extended documentation of the [sublane-model](../Simulation/SublaneModel.md)
  - Added [TraCI performance information](../TraCI.md#performance)
  - The [TraCI command reference](../TraCI.md#traci_commands) now includes links to the corresponding python method(s).
  - Fixed inconsistencies in [TraCI command reference](../TraCI.md#traci_commands) (mostly methods that
    were available in the python client but not yet documented in
    the wiki).

- Simulation
  - A warning is now issued when trying to define a vehicle with
    vClass=*pedestrian* as this vClass should only be used for
    persons. #2830

- TraCI
  - TraCI version is now 14

- Binary format
  - SBX has now version number 2 #2651

- Miscellaneous
  - [sumo-gui](../sumo-gui.md) and
    [netedit](../netedit.md) now remember their last window position