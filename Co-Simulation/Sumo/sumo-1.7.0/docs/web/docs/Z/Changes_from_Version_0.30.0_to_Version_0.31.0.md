---
title: Z/Changes from Version 0.30.0 to Version 0.31.0
permalink: /Z/Changes_from_Version_0.30.0_to_Version_0.31.0/
---

## Version 0.31.0 (14.09.2017)

### Bugfixes

- Simulation
  - [Sublane-model](../Simulation/SublaneModel.md)
    - Lane changing to clear the overtaking lane ([motivation
      *keepRight*](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models))
      is now working properly. #3106
    - Fixed error that prevented violating right-of-way rules in
      the sublane-model. #3140
    - Fixed bug that was preventing speed adaptations for
      strategic changing. #3183
    - Fixed error that prevented changing for speed gain #3107, #3186
    - Insertion with `departPosLat="random" departPos="last"` is now working. #3191
    - Fixed bug that could cause deadlocks on an intersection #3189
    - Fixed collisions #3195, #3213, #3050, #2837
    - Fixed invalid angles when lane-changing at low speeds or low
      step-length. #3220
    - Fixed oscillation. #3333
    - Fixed too-late changing for speed gain when approaching a
      slow leader. #3375
    - Fixed bug that was causing sublane-changing despite speed
      loss. #3372

  - Lane-changing
    - Fixed behavior problems in regard to the rule that prohibits
      overtaking on the right under some circumstances (by default
      this is prohibited in free-flowing motorway traffic).
      Vehicles now avoid overtaking whenever braking is possible
      and they overtake on the left instead if there is a third
      lane. #1214, #2547, #3115, #3379
    - Fixed time loss due to late overtaking in some situations. #2126
    - Fixed invalid overtaking to the left. #3337

  - Persons
    - Attribute *arrivalPos* is no longer ignored for person `<ride>`
      elements. #2811
    - Fixed crash when pedestrian routes contain disallowed edges #3248 #3381
    - Fixed collision at prioritized crossings because pedestrians
      ignored some vehicles #3393
  - Calibrators now respect the option **--ignore-route-errors**. #3089
  - `departLane="allowed"` no longer selects an invalid departLane on multimodal edge. #3065
  - Acquired waiting time of vehicles is now properly loaded from a
    simulation state. #2314
  - Fixed output of meso calibrator (regression in 0.30.0). #3132
  - Modified meanTimeLoss output of [lane area (e2)
    detector](../Simulation/Output/Lanearea_Detectors_(E2).md#generated_output).
    Semantics is now average time loss \*per vehicle\* #3110
  - Fixed invalid stop state and invalid position of vehicles that
    cannot resume from parking due to blocking traffic. #3153
  - Fixed erroneous calculation of occupancy in meandata output for
    vehicles being only partially on the corresponding lane. #153
  - Fixed collision detection of junctions (some collisions were not
    registered). #3171
  - Options **--collision.stoptime** is now working when collisions are detected on a
    junction (using Option **--collision.check-junctions**). #3172
  - FCD-output now contains z-data if the network includes elevation
    information. #3191
  - Fixed crash when loading invalid `<busStop>` definition with `<access>` element. #3214
  - Fixed invalid errors on loading stops on looped routes. #3231
  - Fixed crash on invalid `<e2Detector>` definition. #3233
  - Fixed crash on saving and loading simulation state in
    conjunction with vehroute-output. #3237, #3238
  - Fixed too high density values in meandata output. #3265
  - Fixed invalid *routeLength* in tripinfo-output. #3355

- sumo-gui
  - Fixed visual glitch when drawing vehicles with multiple
    carriages as raster images. #3049
  - Fixed crash when reloading a simulation after editing the
    network. #3059
  - Fixed crash when using invalid routes in calibrator. #3060
  - The number of nodes listed in the network parameter dialog no
    longer includes internal nodes. #3118
  - Fixed crash when multiple vehicles start and end parking on the
    same edge #3152
  - All parking vehicles and empty parking spaces of a `<parkingArea>` are now
    accessible via right-click. #3169
  - Fixed rendering position of `<busStop>, <containerStop>, <chargingStation>, <parkingArea>` on curved roads. #3200
  - Fixed drawing position of vehicles with lateral offset and of
    passengers if a vehicle is on a very short lane (also affects
    fcd-output). #3249
  - Corrected drawing of sublane borders in case the lane width is
    not a multiple of the lateral-resolution.
  - Fixed wrong occupancy values in Parameter Window for short
    lanes.
  - [Pre-configured screenshots](../sumo-gui.md#screenshots)
    are now taken at the correct time regardless of simulation
    speed. #1340
  - Fixed visual glitches when drawing waiting pedestrians, parking
    vehicles and parkingAreas in left-hand networks. #3382
  - The list of additional simulation objects no longer includes
    POIs and polygons (they have their own locator lists). #3384

- netconvert
  - **--ptstop-output** now exports stop lanes with the appropriate vClass. #3075
  - **--ptstop-output** now exports stop lanes in the correct road direction #3101, #3212
  - Fixed invalid geo-reference when loading lefthand *.net.xml*
    files #3198
  - Fixed bug that was causing an error when patching a *.net.xml*
    file with a *.tll.xml* file along with other
    connection-affecting patches. #2868
  - Fixed insufficient precision of internal lane elevation in
    OpenDrive output.
  - Fixed overly long yellow duration in generated tls plans. #3239
  - Fixed invalid lengths of internal turning lanes. As a side
    effect lane-changing is not possible any more while on these
    lanes. The old behavior can be enabled by setting option **--junctions.join-turns** {{DT_BOOL}}. #2082.
  - Variable phase durations are no longer lost when importing from
    *.net.xml* or *.tll.xml* files.
  - Information about edges without connections is no longer lost
    when exporting with option **--plain-output-prefix**. #3332
  - Fixed invalid edge shape after importing a *.net.xml* file with
    custom node shape. #3385
  - Fixes related to importing OpenDRIVE networks
    - Fixed error when loading `<roadMark><type>`-data. #3163
    - Fixed crash when loading OpenDRIVE networks with attribute
      *pRange*. #3164
    - Fixed invalid traffic lights. #3203
    - Fixed invalid connections when importing OpenDRIVE networks
    - Fixed invalid internal-lane speed when importing OpenDRIVE
      networks or setting lane-specific speeds. #3240

- netedit
  - Fixed rendering slowdown (regression in 0.30.0) #3167
  - Fixed error when loading pois with attributes *lane* and *pos*
    (regression in 0.30.0) #3199
  - Fixed crash when trying to filter selection of additionals by ID
    (regression in 0.30.0). #3068
  - The z-Coordinate of junctions is now properly displayed again in
    inspect mode (regression in 0.30.00). #3134
  - The z-Coordinate of junctions is no longer reset to 0 when
    moving them with *move mode*. #3134
  - busStop and chargingStation elements with negative *startPos* or
    *endPos* can now be loaded.
  - Undo now restores the selection status of deleted additionals. #3073
  - Loading color schemes is now working (only schemes saved by
    netedit are valid). #2936
  - Fixed invalid geo-reference when editing lefthand networks #3198
  - The cycle time is now always shown for selected traffic lights
    in tls-mode. #3206
  - Output precision set in the Options dialog now take effect. #3219
  - When selecting edges (or lanes) that allow a specific vehicle
    class, edges (and lanes) with `allow="all"` are now matched. #3168
  - Fixed crash when <ctrl\>-clicking on invalid lanes in
    connection-mode.
  - Fixed invalid edge shape after setting a custom node shape. #3276
  - Function *replace by geometry node* now preserves connections,
    crossings and custom geometry endpoints. If the function is
    disabled, the reason is shown in the menu. #3287
  - Joining junctions now always preserves edge endpoints. #3257
  - Fixed invalid network after deleting traffic light and a
    junction with pedestrian crossings. #3346
  - Fixed crash when joining tls. #3365
  - The junction visualization option *Show lane to lane
    connections* now takes effect.

- duarouter
  - Fixed bugs that were causing intermodal routing failures. #3119, #3226
  - Fixed invalid `speedFactor` output when specifying both `speedFactor` and `speedDev` in the input
    files. #3121
  - Fixed (almost) infinite loop when specifying `<flow>` without *end*. #3225
  - Fixed handling of *departPos* and *arrivalPos* for persons. #3246

- marouter
  - Fixed crash due to error in matrix parsing. #3366

- TraCI
  - After sending command *traci.load()* the simulation now keeps
    running until sending *traci.close()* instead of terminating
    when there are no more vehicles or the end time is reached. #3048
  - Vehicle state change retrieval (*simulation.getDepartedIDList,
    simulation.getArrivedNumber, ...*) are now working after sending
    command *traci.load()*. #3051
  - Vehicle commands *getDistance* and *getDrivingDistance* now
    return correct values when the current vehicle edge or the
    target edge are junction-internal edges. #2321
  - Fixed invalid lane occupancy values when calling moveToXY. #3185
  - *traci.load()* is now working if the previous simulation had
    errors. #3204
  - Invalid edge ids in *traci.vehicle.setRoute()* no longer cause
    crashing. #3205
  - Fixed *moveToXY* mapping failures. #3263
  - Person context subscriptions are now working. #3304
  - Fixed invalid behavior after canceling stop. #3348
  - Fixed freeze when calling *gui.screenshot* #3362

- Tools
  - [netdiff.py](../Tools/Net.md#netdiffpy) now correctly
    handles junctions that had their *radius* or *z* attributes
    changed to the (unwritten) default value.
  - [randomTrips.py](../Tools/Trip.md#randomtripspy) now
    correctly handles **--trip-attributes** that contain a space in the value (i.e.
    *modes*). #3117
  - Fixed bug that was causing insufficient flow when using
    [flowrouter.py](../Tools/Detector.md#flowrouterpy). #3285
  - Fixed [flowrouter.py](../Tools/Detector.md#flowrouterpy)
    crash. #3356

### Enhancements

- Simulation
  - Behavior at intersections can now be configured with new
    [junction model
    parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters). #3148
  - Emergency vehicles (`vClass="emergency"`) may always overtake on the right.
  - The default car following model can now be specified on the
    command line with **--carfollow.model**. #3142
  - Routing with **--routing-algorithm astar** is now working efficiently when using [traffic assignment zones](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#traffic_assignement_zones_taz). #3144
  - [Lanechange-output](../Simulation/Output/Lanechange.md) now
    includes the *type* of the changing and the longitudinal gaps on
    the target lane. #3156
  - Stops on internal lanes may now be defined.
  - Aggregate trip information generated via option **--duration-log.statistics** now includes
    vehicles that were still running at simulation end if the option
    **--tripinfo-output.write-unfinished** is also set. #3209
  - Vehicles now react to pedestrians on the same lane. #3242
  - Pedestrians now react to vehicles that are blocking their path. #3138
  - Collisions between vehicles and pedestrians are now detected
    when setting the option **--collision.check-junctions**.
  - Pedestrian `<walk>`s may now be defined using attribute *route*. #3302
  - Summary-output now includes mean vehicle speed (absolute and
    relative) as well as the number of halting vehicles. #3312
  - Pedestrian statistics are now included in the [aggregated traffic measures](../Simulation/Output.md#aggregated_traffic_measures). #3306
  - Tripinfo-output now includes additional attributes for persons
    and containers stages (*depart, waitingTime, duration, vehicle,
    arrivalPos, actType*). #3305
  - Added new lanechangeModel parameter *lcAccelLat* to model
    lateral acceleration in the sublane model. #3371

- sumo-gui
  - All `<param>` values of simulation objects (i.e. TLS) can now be
    inspected. #3098
  - Calibrators can now be defined for specific lanes not just for
    the whole edge. #2434

    !!! caution
        To obtain the old behavior, calibrators must use the attribute `edge`. When using the attribute `lane` the new behavior is activated.

  - When using the [Sublane model](../Simulation/SublaneModel.md), the lateral offset
    of left and right vehicle side as well as the rightmost and
    leftmost sublane are listed in the vehicle parameter dialog.
  - Added button for calibrating lane/edge colors to the current
    value range. #2464
  - [POIs](../Simulation/Shapes.md#poi_point_of_interest_definitions)
    which are defined using attributes *lane* and *pos* now accept
    the optional attribute *posLat* for specifying lateral offset
    relative to the lane. #3339. These attributes are automatically added
    as [generic parameters](../Simulation/GenericParameters.md)
    retrievable via TraCI.
  - Added option **--tracker-interval** {{DT_FLOAT}} to configure the aggregation interval of value
    tracker windows. Previously this was fixed at 1s. Now it
    defaults to the **--step-length** value.

- netconvert
  - [<split\>-definitions](../Networks/PlainXML.md#road_segment_refining)
    now support the attribute *id* to specify the id of the newly
    created node. Two-way roads can be split with the same node by
    using the same id in two split definitions. #3192
  - Variable lane widths are now taken into account when importing
    OpenDrive networks. The new option **--opendrive.min-width** {{DT_FLOAT}} is used to determine which
    parts of a lane are not usable by the vehicles (*default 1.8m*). #3196
  - Added option **--tls.red.time** {{DT_TIME}} for building a red phase at traffic lights that do
    not have a conflicting stream (i.e. roads with a a pedestrian
    crossing in a network that is not meant for pedestrian
    simulation). The new default value is 5 seconds. #1748
  - Added option **--tls.allred.time** {{DT_TIME}} for building all-red phases after every yellow
    phase. #573
  - Added option **--walkingareas** to allow [generation of walkingareas](../Simulation/Pedestrians.md#walkingareas) in
    networks without pedestrian crossings. #3253
  - Added option **--opposites.guess.fix-lengths** {{DT_BOOL}} to ensure that opposite lane information can be
    set for curved roads. (see
    [Simulation/OppositeDirectionDriving\#Limitations](../Simulation/OppositeDirectionDriving.md#limitations)).
  - Custom lane shapes [can now be defined](../Networks/PlainXML.md#lane-specific_definitions). #2250
  - Added options **--tls.min-dur** {{DT_TIME}} and **--tls.max-dur** {{DT_TIME}} for defining the time range of non-static
    traffic lights. #3329
  - The option **--proj.scale** now accepts arbitrary floats and can be used to
    scale the output network. #3351

- netedit
  - [POIs and Polygons](../netedit.md#pois_and_polygons) can
    now be defined with a new editing mode. #1667
  - Minimum and maximum phase duration for actuated traffic lights
    can now be defined. #831
  - Added button for calibrating lane/edge colors to the current
    value range. #2464

- TraCI
  - Support for multiple clients. #3105
  - Added function *vehicle.getAccumulatedWaitingTime* to retrieve
    the waiting time collected over the interval **--waiting-time-memory**. #999
  - Added many value retrieval functions to the C++ client. Thanks
    to Raphael Riebl for the patch\!
  - New vehicle types can be created dynamically (by duplicating
    existent). -\> *traci.vehicletype.copy()* #3211
  - Added function *person.rerouteTraveltime* to [reroute pedestrians](../TraCI/Change_Person_State.md#command_0xce_change_person_state). #3352
  - Rerouting-device [period can now be set for individual vehicles.
    assumed edge travel times can be set globally.](../TraCI/Change_Vehicle_State.md#supported_device_parameters) #3097
  - Rerouting-device [period and assumed edge travel times can now be retrieved.](../TraCI/Vehicle_Value_Retrieval.md#supported_device_parameters) #3097

- duarouter
  - Routing with **--routing-algorithm astar** is now working efficiently when using [traffic
    assignment zones](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#traffic_assignement_zones_taz). #3144
  - Stops on internal lanes are now supported. #3174
  - Pedestrian `<walk>`s may now be defined using attribute *route*. #3302

- dfrouter
  - Added option **--randomize-flows** for randomizing the departure times of generated
    vehicles.

- od2trips
  - Added option **--pedestrians** for generating pedestrian demand rather than
    vehicles. #3331
  - Added option **--persontrips** for generating [intermodal traffic demand](../Specification/Persons.md#persontrips). #3331

- Tools
  - [osmWebWizard.py](../Tools/Import/OSM.md#osmwebwizardpy)
    now supports location search. #2582
  - [osmWebWizard.py](../Tools/Import/OSM.md#osmwebwizardpy)
    now generates scenarios with actuated traffic lights
  - [osmWebWizard.py](../Tools/Import/OSM.md#osmwebwizardpy)
    now generates traffic with more realistic speed distribution
  - flowrouter.py now supports [specifying route restrictions to
    resolve ambiguities](../Tools/Detector.md#ambiguity).
  - When setting
    [randomTrips.py](../Tools/Trip.md#randomtripspy) option **--vehicle-class**,
    vType attributes from option **-t** are recognized and written to the
    generated vType. #3335

### Other

- SUMO now uses C++11
- specifying the car following model as nested element in a vType is
now deprecated
- trips without ids are deprecated
- router options are now more consistent with simulation options
  - use -a for additional files
  - use -r or --route-files for all kinds of route input (trips,
    flows, routes, alternatives)
  - the old options --flows, --trips, -- alternatives are deprecated
  - The network argument for
    [routeStats.py](../Tools/Routes.md#routestatspy) is now
    optional and set with option **-n**.
- The option **proj.shift** which used to be an alias for **--proj.scale** is no longer supported.
These option were used to set a scaling factor by negative powers of
ten but now **--proj.scale** sets the scaling factor directly (the previous value
**5** now corresponds to **1e-5**)
- default *detector-gap* for [actuated traffic
lights](../Simulation/Traffic_Lights.md#based_on_time_gaps) is
now 2.0s. #3340
- default *minGapLat* value (used by the [sublane
model](../Simulation/SublaneModel.md) is now 0.6m (down from
1.0m) to better match observations.
- Documentation
  - Added description of [automatically generated traffic light
    programs](../Simulation/Traffic_Lights.md#automatically_generated_tls-programs).
  - Added Tutorial for [importing public transport data from
    OSM](../Tutorials/PT_from_OpenStreetMap.md).
  - Extended page on [Safety-related
    topics](../Simulation/Safety.md)
  - Added overview page for
    [Geo-Coordinates](../Geo-Coordinates.md)
- TraCI
  - TraCI version is now 16