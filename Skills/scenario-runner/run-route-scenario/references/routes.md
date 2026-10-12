# Routes: formats, criteria, and how this relates to the Leaderboard

## Route file formats

**2.x (current)** — self-contained. Parsed by `srunner/tools/route_parser.py`.

```xml
<routes>
  <route id="0" town="Town12">
    <weathers>
      <weather route_percentage="0"   cloudiness="5.0" precipitation="0.0"
               precipitation_deposits="0.0" wetness="0.0" wind_intensity="10.0"
               sun_azimuth_angle="-1.0" sun_altitude_angle="90.0" fog_density="2.0"/>
      <weather route_percentage="100" ... sun_altitude_angle="15.0" .../>
    </weathers>
    <waypoints>
      <position x="983.5" y="5382.2" z="371"/>
      ...
    </waypoints>
    <scenarios>
      <scenario name="ParkingExit_1" type="ParkingExit">
        <trigger_point x="983.5" y="5382.2" z="371" yaw="90"/>
        <!-- scenario-specific parameters go here, e.g. -->
        <flow_speed value="5"/>
        <source_dist_interval from="10" to="60"/>
      </scenario>
    </scenarios>
  </route>
</routes>
```

- `<position>` are **coarse** waypoints. The route is densified between them by
  `GlobalRoutePlanner`, so they only need to disambiguate the path, not describe it.
- `route_percentage` interpolates weather along the route — two entries give a
  dusk transition, more give a profile.
- Scenario order in the file is cosmetic; triggering is by position. The
  leaderboard's `scenario_orderer.py` sorts them for readability.

**1.0 (legacy)** — geometry only, two files:

```xml
<route id="0" town="Town01">
  <waypoint pitch="360.0" roll="0.0" x="338.70" y="226.75" yaw="269.98" z="0.0"/>
</route>
```

Scenario triggers live in `data/all_towns_traffic_scenarios_public.json`, passed as
`--scenarios`. `scenario_runner.py` on `master`/`ue5-master` has no `--scenarios`
flag, so 1.0 route files are only usable on the `leaderboard-1.0` branch.

## What `RouteScenario` builds

`srunner/scenarios/route_scenario.py`:

1. parses the route and densifies it,
2. spawns the ego at the first waypoint,
3. instantiates each `<scenario>` whose `type` resolves to a class, wrapping it in
   a trigger on the ego reaching its `trigger_point`,
4. adds `BackgroundActivity` — Traffic-Manager traffic that follows the ego rather
   than filling the whole map, tunable per route via
   `BackgroundActivityParametrizer` scenarios,
5. adds route-level criteria.

Unknown scenario types are **skipped without error**. That is by design (route
files are shared across branches) and is the usual cause of "the ego drives the
route and nothing ever happens". `run_route.sh list` cross-checks every type
against the classes in the checkout.

## Route criteria

| Criterion | Meaning |
|---|---|
| `RouteCompletionTest` | percentage of route length covered |
| `InRouteTest` | fails if the ego leaves the corridor around the route |
| `OutsideRouteLanesTest` | percentage of distance driven on the wrong lane/sidewalk |
| `CollisionTest` | any collision, categorised into vehicle / pedestrian / static |
| `RunningRedLightTest`, `RunningStopTest` | traffic-control violations |
| `ActorBlockedTest` | ego stationary too long |
| `MinimumSpeedRouteTest` | ego consistently slower than surrounding traffic |
| `ScenarioTimeoutTest` | a scenario ran past its own deadline |
| `YieldToEmergencyVehicleTest` | failed to yield |

These are exactly the events the Leaderboard turns into infractions and a driving
score. ScenarioRunner reports them pass/fail; it does not weight them.

## ScenarioRunner route mode vs the Leaderboard

| | `scenario_runner.py --route` | `leaderboard_evaluator.py` |
|---|---|---|
| Route selection | `--route-id` (one id, or all) | `--routes-subset '0-4,7'` |
| Repetitions | `--repetitions` (same seed) | `--repetitions` (new TM seed per rep) |
| Sensors | whatever the agent spawns | validated against a per-track budget |
| Agent API | `AutonomousAgent` | same class, plus `Track`, sensor validation, watchdogs |
| Output | criteria pass/fail | `results.json` with driving score + infractions |
| World settings | sync 20 Hz | sync 20 Hz + deterministic ragdolls, hybrid TM, 650 m streaming |
| Parked vehicles | no | yes (from `parked_vehicles.py`) |
| Resume | no | `--resume` |
| Timeouts | one client timeout | per-route + per-agent-tick watchdogs |

Use route mode to develop and debug — it is faster to start, easier to interrupt
and prints the behaviour tree with `--debug`. Switch to the Leaderboard when you
want a score that means something.

## Forced settings

In `scenario_runner.py`'s `main()`:

```python
if arguments.route:
    arguments.reloadWorld = True
if arguments.agent:
    arguments.sync = True
```

So a route run with an agent always reloads the world and always runs
synchronously. Two consequences worth planning for: anything you spawned before
the run is destroyed by the reload, and an interrupted run leaves the world
synchronous.

## Large maps

Town12/Town13 stream tiles. The Leaderboard sets

```python
settings.tile_stream_distance   = 650
settings.actor_active_distance  = 650
```

after every `load_world`, because CARLA resets large-map settings on load.
ScenarioRunner's route mode does **not** do this, so a Town12 route can behave
differently there — actors freezing at distance, or slow tile loads. Apply the
same values with [[set-world-settings]] if you are comparing against leaderboard
runs.

## Writing your own route

Minimal single-scenario route for testing one scenario type:

```xml
<routes>
  <route id="0" town="Town04">
    <weathers>
      <weather route_percentage="0" cloudiness="5.0" sun_altitude_angle="70.0"/>
    </weathers>
    <waypoints>
      <position x="-25.0" y="-240.0" z="0.2"/>
      <position x="200.0" y="-240.0" z="0.2"/>
    </waypoints>
    <scenarios>
      <scenario name="Accident_1" type="Accident">
        <trigger_point x="0.0" y="-240.0" z="0.2" yaw="0"/>
        <distance value="80"/>
      </scenario>
    </scenarios>
  </route>
</routes>
```

The parameter elements a scenario accepts are read in its `_initialize_actors` /
`__init__` from `config.other_parameters`; grep the class for
`other_parameters` to see the names and defaults. The interactive way is
[[create-leaderboard-route]], which places waypoints and trigger points from the
spectator camera.
