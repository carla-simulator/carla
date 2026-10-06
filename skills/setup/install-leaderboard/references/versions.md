# Leaderboard 1.0 vs 2.0 vs 2.1 — what actually differs

Read off `carla-simulator/leaderboard` and `carla-simulator/scenario_runner`, and
cross-checked against leaderboard.carla.org.

## Branch map

| Repo | LB 1.0 | LB 2.0 | LB 2.1 | `master` |
|---|---|---|---|---|
| `leaderboard` | `leaderboard-1.0` (`2165f5e`, 2023-06-15) | `leaderboard-2.0` (`a87a341`, 2024-04-26) | `leaderboard-2.1` (`cfecdc8`, 2025-03-06) | `aec8131`, 2025-03-27 — **the 2.0 line** |
| `scenario_runner` | `leaderboard-1.0` (`b9c342e`) | `leaderboard-2.0` (`d7bcaf0`) | `leaderboard-2.1` (`d7bcaf0` — same commit) | not compatible |

`git diff origin/leaderboard-2.0 origin/leaderboard-2.1` on **scenario_runner** is
empty: the two branches are the same commit. On **leaderboard** it touches one
file, `leaderboard/utils/statistics_manager.py`, 38 lines each way.

`leaderboard` `master` differs from `leaderboard-2.0` by a spelling fix in that
file plus `scripts/route_bridge.py`, `leaderboard/autoagents/log_agent.py` and a
newer `sensor_interface.py`. Its scoring is 2.0's. Do not use it to reproduce
live scores.

## The 2.0 → 2.1 change: infraction penalty

Both compute `driving_score = route_completion × infraction_penalty`.

**2.0 — multiplicative (geometric).** Each infraction multiplies the penalty:

```
P = Π_j  p_j ^ (#infractions_j)
```

| Infraction | `p_j` |
|---|---|
| collision with pedestrian | 0.50 |
| collision with vehicle | 0.60 |
| collision with static object | 0.65 |
| red light | 0.70 |
| scenario timeout | 0.70 |
| failure to yield to emergency vehicle | 0.70 |
| stop sign | 0.80 |
| min-speed infraction | up to 0.70, scaled: `1-(1-0.7)(1-pct/100)` |
| outside route lanes | coefficient 0 in code — the route-completion side absorbs it |

**2.1 — additive (linear).** Infractions are summed, then inverted once:

```
P = 1 / (1 + Σ_j  c_j × #infractions_j)
```

| Infraction | `c_j` |
|---|---|
| collision with pedestrian | 1.00 |
| collision with vehicle | 0.70 |
| collision with static object | 0.60 |
| red light | 0.40 |
| scenario timeout | 0.40 |
| failure to yield to emergency vehicle | 0.40 |
| stop sign | 0.25 |
| min-speed infraction | `0.40 × (1 - pct/100)` |
| outside route lanes | `P *= (1 - pct/100)` — applied separately, before the inversion |

Why it changed: under a product, each extra infraction costs a *fraction of what
is left*, so a run that has already collided a few times is nearly free to keep
colliding — which rewarded stopping early. Under `1/(1+Σ)` every infraction costs
the same absolute amount, so continuing to drive badly keeps costing.

Practical consequence: **the same recorded run scores differently.** One
pedestrian collision on a fully completed route gives 50.0 under 2.0 and 50.0
under 2.1 (coincidentally); two gives 25.0 under 2.0 but 33.3 under 2.1; five
gives 3.1 under 2.0 and 16.7 under 2.1. 2.1 is harsher on the first infraction
and much more forgiving on the tail.

`infractions` in `results.json` are recorded identically in both, so a 2.0 result
file can be rescored as 2.1 without re-running — see [[read-leaderboard-results]].

## Tracks and sensor budget

| | 1.0 | 2.0 / 2.1 |
|---|---|---|
| Tracks | `SENSORS`, `MAP` | `SENSORS`, `MAP`, `SENSORS_QUALIFIER`, `MAP_QUALIFIER` |
| RGB cameras | 4 | 8 (qualifier: 4) |
| Lidar | 1 | 2 (qualifier: 1) |
| Radar | 2 | 4 (qualifier: 2) |
| GNSS / IMU / speedometer / opendrive_map | 1 each | 1 each |
| Max sensor radius from the ego origin | 3.0 m | 3.0 m |

The qualifier tracks reuse 1.0's budget. `sensor.opendrive_map` is rejected on the
`SENSORS` tracks — that is the whole difference between SENSORS and MAP.

Sensor attributes are **fixed by the harness**, not by the agent: lidar is always
64 channels / 85 m / 10 Hz / 600k pts, radar 1500 pts / 100 m, and GNSS+IMU noise
is hard-coded. Only cameras take `width`/`height`/`fov` from the agent, plus every
sensor's mount transform.

## Agent API break (1.0 → 2.x)

```python
# 1.0 — the base class calls setup() for you
class AutonomousAgent:
    def __init__(self, path_to_conf_file):
        ...
        self.setup(path_to_conf_file)

# 2.x — the evaluator constructs then calls setup()
class AutonomousAgent:
    def __init__(self, carla_host, carla_port, debug=False):
        ...
```

A 1.0 agent instantiated by a 2.x evaluator receives a host string where it
expects a config path. Any agent that overrides `__init__` must be ported.

## Routes and scenarios

**1.0** — two files. `routes_*.xml` holds only geometry:

```xml
<route id="0" town="Town01">
  <waypoint pitch="360.0" roll="0.0" x="338.70" y="226.75" yaw="269.98" z="0.0" />
</route>
```

Scenario triggers live separately in `data/all_towns_traffic_scenarios_public.json`,
passed as `--scenarios`.

**2.x** — one self-contained file per route set:

```xml
<route id="0" town="Town12">
  <weathers>
    <weather route_percentage="0" cloudiness="5.0" ... sun_altitude_angle="90.0"/>
  </weathers>
  <waypoints>
    <position x="983.5" y="5382.2" z="371"/>
  </waypoints>
  <scenarios>
    <scenario name="ParkingExit_1" type="ParkingExit">
      <trigger_point x="983.5" y="5382.2" z="371" yaw="90"/>
    </scenario>
  </scenarios>
</route>
```

Weather now interpolates along the route by `route_percentage`, and scenarios are
per-route rather than per-town. `data/routes_training.xml` carries 90 routes and
4629 scenario instances across 38 types, all in Town12; `routes_validation.xml`
is 20 routes in Town13; `routes_devtest.xml` is 2 routes for smoke tests.

Parked vehicles were added in 2.x (`leaderboard/utils/parked_vehicles.py`) and are
placed from a table of candidate positions, not from the route file.

## Evaluator CLI (2.x)

```
--routes FILE --routes-subset '0-2,5' --repetitions N
--track SENSORS|MAP|SENSORS_QUALIFIER|MAP_QUALIFIER
--agent FILE --agent-config FILE
--checkpoint results.json --debug-checkpoint live_results.txt
--debug 0|1|2  --record PATH  --resume  --timeout 300
--host --port --traffic-manager-port --traffic-manager-seed
```

1.0 additionally requires `--scenarios FILE`. `--routes-subset` exists in both.

Fixed by the harness in 2.x: 20 Hz synchronous mode, `deterministic_ragdolls`,
`spectator_as_ego=False`, TM hybrid physics on, `tile_stream_distance` and
`actor_active_distance` forced to 650 m (large-map streaming), and all traffic
lights reset per route.

## Known rough edges

- `run_leaderboard.sh` (master/2.x) passes `--debug-checkpoint=${DEBUG_CHECKPOINT_ENDPOINT}`
  but never exports that variable, so it lands as the empty string and live
  results go nowhere. Export it yourself.
- `scripts/make_docker.sh` copies `${CARLA_ROOT}/PythonAPI` and renames the eggs
  to `carla-leaderboard-py3x.egg`; it fails on a CARLA tree with no `dist/*.egg`
  (e.g. a wheel-only install).
- `Dockerfile.master` still creates a conda env with Python 3.7 and
  `numpy networkx scipy six requests` — fine, but it does not install the pinned
  requirements into that env, it pip-installs into the system Python.
