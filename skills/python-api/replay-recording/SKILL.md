---
name: replay-recording
description: Replays a CARLA .log recorded by the recorder — the whole run or a time window, at any speed, following a chosen actor, optionally regenerating sensors and restoring weather. Use when the user asks to "replay the recording", "play back the run", "watch it again in slow motion / at 2x", "follow vehicle N during replay", or "replay onto a different map".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Needs a .log on the server (from the record-simulation skill). Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  requires: record-simulation
  prerequisites: scripts/check_env.sh
  reference: references/replay.md
---

# Replay a CARLA recording

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Play back a `.log` produced by [`record-simulation`](../record-simulation/SKILL.md).
The server re-creates the recorded scene; `play` prints its summary. Confirm the
replay by watching the followed actor or by inspecting the same log with
[`query-recording`](../query-recording/SKILL.md).

## The scene, the holders and the clock

`play` puts the world into the state a replay needs, in this order:

1. **Stops local spawn holders.** `spawn-vehicles` / `spawn-walkers` / a held
   `create-sensor` stay resident by design, and each removes its own actors and
   restores the clock in a `finally`, so they are sent SIGINT rather than
   killed. `--keep-scene` skips all of this.
2. **Puts the world back to asynchronous.** The replayer runs **server-side**,
   so no client has to tick anything. If a client still owned a synchronous
   clock the replay would advance only as fast as that client ticked; worse,
   with the holder gone and the world left synchronous, nothing ticks at all
   and the replay appears to do nothing.
3. **Clears the scene.** A replay RECREATES the actors in the log, so it adds
   to the world rather than replacing it: replaying a 50-car log into a live
   50-car scene gives 100 vehicles driving through each other. Traffic signs
   and the spectator survive — they belong to the map, not the scene.

The corollary for recording: **the recorder is free to be the synchronous
client.** Capturing under a fixed timestep is what makes a log reproducible,
and it costs the replay nothing, because playback is driven by the server.

Holder detection reads `/proc` argv rather than matching a command-line
substring — a shell whose own command line merely quotes
`vehicles.py spawn --hold` matches a substring search and gets signalled, which
kills the caller by accident.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: (optional) inspect the log first (query-recording) to pick times/ids
- [ ] Step 3: Play — whole log, or a window; set speed / follow as asked
- [ ] Step 4: Stop when done (decide keep-actors)
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Play

```bash
source scripts/env.sh

# whole log at real time
python3 scripts/replay.py play --file /tmp/run.log

# a 10s window from t=5s, following actor 87, at 2x
python3 scripts/replay.py play --file /tmp/run.log --start 5 --duration 10 --follow 87 --time-factor 2.0

# regenerate sensors and restore the recorded weather
python3 scripts/replay.py play --file /tmp/run.log --replay-sensors --replay-weather

# change speed of the running replay, e.g. slow motion
python3 scripts/replay.py speed --factor 0.25
```

`--start` is seconds from the beginning (negative counts from the end),
`--duration 0` plays to the end, `--follow 0` means don't move the spectator.

### Step 4: Stop

```bash
python3 scripts/replay.py stop                 # remove replayed actors
python3 scripts/replay.py stop --keep-actors   # leave them in the world
```

## Examples

**Example 1: watch it back**

User says: "replay that run"

`play --file /tmp/run.log`. Server summary prints; watch in the CARLA window.

**Example 2: slow-mo on one car**

User says: "replay following car 87 at quarter speed"

`play --file /tmp/run.log --follow 87 --time-factor 0.25`.

**Example 3: regenerate camera data from a recording**

User says: "I need camera frames from that recording"

Re-attach the sensors you want (create-sensor / read-sensor skills), then
`play --file /tmp/run.log --replay-sensors`. The recorder stored no images; they
are regenerated now.

## Troubleshooting

**Problem: `play` prints a file-not-found / parse error**
Cause: the `.log` is not on the server at that path.
Solution: use the server-side path; a relative name resolves under
CarlaUE4/Saved/. Verify with `query-recording info` first.

**Problem: replay shows nothing / wrong map**
Cause: the log was recorded on a different map than the one loaded.
Solution: replay loads the recorded map automatically; for OpenDRIVE-only logs
use `--map-override <Name>`.

**Problem: no sensor images during replay**
Cause: `--replay-sensors` not set, or no sensors attached.
Solution: attach sensors, then replay with `--replay-sensors`.

**Problem: actors vanish when replay ends**
Cause: `stop` removes replayed actors by default.
Solution: `stop --keep-actors` to keep them.

## Outputs

Server state: the recorded scene re-enacted on the running server. Optionally
regenerated sensor data (with `--replay-sensors`). No file is produced.

Detail (time window semantics, sensors/weather, map override, sync interaction)
in [references/replay.md](references/replay.md).
