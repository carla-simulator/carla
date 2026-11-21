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
- **Traffic lights.** The mapping of CARLA light actors to IAI light ids is
  hard-coded for Town10HD (sorted actor ids → 4364+i and 5364+i). Elsewhere the
  lights are not synced and NPCs may run CARLA's reds.
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
