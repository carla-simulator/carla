"""Live check of the path-traced tag AOV (sensor.camera.rt_lens_instance).

The sensor renders through the same lens as sensor.camera.rt_lens but outputs, per pixel, the
CARLA tag of the primary hit in the byte layout sensor.camera.instance_segmentation already uses
(R = semantic label, G = actor id low byte, B = actor id high byte). It is a real path-tracer AOV:
the closest-hit shader reads the same GPUScene custom primitive data ATagger writes
(Carla/Game/Tagger.cpp -> CustomPrimitiveData[1].xy, the field
Engine/Shaders/Private/SegmentationSensor.usf reads), carries it in the material payload, and the
ray generation shader records it into PathTracer.LensTag (R32_UINT), which the capture resolves
through ESceneCaptureSource::SCS_PathTracingLensTag.

Three checks:

  (a) labels and 16-bit ids agree with a pinhole sensor.camera.instance_segmentation for pixels
      inside the pinhole frustum, warped through the f-theta model. Measured on the pixels of
      several freshly spawned vehicles and walkers, whose pinhole 3x3 neighbourhood is uniform,
      so the number measures the AOV rather than the resampling of a silhouette between two
      different lenses.
  (b) determinism: two captures of the same frozen scene are byte-for-byte identical.
  (c) coexistence: rt_lens (colour) + rt_lens_distance + rt_lens_instance in the same tick.

usage: rt_lens_instance_test.py [--port 3100] [--width 480] [--height 270]
"""
import argparse
import queue

import numpy as np
import carla

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, default=3100)
ap.add_argument("--width", type=int, default=480)
ap.add_argument("--height", type=int, default=270)
a = ap.parse_args()

W, H = a.width, a.height

# 7c2cf6cd front wide 120 -- same calibration as rt_lens_ftheta_test.py / rt_lens_distance_test.py
POLY = [0.0, 0.0010525245452299714, 5.696191607285073e-08, -1.6163911875644033e-10,
        2.340258791876798e-13, -9.255288139653364e-17]
FULL_W, FULL_H = 1920, 1080
CX_FULL, CY_FULL, THETA_MAX = 958.9856, 753.2674, 1.34617

FTHETA = dict(camera_model="ftheta",
              distortion_coeffs=",".join("%.12g" % c for c in POLY),
              fx="%.12g" % (1.0 / FULL_W), fy="%.12g" % (1.0 / FULL_H),
              cx="%.12g" % (CX_FULL / FULL_W), cy="%.12g" % (CY_FULL / FULL_H),
              theta_max_deg="%.6f" % np.degrees(THETA_MAX))


def theta_of_r(r):
    return sum(c * r ** i for i, c in enumerate(POLY))


def decode_tags(image):
    """BGRA -> (label, 16-bit actor id), the standard CARLA instance-segmentation layout.

    The astype() calls below copy out of the carla.Image's own memory on purpose: np.frombuffer
    aliases it, so a plain view would become a use-after-free once the image goes out of scope.
    """
    b = np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))
    label = b[:, :, 2].astype(np.int32)
    actor = b[:, :, 1].astype(np.int32) | (b[:, :, 0].astype(np.int32) << 8)
    return label, actor


client = carla.Client("localhost", a.port)
client.set_timeout(180.0)
world = client.get_world()
bpl = world.get_blueprint_library()

settings = world.get_settings()
old_sync, old_dt = settings.synchronous_mode, settings.fixed_delta_seconds
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

spawned = []
sensors = []


def capture(blueprint, transform, ticks=8):
    q = queue.Queue()
    cam = world.spawn_actor(blueprint, transform)
    sensors.append(cam)
    cam.listen(q.put)
    img = None
    for _ in range(ticks):
        world.tick()
        try:
            img = q.get(timeout=30.0)
        except queue.Empty:
            pass
    cam.stop()
    cam.destroy()
    sensors.remove(cam)
    assert img is not None, "no frame from %s" % blueprint.id
    return img


try:
    spawn = world.get_map().get_spawn_points()[0]
    base = carla.Transform(spawn.location + carla.Location(z=2.0), spawn.rotation)
    fwd = base.get_forward_vector()
    right = base.get_right_vector()

    def place(ahead, lateral, up=0.0):
        return carla.Location(base.location.x + fwd.x * ahead + right.x * lateral,
                              base.location.y + fwd.y * ahead + right.y * lateral,
                              base.location.z + fwd.z * ahead + right.z * lateral - 2.0 + up)

    # The 16-bit id in a CARLA instance image is AActor::GetUniqueID() (see
    # ATagger::GetActorLabelColor / SetStencilValue), not the client-side actor id, and it is not
    # exposed to Python. Identify the actors we spawn by differencing the reference image against
    # the same view of the empty scene instead: the "thing"-labelled ids that appear only after
    # the spawn are exactly them.
    ref_bp = bpl.find("sensor.camera.instance_segmentation")
    ref_bp.set_attribute("image_size_x", str(W))
    ref_bp.set_attribute("image_size_y", str(H))
    ref_bp.set_attribute("fov", "90.0")
    before_label, before_actor = decode_tags(capture(ref_bp, base))
    ids_before = set(np.unique(before_actor).tolist())

    veh_bps = [b for b in bpl.filter("vehicle.*") if int(b.get_attribute("number_of_wheels")) == 4]
    walk_bps = list(bpl.filter("walker.pedestrian.*"))
    assert veh_bps and walk_bps, "no vehicle/walker blueprints"

    for i, (ahead, lateral) in enumerate([(9.0, 0.0), (17.0, -3.2), (26.0, 3.2)]):
        v = world.try_spawn_actor(veh_bps[i % len(veh_bps)],
                                  carla.Transform(place(ahead, lateral, 0.3), base.rotation))
        if v is not None:
            v.set_simulate_physics(False)
            spawned.append(v)
    for i, (ahead, lateral) in enumerate([(7.0, 4.2), (13.0, -5.0), (20.0, 5.5)]):
        w_ = world.try_spawn_actor(walk_bps[i % len(walk_bps)],
                                   carla.Transform(place(ahead, lateral, 0.5), base.rotation))
        if w_ is not None:
            spawned.append(w_)

    for _ in range(10):
        world.tick()
    print("spawned %d actors (%d vehicles, %d walkers)"
          % (len(spawned),
             sum(1 for s in spawned if s.type_id.startswith("vehicle")),
             sum(1 for s in spawned if s.type_id.startswith("walker"))))
    assert len(spawned) >= 4, "not enough actors spawned to test with"

    # -- reference: pinhole raster instance segmentation ----------------------------------
    ref_label, ref_actor = decode_tags(capture(ref_bp, base))
    THING_LABELS = (12, 13, 14, 15, 16, 18, 19)  # Pedestrians, Rider, Car, Truck, Bus, Motorcycle, Bicycle
    is_thing = np.isin(ref_label, THING_LABELS)
    spawned_ids = set(np.unique(ref_actor[is_thing]).tolist()) - ids_before
    spawned_ids.discard(0)
    print("visible spawned actors: %d distinct tag ids %s, covering %d reference pixels"
          % (len(spawned_ids), sorted(spawned_ids), int(np.isin(ref_actor, list(spawned_ids)).sum())))
    assert len(spawned_ids) >= 4, \
        "fewer than 4 spawned actors are visible in the reference: %s" % sorted(spawned_ids)

    # -- (a) path-traced tag AOV through the f-theta lens ---------------------------------
    aov_bp = bpl.find("sensor.camera.rt_lens_instance")
    for k, v in {**dict(image_size_x=str(W), image_size_y=str(H)), **FTHETA}.items():
        assert aov_bp.has_attribute(k), "rt_lens_instance has no attribute %s" % k
        aov_bp.set_attribute(k, v)
    q_aov = queue.Queue()
    cam_aov = world.spawn_actor(aov_bp, base)
    sensors.append(cam_aov)
    cam_aov.listen(q_aov.put)
    aov_frames = []
    aov_img = None
    for _ in range(10):
        world.tick()
        try:
            aov_img = q_aov.get(timeout=30.0)
        except queue.Empty:
            continue
        aov_frames.append(np.frombuffer(aov_img.raw_data, np.uint8).copy())
    assert aov_img is not None, "no frame from sensor.camera.rt_lens_instance"
    aov_label, aov_actor = decode_tags(aov_img)

    seen_aov = spawned_ids & set(np.unique(aov_actor).tolist())
    print("[a] AOV frame: %d distinct labels, %d distinct tag ids, %d of the spawned actors "
          "present over %d pixels"
          % (len(np.unique(aov_label)), len(np.unique(aov_actor)), len(seen_aov),
             int(np.isin(aov_actor, list(spawned_ids)).sum())))
    # The f-theta lens packs ~120 deg into the same width the reference gives 90 deg, so a small
    # actor can shrink below a pixel there even though the pinhole reference resolves it. Report
    # what each spawned actor looks like in both so a shortfall is legible rather than a bare
    # assertion failure.
    LABEL_NAME = {12: "pedestrian", 13: "rider", 14: "car", 15: "truck", 16: "bus",
                  18: "motorcycle", 19: "bicycle"}
    for sid in sorted(spawned_ids):
        ref_px = int((ref_actor == sid).sum())
        aov_px = int((aov_actor == sid).sum())
        lbl = int(np.bincount(ref_label[ref_actor == sid]).argmax()) if ref_px else -1
        print("      actor %6d (%-10s): %4d px in the pinhole reference, %4d px in the AOV"
              % (sid, LABEL_NAME.get(lbl, str(lbl)), ref_px, aov_px))
    assert len(seen_aov) >= 3, "the AOV shows fewer than 3 of the spawned actors: %s" % sorted(seen_aov)

    # f-theta pixel -> ray direction (OpenCV camera frame) -> pinhole pixel
    f_pin = W / (2.0 * np.tan(np.radians(90.0) / 2.0))
    u = (np.arange(W) + 0.5) / W
    v = (np.arange(H) + 0.5) / H
    U, V = np.meshgrid(u, v)
    px = (U - CX_FULL / FULL_W) * FULL_W
    py = (V - CY_FULL / FULL_H) * FULL_H
    r = np.hypot(px, py)
    theta = theta_of_r(r)
    inside = theta < np.radians(38.0)
    sin_t, cos_t = np.sin(theta), np.cos(theta)
    with np.errstate(invalid="ignore", divide="ignore"):
        dx = np.where(r > 0, px / r, 0.0) * sin_t
        dy = np.where(r > 0, py / r, 0.0) * sin_t
    dz = cos_t
    pu = (dx / dz) * f_pin + W / 2.0 - 0.5
    pv = (dy / dz) * f_pin + H / 2.0 - 0.5
    pi = np.rint(pv).astype(np.int64)
    pj = np.rint(pu).astype(np.int64)
    inside &= (pi >= 1) & (pi < H - 1) & (pj >= 1) & (pj < W - 1)
    pi = np.clip(pi, 1, H - 2)
    pj = np.clip(pj, 1, W - 2)

    # ids cannot be interpolated, so only compare where the reference is locally uniform --
    # otherwise the number measures sub-pixel misregistration between two different lenses
    from numpy.lib.stride_tricks import sliding_window_view
    key = ref_actor.astype(np.int64) * 256 + ref_label
    win = sliding_window_view(key, (3, 3))
    uniform = np.zeros_like(key, dtype=bool)
    uniform[1:-1, 1:-1] = (win.max(axis=(-1, -2)) == win.min(axis=(-1, -2)))

    on_actor = np.isin(ref_actor[pi, pj], list(spawned_ids))
    m = inside & uniform[pi, pj] & on_actor
    n = int(m.sum())
    assert n > 800, "only %d comparable actor pixels" % n
    id_ok = float((aov_actor[m] == ref_actor[pi, pj][m]).mean())
    label_ok = float((aov_label[m] == ref_label[pi, pj][m]).mean())
    both_ok = float(((aov_actor[m] == ref_actor[pi, pj][m]) &
                     (aov_label[m] == ref_label[pi, pj][m])).mean())
    print("[a] %d actor pixels compared: actor id agreement %.2f%%, label agreement %.2f%%, "
          "both %.2f%%" % (n, 100 * id_ok, 100 * label_ok, 100 * both_ok))
    assert both_ok > 0.98, "path-traced tag AOV disagrees with sensor.camera.instance_segmentation"

    # -- (b) determinism -------------------------------------------------------------------
    # Reuses the single long-lived sensor: on a shared GPU each extra path-traced view costs a
    # shader binding table, so this test creates each one once. The walkers have to go first --
    # their idle animation keeps deforming the mesh every tick, so leaving them in would measure
    # the scene changing rather than the AOV.
    for s in list(spawned):
        if s.type_id.startswith("walker"):
            s.destroy()
            spawned.remove(s)
    still = []
    for _ in range(8):
        world.tick()
        try:
            still.append(np.frombuffer(q_aov.get(timeout=30.0).raw_data, np.uint8).copy())
        except queue.Empty:
            continue
    assert len(still) >= 4, "not enough frames for the determinism check"
    diff = int((still[-3] != still[-1]).sum())
    print("[b] frozen scene (walkers removed, vehicle physics off), two ticks apart: "
          "%d of %d bytes differ" % (diff, still[-1].size))
    assert diff == 0, "tag AOV is not deterministic across ticks"

    # -- (c) coexistence with the colour and distance sensors ------------------------------
    small = dict(image_size_x=str(W // 2), image_size_y=str(H // 2))
    col_bp = bpl.find("sensor.camera.rt_lens")
    for k, v in {**small, **dict(samples_per_pixel="4", enable_denoiser="false"), **FTHETA}.items():
        col_bp.set_attribute(k, v)
    dist_bp = bpl.find("sensor.camera.rt_lens_distance")
    for k, v in {**small, **FTHETA}.items():
        dist_bp.set_attribute(k, v)

    q_col, q_dist = queue.Queue(), queue.Queue()
    cam_col = world.spawn_actor(col_bp, base)
    sensors.append(cam_col)
    cam_col.listen(q_col.put)
    cam_dist = world.spawn_actor(dist_bp, base)
    sensors.append(cam_dist)
    cam_dist.listen(q_dist.put)

    got_col = got_dist = got_inst = None
    for _ in range(12):
        world.tick()
        for q_, setter in ((q_col, "col"), (q_dist, "dist"), (q_aov, "inst")):
            try:
                img = q_.get(timeout=30.0)
            except queue.Empty:
                continue
            if setter == "col":
                got_col = img
            elif setter == "dist":
                got_dist = img
            else:
                got_inst = img
    for c_ in (cam_col, cam_dist):
        c_.stop()
        c_.destroy()
        sensors.remove(c_)
    cam_aov.stop()
    cam_aov.destroy()
    sensors.remove(cam_aov)

    assert got_col is not None and got_dist is not None and got_inst is not None, \
        "not every sensor delivered a frame"
    colour = np.frombuffer(got_col.raw_data, np.uint8).reshape(H // 2, W // 2, 4)[:, :, :3]
    dist = np.frombuffer(got_dist.raw_data, np.float32)
    lab, act = decode_tags(got_inst)
    seen_c = spawned_ids & set(np.unique(act).tolist())
    print("[c] same tick: rt_lens colour mean %.1f, rt_lens_distance median(hits) %.2f m, "
          "rt_lens_instance shows %d of the spawned actors"
          % (colour.mean(), float(np.median(dist[dist < 900])), len(seen_c)))
    assert colour.mean() > 1.0, "colour frame is black"
    assert (dist < 900).any(), "distance frame has no hits"
    assert len(seen_c) >= 2, "instance frame lost the actors"

    print("OK")
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
