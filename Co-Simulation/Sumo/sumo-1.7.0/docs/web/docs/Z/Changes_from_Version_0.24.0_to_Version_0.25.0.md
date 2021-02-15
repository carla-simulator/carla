---
title: Z/Changes from Version 0.24.0 to Version 0.25.0
permalink: /Z/Changes_from_Version_0.24.0_to_Version_0.25.0/
---

## Version 0.25.0 (07.12.2015)

### Bugfixes

- Simulation
  - Fixed crashing and deadlocks when performing [routing in the simulation based](../Demand/Automatic_Routing.md) on
    [districts](../Demand/Importing_O/D_Matrices.md#describing_the_taz). #1938
  - Fixed bug that was hindering lane-changes due to invalid
    cooperative speed adaptations. #1971
  - Fixed bug that was causing erratic lane changes when using
    subsecond simulation. #1440
  - Fixed bug that was causing erratic lane changes in front of
    intersections. #1856
  - Fixed right-of-way in regard to vehicles that were driving
    across the same intersection twice. #2023
  - Vehicles waiting to enter a roundabout no longer yield to other
    vehicles outside the roundabout. #1847
  - Pedestrians no longer walk past their specified arrival
    position. #1780
  - Fixed asymmetrical pedestrian behavior when walking to a
    *busStop*. Now they always walk to the middle of the busstop
    rather than to its *endPos*. #1562

- sumo-gui
  - Fixing crash when selecting *Show all routes* from the vehicle
    menu.
  - When loading a gui-settings-file from the *View Settings*
    dialog, the delay value is now correctly applied.
  - Fix bug that caused giant circles to appear when exaggerating
    the width of lanes with short geometry segments.
  - Vehicle names are now drawn for vehicles that occupy multiple
    edges. #1960
  - Fixed drawing of link indices, link rules, lane markings and bus
    stops for left-hand networks.
  - Fixed error when reloading a network with `<param>`-elements. #1979

- netconvert
  - Fixed missing connections in multi-modal networks. #1865
  - Fixed bug that caused invalid pedestrian crossings to be
    generated after importing a *.net.xml* file. #1907
  - Fixed bug that caused pedestrian crossings to change their
    priority when importing a *.net.xml* file. #1905
  - Fixed geometry bug when building pedestrian crossings and
    walkingareas for left-hand networks.
  - Fixed invalid network after deleting edges at a joined traffic
    light with controlled pedestrian crossing. #1902
  - Motorway ramps are no longer guessed if the lane permissions
    indicate that the edge is not a motorway. #1894
  - Motorway ramps are no longer guessed at roundabouts. #1890
  - Fixed some cases when roads where invalidly guessed to be
    roundabouts. #1933
  - When importing a *.net.xml* file the resulting network is no
    longer modified due to automatic joining of edges that connect
    the same nodes. #1930
  - When importing a *.net.xml* file with pedestrian crossings and
    setting option **--no-internal-links**, the crossings and walkingareas are removed from
    the resulting network. #1730
  - Several fixes in regard to OpenDrive networks:
    - Fixed missing `<laneLink>` elements when exporting networks as
      [OpenDrive](../Networks/Import/OpenDRIVE.md). #1973
    - Now successfully importing [OpenDrive networks](../Networks/Import/OpenDRIVE.md) with
      dead-end edges. #1692
    - Fixed imprecise geometry when importing
      [OpenDrive](../Networks/Import/OpenDRIVE.md) networks.
    - Fixed imprecise geometry when exporting networks as
      [OpenDrive](../Networks/Import/OpenDRIVE.md). #2031
    - Fixed invalid geometry of lanes within intersections when
      exporting networks as
      [OpenDrive](../Networks/Import/OpenDRIVE.md).
  - Fixed crash when specifying inconsistent [tllogic input](../Networks/PlainXML.md#traffic_light_program_definition). #2010
  - When patching an existing network with *.nod.xml* file, existing
    traffic light programs are now preserved unless changes are
    specified explicitly.
  - No longer patching loaded traffic light programs for new
    crossings if they already have the correct state size.
  - When importing a *.net.xml* which was built for left-hand
    traffic, the resulting network will also be built for left-hand
    traffic. #1880
  - When importing a *.net.xml*, generated networks will retain the
    same value of **--junctions.corner-detail**) as the input network. #1897
  - Fixed invalid geo-referencing in left-hand networks. #2020
  - Traffic lights that control multiple intersections no longer
    create unsafe right-of-way rules. The edges that lie within the
    traffic light are now controlled according to the appropriate
    right of way (This does not necessarily model physical traffic
    signals but reflects the behavior of drivers). #812
    - Old signal plans for these *joined* traffic lights no longer
      work for new networks since more link states need to be
      defined. The option **--tls.uncontrolled-within** {{DT_BOOL}} was added to build networks that are
      compatible with old-style signal plans. Note, that this may
      create unsafe intersections, causing collisions.

- netedit
  - When renaming an edge, the lane IDs are now updated as well. #1867
  - Fractional widths can now be set when inspecting edges.
  - Modifying traffic light plans which control multiple nodes is
    now working. #2009

- duarouter
  - Fixed invalid error when compiled without the FOX library. #1956

- od2trips
  - Option **--begin** is now working. #1889

- MESO-GUI
  - Loading of edge-scaling schemes from a *gui-settings-file* is
    now working.
  - The front of each vehicle queue is now drawn at the start of its
    segment.

- TraCI
  - Fixed bug that prevented the [C++ TraCI library](../TraCI/C++TraCIAPI.md) from functioning. #2007
  - [Vehicle command](../TraCI/Change_Vehicle_State.md) *set speed* can now be used in conjunction with *move to VTD*. #1984
  - When using [Vehicle command](../TraCI/Change_Vehicle_State.md) *move to VTD*,
    the speed is set according to the covered distance where this is
    deemed plausible (the value of *set speed* overrides this).

- Tools
  - [osmWebWizard.py](../Tools/Import/OSM.md) no longer fails
    when encountering path names with space-characters in them.
  - [osmWebWizard.py](../Tools/Import/OSM.md) no longer nests
    output directories when generating multiple scenarios.

### Enhancements

- Simulation
  - [Vehicle types](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#abstract_vehicle_class)
    now have `vClass="passenger"` by default. Earlier, the default was `ignoring` which would let
    the vehicles drive along footpaths and railways.
  - [Zipper merging](https://en.wikipedia.org/wiki/Merge_%28traffic%29) is
    now supported (see netconvert enhancement below). #1292
  - Added person statistics to [verbose output](../Simulation/Output.md#commandline_output_verbose). #1898
  - Now warning about jammed pedestrians.
  - Now warning about pedestrians "collisions".
  - Now warning about traffic lights where one link never gets a green light.
  - It is now possible to [modify the offset of an existing tls program](../Simulation/Traffic_Lights.md#modifying_existing_tls-programs)
    without loading a completely new `tlLogic`-definition. #1940
  - Added option **--duration-log.statistics** {{DT_BOOL}} which causes average vehicle trip data to be
    [printed in verbose mode](../Simulation/Output.md#commandline_output_verbose)
    (average route length, travel time and time loss, ...) for quick
    evaluation of a scenario. #1911
  - The option **--max-num-vehicles** {{DT_INT}} can now be used to maintain a constant number of
    vehicles in the network. Vehicle insertions are delayed whenever
    this number would be exceeded. Previously this option would
    terminate the simulation when the number was exceeded. To avoid
    a large number of delayed vehicles it is recommended to use the
    option **--max-depart-delay** {{DT_TIME}}. #1958
  - Traffic detectors which are generated for [actuated traffic lights now support additional parameters](../Simulation/Traffic_Lights.md#actuated_traffic_lights)
    to allow writing output files the same way as [regular detectors](../Simulation/Output/Induction_Loops_Detectors_(E1).md). #1839
  - Angles in simulation output and TraCI results now conform to
    *Navigational Standards* with *0* pointing towards the North and
    *90* pointing due East. #1372

- netedit
  - [netedit](../netedit.md) is now open. Have fun.
  - Int and float options can now be set in the
    *Processing-\>Options* dialog.
  - Added many lane- and junction-coloring modes already known from
    [sumo-gui](../sumo-gui.md#edge2fLane_visualisation_settings.md). #1756
  - Pedestrian crossings are now supported when editing traffic
    light plans.
  - Attributes of pedestrian crossings can now be modified.
  - Added context-menu option for removing intermediate geometry
    points from selected edges. #1913
  - Vehicle class permissions can now be edited via check-boxes
    instead of typing all class names. #1909
  - Individual lanes and selections of lanes can now be deleted when
    unchecking *Select edges*. #1895
  - A lane (or a selections of lanes) can now be duplicated by
    selecting *Duplicate lane* from the context menu. #1896
  - *Selection Mode* now allows [additional operators](../netedit.md#select) when matching against a
    non-numerical attribute. #1923
  - Added new option *Copy edge name to clipboard* to the lane
    popup-menu.
  - Junction attribute *keepClear* is now supported.
  - Custom junction shapes can now be drawn by selecting *Set custom
    shape* from the junction popup-menu. This will create a
    [modifiable shape outline. The popup-menu of this outline allows saving, discarding and simplifying the shape.](../netedit.md#modifiable_poly)
  - Added *reload* menu option. #2029
  - When editing traffic light plans, [states can now be set for multiple links and multiple phases at the same time](../netedit.md#traffic_lights).

- sumo-gui
  - Persons can now be tracked by selecting *Start Tracking* from
    the context menu. #1779
  - The current route of pedestrians can now be shown by selecting
    *Show Current Route* from the context menu. #1906
  - Error messages can now by clicked for jumping to the referenced
    simulation object (i.e. a teleporting vehicle). #900
  - Added person statistics to network parameter dialog. #1898
  - Added new menu option *Edit-\>Open in netedit* for opening the
    current network (at the current location) in
    [netedit](../netedit.md).
  - Added new option *Copy edge name to clipboard* to the lane
    popup-menu.
  - Added new options *Close edge* and *Close lane* to the lane
    popup-menu. This will force vehicles (with an assigned vClass)
    to wait until the corresponding lanes have been reopened (also
    via popup-menu).
  - Added new option *Add rerouter* to the lane popup-menu. This
    will make vehicles recompute their route when entering that
    edge.
  - The size and color of link indices can now be customized (old
    [gui settings files](../sumo-gui.md#changing_the_appearancevisualisation_of_the_simulation)
    may have to be updated).
  - Average trip data (for completed vehicle trips) is now available
    in the network parameter dialogue when running with option **--duration-log.statistics**.
  - Added new junction visualization option *draw
    crossings/walkingareas*. #1899
  - Vehicles can now be colored *by depart delay* (the differences
    between desired and actual depart time). Depart delay was also
    added to the vehicle parameter dialog. #1970
  - Junction shapes are no longer drawn when their color is set to
    fully transparent.
  - The network version is now shown the the network parameter
    dialog.

- netconvert
  - [Zipper merging](https://en.wikipedia.org/wiki/Merge_%28traffic%29) is
    now supported via the new [node type *zipper*](../Networks/PlainXML.md#node_types). #1292
  - [Right-turn-on-red](https://en.wikipedia.org/wiki/Right_turn_on_red)
    is now supported via the new [node type *traffic_light_right_on_red*](../Networks/PlainXML.md#node_types). #1287
  - Importing *.inpx* [VISSIM networks](../Networks/Import/Vissim.md) is now supported.
    Thanks to the [AIT](http://www.ait.ac.at/) for their
    contribution.
  - The positioning and presence of [internal junctions](../Networks/SUMO_Road_Networks.md#internal_junctions)
    can now be [customized with the new connection attribute `contPos`](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which). #2016
  - The maximum number of connections per junction was raised from
    64 to 256.
  - Added options **--geometry.remove.keep-edges.explicit** {{DT_STR}} and **--geometry.remove.keep-edges.input-file** {{DT_FILE}} to exclude edges from being modified when
    using option **--geometry.remove**. #1929
  - When specifying [multiple connections from the same edge to the same target lane](../Networks/PlainXML.md#multiple_connections_from_the_same_edge_to_the_same_target_lane),
    safe right-of-way rules are now established among the
    conflicting connections. #1859
  - Added options **--tls.cycle.time** {{DT_TIME}} to ensure that heuristically generated traffic
    light plans have a fixed cycle length. The new default is *90*
    (s) which will have no effect on most 4-arm intersections but
    will cause different timings for controlled 3-arm intersections
    and other types. #1942
  - Added options **--edges.join** {{DT_BOOL}} to select whether edges that connect the same
    nodes (and have similar geometry) shall be joined into an edge
    with multiple lanes. The new default is *false* (before, this
    heuristic was always active). #1930
  - Street names are now imported form
    [OpenDrive](../Networks/Import/OpenDRIVE.md).
  - Now including sumo edge-ids in [OpenDrive export](../Networks/Further_Outputs.md#opendrive_road_networks)
    if option **--output.original-names** is given (as `<userData sumoId="sumo_edge_id"/>`).
  - Now using more lane types in [OpenDrive export](../Networks/Further_Outputs.md#opendrive_road_networks).
  - Added option **--default.junctions.radius** {{DT_FLOAT}} for setting the default `radius` of
    [nodes](../Networks/PlainXML.md#node_descriptions). #1987
  - Added option **--tls.left-green.time** {{DT_TIME}} to configure the default duration for the
    dedicated left-turn phase. A value of 0 disables building this
    phase. #1949
  - Added option **--sidewalks.guess.exclude** {{DT_STR}} to prevent guessing a sidewalk for the given list
    of edges.
  - Added option **--junctions.internal-link-detail** {{DT_INT}} for setting the number of geometry points for
    lanes within intersections (Before, this was hard-coded as *5*). #1988
  - Added option **--crossings.guess.speed-threshold** {{DT_FLOAT}}. This allows setting the speed threshold above
    which crossings will not be generated at uncontrolled nodes
    (before this was hard-coded to *13.89*m/s). #1819

- polyconvert
  - Added option **--fill** {{DT_BOOL}} to control whether polygons are filled by default
    or not. #1858
  - Added option **--shapefile.fill** {{DT_STR}} to override the fill state when importing
    shapefiles. Allowed values are **\[auto|true|false\]**.

- MESO-GUI
  - It is now possible to select individual vehicles, to examine
    their parameters, track them and show their route just like for
    [sumo-gui](../sumo-gui.md). #1932
  - Vehicles can now be located as in
    [sumo-gui](../sumo-gui.md). #1487

- Tools
  - [traceExporter.py](../Tools/TraceExporter.md) now supports
    exporting traces of persons when using the new option **--persons** {{DT_BOOL}}.
  - Added option **--ignore-gaps** {{DT_BOOL}} for
    [traceExporter.py](../Tools/TraceExporter.md). When this is
    set vehicles will not be destroyed until the end of the fcd-file
    even when disappearing for a few simulation steps..
  - The [osmWebWizard.py](../Tools/Import/OSM.md#serverpy)
    import script is now more robust in finding a suitable output
    directory and recovering from errors.
  - Added new tool
    [tlsCoordinator.py](../Tools/tls.md#tlscoordinatorpy)
    which coordinates traffic lights in a network for a given
    traffic demand in order to create green waves for many vehicles.
  - Connection objects from networks parsed via
    [Sumolib](../Tools/Sumolib.md) can now return
    *getTLLinkIndex* as well as *getJunctionIndex*. #2048

- TraCI
  - Added function *simulationStep()* to the [C++ TraCI library](../TraCI/C++TraCIAPI.md).

### Other

- Documentation
  - Online documentation of
    [TraCI4J](http://github.com/egueli/TraCI4J) can now be found at
    [traci4j-javadoc](http://sumo.dlr.de/daily/javadoc/traci4j/)
  - The section on [Additional netconvert outputs was completely rewritten](../Networks/Further_Outputs.md#further_outputs).
  - Added a [new page on XML Validation](../XMLValidation.md)
  - Added [documentation for the C++ TraCI API client](../TraCI/C++TraCIAPI.md)
  - Added documentation on [route probe detectors](../Simulation/Output/RouteProbe.md) (which was
    quite overdue).
  - Expanded documentation on
    [Calibrators](../Simulation/Calibrator.md) in regard to
    their mechanism for generating new vehicles.
  - Added [overview page for simulation routing](../Simulation/Routing.md).

- Simulation
  - Attribute names of the [energy model (battery device, charging stations)](../Models/Electric.md) were updated to bring
    them in line with naming conventions. Refer to the documentation
    for new attribute names.

- netconvert
  - Network version is now **0.25**. New features that justify the
    version change are:
    - New linkstate *Z* and junction type *zipper* for zipper
      merging.
    - New junction type *traffic_light_right_on_red*. To
      accommodate this type of junction, linkstate 's' (stop) is
      now allowed in traffic light plans.
    - Multiple connections to the same lane do not result in a
      warning any more. (The conflict is resolved using zipper
      merging or priority right of way)
    - New network attributes *lefthand*, *junctionCornerDetail*
      and *junctionLinkDetail*
    - Note, that the network version should have been updated in
      0.23.0 due to the introduction of ships but this was
      forgotten.

- sumo-gui
  - The visualization options *Show internal edge name* and *Show
    crossing and walkingarea name* were moved from the *Streets*-tab
    to the *Junctions*-tab.

- Tools
  - *extractRouteEdges.py* was removed as the functionality is now
    fulfilled by
    [*route2sel.py*](../Tools/Routes.md#route2selpy).
  - [server.py](../Tools/Import/OSM.md) was renamed to
    [osmWebWizard.py](../Tools/Import/OSM.md) and now resides
    directly within the *tools* folder.

- Misc
  - *start-commandline-bat* now adds python (python 2.7 in it's
    default install location) and {{SUMO}}/tools to the path.
  - Simplified runner script of
    [Tutorials/TraCI4Traffic_Lights](../Tutorials/TraCI4Traffic_Lights.md)