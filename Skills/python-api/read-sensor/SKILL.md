---
name: read-sensor
description: Listens to a CARLA sensor and either saves its data to files, shows it live in a window, prints a one-shot summary, or (ros-info) reports the native ROS 2 topics, QoS and enabled-for-ROS state so you can echo it from ROS instead. Cameras save as PNG (depth/semantic auto-colourised) and display in a pygame window; lidar saves as .ply and shows as a top-down scatter; IMU/GNSS/radar/collision stream to JSONL or the console. Use when the user asks to "show/view the camera", "display the lidar", "save the sensor data / capture a dataset", or "what is this sensor reading". Select the sensor by id, type, or the actor it's attached to.
license: MIT
compatibility: Any OS with the CARLA PythonAPI, numpy, and (for windows) pygame installed for the active interpreter, and a reachable running CARLA server with a sensor. A window needs a display; saving/summary work headless. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  requires: create-sensor
  prerequisites: scripts/check_env.sh
  reference: references/read-sensor.md
---

# Read a sensor

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Point this at an existing sensor (make one with [`create-sensor`](../create-sensor/SKILL.md))
and either **show** it in a window, **save** its stream to disk, or get a one-shot
**info** summary. The goal is: select a sensor → see it.

Selector (any one): `--id N` · `--type sensor.camera.rgb` · `--attached-to hero`.

## Capture in lock-step, not as an observer

`save` takes the world's clock when the world is asynchronous, then ticks it
once per frame: one tick, one frame. `--no-sync` keeps the old passive
behaviour, and if another client already owns the clock this one observes
instead (the clock rule the spawn skills follow).

This is not a nicety. Capturing as a passive observer **wedges the recording
client** on this build. Measured against a healthy server, with the world
ticking and a second client still able to pull frames from the very same
sensor:

| write path | frames before it froze |
|---|---|
| `save_to_disk` inside the `listen` callback, HD | 396 |
| same, 720p | 147 |
| `save_to_disk` moved to the main loop, queued | 222 |
| own cv2 encode from `raw_data` | 248 |
| lock-step, client owns the clock | **401 of 401, no stall** |

Every observer variant froze in `futex_wait` with no error and no further
I/O — so the write path was never the cause, the delivery model was. Replacing
`save_to_disk` and then the encoder was wasted effort; owning the clock fixed
it outright, and matches the pattern that captured 5388 frames for a video on
this same build.

Two consequences worth knowing:

* **Timing is exact.** 401 frames over 20.00 s of simulated time at 20 Hz,
  nothing dropped or duplicated, and the run is reproducible.
* **`save_to_disk` is still avoided** for camera frames: the buffer is
  converted and written here (cv2, falling back to PIL). That change did not
  fix the stall on its own, but it keeps the encode where its cost is visible.

The duration budget is read off the world clock (`elapsed_seconds`), not summed
from per-frame deltas: summing counts *observed* frames, so a consumer slower
than the server takes minutes to "reach" 20 seconds.

## Live Traffic Manager traffic and a lock-step capture are incompatible

Measured on 0.10.0: with the camera owning a 20 Hz clock and a
[[spawn-vehicles]] holder attached as an observer, **all 40 vehicles stood
still** for the whole 500-frame capture. The Traffic Manager lives in the
process that created it and only steps when *that* process ticks the world, so
a TM held by one client and a clock owned by another never advances. Traffic
moved in earlier tests only because the holder was itself the ticker — and a
holder that ticks forces this capture into observer mode, which is what wedges
it.

So pick one:

| what you want | how |
|---|---|
| a capture with moving traffic | record the traffic first, then **replay** it into a lock-step capture. The replayer is server-side, needs no TM, and moved 33 of 50 replayed vehicles under the camera's clock |
| live TM traffic on screen, no frame capture | let the traffic holder own the clock and watch it in the window |
| a capture of a static scene | camera alone, as above |

The replay recipe, in order:

```bash
# 1. record the traffic while its holder owns the clock
python3 ../spawn-vehicles/scripts/vehicles.py spawn --count 50 --base-type car --hold &
python3 ../record-simulation/scripts/record.py clip --file /tmp/traffic.log --seconds 20

# 2. stop the holder, clear the scene, spawn the camera
# 3. capture in lock-step while the replay feeds the actors
python3 scripts/read_sensor.py save --id <cam> --out ./frames --seconds 20 &
python3 ../replay-recording/scripts/replay.py play --file /tmp/traffic.log --duration 20 --keep-scene
```

`--keep-scene` matters in step 3: without it the replay skill stops holders and
puts the world back to asynchronous, which takes the clock away from the
capture.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Make sure the sensor exists (create-sensor) and the world is ticking
- [ ] Step 3: info (sanity) → show (window) or save (dataset)
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Read

```bash
source scripts/env.sh

# quick sanity: one reading + summary
python3 scripts/read_sensor.py info --id 123
# or by the actor it's on / by type
python3 scripts/read_sensor.py info --attached-to hero --type camera.rgb

# LIVE WINDOW (cameras + lidar); 0 = until you close it
python3 scripts/read_sensor.py show --id 123 --seconds 30

# SAVE a dataset: PNGs for cameras, .ply for lidar, JSONL for imu/gnss/…
python3 scripts/read_sensor.py save --id 123 --out ./capture --seconds 10
python3 scripts/read_sensor.py save --id 123 --out ./capture --frames 50

# MULTI-SENSOR: tile several cameras/lidars in one window
python3 scripts/read_sensor.py grid --ids 123,124,125 --seconds 30

# ROS 2: which topics does this sensor publish, and is it actually publishing?
python3 scripts/read_sensor.py ros-info --id 123
```

### Reading it from ROS 2 instead

On a server started with `--ros2` the sensor also publishes DDS topics, and
`ros-info` reports them without needing ROS 2 installed — it derives the names
the way the server does and reads back `is_enabled_for_ros()`:

```
sensor id=123 (sensor.camera.rgb)
  ros_name='front' frame_id='front' parent_frame=hero
  rt/carla/hero/front/image        [sensor_msgs/Image]       qos=best_effort, volatile, depth=1   (ROS node sees /carla/hero/front/image)
  rt/carla/hero/front/camera_info  [sensor_msgs/CameraInfo]  ...
  rt/tf: yes
  enabled_for_ros=NO — this sensor is NOT publishing. Fix with: ...
```

Then, from a ROS 2 environment on the **same domain** — and with the checkout's
RMW profile exported, or nothing arrives (see below):

```bash
set +u; source /opt/ros/humble/setup.bash          # setup.bash breaks under set -u
export FASTRTPS_DEFAULT_PROFILES_FILE=$CARLA_UE4_ROOT/PythonAPI/examples/ros2/config/fastrtps-profile.xml
ros2 topic echo --once /carla/hero/front/camera_info
ros2 topic hz /carla/hero/front/image        # rate sanity, cheaper than echo
```

Points to know, all verified against a live server:

- **A local subscriber without that RMW profile gets nothing.** `topic list` shows
  the topics, `hz` prints nothing, no error appears anywhere: CARLA's Fast DDS is
  built with shared memory and a stock ROS 2 install does not match it. The
  profile forces UDP-only. `visualize-ros-rviz local-env` prints the exports.
- **`enabled_for_ros=NO` means silence, not a slow topic.** Enable it with
  [[create-sensor]] `ros --id N` (or `--ros` at spawn).
- **Image and point-cloud topics are best-effort** (`PublisherQos::SensorData`),
  history depth 1: a late or slow subscriber loses frames rather than stalling the
  server. IMU/GNSS/collision are reliable.
- **Durability is transient_local on fastdds — on every topic**, not just
  `rt/carla/map`: the middleware only ever raises durability and Fast DDS's default
  writer QoS is already transient_local. Do not rely on "volatile" anywhere.
- **Rates follow the tick, not the sensor.** With `sensor_tick` unset on an async
  `-nullrhi` server the lidar measured ~2.7 kHz. Set `sensor_tick` or use sync
  mode for realistic rates.
- **Stamps are simulation time**, driven by the same clock as `rt/clock` — run
  subscribers with `use_sim_time` or every timestamp looks wrong.
- **This skill's `show`/`save` and a ROS subscriber can run at once**; they are
  independent consumers of the same sensor.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

Reading sensor data is unchanged. Two notes:

**`ros-info` no longer lists `rt/carla/map`** — that publisher does not exist on
0.10.0 (see [[world-data]]).

**`enable_for_ros` and friends moved class**, from `carla.Actor` on 0.9.x to
`carla.ServerSideSensor` on 0.10.0. A spawned sensor is a `ServerSideSensor` on
both versions, so calls on the actor handle work either way; they are simply not
on the `carla.Sensor` base class, which is what you get if you introspect the
wrong type.

New on 0.10.0 and readable by this skill without changes: the fisheye camera
variants and `sensor.lidar.hss_lidar` (both UE5 lines), plus
`sensor.camera.rt_lens`, `sensor.other.autoware_gnss` and
`sensor.other.vehicle_status` (**UE 5.8 only**)
([[create-sensor]] lists them). `ServerSideSensor.send()` is new, for pushing
custom V2X messages.

## Examples

**Example 1: see the ego's camera**

User says: "show me the ego's camera"

`create-sensor spawn --type camera.rgb --attach-to hero` (note the id), then
`read_sensor.py show --id <id>`. A window opens with the live feed.

**Example 2: capture a short dataset**

User says: "save 5 seconds from the front camera"

`save --id <id> --out ./front --seconds 5` → numbered PNGs in ./front.

**Example 3: what is the GNSS reading?**

User says: "what's the GPS saying?"

`info --type other.gnss --attached-to hero` → one line with lat/lon/alt.

## Troubleshooting

**Problem: `info` times out / no data**
Cause: the world isn't advancing (sync mode, no ticks) or `sensor_tick` is large.
Solution: tick the world (set-world-settings) or run async; lower `sensor_tick`.

**Problem: `show` errors about display / no window**
Cause: no display (headless), or pygame missing.
Solution: run where there's a display; on a headless box use `save` instead. The
window is client-side (independent of the server's own rendering).

**Problem: depth/semantic image looks wrong when saved raw**
Cause: those need a colour converter.
Solution: this skill auto-applies LogarithmicDepth / CityScapes for save and show.

**Problem: `ros2 topic echo` prints nothing but `info` here shows data**
Cause: the sensor ticks because *this* skill is listening; it is not enabled for
ROS, or the subscriber is on a different `ROS_DOMAIN_ID`.
Solution: `ros-info` (reports both), then [[create-sensor]] `ros --id N`; match
the domain the server was started with ([[run-carla-server]]).

**Problem: ROS timestamps look decades off / TF complains about the future**
Cause: the subscriber uses wall time; CARLA stamps simulation time.
Solution: run subscribers with `use_sim_time` and let them follow `/clock`.

**Problem: data stops when the command ends**
Cause: listening is a callback in this process; it ends with the command.
Solution: keep `show`/`save` running for the capture; the sensor itself persists.

## Outputs

- `save`: PNGs (cameras), `.ply` (lidar), or `data.jsonl` (imu/gnss/radar/events)
  under `--out`.
- `show`: a live window (no files).
- `info`: one summary line.

Detail (callbacks vs queues, sync-mode alignment, save formats, converters) in
[references/read-sensor.md](references/read-sensor.md).
