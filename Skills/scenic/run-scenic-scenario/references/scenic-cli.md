# Scenic against CARLA — CLI, models, maps

Detail layer for `run-scenic-scenario`.

## The command

```
scenic FILE --simulate --2d --count N --time T [-s SEED] [-p PARAM VALUE] [-v 0..3]
```

`--simulate` is `-S`. Without it, Scenic samples a scene and opens a **matplotlib
diagram** — which blocks on a headless machine, so it is not a substitute for a
dry run. To check a scenario without a simulator and without a display, sample
through the API instead (`scripts/sample_scenic.py`).

Bounds are not optional: `--count` and `--time` both default to infinity.

Other flags that matter:

| Flag | Why |
|---|---|
| `--2d` | 2D compatibility mode; what the CARLA scenarios were written for |
| `-p PARAM VALUE` | override any `param`, including `map` and `carla_map` |
| `-v 2` | prints one line per rejected sample — the only view into why sampling stalls |
| `--max-sims-per-scene N` | rejected simulations before resampling the scene |
| `--show-params` | resolved global parameters, useful when a `-p` seems ignored |
| `-b` | full internal backtrace; needed to see past Scenic's error wrapper |

## Two world models, same scenarios

The carlaChallenge set exists twice, and the copies are near-identical apart from
their model line, their map and their blueprint constants.

| | `scenic.simulators.carla.model` | `srunner.scenic.models.model` |
|---|---|---|
| Ships in | the `scenic` wheel | a scenario_runner checkout |
| Needs on `PYTHONPATH` | nothing | `SCENARIO_RUNNER_ROOT` |
| Blueprint source | Scenic's table, keyed on client version | same table + `srunner`'s own name list |
| Maps used by the challenge set | Town01, Town05, Town07 | one map for all of them |
| Simulator class | `scenic.simulators.carla.simulator` | `srunner.scenic.models.simulator` |

The ScenarioRunner copies are a *port*: model line rewritten, map repointed, and
blueprint constants updated to the ids the current build actually has. The Scenic
copies are upstream and still carry UE4-era ids such as
`vehicle.lincoln.mkz_2017`, which sample fine and then fail at spawn.

## Map features scenarios select on

Scenarios filter the road network, so a map either supports a scenario or cannot
express it. The counts that matter are unsignalized intersections by arm count:

```bash
python3 scripts/list_scenic.py --check-maps
```

A `filter(lambda i: i.is4Way and not i.isSignalized, network.intersections)` that
matches nothing raises `tried to make discrete distribution over empty domain!`
at **compile** time, because the `Uniform(*...)` over it is evaluated eagerly.

Rule of thumb: dense downtown maps are almost fully signalized, so
negotiation-at-an-unsignalized-junction scenarios need a suburban or highway map.

## Blueprint tables are keyed on the client version

`scenic/simulators/carla/_blueprintData.py` holds `_IDS[version][category]`. On a
client version with no entry, every category resolves empty and *all* scenarios
fail at sample time. On a version with an entry, individual categories may still
be empty — those scenarios fail with
`Scenic has no 'X' blueprints recorded for CARLA <version>` even when the build
does contain such assets.

Two consequences:

- **client and server must match exactly.** Scenic never asks the server what
  exists; a mismatched client offers ids the server cannot spawn.
- **a missing category is a Scenic data gap, not a content gap.** Name a concrete
  id with `with blueprint "..."` to bypass it.

`check_env.sh` reports which table matched and which of its categories are empty.

## World settings

Scenic sets synchronous mode with a fixed timestep (`param timestep`, default
0.1 s) for the duration of a run and restores async when it finishes cleanly. An
interrupted run leaves the world synchronous with no ticking client, which makes
every other tool appear to hang. `run_scenic.sh` restores it from an EXIT trap.

`param render 0` disables the spectator view Scenic otherwise opens; useful on a
headless server, and it does not affect the simulation.
