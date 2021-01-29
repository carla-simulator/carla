---
title: Z/Changes from Version 0.29.0 to Version 0.30.0
permalink: /Z/Changes_from_Version_0.29.0_to_Version_0.30.0/
---

## Version 0.30.0 (02.05.2017)

### Bugfixes

- Simulation
  - Random pedestrian decelerations (configured via option **--pedestrian.striping.dawdling**) are now
    working. #2851
  - Loading state-files with vehicles that stop at a `<busStop>` is now
    working. #2914
  - Fixed collisions when using the sublane model. #2295, #2924, #2154, #3005
  - Various fixes to lateral distance keeping in the sublane model.
  - Vehicles now longer drive beyond the road borders when using the
    sublane model. #2988
  - Fixed crash when loading saved stated with an arriving vehicle. #2938
  - Option **--load-state.offset** now properly applies to vehicles departing in the future
    that are part of the loaded state (i.e. due to having been
    loaded from an additional file before saving). This was a
    regression in version 0.29.0. #2949
  - Simulation behaviour is no longer affected by randomly equipping
    vehicles with devices that only generate outputs. #2977
  - vType-attribute `laneChangeModel` is no longer ignored (was silently replaced
    with "default", since version 0.28.0)
  - Fixed deadlock when setting vehicle attributes *arrivalSpeed*
    and *arrivalPos* both to 0. #2995
  - Fixed issue where a lane-change was blocked for invalid reasons
    causing deadlock. #2996
  - When using `departLane="best"`, the look-ahead distance is now limited to 3000m for
    determining suitable insertion lanes. #2998
  - loading state now writes tls states correctly #1229

- netconvert
  - Various fixes to junction-shape computation. #2551, #2874, #1443
  - Ramp-guessing (option **--ramps.guess**) no longer identifies sharply turning
    roads as motorway ramps.
  - Fixed invalid right-of-way rules at junctions with type
    *traffic_light_right_on_red* when importing a *net.xml* file
    or editing with [netedit](../netedit.md). #2976
  - Networks built with option **--junctions.scurve-stretch** now retain their shape when imported
    again. #2877
  - Networks [imported from VISUM](../Networks/Import/VISUM.md)
    no longer round node positions to meters. #3001
  - Loading *.tll* files from a network that includes node types `rail_crossing` or `rail_signal`
      is now working. #2999
  - Fixed crash when applying a `<split>` to an edge within a roundabout. #2737
  - Fixed infinite loop when importing some OpenDRIVE networks. #2918

- netedit
  - [vClass-specific speed limits](../Networks/PlainXML.md#vehicle-class_specific_speed_limits)
    are no longer lost when saving a network. #2508
  - The lane shapes induced by option **--rectangular-lane-cut** are no longer lost when
    editing a network
  - Deleting whole edges is working again (regression in 0.29.0). #2883
  - Deleting edges and afterwards adding edges no longer creates
    node type "unregulated" (without right-of-way rules). #2882
  - Fixed visual glitches when opening left-hand networks. #2890
  - Fixed various crashes. #2902, #3010, #3026, #2969
  - Fixed coloring of green-verge lanes. #2961

- TraCI
  - Fixed mapping failures when calling *moveToXY*.
  - Vehicles that are moving outside the road network due to
    *moveToXY* calls now return the correct position and angle.
  - Function *vehicle.getSpeedWithoutTraCI* now correctly returns
    the current speed if the vehicle is not being influenced.
  - Fixed crash when adding and erasing persons in the same step. #3014

- Tools
  - Restored Python3.0 compatibility for sumolib and traci
    (regression in 0.29.0)
  - carFollowing child-elements of vType element are now included in
    route2trips.py output. #2954
  - Fixed OSM Web Wizard problems with spaces in SUMO_HOME path. #2939
  - [cutRoutes.py](../Tools/Routes.md#cutroutespy) now writes
    two independent routes instead of one containing edges not
    contained in the reduced network. #3011

### Enhancements

- Simulation
  - [vClass-specific speed limits](../Networks/PlainXML.md#vehicle-class_specific_speed_limits)
    can now be loaded from an [*additional-file*](../sumo.md#format_of_additional_files). #2870
  - Refactored implementation of [E2 detectors](../Simulation/Output/Lanearea_Detectors_(E2).md).
    These may now be defined to span over a sequence of lanes,
    XML-attribute `cont` is deprecated. #1491, #1877, #2773, #2871
  - Added [traffic light type "delay_based"](../Simulation/Traffic_Lights.md#based_on_time_loss)
    which implements an alternative algorithm to type "actuated".
  - Added option **--time-to-impatience** {{DT_TIME}} which defines the waiting time until driver
    impatience grows from 0 to 1. Formerly this was tied to the
    value of **--time-to-teleport**. #2490
  - [lanechange-output](../Simulation/Output/Lanechange.md) now
    includes the lateral gap to the closest neighbor. #2930
  - [attribute `speedFactor`](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
    can use normal distributions with optional cutoff to define the
    distribution of vehicle speeds #2925
  - [Traffic light related outputs](../Simulation/Output/Traffic_Lights.md) have now
    consistent camelCase XML tags.
  - Added option **collision.stoptime** {{DT_TIME}} which lets vehicles stop for a time after
    experiencing a collision before the action set via **--collision.action** takes place. #1102
  - [Electric vehicles](../Models/Electric.md) are now
    initialized with a maximum capacity of 35kWh and and a half full
    battery by default (before, the default was 0 which always made
    it necessary to define this).
  - **--vehroute-output** now includes additional vehicle attributes such as *departLane*
    and *departSpeed* to facilitate scenario replaying.
  - Increased maximum possible insertion flow when using
    *departLane* values *free,allowed* or *best* on multi-lane
    edges. #3000
  - Improvements to lateral-gap keeping when using the [sublane model](../Simulation/SublaneModel.md):
    - Vehicles now attempt to equalize left and right gaps if
      there is insuffient lateral space #2932
    - The semantics of attribute `minGapLat` where changed to define the
      desired gap at 50km/h and do not grow beyond that speed
      (before that threshold was at 100km/h)
    - Vehicles now ignore follower vehicles behind the midpoint of
      their own length in regard to lateral gap keeping.
  - Added option **--max-num-teleports** {{DT_INT}} which can abort the simulation after a number of
    teleports is exceeded. #3003
  - Added option **--chargingstations-output** {{DT_FILE}} which generates output for
    [chargingStations](../Models/Electric.md#charging_station_output). #2357

- sumo-gui
  - Added parameters
    *minGapLat,maxSpeedLat,latAlignment,boardingDuration,loadingDuration,car
    follow model* to the vType-parameter window.
  - Added parameter *acceleration* to the vehicle-parameter window.
  - Added option *Show type parameter dialog* to the person context
    menu. #2973
  - When running the simulation with option **--duration-log.statistics**, the average travel
    speed of completed trips is shown in the network parameter
    dialog.
  - For long-running simulations the time displays can now show
    elapsed days. #2889

- netconvert
  - Networks imported from
    [DlrNavteq](../Networks/Import/DlrNavteq.md)-format now
    process *prohibited_manoeuvres* and *connected_lanes* input
    files. #738
  - Edge types are now imported from a *.net.xml* file. #2508
  - Added option **--opendrive.curve-resolution** {{DT_FLOAT}} for setting the level of detail when importing
    road geometries from parmeterized curves. #2686
  - Node shape computation (especially stop line position) can now
    be influenced by setting edge geometries that do not extend to
    the node position.
    - To deal with ambiguous stop line information in OpenDrive
      networks, the new option **--opendrive.advance-stopline** {{DT_FLOAT}} may now now be used to affect the
      heuristic that computes stop line positioning based on the
      border between roads and connecting roads.
  - Bus stops can now be imported from OpenStreetMap using the new
    option **--ptstop-output** {{DT_FILE}}. #2933
  - Lanes within a network that have no incoming connection and
    edges that have no outgong connections are now reported. #2997
  - Specific lanes can now be deleted [via loaded *.edg.xml* files](../Networks/PlainXML.md#deleting_edges_or_lanes)
    (to ensure that connections are kept as intended). #3009

- netedit
  - The junction visualization options *show link junction index*
    and *show link tls index* are now working #2955
  - When creating or moving edge and junction geometry, positions
    can now be [restricted to a regular grid](../netedit.md#background_grid) (i.e. multiples of
    100). #2776
  - [Custom edge geometry endpoints](../netedit.md#specifying_the_complete_geometry_of_an_edge_including_endpoints)
    values can now be entered in inspect mode. #2012

- TraCI
  - It is now possible to reload the simulation with new options by
    sending the [load command](../TraCI/Control-related_commands.md#command_0x01_load).
  - Added *vehicle.setMaxSpeed* and *vehicle.getMaxSpeed* to the C++
    client. Thanks to Raphael Riebl for the patch.
  - Added *vehicle.changeTarget* to the C++ client.
  - To allow vehicles to run a red light, speedmode *7* can now be
    used instead of *14*. This is much safer as it avoids rear-end
    collisions.
  - Vehicles can now stop at a named ParkingArea or ChargingStation.
    The methods *traci.vehicle.setParkingAreaStop,
    traci.vehicle.setChargingStationStop* were added to the python
    client to simplify this. #2963
  - vehicle function *moveToXY* now supports the special angle value
    *traci.constants.INVALID_DOUBLE_VALUE*. If this is set, the
    angle will not be factored into the scoring of candidate lanes
    and the vehicle will assume the angle of the best found lane.
    For vehicles outside the road network, the angle will be
    computed from the old and new position. #2262
  - Vehicles now support [retrieval of battery device parameters and retrieval of riding persons and containers as well as retrieval of laneChangeModel parameters](../TraCI/Vehicle_Value_Retrieval.md#device_and_lanechangemodel_parameter_retrieval_0x7e)
    using the *vehicle.getParameter* function.
  - Vehicles now support [setting of battery device parameters and laneChangeModel parameters](../TraCI/Change_Vehicle_State.md#setting_device_and_lanechangemodel_parameters_0x7e)
    using the *vehicle.setParameter* function.
  - Added [sublane-model related](../Simulation/SublaneModel.md) vehicle functions
    *getLateralLanePosition, getMaxSpeedLat, getMinGapLat,
    getLateralAlignment, setMaxSpeedLat, setMinGapLat,
    setLateralAlignment, changeSublane*. #2216, #2001
  - Added [sublane-model related](../Simulation/SublaneModel.md) vehicletype
    functions *getMaxSpeedLat, getMinGapLat, getLateralAlignment,
    setMaxSpeedLat, setMinGapLat, setLateralAlignment*. #2216
  - Function *edge.getLastStepPersonIDs* now includes persons riding
    in a vehicle which are on that edge. #2865
  - The TraCI python client now supports
    [StepListeners](../TraCI/Interfacing_TraCI_from_Python.md#adding_a_steplistener).
  - The lane-changing choices of the laneChange model can now be
    retrieved (with and without TraCI influence) using [command *change lane information 0x13*](../TraCI/Vehicle_Value_Retrieval.md).

- Miscellaneous
  - Improved routing efficiency of
    [sumo](../sumo.md), [duarouter](../duarouter.md) and
    [marouter](../marouter.md) when using option **--routing-algorithm astar**.

- Tools
  - [randomTrips.py](../Tools/Trip.md#randomtripspy) now
    supports the option **--flows** {{DT_INT}} to generate a number of random flows
    instead of individual vehicles.
  - [routeStats.py](../Tools/Routes.md#routestatspy) now
    supports generating statistics on departure time by setting the
    option **--attribute depart**.
  - [tls_csv2SUMO.py](../Tools/tls.md#tls_csv2sumopy) can now
    take arbitrary strings as index and has improved signal group
    handling, thanks to Harald Schaefer
  - more tools (including osmWebWizard) are python3 compatible

### Other

- Documentation
  - The [TraCI command documentation](../TraCI.md#traci_commands) now includes
    links to the corresponding python functions for each command.
  - New [overview page on safety-related behavior](../Simulation/Safety.md)
  - The [Quick Start tutorial](../Tutorials/quick_start.md) now
    describes how to create an network with
    [netedit](../netedit.md)

- TraCI
  - TraCI version is now 15
  - some TraCI constants have been renamed
    - CMD_SIMSTEP2 to CMD_SIMSTEP
    - \*AREAL_DETECTOR\* to \*LANEAREA\*
    - \*MULTI_ENTRY_EXIT_DETECTOR\* TO \*MULTIENTRYEXIT\*

- Miscellaneous
  - The compile-option **--disable-internal-lanes** was removed. Simulation without internal
    lanes is still possible using either the netconvert option **--no-internal-links** or
    the simulation option **--no-internal-links**
  - The compile-option **--enable-memcheck** and the corresponding nvwa package were
    removed. Checking for memory leaks should be done using the
    clang build or valgrind.