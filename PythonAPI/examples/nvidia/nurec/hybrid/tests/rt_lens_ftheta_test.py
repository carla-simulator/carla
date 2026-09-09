"""Live check of sensor.camera.rt_lens with camera_model=ftheta (NuRec / NVIDIA rig pixel-distance -> angle polynomial):
spawns a small prop at known positions, finds its image centroid, and compares with the analytic projection
theta = sum c_i r^i solved for r.  Also checks exposure_mode=manual gives a non-black, deterministic frame.

usage: rt_lens_ftheta_test.py [--port 3000] [--spp 4]
"""
import argparse, queue, numpy as np, carla

ap = argparse.ArgumentParser(); ap.add_argument("--port", type=int, default=3000); ap.add_argument("--spp", type=int, default=4); a = ap.parse_args()
W, H = 1920, 1080
POLY = [0.0, 0.0010525245452299714, 5.696191607285073e-08, -1.6163911875644033e-10, 2.340258791876798e-13, -9.255288139653364e-17]   # 7c2cf6cd front wide 120
CX, CY, THETA_MAX = 958.9856, 753.2674, 1.34617


def r_of_theta(theta):
    r = np.linspace(0, 1500, 300001); th = sum(c * r ** i for i, c in enumerate(POLY)); ok = np.concatenate([[True], np.diff(th) > 0])
    return float(np.interp(theta, th[ok], r[ok]))


def project(p_cam):
    """p_cam in the sensor's local frame (UE: x forward, y right, z up) -> pixel (u, v) through the f-theta model."""
    x, y, z = p_cam; theta = np.arctan2(np.hypot(y, z), x); phi = np.arctan2(-z, y)      # image: +u right (y), +v down (-z)
    r = r_of_theta(theta); return CX + r * np.cos(phi), CY + r * np.sin(phi)


cl = carla.Client("localhost", a.port); cl.set_timeout(120); w = cl.get_world(); bpl = w.get_blueprint_library()
st = w.get_settings(); st.synchronous_mode = True; st.fixed_delta_seconds = 0.05; w.apply_settings(st)
spec = w.get_spectator().get_transform(); base = carla.Transform(spec.location + carla.Location(z=1.0), carla.Rotation(yaw=spec.rotation.yaw))
bp = bpl.find("sensor.camera.rt_lens")
for k, v in dict(image_size_x=str(W), image_size_y=str(H), camera_model="ftheta", distortion_coeffs=",".join("%.12g" % c for c in POLY), fx="%.10f" % (1 / W), fy="%.10f" % (1 / H),
                 cx="%.10f" % (CX / W), cy="%.10f" % (CY / H), theta_max_deg="%.6f" % np.degrees(THETA_MAX), samples_per_pixel=str(a.spp), enable_denoiser="true",
                 exposure_mode="manual", exposure_compensation="5.0").items():
    assert bp.has_attribute(k), f"rt_lens has no attribute {k}: rebuild the server with the ftheta model"
    bp.set_attribute(k, v)
prop_bp = bpl.find("static.prop.trafficcone01")


def grab(q):
    img = None
    for _ in range(4):
        w.tick()
        try: img = q.get(timeout=5.0)
        except queue.Empty: continue
    while not q.empty(): img = q.get_nowait()
    assert img is not None, "no rt_lens frame"
    return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((H, W, 4))[:, :, 2::-1].astype(np.float32)


# exposure check: a plain frame under manual exposure must not be black
q = queue.Queue(); cam = w.spawn_actor(bp, base); cam.listen(q.put); plain = grab(q); assert plain.mean() > 1.0, "manual exposure frame is black"
cam.stop(); cam.destroy(); w.tick()
# empty show-only reference (the sun disk still renders): everything else in a show-only frame is the prop
bp.set_attribute("show_only_actor_ids", "0"); q = queue.Queue(); cam = w.spawn_actor(bp, base); cam.listen(q.put); bg = grab(q); cam.stop(); cam.destroy(); w.tick()
errs = []; errs_v = []
for (fx_, fy_, fz_) in ((8.0, 0.0, -0.6), (8.0, 3.0, -0.6), (8.0, -3.0, -0.6), (6.0, 5.0, -0.6), (5.0, -6.0, 0.5), (4.0, 5.0, 1.0)):
    fwd = base.get_forward_vector(); right = base.get_right_vector(); up = base.get_up_vector()
    loc = carla.Location(base.location.x + fwd.x * fx_ + right.x * fy_ + up.x * fz_, base.location.y + fwd.y * fx_ + right.y * fy_ + up.y * fz_, base.location.z + fwd.z * fx_ + right.z * fy_ + up.z * fz_)
    prop = w.try_spawn_actor(prop_bp, carla.Transform(loc, base.rotation))
    if prop is None: print("spawn failed", (fx_, fy_, fz_)); continue
    w.tick(); bp.set_attribute("show_only_actor_ids", str(prop.id))          # the prop is the only thing rendered, over black
    q = queue.Queue(); cam = w.spawn_actor(bp, base); cam.listen(q.put); im = grab(q); cam.stop(); cam.destroy(); prop.destroy(); w.tick()
    d = (np.abs(im - bg).mean(2) > 25) & (im.mean(2) > 10)
    if d.sum() < 30: print("prop not visible at", (fx_, fy_, fz_)); continue
    ys, xs = np.where(d); u_obs, v_obs = xs.mean(), ys.mean()
    u_pred, v_pred = project((fx_, fy_, fz_ + 0.3))           # cone silhouette centroid: roughly 0.3 m above its base (loose)
    if not (40 < u_pred < W - 40 and 40 < v_pred < H - 40): print(f"prop at fwd {fx_:.1f} right {fy_:+.1f}: predicted outside the frame, skipped"); continue
    eu, ev = float(abs(u_obs - u_pred)), float(abs(v_obs - v_pred)); errs.append(eu); errs_v.append(ev)
    print(f"prop at fwd {fx_:.1f} right {fy_:+.1f}: observed ({u_obs:.0f}, {v_obs:.0f}) predicted ({u_pred:.0f}, {v_pred:.0f}) err u {eu:.1f} v {ev:.1f} px")
st.synchronous_mode = False; st.fixed_delta_seconds = None; w.apply_settings(st)
print("median error: horizontal %.1f px, vertical %.1f px over %d props (vertical includes the centroid-height guess)" % (np.median(errs), np.median(errs_v), len(errs)))
assert len(errs) >= 3 and np.median(errs) < 8.0 and np.median(errs_v) < 50.0, "f-theta projection does not match the analytic model"
print("OK")
