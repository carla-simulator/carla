# Recorder queries — detail

Detail layer for the `query-recording` skill. All three are `carla.Client`
methods that make the server parse the `.log` and return a string.

## info — show_recorder_file_info(name, show_all)

`show_all=False` (default): the header and summary —

- the map the run was recorded on,
- total duration and frame count,
- the list of actors (id, type/blueprint, and where they were created),
- per-frame collision and event markers (summarised).

`show_all=True`: the same plus **every frame's** state (positions, traffic
lights, etc.). Output is very large on long logs — use only to inspect a specific
short window, and prefer narrowing with a replay time range instead.

## collisions — show_recorder_collisions(name, type1, type2)

Lists recorded collisions where one party is `type1` and the other `type2`.
Categories are single characters:

| Code | Actor category |
|---|---|
| `h` | hero (the ego / recorded hero vehicle) |
| `v` | vehicle |
| `w` | walker (pedestrian) |
| `t` | traffic light |
| `o` | other |
| `a` | any |

Each result row carries the frame, the simulation time, and the two actor ids.
Use `a` on one side to catch "vehicle vs anything". Collisions are detected from
the recorded actors, so both parties must have been in the recording.

## blocked — show_recorder_actors_blocked(name, min_time, min_distance)

Finds actors that failed to move: an actor is "blocked" when it travels less than
`min_distance` over a span of at least `min_time`.

- `min_time` — **seconds** the actor must remain nearly stationary.
- `min_distance` — **centimetres** it must move to be counted as moving (100 = 1 m).

Typical use: spot vehicles wedged at a junction or deadlocked in traffic. Tune
`min_time` up to ignore normal stops at red lights, and `min_distance` to set how
"still" counts as stuck. Each row gives the actor id and how long it was blocked.

## From query to replay

These reports exist to target a replay: take a collision/blocked frame's time and
actor id and pass them to the replay-recording skill (`--start <t>`,
`--follow <id>`) to watch exactly that moment.
