---
title: Z/Changes from Version 0.15.0 to Version 0.16.0
permalink: /Z/Changes_from_Version_0.15.0_to_Version_0.16.0/
---

## Version 0.16.0 (04.12.2012)

### Bugfixes

- Building
  - Fixed building under gcc4.7; thanks to Karsten Roscher
- Simulation
  - Fixed the bug which prevented vehicles to be inserted in the
    middle of a street using `departPos="free"` ([ticket732](http://sourceforge.net/apps/trac/sumo/ticket/732),
    thanks to Björn Hendriks for reporting it)
  - Fixed several bugs that were causing collisions
  - Fixed bug in the default car-following model wich lead to
    larger-that-necessary following distances. The maximum possible
    flow is now greatly increased.
  - Fixed bug where simulation would not terminate if vehicles with
    fixed depart speeds could not be inserted
  - Fixed bug that lead to unnecessarily low departure speeds under
    some conditions
  - Fixed bug which sometimes caused vehicles to use the wrong lanes
- sumo-gui
  - Fixed loading of breakpoints, reported by Anderson Rocha,
    thanks\!
- netconvert
  - reading tls offset from VISUM files corrected
  - OSM-ids are now read as *long long*s (*long long int* under
    Linux)
  - added the option **--tls.discard-loaded** {{DT_BOOL}} which avoids loading traffic lights from other
    formats than XML (in XML, one can simply replace the type
    "traffic_light" by "priority"); removed option **--osm.discard-tls** {{DT_BOOL}} which did the
    same for OSM-networks only
  - SUMO-networks with tls offset now load correctly
  - connections for highway ramps no longer cross incorrectly
  - OpenDRIVE-export: corrected lane offsets (geometry) and lane
    linkage (topology)
  - connections which can not be set when being loaded are kept and
    retried after network changes, such as ramps building, or nodes
    joining
  - now correctly setting edge priorities when importing dlr-navteq
    networks
  - junctions with incoming edges of differing priority can no
    longer have type 'right_before_left'
  - OSM-imported railways are now correctly imported as one-way
    which avoids lots of bogus rail edges
  - OSM-imported railways no longer yield to regular roads
  - OSM-imported railways now have proper default speeds
  - changing attributes *from* or *to* of an existing edge no longer
    crashes
  - improved heuristic for detecting roundabouts. Now recognizes
    roundabouts with divided attachment edges.
  - fixed bug where import would fail due to **tag** elements with
    empty **v**-attribute
  - fixed bug where the special connection attribute *pass* was lost
    when writing plain xml or importing sumo networks (this
    attribute affects the junctionLogic in built networks and
    declares that a connection never yields).
  - fixed bug that caused right-of-way computations to fail
    (resulting in collision) at turnaround lanes
  - fixed bug that caused connections to be considered foes even if
    they did not conflict. The resulting network inconsistency
    manifested in collisions.
- NETGEN
  - Options **--offset.x** and **--offset.y** are no longer ignored
  - Renamed to [netgenerate](../netgenerate.md) due to naming
    conflicts in Debian, was
    [ticket717](http://sourceforge.net/apps/trac/sumo/ticket/717),
    and [Debian-ticket 673931](http://bugs.debian.org/673931)
- TraCI
  - fixed geo conversion
    [ticket804](http://sourceforge.net/apps/trac/sumo/ticket/804)
- TOOLS
  - method parse_fast in sumolib.output no longer fails when using
    attributes which are also python keywords.
  - fixed flowrouter.py to use the new (0.13) network format and
    write the new route format
- Documentation
  - Patched CSS issues in IE

### Enhancements

- building
  - added configure option --enable-profiling to be used with gprof

- Simulation
  - added the option **--routing-algorithm** {{DT_STR}} to configure simulation routing
    (devices,triggers,traci). Allowed values are *dijkstra*,
    (default) and *astar*
  - Rerouting via 'device.rerouting' is now much faster
  - variable speed signs now accept negative speed values. this
    causes them to reset the edge back to its default speed
  - if any vehicles are [vaporized](../Simulation/Vaporizer.md)
    during simulation the attribute `vaporized="NUMBER_OF_VAPORIZED_VEHICLES>`
    is added to the meanData output for the respective edge.
  - added the option **--maxDist.routesize** {{DT_INT}} to limit the size of routeDistributions
    (useful to conserve memory for long-running simulations)
  - added several new output types by Mario Krumnow: [emission output](../Simulation/Output/EmissionOutput.md), [full output](../Simulation/Output/FullOutput.md), [vtk output](../Simulation/Output/VTKOutput.md), [fcd output](../Simulation/Output/FCDOutput.md), and [queue output](../Simulation/Output/QueueOutput.md). Thanks\!
  - added the option **--fcd-output.geo**. If this option is given the output from **--fcd-output** {{DT_FILE}} will
    contain geo coordinates instead of cartesian coordinates.
  - compact specification of routeDistribution (using attribute
    *routes*) now recognizes attribute *probabilities*
  - added the option **--vehroute-output.write-unfinished**. If this option is given, vehicle routes for
    all vehicles on the road will be written at the end of the
    simulation.
  - added the option **--device.rerouting.init-with-loaded-weights**. If this option is given, weights supplied
    with the existing option **--weight-files** {{DT_FILE}} are used for initialiing the rerouting
    devices.
  - [Rerouter](../Simulation/Rerouter.md) definition
    **destProbReroute** now recognizes the special values
    *keepDestination* and *terminateRoute* which causes a reroute to
    the original destination or terminate the route respectively.
  - It is now possible to combine the
    [rerouter](../Simulation/Rerouter.md) definitions
    **closingReroute** and **destProbReroute**. In this case only
    vehicles which are affected by the **closingReroute** sample a
    new destination from the **destProbReroute**.
  - The verbose simulation summary now includes the number of
    teleports
  - The vType attributes **speedFactor** and **speedDev** are now
    officially supported and allow for a convenient way of
    specifying vehicle speed distributions. Among other things these
    can be used to model speeding vehicles which exceed given edge
    speeds. [More Details](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)

- TraCI
  - The deprecated "environment-related" commands were removed and
    the documentation was finally corrected; [position conversion](../TraCI/Simulation_Value_Retrieval.md#command_0x82_position_conversion)
    and [distance computation](../TraCI/Simulation_Value_Retrieval.md#command_0x83_distance_request)
    are still available via the [Simulation Value Retrieval](../TraCI/Simulation_Value_Retrieval.md) API.
    Please note that the footprint has changed so that it matches
    the other API calls.
  - API version was increased to 5
  - Added [object context subscriptions](../TraCI/Object_Context_Subscription.md)
  - Added method *remove* to the vehicle module
  - Added python interface to convert cartesian to geo coordinates
    and vice versa

- sumo-gui
  - the gui can be started with multiple views by supplying a list
    of files to option **--gui-settings-file**
  - street coloring by selection now colors all lanes for selected
    edges (useful when loading selections from a file)
  - Visualizing rerouters with many trigger edges and many closed
    edges no longer causes the GUI to slow down.
  - The lane-parameters dialog now list the street name as well as
    the vehicle class permissions
  - Rail edges are now rendered with rails and crossties making them
    stand out from normal roads.
  - Persons are now visualized during all steps of their plan and
    allow accessing step information.
  - vehicles with guiShape="rail" are now visualized with multiple
    carriages when the visualization option *Show As* is set to
    *simple-shapes*. The overall length of the train and the number
    of carriages is determined from the vehicle length
  - Vehicles can now be renderd with bitmaps using the new vType
    attribute *imgFile*. This should be a grayscale image with alpha
    channel to allow full recoloring functionality.
  - Persons can now be rendered with bitmaps when specifying a vType
    with attribute *imgFile* just like vehicles (persons bitmaps
    will not be rotated, however).
  - POIs can now be rendered with bitmaps using the new attributes
    *imgFile*, *width* and *height*
  - the default vehicle coloring respects now colors assigned to the
    vehicle, type or route (in that order) before assigning the
    default color
  - the default color can be changed for coloring by vehicle / type
    / route
  - The vehicle parameter dialog now includes the vehicle specific
    *speed factor* if speed distributions are used.

- dfrouter
  - the speed column in flow input files is now optional

- TOOLS
  - added options for generating random trips which help to increase
    the quality of generated trips. see
    [Tools/Trip\#randomTrips.py](../Tools/Trip.md#randomtripspy)
  - improved the method parse in sumolib.output. It now supports
    memory-efficient parsing of nested xml elements into convenient
    python objects based on pulldom traversal
  - added several utility classes in sumolib.miscutils
  - added option **--big** to script route/sort_routes.py which allows
    handling very large files at the price of increased hdd-reading
  - added script *assign/duaIterate_analysis.py* for plotting
    statistics of a duaIterate run
  - added script *route/route2poly.py* for visualizing routes with
    polygons in the sumo-gui

- Documentation
  - Added schema definition for [jtrrouter](../jtrrouter.md)'s
    turns and sink definition files

### Other

- general
  - reworked some of the XML parsing code, watch out for new parsing
    errors or ignored attributes
  - added **--xml-validation** to all executables which enables Xerces-C schema
    validation for input files having a schema defined
- netconvert
  - replaced the option **--plain.keep-edge-shape** with the inverted option **--plain.extend-edge-shape**. The new default
    is to keep given edge shapes and only extend them if this option
    is set. Extending edge shapes inadvertently can lead to invalid
    connections and priorities.