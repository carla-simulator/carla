---
title: Z/Changes from Version 0.31.0 to Version 0.32.0
permalink: /Z/Changes_from_Version_0.31.0_to_Version_0.32.0/
---

## Version 0.32.0 (19.12.2017)

### Bugfixes

- Simulation
  - Fixed collisions in the sublane model #3446, #3595, #3613
  - Fixed bug that was causing invalid behavior in the sublane model
    when used with option **--step-method.ballistic**. #3486
  - Fixed bug that was causing deadlocks after undercutting minimum
    gap. #3486
  - Fixed bug that was causing deadlocks at intersections. #3615
  - Option **--ignore-route-errors** now also applies to invalid (i.e. misordered) stop
    definitions. #3441
  - PHEMlight handles large acceleration values better #3390 and has
    updated emission values for new Diesel cars
  - `<stop>` definitions using attribute *until* that are used within a `<flow>` now
    shift the *until* times according to the offset between
    departure and flow *begin*. #1514
  - `<chargingStation>` attribute *chargeDelay* now accepts floating point values.
  - `<chargingStation>` attribute *chargeDelay* now works with subsecond simulation.
  - Vehicles passing a minor link with impatience 0 no longer force
    braking for prioritized vehicles. #3494
  - Fixed bug that was causing collisions between vehicles and
    pedestrians #3527
  - Fixed slow simulation when combining cars and ships in one
    simulation. #3528
  - Fixed collisions on junctions between vehicles coming from the
    same lane. #1006, #3536
  - Fixed failure to change lanes for speed gain in the sublane
    model. #3582
  - Fixed collision of a vehicle with itself. #3584
  - Several fixes in regard to intermodal routing. #3613, #3622, #3560, #3561, #3562

- sumo-gui
  - Fixed crash when simulating pedestrians. #3484
  - Coloring *by selection* is now working for pedestrian crossings. #3396
  - Options **--window-size** and **--window-pos** are now working when set in a configuration file. #3406
  - Vehicle blinkers that signal left or right turns now remain
    switched on while the vehicle is still on the intersection
    (requires networks to be rebuilt). #3478
  - Fixed invalid lane-change blinkers for near-zero lateral
    movements in the sublane model. #3612
  - Fixed invalid vehicle angle when using the sublane model. #3609
  - Networks created with simple projection can now be shown. #3467
  - Fixed invalid *duration factor* in network parameters for
    sub-second simulation. #3600

- polyconvert
  - Fixed handling of XML special characters when exporting
    arbitrary text via option **--all-attributes**. #3447

- netconvert
  - Fixed crash when importing Vissim networks.
  - Fixed bug that was causing invalid signal plans when loading a
    .net.xml file and removing edges from an intersection with
    pedestrian crossings (the link indices for crossings were
    re-assigned but the signal plan was left unmodified creating a
    mismatch).
  - No longer writing pedestrian crossings with length 0 (minimum
    length is 0.1m).
  - Parameters (i.e. those for actuated traffic lights) are no
    longer lost when importing *.net.xml* files or *plain-xml*
    files. #3343
  - Fixed bug that was causing invalid networks to be generated when
    additional lanes were placed to the right of a sidewalk. #3503
  - Fixed bug that was causing invalid networks to be generated when
    nodes without connections were part of a joined traffic light #3715
  - Defining pedestrian crossings for [Pedestrian Scramble](https://en.wikipedia.org/wiki/Pedestrian_scramble) is
    now supported. #3518
  - Custom traffic light plans for pedestrian crossings are no
    longer modified. #3534
  - Fixed invalid traffic light plans at pedestrian crossings for
    node type *traffic_light_right_on_red*. #3535
  - Fixed invalid right of way rules at node type
    *traffic_light_right_on_red* that could cause deadlock. #3538
  - Networks with intersections that are very close to each other
    can now be re-imported. #3585
  - Edges that do not have connections are now correctly represented
    in plain-xml output. #3589
  - Fixed invalid geometry in opendrive-output for lefthand
    networks. #3678
  - Fixed invalid road markings in opendrive-output.

- netedit
  - Fixed bug that was causing pedestrian crossings to remain
    uncontrolled at traffic light controlled intersections. #3472

    !!! caution
        Regression in 0.31.0. As a workaround, networks that were saved with netedit 0.31.0 can be repaired by calling *netconvert -s bugged.net.xml -o fixed.net.xml* or simply reopened and saved with a fresh nightly build of netedit.

  - Options **--window-size** and **--window-pos** are now working when set in a configuration file. #3406
  - Fixed crash when setting linkIndex. #3642

- duarouter
  - Fixed invalid public transport routing if the last vehicle
    departs before the person enters the simulation. #3493

- TraCI
  - Fixed bug in *traci.trafficlights.setLinkState*.
  - Fixed bug in *traci.vehicle.getDrivingDistance* related to
    internal edges. #3553
  - Fixed bug in *traci.vehicle.getDistance* related to looped
    routes. #3648
  - Fixed bug in *traci.simulation.getDistance2D* and
    *traci.simulation.getDistanceRoad* related to internal edges. #3554
  - Command *load* no longer fails when there are too many arguments
    or long file paths. #3599
  - Fixed bug in *traci.vehicle.changeLane* when using the sublane
    model. #3634

- Tools
  - Fixed bug that would trigger an infinite loop in
    [flowrouter.py](../Tools/Detector.md#flowrouterpy).
  - [ptlines2flows.py](../Tutorials/PT_from_OpenStreetMap.md)
    fixes:
    - missing stops no longer result in crashing
    - fixed invalid *until* times when multiple lines use the same
      stop
  - emissionsDrivingCycle now uses the slope values from the correct
    time step when forward calculation of acceleration is enabled
  - [generateTurnDefs.py](../Tools/Misc.md#generateturndefspy)
    now writes interval information. Thanks to Srishti Dhamija for
    the patch. #3712

### Enhancements

- Simulation
  - Added option **--collision.mingap-factor** to control whether collisions are registered when
    the vehicle *minGap* is violated. With the default value of 1.0
    minGap must always be maintained. When setting this to 0 only
    *physical* collisions are registered. #1102
  - Added new [junction model parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters) #3148:
    - *jmIgnoreFoeProb, jmIgnoreFoeSpeed* can be used to configure
      right-of-way violations.
    - *jmSigmaMinor* allows configuring driving imperfection
      (dawdling) while passing a minor link.
    - *jmTimegapMinor* configures the minimum time gap when
      passing a minor link ahead of a prioritized vehicle.
    - *jmDriveAfterRedTime* and *jmDriveRedSpeed* allow
      configuring red-light violations depending on the duration
      of the red phase.
  - Added new
    [laneChangeModel-attribute](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models)
    *lcLookaheadLeft* to configure the asymmetry between strategic
    lookahead when changing to the left or to the right. #3490
  - Added new
    [laneChangeModel-attribute](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models)
    *lcSpeedGainRight* to configure the asymmetry between
    thrhesholds when changing for speed gain to the left or to the
    right. #3497
  - [Electric vehicles](../Models/Electric.md) can now be used
    for emission-model (electricity) output by setting `emissionClass="Energy/unknown"`
  - Tripinfo-output for pedestrians now includes *routeLength,
    duration* and *timeLoss*. #3305
  - [duration-log.statistics](../Simulation/Output.md#aggregated_traffic_measures)-output
    now informs about person rides. #3620
  - Vehicles that end their route with a stop on a parkingArea
    (arrivalPos is within the parkingArea bounds) will be assigned a
    new destination after [rerouting to another parkingArea](../Simulation/Rerouter.md#rerouting_to_an_alternative_parking_area)
    (previously they would drive to the original parkingArea edge
    after finishing their stop). #3647
  - Rerouters now support the attribute `timeThreshold` which makes their
    activation dependent on on a minimum amount of accumulated
    waiting time. #3669
  - Simulation step length has been decoupled from the action step
    length, which is the vehicle's interval of taking decisions.
    This can be configured globally via the option
    '--default.action-step-length', or per vehicle via the parameter
    'actionStepLength'.

- sumo-gui
  - Transparency is now working for all objects.
  - Junction parameters can now be inspected.
  - Upcoming stops are now shown in the vehicle parameter window and
    also in the network when selecting *show current route*. #3679

- netconvert
  - When using option **--numerical-ids** together with option **--output.original-names**, the original IDs of
    all renamed nodes and edges are written to `<param>` elements with key
    *origId*. #3246
  - connections now support the attribute *speed* to set a custom
    (maximum) speed on intersections. #3460
  - connections now support the attribute *shape* to set a custom
    shape. #2906
  - crossings now support the attribute *shape* to set a custom
    shape. #2906
  - The [new element `<walkingArea>`](../Networks/PlainXML.md#walking_areas)
    can now be used in *con.xml* files to define custom walking area
    shapes. #2906
  - Added options **--osm.stop-output.length.bus**, **--osm.stop-output.length.train**, **--osm.stop-output.length.tram** to set appropriate default stop lengths for
    different modes of traffic (in conjunction with option **--ptstop-output**).
  - Added options **--osm.all-attributes** {{DT_BOOL}} which can be used to import additional edge
    parameters such as *bridge*, *tunnel* and *postcode*.
  - Parallel lanes of connecting roads are now written as a single
    road in opendrive-output. #2700

- netedit
  - Additional objects (i.e. detectors) as well as POIs and Polygons
    can now be located based on their ID. #3069
  - Connection and Crossing shapes can now be edited visually. #3464
  - Object types such as edges or polygons can now be locked against
    selection modification. #3471
  - The traffic light index of controlled connections can now be
    edited in *Inspect Mode*. #2627
  - Added button to traffic light mode for cleaning up unused states
    from a traffic light plan.

- duarouter
  - Vehicles and flows which are considered public transport (have
    the line attribute) are now only routed if an additional option **--ptline-routing**
    is given. #2824
  - route alternative output (*.rou.alt.xml*) now contains costs for
    pedestrian stages. #3491

- Tools
  - [osmWebWizard.py](../Tools/Import/OSM.md#osmwebwizardpy)
    can now import public transport (activated by a checkbox on the
    settings tab). If pedestrians are imported as well they may
    elect to use public transport to shorten their walks.
  - added new tool
    [filterDistrics.py](../Tools/District.md#filterdistrictspy)
    to generate district (TAZ) files that are valid for a given
    vehicle class
  - [traceExporter.py](../Tools/TraceExporter.md) can now built
    a direct socket connection to sumo and can filter fcd output for
    regions and times.
  - [flowrouter.py](../Tools/Detector.md#flowrouterpy)
    improvements:
    - route and flow ids now include source and target edge ids
      for better readability. #3434
    - turn-around flow can now be limited using the new option **--max-turn-flow** {{DT_INT}}
  - Added [new tool **tls_csvSignalGroup.py** for importing traffic light definitions from csv input. The input format aims to be similar to the representation used by traffic engineers](../Tools/tls.md#tls_csvsignalgrouppy). Thanks
    to Mirko Barthauer for the contribution.

- TraCI
  - return value of trafficlights.getControlledLinks is now a list
    of lists (of links) for the C++ client as well
  - python client now supports the whole API for
    *vehicle.setAdaptedTraveltime* and *vehicle.setEffort*
    (resetting custom values or setting with default time range) by
    using default arguments.

    !!! note
        The order of parameters had to be changed to allow this. Old code will still work but trigger a warning.

### Other

- The SUMO license changed to the [Eclipse Public License Version 2](https://eclipse.org/legal/epl-v20.html)
- The SUMO build process now supports CMake. It is likely that version
0.32.0 will be the last one shipping Visual Studio solutions. Please
have a look at
[Installing/Windows_CMake](../Installing/Windows_CMake.md) for
information on how to build SUMO on Windows with CMake. There are
also helper scripts in preparation at [{{SUMO}}/tools/build]({{Source}}tools/build) for instance [{{SUMO}}/tools/build/buildMSVS15Project.py]({{Source}}tools/build/buildMSVS15Project.py).

- Simulation
  - **chargingstations-output** now writes times as seconds rather
    than milliseconds.
  - Default value of option **--pedestrian.striping.stripe-width** changed to *0.64* (previously 0.65).
    This allows vehicles with default width to pass a pedestrian on
    a road with default width.
  - preliminary version of [libsumo](../Libsumo.md) is
    available for experimental building of your own apps using SUMO
    as a "library" (calling its functions directly without TraCI)

- sumo-gui
  - default font changed to [Roboto](https://fonts.google.com/specimen/Roboto)
  - Removed OpenGL visualisation option *Antialias*
  - E3-Entry and -Exit detectors are now drawn in darker color to better distinguish them from traffic lights. #3378

- netconvert
  - The element `<customShape>` is no longer supported. Instead `<connection>` and `<crossing>` support the
    *shape* attribute. To set a custom shape for walkingAreas, the
    new element `<walkingArea>` may be used.

- TraCI
  - TraCI version is now 17

- Documentation
  - Documented [simulation object right-click menus](../sumo-gui.md#object_properties_right-click-functions)
  - Described [visualization of edgeData files](../sumo-gui.md#visualizing_edge-related_data)