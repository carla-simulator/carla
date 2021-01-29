---
title: Z/Changes from Version 0.16.0 to Version 0.17.0
permalink: /Z/Changes_from_Version_0.16.0_to_Version_0.17.0/
---

## Version 0.17.0 (03.05.2013)

### Bugfixes

- Simulation
  - tripinfos now contain the correct arrivalPos and routeLength
    values
    ([\#814](https://sourceforge.net/apps/trac/sumo/ticket/814))
  - fixed collision bug for simulations that were running with
    option **--no-internal-links**
  - fixed bug that caused vehicles to sometimes use the wrong lane
    and occasionally lead to collisions
    ([\#586](https://sourceforge.net/apps/trac/sumo/ticket/586),
    [\#836](https://sourceforge.net/apps/trac/sumo/ticket/836))
  - fixed bug in the default car-following model which lead to
    collisions when using vehicle types with different deceleration
    values
  - fixed crash when specifying an invalid car-following model in
    the input files
  - fixed invalid vehicle insertions under special conditions
  - increased junction throughput (fixed bugs related to
    right-of-way timing computation)
  - fixed bug where vehicles drove faster than allowed at arrival
    and when changing from fast to slow lanes
  - simulation routing with taz now works in the presence of vClass
    restrictions.
  - fixed bug where vehicles sometimes did not follow the
    **arrivalLane** attribute.
  - fixed bug where entryExitDetectors issued invalid warnings about
    vehicles leaving without entering
  - fixed bug where inductionLoops and instantInductionLoops
    sometimes failed to detect vehicles
- sumo-gui
  - fixed occasional crash when additional vehicle visualizations
    are active
  - fixed bug where the simulation control buttons (start,step)
    sometimes remained gray when tracking a vehicle
- netconvert
  - fixed bug that caused right-of-way computations to fail
    (resulting in collision) at left-turning connections
  - fixed bug where networks imported from
    [OpenStreetMap](../Networks/Import/OpenStreetMap.md) had
    incorrect lane numbers when forward and backward directions
    differed in lane count.
  - fixed bug where some sets of junctions where erroneously
    classified as roundabouts when using option **--roundabouts.guess** resulting in
    invalid junction logics and causing collisions during simulation
  - fixed bug where inconsistent (unloadable) networks where created
    using option **--ramps.guess**
  - deleted nodes are no longer included in the final network
    boundary
- polyconvert
  - Removed dependency on key/value ordering when importing from
    OSM. Instead, now elements which match multiple typemap entries
    are exported multiple times.
  - fixed projection errors when importing polygons for very large
    networks.
- ROUTER
  - Fixed bug where output files contained wrong values for the
    emissionClass attribute (i.e routers did not properly copy the
    the value from input to output files). This sometimes caused
    duaIterate.py to fail.
  - major refactoring, now handling route, trip and flow inputs with
    one handler (can be mixed in one file)
  - persons and stops in route input files no longer disappear
  - flow handling is similar to simulation (ids generated with "."
    separator)
- TraCI
  - Fixed handling of the parameter to the simulationStep command
    (CMD_SIMSTEP2). Formerly a single simulation step was performed
    if the given target time was lower than the current simulation
    step. Now the simulationStep command is ignored in this case.
    The propper use case for this command is to run the simulation
    up to the given target time. Some scripts (including tutorial
    scripts) wrongly passed the step duration as argument and only
    worked because of this bug. To make these scripts work it is
    sufficient to omit the parameter altogether.

### Enhancements

- Simulation
  - added support for traffic-actuated traffic lights (see
    [Simulation/Traffic_Lights\#Loading_new_TLS-Programs](../Simulation/Traffic_Lights.md#loading_new_program),
    [Networks/PlainXML\#Node_Descriptions](../Networks/PlainXML.md#node_descriptions))
  - added support for microscopic calibrators. These trigger-type
    objects can be used to increase or reduce traffic volume and to
    regulate speeds at selected edges. Using calibrators it becomes
    possible to calibrate a simulation according to known traffic
    measurements (i.e. from real-life induction loops) without using
    TraCI. For usage examples see the test suite at
    tests/sumo/extended/calibrator.
  - renamed XML root elements of the netstate (raw dump) and the
    mean data outputs.
  - added a car following model which respects slope (kraussPS),
    your network needs z-coordinates then
- sumo-gui
  - added optional POI attributes 'lon' and 'lat' for specifying
    position in geo-referenced networks.
  - added options for showing polygon names and changing their line
    width
  - gui-setting files now support the element `<breakpoints-file value="mybreakpoints.txt"/>` for loading
    breakpoints at the start of the simulation
  - new color representation as four unsigned bytes including an
    alpha channel (transparency) all in the range of 0-255
  - human readable color representations for red, green, blue, cyan,
    magenta, yellow, black, white, and grey
  - junction coloring
- netconvert
  - added support for traffic-actuated traffic lights (see
    [Networks/PlainXML\#Node_Descriptions](../Networks/PlainXML.md#node_descriptions))
  - in plain connection files the element **delete** now also works
    when the connection doesn't exist yet (i.e. when building a
    network from plain nodes and edges and the connection file only
    contains delete elements).
  - now importing most turning restrictions from
    [OpenStreetMap](../Networks/Import/OpenStreetMap.md)
  - when importing SUMO networks without internal link the output
    network will be built without internal links by default.
  - now warning about networks with large absolute coordinates since
    these will cause visual errors in guisim
  - added option **--tls.discard-simple** which discards traffic lights at geometry-like
    junctions (no intersecting streams) loaded from formats other
    than plain-XML. This is particularly usefull when importing
    networks from OpenStreetMap which contain many pedestrian
    actuated traffic-lights.
- Tools
  - added tool for creating smaller scenarios from a big scenario
    [Tools/Routes\#cutRoutes.py](../Tools/Routes.md#cutroutespy)
  - replaced the old trace exporter by a new
    [Tools/TraceExporter](../Tools/TraceExporter.md) tool
  - experimental Python 3 support for TraCI python API and
    duaIterate.py
- Tutorials
  - added a [tutorial on trace generation](../Tutorials/Trace_File_Generation.md)
- Documentation
  - added and improved several XML schemata (thanks to Matthew
    Fullerton)