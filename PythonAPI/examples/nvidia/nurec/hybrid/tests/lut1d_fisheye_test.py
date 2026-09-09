#!/usr/bin/env python3
"""Exact f-theta rendering on the cube-map fisheye sensors (camera_model=lut).

Reads the f-theta polynomial of a NuRec camera from the engine, tabulates R(theta) in pixels, and
renders `sensor.camera.rgb_fisheye`, `depth_fisheye` and `semantic_segmentation_fisheye` with that
table at the recorded image size. From the same pose, a wide pinhole capture is warped into the same
lens the way hybrid_video.py does, and the two are compared (colour, edge alignment, Euclidean
distance, labels).

usage: lut1d_fisheye_test.py OUT_DIR [--port 3200] [--nre-port 46436] [--scene 7c2cf6cd]
                             [--camera camera_front_wide_120fov] [--lut-samples 256]
                             [--cap 3600,2025,130] [--spawn 0]
"""
import argparse, json, os, queue, sys, time
import numpy as np
from PIL import Image

HYBRID_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NUREC_DIR = os.path.dirname(HYBRID_DIR)
sys.path.insert(0, NUREC_DIR)
import carla  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("--port", type=int, default=3200)
ap.add_argument("--nre-port", type=int, default=46436)
ap.add_argument("--scene", default="7c2cf6cd")
ap.add_argument("--camera", default="camera_front_wide_120fov")
ap.add_argument("--lut-samples", type=int, default=256)
ap.add_argument("--cap", default="3600,2025,130", help="pinhole capture used as the warp source: w,h,hfov")
ap.add_argument("--spawn", type=int, default=0, help="spawn point index the camera stands on")
ap.add_argument("--lens-json", default="", help="skip the engine: read the lens from this json (written by a previous run)")
a = ap.parse_args()
os.makedirs(a.out, exist_ok=True)
cap_w, cap_h, cap_hfov = [float(x) for x in a.cap.split(",")]
cap_w, cap_h = int(cap_w), int(cap_h)


# ---------------------------------------------------------------- lens from the engine
def lens_from_engine():
    import grpc
    from nre.grpc.protos import sensorsim_pb2 as s, sensorsim_pb2_grpc as g, common_pb2 as c
    st = g.SensorsimServiceStub(grpc.insecure_channel(f"localhost:{a.nre_port}"))
    sid = next(x for x in st.get_available_scenes(c.Empty(), timeout=60).scene_ids if a.scene in x)
    cam = {k.logical_id: k for k in st.get_available_cameras(s.AvailableCamerasRequest(scene_id=sid), timeout=900).available_cameras}[a.camera]
    fp = cam.intrinsics.ftheta_param
    return dict(scene=sid, camera=a.camera, w=int(cam.intrinsics.resolution_w), h=int(cam.intrinsics.resolution_h),
                cx=float(fp.principal_point_x), cy=float(fp.principal_point_y),
                poly=[float(x) for x in fp.pixeldist_to_angle_poly], max_angle=float(fp.max_angle))


if a.lens_json:
    FT = json.load(open(a.lens_json))
else:
    FT = lens_from_engine()
    json.dump(FT, open(os.path.join(a.out, "lens.json"), "w"), indent=1)
W, H = FT["w"], FT["h"]
print("lens:", json.dumps(FT), flush=True)


def theta_of_r(r):
    return sum(c_ * r ** i for i, c_ in enumerate(FT["poly"]))


# R(theta) table in pixels: invert the (monotone) pixel->angle polynomial on a dense radius grid.
r_dense = np.linspace(0.0, np.hypot(W, H), 200001)
th_dense = theta_of_r(r_dense)
ok = np.concatenate([[True], np.diff(th_dense) > 0])
r_dense, th_dense = r_dense[ok], th_dense[ok]
theta_lut = np.linspace(0.0, FT["max_angle"], a.lut_samples)
r_lut = np.interp(theta_lut, th_dense, r_dense)
lut_attr = ",".join("%.4f" % v for v in r_lut)
theta_max_deg = np.degrees(FT["max_angle"])
print("LUT: %d samples, r(theta_max=%.2f deg) = %.1f px, first samples %s" % (a.lut_samples, theta_max_deg, r_lut[-1], r_lut[:4].round(2)), flush=True)

# Attributes exactly as the hybrid pipeline must pass them (normalized viewport units, rt_lens convention).
LENS_ATTRS = dict(camera_model="lut", lut=lut_attr, theta_max_deg="%.6f" % theta_max_deg,
                  fx="%.10f" % (1.0 / W), fy="%.10f" % (1.0 / H), cx="%.10f" % (FT["cx"] / W), cy="%.10f" % (FT["cy"] / H),
                  image_size_x=str(W), image_size_y=str(H))
json.dump(LENS_ATTRS, open(os.path.join(a.out, "lens_attrs.json"), "w"), indent=1)


# ---------------------------------------------------------------- warp of a pinhole capture into the lens
def ftheta_warp_table():
    vv, uu = np.mgrid[0:H, 0:W].astype(np.float64)
    dx, dy = uu - FT["cx"], vv - FT["cy"]; r = np.hypot(dx, dy)
    theta = theta_of_r(r); phi = np.arctan2(dy, dx)
    d = np.stack([np.sin(theta) * np.cos(phi), np.sin(theta) * np.sin(phi), np.cos(theta)], axis=-1)
    fc = cap_w / 2 / np.tan(np.radians(cap_hfov) / 2)
    valid = (d[..., 2] > 0.05) & (theta <= FT["max_angle"])
    z = np.where(valid, d[..., 2], 1.0)
    su = fc * d[..., 0] / z + cap_w / 2 - 0.5; sv = fc * d[..., 1] / z + cap_h / 2 - 0.5
    valid &= (su >= 0) & (su <= cap_w - 1) & (sv >= 0) & (sv <= cap_h - 1)
    return su, sv, valid, 1.0 / z, d, theta


def warp(rgb, sem, depth, table):
    from scipy.ndimage import map_coordinates
    su, sv, valid, inv_cos, _, _ = table
    coords = [sv, su]
    out_rgb = np.stack([map_coordinates(rgb[..., c_].astype(np.float32), coords, order=1, mode="nearest") for c_ in range(3)], axis=-1)
    out_sem = map_coordinates(sem, coords, order=0, mode="nearest")
    out_dist = map_coordinates(depth, coords, order=0, mode="nearest") * inv_cos
    out_rgb[~valid] = 0; out_sem[~valid] = 0; out_dist[~valid] = 1e6
    return out_rgb.clip(0, 255).astype(np.uint8), out_sem, out_dist.astype(np.float32)


# ---------------------------------------------------------------- CARLA
client = carla.Client("localhost", a.port); client.set_timeout(120.0)
world = client.get_world()
settings = world.get_settings(); settings.synchronous_mode = True; settings.fixed_delta_seconds = 0.05; world.apply_settings(settings)
bpl = world.get_blueprint_library()
sp = world.get_map().get_spawn_points()[a.spawn]
tf = carla.Transform(sp.location + carla.Location(z=1.8), carla.Rotation(pitch=0.0, yaw=sp.rotation.yaw, roll=0.0))


def spawn_cam(bp_id, attrs):
    bp = bpl.find(bp_id)
    for k, v in attrs.items():
        if bp.has_attribute(k): bp.set_attribute(k, v)
        else: print("  (no attribute %s on %s)" % (k, bp_id))
    if bp.has_attribute("enable_dlss"): bp.set_attribute("enable_dlss", "false")
    q = queue.Queue(); cm = world.spawn_actor(bp, tf); cm.listen(q.put); return cm, q


pin_attrs = dict(image_size_x=str(cap_w), image_size_y=str(cap_h), fov="%.3f" % cap_hfov)
cams = {
    "fe_rgb": spawn_cam("sensor.camera.rgb_fisheye", LENS_ATTRS),
    "fe_depth": spawn_cam("sensor.camera.depth_fisheye", LENS_ATTRS),
    "fe_sem": spawn_cam("sensor.camera.semantic_segmentation_fisheye", LENS_ATTRS),
    "pin_rgb": spawn_cam("sensor.camera.rgb", pin_attrs),
    "pin_depth": spawn_cam("sensor.camera.depth", pin_attrs),
    "pin_sem": spawn_cam("sensor.camera.semantic_segmentation", pin_attrs),
}
print("sensors spawned at", tf, flush=True)


def grab():
    out = {}
    for k, (cm, q) in cams.items():
        img = q.get(timeout=60)
        out[k] = np.frombuffer(img.raw_data, dtype=np.uint8).reshape(img.height, img.width, 4).copy()
    return out


try:
    for _ in range(4):   # let exposure and streaming settle
        world.tick(); fr = grab()
    world.tick(); fr = grab()
finally:
    for cm, _ in cams.values(): cm.stop()
    client.apply_batch([carla.command.DestroyActor(cm.id) for cm, _ in cams.values()]); world.tick()
    settings.synchronous_mode = False; settings.fixed_delta_seconds = None; world.apply_settings(settings)
np.savez_compressed(os.path.join(a.out, "frames.npz"), **fr)   # raw BGRA frames, for offline re-analysis


def depth_m(bgra):
    b, g, r = [bgra[..., i].astype(np.float64) for i in range(3)]
    return (r + g * 256.0 + b * 65536.0) / (256.0 ** 3 - 1) * 1000.0


fe_rgb = fr["fe_rgb"][..., 2::-1]; fe_dist = depth_m(fr["fe_depth"]); fe_sem = fr["fe_sem"][..., 2]
pin_rgb = fr["pin_rgb"][..., 2::-1]; pin_depth = depth_m(fr["pin_depth"]); pin_sem = fr["pin_sem"][..., 2]
table = ftheta_warp_table()
wp_rgb, wp_sem, wp_dist = warp(pin_rgb, pin_sem, pin_depth, table)
valid = table[2]
print("fisheye frame %dx%d, warp covers %.1f %% of it" % (fe_rgb.shape[1], fe_rgb.shape[0], 100 * valid.mean()), flush=True)

# --- invalid region: outside the lens' range the sensor must write black / label 0 / far
vv, uu = np.mgrid[0:H, 0:W]; r_px = np.hypot(uu - FT["cx"], vv - FT["cy"])
outside = r_px > r_lut[-1]
res = dict(lens=FT, lut_samples=a.lut_samples, coverage_pct=float(100 * valid.mean()), outside_px=int(outside.sum()))
if outside.any():
    res["outside_rgb_max"] = int(fe_rgb[outside].max()); res["outside_sem_max"] = int(fe_sem[outside].max())
    res["outside_dist_min_m"] = float(fe_dist[outside].min())

# --- colour agreement (exposure-normalized), edge alignment
def gray(x): return (0.299 * x[..., 0] + 0.587 * x[..., 1] + 0.114 * x[..., 2]).astype(np.float32)
g_fe, g_wp = gray(fe_rgb), gray(wp_rgb)
m = valid & ~outside
scale = np.median(g_wp[m]) / max(np.median(g_fe[m]), 1e-3)
res["mae_rgb_raw"] = float(np.abs(fe_rgb[m].astype(np.float32) - wp_rgb[m].astype(np.float32)).mean())
res["mae_gray_exposure_normalized"] = float(np.abs(g_fe[m] * scale - g_wp[m]).mean())
res["exposure_ratio_warp_over_fisheye"] = float(scale)


def sobel(x):
    from scipy.ndimage import sobel as sb
    return np.hypot(sb(x, 0), sb(x, 1))


e_fe, e_wp = sobel(g_fe * scale), sobel(g_wp)
best = None; grid = {}
for dy in range(-4, 5):
    for dx in range(-4, 5):
        sh = np.roll(np.roll(e_wp, dy, axis=0), dx, axis=1)
        mm = m.copy(); mm[:8] = mm[-8:] = False; mm[:, :8] = mm[:, -8:] = False
        c_ = float(np.corrcoef(e_fe[mm], sh[mm])[0, 1]); grid["%d,%d" % (dx, dy)] = c_
        if best is None or c_ > best[0]: best = (c_, dx, dy)
res["edge_corr_best"] = dict(corr=best[0], shift_px=[best[1], best[2]], corr_at_zero=grid["0,0"])

# --- depth: which convention does depth_fisheye follow?
d = table[4]; theta = table[5]
mm = m & (wp_dist < 500) & (fe_dist < 999)
res["depth_compared_px"] = int(mm.sum())
if mm.any():
    rel = np.abs(fe_dist[mm] - wp_dist[mm]) / np.maximum(wp_dist[mm], 0.5)
    res["depth_vs_euclid_median_rel"] = float(np.median(rel)); res["depth_vs_euclid_p90_rel"] = float(np.percentile(rel, 90))
    # hypothesis 2: planar depth of the cube face the ray fell in (max |component| axis)
    dom = np.argmax(np.abs(d), axis=-1); cosf = np.take_along_axis(np.abs(d), dom[..., None], axis=-1)[..., 0]
    rel_face = np.abs(fe_dist[mm] / np.maximum(cosf[mm], 1e-3) - wp_dist[mm]) / np.maximum(wp_dist[mm], 0.5)
    res["depth_as_faceplanar_median_rel"] = float(np.median(rel_face))
    # hypothesis 3: planar along the optical axis
    rel_axis = np.abs(fe_dist[mm] / np.maximum(np.cos(theta[mm]), 1e-3) - wp_dist[mm]) / np.maximum(wp_dist[mm], 0.5)
    res["depth_as_axisplanar_median_rel"] = float(np.median(rel_axis))

# --- labels
res["sem_agreement_pct"] = float(100 * (fe_sem[m] == wp_sem[m]).mean())

json.dump(res, open(os.path.join(a.out, "result.json"), "w"), indent=1)
print(json.dumps(res, indent=1), flush=True)

# --- pictures
cap = "/home/german/Projects/CARLA_SOURCE/.omc/captures"
os.makedirs(cap, exist_ok=True)
Image.fromarray(fe_rgb).save(os.path.join(cap, "lut1d_fisheye_rgb.png"))
Image.fromarray(wp_rgb).save(os.path.join(cap, "lut1d_warped_pinhole.png"))
blend = (0.5 * fe_rgb.astype(np.float32) * scale + 0.5 * wp_rgb.astype(np.float32)).clip(0, 255).astype(np.uint8)
Image.fromarray(blend).save(os.path.join(cap, "lut1d_blend.png"))
diff = np.abs(g_fe * scale - g_wp); diff[~m] = 0
Image.fromarray((diff.clip(0, 64) * 4).astype(np.uint8)).save(os.path.join(cap, "lut1d_absdiff.png"))
dv = lambda x: (np.log1p(np.clip(x, 0, 300)) / np.log1p(300) * 255).astype(np.uint8)
Image.fromarray(np.concatenate([dv(fe_dist), dv(wp_dist)], axis=1)).save(os.path.join(cap, "lut1d_depth_fisheye_vs_warp.png"))
Image.fromarray(np.concatenate([fe_sem * 7, wp_sem * 7], axis=1).astype(np.uint8)).save(os.path.join(cap, "lut1d_sem_fisheye_vs_warp.png"))
# checker: alternate 64 px tiles from the two colour images; misalignment shows as broken lines
tiles = ((uu // 64 + vv // 64) % 2).astype(bool)
chk = np.where(tiles[..., None], (fe_rgb.astype(np.float32) * scale).clip(0, 255).astype(np.uint8), wp_rgb)
Image.fromarray(chk).save(os.path.join(cap, "lut1d_checker.png"))
print("captures in", cap, flush=True)
os._exit(0)   # the client's sensor threads keep the interpreter alive otherwise
