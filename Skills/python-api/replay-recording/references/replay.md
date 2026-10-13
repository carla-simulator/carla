# Replay — detail

Detail layer for the `replay-recording` skill.

## API

`client.replay_file(name, time_start, duration, follow_id, replay_sensors=False,
replay_weather=False, offset=Transform(), map_override='')`:

| Arg | Meaning |
|---|---|
| `name` | server-side `.log` path |
| `time_start` | seconds from the start; **negative counts from the end** |
| `duration` | seconds to replay; `0` = to the end |
| `follow_id` | actor id the spectator follows; `0` = free spectator |
| `replay_sensors` | regenerate sensor data from re-attached sensors |
| `replay_weather` | restore the weather recorded in the log |
| `offset` | a `carla.Transform` spatial offset applied to the replay |
| `map_override` | replay onto this map instead of the recorded one |

Related replayer calls:

- `client.set_replayer_time_factor(f)` — playback speed; `1.0` real time, `2.0`
  double, `0.5` half. Settable before and during replay (`speed` command).
- `client.set_replayer_ignore_hero(bool)` — skip the recorded hero (drive it live).
- `client.set_replayer_ignore_spectator(bool)` — don't move the spectator.
- `client.stop_replayer(keep_actors)` — stop; `keep_actors=True` leaves the
  replayed actors in the world instead of removing them.

## Behaviour notes

- **Map loads automatically.** Replay switches to the map recorded in the log; you
  do not load it yourself. `map_override` is for logs without a bundled map
  (OpenDRIVE-generated worlds).
- **Sensors are regenerated, not stored.** The recorder never saved images. With
  `replay_sensors=True` and sensors attached, the world is re-simulated and the
  sensors produce fresh data — a common way to re-derive camera/lidar from a run.
- **Actor ids.** Replayed actors may get new ids; use `query-recording info
  --all` to read the recorded ids and pick a `--follow` target.
- **Sync mode.** For frame-accurate replay (e.g. to capture regenerated sensors
  deterministically) replay in synchronous mode (set-world-settings skill) and
  tick the world.
- **Ending.** `stop` without `--keep-actors` removes the replay's actors; with it,
  they persist so you can keep driving from that state.
