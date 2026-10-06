# Atomic behaviours, trigger conditions and criteria

The building blocks of a scenario's behaviour tree. All are py_trees 0.8 nodes; a
scenario is a tree of `py_trees.composites.Sequence` / `Parallel` with these as
leaves. Extracted from the `master` branch — the same names exist on `ue5-master`.

## Behaviours — `srunner/scenariomanager/scenarioatomics/atomic_behaviors.py`

**Motion of a single actor**

| Behaviour | What it does |
|---|---|
| `WaypointFollower` | drives an actor along a plan (or its lane) at a target speed; the workhorse |
| `LaneChange` | lane change with a given speed and distance in the new lane |
| `CutIn` | lane change *into* the ego's lane from left/right |
| `AccelerateToVelocity`, `DecelerateToVelocity`, `KeepVelocity` | longitudinal control |
| `UniformAcceleration`, `ChangeTargetSpeed`, `ChangeActorTargetSpeed` | speed profiles |
| `AccelerateToCatchUp` | close a gap to another actor |
| `StopVehicle`, `HandBrakeVehicle`, `SetInitSpeed` | discrete state changes |
| `KeepLongitudinalGap` | maintain a gap |
| `ChangeActorWaypoints`, `ChangeActorWaypointsToReachPosition` | replace an actor's plan |
| `ChangeActorLateralMotion`, `ChangeActorLaneOffset`, `ChangeLateralDistance` | lateral offsets |
| `SyncArrival`, `SyncArrivalWithAgent`, `SyncArrivalOSC` | time an actor to meet the ego |
| `BasicAgentBehavior`, `ConstantVelocityAgentBehavior`, `AdaptiveConstantVelocityAgentBehavior` | hand an actor to `agents.navigation` |
| `ChangeAutoPilot`, `ChangeActorControl`, `UpdateAllActorControls` | switch controller |
| `AddNoiseToVehicle`, `AddNoiseToRouteEgo`, `ChangeNoiseParameters` | control-loss noise |

**Actor lifecycle**

`AddActor`, `ActorDestroy`, `ActorTransformSetter`, `BatchActorTransformSetter`,
`ActorTransformSetterToOSCPosition`, `ActorSource`, `ActorSink`.

`ActorTransformSetter` is how a scenario puts a pre-spawned actor into place at the
right moment — spawn far away in `_initialize_actors`, teleport in when triggered.

**Flows** (continuous streams of traffic — the Leaderboard 2.x scenarios lean on these)

`ActorFlow`, `OppositeActorFlow`, `InvadingActorFlow`, `BicycleFlow`, `WalkerFlow`,
`AIWalkerBehavior`, `MovePedestrianWithEgo`.

**World / infrastructure**

`ChangeWeather`, `ChangeRoadFriction`, `ChangeParameter`,
`TrafficLightStateSetter`, `TrafficLightControllerSetter`, `TrafficLightFreezer`,
`TrafficLightManipulator`, `OpenVehicleDoor`, `StartRecorder`, `StopRecorder`,
`RunScript`.

`ChangeWeather` is a no-op on `ue5-master` (weather behaviours are commented out in
`basic_scenario.py`).

**Control flow**

| Behaviour | Use |
|---|---|
| `Idle(duration)` | wait; with no duration it never finishes |
| `WaitForever` | park a branch of a Parallel |
| `ScenarioTimeout` | fail the scenario after N seconds (feeds `ScenarioTimeoutTest`) |
| `ScenarioTriggerer` | the route-mode gate that arms scenarios by ego position |
| `SwitchWrongDirectionTest`, `SwitchMinSpeedCriteria` | temporarily disable a criterion |

`SwitchWrongDirectionTest(False)` is what lets a scenario legitimately push the ego
into the oncoming lane (overtakes, `*TwoWays` scenarios) without failing
`WrongLaneTest`. Turn it back on afterwards.

## Trigger conditions — `atomic_trigger_conditions.py`

| Condition | Fires when |
|---|---|
| `InTriggerDistanceToVehicle` | two actors within a distance |
| `InTriggerDistanceToLocation` | actor near a world location |
| `InTriggerDistanceToNextIntersection` | actor near the next junction |
| `InTriggerDistanceToLocationAlongRoute` | distance measured **along the route**, not euclidean |
| `InTriggerRegion` | actor inside an axis-aligned box |
| `InTriggerNearCollision` | closing fast at short range |
| `InTimeToArrivalToLocation`, `InTimeToArrivalToVehicle`, `InTimeToArrivalToVehicleSideLane` | TTC-style |
| `DriveDistance` | actor has travelled N m |
| `StandStill` | actor stationary for N s |
| `TriggerVelocity`, `TriggerAcceleration`, `RelativeVelocityToOtherActor` | kinematics thresholds |
| `WaitUntilInFront`, `WaitUntilInFrontPosition` | overtake completion |
| `AtRightmostLane` | lane position |
| `WaitForTrafficLightState`, `WaitForTrafficLightControllerState`, `WaitEndIntersection` | signals |
| `WaitForBlackboardVariable`, `CheckParameter`, `IfTriggerer` | blackboard / parameter gates |
| `TimeOfDayComparison`, `TimeOfWaitComparison`, `OSCStartEndCondition` | OpenSCENARIO support |

Conditions return RUNNING until satisfied, so they are used as the first child of a
`Sequence` or as the trigger side of a `Parallel` with
`SUCCESS_ON_ONE`.

## Criteria — `atomic_criteria.py`

| Criterion | Fails on |
|---|---|
| `CollisionTest` | any collision; splits vehicle / pedestrian / static |
| `DrivenDistanceTest` | less than the expected distance covered |
| `AverageVelocityTest`, `MaxVelocityTest` | speed bounds |
| `KeepLaneTest` | lane invasions |
| `OffRoadTest`, `OnSidewalkTest`, `EndofRoadTest` | leaving the road |
| `WrongLaneTest` | driving against traffic |
| `OutsideRouteLanesTest` | % of route driven outside its lanes |
| `InRouteTest`, `RouteCompletionTest` | route deviation / progress |
| `ReachedRegionTest`, `InRadiusRegionTest` | goal regions |
| `RunningRedLightTest`, `RunningStopTest` | traffic control |
| `ActorBlockedTest` | stationary too long |
| `MinimumSpeedRouteTest` | too slow relative to traffic |
| `YieldToEmergencyVehicleTest` | failure to yield |
| `ScenarioTimeoutTest` | paired `ScenarioTimeout` fired |

Every criterion takes `optional=False` and `terminate_on_failure=False` by default.
`optional=True` records the result without failing the scenario;
`terminate_on_failure=True` stops the run at the first violation.

Criteria raise `TrafficEvent`s, and those events are what the Leaderboard converts
into infractions and a score — so a criterion added here is visible there too.

## Helpers — `srunner/tools/scenario_helper.py`

The geometry you would otherwise write badly:

| Function | Use |
|---|---|
| `get_waypoint_in_distance(wp, d)` | walk forward, handling lane ends |
| `get_location_in_distance(actor, d)`, `get_location_in_distance_from_wp` | same, returning a location |
| `generate_target_waypoint(wp, turn)` | the waypoint after a junction, taking a turn |
| `generate_target_waypoint_list`, `_multilane`, `_in_route` | full plans for `WaypointFollower` |
| `choose_at_junction`, `get_junction_topology`, `filter_junction_wp_direction` | junction reasoning |
| `get_crossing_point`, `get_geometric_linear_intersection` | conflict points |
| `get_same_dir_lanes`, `get_opposite_dir_lanes` | lane sets for flows and overtakes |
| `get_closest_traffic_light` | the light governing a waypoint |
| `get_distance_between_actors`, `get_distance_along_route` | measurement |
| `detect_lane_obstacle` | is there something in the way |

`srunner/tools/background_manager.py` is the interface to `BackgroundActivity` from
inside a scenario — `ChangeOppositeBehavior`, `RemoveRoadLane`, `StopFrontVehicles`
and friends let a route scenario clear or redirect the ambient traffic for its
duration. That is how the `*TwoWays` scenarios make the oncoming lane usable.

## Reading order for a new scenario

1. `srunner/scenarios/basic_scenario.py` — the lifecycle and what the base class
   builds for you.
2. `srunner/scenarios/follow_leading_vehicle.py` — the simplest complete example.
3. `srunner/scenarios/cut_in.py` — parameters from XML, `ActorTransformSetter`.
4. `srunner/scenarios/route_obstacles.py` — route-mode scenario with
   `BackgroundManager` interaction and `SwitchWrongDirectionTest`.
