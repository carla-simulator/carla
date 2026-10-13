# Scenario inventory

Generated from the repository, not from the docs. Regenerate at any time with
`python3 scripts/list_scenarios.py --types` and `--check`.

## Two kinds of scenario

**Standalone** — has an XML config in `srunner/examples/` giving it a town, an ego
spawn point and parameters. Launch with `--scenario <config name>`.

**Route-only** — a scenario class with no standalone config. It takes its trigger
point from a route file's `<scenarios>` block and only runs inside `--route`.
Most of the library is in this group: these are the Leaderboard 2.0 scenarios,
which are positioned along Town12/Town13 routes rather than at fixed spawn points.

## master — 21 standalone types, 119 configs

```
TYPE                              CONFIGS  TOWNS
Accident                                1  Town04
ChangeLane                              2  Town01,Town04
ConstructionObstacle                    2  Town02,Town04
ControlLoss                            15  Town01..Town05
CutIn                                   2  Town04
DynamicObjectCrossing                   9  Town02..Town05
EnterActorFlow                          1  Town04
FollowLeadingVehicle                   11  Town01..Town05
FollowLeadingVehicleWithObstacle       11  Town01..Town05
FreeRide                                6  Town01..Town04
HighwayCutIn                            1  Highway      <-- BROKEN, no such map
ManeuverOppositeDirection               4  Town03..Town05
NoSignalJunctionCrossing                1  Town03
OppositeVehicleRunningRedLight          5  Town03,Town04
OtherLeadingVehicle                    10  Town04,Town05
SignalizedJunctionLeftTurn              6  Town03..Town05
SignalizedJunctionRightTurn             7  Town03..Town05
StationaryObjectCrossing                8  Town02..Town05
VehicleOpensDoorTwoWays                 1  Town10HD_Opt
VehicleTurningLeft                      8  Town01..Town04
VehicleTurningRight                     8  Town01..Town04
```

Config names are `<Type>_<n>` (`FollowLeadingVehicle_1`), except
`VehicleTurning*` and `CutIn`, whose names read
`VehicleTurningRight_1` / `CutInFrom_left_Lane`. `group:<Type>` runs all configs
of a type in sequence, reloading each one's map.

### What the scenarios do

| Type | Behaviour |
|---|---|
| `FollowLeadingVehicle` | leader slows and stops ahead of the ego; ego must not rear-end it |
| `FollowLeadingVehicleWithObstacle` | as above, with a hidden obstacle in front of the leader |
| `OtherLeadingVehicle` | leader decelerates; ego is expected to change lane and pass |
| `ChangeLane` | slow/stopped vehicle in the ego's lane on a highway |
| `CutIn` | a vehicle cuts into the ego's lane from left or right |
| `HighwayCutIn` | merge-lane vehicle forces itself in front of the ego |
| `ControlLoss` | friction/jitter patches make the ego lose control; it must recover |
| `StationaryObjectCrossing` | cyclist standing still in the ego's lane |
| `DynamicObjectCrossing` | pedestrian/cyclist runs out from behind an occlusion |
| `VehicleTurningRight` / `Left` | cyclist crosses as the ego turns out of a junction |
| `OppositeVehicleRunningRedLight` | crossing vehicle runs its red light at a junction |
| `SignalizedJunctionLeftTurn` | ego turns left across an oncoming flow at lights |
| `SignalizedJunctionRightTurn` | ego turns right into a lateral flow at lights |
| `NoSignalJunctionCrossing` | unsignalised junction negotiation with one other vehicle |
| `ManeuverOppositeDirection` | ego overtakes using the oncoming lane |
| `Accident` | crashed vehicles block a lane; ego must route around |
| `ConstructionObstacle` | cones/works block a lane |
| `EnterActorFlow` | ego must merge into a continuous stream of traffic |
| `ParkingExit` (ue5) | ego pulls out of a parking bay into moving traffic |
| `VehicleOpensDoorTwoWays` | parked car opens a door into the ego's path |
| `FreeRide` | no adversary — free driving, useful for smoke tests and multi-ego |

`FreeRide` is the one to reach for when checking that the whole stack runs: it has
`rolename="hero"` egos and no criteria that can fail early. `MultiEgo_1` and
`MultiEgo_2` in `FreeRide.xml` are the only multi-ego configs (`hero` + `hero2`).

## master — 34 route-only classes

```
AccidentTwoWays              HazardAtSideLane             ParkedObstacleTwoWays
BackgroundActivityParametrizer  HazardAtSideLaneTwoWays   ParkingCrossingPedestrian
BaseVehicleTurning           HighwayExit                  ParkingCutIn
BlockedIntersection          InterurbanActorFlow          ParkingExit
ConstructionObstacleTwoWays  InterurbanAdvancedActorFlow  PedestrianCrossing
CrossingBicycleFlow          InvadingTurn                 PriorityAtJunction
EnterActorFlowV2             JunctionLeftTurn             StaticCutIn
HardBreakRoute               JunctionRightTurn            VehicleTurningRoute
                             MergerIntoSlowTraffic        VehicleTurningRoutePedestrian
                             MergerIntoSlowTrafficV2      YieldToEmergencyVehicle
                             NoSignalJunctionCrossingRoute
                             NonSignalizedJunctionLeftTurn
                             NonSignalizedJunctionRightTurn
                             OppositeVehicleJunction
                             OppositeVehicleTakingPriority
```

These are exactly the types that appear in the Leaderboard 2.x route files. Their
frequency in `leaderboard/data/routes_training.xml` (4629 instances, 38 types) is
a good guide to what matters: `HardBreakRoute` 573, `ControlLoss` 429,
`DynamicObjectCrossing` 351, `PriorityAtJunction` 237, `ParkedObstacleTwoWays` 204,
`HazardAtSideLaneTwoWays` 177, then a long tail.

## ue5-master — the port is partial

101 configs, of which **11** name `Town10HD_Opt`, the only map CARLA 0.10.0 ships:

```
BlockedIntersection_1  BlockedIntersection_2
ConstructionObstacle_1 ConstructionObstacle_2
ControlLoss_1
OppositeVehicleRunningRedLight_1
ParkingExit_1          ParkingExit_2
SignalizedJunctionLeftTurn_1
StaticCutIn_1
VehicleOpensDoorTwoWays_1
```

The other 90 still name Town01–Town05 and cannot load. Five configs are outright
broken: `ChangeLane_1`, `ChangeLane_2`, `CutInFrom_left_Lane`,
`CutInFrom_right_Lane` (classes deleted) and `HighwayCutIn_1` (bad town).
`list_scenarios.py --check` lists them.

Also on `ue5-master`: ego blueprint is `vehicle.lincoln.mkz`, and
`_create_weather_behavior()` / `world.set_weather()` are commented out in
`basic_scenario.py`, so `<weather>` in a config does nothing.

## Criteria (what pass/fail means)

Every scenario adds criteria from
`srunner/scenariomanager/scenarioatomics/atomic_criteria.py`. The usual set:

| Criterion | Fails when |
|---|---|
| `CollisionTest` | the ego touches anything |
| `DrivenDistanceTest` | the ego covers less than the expected distance |
| `KeepLaneTest` | lane invasions |
| `RunningRedLightTest` / `RunningStopTest` | traffic-control violations |
| `WrongLaneTest` / `OnSidewalkTest` / `OutsideRouteLanesTest` | off-lane driving |
| `InRouteTest` / `RouteCompletionTest` | route deviation / progress (route mode) |
| `ActorSpeedAboveThresholdTest` | the ego is blocked (stationary too long) |
| `MinimumSpeedRouteTest` | the ego drives too slowly relative to traffic |

`--output` prints them as a table; `--json` writes the same data machine-readably.
Interpreting the report is [[analyze-scenario-results]].

## Towns and what they are for

| Town | Character | Typical scenarios |
|---|---|---|
| Town01 / Town02 | small grid, no highway | FollowLeadingVehicle, VehicleTurning |
| Town03 | large urban, roundabout, tunnel | junctions, NoSignalJunction |
| Town04 | highway ring + small town | ChangeLane, CutIn, Accident, actor flows |
| Town05 | grid with multi-lane roads | SignalizedJunction*, OtherLeadingVehicle |
| Town06 | long highways, many lanes | merges (route mode) |
| Town07 | rural | — |
| Town10HD_Opt | dense downtown | VehicleOpensDoor, all ue5 configs |
| Town12 / Town13 | large maps | Leaderboard 2.x routes only |

Large maps (Town11–Town13) need `AdditionalMaps` on a release build, and the
Leaderboard forces `tile_stream_distance`/`actor_active_distance` to 650 m for
them.
