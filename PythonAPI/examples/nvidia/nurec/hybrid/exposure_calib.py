"""Exposure calibration of the path-traced CARLA layer against the neural scene.

The CARLA layer is lit from the probe (sun direction + panorama sky light), so the only free radiometric scalar
left is the sensor exposure: `sensor.camera.rt_lens` under `exposure_mode=manual` applies `exposure_compensation`
(EV) before the tonemapper. This module measures that EV so that the proxy road, rendered by CARLA alone (pass A,
shadow catchers only), has the same luminance as the real road in the neural render at the same pixels.

Reference and statistic
-----------------------
* Like with like: a pixel counts as "road in both images" when the CARLA tag AOV says `Roads` (lane markings,
  `RoadLines`, are kept apart), the proxy surface is 4..80 m away, the neural distance agrees with the proxy
  distance (|dE - dC| <= max(0.5 m, 3 %)) so the neural pixel is the road surface too (not a parked car, a
  barrier or the ego hood), the neural opacity is >= 0.9, and neither side is clipped (max channel < 250).
* Both 8-bit images are sRGB-decoded to linear light; luminance = Rec.709 weights. Per pixel r = log2(L_carla /
  L_neural). The per-frame estimate is the median of r over the road set, re-taken after dropping pixels more than
  1 EV from that median (real cast shadows of trees/bridges, which the bare proxy does not have, and real markings
  under a CARLA road pixel land there). The per-scene estimate is the median over frames; `residual_ev_mad` is the
  median absolute deviation over frames and says how stable the match is along the clip.
* The tonemapper compresses, so the 8-bit response to EV is not 2^EV: the solve re-renders pass A at the updated EV
  and iterates (secant on the measured slope, clipped to [0.5, 1.5] EV/EV) until |median r| <= `tol`.
* Second reference: the same statistic on `RoadLines` pixels whose neural counterpart is brighter than 1.4x the
  neural road median (a real marking is there). Road matching absorbs the albedo ratio real asphalt / proxy asphalt
  into the exposure; if the marking EV differs from the road EV, the proxy's marking/asphalt contrast differs from
  the real one and the actors (whose materials are neither) inherit a bias of the order of that difference. The
  road is the reference used because it is the surface the actors stand on, receives the same sun and sky, and
  covers thousands of pixels in every frame; the number is reported so the bias is visible, not hidden.

`frame_stats` (the mask and the statistic) and `solve_ev` (the iteration) are pure numpy and unit-tested without a
server (tests/exposure_calib_unit_test.py); `solve` binds them to the live capture.
"""
import json
import os
import time

import numpy as np

ROADS, ROADLINES = 1, 24            # crp::CityObjectLabel
MIN_ROAD_PX = 2000                  # per frame, below this the frame's road EV is NaN


def srgb_to_linear(x8):
    x = np.asarray(x8, np.float32) / 255.0
    return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4).astype(np.float32)


def luminance(rgb8):
    lin = srgb_to_linear(rgb8)
    return 0.2126 * lin[..., 0] + 0.7152 * lin[..., 1] + 0.0722 * lin[..., 2]


def frame_stats(carla_rgb, tag, dist_c, neural_rgb, dist_e, opa_e, d_min=4.0, d_max=80.0, tol_abs=0.5, tol_rel=0.03, opa_min=0.9, trim_ev=1.0, min_px=MIN_ROAD_PX):
    """Road / marking EV offsets of one frame (CARLA pass A vs neural). Returns (stats dict, road mask); ev fields are NaN
    when fewer than min_px road pixels survive the mask."""
    Lc = luminance(carla_rgb); Le = luminance(neural_rgb)
    geom = (opa_e >= opa_min) & (dist_c > d_min) & (dist_c < d_max) & (np.abs(dist_e - dist_c) <= np.maximum(tol_abs, tol_rel * dist_c))
    valid = geom & (Lc > 1e-3) & (Le > 1e-3) & (carla_rgb.max(-1) < 250) & (neural_rgb.max(-1) < 250)
    r = np.log2(np.maximum(Lc, 1e-6) / np.maximum(Le, 1e-6))
    tag_road = tag == ROADS; road = valid & tag_road
    # how the road set shrinks through the mask, so a frame with too few pixels says which side is at fault
    out = dict(road_px=int(road.sum()), road_ev=float("nan"), road_ev_raw=float("nan"), road_trim_frac=float("nan"), road_px_spread=float("nan"),
               lines_px=0, lines_ev=float("nan"), neural_road_lum=float("nan"), carla_road_lum=float("nan"),
               mask=dict(tag=int(tag_road.sum()), carla_range=int((tag_road & (dist_c > d_min) & (dist_c < d_max)).sum()), neural_agrees=int((tag_road & geom).sum()),
                         carla_lit=int((tag_road & geom & (Lc > 1e-3) & (carla_rgb.max(-1) < 250)).sum()), neural_lit=int((tag_road & geom & (Le > 1e-3) & (neural_rgb.max(-1) < 250)).sum())))
    if road.sum() >= min_px:
        rr = r[road]; m0 = float(np.median(rr)); keep = np.abs(rr - m0) <= trim_ev
        out.update(road_ev_raw=m0, road_ev=float(np.median(rr[keep])), road_trim_frac=float(1.0 - keep.mean()),
                   road_px_spread=float(np.median(np.abs(rr[keep] - np.median(rr[keep])))),
                   neural_road_lum=float(np.median(Le[road])), carla_road_lum=float(np.median(Lc[road])))
        lines = valid & (tag == ROADLINES) & (Le > 1.4 * out["neural_road_lum"])
        out["lines_px"] = int(lines.sum())
        if lines.sum() >= 300:
            rl = r[lines]; ml = float(np.median(rl)); keep = np.abs(rl - ml) <= trim_ev
            out["lines_ev"] = float(np.median(rl[keep]))
    return out, road


def _nanmedian(xs):
    xs = np.array([x for x in xs if np.isfinite(x)], np.float64)
    return float(np.median(xs)) if xs.size else float("nan")


def aggregate(per):
    """Per-scene numbers from the per-frame stats: (road EV median, MAD over frames, markings EV median)."""
    ev = _nanmedian([p["road_ev"] for p in per])
    return ev, _nanmedian([abs(p["road_ev"] - ev) for p in per]), _nanmedian([p["lines_ev"] for p in per])


def solve_ev(measure, ec0=5.0, max_iter=5, tol=0.05, fixed=False, log=print):
    """Secant iteration on the sensor EV. measure(ec) -> list of frame_stats dicts (one per calibration frame) at that EV.
    Returns (ec, calib) with calib serialisable; raises SystemExit when the road statistic cannot be formed."""
    history = []; ec = float(ec0); best = None
    for it in range(1 if fixed else max_iter):
        per = measure(ec)
        if not per: raise SystemExit("exposure calib: no calibration frame was measured (every sensor frame missing)")
        ev, mad, ev_lines = aggregate(per)
        rec = dict(ec=ec, road_ev=ev, road_ev_mad=mad, lines_ev=ev_lines, frames=per)
        history.append(rec)
        px = [p["road_px"] for p in per]
        log(f"exposure calib it{it}: EC {ec:+.3f} EV -> road log2(CARLA/neural) median {ev:+.3f} (MAD over frames {mad:.3f}, "
            f"per frame {' '.join('%+.2f' % p['road_ev'] for p in per)}; px {min(px)}..{max(px)}), markings {ev_lines:+.3f}")
        for p in per:
            if not np.isfinite(p["road_ev"]): log(f"  calib frame {p.get('k', '?')}: {p['road_px']} road px < {MIN_ROAD_PX}; road set through the mask: {p['mask']}")
        if not np.isfinite(ev):
            raise SystemExit(f"exposure calib: only {min(px)}..{max(px)} road pixels per frame shared by the proxy road and the neural scene "
                             f"(need >= {MIN_ROAD_PX}: 4..80 m, distances within max(0.5 m, 3 %), opacity >= 0.9, unclipped); check the scene, sun and catcher tags")
        if best is None or abs(ev) < abs(best["road_ev"]): best = rec
        if abs(ev) <= tol or fixed: break
        slope = 1.0
        if len(history) >= 2 and abs(history[-1]["ec"] - history[-2]["ec"]) > 1e-3:
            slope = float(np.clip((history[-1]["road_ev"] - history[-2]["road_ev"]) / (history[-1]["ec"] - history[-2]["ec"]), 0.5, 1.5))
        ec = ec - ev / slope
    ec_out = best["ec"] if not fixed else float(ec0)
    calib = dict(mode="fixed" if fixed else "auto", exposure_comp=float(ec_out), residual_ev=float(best["road_ev"]), residual_ev_mad=float(best["road_ev_mad"]),
                 lines_ev=float(best["lines_ev"]), reference="road luminance median (linear light), see exposure_calib.py", ec0=float(ec0), tol=tol,
                 iterations=len(history), converged=bool(abs(best["road_ev"]) <= tol), history=[dict(ec=h["ec"], road_ev=h["road_ev"], road_ev_mad=h["road_ev_mad"], lines_ev=h["lines_ev"]) for h in history],
                 per_frame=best["frames"])
    log(f"exposure calib: EC {ec_out:+.3f} EV, residual {best['road_ev']:+.3f} EV (MAD {best['road_ev_mad']:.3f}), markings {best['lines_ev']:+.3f} EV, {len(history)} iteration(s)")
    return ec_out, calib


def solve(world, spawn_cam, capture, arr, dist_arr, cam_tf_at, render_engine, frame_t, rtlens, catcher_tags, ec0=5.0, n_frames=8, max_iter=5, tol=0.05,
          out_dir=None, log=print, fixed=False):
    """Solve the rt_lens exposure_compensation (EV) matching the proxy road to the neural road on the live server.

    world/spawn_cam/capture/arr/dist_arr/cam_tf_at/render_engine: hybrid_video.py's own helpers (synchronous mode, proxy
    world, sun and sky light already set). frame_t: the run's frame times; n_frames of them, spread over the clip, are used.
    rtlens: the rt_lens attribute dict (its exposure_compensation is overridden). fixed=True only measures at ec0.
    Returns (ec, calib) with calib serialisable (also written to out_dir/exposure_calib.json when out_dir is given).
    """
    ks = sorted(set(int(round(x)) for x in np.linspace(0, len(frame_t) - 1, n_frames)))
    t0 = time.time(); neural = {}
    for k in ks:
        _, Tcam = cam_tf_at(frame_t[k]); neural[k] = render_engine(Tcam, frame_t[k])
    log(f"exposure calib: {len(ks)} neural frames {ks} in {time.time() - t0:.1f}s")
    sheets = {}

    def measure(ec):
        attrs = dict(rtlens, exposure_compensation="%.4f" % ec, show_only_tags=catcher_tags)
        tf0, _ = cam_tf_at(frame_t[ks[0]])
        cams = {"rgb": spawn_cam("sensor.camera.rt_lens", tf0, attrs), "tag": spawn_cam("sensor.camera.rt_lens_instance", tf0, attrs),
                "dist": spawn_cam("sensor.camera.rt_lens_distance", tf0, attrs)}
        try:
            # warm up until all three sensors deliver the road (first spawn in a fresh world: ray-tracing PSOs compile for a
            # few ticks and the sensors return empty frames meanwhile; the tag AOV can be ready one or two ticks before the
            # colour and distance), then two more ticks
            n_road = 0; ready = 0
            for i in range(60):
                world.tick(); fr = capture(cams)
                if any(fr.get(x) is None for x in ("rgb", "tag", "dist")): continue
                road = arr(fr["tag"])[:, :, 2] == ROADS; n_road = int(road.sum())
                lit = n_road >= MIN_ROAD_PX and float(np.median(arr(fr["rgb"])[road].max(-1))) > 0 and float(np.median(dist_arr(fr["dist"])[road])) < 1e4
                ready = ready + 1 if lit else 0
                if ready >= 2: log(f"  calib: sensors warmed up after {i + 1} ticks"); break
            else: raise SystemExit(f"exposure calib: {n_road} lit road pixels shared by rt_lens, rt_lens_instance and rt_lens_distance after 60 warm-up ticks (need >= {MIN_ROAD_PX}); the proxy road is not in view")
            for _ in range(2): world.tick(); capture(cams)
            per = []
            for k in ks:
                tf, _ = cam_tf_at(frame_t[k])
                for cm, _ in cams.values(): cm.set_transform(tf)
                world.tick(); fr = capture(cams)
                missing = [x for x in ("rgb", "tag", "dist") if fr.get(x) is None]
                if missing: raise SystemExit(f"exposure calib frame {k}: no frame from the calibration sensor(s) {missing} (synchronous mode delivers every tick)")
                rgb = arr(fr["rgb"])[:, :, 2::-1]; tag = arr(fr["tag"])[:, :, 2]; dc = dist_arr(fr["dist"])
                e_rgb, e_dist, e_opa = neural[k]
                st, road = frame_stats(rgb, tag, dc, e_rgb, e_dist, e_opa); st["k"] = int(k); per.append(st)
                if k == ks[0]: sheets[ec] = (rgb, e_rgb, road)
            return per
        finally:
            for cm, _ in cams.values(): cm.stop()
            world.tick()
            for cm, _ in cams.values(): cm.destroy()
            world.tick()

    ec_out, calib = solve_ev(measure, ec0=ec0, max_iter=max_iter, tol=tol, fixed=fixed, log=log)
    calib["frames"] = ks
    if out_dir:
        json.dump(calib, open(os.path.join(out_dir, "exposure_calib.json"), "w"), indent=1)
        try:
            from PIL import Image
            rgb, e_rgb, road = sheets[ec_out]
            ov = e_rgb.copy(); ov[road] = (0.5 * ov[road] + [0, 127, 0]).astype(np.uint8)
            sheet = np.concatenate([rgb[::3, ::3], e_rgb[::3, ::3], ov[::3, ::3]], axis=1)
            Image.fromarray(sheet).save(os.path.join(out_dir, "exposure_calib.jpg"), quality=85)
        except Exception as e:  # diagnostics only
            log(f"exposure calib: sheet not written ({e})")
    return ec_out, calib
