"""TM traffic: aerial Cinematic camera + top-down camera with hybrid-physics overlay.

python3 traffic.py OUT --aerial x,y,z,yaw,pitch --top x,y,z --frames 160
"""
import argparse, os, queue, sys, math
import numpy as np, carla
from PIL import Image, ImageDraw, ImageFont
sys.path.insert(0, os.path.dirname(__file__))
import scene
W, H = 960, 540
FONT = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16)

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("--aerial", required=True)
    ap.add_argument("--top", required=True); ap.add_argument("--frames", type=int, default=160)
    ap.add_argument("--radius", type=float, default=50); ap.add_argument("--topfov", type=float, default=60)
    a = ap.parse_args()
    for d in ("aerial", "hybrid"): os.makedirs(os.path.join(a.out, d), exist_ok=True)
    ae = [float(v) for v in a.aerial.split(",")]; tp = [float(v) for v in a.top.split(",")]
    client, world = scene.connect(); lib = world.get_blueprint_library()
    wp = world.get_weather()
    for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0,
                     sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
    world.set_weather(wp)
    tm = client.get_trafficmanager(8000); scene.set_sync(world, tm, True)
    center = carla.Location(tp[0], tp[1], 0)
    # the hero (role_name "hero") is the vehicle TM measures the hybrid radius from
    sp = min(world.get_map().get_spawn_points(), key=lambda t: t.location.distance(center))
    hbp = lib.find("vehicle.lincoln.mkz"); hbp.set_attribute("role_name", "hero"); hbp.set_attribute("color", "255,255,255")
    hero = world.spawn_actor(hbp, sp)
    tm.set_hybrid_physics_mode(True); tm.set_hybrid_physics_radius(a.radius)
    hero.set_autopilot(True, tm.get_port())
    tm, veh, walk, ctrl = scene.populate(client, world, center, 70, 50, radius=170, exclude=6)
    veh.append(hero)
    cams, qs = [], []
    for name, tr, prof, fov in (("aerial", carla.Transform(carla.Location(*ae[:3]), carla.Rotation(yaw=ae[3], pitch=ae[4])), "Cinematic", 90),
                                ("hybrid", carla.Transform(carla.Location(*tp[:3]), carla.Rotation(pitch=-90, yaw=-90)), "Cinematic", a.topfov)):
        bp = lib.find("sensor.camera.rgb")
        for k, v in dict(image_size_x=W, image_size_y=H, fov=fov, post_process_profile=prof).items(): bp.set_attribute(k, str(v))
        c = world.spawn_actor(bp, tr); q = queue.Queue(); c.listen(q.put); cams.append(c); qs.append(q)
    try:
        for i in range(200 + a.frames):
            hl0 = hero.get_location()
            cams[1].set_transform(carla.Transform(carla.Location(hl0.x, hl0.y, tp[2]), carla.Rotation(pitch=-90, yaw=-90)))
            fr = world.tick(); ims = []
            for q in qs:
                im = q.get(timeout=30)
                while im.frame < fr: im = q.get(timeout=5)
                ims.append(im)
            if i < 200: continue
            n = i - 200
            ims[0].save_to_disk(os.path.join(a.out, "aerial", "%04d.png" % n))
            # top-down overlay: radius around hero, vehicles inside (physics on) / outside (physics off)
            arr = np.frombuffer(ims[1].raw_data, np.uint8).reshape(H, W, 4)[:, :, 2::-1]
            img = Image.fromarray(arr.copy()); d = ImageDraw.Draw(img, "RGBA")
            w2c = np.array(carla.Transform(carla.Location(hl0.x, hl0.y, tp[2]), carla.Rotation(pitch=-90, yaw=-90)).get_inverse_matrix())
            f = W / (2 * math.tan(math.radians(a.topfov / 2)))
            def pr(l):
                p = w2c @ np.array([l.x, l.y, l.z, 1.0]); return (W / 2 + f * p[1] / p[0], H / 2 - f * p[2] / p[0])
            hl = hl0
            ring = [pr(carla.Location(hl.x + a.radius * math.cos(t), hl.y + a.radius * math.sin(t), hl.z)) for t in np.linspace(0, 2 * math.pi, 90)]
            d.polygon(ring, fill=(46, 204, 113, 40), outline=(46, 204, 113, 230))
            for v in veh:
                l = v.get_location(); x, y = pr(l)
                if not (0 <= x < W and 0 <= y < H): continue
                inside = l.distance(hl) <= a.radius
                col = (255, 255, 255) if v.id == hero.id else ((46, 204, 113) if inside else (52, 152, 219))
                r = 9 if v.id == hero.id else 6
                d.ellipse((x - r, y - r, x + r, y + r), fill=col + (230,), outline=(0, 0, 0, 200), width=2)
            for k, (lab, col) in enumerate((("hero", (255, 255, 255)), ("physics on (inside radius)", (46, 204, 113)), ("physics off, teleported", (52, 152, 219)))):
                y0 = 10 + k * 24; d.rectangle((8, y0 - 2, 300, y0 + 20), fill=(0, 0, 0, 150))
                d.ellipse((14, y0 + 3, 28, y0 + 17), fill=col); d.text((36, y0), lab, font=FONT, fill="white")
            img.save(os.path.join(a.out, "hybrid", "%04d.png" % n))
            if n % 40 == 0: print("frame", n, flush=True)
    finally:
        tm.set_hybrid_physics_mode(False)
        for c in cams: c.stop()
        scene.cleanup(client, world, tm, [cams, veh, walk, ctrl])

if __name__ == "__main__":
    main()
