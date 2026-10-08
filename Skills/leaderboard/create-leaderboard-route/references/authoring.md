# Route authoring reference

## Elements

```xml
<routes>
  <route id="0" town="Town12">
    <weathers>
      <weather route_percentage="0"
               cloudiness="5.0" precipitation="0.0" precipitation_deposits="0.0"
               wetness="0.0" wind_intensity="10.0"
               sun_azimuth_angle="-1.0" sun_altitude_angle="90.0" fog_density="2.0"/>
    </weathers>
    <waypoints>
      <position x="983.5" y="5382.2" z="371"/>
    </waypoints>
    <scenarios>
      <scenario name="ParkingExit_1" type="ParkingExit">
        <trigger_point x="983.5" y="5382.2" z="371" yaw="90"/>
        <flow_speed value="5"/>
        <source_dist_interval from="10" to="60"/>
      </scenario>
    </scenarios>
  </route>
</routes>
```

| Element | Notes |
|---|---|
| `route@id` | referenced in results as `RouteScenario_<id>`. `--routes-subset` uses the **position** in the file, not this id — keep them sequential from 0 so they coincide |
| `route@town` | must exist on the server; Town12/Town13 need AdditionalMaps or the leaderboard build |
| `weather@route_percentage` | 0–100; values interpolate along the route. One entry = constant, two = a transition |
| `waypoints/position` | coarse keypoints. `GlobalRoutePlanner` densifies between them, so they only need to disambiguate the path |
| `scenario@name` | must be unique within the route; appears in results and logs |
| `scenario@type` | must match a class in the paired `scenario_runner`. Unknown types are **skipped silently** |
| `trigger_point` | where the ego arms the scenario; `yaw` sets the expected heading |
| any other child | scenario parameters, read from `config.other_parameters` as **strings** |

Weather attributes are the `carla.WeatherParameters` fields; anything omitted keeps
the town default. There is no `sun_altitude_angle` shortcut for "night" — use a
negative value.

## Scenario parameters

Each scenario class reads its own set. To find them, grep the class:

```bash
grep -n "other_parameters" $SCENARIO_RUNNER_ROOT/srunner/scenarios/route_obstacles.py
```

Common ones across the Leaderboard 2.x scenarios:

| Parameter | Meaning |
|---|---|
| `distance` | how far ahead of the trigger the obstacle/actor is placed |
| `direction` | `left` / `right` |
| `flow_speed` | speed of an actor flow, m/s |
| `source_dist_interval` | `from`/`to` spacing between spawned flow actors |
| `speed` | target speed of the adversary |
| `crossing_angle` | for pedestrian/cyclist crossings |
| `frequency` | `from`/`to` for repeated events (e.g. doors opening) |
| `timeout` | scenario-specific deadline feeding `ScenarioTimeoutTest` |

## The shipped scripts

All in `$LEADERBOARD_ROOT/scripts/`, all needing a running server on the route's
town.

| Script | Key flags | What it does |
|---|---|---|
| `route_creator.py` | `-f FILE...` `--host` `--port` | fly the spectator, record keypoints into a route |
| `scenario_creator.py` | `-f FILE...` `-s/--show-only` | place scenario trigger points along an existing route |
| `route_displayer.py` | `-f FILE` `-sr ID` `-sa` `-sk` `-ss` | draw route / all routes / keypoints / scenarios in the world |
| `route_summarizer.py` | `-f FILE` `--endpoint OUT` `--show` | route file → table |
| `scenario_orderer.py` | `-f FILE` | sort the `<scenarios>` block by position along the route |
| `weather_creator.py` | `-r ROUTE` | print the current simulation weather as a `<weather>` element |
| `route_bridge.py` | `-r ROUTES` `-s SCENARIOS` `-e ENDPOINT` | 1.0 route + scenario JSON → 2.x route file. **`master` only** |
| `manage_scenarios.py` | `--draw-scenarios` `--validate-scenarios` `--create-junction-scenarios` `--load-town` | LB **1.0** scenario JSON tooling |

`route_creator` and `scenario_creator` take `-f` with `nargs="+"`, so they can work
across several files at once.

`manage_scenarios.py` defaults `-f` to
`../data/all_towns_traffic_scenarios_public.json`, i.e. the LB 1.0 format — it is
not useful for 2.x routes.

## Practical workflow

1. **Load the town.** None of the tools do it for you.
2. **`route_creator`** — place 3–5 keypoints for a test route; dozens for a real one.
   Keypoints only need to force the intended path through junctions.
3. **`scenario_creator`** — position the spectator where the ego should be when the
   scenario arms, and record. The trigger's `yaw` comes from the spectator heading,
   so face along the direction of travel.
4. **`check`** (this skill) — offline structural validation.
5. **`scenario_orderer`** — sort for readability before committing the file.
6. **Run with `npc_agent.py`.** A route the NPC agent cannot finish is a broken
   route. This step catches wrong-way starts, unreachable endpoints and triggers in
   the wrong lane.
7. **`route_displayer -ss`** while it runs, to see triggers relative to the path.

## Why triggers "never fire"

In order of frequency:

1. **Branch mismatch** — the type has no class in this `scenario_runner`;
   `RouteScenario` skips it without a message.
2. **Trigger in the wrong lane** — laterally close but on the opposite carriageway.
   `display --scenarios` shows this immediately.
3. **Trigger beyond the route end** — the route finishes before the ego arrives.
4. **Two scenarios overlapping** — one grabs the background traffic the other needs.
   `scenario_orderer` makes the overlap visible.

The keypoint polyline is only a hint, so a trigger being far from it is weak
evidence: across the 6534 scenarios in the shipped Town12/Town13 files the offset is
24 m at the median, 224 m at p95 and up to 1242 m. `check` only flags gross
outliers for that reason.
