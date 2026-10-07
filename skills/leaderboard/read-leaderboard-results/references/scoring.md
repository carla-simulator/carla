# Leaderboard scoring and the results.json schema

## The formula

```
driving_score_i = route_completion_i × infraction_penalty_i        (capped at 100)
global driving score = arithmetic mean over routes
```

`leaderboard/utils/statistics_manager.py` is the authority.

### Leaderboard 2.0 (and `master`) — multiplicative

```python
score_penalty = 1.0
for event in criteria_events:
    if event.type in PENALTY_VALUE_DICT:
        score_penalty *= PENALTY_VALUE_DICT[event.type]
    elif event.type in PENALTY_PERC_DICT:
        penalty_value, kind = PENALTY_PERC_DICT[event.type]
        if kind == "decreases":   # ideal is 100
            score_penalty *= (1 - (1 - penalty_value) * (1 - pct / 100))
        elif kind == "increases": # ideal is 0
            score_penalty *= (1 - (1 - penalty_value) * pct / 100)
```

| Infraction | coefficient |
|---|---|
| collision with pedestrian | 0.50 |
| collision with vehicle | 0.60 |
| collision with static layout | 0.65 |
| red light | 0.70 |
| scenario timeout | 0.70 |
| yield to emergency vehicle | 0.70 |
| stop sign | 0.80 |
| min speed | `[0.70, 'decreases']` |
| outside route lanes | `[0, 'increases']` — coefficient 0, i.e. absorbed by route completion |

### Leaderboard 2.1 — additive

```python
infraction_value = 0
for event in criteria_events:
    if event.type in PENALTY_VALUE_DICT:
        if event.type == MIN_SPEED_INFRACTION:
            value = 0.4 * (1 - pct / 100)
        else:
            value = PENALTY_VALUE_DICT[event.type]
        infraction_value += value
    elif event.type == OUTSIDE_ROUTE_LANES_INFRACTION:
        score_penalty *= (1 - pct / 100)
score_penalty *= 1 / (1 + infraction_value)
```

| Infraction | coefficient |
|---|---|
| collision with pedestrian | 1.00 |
| collision with vehicle | 0.70 |
| collision with static layout | 0.60 |
| red light | 0.40 |
| scenario timeout | 0.40 |
| yield to emergency vehicle | 0.40 |
| stop sign | 0.25 |
| min speed | `0.40 × (1 - pct/100)` |
| outside route lanes | separate multiplier `(1 - pct/100)` |

### Why it changed

Under a product, the marginal cost of an infraction shrinks with each one: after
three collisions the penalty is already 0.125 and a fourth costs 0.0625 of the
original. That made "stop before things go wrong" a rational strategy and
compressed the whole field into the low scores. Under `1/(1+Σ)` each infraction
subtracts a comparable amount, so driving further and behaving keeps paying. The
official wording is a change "from an exponential to linear scoring model to
prevent early-stopping exploitation strategies" (leaderboard.carla.org, March 2025).

Worked comparison, route completion 100%:

| infractions | 2.0 | 2.1 |
|---|---|---|
| 1 pedestrian | 50.0 | 50.0 |
| 2 pedestrian | 25.0 | 33.3 |
| 5 pedestrian | 3.1 | 16.7 |
| 1 vehicle + 1 red light | 42.0 | 47.6 |
| 3 red lights | 34.3 | 45.5 |
| 1 stop sign | 80.0 | 80.0 |

Note 2.1 is *harsher* on a single pedestrian collision (coefficient 1.0 vs 0.5) and
much more forgiving in the tail.

## Terminating infractions

Some events end the route rather than scaling the penalty:

| Event | `status` | Meaning |
|---|---|---|
| `ROUTE_DEVIATION` | Agent deviated from the route | left the corridor |
| `VEHICLE_BLOCKED` | Agent got blocked | stationary too long (`ActorBlockedTest`) |
| route timeout | Agent timed out | the route's own deadline |

These leave route completion at whatever was achieved, so they hurt through the
first factor, not the second.

## Infraction keys

`PENALTY_NAME_DICT` maps criteria events to the JSON keys:

```
COLLISION_STATIC                 -> collisions_layout
COLLISION_PEDESTRIAN             -> collisions_pedestrian
COLLISION_VEHICLE                -> collisions_vehicle
TRAFFIC_LIGHT_INFRACTION         -> red_light
STOP_INFRACTION                  -> stop_infraction
OUTSIDE_ROUTE_LANES_INFRACTION   -> outside_route_lanes
MIN_SPEED_INFRACTION             -> min_speed_infractions
YIELD_TO_EMERGENCY_VEHICLE       -> yield_emergency_vehicle_infractions
SCENARIO_TIMEOUT                 -> scenario_timeouts
ROUTE_DEVIATION                  -> route_dev
VEHICLE_BLOCKED                  -> vehicle_blocked
(route timeout)                  -> route_timeout
```

`ROUND_DIGITS = 3` for meta, `ROUND_DIGITS_SCORE = 6` for scores.

## results.json schema

```json
{
  "_checkpoint": {
    "global_record": {
      "index": -1, "route_id": -1, "status": "Perfect|Failed|...",
      "infractions": { "<key>": <per-km float>, ... },
      "scores_mean":    { "score_composed": f, "score_route": f, "score_penalty": f },
      "scores_std_dev": { "score_composed": f, "score_route": f, "score_penalty": f },
      "meta": { "total_length": f, "duration_game": f, "duration_system": f,
                "exceptions": [ ... ] }
    },
    "progress": [done, total],
    "records": [
      {
        "index": 0,
        "route_id": "RouteScenario_0",
        "status": "Completed|Agent got blocked|Agent crashed|...",
        "num_infractions": n,
        "infractions": { "<key>": ["message with location", ...], ... },
        "scores": { "score_route": f, "score_penalty": f, "score_composed": f },
        "meta": { "route_length": f, "duration_game": f, "duration_system": f }
      }
    ]
  },
  "entry_status": "Started|Finished|Rejected|Crashed|Invalid",
  "eligible": true,
  "sensors": ["carla_camera", "carla_lidar", ...],
  "values": [], "labels": []
}
```

Key details:

- **Per-route `infractions` values are lists of message strings**; their length is
  the count. Global `infractions` values are floats — **per kilometre**, not counts.
- Records with `index == -1` (a route in progress) are **not** written, so a live
  file lags by one route.
- `sensors` uses the icon names from `sensors_to_icons` in the evaluator
  (`carla_camera`, `carla_lidar`, `carla_radar`, `carla_gnss`, `carla_imu`,
  `carla_opendrive_map`, `carla_speedometer`), not the sensor ids.
- `eligible` comes from `ELIGIBLE_VALUES`: only `Finished` is `True`.
- The file is rewritten after every route, which is what makes `--resume` work and
  also what makes it truncatable if you kill the process mid-write.

## Entry statuses

```python
ENTRY_STATUS_VALUES = ['Started', 'Finished', 'Rejected', 'Crashed', 'Invalid']
ELIGIBLE_VALUES = {'Started': False, 'Finished': True, 'Rejected': False,
                   'Crashed': False, 'Invalid': False}
FAILURE_MESSAGES = {
  "Simulation":    ["Crashed",  "Simulation crashed"],
  "Sensors":       ["Rejected", "Agent's sensors were invalid"],
  "Agent_init":    ["Started",  "Agent couldn't be set up"],
  "Agent_runtime": ["Started",  "Agent crashed"],
}
```

## Leaderboard 1.0

Same `route_completion × infraction_penalty` shape with the multiplicative form, but
a smaller infraction set (no scenario timeout, no min speed, no emergency-vehicle
yield) and different route/town data. A 1.0 result is not comparable to a 2.x one at
all: the routes, towns, scenarios and sensor budget all differ.

## Utility scripts in the repo

| Script | Use |
|---|---|
| `scripts/pretty_print_json.py` | formatted dump of a results file |
| `scripts/merge_statistics.py` | join shards **and** recompute the global record |
| `scripts/route_summarizer.py` | route file → table (not results) |

Use `merge_statistics.py` rather than this skill's `--merge` when the merged global
score has to be official — `--merge` deliberately does not recompute it.
