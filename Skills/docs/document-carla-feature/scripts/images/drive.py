"""Ego drive sequence with hood cameras: RGB (+2D/3D boxes), DVS, optical flow.

python3 drive.py OUT --start x,y,z,yaw --frames 160 --warm 80
Frames: OUT/{rgb,box2d,box3d,boxtl,dvs,flow}/NNNN.png
"""
import argparse, os, queue, sys
import numpy as np, carla
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(__file__))
import scene
W, H, FOV = 960, 540, 90
EDGES = [(0, 1), (1, 3), (3, 2), (2, 0), (4, 5), (5, 7), (7, 6), (6, 4), (0, 4), (1, 5), (2, 6), (3, 7)]

def K():
    f = W / (2 * np.tan(np.radians(FOV / 2)));  return np.array([[f, 0, W / 2], [0, f, H / 2], [0, 0, 1]])

def proj(Km, w2c, verts):
    out = []
    for v in verts:
        p = w2c @ np.array([v.x, v.y, v.z, 1.0]); c = np.array([p[1], -p[2], p[0]])
        out.append(None if c[2] <= 0.3 else (Km @ c)[:2] / c[2])
    return out

def visible(depth_m, pts, dist):
    ps = np.array([p for p in pts if p is not None])
    if len(ps) < 8: return None
    x0, y0 = np.clip(ps.min(0), 0, [W - 1, H - 1]).astype(int); x1, y1 = np.clip(ps.max(0), 0, [W - 1, H - 1]).astype(int)
    if x1 - x0 < 4 or y1 - y0 < 4: return None
    roi = depth_m[y0:y1, x0:x1]
    frac = np.mean(np.abs(roi - dist) < max(2.5, 0.06 * dist))
    return (x0, y0, x1, y1) if frac > 0.08 else None

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("--start", required=True)
    ap.add_argument("--frames", type=int, default=160); ap.add_argument("--warm", type=int, default=80)
    ap.add_argument("--ego", default="vehicle.lincoln.mkz"); ap.add_argument("--nveh", type=int, default=60)
    ap.add_argument("--nwalk", type=int, default=50)
    a = ap.parse_args()
    for d in ("rgb", "box2d", "box3d", "boxtl", "dvs", "flow"): os.makedirs(os.path.join(a.out, d), exist_ok=True)
    p = [float(v) for v in a.start.split(",")]
    client, world = scene.connect(); lib = world.get_blueprint_library()
    wp = world.get_weather()
    for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0,
                     sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
    world.set_weather(wp)
    tm = client.get_trafficmanager(8000); scene.set_sync(world, tm, True)
    start = carla.Transform(carla.Location(p[0], p[1], p[2]), carla.Rotation(yaw=p[3]))
    ego = world.spawn_actor(lib.find(a.ego), start)
    tm, veh, walk, ctrl = scene.populate(client, world, start.location, a.nveh, a.nwalk, radius=160, exclude=8)
    ego.set_autopilot(True, tm.get_port()); tm.ignore_lights_percentage(ego, 0); tm.vehicle_percentage_speed_difference(ego, 10)
    mount = carla.Transform(carla.Location(x=0.6, z=1.65))
    sens, qs = [], []
    for bid in ("sensor.camera.rgb", "sensor.camera.depth", "sensor.camera.dvs", "sensor.camera.optical_flow"):
        bp = lib.find(bid); bp.set_attribute("image_size_x", str(W)); bp.set_attribute("image_size_y", str(H)); bp.set_attribute("fov", str(FOV))
        s = world.spawn_actor(bp, mount, attach_to=ego); q = queue.Queue(); s.listen(q.put); sens.append(s); qs.append(q)
    Km = K(); tls = world.get_level_bbs(carla.CityObjectLabel.TrafficLight) + world.get_level_bbs(carla.CityObjectLabel.TrafficSigns)
    try:
        for i in range(a.warm + a.frames):
            fr = world.tick(); data = []
            for k, q in enumerate(qs):
                try:
                    d = q.get(timeout=30 if k != 2 else 2)
                    while d.frame < fr: d = q.get(timeout=5)
                except Exception:
                    if k != 2: raise
                    d = None
                data.append(d)
            if i < a.warm: continue
            n = i - a.warm
            rgbd, depd, dvsd, flowd = data
            rgb = np.frombuffer(rgbd.raw_data, np.uint8).reshape(H, W, 4)[:, :, 2::-1]
            dep = np.frombuffer(depd.raw_data, np.uint8).reshape(H, W, 4).astype(np.float64)
            depth_m = (dep[:, :, 2] + dep[:, :, 1] * 256 + dep[:, :, 0] * 65536) / (256 ** 3 - 1) * 1000
            base = Image.fromarray(rgb.copy()); base.save(os.path.join(a.out, "rgb", "%04d.png" % n))
            cam = sens[0]; w2c = np.array(cam.get_transform().get_inverse_matrix()); cl = cam.get_transform().location
            b2, b3, bt = base.copy(), base.copy(), base.copy()
            d2, d3, dt = ImageDraw.Draw(b2), ImageDraw.Draw(b3), ImageDraw.Draw(bt)
            for act in list(world.get_actors().filter("vehicle.*")) + list(world.get_actors().filter("walker.pedestrian.*")):
                if act.id == ego.id: continue
                dist = act.get_transform().location.distance(cl)
                if dist > 60: continue
                verts = act.bounding_box.get_world_vertices(act.get_transform())
                pts = proj(Km, w2c, verts)
                box = visible(depth_m, pts, dist)
                if box is None: continue
                col = (255, 60, 60) if act.type_id.startswith("vehicle") else (255, 200, 0)
                d2.rectangle(box, outline=col, width=2)
                if act.type_id.startswith("vehicle"):
                    for e0, e1 in EDGES:
                        if pts[e0] is not None and pts[e1] is not None:
                            d3.line([tuple(pts[e0]), tuple(pts[e1])], fill=(60, 140, 255), width=2)
            for bb in tls:
                dist = bb.location.distance(cl)
                if dist > 45: continue
                f = cam.get_transform().get_forward_vector(); r = bb.location - cl
                if f.x * r.x + f.y * r.y + f.z * r.z < 1: continue
                pts = proj(Km, w2c, bb.get_world_vertices(carla.Transform()))
                if any(q is None for q in pts) or visible(depth_m, pts, dist) is None: continue
                for e0, e1 in EDGES: dt.line([tuple(pts[e0]), tuple(pts[e1])], fill=(255, 140, 0), width=2)
            b2.save(os.path.join(a.out, "box2d", "%04d.png" % n)); b3.save(os.path.join(a.out, "box3d", "%04d.png" % n))
            bt.save(os.path.join(a.out, "boxtl", "%04d.png" % n))
            ev = np.zeros(0, dtype=np.dtype([("x", np.uint16), ("y", np.uint16), ("t", np.int64), ("pol", np.bool_)])) if dvsd is None else np.frombuffer(dvsd.raw_data, dtype=np.dtype([("x", np.uint16), ("y", np.uint16), ("t", np.int64), ("pol", np.bool_)]))
            img = np.zeros((H, W, 3), np.uint8)
            img[ev["y"], ev["x"], ev["pol"].astype(int) * 2] = 255
            Image.fromarray(img).save(os.path.join(a.out, "dvs", "%04d.png" % n))
            fl = flowd.get_color_coded_flow()
            Image.fromarray(np.frombuffer(fl.raw_data, np.uint8).reshape(H, W, 4)[:, :, 2::-1]).save(os.path.join(a.out, "flow", "%04d.png" % n))
            if n % 20 == 0: print("frame", n, "events", len(ev), flush=True)
    finally:
        for s in sens: s.stop()
        scene.cleanup(client, world, tm, [sens, [ego], veh, walk, ctrl])

if __name__ == "__main__":
    main()
