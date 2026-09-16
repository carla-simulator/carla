"""Live check that the path-traced lens sensors show freshly spawned actors completely from the
FIRST delivered frame, in synchronous mode.

Regression test for the "actors appear several ticks late" defect: the renderer compiles a new
ray tracing material pipeline in the background whenever an actor with materials it has not
rendered before shows up, and until then binds the black, untagged PathTracingDefaultOpaqueCHS
to those materials (Renderer/Private/RayTracing/RayTracingMaterialHitShaders.cpp). In synchronous
mode the sensors now make that pipeline creation blocking
(RTLensEngineAdapter::SetBlockingRayTracingPipelineCreation), so every tick's frame is complete.

Method: a pinhole sensor.camera.instance_segmentation (raster, immediate) at the same pose,
fov and resolution as a perspective rt_lens rig gives the per-actor reference silhouette. Two
vehicles + two walkers are spawned in view, then rt_lens + rt_lens_instance + rt_lens_distance
with show_only_actor_ids=<their ids> and shadow_catcher_tags=Roads,Sidewalks,RoadLines. For every
one of the first N ticks the test asserts, per actor visible in the raster reference:

  tag AOV       >= 90 % of the reference pixels carry the actor's id
  distance AOV  >= 80 % of the reference pixels are within 4 m of the actor's distance
  colour        >= 25 % of the reference pixels are not black (dark paint stays dark, but a
                   fallback-shaded actor is exactly black)

then spawns a second batch of actors with other blueprints (new materials, hence a new pipeline)
while the rig keeps capturing, and asserts the first batch stays complete on every tick and the
road tag count never collapses (the pipeline change must not produce an empty frame).

The blueprints are chosen from a run-dependent offset so that repeated runs on a long-lived
server keep presenting materials the server has not rendered yet; pass --offset to pin them.

usage: rt_lens_first_frames_test.py [--port 3100] [--width 960] [--height 540] [--ticks 12] [--offset N]
"""
import argparse
import queue
import time

import numpy as np
import carla

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, default=3100)
ap.add_argument("--width", type=int, default=960)
ap.add_argument("--height", type=int, default=540)
ap.add_argument("--ticks", type=int, default=12)
ap.add_argument("--offset", type=int, default=int(time.time()) % 1000)
a = ap.parse_args()
W, H = a.width, a.height

THING = (12, 13, 14, 15, 16, 18, 19)  # Pedestrians, Rider, Car, Truck, Bus, Motorcycle, Bicycle
LABEL_ROAD = 1
MIN_REF_PIXELS = 150  # smaller silhouettes are not measured (sub-pixel lens differences dominate)


def decode_tags(image):
    """BGRA -> (label, 16-bit actor id); astype copies out of the image's own memory."""
    b = np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))
    return b[:, :, 2].astype(np.int32), b[:, :, 1].astype(np.int32) | (b[:, :, 0].astype(np.int32) << 8)


def colour_of(image):
    return np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))[:, :, :3].copy()


def dist_of(image):
    return np.frombuffer(image.raw_data, dtype=np.float32).reshape((image.height, image.width)).copy()


def drain(q, timeout=120.0):
    """Newest frame in the queue (waits for at least one)."""
    try:
        img = q.get(timeout=timeout)
    except queue.Empty:
        return None
    while True:
        try:
            img = q.get_nowait()
        except queue.Empty:
            return img


client = carla.Client("localhost", a.port)
client.set_timeout(240.0)
world = client.get_world()
bpl = world.get_blueprint_library()
settings = world.get_settings()
old_sync, old_dt = settings.synchronous_mode, settings.fixed_delta_seconds
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

spawned, sensors, failures = [], [], []


def make_sensor(bp_id, attrs, transform):
    bp = bpl.find(bp_id)
    for k, v in attrs.items():
        assert bp.has_attribute(k), "%s has no attribute %s" % (bp_id, k)
        bp.set_attribute(k, str(v))
    q = queue.Queue()
    s = world.spawn_actor(bp, transform)
    sensors.append(s)
    s.listen(q.put)
    return s, q


try:
    spawn = world.get_map().get_spawn_points()[0]
    base = carla.Transform(spawn.location + carla.Location(z=2.0), spawn.rotation)
    fwd, right = base.get_forward_vector(), base.get_right_vector()

    def place(ahead, lateral, up=0.0):
        return carla.Location(base.location.x + fwd.x * ahead + right.x * lateral,
                              base.location.y + fwd.y * ahead + right.y * lateral,
                              base.location.z + fwd.z * ahead + right.z * lateral - 2.0 + up)

    size = dict(image_size_x=W, image_size_y=H, fov=90.0)
    _, q_ref = make_sensor("sensor.camera.instance_segmentation", size, base)
    for _ in range(3):
        world.tick()
    _, before_actor = decode_tags(drain(q_ref))
    ids_before = set(np.unique(before_actor).tolist())

    veh_bps = [b for b in bpl.filter("vehicle.*") if int(b.get_attribute("number_of_wheels")) == 4]
    walk_bps = list(bpl.filter("walker.pedestrian.*"))
    assert veh_bps and walk_bps, "no vehicle/walker blueprints"

    def spawn_batch(offset, slots):
        batch = []
        for i, (kind, ahead, lateral, up) in enumerate(slots):
            pool = veh_bps if kind == "v" else walk_bps
            bp = pool[(offset + i) % len(pool)]
            actor = world.try_spawn_actor(bp, carla.Transform(place(ahead, lateral, up), base.rotation))
            assert actor is not None, "could not spawn %s" % bp.id
            if kind == "v":
                actor.set_simulate_physics(False)
            spawned.append(actor)
            batch.append(actor)
            print("  spawned %-32s id %d" % (bp.id, actor.id))
        return batch

    print("batch A (offset %d)" % a.offset)
    batch_a = spawn_batch(a.offset, [("v", 9.0, -1.5, 0.3), ("v", 16.0, 2.5, 0.3), ("w", 7.0, 3.5, 0.6), ("w", 12.0, -4.5, 0.6)])
    world.tick()
    drain(q_ref)

    rt = dict(size, samples_per_pixel=2, enable_denoiser="false",
              shadow_catcher_tags="Roads,Sidewalks,RoadLines",
              show_only_actor_ids=",".join(str(x.id) for x in batch_a))
    _, q_col = make_sensor("sensor.camera.rt_lens", rt, base)
    _, q_ins = make_sensor("sensor.camera.rt_lens_instance", rt, base)
    _, q_dst = make_sensor("sensor.camera.rt_lens_distance", rt, base)

    cam_loc = base.location
    actor_dist = {x.id: x.get_location().distance(cam_loc) for x in batch_a}

    road_counts = []
    batch_b = None
    n_ticks = a.ticks * 2
    print("tick  frame     dt | per actor: ref px -> tag px / distance-ok px / colour non-black px")
    for tick in range(n_ticks):
        if tick == a.ticks:
            print("batch B (new materials while the rig captures; not in show_only_actor_ids)")
            batch_b = spawn_batch(a.offset + 7, [("v", 22.0, -3.0, 0.3), ("w", 18.0, 5.0, 0.6)])
        t1 = time.time()
        frame = world.tick()
        img_ref, img_col, img_ins, img_dst = drain(q_ref), drain(q_col), drain(q_ins), drain(q_dst)
        dt = time.time() - t1
        for name, img in (("instance_segmentation", img_ref), ("rt_lens", img_col), ("rt_lens_instance", img_ins), ("rt_lens_distance", img_dst)):
            assert img is not None, "tick %d: no frame from %s" % (tick, name)
        assert img_col.frame == img_ins.frame == img_dst.frame == frame, \
            "tick %d: sensor frames %d/%d/%d do not match the tick frame %d" % (tick, img_col.frame, img_ins.frame, img_dst.frame, frame)

        ref_label, ref_actor = decode_tags(img_ref)
        lab, act = decode_tags(img_ins)
        col = colour_of(img_col)
        dst = dist_of(img_dst)
        road_counts.append(int((lab == LABEL_ROAD).sum()))

        new_ids = (set(np.unique(ref_actor[np.isin(ref_label, THING)]).tolist()) - ids_before) - {0}
        parts = []
        for sid in sorted(new_ids):
            m = ref_actor == sid
            n_ref = int(m.sum())
            if n_ref < MIN_REF_PIXELS:
                continue
            n_tag = int((act[m] == sid).sum())
            d_med = float(np.median(dst[m]))
            nearest = min(actor_dist.values(), key=lambda d: abs(d - d_med)) if actor_dist else d_med
            n_dst = int((np.abs(dst[m] - nearest) < 4.0).sum())
            n_col = int((col[m].max(axis=1) > 8).sum())
            # batch B is deliberately outside the show-only list: it must be ABSENT from the AOV
            in_b = batch_b is not None and min(abs(x.get_location().distance(cam_loc) - d_med) for x in batch_b) < \
                min(abs(d - d_med) for d in actor_dist.values())
            if in_b:
                if n_tag > 0.05 * n_ref:
                    failures.append("tick %d: actor id %d (batch B, not in show_only_actor_ids) has %d tag pixels" % (tick, sid, n_tag))
                parts.append("%d(B):%d->%d" % (sid, n_ref, n_tag))
                continue
            parts.append("%d:%d->%d/%d/%d" % (sid, n_ref, n_tag, n_dst, n_col))
            if n_tag < 0.9 * n_ref:
                failures.append("tick %d: actor id %d tag AOV %d of %d reference pixels" % (tick, sid, n_tag, n_ref))
            if n_dst < 0.8 * n_ref:
                failures.append("tick %d: actor id %d distance AOV %d of %d pixels within 4 m of the actor (median %.1f m)" % (tick, sid, n_dst, n_ref, d_med))
            if n_col < 0.25 * n_ref:
                failures.append("tick %d: actor id %d colour %d of %d pixels non-black" % (tick, sid, n_col, n_ref))
        print("%4d %6d %6.2f | %s" % (tick, frame, dt, "  ".join(parts)))
        if not parts:
            failures.append("tick %d: no spawned actor of at least %d pixels in the raster reference" % (tick, MIN_REF_PIXELS))

    med_road = float(np.median(road_counts))
    for tick, n in enumerate(road_counts):
        if n < 0.95 * med_road:
            failures.append("tick %d: road tag count %d collapsed (median %d)" % (tick, n, med_road))

    if failures:
        print("\nFAILURES:")
        for f in failures:
            print("  " + f)
        raise SystemExit(1)
    print("OK: %d actors complete in the tag, distance and colour outputs on every one of %d ticks" % (len(batch_a), n_ticks))
finally:
    for s in sensors:
        try:
            s.stop()
            s.destroy()
        except Exception:
            pass
    for s in spawned:
        try:
            s.destroy()
        except Exception:
            pass
    settings.synchronous_mode = old_sync
    settings.fixed_delta_seconds = old_dt
    world.apply_settings(settings)
