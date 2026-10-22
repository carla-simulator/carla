# Recorder

The recorder saves the state of the simulation on every frame: vehicles, pedestrians, traffic lights, weather and the rest of what the world holds at that moment.

The replayer plays that file back exactly as it was recorded, so the run can be inspected afterwards: from any point in the file, at any speed, and followed with the spectator. The [queries](#queries) below search a recording for collisions and for vehicles that got stuck, and [Recorder format](#recorder-format) covers the layout of the file itself.

- [__Recording__](#recording)
- [__Simulation playback__](#simulation-playback)
- [__Recorded file__](#recorded-file)
- [__Queries__](#queries)
- [__Sample Python scripts__](#sample-python-scripts)
- [__Recorder format__](#recorder-format)

---
## Recording

All the data is written in a binary file on the server side only. However, the recorder is managed using the [carla.Client](python_api.md#carla.Client).

Actors are updated on every frame according to the data contained in the recorded file. Actors in the current simulation that appear in the recording will be either moved or re-spawned to emulate it. Those that do not appear in the recording will continue their way as if nothing happened.

Every recording holds:

*   __Actors__ — creation and destruction.
*   __Traffic lights__ — state changes.
*   __Vehicles__ — position and orientation, light state, door state and wheel animation.
*   __Pedestrians__ — position and orientation, and their animation state.
*   __Bikers__ — the rider's animation state.
*   __Lights__ — light states from buildings, streets and vehicles.
*   __Weather__ — the weather parameters and every change made to them.

That is what the replayer needs to reproduce the run. Recording with `additional_data` adds everything else that describes the state rather than reproduces it:

*   __Kinematics__ — linear and angular velocity of vehicles and pedestrians.
*   __Bounding boxes and trigger volumes__ of the actors.
*   __Physics control__ of each vehicle.
*   __Traffic light times__ — the duration configured for each state.
*   __Walker bones__ — the full skeleton pose of each pedestrian.
*   __Execution time__ — how long the frame took on the machine that recorded it.

To start recording there is only need for a file name. Using `\`, `/` or `:` characters in the file name will define it as an absolute path. If no path is detailed, the file will be saved in `CarlaUnreal/Saved`.

```py
client.start_recorder("/home/carla/recording01.log")
```

To record the additional data as well, pass `additional_data`:

```py
client.start_recorder("/home/carla/recording01.log", True)
```

To stop the recording, the call is also straightforward.

```py
client.stop_recorder()
```

Starting a recording stops any playback that is running, so that the recording captures the live simulation. Pass `stop_replayer=False` to record while a playback continues, which is how a replayed situation is re-recorded with new sensors or new actors in it.

```py
client.start_recorder("/home/carla/recording02.log", False, False)
```

!!! Note
    As an estimate, 1h recording with 50 traffic lights and 100 vehicles takes around 200MB in size.

---
## Simulation playback

A playback can be started at any point during a simulation. Besides the path to the log file, this method needs some parameters.

```py
client.replay_file("recording01.log", time_start, duration, follow_id)
```

| Parameter | Default | Description |
| --- | --- | --- |
| `name` | — | Path of the recording. |
| `time_start` | — | Second of the recording to start at. Positive counts from the beginning, negative from the end. |
| `duration` | — | Seconds to play. `0` plays the whole recording. |
| `follow_id` | — | Id of the actor the spectator follows. `0` leaves the spectator free. |
| `replay_sensors` | `False` | Re-spawn the sensors that were recorded. |
| `replay_weather` | `False` | Restore the weather of the recording, and the changes made to it during it. Otherwise the current weather is kept. |
| `offset` | 10 m behind, 5 m above, pitched 25&deg; down | Where the spectator sits relative to the followed actor. Ignored when `follow_id` is `0`. |
| `map_override` | empty | Name of the map to replay on, instead of the one the recording names. For replaying on a different variant of the same road network. |

At the end of the playback, vehicles may be returned to autopilot and pedestrians stop, depending on how the replayer was configured.



### Setting a time factor

The time factor will determine the playback speed. It can be changed any moment without stopping the playback.

```py
client.set_replayer_time_factor(2.0)
```

| `time_factor` | Playback speed |
| --- | --- |
| `1.0` | Real time, the default |
| `> 1.0` | Fast motion |
| `< 1.0` | Slow motion |

<br>



!!! Important
    From `time_factor` 2.0 upwards the actors' positions are snapped instead of interpolated. Pedestrian animations are not affected by the time factor.

When the time factor is around __20x__ traffic flow is easily appreciated.

![flow](img/core_recorder/recorder_replay_traffic.webp)
<div class="img-caption">A replay of Traffic Manager traffic at a junction, seen from above.</div>

---
## Recorded file

The details of a recording can be retrieved using a simple API call. By default, it only retrieves those frames where an event was registered. Setting the parameter `show_all` would return all the information for every frame. The specifics of how the data is stored are in [Recorder format](#recorder-format).

```py
# Show info for relevant frames
print(client.show_recorder_file_info("recording01.log", False))
```

* __Opening information.__ Map, date and time when the simulation was recorded.

* __Frame information.__ Any event that could happen such as actors spawning or collisions. It contains the actors' ID and some additional information.

* __Closing information.__ Number of frames and total time recorded.

For the one-minute traffic recording used in the queries below, trimmed (`...`):

```
Version: 1
Map: Town10HD_Opt
Date: 10/16/26 12:14:19

Frame 1 at 0 seconds
 Create 1: spectator (0) at (-17219.7, 18385.9, 2763.81)
 Create 2: traffic.stop (4) at (5745, 6115, 15)
 Create 3: traffic.stop (4) at (2610, 7515, 15)
 ...
 Create 8: traffic.traffic_light (3) at (-4617.16, -7355.66, 25.4254)
 Create 9: traffic.traffic_light (3) at (-3444.92, -5102.05, 25.4255)
 ...
 Create 25: vehicle.nissan.patrol (1) at (4040.32, 4270.69, 4.93826)
  role_name = autopilot
  number_of_wheels = 4
  color = 2,35,54
  base_type = car
  sticky_control = true
  ...

Frames: 1200
Duration: 59.95 seconds
```

The number in brackets after the blueprint id is the actor type (`0` other, `1` vehicle, `2` walker, `3` traffic light, `4` traffic sign, `5` sensor), and locations are in centimetres.

---
## Queries

### Collisions

Vehicles must have a [collision detector](ref_sensors.md#collision-detector) attached to record collisions. These can be queried, using arguments to filter the type of the actors involved in the collisions. For example, `h` identifies actors whose `role_name = hero`, usually assigned to vehicles managed by the user. There is a specific set of actor types available for the query.

* __h__ = Hero  
* __v__ = Vehicle  
* __w__ = Walker  
* __t__ = Traffic light  
* __o__ = Other  
* __a__ = Any

!!! Note
    The `manual_control.py` script assigns `role_name = hero` for the ego vehicle.

The collision query requires two flags to filter the collisions. The following example would show collisions between vehicles, and any other object.

```py
print(client.show_recorder_collisions("recording01.log", "v", "a"))
```

The output summarizes time of the collision, and type, ID and description of the actors involved. For the recording in the image below, a Lincoln MKZ carrying a collision sensor driven into the side of a Nissan Patrol:

```
Version: 1
Map: Town10HD_Opt
Date: 10/15/26 11:56:04

    Time  Types     Id Actor 1                                 Id Actor 2
       5   v v    1206 vehicle.lincoln.mkz                   1205 vehicle.nissan.patrol

Frames: 140
Duration: 7 seconds
```

!!! Important
    Only actors with a `sensor.other.collision` attached register collisions, and the actor carrying the sensor is always `Actor 1`.

The collision can be reenacted by using the recorder and setting it seconds before the event, following the actor that registered it.

```py
client.replay_file("crash.log", 3, 0, 1206)
```
In this case, the playback showed this.

![collision](img/core_recorder/recorder_collision.webp)
<div class="img-caption">A recorded collision; show_recorder_collisions(log, 'v', 'v') lists it (lincoln.mkz against nissan.patrol at t = 5 s).</div>

### Blocked actors

Detects vehicles that were stuck during the recording. An actor is considered blocked if it does not move a minimum distance in a certain time. This definition is made by the user during the query.

```py
print(client.show_recorder_actors_blocked("recording01.log", min_time, min_distance))
```

Both arguments are required; neither has a default.

| Parameter | Description |
| --- | --- |
| `min_time` | Seconds the actor has to move `min_distance` in. |
| `min_distance` | Centimetres it has to move in that time to not count as blocked. |



---

!!! Note
    Sometimes vehicles are stopped at traffic lights for longer than expected.

The following example considers that vehicles are blocked when moving less than 1 meter during 20 seconds. The recording is one minute of Traffic Manager traffic, 50 vehicles on Town10HD_Opt.

```py
client.show_recorder_actors_blocked("traffic60.log", 20, 100)
```

The output is sorted by __duration__, which states how long it took to stop being "blocked" and move the `min_distance`. The list below is trimmed to its first rows:

```
Version: 1
Map: Town10HD_Opt
Date: 10/16/26 12:14:19

    Time     Id Actor                                 Duration
       0      1 spectator                                   59
      11     72 vehicle.ue4.mercedes.ccc                    49
       2     42 vehicle.fuso.mitsubishi                     42
      10     40 vehicle.sprinter.mercedes                   35
      29     53 vehicle.ue4.audi.tt                         31
      15     61 vehicle.dodgecop.charger                    30
      ...

Frames: 1200
Duration: 59.95 seconds
```

The spectator never moves, so it is always listed. The vehicle `72` was stopped for `49` seconds from second `11`, most likely queued at a traffic light. Reenact the simulation a few seconds before second `11` to check it out.

```py
client.replay_file("traffic60.log", 9, 0, 72)
```

---
## Sample python scripts

Four scripts ship with CARLA to drive the recorder from the terminal.

__`PythonAPI/util/start_replaying.py`__ — play a recording back.

| Parameter | Description |
| --- | --- |
| `-f`, `--recorder-filename` | File to replay. |
| `-s`, `--start` | Second of the recording to start at. Default `0.0`. |
| `-d`, `--duration` | Seconds to play. `0.0`, the default, plays all of it. |
| `-c`, `--camera` | Id of the actor the spectator follows. |
| `-x`, `--time-factor` | Playback speed. `2.0` is twice as fast. |
| `-i`, `--ignore-hero` | Do not replay the hero vehicle. |
| `--move-spectator` | Move the spectator with the followed actor. |
| `--spawn-sensors` | Re-spawn the sensors recorded in the file. |

__`PythonAPI/util/show_recorder_file_info.py`__ — print what a recording contains.

| Parameter | Description |
| --- | --- |
| `-f`, `--recorder_filename` | File to inspect. |
| `-a`, `--show_all` | Show every frame, not only those with events. |
| `-s`, `--save_to_file` | Write the output to a file. |

__`PythonAPI/util/recorder_comparer.py`__ — compare two recordings frame by frame, which is how a determinism regression is found.

| Parameter | Description |
| --- | --- |
| `-f1`, `--file1` | First recording. |
| `-f2`, `--file2` | Second recording. |

__`PythonAPI/examples/recorder_replay.py`__ — a fuller replay example, with weather, map override and camera control.

| Parameter | Description |
| --- | --- |
| `-f`, `--file` | File to replay. Required. |
| `--start-time`, `--end-time` | Window of the recording to play. |
| `--follow-id`, `--follow-ego` | Actor the spectator follows. |
| `--top-view` | Watch from above instead. |
| `-m`, `--map-override` | Replay on a different map. |
| `--replay-weather` | Replay the recorded weather too. |
| `--factor` | Playback speed. |

Every one of them also takes `--host` and `-p`/`--port`.

---
## Recorder format

A recording is a binary file of its own, not a text log: a header naming the map and the duration, then one block per frame holding the packets that describe what changed in it. Reading it does not need CARLA — the layout is fully specified, so a recording can be parsed, filtered or converted by a tool of your own.

[__Recorder binary file format__](ref_recorder_binary_file_format.md) documents all of it: how strings and vectors are written, the info header, every one of the packet types with its fields, and the order packets appear in within a frame.

---
Now it is time to experiment for a while. Use the recorder to playback a simulation, trace back events, make changes to see new outcomes. Feel free to share what you find in [GitHub Discussions](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to CARLA's GitHub Discussions">
GitHub Discussions</a>
</p>
</div>
