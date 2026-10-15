"""Walker skeleton projected on an RGB camera that follows it."""
import os, re, sys, queue, math
import numpy as np, carla
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(__file__)); import scene
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
ROOT = os.environ.get("CARLA_ROOT", os.path.join(os.path.dirname(os.path.abspath(__file__)), *[".."] * 5))
src = open(os.path.join(ROOT, "PythonAPI", "examples", "draw_skeleton.py")).read()
PAIRS = re.findall(r'boneIndex\["(\w+)"\]\], points2d\[boneIndex\["(\w+)"\]', src)
PAIRS = [p for p in PAIRS if "hand" not in p[1] or p[1].startswith("crl_hand__")]   # drop finger bones
W, H, FOV = 960, 540, 60
c, w = scene.connect(); tm = c.get_trafficmanager(8000); lib = w.get_blueprint_library()
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
start = carla.Location(*[float(x) for x in os.environ.get("START", "80.0,-1.5,1.0").split(",")])
loc = w.get_random_location_from_navigation()
best = None
for _ in range(400):
    l = w.get_random_location_from_navigation()
    if l and (best is None or l.distance(start) < best.distance(start)): best = l
bp = lib.find(os.environ.get("WALKER", "walker.pedestrian.0015"))
if bp.has_attribute("is_invincible"): bp.set_attribute("is_invincible", "false")
ped = w.spawn_actor(bp, carla.Transform(best + carla.Location(z=1.0)))
w.tick(); ctl = w.spawn_actor(lib.find("controller.ai.walker"), carla.Transform(), attach_to=ped); w.tick()
ctl.start(); ctl.set_max_speed(1.3)
tgt = None
for _ in range(400):
    l = w.get_random_location_from_navigation()
    if l and 40 < l.distance(best) < 70: tgt = l; break
ctl.go_to_location(tgt or w.get_random_location_from_navigation())
cb = lib.find("sensor.camera.rgb")
for k, v in dict(image_size_x=W, image_size_y=H, fov=FOV, post_process_profile="AutomotiveHDR").items(): cb.set_attribute(k, str(v))
cam = w.spawn_actor(cb, carla.Transform()); q = queue.Queue(); cam.listen(q.put)
f = W / (2 * math.tan(math.radians(FOV / 2)))
try:
    n = 0
    for i in range(60 + 140):
        pl = ped.get_location(); fw = ped.get_transform().get_forward_vector()
        side = carla.Location(-fw.y, fw.x, 0)
        cl = pl + side * 2.6 + fw * 4.2 + carla.Location(z=0.5)
        yaw = math.degrees(math.atan2(pl.y - cl.y, pl.x - cl.x))
        ct = carla.Transform(cl, carla.Rotation(yaw=yaw, pitch=-4)); cam.set_transform(ct)
        fr = w.tick(); im = q.get(timeout=30)
        while im.frame < fr: im = q.get(timeout=5)
        if i % 40 == 0: print('speed', round(ped.get_velocity().length(), 2), flush=True)
        if i < 60: continue
        arr = np.frombuffer(im.raw_data, np.uint8).reshape(H, W, 4)[:, :, 2::-1]
        img = Image.fromarray(arr.copy()); d = ImageDraw.Draw(img)
        bones = ped.get_bones().bone_transforms; idx = {b.name: b.world.location for b in bones}
        w2c = np.array(ct.get_inverse_matrix())
        def pr(l):
            p = w2c @ np.array([l.x, l.y, l.z, 1.0]); return (W / 2 + f * p[1] / p[0], H / 2 - f * p[2] / p[0])
        for a, b in PAIRS:
            if a in idx and b in idx: d.line([pr(idx[a]), pr(idx[b])], fill=(255, 50, 50), width=3)
        for nme, l in idx.items():
            if "hand" in nme and not nme.startswith("crl_hand__"): continue
            x, y = pr(l); d.ellipse((x - 3, y - 3, x + 3, y + 3), fill=(255, 220, 0))
        img.save(os.path.join(OUT, "%04d.png" % n)); n += 1
finally:
    cam.stop(); ctl.stop(); scene.cleanup(c, w, tm, [[cam], [ped], [ctl]])
print("frames", n, "pairs", len(PAIRS))
