# Reading sensors — detail

Detail layer for the `read-sensor` skill.

## Callbacks vs queue draining

`sensor.listen(callback)` registers a function called on a background thread each
time the sensor produces data. This skill uses callbacks:

- `info` waits for the first callback, then stops.
- `save` writes each callback's data to disk.
- `show` stores the latest frame under a lock; the pygame loop renders it.

The alternative pattern (CARLA's `sensor_synchronization.py`) is to push each
reading into a `queue.Queue` from the callback and **drain the queue after each
`world.tick()`** in synchronous mode — that guarantees you consume exactly one
reading per frame, aligned across multiple sensors. Use a queue when you need
frame-exact multi-sensor alignment; callbacks are fine for viewing and loose
capture.

Listening lives in the process that calls `listen()`: data flows only while the
command runs, and `sensor.stop()` (called on exit here) detaches it. The sensor
actor itself persists (create-sensor owns its lifecycle).

## Sync-mode alignment

In asynchronous mode readings arrive as the server ticks (roughly at
`1/sensor_tick`). In synchronous mode a reading is produced per `world.tick()`;
for deterministic, frame-locked capture across sensors, tick the world yourself
(set-world-settings) and drain one reading per sensor per tick. Set each sensor's
`rotation_frequency`/`sensor_tick` so exactly one reading lands per frame.

## Save formats

| Sensor | Saved as |
|---|---|
| camera.rgb / instance / optical_flow / normals | PNG (Raw) |
| camera.depth | PNG (LogarithmicDepth converter) |
| camera.semantic_segmentation | PNG (CityScapesPalette converter) |
| lidar.* | `.ply` point cloud (`save_to_disk`) |
| imu / gnss / radar / collision / lane_invasion / obstacle | rows in `data.jsonl` |

`carla.Image.save_to_disk(path, color_converter)` and
`LidarMeasurement.save_to_disk(path)` are built-in. JSONL rows come from a
per-type field extractor (frame, timestamp, and the key values — accel/gyro for
IMU, lat/lon/alt for GNSS, detection count for radar, impacted actor for events).

## Visualisation (`show`)

- **Cameras**: `raw_data` is a BGRA byte buffer → reshape to (H, W, 4) → drop
  alpha, swap to RGB → pygame surface. depth/semantic are `convert()`-ed first so
  they are colourised, not raw.
- **Lidar**: the sweep's XY points are scattered into a top-down image (white on
  black, ±50 m). A richer view (intensity colouring, BEV) is possible but this is
  the simple default.
- **Other sensors**: no image, so `show` streams `info`-style readings to the
  console instead of opening a window.

The window is a **client-side pygame** window, independent of whether the CARLA
server itself renders — but it still needs a display (`$DISPLAY`). On a headless
machine, use `save`.

## Reference example

CARLA's `manual_control.py` shows the full camera-to-pygame pipeline (and a HUD,
sensor switching, lidar view) — it is a large example; this skill distils the
"pick a sensor, show/save it" core.
