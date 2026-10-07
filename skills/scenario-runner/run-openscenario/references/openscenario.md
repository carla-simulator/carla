# OpenSCENARIO in ScenarioRunner

## The schema actually shipped

`srunner/openscenario/0.9.x/` — `OpenSCENARIO_v0.9.1.xsd`,
`OpenSCENARIO_Catalog.xsd`, `OpenSCENARIO_TypeDefs.xsd`, plus a
`migration0_9_1to1_0.xslt`. Validation runs through `xmlschema==1.0.18` (pinned;
newer xmlschema rejects these files).

The directory name says 0.9.x but the parser implements a good part of
OpenSCENARIO **1.0**. Files written against 1.1/1.2/1.3 will validate-fail on the
newer elements. There is no way to select a different schema revision.

## Actions the parser handles

From `srunner/tools/openscenario_parser.py`:

*Private / entity actions*
`TeleportAction`, `SpeedAction`, `LongitudinalDistanceAction`,
`LaneChangeAction`, `LaneOffsetAction`, `LateralDistanceAction`,
`FollowTrajectoryAction` (Polyline, Clothoid, Nurbs), `AcquirePositionAction`,
`AssignRouteAction`, `ActivateControllerAction`, `AssignControllerAction`,
`OverrideControllerValueAction`, `VisibilityAction`, `SynchronizeAction`,
`AddEntityAction`, `DeleteEntityAction`, `CustomCommandAction`,
`UserDefinedAction`

*Global actions*
`EnvironmentAction` (weather, time of day, road friction),
`InfrastructureAction` → `TrafficSignalAction` / `TrafficSignalStateAction` /
`TrafficSignalControllerAction`, `ParameterAction` (Set / Modify)

## Conditions the parser handles

*Entity conditions*
`EndOfRoadCondition`, `CollisionCondition`, `OffroadCondition`,
`TimeHeadwayCondition`, `TimeToCollisionCondition`, `AccelerationCondition`,
`StandStillCondition`, `SpeedCondition`, `RelativeSpeedCondition`,
`TraveledDistanceCondition`, `ReachPositionCondition`, `DistanceCondition`,
`RelativeDistanceCondition`

*Value conditions*
`ParameterCondition`, `SimulationTimeCondition`, `TimeOfDayCondition`,
`StoryboardElementStateCondition`, `UserDefinedValueCondition`,
`TrafficSignalCondition`, `TrafficSignalControllerCondition`

## Positions

`WorldPosition`, `RelativeWorldPosition`, `RelativeObjectPosition`,
`RoadPosition`, `RelativeRoadPosition`, `LanePosition`, `RelativeLanePosition`,
`RoutePosition`.

## Controllers

CARLA-specific controllers live in
`srunner/scenariomanager/actorcontrols/`:

| Controller | Behaviour |
|---|---|
| `simple_vehicle_control` | direct target-speed/waypoint following, ignores traffic |
| `npc_vehicle_control` | hands the actor to the Traffic Manager |
| `vehicle_longitudinal_control` | longitudinal only, PID |
| `pedestrian_control` | walker control |
| `external_control` | the actor is driven from outside (manual_control, an agent) |
| `visualizer` | debug drawing only |

Select one with `<ObjectController><CatalogReference>` /
`<Controller><Properties><Property name="module" value="..."/>`. See
`srunner/examples/OscControllerExample.xosc` and
`srunner/examples/catalogs/ControllerCatalog.xosc`.

An entity with `external_control` will not move unless you drive it —
`manual_control.py -a --rolename=hero` or a route agent.

## Known gaps

- No support for OpenSCENARIO 1.1+ additions (`Wind`, `DomeImage`,
  `AnimationAction`, `LightStateAction`, variables/`VariableDeclarations`).
- Story parameters cannot be overridden from the CLI; only global
  `ParameterDeclarations` via `--openscenarioparams`.
- Unimplemented actions and conditions are frequently *skipped* rather than
  raising, which is why a file can load and then do nothing.
- Timeout is hard-coded to `100000` s in `scenario_runner.py`, so a stalled OSC run
  hangs rather than failing.
- `.xosc`/`.osc` examples moved to `srunner/osc_examples/` on `ue5-master`.

## OpenSCENARIO 2.0 (`.osc`)

A separate pipeline under `srunner/osc2/`:

```
srunner/osc2/osc2_parser/     ANTLR4-generated lexer/parser (pinned to antlr4 4.10)
srunner/osc2/ast_manager/     AST builder, listener, visitor
srunner/osc2/symbol_manager/  scopes and symbol tables
srunner/osc2/error_manager/   diagnostics
srunner/osc2/osc_preprocess/  import resolution
srunner/osc2_stdlib/          modifiers, paths, vehicle types
srunner/osc2_dm/              physical types and units
srunner/scenarios/osc2_scenario.py   turns the AST into a py_trees tree
```

Bundled examples: `basic.osc`, `acceleration.osc`, `change_lane.osc`,
`change_speed.osc`, `keep_lane.osc`, `overspeed.osc`, `overtake1.osc`,
`overtake_concrete.osc`, `cut_in_and_slow_{range,right,single_over_junction}.osc`,
`follow_trajectory.osc`, `force_over_signal.osc`, `emit.osc`, `one_of.osc`,
`method_invocation.osc`.

The test corpus in `tests/testcases/` and `tests/testcases1/` is the practical
statement of what the grammar accepts; `tests/test-ast.py`,
`test-ast-listener.py`, `test-ast-visitor.py` and `test-symbol.py` exercise it
without a simulator, which makes them a cheap way to check an install:

```bash
cd "$SCENARIO_RUNNER_ROOT" && python3 tests/test-ast.py
```

`Docs/README_OpenSCENARIO_2.0.md` in the checkout is the language-side reference.
It is one of the few SR docs that tracks the code.

## Practical validation order

1. `xmllint --noout file.xosc` — is it even well-formed XML?
2. `validate` in this skill — schema, map, entities, parameters.
3. Run with `DEBUG=1` — the py_trees tree printed each tick shows which
   `Act`/`Maneuver` is active, and an action that was silently skipped simply
   never appears.
