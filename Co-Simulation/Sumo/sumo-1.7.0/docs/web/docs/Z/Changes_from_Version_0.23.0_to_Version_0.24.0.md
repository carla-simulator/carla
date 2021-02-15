---
title: Z/Changes from Version 0.23.0 to Version 0.24.0
permalink: /Z/Changes_from_Version_0.23.0_to_Version_0.24.0/
---

## Version 0.24.0 (02.09.2015)

### Bugfixes

- Simulation
  - Fixed default arrivalPos when loading `<trip>` or `<flow>` elements using
    attributes `from, to` directly into SUMO. #1739
  - Fixed crash when specifying consecutive walks for the same
    person. #1781
  - Fixed bug that caused pedestrians to get too close to each
    other. #1769
  - Fixed crash and other bugs when using option **--lanechange.duration**. #1152, #1795, #1796, #1797
  - Fixed bug that sometimes caused the rear end of vehicles to be
    placed on the wrong lane after lane-changing. #1804
  - Rerouters where closed edges are disabled for specific vehicle
    classes now cause these vehicles to wait until the closing ends
    if the destination edge is closed.
  - Vehicles with a `<stop>` in their route are now being overtaken if there
    is sufficient space. This was causing problems when modelling
    parked vehicles on a multi-lane road.
  - Fixed bug that resulted in invalid routes when routing at
    simulation time (at intersections where a required connection
    originates from a prohibited lane). #1861
- sumo-gui
  - Fixed centering of names on objects (a large mismatch was
    visible for persons).
  - Polygons are no longer drawn when setting their
    size-exaggeration to 0.
- duarouter
  - Custom car following model specification is now preserved in the
    route output. #1832
  - Fixed bug when using option **--remove-loops** where the start/destination edge
    changed needlessly.
  - Vehicles may now depart/arrive at any *TAZ* regardless of their
    vehicle class.
- netconvert
  - When adding crossings to a *.net.xml* without internal links,
    the output network will be built with internal links. #1729
  - Fixed bug where duplicate crossings between the same pair of
    walkingareas where sometimes build when using option **--crossings.guess**. #1736
  - Fixed bugs where loading a *.net.xml* file and removing edges
    would lead to an invalid network. #1742, #1749, #1753
  - Permissions are no longer lost when guessing ramps. #1777
  - Defining pedestrian crossings at a node with `type="traffic_light_unregulated"` now works. #1813
  - Fixed invalid TLS-plans when loading a *.net.xml* file with
    TLS-controlled pedestrian crossings.
  - Connections between lanes with incompatible vehicle classes are
    no longer generated. #1630
  - Fixed generation of connections at intersections with dedicated
    bicycle lanes (symptoms were invalid connections and missing
    connections).
  - Modifying lane permissions in a network with crossings, so that
    crossings are no longer valid, is now working (invalid crossings
    are removed with a warning).
  - Fixed bug that caused invalid lane lengths when building with
    option **--no-internal-links** (near intersections with sharp angles).
  - Fixed some invalid clusters when using option **--junctions.join**.
- TraCI
  - Fixed python API for [function *move to VTD* (0xb4)](../TraCI/Change_Vehicle_State.md). #1720
  - Fixed subscriptions for `<laneAreaDetector>`
  - Vehicle command *move to* (0x5c) can now be used to forcefully
    insert vehicles which have not yet entered the network. #1809
  - Parking vehicles now return a reasonable position (and other
    values which do not depend on being on a lane). #1809
  - Fixed screenshots for Linux. #1341
  - The close command returns now a little later to have more data
    written to files (still not completely flushed though, see #1816)
- netedit
  - Fixed bug where unselected objects were wrongly selected after
    undoing deletion.
  - No longer creating invalid network when loading and saving a
    network with split crossings.
- Tools
  - [randomTrips.py](../Tools/Trip.md#randomtripspy) no longer
    attempts to find a fringe based on edge-direction when using
    option **--pedestrians**. #1737
- All Applications
  - Fixed invalid paths when using option **--output-prefix** {{DT_STR}} and loading a
    configuration file in a subdirectory. #1793

### Enhancements

- Simulation
  - Simulation of [electric vehicles is now supported](../Models/Electric.md) with a new model for
    energy consumption and battery charging.
  - Maximum time that may be simulated increased from 24 days to 290
    million years. #1728
  - [`<walk>`-definitions](../Specification/Persons.md#walking) now support
    and `departPos="random"` and `arrivalPos="random"`.
  - **--fcd-output** now includes z-data if the network has elevation. #1751
  - [`<vTypeProbe>`](../Simulation/Output/VTypeProbe.md) output now
    includes z-data if the network has elevation.
  - Device [assignment by `<param>`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices)
    can now be used to override device assignment by option **--device.<DEVICENAME\>.probability**.
  - Added new `vType`-attribute `carFollowModel` which can be used instead of a child
    element when declearing the car following model. #663
  - Vehicles which do not have a route and cannot find one on
    insertion get discarded when **--ignore-route-errors** is given. #1825
  - **--vehroute-output** now includes the route length. #1790

- sumo-gui
  - The view can now be moved and zoomed via [keyboard shortcuts](../sumo-gui.md#keyboard_shortcuts). #298
  - Lanes which disallow passenger cars (i.e. paths and service
    roads) now have a distinct shade of grey. The color can be
    customized in the gui-settings dialog.
  - Vehicles now activate their blinker during continuous lane
    change manoeuvres.
  - The types of POIs and Polygons can now be displayed via *View
    Settings*. #1803
  - The `personNumber,containerNumber,personCapacity,containerCapacity` information for a vehicle is now shown in the parameter
    dialog. #1617
  - The GUI asks at simulation end whether all files and windows
    should be closed
- netconvert
  - added option **--remove-edges.input-file** {{DT_FILE}} which works as an analogue to **--keep-edges.input-file**. Giving either
    option a selection file (where edge ids are prefixed with
    **edge:** as argument is now supported.
  - added option **--ramps.unset** {{DT_STR}} which works as an analogue to **--ramps.set** and prevents edges
    from being treated as on- or off-ramps. #1763
  - Now importing signalized pedestrian crossings from
    [OSM](../Networks/Import/OpenStreetMap.md) (`<tag k="crossing" v="traffic_signals"/>`).
  - added new option **--default.junctions.keep-clear** {{DT_BOOL}} and new attributes `<node keepClear="..."/> <connection keepClear="..."/>` [for allowing drivers to drive onto an intersection despite the risk of blocking it for cross-traffic](../Simulation/Intersections.md#netconvert_options_for_allowing_drivers_to_drive_onto_intersections). #1290
  - pedestrian crossings may be removed using the [new attribute `discard`](../Networks/PlainXML.md#pedestrian_crossings).
  - when splitting an edge, the new node is not removed by option **--geometry.remove**
    even when speed and lane count remain unchanged. #1842
  - Connections from and to sidewalks are only generated when also
    building pedestrian crossings since they are superfluous
    otherwise. When using pedestrian model *nonInteracting* these
    connections are not used (pedestrians *jump* across
    intersections between any two sidewalks) and when using model
    *striping*, crossings are mandatory.
  - Bicycle lanes are now imported from [OSM (when using the appropriate typemap)](../Networks/Import/OpenStreetMap.md#recommended_typemaps). #1289
  - Bus lanes are now imported from [OSM](../Networks/Import/OpenStreetMap.md). #1682
  - Improved control over edge types (typemaps) when importing from
    [OSM](../Networks/Import/OpenStreetMap.md). All defaults
    can now be overridden in a transparent manner and it's easier to
    add additional modes of traffic than ever before [(see documentation)](../Networks/Import/OpenStreetMap.md#recommended_typemaps).
  - Option **--lefthand** now works for generating networks with left-hand
    traffic. Thanks to Andrea Fuksova for suggesting the
    double-mirroring technique.
  - Edge types can now be used to [define *vClass*-specific speed limits](../Networks/PlainXML.md#vehicle-class_specific_speed_limits). #1800
  - Additional attributes are now supported to specify the node that
    is generated when [splitting an edge](../Networks/PlainXML.md#road_segment_refining). #1843

- duarouter
  - option **--ignore-errors** now also applies if no input trips/routes are loaded.
  - Added option **--bulk-routing** {{DT_BOOL}} to improve routing speed when many (similar)
    vehicles depart at the same time from the same location. The
    time aggregation can be controlled using option **--route-steps** {{DT_TIME}} The *bulkstar*
    routing algorithm is now obsolete and no longer supported. #1792
  - Added options **--repair.from** {{DT_BOOL}}, **--repair.to** {{DT_BOOL}} which attempts to repair invalid source or
    destination edges in the route input.
- TraCI
  - added [command to retrieve persons on an edge](../TraCI/Edge_Value_Retrieval.md). Also added the
    corresponding method *traci.edge.getLastStepPersonIDs()* to the
    [python API](../TraCI/Interfacing_TraCI_from_Python.md). #1612
  - added method *traci.vehicle.getStopState()* to the [python API](../TraCI/Interfacing_TraCI_from_Python.md) [(command 0xb5)](../TraCI/Vehicle_Value_Retrieval.md). Also added
    convenience methods *traci.vehicle.isStopped(),
    traci.vehicle.isStoppedParking,
    traci.vehicle.isStoppedTriggered()*.
  - added [command to retrieve the next edge of a walking person](../TraCI/Person_Value_Retrieval.md). Also added the
    corresponding method *traci.person.getNextEdge()* to the [python API](../TraCI/Interfacing_TraCI_from_Python.md). This can
    be used to implement pedestrian-actuated traffic lights.
  - Added named constant *traci.vehicle.DEPART_SPEED_RANDOM*. This
    corresponds to `<vehicle departSpeed="random" .../>`.
  - Added variable retrieval functions for
    [`<laneAreaDetector>`](../TraCI/Lane_Area_Detector_Value_Retrieval.md):
    lane, position, and length. Also added corresponding functions
    to the [python API](../TraCI/Interfacing_TraCI_from_Python.md)
  - added [command to retrieve the index of the vehicles edge within it's route](../TraCI/Vehicle_Value_Retrieval.md). Also
    added the corresponding method *traci.vehicle.getRouteIndex()*
    to the [python API](../TraCI/Interfacing_TraCI_from_Python.md). #1823
  - extended method *traci.vehicle.setStop(...)* to optionally
    include the startPos of a stop.

- polyconvert
  - When importing OSM data, POIs are now raised above the layer of
    polygons and roads by default to make them always visible. #1771
  - Added option **--poi-layer-offset** {{DT_FLOAT}} to control the layer of pois relative to polygons
    (especially in cases where they have the same type).
  - **--offset** {{DT_FLOAT}} options are now respected even when used together with a network.

- netedit
  - The view can now be moved and zoomed via [keyboard shortcuts](../sumo-gui.md#keyboard_shortcuts).
  - Added menu option for replacing junctions by geometry points. #1754
  - Geometry points of parallel edges can now be moved
    simultaneously when both edges are selected.
  - option **--gui-settings-file** is now supported.

- od2trips
  - can now choose only differing sources and sinks #1837

<!-- end list -->

- Tools
  - made edgesInDistricts.py aware of vClasses (and use the sumolib
    for parsing)
  - added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **--binomial** {{DT_INT}} for achieving binomially distributed arrival rates.
  - added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **--validate** {{DT_BOOL}} for generating trips with validated connectivity. #1783
  - [randomTrips.py](../Tools/Trip.md#randomtripspy) now
    supports attributes for `<person>` and `<walk>` definitions when using option **--trip-attributes**.
  - [netcheck.py](../Tools/Net.md#netcheckpy) now supports
    discovering reverse connectivity by using option **--destination**
  - [netcheck.py](../Tools/Net.md#netcheckpy) now supports
    edge permissions by using option **--vclass**
  - [netcheck.py](../Tools/Net.md#netcheckpy) now supports
    writing an edge selecting for every (weakly) connected component
    when using option **--selection-output**
  - [netcheck.py](../Tools/Net.md#netcheckpy) now outputs
    additional statistics in regard to the disconnected components.
    Thanks to Gregory Albiston for the patch.
  - added new tool
    [districts2poly.py](../Tools/District.md#districts2polypy)
    for visualizing districts.
  - added new tool
    [route2sel.py](../Tools/Routes.md#route2selpy) for
    creating an edge selection from a route file.
  - added new tool
    [edgeDataDiff.py](../Tools/Output.md#edgedatadiffpy) for
    comparing two traffic scenarios via their [edgeData output](../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md).
  - [tls_csv2SUMO.py](../Tools/tls.md) now supports multiple
    definitions in a single input file. Thanks to Thomas Lockhart
    for the patch.
  - OSM-scenario-generator script
    [server.py](../Tools/Import/OSM.md) now supports additional
    modes of traffic.
  - *traceExporter.py* now supports filtering to a bounding box. #1774

### Other

- Scenarios
  - An updated version of the [TAPASCologne scenario](../Data/Scenarios/TAPASCologne.md) can be found
    on [the sourceforge download page](http://sourceforge.net/projects/sumo/files/traffic_data/scenarios/TAPASCologne).
    This updates the network to the latest OSM data and
    [netconvert](../netconvert.md) version.
  - An updated version of the [Bologna scenarios](../Data/Scenarios.md#bologna) can be found on
    [the sourceforge download page](https://sourceforge.net/projects/sumo/files/traffic_data/scenarios/Bologna_small/).
    This contains minor network fixes and contains a new pedestrian
    version of the *acosta*-scenario
- Documentation
  - Added new tutorial
    [Tutorials/TraCIPedCrossing](../Tutorials/TraCIPedCrossing.md)
    which shows how to build a pedestrian-actuated traffic light
    controller.
  - Documented [TraCI function *move to VTD* (0xb4)](../TraCI/Change_Vehicle_State.md)
  - cleaned XML schema concerning person capacity and person number
    definition
  - Documented [tllogic files](../Networks/PlainXML.md#traffic_light_program_definition)
  - Documented current state of [bicycle simulation](../Simulation/Bicycles.md), [train simulation](../Simulation/Railways.md) and [waterway simulation](../Simulation/Waterways.md).
  - Added description of [intersection dynamics](../Simulation/Intersections.md).
- TraCI
  - TraCI version is now 10
  - The named constant *traci.vehicle.DEPART_MAX* is now named
    *traci.vehicle.DEPART_SPEED_MAX*. This corresponds to `<vehicle departSpeed="max" .../>`.
- Tools
  - GDAL 2.0 is now supported. Thanks to Thomas Lockhart for the
    patch.
  - osmBuild.py and [server.py](../Tools/Import/OSM.md) no
    longer use option **--geometry.remove-isolated** to avoid removing rivers and railways.
- Misc
  - The scripts *randomTrips.py* and *route2trips.py* moved from
    {{SUMO}}/tools/trip to {{SUMO}}/tools
  - The [OSM typemaps](../Networks/Import/OpenStreetMap.md#recommended_typemaps)
    now disallow vClass *tram* and *ship* where appropriate
  - The tool *sumoplayer* has been removed because it became
    obsolete with the introduction of **--fcd-output** and *traceExporter.py*. #1651
  - The Win64 binaries no longer have a 64 suffix.
  - Error reporting on opening files got a little more verbose.
  - Whitespace in filenames is handled a little bit better