---
name: analyze-scenario-results
description: Reads and summarises ScenarioRunner output — the criteria pass/fail tables from --output/--file, the machine-readable --json and --junit result files, and the criteria JSON written alongside a --record recording — and runs the metrics module (metrics_manager.py) to compute custom measurements over a recorded run offline, without the simulator. Use when the user asks "did the scenario pass", "why did it fail", "summarise these results", "compare these runs", or wants distance/speed/lane metrics from a recording.
license: MIT
compatibility: Any OS with a scenario_runner checkout. Result-file summarising needs no CARLA at all. The metrics module needs an importable `carla` and a running server, because it replays the recording to reconstruct the map and actor states.
metadata:
  group: scenario-runner
  prerequisites: scripts/check_env.sh
  reference: references/metrics.md
---

# Analyse scenario results

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Three separate things get called "results", and they answer different questions:

| Artefact | Produced by | Answers |
|---|---|---|
| criteria table | `--output` / `--file` / `--json` / `--junit` | did it pass, and which criterion failed |
| criteria JSON next to a recording | `--record` | the full criterion state, for tooling |
| CARLA recorder log | `--record` | everything that happened, replayable |

The metrics module works off the third one, so anything you did not `--record` can
only be re-measured by re-running.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Summarise the result files you have
- [ ] Step 3: For deeper analysis, run a metric over a recording
- [ ] Step 4: Compare runs if you have more than one
```

### Step 2: Summarise

```bash
source scripts/env.sh

python3 scripts/analyze_results.py summary ./results            # a directory of results
python3 scripts/analyze_results.py summary ./results/*.json     # specific files
python3 scripts/analyze_results.py compare ./before ./after      # two runs, criterion by criterion
```

`summary` reads `.json`, `.xml` (junit) and `.txt` outputs, normalises them, and
prints one line per scenario with the failing criteria named. `compare` diffs two
sets by scenario name and criterion, which is the useful view when you changed a
controller and want to know what got worse.

To produce these in the first place, run with the output flags:

```bash
cd ../run-scenario
OUTPUT=1 JSON=1 OUTPUT_DIR=./results bash scripts/run_scenario.sh group:ControlLoss
```

Note the file naming: ScenarioRunner writes
`<outputDir>/<config name><YYYY-MM-DD-HH-MM-SS>.json` with **no separator** before
the timestamp, so names look like `ControlLoss_12025-08-14-10-22-31.json`.
`analyze_results.py` splits that back apart.

### Step 3: Metrics over a recording

Record first — this writes both a `.log` recording and a `<name>.json` of criteria:

```bash
cd ../run-scenario
RECORD=recordings bash scripts/run_scenario.sh FollowLeadingVehicle_1
# -> $SCENARIO_RUNNER_ROOT/recordings/FollowLeadingVehicle_1.log
#    $SCENARIO_RUNNER_ROOT/recordings/FollowLeadingVehicle_1.json
```

Then run a metric:

```bash
source scripts/env.sh

python3 scripts/analyze_results.py metrics --list      # bundled example metrics

python3 scripts/analyze_results.py metrics \
    --metric "$SCENARIO_RUNNER_ROOT/srunner/metrics/examples/distance_between_vehicles.py" \
    --log recordings/FollowLeadingVehicle_1.log \
    --criteria recordings/FollowLeadingVehicle_1.json
```

which is a wrapper for

```bash
python3 "$SCENARIO_RUNNER_ROOT/metrics_manager.py" \
    --metric <metric.py> --log <recording.log> [--criteria <criteria.json>]
```

**The metrics module needs a running server** even though it computes offline: it
replays the recording to recover the map and to resolve actor ids to positions.
That is the one non-obvious prerequisite.

Bundled examples, in increasing complexity:

| Metric | What it shows |
|---|---|
| `basic_metric.py` | the minimum: subclass `BasicMetric`, implement `_create_metric` |
| `criteria_filter.py` | pull specific fields out of the criteria JSON into a table |
| `distance_between_vehicles.py` | per-frame distance between two actors, plotted |
| `distance_to_lane_center.py` | lateral error against the lane centre, plotted |

Writing your own is a subclass of `BasicMetric` with one method; the recording is
exposed as a `MetricsLog` object with `get_actor_transforms`, `get_ego_vehicle_id`,
`get_all_frames` and friends. Details in
[references/metrics.md](references/metrics.md).

### Step 4: What the criteria mean

A failing criterion names the behaviour that broke, not the cause:

| Criterion failed | Read as |
|---|---|
| `CollisionTest` | the ego hit something — check the recording for what |
| `DrivenDistanceTest` | the ego did not get far enough: blocked, or the scenario ended early |
| `ActorBlockedTest` | the ego stopped and stayed stopped |
| `KeepLaneTest` / `WrongLaneTest` / `OnSidewalkTest` | lateral control or a bad overtake |
| `RunningRedLightTest` / `RunningStopTest` | traffic-control handling |
| `RouteCompletionTest` < 100 | route not finished (route mode) |
| `InRouteTest` | the ego left the route corridor — usually a wrong turn |
| `ScenarioTimeoutTest` | a scenario did not finish in its own budget |

"All scenario tests were passed successfully!" on stdout means every criterion
passed. "Not all scenario tests were successful" with no table means you forgot
`--output`.

## Examples

**Example 1: "did my controller pass the control-loss scenarios?"**

Run `group:ControlLoss` with `OUTPUT=1 JSON=1 OUTPUT_DIR=./results`, then
`summary ./results`. 15 configs, one line each, failures named.

**Example 2: "why did FollowLeadingVehicle_1 fail?"**

`summary` says `CollisionTest`. Re-run with `RECORD=recordings`, then replay it
([[replay-recording]]) to watch the impact, and run
`distance_between_vehicles.py` to see the gap closing rate.

**Example 3: "compare before and after my change"**

Two `OUTPUT_DIR`s, then `compare ./before ./after`. It reports criteria that
flipped in either direction, so a fix that broke something else is visible.

## Troubleshooting

**Problem: "Not all scenario tests were successful" with no detail**
Cause: no output flag.
Solution: add `OUTPUT=1` (stdout) and/or `JSON=1`.

**Problem: no result files in `OUTPUT_DIR`**
Cause: `--outputDir` only takes effect with one of `--file`/`--json`/`--junit`;
`--output` alone prints to stdout.
Solution: set `JSON=1` as well.

**Problem: `metrics_manager.py` fails with a connection error**
Cause: it needs a live server to replay against.
Solution: start one ([[run-carla-server]]); the map does not matter, the recording
names its own.

**Problem: the metric runs but every transform is `None`**
Cause: actor ids in the recording do not match what the metric asks for, usually
because it hard-codes ids from a different run.
Solution: resolve ids through `MetricsLog.get_ego_vehicle_id()` and
`get_actor_ids_with_role_name()` rather than by number.

**Problem: `--criteria` file not found next to the recording**
Cause: the criteria JSON is written by `_record_criteria` only when `--record` was
used, and its name is the recording name with `.log` replaced by `.json`.
Solution: re-run with `RECORD=`; without it there is no criteria file to pass.

**Problem: replaying a recording made by a different CARLA version does nothing**
Cause: recorder logs are version-specific.
Solution: replay with the version that recorded it.

## Outputs

A per-scenario pass/fail summary, a criterion-level diff between two runs, or a
metric's table/plot over a recording. Nothing is modified: all three modes are
read-only over files, plus a replay on the server for metrics.

The metrics API — `BasicMetric`, `MetricsLog`, and what the recorder actually
stores — is in [references/metrics.md](references/metrics.md).
