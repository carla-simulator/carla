# The leaderboard evaluator in detail

## CLI

```
leaderboard_evaluator.py
  --routes FILE                 (required)
  --routes-subset ''            '' = all; '0-4'; '1,6,8'; '0-2,5,8-10'
  --repetitions 1
  --agent FILE                  (required)
  --agent-config ''
  --track SENSORS               SENSORS | MAP | SENSORS_QUALIFIER | MAP_QUALIFIER (2.x)
  --checkpoint ./simulation_results.json
  --debug-checkpoint ./live_results.txt
  --resume False                type=bool — see the gotcha below
  --debug 0                     0 quiet, 1 route info, 2+ live results
  --record ''
  --timeout 300.0
  --host localhost --port 2000
  --traffic-manager-port 8000 --traffic-manager-seed 0
```

LB 1.0 additionally requires `--scenarios FILE` (the
`all_towns_traffic_scenarios_public.json`).

### `--resume` is a `type=bool` argparse trap

`argparse` with `type=bool` calls `bool(str)`, so **every non-empty string is
True**, including `--resume False` and `--resume 0`. Only an empty string is False.
Pass `--resume 1` to resume and omit the flag otherwise. The repo's
`run_leaderboard.sh` passes `--resume=${RESUME}` with `RESUME` unset, which
evaluates to `--resume=` — empty, i.e. False. That is why it works by accident.

## Forced world settings

`_setup_simulation`, applied before any route:

```python
carla.WorldSettings(
    synchronous_mode      = True,
    fixed_delta_seconds   = 1/20,          # frame_rate = 20.0
    deterministic_ragdolls = True,
    spectator_as_ego      = False)
traffic_manager.set_synchronous_mode(True)
traffic_manager.set_hybrid_physics_mode(True)
```

`_load_and_wait_for_world`, after every `load_world(town, reset_settings=False)`:

```python
settings.tile_stream_distance  = 650
settings.actor_active_distance = 650      # large maps reset these on load
world.reset_all_traffic_lights()
traffic_manager.set_random_device_seed(args.traffic_manager_seed)
```

`_reset_world_settings` on the way out sets async, `fixed_delta_seconds = None`,
`deterministic_ragdolls = False`, `spectator_as_ego = True`, TM async — **but only
`if self.world and self.manager and not self._client_timed_out`**. A timed-out or
killed run therefore leaves the world synchronous.

## Watchdogs

`leaderboard/scenarios/scenario_manager.py` runs two `Watchdog(self._timeout)`
instances, both using `--timeout` (default 300 s):

| Watchdog | Fires when | Message |
|---|---|---|
| `_agent_watchdog` | the agent's `run_step` does not return | `Agent took longer than {t}s to send its command` |
| `_watchdog` | `world.tick(timeout)` does not return | `The simulation took longer than {t}s to update` |

A separate check in `_signal_handler` covers agent construction:
`Timeout: Agent took longer than {client_timeout}s to setup`.

Both are wall clock, not simulated time, so a slow machine can trip them on a
perfectly correct agent. Raising `--timeout` is legitimate for local debugging; the
submitted limit is not yours to set.

## Failure taxonomy

`FAILURE_MESSAGES` in `statistics_manager.py` maps a failure to an entry status:

| Failure | entry_status | status text |
|---|---|---|
| `Simulation` | `Crashed` | Simulation crashed |
| `Sensors` | `Rejected` | Agent's sensors were invalid |
| `Agent_init` | `Started` | Agent couldn't be set up |
| `Agent_runtime` | `Started` | Agent crashed |

`ELIGIBLE_VALUES` means only `Finished` counts as an eligible submission —
`Started`, `Rejected`, `Crashed` and `Invalid` are all ineligible. So an agent that
crashes on one route out of 90 does not merely lose that route's score; the whole
entry is ineligible until every route completes.

## Resume mechanics

`RouteIndexer.validate_and_resume(endpoint)`:

1. reads the checkpoint JSON,
2. requires a `progress` key — `[done, total]`,
3. **requires `progress[1] == self.total`**: if the route file, subset or
   repetitions changed, the totals differ and it starts over,
4. sets `resume_index = progress[0]`,
5. patches the case where progress exceeds the number of saved records.

Consequences: do not edit the route file, `--routes-subset` or `--repetitions`
between the original run and the resume, and keep the same `--checkpoint`.

## The run loop

For each (route, repetition):

```
load the world for the route's town  ->  apply large-map settings
construct the agent                  ->  agent watchdog
agent.setup(agent_config)            ->  self.track must match --track
validate_sensor_configuration()      ->  Rejected on failure
build RouteScenario (srunner)        ->  scenarios + background activity + parked vehicles
set_global_plan(...)                 ->  downsampled GPS + world waypoints
tick loop at 20 Hz                   ->  run_step -> VehicleControl
register statistics                  ->  compute score, write checkpoint
agent.destroy(); remove all actors; stop and destroy every sensor
```

`_cleanup` explicitly hunts leftovers:
`world.get_actors().filter('*sensor*')` → `stop()` + `destroy()`, because a
streaming sensor left behind slows every subsequent route.

The same process runs all routes, so `destroy()` between routes is where your
memory goes back.

## Progress while it runs

The checkpoint file is rewritten after every route, so it is safe to read mid-run:

```bash
python3 -c "import json;d=json.load(open('results.json'));print(d['_checkpoint']['progress'])"
```

`--debug 2` additionally writes a human-readable live file to
`--debug-checkpoint`, updated during the route with the current infractions.

## Cost

At 20 Hz, a Town12 route of ~10 km takes roughly 10–25 minutes of wall clock
depending on GPU and agent speed. `routes_training.xml` is 90 routes: budget a day
per repetition, and run subsets during development. `routes_devtest.xml` (2 routes)
is what to iterate on.

## The ROS agent path

`AgentWrapperFactory.get_wrapper()` returns a `ROSAgentWrapper` when the agent is a
`ROSBaseAgent`. The evaluator also holds a single `self._ros1_server` across all
routes deliberately — restarting it per route causes reconnection failures in
`roslibpy`. So a ROS agent's stack is launched per route but the bridge server is
not.

## Docker

Submissions run in a container. `scripts/make_docker.sh` builds it from
`scripts/Dockerfile.master` (or `Dockerfile.ros` with `-r melodic|noetic|foxy`),
requiring `CARLA_ROOT`, `SCENARIO_RUNNER_ROOT`, `LEADERBOARD_ROOT`,
`TEAM_CODE_ROOT` (and `CARLA_ROS_BRIDGE_ROOT` for the ROS variant). It copies
`$CARLA_ROOT/PythonAPI` and renames the eggs to `carla-leaderboard-py3x.egg`, so it
fails on a wheel-only CARLA tree with no `dist/*.egg`. See
[[package-leaderboard-agent]].
