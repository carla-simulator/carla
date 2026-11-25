#!/usr/bin/env python3
"""Listen to a CARLA sensor and save its data, show it in a window, or summarise it.

Select an existing sensor (spawn one with the create-sensor skill first):
    --id N | --type sensor.camera.rgb | --attached-to hero

Commands:
    info   [--seconds 5]                    grab one reading and print a summary
    save   --out DIR [--seconds 10] [--frames N]   write frames to DIR
    save   --ids 12,13,14 | --attached-to hero --all  --out DIR
                                            several sensors frame-aligned: DIR/<id>_<type>/ each,
                                            and DIR/frames.jsonl mapping every frame to its files
    info   --ids 12,13,14 | --attached-to hero --all   one reading from each
    show   [--seconds 20]                   live window (cameras + lidar)
    grid   --ids 12,13,14 [--seconds 20]    tile several sensors in one window
    ros-info                                the ROS 2 topics this sensor
                                            publishes, their QoS, and whether it
                                            is actually enabled for ROS

save writes cameras as PNG (depth→logarithmic, semantic→CityScapes palette),
lidar as .ply, and other sensors (imu/gnss/radar/…) as rows in data.jsonl.
show opens a pygame window: camera images directly, lidar as a top-down scatter;
non-visual sensors stream their readings to the console instead. It runs for
--seconds (0 = until the window is closed).

Listening is a callback in THIS process — the data only flows while the command
runs. See references/read-sensor.md for callbacks-vs-queue and sync alignment.
Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import json
import queue
import math
import os
import sys
import threading
import time

import carla  # provided by the active interpreter; check_env.sh verifies this
import numpy as np


def _fresh(world):
    """A world handle that already holds a snapshot.

    In synchronous mode a freshly connected client has not seen a frame yet, so
    `get_actors()` comes back EMPTY and every --id/--filter lookup reports "no
    matching actor" while the scene is full of them. Observed against a live
    world holding 48 vehicles. One frame of waiting is the whole fix, and it
    must be a wait rather than a tick: another client owns that clock.
    """
    if world.get_settings().synchronous_mode:
        world.wait_for_tick()
    return world


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


def _resolve(world, args) -> carla.Actor:
    actors = world.get_actors()
    if args.id is not None:
        s = actors.find(args.id)
        if s is None or not s.type_id.startswith("sensor."):
            raise SystemExit(f"no sensor with id {args.id}")
        return s
    sensors = list(actors.filter("sensor.*"))
    if args.attached_to:
        sensors = [s for s in sensors if s.parent is not None
                   and s.parent.attributes.get("role_name", "") == args.attached_to]
    if args.type:
        sensors = [s for s in sensors if s.type_id == (args.type if args.type.startswith("sensor.")
                                                       else f"sensor.{args.type}")]
    if not sensors:
        raise SystemExit("no matching sensor (create one with create-sensor, "
                         "or check --id/--type/--attached-to)")
    if len(sensors) > 1:
        print(f"note: {len(sensors)} sensors matched; using id={sensors[0].id} ({sensors[0].type_id})")
    return sensors[0]


def _resolve_many(world, args):
    """Several sensors: --ids a,b,c, or --all with --attached-to/--type; otherwise the single _resolve()."""
    actors = world.get_actors()
    if getattr(args, "ids", None):
        out = []
        for i in (int(v) for v in args.ids.split(",")):
            s = actors.find(i)
            if s is None or not s.type_id.startswith("sensor."):
                raise SystemExit(f"no sensor with id {i}")
            out.append(s)
        return out
    if getattr(args, "all", False):
        sensors = list(actors.filter("sensor.*"))
        if args.attached_to:
            sensors = [s for s in sensors if s.parent is not None
                       and s.parent.attributes.get("role_name", "") == args.attached_to]
        if args.type:
            full = args.type if args.type.startswith("sensor.") else f"sensor.{args.type}"
            sensors = [s for s in sensors if s.type_id == full]
        if not sensors:
            raise SystemExit("no matching sensors for --all")
        return sorted(sensors, key=lambda s: s.id)
    return [_resolve(world, args)]


_EVENT_SENSORS = ("sensor.other.collision", "sensor.other.lane_invasion", "sensor.other.obstacle")


def _cc(type_id):
    if "depth" in type_id:
        return carla.ColorConverter.LogarithmicDepth
    if "semantic_segmentation" in type_id:
        return carla.ColorConverter.CityScapesPalette
    return carla.ColorConverter.Raw


def _describe(data) -> dict:
    """Key fields per measurement type — for info and jsonl saving."""
    d = {"frame": data.frame, "t": round(data.timestamp, 4)}
    if isinstance(data, carla.DVSEventArray):
        ev = _dvs_events(data)
        d.update(kind="dvs", w=data.width, h=data.height, events=int(len(ev)), positive=int(ev["pol"].sum()))
    elif isinstance(data, carla.Image):
        d.update(kind="image", w=data.width, h=data.height, fov=data.fov)
    elif isinstance(data, (carla.LidarMeasurement, carla.SemanticLidarMeasurement)):
        d.update(kind="lidar", points=len(data))
    elif isinstance(data, carla.RadarMeasurement):
        d.update(kind="radar", detections=len(data))
    elif isinstance(data, carla.IMUMeasurement):
        a, g = data.accelerometer, data.gyroscope
        d.update(kind="imu", accel=[round(a.x, 3), round(a.y, 3), round(a.z, 3)],
                 gyro=[round(g.x, 3), round(g.y, 3), round(g.z, 3)], compass=round(data.compass, 3))
    elif isinstance(data, carla.GnssMeasurement):
        d.update(kind="gnss", lat=data.latitude, lon=data.longitude, alt=data.altitude)
    elif isinstance(data, carla.ObstacleDetectionEvent):
        d.update(kind="obstacle", other=data.other_actor.type_id, distance=round(data.distance, 2))
    elif isinstance(data, carla.CollisionEvent):
        n = data.normal_impulse
        d.update(kind="collision", other=data.other_actor.type_id,
                 impulse=round((n.x**2 + n.y**2 + n.z**2) ** 0.5, 2))
    elif isinstance(data, carla.LaneInvasionEvent):
        d.update(kind="lane_invasion", markings=[str(m.type) for m in data.crossed_lane_markings])
    else:
        d.update(kind=data.__class__.__name__)
    return d


def _camera_rgb(image) -> np.ndarray:
    a = np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))
    return a[:, :, :3][:, :, ::-1]  # BGRA -> RGB


# One DVS event as packed in raw_data; numpy reads it without building a Python list per event.
_DVS_DTYPE = np.dtype([("x", np.uint16), ("y", np.uint16), ("t", np.int64), ("pol", np.bool_)])


def _dvs_events(data) -> np.ndarray:
    return np.frombuffer(data.raw_data, dtype=_DVS_DTYPE)


def _dvs_rgb(data) -> np.ndarray:
    """Events as an image: positive polarity blue, negative red, as DVS viewers draw them."""
    ev = _dvs_events(data)
    img = np.zeros((data.height, data.width, 3), np.uint8)
    img[ev["y"], ev["x"], 2 * ev["pol"].astype(np.int64)] = 255
    return img


def _lidar_topdown(measure, size=600, rng=50.0) -> np.ndarray:
    """Top-down scatter of a lidar sweep into an RGB image (rng metres half-extent)."""
    pts = np.frombuffer(measure.raw_data, dtype=np.float32).reshape(-1, 4)[:, :2]
    img = np.zeros((size, size, 3), np.uint8)
    px = ((pts / rng) * (size / 2) + (size / 2)).astype(np.int32)
    ok = (px[:, 0] >= 0) & (px[:, 0] < size) & (px[:, 1] >= 0) & (px[:, 1] < size)
    img[px[ok, 1], px[ok, 0]] = (255, 255, 255)
    return img


def _render_frame(data, type_id, cc):
    """Measurement -> RGB array for cameras (converted) and lidar; None otherwise."""
    if type_id == "sensor.camera.dvs":
        return _dvs_rgb(data)
    if type_id.startswith("sensor.camera"):
        data.convert(cc)
        return _camera_rgb(data)
    if "lidar" in type_id:
        return _lidar_topdown(data)
    return None


def _stop_and_drain(world, sensors, frames=None):
    """Stop listening, then let callbacks already in flight run out.

    stop() returns while a callback can still be executing on the client's
    streaming thread. Tearing the process down under it is what left `info` on an
    RGB camera hanging at exit after it had printed its result. One more frame
    lets those callbacks finish; the queue is emptied so nothing still holds
    sensor data when the client goes away.
    """
    for s in sensors:
        try:
            s.stop()
        except RuntimeError:
            pass
    try:
        if _OWNS_CLOCK:
            world.tick()
        else:
            world.wait_for_tick(2.0)
    except RuntimeError:
        pass   # a stalled clock: nothing more is coming anyway
    if frames is not None:
        while True:
            try:
                frames.get_nowait()
            except queue.Empty:
                break


# ---- commands --------------------------------------------------------------

def cmd_info(args):
    world = _fresh(_client().get_world())
    sensors = _resolve_many(world, args)
    if len(sensors) > 1:
        return _info_many(world, sensors, args)
    sensor = sensors[0]
    got = {"data": None}
    ev = threading.Event()

    def cb(data):
        got["data"] = data
        ev.set()

    sensor.listen(cb)
    ok = ev.wait(args.seconds)
    _stop_and_drain(world, [sensor])
    if not ok:
        raise SystemExit(f"no data from id={sensor.id} within {args.seconds}s "
                         "(is the world ticking? sensor_tick too high?)")
    print(f"sensor id={sensor.id} ({sensor.type_id}):")
    print(f"  {_describe(got['data'])}")


def _info_many(world, sensors, args):
    got = {s.id: None for s in sensors}
    for s in sensors:
        s.listen(lambda data, sid=s.id: got.__setitem__(sid, data) if got[sid] is None else None)
    deadline = time.time() + args.seconds
    while time.time() < deadline and any(got[s.id] is None for s in sensors if s.type_id not in _EVENT_SENSORS):
        time.sleep(0.05)
    _stop_and_drain(world, sensors)
    print(f"{len(sensors)} sensors:")
    for s in sensors:
        d = got[s.id]
        if d is None:
            note = "no event (fires only on a collision/crossing/obstacle)" if s.type_id in _EVENT_SENSORS else \
                f"no data within {args.seconds}s"
            print(f"  id={s.id:6d} {s.type_id:38s} {note}")
        else:
            print(f"  id={s.id:6d} {s.type_id:38s} {_describe(d)}")


# --- waiting on the simulation, not on the wall clock ----------------------
# These commands are observers: they never own the world's clock, so they
# advance by waiting for whoever does (the server itself in async, the holding
# client in sync). A fixed time.sleep() is wrong for both -- it measures wall
# time while the thing being waited for is measured in frames, so it overruns
# a fast server and cuts a slow one short, and in a synchronous world with a
# stalled ticker it "succeeds" having observed nothing at all.

def _for_seconds(world, seconds):
    """Yield one snapshot per frame until `seconds` of SIMULATED time pass.

    The budget is read off the world clock (`elapsed_seconds`), not summed from
    per-frame deltas. Summing deltas counts *observed* frames, so a consumer
    slower than the server -- a client encoding PNGs at 4 Hz against a 30 Hz
    server, say -- accumulates 0.03 s per poll and takes minutes to "reach"
    20 seconds. Reading the clock is correct whether or not frames are missed.
    """
    start = None
    while True:
        snapshot = world.wait_for_tick()
        now = snapshot.timestamp.elapsed_seconds
        if start is None:
            start = now
        elif now - start >= seconds:
            return
        yield snapshot


def _until(world, predicate, seconds):
    """Advance frames until predicate() is true; returns whether it became true.

    The budget is simulated seconds, so a stalled clock ends this by timing out
    in wait_for_tick() rather than by silently burning the budget.
    """
    if predicate():
        return True
    for _ in _for_seconds(world, seconds):
        if predicate():
            return True
    return False


_OWNS_CLOCK = False


def _claim_clock(world, delta, allow_sync=True):
    """Own the clock if it is free, so capture is lock-step with the sim.

    Asked for by the clock rule the spawn skills follow: already synchronous
    means another client owns it (observe, never tick); asynchronous means this
    client may take it. For a capture, owning it is strictly better -- one
    tick, one frame, no dropped or duplicated frames, and a fixed timestep so
    the output is reproducible.
    """
    global _OWNS_CLOCK
    previous = world.get_settings()
    if previous.synchronous_mode:
        _OWNS_CLOCK = False
        print("world is already synchronous — another client owns the clock; "
              "capturing as an observer")
        return None
    if not allow_sync:
        _OWNS_CLOCK = False
        return None
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = delta
    world.apply_settings(settings)
    _OWNS_CLOCK = True
    print(f"world was asynchronous -> synchronous, fixed_delta_seconds={delta:g} "
          f"({1.0 / delta:g} Hz), this client ticks and captures in lock-step")
    return previous


def _release_clock(world, previous):
    global _OWNS_CLOCK
    if previous is None:
        return
    world.apply_settings(previous)
    _OWNS_CLOCK = False
    print("world settings restored (asynchronous)")


def _write_png(data, path, cc):
    """Encode a camera image to PNG from its raw buffer."""
    data.convert(cc)
    frame = np.frombuffer(data.raw_data, dtype=np.uint8)
    frame = frame.reshape((data.height, data.width, 4))[:, :, :3]   # BGRA -> BGR
    try:
        import cv2
        cv2.imwrite(path, frame)
        return
    except ImportError:
        pass
    from PIL import Image                      # fallback, no cv2 on this box
    Image.fromarray(frame[:, :, ::-1]).save(path)


def _write_dvs(data, stem):
    """DVS frame: the events as .npy (x, y, t, pol) and a polarity image as .png."""
    np.save(stem + ".npy", _dvs_events(data))
    rgb = _dvs_rgb(data)
    try:
        import cv2
        cv2.imwrite(stem + ".png", rgb[:, :, ::-1])
    except ImportError:
        from PIL import Image
        Image.fromarray(rgb).save(stem + ".png")


def cmd_save(args):
    """Save sensor output, writing from the main thread rather than the callback.

    The obvious implementation -- `data.save_to_disk(...)` straight inside
    `sensor.listen()` -- deadlocks this build's client. Measured twice against
    a healthy server: an HD camera stalled at 396 frames and a 720p one at 147,
    both leaving the process in futex_wait with no further I/O and no error,
    because a PNG encode takes ~0.2 s and the stream's callback machinery
    cannot absorb a callback that slow. The server was fine throughout; it was
    the recording client that hung.

    So the callback does one cheap thing -- put the frame on a queue -- and the
    encoding happens here, in the loop. That is the pattern CARLA's own
    examples use, and the one that recorded four videos on this build without
    stalling.
    """
    world = _fresh(_client().get_world())
    sensors = _resolve_many(world, args)
    if len(sensors) > 1:
        return _save_many(world, sensors, args)
    sensor = sensors[0]
    os.makedirs(args.out, exist_ok=True)
    cc = _cc(sensor.type_id)
    is_img = sensor.type_id.startswith("sensor.camera")
    is_lidar = "lidar" in sensor.type_id
    jsonl = None if (is_img or is_lidar) else open(os.path.join(args.out, "data.jsonl"), "w")

    previous = _claim_clock(world, args.delta, allow_sync=not args.no_sync)
    frames = queue.Queue()
    sensor.listen(frames.put)          # cheap: hand off and return

    def write(data):
        if sensor.type_id == "sensor.camera.dvs":
            _write_dvs(data, os.path.join(args.out, f"{data.frame:08d}"))
        elif is_img:
            # NOT save_to_disk. That function deadlocks this build's client:
            # measured twice with the call inside the listen callback (HD
            # stalled at 396 frames, 720p at 147) and once with it moved to
            # this loop (222 frames), every time leaving the process in
            # futex_wait with no error while a fresh client could still pull
            # frames from the very same sensor — so the hang is inside
            # save_to_disk's own threading, not in the callback or the server.
            # Converting the raw buffer here and writing it ourselves is what
            # the video-capture scripts on this build do, across thousands of
            # frames, without stalling.
            _write_png(data, os.path.join(args.out, f"{data.frame:08d}.png"), cc)
        elif is_lidar:
            data.save_to_disk(os.path.join(args.out, f"{data.frame:08d}.ply"))
        else:
            jsonl.write(json.dumps(_describe(data)) + "\n"); jsonl.flush()

    # Discard the first frames: a camera opens underexposed and its auto
    # exposure ramps. Measured on this build, mean frame luminance went
    # 34 -> 100 -> 114 -> 131 over the first five frames and settled near 145,
    # reaching within 5% of settled by frame 9. Keeping them puts a black
    # flash at the start of every video.
    for _ in range(args.warmup):
        if _OWNS_CLOCK:
            world.tick()
        try:
            frames.get(timeout=args.frame_timeout)
        except queue.Empty:
            break
    if args.warmup:
        print(f"  discarded {args.warmup} warm-up frame(s) for auto exposure")

    saved = 0
    dropped = 0
    first = last = None
    try:
        while True:
            if args.frames and saved >= args.frames:
                break
            if _OWNS_CLOCK:
                world.tick()           # exactly one frame per tick
            try:
                data = frames.get(timeout=args.frame_timeout)
            except queue.Empty:
                print(f"  no frame for {args.frame_timeout:g}s — stopping "
                      "(is the sensor still attached and the world running?)")
                break
            if first is None:
                first = data.timestamp
            last = data.timestamp
            write(data)
            saved += 1
            # The budget is simulated time, read off the frames themselves.
            if not args.frames and last - first >= args.seconds:
                break
            # Encoding is slower than a 30 Hz camera, so the queue grows. Past
            # the cap, drop the oldest rather than run the machine out of RAM:
            # a gap in the capture is recoverable, an OOM is not.
            while frames.qsize() > args.queue_max:
                frames.get_nowait()
                dropped += 1
    finally:
        _stop_and_drain(world, [sensor], frames)
        if jsonl:
            jsonl.close()
        _release_clock(world, previous)
    span = (last - first) if (first is not None and last is not None) else 0.0
    print(f"saved {saved} frames from id={sensor.id} ({sensor.type_id}) to {args.out}")
    print(f"  {span:.2f}s of simulated time, {saved / span if span else 0:.1f} frames/s"
          + (f", {dropped} dropped to keep the queue under {args.queue_max}" if dropped else ""))


def _save_many(world, sensors, args):
    """Capture several sensors frame-aligned: one sub-directory each, and frames.jsonl tying a frame to its files.

    Every streaming sensor must deliver the frame of each tick; event sensors (collision, lane
    invasion, obstacle) are written when they fire. Same queue-then-encode pattern as cmd_save.
    """
    os.makedirs(args.out, exist_ok=True)
    dirs, queues, streaming = {}, {}, []
    for s in sensors:
        dirs[s.id] = os.path.join(args.out, f"{s.id}_{s.type_id[len('sensor.'):].replace('.', '_')}")
        os.makedirs(dirs[s.id], exist_ok=True)
        queues[s.id] = queue.Queue()
        if s.type_id not in _EVENT_SENSORS:
            if float(s.attributes.get("sensor_tick", "0") or 0) > 0:
                print(f"  note: id={s.id} has sensor_tick={s.attributes['sensor_tick']}; it is written when it fires")
            else:
                streaming.append(s)
    jsonl = {s.id: open(os.path.join(dirs[s.id], "data.jsonl"), "w") for s in sensors
             if not (s.type_id.startswith("sensor.camera") or "lidar" in s.type_id)}
    manifest = open(os.path.join(args.out, "frames.jsonl"), "w")

    def write(s, data):
        stem = os.path.join(dirs[s.id], f"{data.frame:08d}")
        if s.type_id == "sensor.camera.dvs":
            _write_dvs(data, stem); return stem + ".npy"
        if s.type_id.startswith("sensor.camera"):
            _write_png(data, stem + ".png", _cc(s.type_id)); return stem + ".png"
        if "lidar" in s.type_id:
            data.save_to_disk(stem + ".ply"); return stem + ".ply"
        jsonl[s.id].write(json.dumps(_describe(data)) + "\n"); jsonl[s.id].flush()
        return os.path.join(dirs[s.id], "data.jsonl")

    previous = _claim_clock(world, args.delta, allow_sync=not args.no_sync)
    for s in sensors:
        s.listen(queues[s.id].put)

    def step():
        if _OWNS_CLOCK:
            world.tick()
            return world.get_snapshot()
        return world.wait_for_tick(args.frame_timeout)

    for _ in range(args.warmup):
        step()
    for q in queues.values():
        while not q.empty():
            q.get_nowait()
    if args.warmup:
        print(f"  discarded {args.warmup} warm-up frame(s) for auto exposure")

    saved = missing = 0
    first = last = None
    try:
        while not (args.frames and saved >= args.frames):
            snap = step()
            row = {"frame": snap.frame, "t": round(snap.timestamp.elapsed_seconds, 4), "files": {}}
            for s in streaming:
                data = None
                try:
                    while data is None or data.frame < snap.frame:
                        data = queues[s.id].get(timeout=args.frame_timeout)
                except queue.Empty:
                    pass
                if data is None or data.frame != snap.frame:
                    missing += 1
                    if data is not None and data.frame > snap.frame:
                        queues[s.id].queue.appendleft(data)
                    continue
                row["files"][s.id] = os.path.relpath(write(s, data), args.out)
            for s in sensors:
                if s in streaming:
                    continue
                while not queues[s.id].empty():
                    d = queues[s.id].get_nowait()
                    row["files"][s.id] = os.path.relpath(write(s, d), args.out)
            manifest.write(json.dumps(row) + "\n"); manifest.flush()
            saved += 1
            first = row["t"] if first is None else first
            last = row["t"]
            if not args.frames and last - first >= args.seconds:
                break
    finally:
        _stop_and_drain(world, sensors)
        for f in list(jsonl.values()) + [manifest]:
            f.close()
        _release_clock(world, previous)
    print(f"saved {saved} frame(s) from {len(sensors)} sensors to {args.out} ({len(streaming)} frame-aligned)")
    print(f"  {(last or 0) - (first or 0):.2f}s of simulated time; frames.jsonl maps each frame to its files"
          + (f"; {missing} sensor-frame(s) missing" if missing else ""))


def _elapsed(latest):
    """Simulated seconds of sensor data seen so far (0 until the first frame)."""
    if "t" not in latest or "t0" not in latest:
        return 0.0
    return latest["t"] - latest["t0"]


def cmd_show(args):
    world = _fresh(_client().get_world())
    sensor = _resolve(world, args)
    is_img = sensor.type_id.startswith("sensor.camera")
    is_lidar = "lidar" in sensor.type_id
    if not (is_img or is_lidar):
        # Non-visual sensor: stream readings to the console instead of a window.
        print(f"id={sensor.id} ({sensor.type_id}) has no image; streaming readings "
              f"for {args.seconds or 10}s (Ctrl-C to stop):")
        sensor.listen(lambda d: print("  ", _describe(d)))
        try:
            for _ in _for_seconds(world, args.seconds or 10):
                pass
        except KeyboardInterrupt:
            pass
        _stop_and_drain(world, [sensor])
        return

    import pygame  # only needed for the window
    cc = _cc(sensor.type_id)
    latest = {"frame": None}
    lock = threading.Lock()

    def cb(data):
        frame = _render_frame(data, sensor.type_id, cc)
        with lock:
            latest["frame"] = frame
            latest.setdefault("t0", data.timestamp)
            latest["t"] = data.timestamp

    sensor.listen(cb)
    pygame.init()
    screen = None
    clock = pygame.time.Clock()
    # Duration in SIMULATED seconds, read off the frames themselves: a viewer
    # bounded by wall time shows a different amount of simulation depending on
    # how loaded the server is. pygame's clock.tick(30) below stays -- that is
    # the window's redraw cap, not a wait on simulation state.
    span = args.seconds or None
    running = True
    while running:
        for e in pygame.event.get():
            if e.type == pygame.QUIT:
                running = False
        with lock:
            frame = latest["frame"]
        if frame is not None:
            if screen is None:
                screen = pygame.display.set_mode((frame.shape[1], frame.shape[0]))
                pygame.display.set_caption(f"{sensor.type_id} id={sensor.id}")
            surf = pygame.surfarray.make_surface(frame.swapaxes(0, 1))
            screen.blit(surf, (0, 0))
            pygame.display.flip()
        clock.tick(30)
        if span is not None and _elapsed(latest) >= span:
            running = False
    _stop_and_drain(world, [sensor])
    pygame.quit()
    print(f"closed window for id={sensor.id} ({sensor.type_id})")


def cmd_grid(args):
    import cv2
    import pygame
    world = _fresh(_client().get_world())
    ids = [int(x) for x in args.ids.split(",")]
    sensors = []
    for i in ids:
        s = world.get_actors().find(i)
        if s is None or not s.type_id.startswith("sensor."):
            raise SystemExit(f"no sensor with id {i}")
        sensors.append(s)

    cw, ch = 400, 300                       # per-cell size
    cols = math.ceil(math.sqrt(len(sensors)))
    rows = math.ceil(len(sensors) / cols)
    latest = {s.id: None for s in sensors}
    seen = {}                    # t0/t of simulated time, for the duration budget
    lock = threading.Lock()

    def mk_cb(s):
        cc = _cc(s.type_id)
        def cb(data):
            f = _render_frame(data, s.type_id, cc)
            if f is not None:
                with lock:
                    latest[s.id] = cv2.resize(f, (cw, ch))
                    seen.setdefault("t0", data.timestamp)
                    seen["t"] = data.timestamp
        return cb

    for s in sensors:
        s.listen(mk_cb(s))
    pygame.init()
    screen = pygame.display.set_mode((cols * cw, rows * ch))
    pygame.display.set_caption(f"{len(sensors)} sensors")
    clock = pygame.time.Clock()
    # Duration in SIMULATED seconds, read off the frames themselves: a viewer
    # bounded by wall time shows a different amount of simulation depending on
    # how loaded the server is. pygame's clock.tick(30) below stays -- that is
    # the window's redraw cap, not a wait on simulation state.
    span = args.seconds or None
    running = True
    while running:
        for e in pygame.event.get():
            if e.type == pygame.QUIT:
                running = False
        canvas = np.zeros((rows * ch, cols * cw, 3), np.uint8)
        with lock:
            for k, s in enumerate(sensors):
                f = latest[s.id]
                if f is not None:
                    r, c = divmod(k, cols)
                    canvas[r*ch:(r+1)*ch, c*cw:(c+1)*cw] = f
        screen.blit(pygame.surfarray.make_surface(canvas.swapaxes(0, 1)), (0, 0))
        pygame.display.flip()
        clock.tick(30)
        if span is not None and _elapsed(seen) >= span:
            running = False
    _stop_and_drain(world, sensors)
    pygame.quit()
    print(f"closed grid of {len(sensors)} sensors")


# --- ROS 2 ------------------------------------------------------------------
# Mirrors LibCarla/source/carla/ros2: the base topic is "rt/carla/<ros_name>",
# nested under the parent's when attached, and each publisher appends its suffix.
# Sensors absent from ROS2::GetOrCreateSensor's switch have no publisher at all.
_CAMERA_TOPICS = (("/image", "sensor_msgs/Image"),
                  ("/camera_info", "sensor_msgs/CameraInfo"))
_CLOUD_TOPICS = (("/point_cloud", "sensor_msgs/PointCloud2"),)
_ROS_UNSUPPORTED = ("other.lane_invasion", "other.obstacle", "other.rss")


def _ros_topics_for(type_id: str, base: str):
    short = type_id[len("sensor."):] if type_id.startswith("sensor.") else type_id
    if short in _ROS_UNSUPPORTED:
        return []
    if short == "camera.dvs":
        return [(base + s, m) for s, m in _CAMERA_TOPICS + _CLOUD_TOPICS]
    if short.startswith("camera."):
        return [(base + s, m) for s, m in _CAMERA_TOPICS]
    if short.startswith("lidar.") or short == "other.radar":
        return [(base + s, m) for s, m in _CLOUD_TOPICS]
    if short == "other.imu":
        return [(base, "sensor_msgs/Imu")]
    if short == "other.gnss":
        return [(base, "sensor_msgs/NavSatFix")]
    if short == "other.collision":
        return [(base, "carla_msgs/CarlaCollisionEvent")]
    return []


def cmd_ros_info(args):
    """What this sensor publishes natively, and whether it is publishing at all.

    Read-only, and answerable without ROS 2 installed: the names are derived from
    the actor's own attributes, exactly as the server derives them.
    """
    world = _fresh(_client().get_world())
    sensor = _resolve(world, args)
    ros_name = sensor.attributes.get("ros_name", "") or f"actor{sensor.id}"
    # ros_frame_id / ros_publish_tf are per-actor attributes on the UE4 line
    # only. The UE5 line has neither: the frame id is the ros name, and rt/tf
    # is switched for the whole world by World.set_publish_tf().
    frame_id = sensor.attributes.get("ros_frame_id", "") or ros_name
    if "ros_publish_tf" in sensor.attributes:
        publish_tf = sensor.attributes["ros_publish_tf"].lower() != "false"
        tf_why = "ros_publish_tf=false"
    else:
        try:
            publish_tf = bool(world.get_publish_tf())
        except AttributeError:          # client without the binding
            publish_tf = True
        tf_why = "World.get_publish_tf() is False"

    base = f"rt/carla/{ros_name}"
    parent = sensor.parent
    if parent is not None:
        parent_ros = parent.attributes.get("ros_name", "") or f"actor{parent.id}"
        base = f"rt/carla/{parent_ros}/{ros_name}"
    # ros_topic_name replaces the whole generated base; the server treats the
    # blueprint id (the attribute's default) as "not set".
    override = sensor.attributes.get("ros_topic_name", "")
    if override and override != sensor.type_id:
        base = "rt" + ("" if override.startswith("/") else "/") + override

    print(f"sensor id={sensor.id} ({sensor.type_id})")
    print(f"  ros_name={ros_name!r} frame_id={frame_id!r} "
          f"parent_frame={'map' if parent is None else (parent.attributes.get('ros_frame_id') or parent.attributes.get('ros_name') or f'actor{parent.id}')}")
    topics = _ros_topics_for(sensor.type_id, base)
    if override and override != sensor.type_id:
        print(f"  ros_topic_name={override!r} overrides the generated topic")
        # With an override the lidar and radar publishers drop their suffix.
        if sensor.type_id.startswith("sensor.lidar.") or sensor.type_id == "sensor.other.radar":
            topics = [(base, m) for _, m in topics]
    if not topics:
        print(f"  NO native publisher for {sensor.type_id} — it never appears on ROS")
    for t, m in topics:
        # Image/point-cloud publishers use the sensor-data profile (best effort);
        # everything else keeps the reliable default. Depth 1 in both cases.
        #
        # Durability is NOT volatile in practice: the middleware only ever RAISES
        # it, and Fast DDS's default writer QoS is already TRANSIENT_LOCAL, so
        # every topic reports transient_local there (verified against a live
        # server). Other RMWs may differ, hence the hedge in the label.
        qos = "best_effort" if t.endswith(("/image", "/camera_info", "/point_cloud")) else "reliable"
        print(f"  {t}  [{m}]  qos={qos}, depth=1, durability=transient_local on fastdds"
              f"   (ROS node sees /{t[3:]})")
    print(f"  rt/tf: {'yes' if publish_tf else f'no ({tf_why})'}")

    # is_listening covers Python listeners only; enable_for_ros is a separate
    # server-side flag, and it is the one that makes a sensor tick for ROS.
    try:
        enabled = sensor.is_enabled_for_ros()
    except AttributeError:      # older client without the binding
        enabled = None
    if enabled is False:
        print("  enabled_for_ros=NO — this sensor is NOT publishing. Fix with:")
        print(f"    python3 -c \"import carla;a=carla.Client('127.0.0.1',2000)."
              f"get_world().get_actors().find({sensor.id});a.enable_for_ros()\"")
    elif enabled is True:
        print("  enabled_for_ros=yes")
    else:
        print("  enabled_for_ros=? (client too old for is_enabled_for_ros)")


def _sel(sp, many=False):
    sp.add_argument("--id", type=int); sp.add_argument("--type"); sp.add_argument("--attached-to")
    if many:
        sp.add_argument("--ids", help="several sensors, comma-separated ids")
        sp.add_argument("--all", action="store_true", help="every sensor matching --attached-to / --type")
    return sp


def main() -> None:
    p = argparse.ArgumentParser(description="Read/save/show a CARLA sensor.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pi = _sel(sub.add_parser("info", help="one reading + summary"), many=True)
    pi.add_argument("--seconds", type=float, default=5.0)
    pi.set_defaults(func=cmd_info)

    ps = _sel(sub.add_parser("save", help="write frames to a directory"), many=True)
    ps.add_argument("--out", required=True)
    ps.add_argument("--seconds", type=float, default=10.0)
    ps.add_argument("--frames", type=int, help="stop after this many frames")
    ps.add_argument("--warmup", type=int, default=15,
                    help="frames to discard before saving, so auto exposure has "
                         "settled. 0 keeps them, and the first frame is dark")
    ps.add_argument("--no-sync", action="store_true",
                    help="capture as a passive observer instead of taking the "
                         "clock when the world is asynchronous")
    ps.add_argument("--delta", type=float, default=0.05,
                    help="fixed_delta_seconds when this client takes the clock "
                         "(default 0.05 = 20 Hz)")
    ps.add_argument("--queue-max", type=int, default=120,
                    help="frames allowed to back up while encoding. Encoding is "
                         "slower than the camera, so past this the oldest are "
                         "dropped: a gap is recoverable, an OOM is not")
    ps.add_argument("--frame-timeout", type=float, default=20.0,
                    help="seconds to wait for a frame before giving up, so a "
                         "detached sensor or a stopped world ends the run "
                         "instead of hanging")
    ps.set_defaults(func=cmd_save)

    psh = _sel(sub.add_parser("show", help="live window (camera/lidar) or console stream"))
    psh.add_argument("--seconds", type=float, default=20.0, help="0 = until window closed")
    psh.set_defaults(func=cmd_show)

    pgr = sub.add_parser("grid", help="tile several sensors in one window")
    pgr.add_argument("--ids", required=True, help="comma-separated sensor ids, e.g. 12,13,14")
    pgr.add_argument("--seconds", type=float, default=20.0, help="0 = until window closed")
    pgr.set_defaults(func=cmd_grid)

    _sel(sub.add_parser("ros-info", help="ROS 2 topics + QoS for this sensor")) \
        .set_defaults(func=cmd_ros_info)

    args = p.parse_args()
    rc = 0
    try:
        args.func(args)
    except SystemExit as e:
        if isinstance(e.code, int):
            rc = e.code
        elif e.code is not None:
            print(e.code, file=sys.stderr)
            rc = 1
    except KeyboardInterrupt:
        rc = 130
    # os._exit, not a normal return: interpreter teardown destroys the client
    # and joins its streaming threads, and with a sensor just stopped that join
    # is where the process hung after printing its result. Everything this
    # script owns is already released above, so skipping teardown loses nothing.
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(rc)


if __name__ == "__main__":
    main()
