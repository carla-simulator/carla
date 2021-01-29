---
title: Z/Changes from Version 0.20.0 to Version 0.21.0
permalink: /Z/Changes_from_Version_0.20.0_to_Version_0.21.0/
---

## Version 0.21.0 (11.06.2014)

### Bugfixes

- Simulation
  - Fixed infinite loop when tls-phases were shorter than the
    step-size
  - Fixed bug where vehicles would block the intersection when
    stopping at the very end of a lane due to a red light
  - Fixed lane-changing bug which could cause differing results
    between different compilers due to uninitialized memory.
  - The vehicle class `ignoring` now properly ignores edge permissions and is
    allowed to drive anywhere.
  - Time values are now checked on input whether they are in the
    correct range (less than 2147483 seconds)
  - Teleport over an edge which has a stop now removes the stop from
    the list of pending stops
  - the IDM and the IDMM car following model now stop closer to the
    halting line (in front of intersections), furthermore the moving
    average in the IDMM was repaired
  - Bluetooth device handles subsecond simulation correctly
  - Fixed check for slow lanes ahead of the depart position to allow
    for higher insertion speeds
  - Repaired rerouting of vehicles which are on an internal lane
  - Several fixes with teleporting over, on and onto induction loops
    and range detectors.
  - Fix to the computation of the expected time at which vehicles
    leave an intersection. This results in a minor increase in
    junction throughput.

- sumo-gui
  - The time shown in the gui now matches the time found in
    simulation outputs for the shown vehicle positions. Previously,
    the gui showed the time + 1 step.
  - Fixed crash when drawing persons
  - Patched building under macOS, thanks to Alex Voronov
  - lane coloring by netto occupancy no longer shows brutto
    occupancy (see [lane coloring](../sumo-gui.md#edgelane_visualisation_settings)).
  - drawing of simple vehicle shapes now always uses a distinct
    color for drawing details.
  - Fixed crash when closing the breakpoint editor
  - Fixed crash when loading viewsettings and changing them
    immediately.
- netconvert
  - when loading traffic light programs from *tllogic-files*, the
    node attribute *controlledInner* is now propperly preserved
  - guessed traffic light programs no longer contain duplicate
    consecutive phases
  - Fixed bug where option **ramps.guess** would sometimes fail to add necessary
    lanes.
  - Extremely short internal lanes are no longer split. Instead a
    warning is issued since these cases usually indicate an invalid
    junction geometry.
- dfrouter
  - removed option **--all-end-follower** as it's not longer needed and buggy
- duarouter
  - when using tazs (districts) they are now correctly attached to
    the network with an edge with travel time 0 (before it was 1)
- TraCI
  - subscriptions for VAR_LOADED_VEHICLES_IDS now work
  - when setting lane change mode to prevent all changes due to a
    certain change-motivation, motivations with lower priority are
    now correctly considered.
  - adding a vehicle with a negative depart position gives an error
    message (applies only to ADD not to ADD_FULL)
  - adding a vehicle with a depart time in the past gives an error
    instead of silently dropping the vehicle
  - vaporizing parked vehicles is now possible #1166
  - forcing a lane change for a stopped vehicle now correctly
    removes the current stop from the list of pending stops
- Tools
  - Fixed bug where sortRoutes.py would produce invalid XML (thanks
    to Pieter Loof for the patch)
  - Fixed bug in traceExporter.py when generating files for NS2
    which caused non-sequential IDs to be written.
- General
  - Fixed several divisions by zero and integer over- and underflows
    reported by the clang sanitizer
  - Trying to open existing files which do not have reading
    permissions now gives a correct error message
  - All tools now give error messages when an output socket is not
    any longer available on closing

### Enhancements

- Simulation
  - Added improved capabilities for [pedestrian simulation](../Simulation/Pedestrians.md)
    - Added option **--pedestrian.model** {{DT_STR}} with available values **nonInteracting** and
      **striping**. The most important features of the
      **striping** model (which is the new default are:
    - Pedestrian cross streets and interact with traffic (blocking
      vehicles, blocked by vehicles, reacting to traffic lights)
    - Pedestrians interact with each other (jam)
    - Pedestrians select their route dynamically to avoid red
      lights when multiple paths across an intersection are
      available.
    - Added options **--pedestrian.striping.stripe-width** {{DT_FLOAT}} and **--pedestrian.striping.dawdling** {{DT_FLOAT}} to configure the 'striping'-model

  - Added option **--lanechange.overtake-right** {{DT_BOOL}} to switch between german overtaking laws (default)
    where overtaking on the right on highways is prohibited and
    american overtaking laws
  - Vehicles which are already on a junction but have to wait for
    another vehicles (also on the junction) now always drive up to
    point where their path crosses with the blocking vehicle.
  - Added option **--random-depart-offset** {{DT_TIME}} to randomize departure times for all vehicles by
    applying a positive (uniform) random offset.
  - `flow` elements now support attribute `probability` for specifying flows with
    randomly distributed departures (following a binomial
    distribution). Each second a vehicle is inserted with the given
    probability (which is scaled according to the given **--step-length**).
  - Vehicles with a `vType` which has a specified `vClass` will now get appropriate
    default values for some of their parameters (i.e. busses will be
    longer than 5m by default). Refer to
    [Vehicle_Type_Parameter_Defaults](../Vehicle_Type_Parameter_Defaults.md)
    for details.
  - the option **--scale** {{DT_FLOAT}} can now also handle values \> 1 and inserts more
    vehicles in this case
  - added [HBEFA3](../Models/Emissions/HBEFA3-based.md)
    emission model
  - added [outputs](../Simulation/Output.md) conforming to the
    Amitran standards

- sumo-gui
  - the option **--no-warnings** {{DT_BOOL}} now also applies to the GUI message window
  - Added person coloring scheme *by selection* and *by angle*
  - All person modes now give a waitingTime
  - Person parameter dialog now includes attributes 'edge',
    'position', 'angle' and 'waitingTime'
  - Pedestrian crossings are now indicated by the typical
    zebra-pattern (light color indicates that pedestrians have
    priority, dark color means that vehicles have priority)
  - Showing permission code in lane parameter dialog, updated style
    color by permissions
  - Uncontrolled links (not part of a junction logic) are now shown
    with index -1
  - Now showing edge type in lane parameter dialog
  - Customizable drawing of ids for edges of type crossings and
    walkingarea
  - Added lane coloring scheme *by angle*
  - Lanes are now drawn with smooth shapes regardless of geometry
    (fixed white gaps at corners)

- netconvert
  - Added element `<crossing>` for '.con.xml' files. [This can be used to specify pedestrian crossings](../Networks/PlainXML.md#pedestrian_crossings)
  - Added attribute `sidewalkWidth` to `<type>` elements in [*.typ.xml* files](../SUMO_edge_type_file.md). If this is given, edges
    of this type will get an extra lane with `allow="pedestrian"` and the specified width.
  - Added options **--sidewalks.guess** {{DT_BOOL}}, **--sidewalks.guess.min-speed** {{DT_FLOAT}} and **--sidewalks.guess.max-speed** {{DT_FLOAT}} to trigger guessing of sidewalks
  - Added option **--crossings.guess** {{DT_BOOL}} to trigger guessing of pedestrian crossings
  - Added options **--tls.guess-signals** {{DT_BOOL}} and **--tls.guess-signals.dist** {{DT_FLOAT}} to interpret special traffic light nodes
    which surround an intersection as the signal positions for that
    intersection. This heuristic is useful for correctly importing a
    typical representation of traffic lights in OSM data.
  - Added attribute `shape` to `<node>` elements in [*.nod.xml* files](../Networks/PlainXML.md#node_descriptions).
    This allows to specify a custom shape for that node.
  - can write networks in the Amitran format

- duarouter
  - `<trip>` and `<flow>` elements now support attribute `via` for specifying intermediate
    edges to use during routing.
  - `vType` and `route` definitions can now be given in [*additional-files*](../sumo.md#format_of_additional_files) just like for
    [sumo](../sumo.md)
  - Pedestrian walks with attributes `from` and `to` are now routed and
    written with attribute `edges`.
  - the maximum (the average) speed factor are taken into account
    when calculating the minimum (the default) travel times
  - References to an unknown `vType` are now an error (which can be ignored
    with **--ignore-errors**). Previously, an unknown `vType` was silently copied to the
    output and treated like the default type during routing.

- polyconvert
  - OSM road shapes and traffic light positions can now be exported
    [by setting the appropriate entries in the typemap file.](../Networks/Import/OpenStreetMap.md#importing_additional_polygons_buildings_water_etc)

- od2trips
  - reads now OD matrices in Amitran format

- Tools
  - Added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **--pedestrians** {{DT_BOOL}} to generate pedestrian walks
  - Added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **--max-distance** {{DT_FLOAT}} to limit the straight-line distance of generated trips
  - Added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **-a, --additional-files** {{DT_FILE}}[,{{DT_FILE}}\]\* which is passed to [duarouter](../duarouter.md)
  - Added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **-i, --intermediate** {{DT_INT}} for generating trips with intermediate waypoints
  - Added [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option **--maxtries** {{DT_INT}} to control the number of attemps for finding a trip
    which meet the given distance constraints
  - convert almost arbitrary XML outputs (with a given schema) to
    CSV and protobuf, see [Tools/Xml](../Tools/Xml.md)

### Other

- Simulation
  - Removed obsoleted **--incremental-dua-step** and **--incremental-dua-base** option (use **--scale** instead)

- Tools
  - Renamed [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option for setting the id prefix of generated trips from **-t, --trip-id-prefix** to **--prefix**
  - Renamed [randomTrips.py](../Tools/Trip.md#randomtripspy)
    option for setting trip attributes from **-a, --trip-parameters** to **-t, --trip-attributes**

- Documentation
  - An explanation for vehicle insertion can be found
    [here](../sumo.md#insertion_of_vehicles).
  - [Default parameters for different vehicle classes](../Vehicle_Type_Parameter_Defaults.md).
  - [Documentation for Calibrators](../Simulation/Calibrator.md). These simulation
    objects allow for dynamic adaption to traffic flow and speed at
    selected edges.
  - The obsolete Media Wiki, Trac and Wordpress instances at the
    SourceForge Hosted apps facility have been removed altogether

- Miscellaneous
  - Some
    [`vClass` definitions](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#abstract_vehicle_class)
    where added and others renamed. Old definitions will continue to
    work but some will cause deprecation warnings.
  - Referenze [typemap files](../SUMO_edge_type_file.md) for
    importing edges and polygon data from OSM networks as well as
    from other formats can now be found at {{SUMO}}/data
  - SUMO builds now with [clang](http://clang.llvm.org/) as well
  - The shortcut **-p** for the option **--print-options** was removed because of possible
    name clashes