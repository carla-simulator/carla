"""Illumination probe: six 90-deg pinhole faces rendered by the engine at the rig pose -> equirectangular
panorama in the rig FLU frame (azimuth from +x forward toward +y left, elevation up) -> sun direction,
sky statistics, ground brightness. Writes OUT.json, OUT.jpg (clean panorama, the sky light map input),
OUT_vis.jpg (sun crosshair, candidates, sky mask) and OUT_aux.npz (engine distance / opacity per pixel).
usage: illum_probe.py OUT --scene 7c2cf6cd [--frac 0.5 | --t-us T] [--port 46436] [--face 512]
       illum_probe.py OUT --offline PANO.jpg     re-run the detection on a saved panorama (its .json / _aux.npz next to it)

Sun detection (detect_sun): the recorded cameras saw the band el 5..45 deg, |az| < 135 deg; above it the panorama is
splat noise. Inside the band a *sky* mask is grown from the top rows through pixels that are blue (B/R >= 1.08) or
neutral white (lum >= 160, saturation <= 0.15, B >= 0.97 R: a sunlit facade is warmer than any sky) and smooth
(luminance gradient < 25/px); its lowest elevation per column is the local skyline. Candidates are the bright blobs of
the band (lum >= max(P99.7, 200), the old rule, so a compact sun disc is found) plus the blobs of the smoothed sky
luminance above max(P99 of the sky, 180) (a veiled sun is a broad aureole, not a hot disc). A candidate is accepted
only if it lies in the sky mask, above the local skyline by --skyline-margin, above the ground-plane horizon by
--min-alt, and -- when the engine's distance/opacity are available -- is not an opaque surface closer than
--surface-max-m. The sky's own brightness gradient is fitted independently (least squares of the sky luminance on
[1, sin el, sin^2 el, exp(-gamma/25deg), cos gamma], gamma = angle to a trial sun, over a 5-deg grid; the elevation of
that fit is weak, its azimuth is not) and a candidate within 30 deg of azimuth of it gains confidence. With no accepted
candidate the gradient azimuth is used at its (clipped) elevation with low confidence; with no significant gradient
either, `sun` is null and the caller must pass the sun itself. All candidates, accepted or not, are written with the
reason.
"""
import argparse, json, os, struct, sys
import numpy as np
from PIL import Image
from scipy.ndimage import map_coordinates, label, gaussian_filter, gaussian_gradient_magnitude, binary_dilation
from scipy.optimize import nnls
from scipy.spatial.transform import Rotation as Rot, Slerp

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("out"); ap.add_argument("--scene", help="scene id or a unique substring of it (engine mode)"); ap.add_argument("--frac", type=float, default=0.5)
ap.add_argument("--t-us", type=int, default=0); ap.add_argument("--port", type=int, default=46436); ap.add_argument("--face", type=int, default=512)
ap.add_argument("--camera", default="camera_front_wide_120fov"); ap.add_argument("--pano-w", type=int, default=1024)
ap.add_argument("--offline", metavar="PANO.jpg", help="no engine: detect on this saved rig-frame panorama (the .json beside it supplies rig_pose and the old crosshair position, _aux.npz the distance/opacity if present)")
ap.add_argument("--min-alt", type=float, default=5.0, help="physical margin: sun altitude over the ground plane (deg, scene up axis from rig_pose) below which a candidate is rejected")
ap.add_argument("--skyline-margin", type=float, default=3.0, help="a candidate must sit this many degrees above the local skyline (lowest sky pixel of its columns)")
ap.add_argument("--surface-max-m", type=float, default=300.0, help="with engine distance/opacity: a candidate that is opaque (>= 0.9) and closer than this is a lit surface, not the sun")
ap.add_argument("--gradient-min", type=float, default=0.15, help="R^2 gain of the sky-gradient fit below which it neither confirms a candidate nor serves as the fallback")
a = ap.parse_args()
if not a.offline and not a.scene: ap.error("--scene is required unless --offline is given")


def M(pose):
    q = pose.quat; T = np.eye(4); T[:3, :3] = Rot.from_quat([q.x, q.y, q.z, q.w]).as_matrix(); T[:3, 3] = [pose.vec.x, pose.vec.y, pose.vec.z]; return T


def P(T):
    q = Rot.from_matrix(T[:3, :3]).as_quat()
    return c.Pose(vec=c.Vec3(x=T[0, 3], y=T[1, 3], z=T[2, 3]), quat=c.Quat(w=q[3], x=q[0], y=q[1], z=q[2]))


def pano_grid(PW, PH):
    """Azimuth, elevation and unit direction (rig FLU) of every panorama pixel."""
    az = (0.5 - (np.arange(PW) + 0.5) / PW) * 2 * np.pi          # +pi (left) ... -pi ; centre column = forward, left of centre = +az (left)
    el = (0.5 - (np.arange(PH) + 0.5) / PH) * np.pi
    AZ, EL = np.meshgrid(az, el)
    return AZ, EL, np.stack([np.cos(EL) * np.cos(AZ), np.cos(EL) * np.sin(AZ), np.sin(EL)], axis=-1)


def render_faces(sid, t_us, Trig, cam_off, parked, N):
    """Six 90-deg faces (rig frame) from the patched engine: rgb float, distance, opacity per face."""
    f = N / 2
    spec = s.CameraSpec(logical_id=cam.logical_id, trajectory_idx=cam.trajectory_idx, resolution_w=N, resolution_h=N, shutter_type=s.GLOBAL,
                        opencv_pinhole_param=s.OpenCVPinholeCameraParam(principal_point_x=N / 2, principal_point_y=N / 2, focal_length_x=f, focal_length_y=f,
                                                                        radial_coeffs=[0] * 6, tangential_coeffs=[0] * 2, thin_prism_coeffs=[0] * 4))
    FACES = {"front": ((1, 0, 0), (0, 0, 1)), "back": ((-1, 0, 0), (0, 0, 1)), "left": ((0, 1, 0), (0, 0, 1)), "right": ((0, -1, 0), (0, 0, 1)),
             "up": ((0, 0, 1), (-1, 0, 0)), "down": ((0, 0, -1), (1, 0, 0))}
    faces = {}
    for name, (fw, up) in FACES.items():
        fw = np.array(fw, float); up = np.array(up, float); right = np.cross(fw, up)
        R = np.stack([right, -up, fw], axis=1)            # optical (x right, y down, z fwd) -> rig FLU
        Tc = np.eye(4); Tc[:3, :3] = Trig[:3, :3] @ R; Tc[:3, 3] = Trig[:3, 3] + Trig[:3, :3] @ cam_off
        req = s.RGBRenderRequest(scene_id=sid, resolution_h=N, resolution_w=N, camera_intrinsics=spec, frame_start_us=t_us, frame_end_us=t_us + 1,
                                 sensor_pose=s.PosePair(start_pose=P(Tc), end_pose=P(Tc)), dynamic_objects=parked, image_format=s.RGB_UINT8_PLANAR, image_quality=-7.0)
        try: b = st.render_rgb(req, timeout=900).image_bytes
        except grpc.RpcError as e: raise SystemExit(f"illum_probe: engine render_rgb failed ({e.code().name} {e.details()})")
        if b[:4] != b"HYBR": raise SystemExit(f"illum_probe: engine on port {a.port} is not the patched serve-grpc (no HYBR payload); start it via hybrid_run.py")
        h, w = struct.unpack("<II", b[4:12]); o = 12
        rgb = np.frombuffer(b[o:o + 3 * h * w], dtype=np.uint8).reshape(3, h, w).transpose(1, 2, 0).astype(np.float32); o += 3 * h * w
        dist = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w); o += 4 * h * w
        opa = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w)
        faces[name] = (R, rgb, dist.copy(), opa.copy())
        print(name, "done", flush=True)
    return faces


def stitch(faces, D, N):
    """Equirectangular colour, distance and opacity in the rig frame from the six faces (nearest face wins by depth of the ray)."""
    PH, PW = D.shape[:2]; f = N / 2
    pano = np.zeros((PH, PW, 3), np.float32); pdist = np.zeros((PH, PW), np.float32); popa = np.zeros((PH, PW), np.float32); best = np.full((PH, PW), -1.0)
    for R, rgb, dist, opa in faces.values():
        d_opt = D @ R                                   # rows: D · columns of R  = R^T d
        z = d_opt[..., 2]; ok = z > 1e-3
        u = np.where(ok, f * d_opt[..., 0] / np.where(ok, z, 1) + N / 2 - 0.5, -1); v = np.where(ok, f * d_opt[..., 1] / np.where(ok, z, 1) + N / 2 - 0.5, -1)
        inside = ok & (u >= 0) & (u <= N - 1) & (v >= 0) & (v <= N - 1) & (z > best)
        for ch in range(3):
            val = map_coordinates(rgb[..., ch], [v, u], order=1, mode="nearest"); pano[..., ch] = np.where(inside, val, pano[..., ch])
        pdist = np.where(inside, map_coordinates(dist, [v, u], order=0, mode="nearest"), pdist)
        popa = np.where(inside, map_coordinates(opa, [v, u], order=1, mode="nearest"), popa)
        best = np.where(inside, z, best)
    return pano, pdist, popa


def sky_mask(pano, lum, band, edge_max=25.0):
    """Sky pixels of the band: blue or neutral-white, smooth, and 8-connected to the band's top rows."""
    r, b = pano[..., 0], pano[..., 2]; mx = pano.max(-1); sat = (mx - pano.min(-1)) / np.maximum(mx, 1); br = b / np.maximum(r, 1)
    skylike = ((br >= 1.08) & (lum >= 40)) | ((lum >= 160) & (sat <= 0.15) & (br >= 0.97))
    cand = skylike & (gaussian_gradient_magnitude(lum, 1.0) < edge_max) & band
    lab, _ = label(cand, structure=np.ones((3, 3)))
    r0 = int(np.argmax(band.any(1))); ids = np.unique(lab[r0:r0 + 2][cand[r0:r0 + 2]]); ids = ids[ids > 0]
    return np.isin(lab, ids)


def skyline(sky, EL):
    """Lowest sky elevation per column (deg), NaN where the column has no sky."""
    rows = np.where(sky, np.arange(sky.shape[0])[:, None], -1).max(0)
    return np.where(rows >= 0, np.degrees(EL[np.maximum(rows, 0), np.arange(sky.shape[1])]), np.nan)


def sky_gradient(lum, D, EL, use, sig_deg=25.0):
    """Sun direction from the sky's brightness distribution: for every trial direction on a 5-deg grid, non-negative least squares
    of the sky luminance on [1, sin el, sin^2 el, exp(-gamma/sig), cos gamma]; returns (az_deg, el_deg, R^2 gain over the
    elevation-only model). The aureole term needs the sun's neighbourhood in the band, the cosine term carries the rest."""
    sub = np.zeros_like(use); sub[::2, ::2] = True; use = use & sub
    y = lum[use]; d = D[use]; se = np.sin(EL[use])
    base = np.stack([np.ones_like(y), se, se * se], 1)
    if y.size < 200: return float("nan"), float("nan"), 0.0
    c0 = np.linalg.lstsq(base, y, rcond=None)[0]; rss0 = float(((base @ c0 - y) ** 2).sum())
    best = (0.0, float("nan"), float("nan"))
    for el in range(5, 90, 5):
        for az in range(-180, 180, 5):
            sdir = np.array([np.cos(np.radians(el)) * np.cos(np.radians(az)), np.cos(np.radians(el)) * np.sin(np.radians(az)), np.sin(np.radians(el))])
            cg = np.clip(d @ sdir, -1, 1); gam = np.degrees(np.arccos(cg))
            _, res = nnls(np.concatenate([base, np.exp(-gam / sig_deg)[:, None], cg[:, None]], 1), y)
            gain = 1.0 - res * res / rss0
            if gain > best[0]: best = (gain, float(az), float(el))
    return best[1], best[2], float(best[0])


def detect_sun(pano, AZ, EL, D, up_rig, pdist=None, popa=None):
    """Sun direction (rig frame) with confidence, the rejected candidates and the masks used. See the module docstring."""
    lum = 0.2126 * pano[..., 0] + 0.7152 * pano[..., 1] + 0.0722 * pano[..., 2]
    band = (EL > np.radians(5)) & (EL < np.radians(45)) & (np.abs(AZ) < np.radians(135))
    sky = sky_mask(pano, lum, band); sl = skyline(sky, EL)
    hot_band = (lum >= max(np.percentile(lum[band], 99.7), 200)) & band
    ls = gaussian_filter(lum, 2.0); hot_sky = (ls >= max(np.percentile(ls[sky], 99.0), 180)) & sky if sky.sum() > 100 else np.zeros_like(sky)
    lab, n = label(hot_band | hot_sky)
    g_az, g_el, g_gain = sky_gradient(lum, D, EL, sky & ~binary_dilation(hot_band | hot_sky, iterations=6))
    grad = dict(az_deg=g_az, el_deg=g_el, r2_gain=g_gain, significant=bool(g_gain >= a.gradient_min))
    cands = []
    for i in range(1, n + 1):
        m = lab == i; px = int(m.sum())
        if px < 12: continue
        wgt = lum[m]; vec = (D[m] * wgt[:, None]).sum(0); vec /= np.linalg.norm(vec)
        az_d = float(np.degrees(np.arctan2(vec[1], vec[0]))); el_d = float(np.degrees(np.arcsin(vec[2]))); alt = float(np.degrees(np.arcsin(np.clip(vec @ up_rig, -1, 1))))
        cols = np.unique(np.nonzero(m)[1]); sl_c = sl[cols]; above = float(el_d - np.nanmedian(sl_c)) if np.isfinite(sl_c).any() else float("nan")
        cd = dict(az_deg=az_d, el_deg=el_d, alt_deg=alt, px=px, peak=float(lum[m].max()), in_sky=float(sky[m].mean()), above_skyline_deg=above,
                  dist_m=float(np.median(pdist[m])) if pdist is not None else None, opacity=float(np.median(popa[m])) if popa is not None else None,
                  gradient_sep_deg=float(abs((az_d - g_az + 180) % 360 - 180)) if grad["significant"] else None, dir_rig=[float(x) for x in vec])
        why = []
        if cd["peak"] < 180: why.append("dim (peak < 180)")
        if cd["in_sky"] < 0.5: why.append("not in the sky mask (facade, tree, glass)")
        if not (above >= a.skyline_margin): why.append(f"below the local skyline + {a.skyline_margin:g} deg")
        if alt < a.min_alt: why.append(f"altitude over the ground plane < {a.min_alt:g} deg")
        if cd["dist_m"] is not None and cd["opacity"] >= 0.9 and cd["dist_m"] < a.surface_max_m: why.append(f"opaque surface at {cd['dist_m']:.0f} m")
        cd["accepted"] = not why; cd["reason"] = "; ".join(why) if why else "ok"
        cands.append(cd)
    cands.sort(key=lambda cd: -(cd["peak"] + 0.01 * cd["px"] + (40.0 if cd["gradient_sep_deg"] is not None and cd["gradient_sep_deg"] <= 30 else 0.0)))
    acc = [cd for cd in cands if cd["accepted"]]; sun = None
    if acc:
        # a veiled sun's aureole thresholds into several fragments: merge the accepted blobs within 15 deg of the best one
        cd = acc[0]; d0 = np.array(cd["dir_rig"]); near = [x for x in acc if np.degrees(np.arccos(np.clip(np.array(x["dir_rig"]) @ d0, -1, 1))) <= 15.0]
        vec = sum(x["px"] * x["peak"] * np.array(x["dir_rig"]) for x in near); vec /= np.linalg.norm(vec)
        agree = cd["gradient_sep_deg"] is not None and cd["gradient_sep_deg"] <= 30
        conf = 0.5 + (0.3 if agree else 0.0) + (0.1 if cd["peak"] >= 200 else 0.0) + (0.1 if cd["dist_m"] is not None else 0.0)
        sun = dict(az_deg=float(np.degrees(np.arctan2(vec[1], vec[0]))), el_deg=float(np.degrees(np.arcsin(vec[2]))), alt_deg=float(np.degrees(np.arcsin(np.clip(vec @ up_rig, -1, 1)))),
                   blob_px=sum(x["px"] for x in near), peak=cd["peak"], dir_rig=[float(x) for x in vec], method="blob", merged=len(near), confidence=round(conf, 2), gradient_agrees=bool(agree))
    elif grad["significant"]:
        el_c = float(np.clip(g_el, 15.0, 60.0)); v = np.array([np.cos(np.radians(el_c)) * np.cos(np.radians(g_az)), np.cos(np.radians(el_c)) * np.sin(np.radians(g_az)), np.sin(np.radians(el_c))])
        sun = dict(az_deg=g_az, el_deg=el_c, alt_deg=float(np.degrees(np.arcsin(np.clip(v @ up_rig, -1, 1)))), blob_px=0, peak=float("nan"), dir_rig=[float(x) for x in v],
                   method="sky_gradient", confidence=round(min(0.35, g_gain), 2), gradient_agrees=True)
    return sun, cands, grad, dict(lum=lum, sky=sky, hot=hot_band | hot_sky, skyline=sl)


def inpaint_crosshair(img, meta):
    """Old probes drew a magenta crosshair through the sun on the .jpg: fill its 9-px bands from the neighbours."""
    PH, PW = img.shape[:2]
    cu = int((0.5 - np.radians(meta["sun"]["az_deg"]) / (2 * np.pi)) * PW); cv = int((0.5 - np.radians(meta["sun"]["el_deg"]) / np.pi) * PH)
    r0, r1 = max(cv - 4, 0), min(cv + 5, PH); c0, c1 = max(cu - 4, 0), min(cu + 5, PW)
    img[r0:r1] = 0.5 * (img[max(r0 - 1, 0)] + img[min(r1, PH - 1)])[None]; img[:, c0:c1] = 0.5 * (img[:, max(c0 - 1, 0)] + img[:, min(c1, PW - 1)])[:, None]
    return img


if a.offline:
    stem = os.path.splitext(a.offline)[0]
    meta = json.load(open(stem + ".json")); sid = meta["scene"]; t_us = int(meta["t_us"]); Trig = np.array(meta["rig_pose"])
    pano = np.asarray(Image.open(a.offline).convert("RGB"), np.float32)
    if meta.get("sun") and not meta.get("jpg_clean"): pano = inpaint_crosshair(pano, meta)
    pdist = popa = None
    if os.path.exists(stem + "_aux.npz"):
        aux = np.load(stem + "_aux.npz"); pdist, popa = aux["dist"].astype(np.float32), aux["opacity"].astype(np.float32)
    PH, PW = pano.shape[:2]; AZ, EL, D = pano_grid(PW, PH)
    print(f"offline: {a.offline} {PW}x{PH}, distance/opacity {'loaded' if pdist is not None else 'unavailable'}", flush=True)
else:
    import grpc
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from nre.grpc.protos import sensorsim_pb2 as s, sensorsim_pb2_grpc as g, common_pb2 as c
    st = g.SensorsimServiceStub(grpc.insecure_channel(f"localhost:{a.port}", options=[("grpc.max_receive_message_length", 256 << 20)]))
    try: scenes = list(st.get_available_scenes(c.Empty(), timeout=60).scene_ids)
    except grpc.RpcError as e: raise SystemExit(f"illum_probe: no NuRec engine on port {a.port} ({e.code().name})")
    sid = next((x for x in scenes if a.scene in x), None)
    if sid is None: raise SystemExit(f"illum_probe: engine on port {a.port} does not serve scene '{a.scene}' ({scenes})")
    cam = {k.logical_id: k for k in st.get_available_cameras(s.AvailableCamerasRequest(scene_id=sid), timeout=900).available_cameras}[a.camera]
    traj = st.get_available_trajectories(s.AvailableTrajectoriesRequest(scene_id=sid), timeout=120).available_trajectories[0].trajectory.poses
    tt = np.array([p.timestamp_us for p in traj], dtype=np.int64); Ts = [M(p.pose) for p in traj]
    slerp = Slerp(tt.astype(np.float64), Rot.from_matrix(np.array([T[:3, :3] for T in Ts]))); pos = np.array([T[:3, 3] for T in Ts])
    t_us = a.t_us if a.t_us else int(tt[0] + a.frac * (tt[-1] - tt[0]))
    Trig = np.eye(4); Trig[:3, :3] = slerp(float(t_us)).as_matrix(); Trig[:3, 3] = [np.interp(t_us, tt, pos[:, i]) for i in range(3)]
    cam_off = M(cam.rig_to_camera)[:3, 3]           # probe from the camera's position, rig orientation
    dyn = st.get_dynamic_objects(s.AvailableDynamicObjectsRequest(scene_id=sid), timeout=120).dynamic_objects
    parked = []
    for d in dyn:
        if d.trajectory.poses:
            T = M(d.trajectory.poses[0].pose); T[2, 3] -= 200.0
            parked.append(s.DynamicObject(track_id=d.id, pose_pair=s.PosePair(start_pose=P(T), end_pose=P(T))))
    PW = a.pano_w; PH = PW // 2; AZ, EL, D = pano_grid(PW, PH)
    pano, pdist, popa = stitch(render_faces(sid, t_us, Trig, cam_off, parked, a.face), D, a.face)

up_rig = Trig[:3, :3].T @ np.array([0.0, 0.0, 1.0])            # scene up axis seen from the rig: the ground-plane horizon of the probe
sun, cands, grad, dbg = detect_sun(pano, AZ, EL, D, up_rig, pdist, popa)
lum, hot = dbg["lum"], dbg["hot"]
# sky statistics in the band the real cameras saw (el 10..35 deg), excluding the hot blobs
band = (EL > np.radians(10)) & (EL < np.radians(35)) & ~hot
skyrgb = pano[band]; skylum = lum[band]
# ground brightness: road ahead, el -5..-20 within +-20 deg of forward
gnd = (EL < np.radians(-5)) & (EL > np.radians(-20)) & (np.abs(AZ) < np.radians(20))
res = dict(scene=sid, t_us=int(t_us), sun=sun, sun_candidates=cands, sky_gradient=grad, sky_px=int(dbg["sky"].sum()), jpg_clean=True,
           sky_mean_rgb=[float(x) for x in skyrgb.mean(0)], sky_lum_mean=float(skylum.mean()), sky_lum_std=float(skylum.std()),
           sky_blue_ratio=float(skyrgb[:, 2].mean() / max(skyrgb[:, 0].mean(), 1)), ground_lum_mean=float(lum[gnd].mean()), ground_rgb=[float(x) for x in pano[gnd].mean(0)],
           horizon_lum=float(lum[(np.abs(EL) < np.radians(3))].mean()), rig_pose=Trig.tolist())
json.dump(res, open(a.out + ".json", "w"), indent=1)
Image.fromarray(pano.clip(0, 255).astype(np.uint8)).save(a.out + ".jpg", quality=92)
if pdist is not None: np.savez_compressed(a.out + "_aux.npz", dist=pdist.astype(np.float32), opacity=popa.astype(np.float16))
# diagnostics: sky mask tinted green, skyline dotted, rejected candidates red, accepted blue, the sun as a magenta crosshair
vis = pano.copy(); vis[dbg["sky"]] = 0.6 * vis[dbg["sky"]] + [0, 80, 0]
for cc, sl_ in enumerate(dbg["skyline"]):
    if np.isfinite(sl_) and cc % 2 == 0: vis[int((0.5 - np.radians(sl_) / np.pi) * PH), cc] = [255, 255, 0]
for cd in cands:
    cu = int((0.5 - np.radians(cd["az_deg"]) / (2 * np.pi)) * PW); cv = int((0.5 - np.radians(cd["el_deg"]) / np.pi) * PH)
    vis[max(cv - 6, 0):cv + 7, max(cu - 6, 0):cu + 7] = [0, 128, 255] if cd["accepted"] else [255, 0, 0]
if sun:
    cu = int((0.5 - np.radians(sun["az_deg"]) / (2 * np.pi)) * PW); cv = int((0.5 - np.radians(sun["el_deg"]) / np.pi) * PH)
    vis[max(cv - 2, 0):cv + 3, :] = [255, 0, 255]; vis[:, max(cu - 2, 0):cu + 3] = [255, 0, 255]
Image.fromarray(vis.clip(0, 255).astype(np.uint8)).save(a.out + "_vis.jpg", quality=88)
print(json.dumps({k: v for k, v in res.items() if k != "rig_pose"}, indent=1))
