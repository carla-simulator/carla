---
title: Z/Changes from Version 0.26.0 to Version 0.27.0
permalink: /Z/Changes_from_Version_0.26.0_to_Version_0.27.0/
---

## Version 0.27.0 (12.07.2016)

### Bugfixes

- Simulation
  - Attribute `via` is now used when routing trips and flows within the
    simulation. #1272
  - Vehicles stopping at a `<stop>` now reach the exact location when using
    sub-second step-lengths. #1224
  - Vehicles are no longer considered stopped at a `<stop>` while still
    driving with high speed. #1846
  - Scheduled
    [stops](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops)
    no longer count towards *waitSteps* and *timeLoss* in
    [tripinfo-output](../Simulation/Output/TripInfo.md#generated_output). #2311
  - Fixed bug where vehicles would not depart from a triggered stop #2339
  - Fixed deadlock when vehicles with triggered stops could not load
    passengers or containers due to capacity constraints.
  - Fixed invalid edge travel times used for dynamic routing in case
    flow differs among the lanes. #2362
  - Fixed invalid edge travel times used for dynamic routing due to
    invalid averaging #2351
  - Fixed invalid time stamps for leave times and an off by one for
    instant induction loops #1841

- MESO
  - The jam-front back-propagation speed now reaches realistic
    values (it was illogically low before). Note that default value
    of option **--meso-taujj** changes as well as it's semantics. #2244
  - Fixed bug where the simulation would not terminate when using
    calibrators. #2346
  - The options **--meso-tauff** and **--meso-taufj** now define net time gaps (default values were
    changed accordingly). The gross time gaps are computed based on
    vehicle lengths and edge speed to allow for more realistic flow
    in networks with widely varying speed limits. This also affects
    the threshold that defines jamming when using default options
    (thresholds based on allowed speeds). #2364

- netconvert
  - Fixed connection-guessing heuristic. #1992, #1219, #1633, #2398
  - Option **--remove-edges.by-vclass** is now working when loading a *.net.xml* file. #2280
  - Fixed bugs when importing cycleways from OSM. #2324
  - Option **--output.original-names** now records original edge ids even if input edges were
    joined. #2369
  - Fixed invalid road types when exporting OpenDRIVE networks. #2487
  - Fixed invalid lane permissions due to invalid removal of
    geometry-like nodes. #2488

- sumo-gui
  - Fixed crash when loading a large number of background images.
  - Fixed persons showing up too early in the locator. #1673

- netedit
  - Fixed crash when trying to set an empty string as edge length or
    edge width. #2322
  - Fixed crash when deleting the last lane of an edge.

- duarouter
  - Trips and flows that use attribute `via` to loop over the destination
    edge more than once are now working. #2271

- marouter
  - The output is now correctly sorted when using trips as input. #2361

- polyconvert
  - polyconvert output files can now be imported again by
    polyconvert (i.e. for further transformations). #1715

- TraCI
  - Fixed *route.add*, *gui.screenshot* and *gui.trackVehicle* and
    various *lane* commands for the [C++ TraCI API
    client](../TraCI/C++TraCIAPI.md)
  - Fixed crash when trying to set invalid routes. #2285
  - Fixed invalid values when retrieving edge mean speed and edge
    travel time in case flow differs among the lanes. #2362
  - Fixed retrieval of exit times for vehicles that spend multiple
    steps on an inductionloop when retrieving *last step vehicle
    data (0x17)*. #2326

### Enhancements

- Simulation
  - Added [model for
    sublane-simulation](../Simulation/SublaneModel.md). This is
    activated by setting the option **--lateral-resolution** {{DT_FLOAT}}. When using this option,
    vehicles may move laterally within their lanes. This is
    influenced by the new [vType-attributes *latAlignment*,
    *maxSpeedLat*,
    *minGapLat*](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types).
    Lane changing is performed according to the new lane changing
    model
    [SL2005](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models).
  - Lane-changing models can now be configured with additional `vType`
    parameters. There exists [one parameter for each of the
    lane-changing
    motivations](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models)
    *strategic,cooperative,speedGain* and *keepRight* which can be
    used to increase or reduce that type of lane changing. #1136
  - Added capabilities for [overtaking through the
    opposite-direction
    lane](../Simulation/OppositeDirectionDriving.md).
  - Added new option **--collision.action** {{DT_STR}} for configuring the action to take on vehicle
    collision. Allowed actions are *none,warn,teleport* and
    *remove*. The default is *teleport* which moves the rear vehicle
    involved in a collision onto a subsequent lane (as before).
  - Added new option **--collision.check-junctions** {{DT_BOOL}} to enable geometrical collision checking on
    junctions. By default this option is set to *false* and
    collisions between non-consecutive lanes are ignored as before.
    This option may slow down the simulation. #984
  - ChargingStations [can now be used to declare vehicle
    stops](../Models/Electric.md#stopping_at_a_charging_station). #2248
  - the [vehicle route
    output](../Simulation/Output/VehRoutes.md) now includes
    optional vehicle parameters as `param` entries
  - Added new option **--device.rerouting.adaptation-steps** {{DT_INT}} to switch the default algorithm for averaging
    edge travel times from exponential averaging to a moving average
    over the given number of steps. #2374
  - Added new option **--tls.all-off** {{DT_BOOL}} for switching off all traffic lights (the
    traffic lights can still be switched on via GUI or TraCI). #2442
  - Added new [output for tracking lane change
    events](../Simulation/Output/Lanechange.md). This is
    enabled using the new option **--lanechange-output** {{DT_FILE}}. #2461

- MESO
  - The option **--meso-jam-threshold** {{DT_FLOAT}} now gives additional freedom when configuring a
    speed dependent jam-threshold. When an value below 0 is given
    the absolute value is taking as a factor for the minimum
    unjammed speed. Thus, negative values closer to 0 result in less
    jamming. The default value remains at *-1* and results in the
    original behaviour (values above 0 set the occupancy fraction
    threshold independent of edge speed as before).

- sumo-gui
  - The number of running vehicles and persons is now shown in the
    status bar. This display also acts as a button for opening the
    network parameter dialog. #1943
  - [Charging
    stations](../Models/Electric.md#charging_stations) are now
    shown in a different color when active.
  - Persons are now more visible when selecting *Draw with constant
    size when zoomed out*.
  - Added the averaged speeds that are used for [simulation
    routing](../Demand/Automatic_Routing.md) to the lane
    parameter dialog.
  - Added new option **--demo** {{DT_BOOL}} which automatically reloads and starts the
    simulation every time it ends. #1645

- MESO-GUI
  - Can now color edges by the averaged speeds that are used for
    [simulation routing](../Demand/Automatic_Routing.md).
  - Can now color edge segments (mesoscopic vehicle queues)
    individually by various traffic measures. #2243

- netconvert
  - Added option **default.lanewidth** {{DT_FLOAT}} for setting the default width of lanes (also
    applies to [netgenerate](../netgenerate.md)).
  - Added option **numerical-ids** {{DT_BOOL}} for forcing all node and edge IDs to be integers
    (also applies to [netgenerate](../netgenerate.md)). #1742
  - Added Option **speed.minimum** to avoid negative speeds when using Option **--speed.offset**. #2363

- netedit
  - Many additional network structures such as busStops, detectors
    and variable speed signs can now be defined and manipulated. #1916

- netgenerate
  - Added option **grid.alphanumerical-ids** {{DT_BOOL}} for using a chess-like intersection naming scheme
    (A1, B3, etc).

- TraCI
  - Added [vehicle
    command](../TraCI/Vehicle_Value_Retrieval.md) *next TLS* to
    retrieve upcoming traffic lights along a vehicles route. #1760
  - The [vehicle
    command](../TraCI/Change_Vehicle_State.md#move_to_xy_0xb4)
    *move to XY* (formerly *move to VTD*) now supports an additional
    flag which selects whether the original route shall be kept or
    the route may change and whether the vehicle may leave the road
    network. #2033, #2258
  - The [vehicle
    command](../TraCI/Change_Vehicle_State.md#move_to_xy_0xb4)
    *move to XY* now allows moving vehicles that are still in the
    insertion buffer.
  - Added functions *vehicle.add, vehicle.remove* and
    *vehicle.moveToXY* to the [C++ TraCI API
    client](../TraCI/C++TraCIAPI.md)
  - Added object variable subscriptions and context subscriptions to
    the C++ TraCI-library (*subscribe, getSubscriptionResults,
    subscribeContext, getContextSubscriptionResults*). Thanks to
    Erik Newton for the patch.
  - Added person value retrieval functions to the [C++ TraCI API
    client](../TraCI/C++TraCIAPI.md). Thanks to Caner Ipek
    for the patch.
  - Added [vehicle
    command](../TraCI/Vehicle_Value_Retrieval.md) *get
    speedmode 0xb3* to retrieve the [speed
    mode](../TraCI/Change_Vehicle_State.md#speed_mode_0xb3)
    of vehicles. #2455
  - Added [vehicle
    command](../TraCI/Vehicle_Value_Retrieval.md) *get slope
    0x36* to retrieve the slope at its current position #2071
  - Added [vehicle](../TraCI/Vehicle_Value_Retrieval.md),
    [lane](../TraCI/Lane_Value_Retrieval.md) and
    [edge](../TraCI/Edge_Value_Retrieval.md) command *get
    electricity consumption 0x71* to retrieve the electricity
    consumption if the emission model supports it. #2211
  - Multiple subscriptions for the same object are now merged. #2318

- Tools
  - Added new tool
    [createVehTypeDistributions.py](../Tools/Misc.md#createvehtypedistributionspy)
    to simplify definition of heterogeneous vehicle fleets by
    sampling numerical attributes from configurable distributions.
    Thanks to Mirko Barthauer for the contribution.
  - parsing xml files with
    [sumolib.output.parse()](../Tools/Sumolib.md) is now much
    faster. #1879

### Other

- Documentation
  - Test coverage analysis can now be found at
    [\[1\]](http://sumo.dlr.de/daily/lcov/html/).
  - Documented [Wireless device detection
    model](../Simulation/Bluetooth.md) which has been available
    since version 0.18.0

- netconvert
  - Network version is now 0.27.0

- TraCI
  - TraCI version is now 12.
  - The [vehicle command](../TraCI/Change_Vehicle_State.md)
    *move to VTD* is now referred to as *move to XY* in client code. #2032