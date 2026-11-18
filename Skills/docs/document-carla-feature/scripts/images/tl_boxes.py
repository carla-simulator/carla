"""3D boxes of traffic lights + signs on the gallery RGB (static level geometry, same pose)."""
import json, os, sys
import numpy as np, carla
from PIL import Image, ImageDraw
D, OUT = sys.argv[1], sys.argv[2]
meta = json.load(open(os.path.join(D, "meta.json"))); W, H, FOV = meta["w"], meta["h"], meta["fov"]; p = meta["pose"]
c = carla.Client("localhost", 2000); c.set_timeout(30); world = c.get_world()
cam_t = carla.Transform(carla.Location(p[0], p[1], p[2]), carla.Rotation(yaw=p[3]))
w2c = np.array(cam_t.get_inverse_matrix()); f = W / (2 * np.tan(np.radians(FOV / 2)))
K = np.array([[f, 0, W / 2], [0, f, H / 2], [0, 0, 1]])
dep = np.asarray(Image.open(os.path.join(D, "depth_raw.png")).convert("RGB")).astype(np.float64)
depth_m = (dep[..., 0] + dep[..., 1] * 256 + dep[..., 2] * 65536) / (256 ** 3 - 1) * 1000
E = [(0, 1), (1, 3), (3, 2), (2, 0), (4, 5), (5, 7), (7, 6), (6, 4), (0, 4), (1, 5), (2, 6), (3, 7)]
im = Image.open(os.path.join(D, "rgb.png")).convert("RGB"); d = ImageDraw.Draw(im); n = 0
for label, col in ((carla.CityObjectLabel.TrafficLight, (255, 150, 0)), (carla.CityObjectLabel.TrafficSigns, (0, 200, 255))):
    for bb in world.get_level_bbs(label):
        dist = bb.location.distance(cam_t.location)
        if dist > 70: continue
        pts = []
        for v in bb.get_world_vertices(carla.Transform()):
            q = w2c @ np.array([v.x, v.y, v.z, 1.0]); cc = np.array([q[1], -q[2], q[0]])
            pts.append(None if cc[2] <= 0.5 else (K @ cc)[:2] / cc[2])
        if any(x is None for x in pts): continue
        a = np.array(pts); x0, y0 = np.clip(a.min(0), 0, [W - 1, H - 1]).astype(int); x1, y1 = np.clip(a.max(0), 0, [W - 1, H - 1]).astype(int)
        if x1 <= x0 or y1 <= y0: continue
        roi = depth_m[y0:y1 + 1, x0:x1 + 1]
        if np.mean(np.abs(roi - dist) < max(3, 0.1 * dist)) < 0.02: continue
        for e0, e1 in E: d.line([tuple(pts[e0]), tuple(pts[e1])], fill=col, width=3)
        n += 1
im = im.resize((1280, 720), Image.LANCZOS); im.save(OUT, quality=86); print("boxes", n)
