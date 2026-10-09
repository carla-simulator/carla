---
name: read-leaderboard-results
description: Reads a CARLA Leaderboard results.json — per-route driving score, route completion, infraction penalty and the infraction lists — explains how the score was computed for that leaderboard version, and rescores an existing result under the other version's rules (2.0 multiplicative vs 2.1 additive) without re-running anything. Also diffs two runs and merges sharded result files. Use when the user asks "what did I score", "why is my score low", "did my agent pass", "why don't my scores match the leaderboard", or hands over a results.json.
license: MIT
compatibility: Any OS, Python 3.7+. Pure file reading — no CARLA, no server, no leaderboard checkout required (a checkout only helps confirm which version produced the file).
metadata:
  group: leaderboard
  prerequisites: scripts/check_env.sh
  reference: references/scoring.md
---

# Read leaderboard results

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

```
driving score = route completion × infraction penalty
```

Route completion is the percentage of the route driven. The infraction penalty
starts at 1.0 and is reduced by what you did wrong — and **how** it is reduced is
the one thing that changed between 2.0 and 2.1.

## Instructions

```
Progress:
- [ ] Step 1: Read the file (summary)
- [ ] Step 2: Drill into the routes that scored badly
- [ ] Step 3: If the numbers disagree with the online leaderboard, rescore
```

### Step 1: Summary

```bash
python3 scripts/read_results.py ~/results/results.json
```

Prints entry status and eligibility, the declared sensors, progress, a per-route
table (driving score / route % / penalty / top infractions), and the global record
with mean **and standard deviation**.

Two fields decide whether the run counts at all:

- `entry_status` — `Finished`, `Started`, `Rejected`, `Crashed` or `Invalid`.
- `eligible` — **`True` only for `Finished`**. One crashed route out of 90 makes
  the entire entry ineligible, not just that route.

### Step 2: Drill in

```bash
python3 scripts/read_results.py results.json --route RouteScenario_3 -v
```

Full scores, meta (route length, game and wall-clock duration) and every infraction
message with its location. The `-v` messages are what tell you *where* the ego hit
something, which is the difference between "collided" and "collided at the
`Accident` scenario in the third junction".

Reading a score:

| Symptom | Read as |
|---|---|
| route % high, penalty low | drives the route but breaks rules |
| route % low, penalty 1.0 | clean but stops early — check `vehicle_blocked`, `route_dev`, `route_timeout` |
| status `Agent got blocked` | `ActorBlockedTest` fired; the ego stood still too long |
| status `Agent crashed` | an exception in `run_step`, or a watchdog timeout |
| `eligible: false` with good scores | some route did not finish; find it and fix it |
| `outside_route_lanes` large | driving on the wrong lane; it reduces route completion, not the penalty |

### Step 3: Rescore across versions

```bash
python3 scripts/read_results.py results.json --as 2.1   # 2.0 result, 2.1 rules
python3 scripts/read_results.py results.json --as 2.0
```

This is exact for constant-penalty infractions, because the infraction **counts**
are what both formulas consume and the file stores them precisely. Verified
round-trip: rescoring a 2.0 file `--as 2.0` reproduces its stored scores to the
cent.

The difference is large and not a constant offset:

| pedestrian collisions on a completed route | 2.0 | 2.1 |
|---|---|---|
| 1 | 50.0 | 50.0 |
| 2 | 25.0 | 33.3 |
| 5 | 3.1 | 16.7 |

2.0 multiplies (`P = Π p_j^n_j`), so the first infraction is cheap and later ones
are nearly free — which rewarded stopping early. 2.1 sums
(`P = 1/(1+Σ c_j n_j)`), so every infraction costs the same absolute amount. Full
coefficient tables in [references/scoring.md](references/scoring.md).

`min_speed_infractions` and `outside_route_lanes` scale with a percentage the file
does not store, so rescoring leaves them out and says so — those routes come out as
an **upper bound**.

### Compare and merge

```bash
python3 scripts/read_results.py before.json after.json --diff
python3 scripts/read_results.py shard0.json --merge shard1.json shard2.json --out all.json
```

`--diff` matches by route id and reports the score delta plus which infraction
counts changed. `--merge` joins shards from parallel runs; it does not recompute the
global record — use the leaderboard's own `scripts/merge_statistics.py` when you
need an official global score.

## Examples

**Example 1: "what did my agent score?"**

`read_results.py results.json`. The global `driving score` mean is the number that
would rank you; the std dev tells you how much of it is luck.

**Example 2: "why is my score 12 when route completion is 95%?"**

`--route <id> -v`. A penalty of ~0.13 with 95% completion means several
infractions. Under 2.0 that is roughly three collisions; the messages name them.

**Example 3: "my local score doesn't match my submission"**

Check which version produced the file: `master` and `leaderboard-2.0` score
multiplicatively, the live leaderboard has scored additively since March 2025.
`--as 2.1` converts without re-running. If the counts *also* differ, it is not the
formula — it is a different CARLA build, TM seed or route subset.

**Example 4: "did I improve?"**

`--diff before.json after.json`. Score deltas plus infraction-count changes, so a
higher mean that hides a new failure mode is visible.

## Troubleshooting

**Problem: `results.json is not valid JSON`**
Cause: the run was killed mid-write; the checkpoint is rewritten after every route.
Solution: the previous route's data is lost. Re-run with `--resume 1` after
truncating, or use a shard from another run.

**Problem: `no completed routes`**
Cause: only in-progress routes. The evaluator saves records with `index != -1`
only, so a route still running is not in the file.
Solution: wait; watch `_checkpoint.progress`.

**Problem: `global_record` missing**
Cause: it is written when the run finishes every route.
Solution: expected for a partial run; per-route records are still there.

**Problem: scores are 0 with no infractions**
Cause: route completion is 0 — the agent never moved, or the sensors were rejected.
Solution: check `entry_status`; `Rejected` means sensor validation failed
([[write-leaderboard-agent]] `validate`).

**Problem: infractions per km look tiny in the global record**
Cause: global infractions are normalised per kilometre; per-route records hold raw
counts.
Solution: expected. Compare like with like.

**Problem: `--merge` drops routes**
Cause: duplicate route ids across shards are skipped, by design.
Solution: shard by `--routes-subset` so ids do not overlap.

## Outputs

A readable summary of any results file, a per-route drill-down, a cross-version
rescore, a two-run diff, or a merged results file. All read-only except `--merge`,
which writes a new file and never modifies its inputs.

The formulas, coefficient tables, every infraction key and the full results.json
schema are in [references/scoring.md](references/scoring.md).
