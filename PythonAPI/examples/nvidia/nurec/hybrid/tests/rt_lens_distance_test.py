"""Live check of the path-traced distance AOV (sensor.camera.rt_lens_distance).

The sensor renders through the same lens as sensor.camera.rt_lens but outputs, per pixel, the
Euclidean distance from the camera origin to the primary hit along that pixel's lens ray -- a
real path-tracer AOV (FPostProcessSettings::PathTracingLensAOV -> PathTracer.LensDistance)
resolved by ESceneCaptureSource::SCS_PathTracingLensDistance, with no denoiser, no exposure and
no quantization. It ships one float32 per pixel, in metres (carla.DistanceImage).

Three checks, all against sensor.camera.depth (the raster pinhole ground truth):

  1. perspective, fov 90: every pixel, against the pinhole depth converted from view-space Z to
     Euclidean distance analytically. This isolates the AOV itself.
  2. ftheta (the NuRec / NVIDIA rig pixel-distance -> angle polynomial): each fisheye pixel's ray
     direction is projected into the pinhole camera and compared there. This checks that the AOV
     follows the distorted lens, over the whole frame rather than the ~95% a warp can reach.
  3. a prop spawned at a known distance straight down the optical axis, rendered alone with
     show_only_actor_ids, against the analytic distance to its front face.

usage: rt_lens_distance_test.py [--port 3100] [--width 480] [--height 270]
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

# 7c2cf6cd front wide 120 -- same calibration as rt_lens_ftheta_test.py. The polynomial is in
# pixels of the original 1920x1080 frame, so fx = 1/1920 keeps r in those pixels whatever the
# render resolution is.
POLY = [0.0, 0.0010525245452299714, 5.696191607285073e-08, -1.6163911875644033e-10,
        2.340258791876798e-13, -9.255288139653364e-17]
FULL_W, FULL_H = 1920, 1080
CX_FULL, CY_FULL, THETA_MAX = 958.9856, 753.2674, 1.34617

SKY = 900.0  # anything beyond this is "no hit" in one sensor or the other


def theta_of_r(r):
    return sum(c * r ** i for i, c in enumerate(POLY))


def decode_distance(image):
    assert type(image).__name__ == "DistanceImage", type(image).__name__
    buf = np.frombuffer(image.raw_data, dtype=np.float32)
    assert buf.size == image.height * image.width, (buf.size, image.width, image.height)
    # raw_data is a view into the carla.DistanceImage's own memory (PyMemoryView_FromMemory,
    # no ownership): copy, or the array dangles once the image is released and the next
    # same-sized frame (a 480x270 BGRA depth image is exactly as big) overwrites it.
    return buf.reshape((image.height, image.width)).copy()


def decode_carla_depth(image):
    b = np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))
    b = b.astype(np.float64)
    normalized = (b[:, :, 2] + b[:, :, 1] * 256.0 + b[:, :, 0] * 65536.0) / (256.0 ** 3 - 1.0)
    return normalized * 1000.0


client = carla.Client("localhost", a.port)
client.set_timeout(180.0)
world = client.get_world()
bpl = world.get_blueprint_library()

settings = world.get_settings()
old_sync, old_dt = settings.synchronous_mode, settings.fixed_delta_seconds
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

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

    depth_bp = bpl.find("sensor.camera.depth")
    depth_bp.set_attribute("image_size_x", str(W))
    depth_bp.set_attribute("image_size_y", str(H))
    depth_bp.set_attribute("fov", "90.0")
    pin_z = decode_carla_depth(capture(depth_bp, base))

    # view-space Z -> Euclidean distance for a pinhole of horizontal fov 90
    f_pin = W / (2.0 * np.tan(np.radians(90.0) / 2.0))
    xs = (np.arange(W) + 0.5 - W / 2.0) / f_pin
    ys = (np.arange(H) + 0.5 - H / 2.0) / f_pin
    Xn, Yn = np.meshgrid(xs, ys)
    pin_euclid = pin_z * np.sqrt(1.0 + Xn ** 2 + Yn ** 2)

    # -- 1. perspective ------------------------------------------------------------------
    aov_bp = bpl.find("sensor.camera.rt_lens_distance")
    for k, v in dict(image_size_x=str(W), image_size_y=str(H), fov="90.0",
                     camera_model="perspective").items():
        assert aov_bp.has_attribute(k), "rt_lens_distance has no attribute %s" % k
        aov_bp.set_attribute(k, v)
    aov = decode_distance(capture(aov_bp, base))

    valid = (pin_z < SKY) & (aov < SKY)
    err = np.abs(aov[valid] - pin_euclid[valid])
    print("[1] perspective fov 90, %d/%d pixels hit geometry in both" % (valid.sum(), valid.size))
    print("    |AOV - euclid(sensor.camera.depth)|  median %.4f m  p90 %.4f m" %
          (np.median(err), np.percentile(err, 90)))
    assert np.median(err) < 0.05, "path-traced distance AOV disagrees with the pinhole depth"

    # -- 2. ftheta -----------------------------------------------------------------------
    for k, v in dict(camera_model="ftheta",
                     distortion_coeffs=",".join("%.12g" % c for c in POLY),
                     fx="%.12g" % (1.0 / FULL_W), fy="%.12g" % (1.0 / FULL_H),
                     cx="%.12g" % (CX_FULL / FULL_W), cy="%.12g" % (CY_FULL / FULL_H),
                     theta_max_deg="%.6f" % np.degrees(THETA_MAX)).items():
        aov_bp.set_attribute(k, v)
    fish = decode_distance(capture(aov_bp, base))

    # ray direction of every fisheye pixel, in the OpenCV camera frame (+x right, +y down, +z fwd)
    u = (np.arange(W) + 0.5) / W
    v = (np.arange(H) + 0.5) / H
    U, V = np.meshgrid(u, v)
    px = (U - CX_FULL / FULL_W) / (1.0 / FULL_W)          # pixels of the 1920x1080 calibration
    py = (V - CY_FULL / FULL_H) / (1.0 / FULL_H)
    r = np.hypot(px, py)
    theta = theta_of_r(r)
    # only compare where the ray is inside the pinhole's 90 deg frustum, with margin
    inside = theta < np.radians(38.0)
    sin_t, cos_t = np.sin(theta), np.cos(theta)
    with np.errstate(invalid="ignore", divide="ignore"):
        dx = np.where(r > 0, px / r, 0.0) * sin_t
        dy = np.where(r > 0, py / r, 0.0) * sin_t
    dz = cos_t
    # project into the pinhole camera
    pu = (dx / dz) * f_pin + W / 2.0 - 0.5
    pv = (dy / dz) * f_pin + H / 2.0 - 0.5
    pi = np.rint(pv).astype(np.int64)
    pj = np.rint(pu).astype(np.int64)
    inside &= (pi >= 2) & (pi < H - 2) & (pj >= 2) & (pj < W - 2)
    pi = np.clip(pi, 1, H - 2)
    pj = np.clip(pj, 1, W - 2)

    # The reference is a different lens sampled on a different grid, so a fisheye pixel almost
    # never lands on a pinhole pixel centre: sample the reference bilinearly, and only compare
    # where the pinhole neighbourhood is locally smooth (no silhouette, no grazing surface whose
    # distance changes by more than 1% across a pixel) -- otherwise the residual measures the
    # resampling, not the AOV.
    from numpy.lib.stride_tricks import sliding_window_view
    win = sliding_window_view(pin_euclid, (3, 3))
    spread = np.full_like(pin_euclid, np.inf)
    spread[1:-1, 1:-1] = win.max(axis=(-1, -2)) - win.min(axis=(-1, -2))

    fu = np.clip(pu, 0.0, W - 1.001)
    fv = np.clip(pv, 0.0, H - 1.001)
    j0 = fu.astype(np.int64)
    i0 = fv.astype(np.int64)
    tu = fu - j0
    tv = fv - i0
    ref = ((1 - tv) * ((1 - tu) * pin_euclid[i0, j0] + tu * pin_euclid[i0, j0 + 1]) +
           tv * ((1 - tu) * pin_euclid[i0 + 1, j0] + tu * pin_euclid[i0 + 1, j0 + 1]))

    smooth = spread[pi, pj] < 0.01 * pin_euclid[pi, pj]
    m = inside & smooth & (ref < SKY) & (fish < SKY)
    ferr = np.abs(fish[m] - ref[m])
    rel = ferr / ref[m]
    print("[2] ftheta, %d pixels compared inside the pinhole frustum on locally smooth surfaces"
          % m.sum())
    print("    |AOV(ftheta) - euclid(sensor.camera.depth)|  median %.4f m  p90 %.4f m"
          "   (relative: median %.3f%%  p90 %.3f%%)" %
          (np.median(ferr), np.percentile(ferr, 90),
           100.0 * np.median(rel), 100.0 * np.percentile(rel, 90)))
    assert m.sum() > 2000, "not enough comparable pixels"
    assert np.median(rel) < 0.01, "ftheta distance AOV does not follow the lens"

    # coverage: the whole ftheta frame carries a distance, not just the pinhole's 95%
    hit = float((fish < SKY).mean())
    print("    ftheta frame coverage (pixels with a hit): %.1f%%" % (100.0 * hit))

    # -- 3. prop at a known distance -----------------------------------------------------
    prop_bp = bpl.find("static.prop.trafficcone01")
    fwd = base.get_forward_vector()
    aov_bp.set_attribute("camera_model", "perspective")
    aov_bp.set_attribute("fx", "0.0")
    aov_bp.set_attribute("fy", "0.0")
    aov_bp.set_attribute("cx", "0.5")
    aov_bp.set_attribute("cy", "0.5")
    aov_bp.set_attribute("theta_max_deg", "0.0")
    aov_bp.set_attribute("distortion_coeffs", "")
    rows = []
    for dist_m in (6.0, 12.0, 25.0):
        loc = carla.Location(base.location.x + fwd.x * dist_m,
                             base.location.y + fwd.y * dist_m,
                             base.location.z + fwd.z * dist_m - 2.0)
        prop = world.try_spawn_actor(prop_bp, carla.Transform(loc, base.rotation))
        if prop is None:
            print("    spawn failed at %.1f m" % dist_m)
            continue
        world.tick()
        extent = prop.bounding_box.extent
        aov_bp.set_attribute("show_only_actor_ids", str(prop.id))
        only = decode_distance(capture(aov_bp, base))
        depth_bp.set_attribute("show_only_actor_ids", str(prop.id))
        only_z = decode_carla_depth(capture(depth_bp, base))
        prop.destroy()
        world.tick()

        seen = only[only < SKY]
        only_euclid = only_z * np.sqrt(1.0 + Xn ** 2 + Yn ** 2)
        seen_ref = only_euclid[only_z < SKY]
        if seen.size < 20 or seen_ref.size < 20:
            print("    prop at %.1f m not visible (%d / %d px)" % (dist_m, seen.size, seen_ref.size))
            continue
        # The cone's exact silhouette is mesh detail, so the analytic statement is a bracket: the
        # nearest surface the sensor sees must lie inside the actor's own bounding box, i.e.
        # within +/- extent.x of the distance the prop was placed at. On top of that, compare
        # against sensor.camera.depth rendering the same isolated prop -- that reference is
        # shape-exact.
        near_aov, near_ref = float(seen.min()), float(seen_ref.min())
        inside_box = abs(near_aov - dist_m) <= float(extent.x) + 0.05
        rows.append((dist_m, near_aov, near_ref, inside_box))
        print("    prop at %5.1f m (+/- %.3f m box): nearest AOV %7.3f m, nearest pinhole %7.3f m,"
              " diff %+0.3f m, inside box %s (%d px)"
              % (dist_m, float(extent.x), near_aov, near_ref, near_aov - near_ref,
                 inside_box, seen.size))
    print("[3] prop check over %d distances" % len(rows))
    assert rows, "no prop measurements"
    assert all(r[3] for r in rows), "measured surface is outside the prop's bounding box"
    assert max(abs(r[1] - r[2]) for r in rows) < 0.10, \
        "path-traced distance disagrees with sensor.camera.depth on the same isolated prop"

    print("OK")
finally:
    for s in sensors:
        try:
            s.stop()
            s.destroy()
        except Exception:
            pass
    settings.synchronous_mode = old_sync
    settings.fixed_delta_seconds = old_dt
    world.apply_settings(settings)
