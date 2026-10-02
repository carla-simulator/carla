"""Live check that sensor.camera.rt_lens delivers deterministically in synchronous mode.

(a) latency: a prop moves 1 m per tick across the view; the rt_lens centroid at tick k must
    correspond to the instance-segmentation mask centroid at tick k (a blocking raster sensor
    at the same pose), not to the one from tick k-1. The two cameras have different lens
    models, so the comparison is which SHIFT of the mask series explains the rt series: a
    cubic rt(k) = f(seg(k - L)) is fitted for each candidate lag L and the median absolute
    residuals compared. The prop follows a triangle wave, so a wrong lag puts two different
    rt positions on the same seg position and no smooth f can absorb it. The same fit is
    also reported against the commanded lateral offset, which is exact ground truth.
(b) drops: one rt_lens callback per world.tick() over N ticks, both with the client waiting
    for each image and free-running (never waiting -- the condition under which an
    asynchronous readback drops).

Masks: the rt_lens frame is show-only, i.e. the prop over black under a sky, so below the
horizon the prop is simply the lit pixels. The instance frame is fully painted, so the prop
is found there by its exact instance code -- identified once as the only code whose centroid
moves across the sweep. Both are restricted to rows below the horizon: the sky has animated
clouds and, covering more than half the frame, they otherwise dominate any difference.
(Parking the prop out of frame to grab a background does not work with show_only_actor_ids:
the capture resolves its show-only list once, and a prop that leaves never re-enters.)

usage: rt_lens_sync_determinism_test.py [--port 3000] [--spp 8] [--ticks 50] [--drop-ticks 200]
"""
import argparse
import queue
import time

import numpy as np
import carla

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, default=3000)
ap.add_argument("--spp", type=int, default=8)
ap.add_argument("--ticks", type=int, default=50)
ap.add_argument("--drop-ticks", type=int, default=200)
ap.add_argument("--skip-drop", action="store_true")
ap.add_argument("--skip-lag", action="store_true")
a = ap.parse_args()

W, H = 1920, 1080
# 7c2cf6cd front wide 120 (same rig as rt_lens_ftheta_test.py)
POLY = [0.0, 0.0010525245452299714, 5.696191607285073e-08, -1.6163911875644033e-10,
        2.340258791876798e-13, -9.255288139653364e-17]
CX, CY, THETA_MAX = 958.9856, 753.2674, 1.34617

RT_ATTRS = dict(
    image_size_x=str(W), image_size_y=str(H), camera_model="ftheta",
    distortion_coeffs=",".join("%.12g" % c for c in POLY),
    fx="%.10f" % (1 / W), fy="%.10f" % (1 / H),
    cx="%.10f" % (CX / W), cy="%.10f" % (CY / H),
    theta_max_deg="%.6f" % np.degrees(THETA_MAX),
    samples_per_pixel=str(a.spp), enable_denoiser="true",
    exposure_mode="manual", exposure_compensation="5.0",
)

FWD = 20.0          # prop distance in front of the camera
LAT = 12.0          # lateral sweep half-width; the prop moves exactly 1 m per tick

cl = carla.Client("localhost", a.port)
cl.set_timeout(120)
w = cl.get_world()
bpl = w.get_blueprint_library()

orig = w.get_settings()
st = w.get_settings()
st.synchronous_mode = True
st.fixed_delta_seconds = 0.05
w.apply_settings(st)

sp = w.get_map().get_spawn_points()[0]
base = carla.Transform(sp.location + carla.Location(z=2.0),
                       carla.Rotation(yaw=sp.rotation.yaw))
# Build the camera basis from the yaw directly rather than through Transform's basis
# accessors, which are known to mirror pitch/roll in this tree. Yaw only here, so the two
# agree, but the explicit form is what the sweep geometry depends on.
_yaw = np.radians(base.rotation.yaw)
FWD_V = np.array([np.cos(_yaw), np.sin(_yaw), 0.0])
RIGHT_V = np.array([-np.sin(_yaw), np.cos(_yaw), 0.0])
UP_V = np.array([0.0, 0.0, 1.0])


def at(f, r, u):
    p = (np.array([base.location.x, base.location.y, base.location.z])
         + FWD_V * f + RIGHT_V * r + UP_V * u)
    return carla.Location(float(p[0]), float(p[1]), float(p[2]))


def lateral(i):
    """Triangle wave: exactly 1 m of lateral motion per tick, never leaves the frame."""
    period = int(4 * LAT)
    j = i % period
    return -LAT + (j if j <= 2 * LAT else period - j)


def gray(img):
    """Max over BGR as uint8 -- enough to separate the prop from the background in both the
    path-traced colour frame and the instance-segmentation id frame."""
    return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((H, W, 4))[:, :, :3].max(2)


def centroids(stack, row_min, absolute):
    """Per-frame horizontal centroid of whatever differs from the run's temporal median.

    Only rows at or below row_min are considered. The sky is the one part of the frame that
    moves on its own (animated clouds, and they cover more than half the image), so without
    the horizon cut the cloud residual dwarfs the prop and every centroid collapses towards
    the image centre. Below the horizon the prop is the only thing that moves.
    """
    band = stack[:, row_min:, :].astype(np.int16)
    # The show-only path-traced frame is sky over black, so below the horizon the prop is
    # simply the lit pixels -- no background model needed, and nothing else can creep in.
    # The instance-id frame has a fully painted road below the horizon, so there the prop is
    # what differs from the run's temporal median.
    bg = 0 if absolute else np.median(band, axis=0)
    out, sizes = [], []
    for frame in band:
        d = np.abs(frame - bg)
        # In absolute mode the ground is not perfectly black -- a thin lit band survives at
        # the horizon and, spanning the full width, would drag the centroid to the image
        # centre. Key off the frame's own peak: the prop is much brighter than that band.
        thr = max(60.0, 0.5 * float(d.max())) if absolute else 25.0
        m = d > thr
        n = int(m.sum())
        sizes.append(n)
        out.append(float(np.median(np.where(m)[1])) if n >= 200 else np.nan)
    return np.array(out), np.array(sizes)


def codes(img):
    """Instance-segmentation pixel packed into one integer, so an actor's id is an exact
    value to compare against rather than a threshold."""
    b = np.frombuffer(img.raw_data, dtype=np.uint8).reshape((H, W, 4))[:, :, :3].astype(np.uint32)
    return (b[:, :, 0] << 16) | (b[:, :, 1] << 8) | b[:, :, 2]


def find_prop_code(stack, row_min, min_px=400):
    """The instance code of the only actor that moves: of the codes that cover a decent area
    in both the first and the middle frame of the sweep, the prop's is the one whose centroid
    moved the furthest. Everything else in the scene is static."""
    first, mid = stack[0, row_min:, :], stack[len(stack) // 2, row_min:, :]
    best_code, best_shift = None, -1.0
    vals, counts = np.unique(first, return_counts=True)
    for code in vals[counts >= min_px]:
        m1, m2 = first == code, mid == code
        if m2.sum() < min_px:
            continue
        shift = abs(float(np.median(np.where(m1)[1])) - float(np.median(np.where(m2)[1])))
        if shift > best_shift:
            best_code, best_shift = int(code), shift
    assert best_code is not None and best_shift > 20.0, \
        "no moving instance found in the segmentation frames (best shift %.1f px)" % best_shift
    return best_code


def code_centroids(stack, row_min, code):
    out, sizes = [], []
    for frame in stack[:, row_min:, :]:
        m = frame == code
        n = int(m.sum())
        sizes.append(n)
        out.append(float(np.median(np.where(m)[1])) if n >= 200 else np.nan)
    return np.array(out), np.array(sizes)


def horizon_row(stack):
    """First row (scanning down) whose temporal-median brightness has dropped to the dark
    ground level -- the show-only frame is sky over black, so this is the horizon."""
    bg = np.median(stack[:min(len(stack), 12)], axis=0)
    rows = bg.mean(axis=1)
    dark = np.where(rows < 0.25 * rows.max())[0]
    return int(dark[0]) + 5 if len(dark) else H // 2


results = {}
prop = rt = seg = None
try:
    # A vehicle, not a static.prop.*: those spawn as StaticMeshActors with Static mobility
    # and simply ignore set_location, so nothing would move. Physics off so set_location
    # teleports it exactly where we ask, once per tick, with no settling.
    prop_bp = bpl.find("vehicle.dodge.charger")
    prop = w.spawn_actor(prop_bp, carla.Transform(at(FWD, 0.0, -2.0), base.rotation))
    prop.set_simulate_physics(False)
    w.tick()

    # Show-only, as the hybrid pipeline runs it: the path tracer then only builds an
    # acceleration structure for the prop, which is what makes a 1080p path-traced capture
    # affordable next to the NuRec engines on the same GPU.
    rt_bp = bpl.find("sensor.camera.rt_lens")
    for k, v in RT_ATTRS.items():
        assert rt_bp.has_attribute(k), "rt_lens has no attribute %s" % k
        rt_bp.set_attribute(k, v)
    rt_bp.set_attribute("show_only_actor_ids", str(prop.id))

    seg_bp = bpl.find("sensor.camera.instance_segmentation")
    seg_bp.set_attribute("image_size_x", str(W))
    seg_bp.set_attribute("image_size_y", str(H))
    seg_bp.set_attribute("fov", "110")

    empty = []
    for _ in range(10):
        t0 = time.perf_counter()
        w.tick()
        empty.append((time.perf_counter() - t0) * 1e3)
    empty_ms = float(np.median(empty))

    rt_q, seg_q = queue.Queue(), queue.Queue()
    rt = w.spawn_actor(rt_bp, base)
    seg = w.spawn_actor(seg_bp, base)
    rt.listen(rt_q.put)
    seg.listen(seg_q.put)

    # Warm up with the prop in view (see the show-only note in the docstring).
    for _ in range(6):
        w.tick()
        rt_q.get(timeout=60)
        seg_q.get(timeout=60)
    while not rt_q.empty():
        rt_q.get_nowait()
    while not seg_q.empty():
        seg_q.get_nowait()

    # --- (a) same-tick correspondence -----------------------------------------
    if not a.skip_lag:
        rt_stack = np.empty((a.ticks, H, W), dtype=np.uint8)
        seg_stack = np.empty((a.ticks, H, W), dtype=np.uint32)
        ids, tick_ms, frame_ms = [], [], []
        for i in range(a.ticks):
            prop.set_location(at(FWD, lateral(i), -2.0))
            t0 = time.perf_counter()
            w.tick()
            t1 = time.perf_counter()
            rt_img = rt_q.get(timeout=60)
            seg_img = seg_q.get(timeout=60)
            t2 = time.perf_counter()
            rt_stack[i] = gray(rt_img)
            seg_stack[i] = codes(seg_img)
            ids.append((rt_img.frame, seg_img.frame))
            tick_ms.append((t1 - t0) * 1e3)
            frame_ms.append((t2 - t0) * 1e3)

        rt_row0 = horizon_row(rt_stack)
        seg_row0 = H // 2 + 20          # pinhole at zero pitch: the horizon is the midline
        rt_c, rt_n = centroids(rt_stack, rt_row0, absolute=True)
        prop_code = find_prop_code(seg_stack, seg_row0)
        seg_c, seg_n = code_centroids(seg_stack, seg_row0, prop_code)
        # Keep only frames where both masks look like the prop. A mask that came out far
        # from the typical prop area caught something else (a sliver of the lit horizon
        # band, or the prop clipped at the frame edge) and its centroid is meaningless.
        def plausible(n):
            med = np.median(n[n > 0]) if (n > 0).any() else 0
            return (n > 0.4 * med) & (n < 2.5 * med)

        good = (np.isfinite(rt_c) & np.isfinite(seg_c)
                & plausible(rt_n) & plausible(seg_n))
        assert good.sum() >= a.ticks // 2, \
            "prop not visible in enough frames (%d/%d; rt px %s, seg px %s)" \
            % (good.sum(), a.ticks, rt_n[:5], seg_n[:5])
        rt_s, seg_s = rt_c[good], seg_c[good]
        step = float(np.median(np.abs(np.diff(rt_s))))
        frame_mismatch = sum(1 for f1, f2 in ids if f1 != f2)

        def lag_residual(L):
            """RMS residual of the best cubic seg -> rt map at this lag.

            Cubic, not linear: the f-theta centroid is (near) linear in angle while the
            pinhole one goes as tan, so a straight line cannot express the correspondence
            even at the correct lag. What no smooth function of seg can express is a wrong
            lag: the prop sweeps back and forth, so each seg position occurs twice with two
            different rt positions, and the fit has to average them.
            """
            x, y = (seg_s[:len(seg_s) - L], rt_s[L:]) if L >= 0 else (seg_s[-L:], rt_s[:L])
            if len(x) < 10:
                return np.inf
            coef = np.polyfit(x, y, 3)
            # Median absolute residual, not RMS: a single frame whose mask caught the wrong
            # thing (the prop clipped at the very edge of the sweep) would otherwise
            # dominate the score and decide the ranking on its own.
            return float(np.median(np.abs(np.polyval(coef, x) - y)))

        lags = {L: lag_residual(L) for L in (-1, 0, 1, 2)}
        best = min(lags, key=lags.get)

        # Same test against the commanded lateral offset, which is exact ground truth
        # rather than another camera's estimate.
        lat_s = np.array([lateral(i) for i in range(a.ticks)])[good]

        def lat_lag_residual(L):
            x, y = (lat_s[:len(lat_s) - L], rt_s[L:]) if L >= 0 else (lat_s[-L:], rt_s[:L])
            if len(x) < 10:
                return np.inf
            coef = np.polyfit(x, y, 3)
            # Median absolute residual, not RMS: a single frame whose mask caught the wrong
            # thing (the prop clipped at the very edge of the sweep) would otherwise
            # dominate the score and decide the ranking on its own.
            return float(np.median(np.abs(np.polyval(coef, x) - y)))

        lat_lags = {L: lat_lag_residual(L) for L in (-1, 0, 1, 2)}
        lat_best = min(lat_lags, key=lat_lags.get)
        print("  series (i, commanded_lat_m, rt_px, seg_px):")
        idx = np.where(good)[0]
        for j in range(min(10, len(rt_s))):
            print("     %2d  %+6.1f  %8.1f  %8.1f"
                  % (idx[j], lateral(idx[j]), rt_s[j], seg_s[j]))
        print("  cubic-fit median residual of rt(k) vs commanded lateral(k-L):")
        for L in sorted(lat_lags):
            print("     L=%+d : %8.2f px%s" % (L, lat_lags[L], "   <== best" if L == lat_best else ""))
        results.update(lat_lags=lat_lags, lat_best_lag=lat_best)

        print("--- (a) same-tick correspondence, spp=%d, %d ticks ---" % (a.spp, a.ticks))
        print("  usable frames %d; rt prop mask %d px median; motion %.1f px/tick; "
              "horizon cut rt y>=%d, seg y>=%d"
              % (int(good.sum()), int(np.median(rt_n)), step, rt_row0, seg_row0))
        print("  cubic-fit median residual of rt(k) vs seg(k-L):")
        for L in sorted(lags):
            print("     L=%+d : %8.2f px%s" % (L, lags[L], "   <== best" if L == best else ""))
        print("  frame-id mismatches rt vs seg: %d/%d" % (frame_mismatch, len(ids)))
        print("  world.tick() alone      : %6.1f ms median (empty world %.1f ms)"
              % (float(np.median(tick_ms)), empty_ms))
        print("  tick + both images ready: %6.1f ms median, %6.1f ms p95"
              % (float(np.median(frame_ms)), float(np.percentile(frame_ms, 95))))
        results.update(lags=lags, best_lag=best, step=step, empty_ms=empty_ms,
                       tick_ms=float(np.median(tick_ms)),
                       frame_ms=float(np.median(frame_ms)),
                       frame_mismatch=frame_mismatch)

        assert frame_mismatch == 0, "rt_lens and instance_segmentation report different frame ids"
        assert step > 5.0, "degenerate: the prop barely moves per tick (%.1f px)" % step
        assert best == 0, "rt_lens image lags the tick by %d frame(s): %s" % (best, lags)
        assert lags[0] < 0.5 * min(lags[1], lags[-1]), \
            "same-tick fit not decisively better than the shifted fits: %s" % lags

    # --- (b) zero drops --------------------------------------------------------
    if not a.skip_drop:
        while not rt_q.empty():
            rt_q.get_nowait()
        n = a.drop_ticks
        drop_ms = []
        delivered = 0
        for i in range(n):
            prop.set_location(at(FWD, lateral(i), -2.0))
            t0 = time.perf_counter()
            w.tick()
            rt_q.get(timeout=60)          # exactly one callback for this tick
            drop_ms.append((time.perf_counter() - t0) * 1e3)
            delivered += 1
        extra = rt_q.qsize()
        print("--- (b) drops over %d ticks, spp=%d ---" % (n, a.spp))
        print("  callbacks: %d consumed one-per-tick, %d left over (dropped %d, duplicated %d)"
              % (delivered, extra, max(0, n - delivered - extra), extra))
        print("  tick + image ready: %6.1f ms median, %6.1f ms p95"
              % (float(np.median(drop_ms)), float(np.percentile(drop_ms, 95))))
        results.update(delivered=delivered, extra=extra, drop_ticks=n,
                       drop_frame_ms=float(np.median(drop_ms)))
        assert extra == 0 and delivered == n, \
            "not exactly one callback per tick: %d consumed, %d extra" % (delivered, extra)

        # Free-running: tick back to back without waiting for each image. Waiting for the
        # image (above) paces the client to the sensor and hides the very condition that
        # makes an asynchronous readback drop frames, so count callbacks over a run that
        # never waits. Counting callback, not a queue: 200 frames of 1080p would be 1.6 GB.
        rt.stop()
        seen = [0]
        rt.listen(lambda image, c=seen: c.__setitem__(0, c[0] + 1))
        t0 = time.perf_counter()
        for i in range(n):
            prop.set_location(at(FWD, lateral(i), -2.0))
            w.tick()
        free_ms = (time.perf_counter() - t0) / n * 1e3
        for _ in range(10):     # let anything still in flight land
            w.tick()
        free_seen = seen[0]
        print("  free-running (no per-tick wait): %d callbacks over %d ticks "
              "(dropped %d), %.1f ms per tick"
              % (free_seen, n, max(0, n - free_seen), free_ms))
        results.update(free_seen=free_seen, free_ms=free_ms)
        assert free_seen >= n, \
            "dropped %d of %d frames when the client does not wait per tick" \
            % (n - free_seen, n)

    print("OK")
finally:
    for actor in (rt, seg):
        if actor is not None:
            try:
                actor.stop()
                actor.destroy()
            except RuntimeError:
                pass
    if prop is not None:
        try:
            prop.destroy()
        except RuntimeError:
            pass
    w.apply_settings(orig)
