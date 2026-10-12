---
name: run-leaderboard-evaluation
description: Runs the CARLA Leaderboard evaluator over a set of routes with your agent — routes/routes-subset/repetitions/track selection, checkpoint and live-results endpoints, resume after a crash, recording, and the debug levels — for leaderboard 1.0, 2.0 or 2.1. Preflights the whole stack (version pairing, maps, ports, agent class, sensor budget) before spending hours, and hands the world back in async mode. Use when the user asks to "run the leaderboard", "evaluate my agent", "run routes_training", "resume an evaluation", or "reproduce my submission locally".
license: MIT
compatibility: Linux with a leaderboard + matching scenario_runner checkout and a CARLA build for that version (0.9.10.1 for LB 1.0; the leaderboard 0.9.14+large-maps build for 2.x). Town12/Town13 are required for 2.x routes. Long runs — 90 training routes take many hours.
metadata:
  group: leaderboard
  prerequisites: scripts/check_env.sh
  reference: references/evaluator.md
---

# Run a leaderboard evaluation

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

`leaderboard_evaluator.py` drives your agent through each route, records
infractions, computes a driving score and writes a JSON checkpoint it can resume
from. It is a long-running batch job: `routes_devtest.xml` is 2 routes,
`routes_validation.xml` is 20, `routes_training.xml` is **90 routes in Town12** —
hours of wall clock, so the preflight matters more than usual.

## Instructions

```
Progress:
- [ ] Step 1: Preflight everything (bash scripts/check_env.sh) — clear every FAIL
- [ ] Step 2: Smoke-test on one devtest route
- [ ] Step 3: Run the real set, with a checkpoint you can resume from
- [ ] Step 4: Read the results (read-leaderboard-results)
```

### Step 1: Preflight

```bash
source scripts/env.sh
bash scripts/check_env.sh
```

This is the skill's main value. It checks, before anything expensive: the
leaderboard version, that `scenario_runner` is on the paired branch, that `carla`
/ `agents` / `srunner` / `leaderboard` all import, that the agent file exists and
defines the class the evaluator will look for, that the track is valid for the
version, and that Town12/Town13 exist on the running server.

### Step 2-3: Run

```bash
source scripts/env.sh

# smoke test: one route, debug on
TEAM_AGENT="$LEADERBOARD_ROOT/leaderboard/autoagents/npc_agent.py" \
    bash scripts/run_leaderboard.sh --routes-subset 0 --debug 1

# your agent over the validation set, resumable
TEAM_AGENT=~/team_code/my_agent.py TEAM_CONFIG=~/team_code/config.json \
CHECKPOINT_ENDPOINT=~/results/validation.json \
    bash scripts/run_leaderboard.sh --routes "$LEADERBOARD_ROOT/data/routes_validation.xml"

# a slice of the training set, 3 repetitions each
    bash scripts/run_leaderboard.sh --routes-subset '0-4,10' --repetitions 3

# resume after a crash — picks up where the checkpoint left off
    bash scripts/run_leaderboard.sh --resume 1
```

The wrapper sets the environment, runs the preflight, then calls the evaluator.
The repo's own `run_leaderboard.sh` does the same thing with less checking — and
note it references `DEBUG_CHECKPOINT_ENDPOINT` without ever exporting it, so live
results silently go nowhere. This wrapper defaults it next to the checkpoint.

Everything the evaluator accepts, and what it means:

| Flag / env | Meaning |
|---|---|
| `--routes` / `ROUTES` | route XML (default `data/routes_devtest.xml`) |
| `--routes-subset` / `ROUTES_SUBSET` | `''` = all, `0-4`, `1,6,8`, or `0-2,5,8-10` |
| `--repetitions` / `REPETITIONS` | repeats per route; each gets a fresh TM seed |
| `--track` / `CHALLENGE_TRACK_CODENAME` | `SENSORS`, `MAP`, `SENSORS_QUALIFIER`, `MAP_QUALIFIER` |
| `--agent` / `TEAM_AGENT` | the agent `.py` |
| `--agent-config` / `TEAM_CONFIG` | passed to your `setup()` |
| `--checkpoint` / `CHECKPOINT_ENDPOINT` | results JSON; also the resume state |
| `--debug-checkpoint` / `DEBUG_CHECKPOINT_ENDPOINT` | live text results, written when `--debug >= 2` |
| `--debug` / `DEBUG_CHALLENGE` | `0` quiet, `1` route/scenario info, `2`+ live results |
| `--record` / `RECORD_PATH` | CARLA recorder output directory |
| `--resume` / `RESUME` | continue from the checkpoint instead of restarting |
| `--timeout` | per-route client timeout, default **300 s** |
| `--traffic-manager-seed` | fixed per run; repetitions vary it deterministically |

`--routes-subset` is indices into the file, **not** route ids. They coincide in the
shipped files because ids are sequential from 0, but not in a hand-edited file.

### What the evaluator forces

`_setup_simulation` applies these regardless of the server's current settings:

```python
synchronous_mode = True, fixed_delta_seconds = 1/20
deterministic_ragdolls = True, spectator_as_ego = False
traffic_manager: synchronous, hybrid_physics_mode = True
per load_world: tile_stream_distance = 650, actor_active_distance = 650
world.reset_all_traffic_lights()
```

So do not bother setting weather, sync or TM options beforehand — they are
overwritten. The 20 Hz tick rate is fixed and is what your agent's per-tick budget
is measured against.

### Step 4: Aftermath

The evaluator resets the world to async itself in `_reset_world_settings`, **but
only if the run did not time out** (`if self.world and self.manager and not
self._client_timed_out`). A killed or timed-out run leaves synchronous mode on and
the world looks hung. The wrapper traps EXIT and resets it; otherwise
[[set-world-settings]] `async`.

Results go to `CHECKPOINT_ENDPOINT` — read them with
[[read-leaderboard-results]].

## Examples

**Example 1: "check my leaderboard setup works"**

`check_env.sh`, then the NPC agent on `--routes-subset 0`. It should finish with a
driving score near 100. If it does not, the setup is wrong, not your agent.

**Example 2: "evaluate my agent properly"**

Run `routes_validation.xml` (20 routes, Town13 — the closest public proxy for the
test set) with `CHECKPOINT_ENDPOINT` on a persistent path. Expect hours. Check
progress by reading the checkpoint as it grows.

**Example 3: "it crashed on route 47 of 90"**

`--resume 1` with the same `--checkpoint`. The evaluator reads
`_checkpoint.progress` and continues. Do not change the route file between the two
halves — the indices must still mean the same thing.

**Example 4: "reproduce my submitted score"**

Confirm the version: only `leaderboard-2.1` scores the way the live leaderboard
does. `master` and `leaderboard-2.0` use the older multiplicative penalty and will
disagree. If you already have a 2.0 `results.json`, rescore it in place with
[[read-leaderboard-results]] `--as 2.1` instead of re-running.

## Troubleshooting

**Problem: `ImportError: CARLA version 0.9.14 or newer required`**
Cause: client older than the evaluator's minimum. The leaderboard CARLA build
reports the literal version `leaderboard` and is exempted from the check.
Solution: install the matching client ([[install-python-api]]).

**Problem: `Exception: The CARLA server uses the wrong map!`**
Cause: the route's town is missing — Town12/Town13 without AdditionalMaps.
Solution: install AdditionalMaps or use the leaderboard build.

**Problem: `Agent's sensors were invalid` / entry status `Rejected`**
Cause: sensor budget, duplicate id, mount radius, or track mismatch.
Solution: [[write-leaderboard-agent]] `validate` before running.

**Problem: `Agent couldn't be set up` / `Timeout: Agent took longer than 300s to setup`**
Cause: `setup()` too slow, or it raised.
Solution: the traceback is printed; raise `--timeout` while debugging.

**Problem: `Agent crashed` mid-route**
Cause: an exception in `run_step`, or a per-tick watchdog overrun.
Solution: `--debug 1` prints the route position when it died; the recorder
(`--record`) lets you replay it.

**Problem: `Simulation crashed` / entry status `Crashed`**
Cause: the *server* died or stopped answering — commonly out of GPU memory on
Town12/Town13, or a segfault.
Solution: check the server log; run with `-quality-level=Low`; confirm no second
server is on the port ([[run-carla-server]]).

**Problem: nothing in the live-results file**
Cause: `--debug` below 2, or `DEBUG_CHECKPOINT_ENDPOINT` empty (the repo's own
`run_leaderboard.sh` never exports it).
Solution: `--debug 2` and set the endpoint; the wrapper defaults it.

**Problem: `--resume` restarts from zero**
Cause: `--resume` takes `type=bool`, so **any** non-empty string is `True` and an
empty one is `False`; and the checkpoint path must match the previous run.
Solution: pass `--resume 1` and the same `--checkpoint`.

**Problem: the second run scores differently with the same agent**
Cause: TM seed and repetition index change the traffic; agents with any
nondeterminism amplify it.
Solution: fix `--traffic-manager-seed` and compare like for like; use
`--repetitions` and read the mean and std dev the evaluator computes.

## Outputs

A `results.json` checkpoint with per-route scores, infractions and a global record
(mean and standard deviation), optionally a live text file and CARLA recordings.
The world is handed back in asynchronous mode.

Every evaluator flag, the forced world settings, the watchdogs and the resume
mechanics are in [references/evaluator.md](references/evaluator.md).
