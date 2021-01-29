---
title: Z/Changes from Version 0.22.0 to Version 0.23.0
permalink: /Z/Changes_from_Version_0.22.0_to_Version_0.23.0/
---

## Version 0.23.0 (31.03.2015)

### Bugfixes

- Simulation
  - fixed bugs that caused multiple pedestrians to enter the same
    spot when using the *striping* model. #1506
  - fixed crash when vehicles with `vClass="ignoring"` attempt to drive across
    sidewalks and walkingareas.
  - fixed invalid `departDelay` in **--tripinfo-output** when using flows with attribute `probablity`. #1482
  - fixed rerouter interval ends which are now exclusive like all
    other intervals
  - fixed bug that caused invalid exit times when vehicles were
    teleported while on an intersection. #1520
  - fixed invalid estimation of the time and speed when reaching an
    intersection which could cause collisions. #1531 #1533
  - explicitly setting option **--weight-attribute** *traveltime* now properly fills the
    edge-weight container (i.e. for visualization). #1541
  - fixed invalid `routeLength` in **--tripinfo-output** when vehicles did not finish their route. #1539
  - fixed invalid `routeLength` in **--tripinfo-output** when using networks with internal lanes. #443
  - fixed bugs that were causing collisions. #1549, #1551, #1553, #1575, #1591
  - fixed bug that was causing vehicles to brake prematurely when
    turning left on a prioritized road. #1566
  - fixed crash when pedestrians were using the same edge twice in a
    row. #1555
  - fixed crash when pedestrians were given a disconnected route. #1578.
    Now disconnected routes are an error which is recoverable (by
    *teleporting*) using option **--ignore-route-errors**
  - pedestrians with a disconnected trip now properly reach their
    destination (if the option **--ignore-route-errors** is given). #1581
  - corrected reading the PHEMLIGHT_PATH environment variable
  - simulation now properly terminates when loading a `<trip>` with a `from`-edge
    that is shorter than the vehicle. #1559
  - vehicles no longer collide (visually) within an intersection
    when using the
    [`endOffset` attribute](../Networks/PlainXML.md#edge_descriptions). #970
  - fixed crash when attempting to load an additional tls program
    with unused phase states. #1641
  - fixed crash when specifying duplicate person ids. #1650

- netconvert
  - when adding sidewalks via
    [type-map](../SUMO_edge_type_file.md) and a sidewalk
    already exists, the edge now remains unchanged.
  - fixed bug where some generated pedestrian crossings at
    T-intersections had a permanent red light. #1363
  - fixed bug where right-turning vehicles had the green light
    during the left-turn phase when pedestrian crossings where
    present.
  - the width of adjacent sidewalks is now taken into account when
    computing the width of a walkingarea.
  - fixed bug where an invalid network was generated when declaring
    a pedestrian crossing at a dead-end node. #1366
  - fixed bug in computation of *walkingarea* shape. #1257
  - fixed bug in connection computation when some connections were
    explicitly deleted (also applied to some OSM turn-restrictions). #1457
  - changing offset or algorithm type with *.tll.xml* file for TLS
    loaded from a *.net.xml* file without changing the phases now
    works. #1207
  - when loading `<split>` elements along with a *.net.xml* file, existing
    connections are preserved whenever possible. #1353
  - Fixed 3D-geometry of internal lanes at elevated junctions. #1526
  - Fixed invalid connections when generating ramps. #1529
  - Fixed invalid geometry when generating ramps. #1535
  - fixed bug where user defined connections were ignored at
    intersections with one incoming and one outgoing edge. #1552
  - fixed crash when loading a sumo net with pedestrian crossings
    and removing edges via options. #1557
  - fixed errors when building pedestrian crossings. #1582
  - fixed invalid connections at edges with sidewalks. #1586
  - fixed positioning of internal junctions at turnarounds when
    using the
    [`endOffset` attribute](../Networks/PlainXML.md#edge_descriptions). #1589
  - when importing from OSM, compound types now honor the setting `discard="true"` of the individual component types.
  - fixed junction shape when using lanes with custom width. #1604
  - user-defined node shapes are now correctly imported from
    *.net.xml* files. #1450
  - lane-specific settings are no longer lost when applying an
    *.edg.xml* patch file to an existing network definition. #1609
  - fixed some of the bugs that were causing inappropriate node
    shapes.
  - several fixes to the placement of internal junctions (waiting
    positions on the intersection for selected connections)
    - removed some internal junctions that were not necessary
    - repositioned internal junctions so that the waiting vehicles
      do not overlap with foe vehicles #1611
    - moved internal junctions at TLS-controlled nodes further
      towards the junction center (previously they respected foe
      vehicles which would never get the green light at the same
      time)
  - fixed invalid junction logic (right-of-way rules) when incoming
    edges had uncommon angles. #1632 #1462
  - generate tls logic now matches the underlying edge priorities.
    The streams that would have priority if the node type was
    "priority" now always have a common green phase. #1642
  - the attribute `controlledInner` is now correctly exported to *plain-xml* output
    when importing a *.net.xml* where edges within a joined TLS are
    controlled. #1264
  - fixed bug that was causing the wrong flow to receive the right
    of way at priority junctions. #1689
- [MESO](../Simulation/Meso.md)
  - fixed crash when updating vehicle routes while running with **--meso-junction-control**.
    see #1502
  - fixed angle and position of vehicle ids in the GUI
  - fixed crashes when using various outputs. #1505
  - fixed bug leading to exaggerated emissions. #1504
  - vehicles are now drawn correctly on curving lanes. #758
  - vehicle size exaggeration now working. #1406
- sumo-gui
  - Fixed crash when using a vehicle parameter window while the
    vehicle disappears. #1534
  - Coloring by loaded edge weight now correctly reflects changes
    over time.
  - Edges/Lanes set to transparent (alpha = 0) are no longer drawn
    when zoomed out.
  - The view-settings dialog can no longer become hidden when
    switching between displays with a different resolution. #1512
  - Traffic-light *show phase* dialog no longer crashes when loading
    very long phases. #1451
  - fixed visualization of closed edges in rerouters
  - Lane coloring *by inclination* now correctly shows the
    direction. #1526
  - The checkbox Streets-\>'show rails' in the gui-settings dialog
    can now be used to toggle the visualization style of railways.
  - The edge width exaggeration factor is now properly saved to and
    loaded from a
    [gui-settings-file](../sumo-gui.md#configuration_files).
  - fixed gui glitch when zooming with transparent lanes in view.
  - now drawing vehicles on transparent lanes.
  - Selecting the *Center* action from a popup-menu immediately
    centers the view on that object (instead of delaying until the
    next window update). #1593
  - when an error occurs during the initial route loading, reloading
    the simulation will now work after the user corrects the error
    in the input files. #1508
- duarouter
  - no longer building invalid routes when the connectivity among
    edges differs between vClasses. #303
  - fixed bug that caused unreasonable detours in pedestrian routes
    (also affected routing in the simulation). #1556
- all routers
  - Unknown vehicle type in `<flow>` element can now be ignored with option **--ignore-errors**.
- Tools
  - [cutRoutes.py](../Tools/Routes.md#cutroutespy) now sorts
    unmodified departure times correctly.
  - [server.py](../Tools/Import/OSM.md#serverpy) no longer
    creates disconnected routes. Also, trains will no longer be
    generated on inappropriate edges.
  - [sort_routes.py](../Tools/Routes.md#sort_routespy) now
    sorts correctly when using option **--big**. #1603

### Enhancements

- Simulation
  - Pedestrians now have [appropriate default values for length, minGap and width](../Vehicle_Type_Parameter_Defaults.md).
    These values can be changed by using a `vType`.
  - Improved realism of pedestrian dynamics on walkingareas when
    using model *striping*. [Pedestrians at arbitrary angles now properly avoid each other](../Simulation/Pedestrians.md#model_striping).
  - Added option **--pedestrian.striping.jamtime** {{DT_TIME}} to control behavior of jammed pedestrians. By
    default, pedestrians will now start squeezing through the jam
    after being stopped for that time. As a consequence, pedestrian
    scenarios now always terminate.
  - Added option **--output-prefix** {{DT_FILE}} to automatically add a prefix to all output files.
    The special string *TIME* is replaced with the current time.
  - Rerouters can now set permitted vehicle classes on closed lanes,
    see #1518
  - A warning is now issued when a loaded network contains *unsafe*
    connections. #447
  - A new car-following class based on the original Krauß was added
    that that takes into account the maximum acceleration ability
    computed by PHEMlight
  - Pedestrian routing now also [works on networks without pedestrian infrastructure (walkingareas)](../Simulation/Pedestrians.md#building_a_network_for_pedestrian_simulation). #1564
  - Added option **--netstate-dump.precision** {{DT_INT}} to control the precision (number of digits after
    the dot) of vehicle positions and speeds when using option **--netstate-dump**.
  - Added vehicle class `ship`.
  - option **--ignore-route-errors** now also handles errors which prevent the vehicle from
    being inserted into the network. #1661
  - Added option **--device.rerouting.output** {{DT_FILE}} to export the travel times which are used for
    dynamic rerouting during every adaptation interval. #1663

- sumo-gui
  - Can now color and scale lanes *by average speed* and by *by
    average relative speed* (relative to the speed limit).
  - Added option for drawing the path across the current walkingarea
    to the person popup menu.
  - Can now color pedestrians randomly.
  - When drawing persons as triangles, the direction they are facing
    is now indicated (it was ambiguous for equilateral triangles).
  - The lane menu now shows the height under the cursor as well as
    the offset (*pos*).
  - Can now color lanes *by height at segment start* and by *by
    inclination at segment start*. This allows investigating the
    3D-geometry in detail.
  - [Added `param`](../Simulation/Traffic_Lights.md#additional_parameters)
    *show-detectors* that allows showing detectors of *actuated*
    traffic lights in the GUI.
  - When drawing streets with exaggerated width, the edge in one
    direction no longer overlaps with the opposite direction. #1540
  - Now setting window title when loading config or net at
    application start.
  - Added menu for locating persons. #1276
  - The `line` information for a vehicle is now shown below the vehicle id
    and in the parameter dialog. #1563
  - The network parameter dialog now includes some statistics on the
    number of nodes and edges as well as the total road length. #1367
  - Added guiShape `ship`.
  - Now drawing waterways (lanes with `allow="ship"`) in a distinct style.
  - Added menu option for loading an [*additional-file*](../sumo.md#format_of_additional_files) with
    [shapes](../Simulation/Shapes.md).
  - Improved gui shapes for two-wheeled vehicles.
  - Markings for bike lanes are now drawn at road intersections
    (bike lanes are lanes which only allow vClass *bicycle*).
  - Added options for saving the current viewport and simulation
    delay along with the current visualisation settings. #1625

- netconvert
  - read heightmaps from GeoTIFFs for instance [SRTM data](http://opendem.info/download_srtm.html) #1495
  - when adding sidewalks via
    [typemap](../SUMO_edge_type_file.md) or **--sidewalks.guess**, permissions on
    all other lanes are now automatically set to disallow
    pedestrians. #1507
  - sidewalks can now be added for specific edges by adding the
    attribute `sidewalkWidth="<width_in_m>"` to `<edge>`-elements in *.edg.xml*-files. This automatically
    remaps any existing connections. #1580
  - when using pedestrian crossings and green verges (lanes which
    disallow all vClasses), the crossing now starts at the side of
    the ride instead of the green verge. #1583
  - Added option **--sidewalks.guess.from-permissions** {{DT_BOOL}}. This adds sidewalks for edges that allow
    pedestrians on any of their lanes regardless of speed. #1588
  - Can now import waterways from OSM ([needs a custom type-map]({{Source}}data/typemap/osmNetconvertShips.typ.xml)).
  - The junction size can now be customized [using the new `<node>`-attribute `radius`](../Networks/PlainXML.md#node_descriptions) #1605
  - Added option **--junctions.corner-detail** {{DT_INT}} which can be used to generate rounded intersection
    corners. #1606
  - user-defined shapes for internal lanes, crossings and
    walkingareas are now supported via [the new `customShape`-element in *.con.xml* files](../Networks/PlainXML.md#custom_shapes_for_internal_lanes_crossings_and_walkingareas) #1607
  - right-turning road vehicles will now wait within the
    intersection when yielding to straight-going bicycles. #1255

- netgenerate
  - can now generate networks with unregulated intersections. #1558

- polyconvert
  - added option **--output.plain-geo** {{DT_BOOL}} for writing POIs and polygons with
    geo-coordinates. #1608

- netedit
  - can now visualize railways the same way as sumo-gui. #1542
  - can now disable drawing of junction shapes via gui-settings.
  - can now load all types of shapes from the menu.
  - can now undo/redo bulk selection/deselection of objects. #1614
  - added option for auto-selecting an edges nodes.
  - can now modify the nodes of an edge via the attribute dialog. #1662
  - the attribute panel is now updated on undo/redo/recompute. #696
  - when creating a reverse edge with spread type *center*, the new
    edge is shifted sideways and given new endpoints.
  - can now move the view in 'Create Edge'-mode by holding <CTRL\>

- Tools
  - [sort_routes.py](../Tools/Routes.md#sort_routespy) now
    supports sorting persons and also copies additional elements
    (such as `vType` to the output).
  - [cutRoutes.py](../Tools/Routes.md#cutroutespy) can now
    handle nested stops in a vehicle definition.
  - added [routeStats.py](../Tools/Routes.md#routestatspy) for
    computing statistics on route length.
  - added [traceExporter.py](../Tools/TraceExporter.md) option **--orig-ids** {{DT_BOOL}}
    for keeping the original vehicle IDs instead of a running index.
    For some output formats such as *NS2* this only works if the
    vehicle IDs are numeric to begin with.
  - [route2trips.py](../Tools/Trip.md#route2tripspy) now
    preserves `vType` elements.
  - [server.py](../Tools/Import/OSM.md#serverpy) now starts
    with 'real world' gui settings and initial delay of 20. Also,
    the trip generation settings have been tweaked for realism (i.e.
    shorter pedestrian trips, longer train routes)

- TraCI
  - Several simulation entities support [named string parameters](../Simulation/GenericParameters.md) in their
    XML-definitions. These can now be set and retrieved [via TraCI](../TraCI/GenericParameters.md) and via the [python library](../TraCI/Interfacing_TraCI_from_Python.md) for the
    following entities: Edge, Lane, Person, Vehicle, VehicleType,
    PoI, Polygon, Route, TrafficLight
  - Added [person API methods](../TraCI/Person_Value_Retrieval.md) to sumo and to
    the [python library](../TraCI/Interfacing_TraCI_from_Python.md).

### Other

- Documentation
  - Added overview on [Randomness in the Simulation](../Simulation/Randomness.md)
  - Added overview on [Vehicle speeds](../Simulation/VehicleSpeed.md)
  - The feature for passing arbitrary sumo options to
    [duaIterate.py](../Demand/Dynamic_User_Assignment.md) is
    now mentioned when calling **--help**.
  - The attribute `endOffset` which can be used to move the stop line when
    [defining networks](../Networks/PlainXML.md#edge_descriptions)
    is now documented. It has been available since version 0.13.0.
  - Documented recommended options when [importing OSM networks](../Networks/Import/OpenStreetMap.md)
  - Documented TraCI retrieval of *waiting time* for vehicles,
    persons, edges and lanes
- TraCI
  - consolidated the message IDs for ArealDetectors
  - TraCI version is now 9
- Miscellaneous
  - netconvert option **--map-output** was removed since it was neither working nor
    deemed useful enough to repair it.
  - added warning about invalid geo-coordinates in netconvert and
    polyconvert input