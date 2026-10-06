---
name: record-simulation
description: Records a running CARLA simulation to a .log file via start_recorder/stop_recorder for later deterministic replay — capturing actor spawns, transforms, traffic-light states and animations (optionally velocities/controls). Use when the user asks to "record the simulation", "start/stop the recorder", "capture this run", or "save a scenario to replay later". Does NOT capture sensor images.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT or a source checkout. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/recorder.md
---

# Record a CARLA simulation

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Capture a running simulation to a `.log` you can replay later ([`replay-recording`](../replay-recording/SKILL.md))
or interrogate ([`query-recording`](../query-recording/SKILL.md)). The deliverable
is a **verified** log: after stopping, the file is parsed back from the server to
confirm it holds frames — `start`/`stop` alone confirm nothing.

What it captures: world state per frame — actor create/destroy, transforms,
traffic-light states, vehicle wheel/animation and walker bones. With `--extra`
(`additional_data`) also velocities, accelerations and control inputs. **Not
captured:** sensor output (camera/lidar) — those re-simulate only on replay.

The `.log` is written on the **server's** filesystem (see the reference for
exactly where).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Start recording (or use `clip` for a fixed duration)
- [ ] Step 3: Let the simulation run (traffic, scenario, manual drive, ...)
- [ ] Step 4: Stop, then verify the log parses back with frames
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`. Prefix with
`source scripts/env.sh` or export them.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-4: Record

```bash
source scripts/env.sh

# manual bracketing: start, do things, stop
python3 scripts/record.py start --file /tmp/run.log
# ... run traffic / drive / trigger a scenario ...
python3 scripts/record.py stop

# or record a fixed-length clip and auto-verify
python3 scripts/record.py clip --file /tmp/run.log --seconds 20

# capture velocities/controls too (larger file)
python3 scripts/record.py start --file /tmp/run.log --extra
```

Use an **absolute** `--file` so you know where the log lands (a relative name
goes under the server's `CarlaUE4/Saved/`).

### Verify

`clip` verifies automatically; after a manual `stop`, confirm with the
[`query-recording`](../query-recording/SKILL.md) skill:

```bash
python3 ../query-recording/scripts/query.py info --file /tmp/run.log
```

A valid log shows a frame count and a duration. Zero frames means nothing ran
between start and stop, or the path was wrong.

## Examples

**Example 1: capture a traffic run**

User says: "record 30 seconds of traffic"

Start traffic (traffic-manager skill), then `clip --file /tmp/traffic.log
--seconds 30`. VERIFY shows frames + duration ~30s.

**Example 2: record around a manual action**

User says: "record while I run my scenario"

`start --file /tmp/scenario.log`, run the scenario, `stop`, then `query info`.

## Troubleshooting

**Problem: log has 0 frames / query says empty**
Cause: nothing simulated between start and stop, or a bad server-side path.
Solution: ensure actors are moving; use an absolute writable server path.

**Problem: replayed run has no camera images**
Cause: the recorder never stores sensor data.
Solution: expected — re-attach sensors and replay with `--replay-sensors`
(replay-recording skill) to regenerate them.

**Problem: file not found where I expected**
Cause: relative name resolves on the server, under CarlaUE4/Saved/.
Solution: pass an absolute path, and remember it is on the server machine.

## Outputs

- A `.log` recording on the server (path from `--file`), verified to contain
  frames. Feed it to [`replay-recording`](../replay-recording/SKILL.md) or
  [`query-recording`](../query-recording/SKILL.md).

Detail (capture contents, file location, size) in
[references/recorder.md](references/recorder.md).
