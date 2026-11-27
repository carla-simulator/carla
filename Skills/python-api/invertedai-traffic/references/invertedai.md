# Inverted AI in CARLA: reference

## What it is

Inverted AI serves a learned driving model (trained on real traffic) over an HTTP
API. CARLA does no driving for these NPCs: each 0.1 s step the client sends the
agents' states, gets the next states back, and teleports the actors
(`set_simulate_physics(False)` + `ApplyTransform`). This is the difference from
the Traffic Manager, which is rule-based and runs locally.

## SDK calls used

| Call | Where | Purpose |
|---|---|---|
| `iai.add_apikey(key)` | both | authenticate; the key comes from `IAI_API_KEY` |
| `iai.location_info(location)` | `ping`, `check_env.sh --online` | map metadata; cheapest authenticated call |
| `iai.get_regions_default(location, total_num_agents, area_shape, map_center)` | `run` | split the area into 100 m regions |
| `iai.large_initialize(location, regions, traffic_light_state_history, random_seed)` | `run` | sample N realistic initial states |
| `iai.large_drive(location, agent_states, agent_properties, recurrent_states, traffic_lights_states, ...)` | `run`, each step | advance every agent 0.1 s |

The example additionally uses `iai.WaypointManager` (route goals), `iai.LogWriter`
(JSON log + birdview GIF) and drives CARLA walkers as "CARLA-controlled" agents
that Inverted AI vehicles react to.

## Facts that matter

- **Step is 0.1 s.** The world must run synchronous with `fixed_delta_seconds=0.1`.
  Any other rate makes the NPCs move at the wrong speed.
- **Location strings** are `carla:<Town>`. CARLA on UE 5 ships `Town10HD_Opt`;
  the IAI name is `carla:Town10HD`. The client strips `_Opt`.
- **Coordinates** are shared: IAI `center.x/y` are CARLA metres; orientation is
  radians (CARLA yaw is degrees).
- **Traffic lights.** IAI numbers its lights (e.g. Town10HD 4364-4378 and
  5364-5378, two per CARLA light) independently of CARLA's actor ids. They are
  paired along the lane: each IAI light from `location_info().static_actors`
  goes to the nearest CARLA stop waypoint (`get_stop_waypoints()`) within
  2.5 m of its lane centre, facing within 45 degrees, from 2 m before to 20 m
  past it. Measured with that rule:

  | Map | CARLA lights | IAI lights | Unpaired |
  |---|---|---|---|
  | Town01 / Town02 | 36 / 24 | 36 / 24 | none (IAI lights up to 13.5 m past the stop waypoint) |
  | Town03 | 38 | 57 | 1 CARLA light |
  | Town04 / Town05 / Town06 / Town07 | 43 / 54 / 19 / 17 | 52 / 109 / 67 / 18 | none |
  | Town10HD | 15 | 30 | none |

  Town15 does not pair at all: Inverted AI's `carla:Town15` is rotated 180
  degrees against CARLA 1.0's Town15 (its lights match CARLA's stop lines only
  with x and y negated, and no initialized vehicle lines up with a CARLA lane
  until rotated), so the co-simulation is not usable there. Its light ids also
  repeat once per signal head; the clients keep each id once.

  A plain radius does not work: Town01/02 lights sit 10-13.5 m along the lane,
  and Town05/06 ones on the line between two lanes. The old table built from
  sorted actor ids (4364+i) was off by two on UE 5.8 Town10HD, so every NPC
  obeyed another junction's light. Do not reintroduce it.
- **Per-lane lights.** Where several IAI lights pair with one CARLA light they
  are one per lane. On Town06 they really differ (the outer lane often stays
  green), so in `iai` mode the client shows their majority state and counts the
  split steps.
- **Who runs the lights.** `traffic_lights_states` sent to `large_drive`
  override the server's lights; with them left out and the previous response's
  `light_recurrent_states` passed instead, the server runs the lights (each
  light group's state and time remaining; a phase lasts up to about 20 s). The
  server steps them at the start of the call and the agents react to the new
  states, so a step's lights are only known when the call returns. The SDK
  returns light ids as `int`, while the clients key them as `str`.
- **Stop lines differ.** Each IAI light sits 0.5-6.5 m further along the lane
  than the matching CARLA stop waypoint, so IAI cars stop a few metres past
  CARLA's stop line. That is map geometry, not a light-state difference;
  measure red-light crossings at IAI's light positions.
- **Cost / latency.** Every step is one or more HTTPS requests
  (`single_call_agent_limit` agents each). Measured on Town10HD with 28 agents:
  ~385 ms per `large_drive` and ~11 s for `large_initialize`, so the simulation
  runs about 4x slower than real time. Each call counts against the key's quota.
- **Traffic-light enum** is `TrafficLightState.none/green/yellow/red`. Older
  examples used `TrafficLightState.Off`, which raises `AttributeError` when a
  light is off.
- **Academic keys** make the SDK print `Commercial access denied and fallback to
  check for academic access.` on each call. It is harmless.
- **numpy pin.** The commit pinned in `PythonAPI/examples/requirements.txt` forces
  numpy 1.26; keep it in its own venv (`--system-site-packages` so `carla` from
  the user site is visible). Python must be < 3.13.

## Outputs to other skills

- `run --record FILE` / `run_example.sh ... --record` produce a CARLA recorder
  log: inspect it with `query-recording`, replay it with `replay-recording` (the
  replay needs neither the key nor the venv).
- `run_example.sh ... --iai-log` writes `<OUT>/<timestamp>/iai_log/*.json` and a
  birdview `*.gif`.

## Links

- SDK and docs: https://github.com/inverted-ai/invertedai, https://docs.inverted.ai
- API key: https://www.inverted.ai
