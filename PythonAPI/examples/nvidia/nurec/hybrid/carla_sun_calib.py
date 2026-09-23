"""Calibrate CARLA's sun_azimuth_angle convention: a car under a top-down camera, four azimuths at 45 deg altitude,
shadow direction measured in the CARLA world frame. Writes OUT.json with the fit  sun_bearing_deg = k * az + c
(sun_bearing = atan2(dir.y, dir.x) of the direction TOWARD the sun in CARLA world coordinates) and a sheet.
usage: carla_sun_calib.py OUT [--port 3000] [--at X,Y,Z]  (default: spectator location)
"""
import argparse, json, queue, socket
import numpy as np
from PIL import Image
import carla, msgpack

ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("--port", type=int, default=3000); ap.add_argument("--at", default=None)
ap.add_argument("--alt", type=float, default=45.0); ap.add_argument("--azs", default="0,90,180,270")
a = ap.parse_args()
client = carla.Client("localhost", a.port); client.set_timeout(120.0); world = client.get_world()
settings = world.get_settings(); settings.synchronous_mode = True; settings.fixed_delta_seconds = 0.05; world.apply_settings(settings)


def console(cmd):
    sk = socket.create_connection(("localhost", a.port), timeout=10)
    sk.sendall(msgpack.packb([0, 1, "console_command", [[False], cmd]], use_bin_type=True)); sk.settimeout(10); sk.recv(1 << 16); sk.close()


console("r.EyeAdaptation.MethodOverride 3"); console("r.EyeAdaptation.LensAttenuation 0.44")
if a.at:
    x, y, z = [float(v) for v in a.at.split(",")]; base = carla.Location(x, y, z)
else:
    base = world.get_spectator().get_transform().location
wp = world.get_map().get_waypoint(base, project_to_road=True, lane_type=carla.LaneType.Driving)
base = wp.transform.location if wp else base
bpl = world.get_blueprint_library(); actors = []
bp = bpl.find("vehicle.nissan.patrol"); bp.set_attribute("color", "200,200,200")
car = world.try_spawn_actor(bp, carla.Transform(carla.Location(base.x, base.y, base.z + 0.5), carla.Rotation(yaw=0.0)))
assert car is not None, "spawn failed"; car.set_simulate_physics(False); actors.append(car)
bp2 = bpl.find("vehicle.ue4.audi.tt"); bp2.set_attribute("color", "200,30,30")
mark = world.try_spawn_actor(bp2, carla.Transform(carla.Location(base.x, base.y + 8.0, base.z + 0.5), carla.Rotation(yaw=0.0)))   # +y marker
if mark is not None: mark.set_simulate_physics(False); actors.append(mark)
H = 30.0
cam_tf = carla.Transform(carla.Location(base.x, base.y, base.z + H), carla.Rotation(pitch=-90.0, yaw=0.0, roll=0.0))


def spawn_cam(kind):
    b = bpl.find(kind)
    for k, val in (("image_size_x", "1024"), ("image_size_y", "1024"), ("fov", "60")):
        b.set_attribute(k, val)
    q = queue.Queue(); cm = world.spawn_actor(b, cam_tf); cm.listen(q.put); actors.append(cm); return cm, q


cams = {k: spawn_cam(f"sensor.camera.{k}") for k in ("rgb", "semantic_segmentation")}


def arr(img): return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((img.height, img.width, 4))


def capture():
    fr = {}
    for k, (cm, q) in cams.items():
        img = q.get(timeout=30)
        while not q.empty(): img = q.get_nowait()
        fr[k] = arr(img)
    return fr


res = {}; tiles = []
try:
    # reference: sun overhead -> shortest shadow
    world.set_weather(carla.WeatherParameters(cloudiness=0, sun_azimuth_angle=0, sun_altitude_angle=89.0))
    for _ in range(4): world.tick(); capture()
    world.tick(); fr = capture(); ref = fr["rgb"][..., :3].astype(np.float32).mean(axis=2); sem = fr["semantic_segmentation"][..., 2]
    car_mask = np.isin(sem, list(range(14, 20)))
    ys, xs = np.where(car_mask); cy, cx = ys.mean(), xs.mean()
    # marker (+y) position in the image tells the axis convention of the top-down view
    from scipy.ndimage import label
    lab, n = label(car_mask); cents = [(np.where(lab == i)[0].mean(), np.where(lab == i)[1].mean(), (lab == i).sum()) for i in range(1, n + 1)]
    cents.sort(key=lambda t: -t[2]); print("blobs (row, col, px):", [(round(r), round(c_), p) for r, c_, p in cents[:3]], flush=True)
    main = cents[0]; cy, cx = main[0], main[1]
    plus_y = (cents[1][0] - cy, cents[1][1] - cx) if len(cents) > 1 else None
    print("car centre (row, col)", round(cy), round(cx), "+y marker offset (drow, dcol)", plus_y, flush=True)
    for az in [float(v) for v in a.azs.split(",")]:
        world.set_weather(carla.WeatherParameters(cloudiness=0, sun_azimuth_angle=az, sun_altitude_angle=a.alt))
        for _ in range(4): world.tick(); capture()
        world.tick(); fr = capture(); img = fr["rgb"][..., :3].astype(np.float32).mean(axis=2)
        ratio = img / np.maximum(ref, 1.0)
        shadow = (ratio < 0.6) & ~car_mask & np.isin(fr["semantic_segmentation"][..., 2], [1, 2, 24, 22, 10, 9])
        sy, sx = np.where(shadow)
        d = np.array([sy.mean() - cy, sx.mean() - cx]) if len(sy) > 50 else np.array([np.nan, np.nan])
        res[str(az)] = dict(shadow_drow=float(d[0]), shadow_dcol=float(d[1]), px=int(len(sy)))
        print(f"az {az:5.0f}: shadow offset drow {d[0]:7.1f} dcol {d[1]:7.1f} ({len(sy)} px)", flush=True)
        vis = fr["rgb"][..., 2::-1].copy(); vis[shadow] = [255, 0, 255]; tiles.append(vis[::2, ::2])
finally:
    console("r.EyeAdaptation.MethodOverride -1"); console("r.EyeAdaptation.LensAttenuation 0.78")
    for cm, _ in cams.values(): cm.stop()
    settings.synchronous_mode = False; world.apply_settings(settings)
    client.apply_batch([carla.command.DestroyActor(x) for x in actors])
# image axes -> world: camera pitch -90 yaw 0: image up = +x world; the +y marker tells which image side is +y
if plus_y is not None:
    ycol = np.sign(plus_y[1]) if abs(plus_y[1]) > abs(plus_y[0]) else 0.0
    yrow = np.sign(plus_y[0]) if ycol == 0 else 0.0
else:
    ycol, yrow = 1.0, 0.0
fit = {}
for az, r in res.items():
    if np.isnan(r["shadow_drow"]): continue
    # shadow direction in world xy; the sun is the opposite way
    wx = -r["shadow_drow"]                       # image up = +x
    wy = r["shadow_dcol"] * ycol + r["shadow_drow"] * yrow if ycol else r["shadow_drow"] * yrow
    sun_bearing = (np.degrees(np.arctan2(-wy, -wx))) % 360
    fit[az] = dict(shadow_world_xy=[float(wx), float(wy)], sun_bearing_deg=float(sun_bearing))
    print(f"az {az}: sun bearing in CARLA world (atan2 y,x) = {sun_bearing:.1f} deg", flush=True)
azs = np.array([float(k) for k in fit]); bear = np.array([fit[k]["sun_bearing_deg"] for k in fit])
best = None
for k in (1, -1):
    c = np.degrees(np.angle(np.mean(np.exp(1j * np.radians(bear - k * azs)))))
    err = np.abs((bear - (k * azs + c) + 180) % 360 - 180).mean()
    if best is None or err < best["err"]: best = dict(k=k, c=float(c), err=float(err))
print("fit: sun_bearing = %d * az + %.1f (mean err %.1f deg)" % (best["k"], best["c"], best["err"]), flush=True)
json.dump(dict(marker_plus_y=plus_y, ycol=float(ycol), yrow=float(yrow), per_az=fit, fit=best), open(a.out + ".json", "w"), indent=1)
if tiles: Image.fromarray(np.concatenate(tiles, axis=1)).save(a.out + ".jpg", quality=85)
