# Recorder — detail

Detail layer for the `record-simulation` skill: the API, what is and isn't
captured, and where the file goes.

## API

- `client.start_recorder(name, additional_data=False, stop_replayer=True)` →
  begins recording; returns a status string. `stop_replayer=True` stops any
  ongoing replay first.
- `client.stop_recorder()` → ends recording and flushes the file.
- `client.show_recorder_file_info(name, show_all=False)` → parse a log back
  (used here to verify a recording; see the query-recording skill for full use).

## What is captured

Per simulation frame, the recorder stores world state, not renders:

- actor spawns and destroys (with blueprint + id),
- actor transforms (position + rotation),
- traffic-light states and timing,
- vehicle wheel angles and light state, walker bone poses,
- with `additional_data=True`: linear/angular velocities, accelerations, vehicle
  control inputs, and physics control — a larger file, needed only if you will
  query or analyse those quantities.

## What is NOT captured

- Sensor output: camera images, lidar/radar point clouds, etc. The recorder
  records the *scene*, so on replay you re-attach sensors and set
  `replay_sensors=True` to regenerate their data from the replayed world.
- Anything spawned outside CARLA's actor system.

## Where the file goes

`start_recorder(name)` writes on the **server**:

- **Relative** name → under the server's `CarlaUE4/Saved/` directory (source
  build) or the packaged build's `.../CarlaUE4/Saved/`.
- **Absolute** path → exactly there, on the server machine.

On a remote server the `.log` is on that host, not the client — copy it over if
you need it locally. Logs can grow quickly (more with `--extra`); a long run is
many MB.

## Determinism note

Replay fidelity is best when the recording was made in synchronous mode with a
fixed step (see the set-world-settings skill). Variable-step recordings replay,
but frame timing is approximate.
