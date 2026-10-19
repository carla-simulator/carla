"""Aerial still per map: mapshots.py OUTDIR MAP[:x,y,z,yaw,pitch] ...  (Cinematic, sunny afternoon, 1920x1080)."""
import sys, os, queue, time, math, carla
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
c = carla.Client('localhost', 2000); c.set_timeout(600)
for arg in sys.argv[2:]:
    name, _, pose = arg.partition(':')
    w = c.get_world()
    if not w.get_map().name.endswith(name): w = c.load_world(name); time.sleep(5)
    s = w.get_settings(); s.synchronous_mode = True; s.fixed_delta_seconds = 0.05; w.apply_settings(s)
    wp = w.get_weather()
    for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
    w.set_weather(wp)
    if pose: p = [float(v) for v in pose.split(',')]
    else:
        sps = [x.location for x in w.get_map().get_spawn_points()]
        xs = [l.x for l in sps]; ys = [l.y for l in sps]; cx, cy = sum(xs) / len(xs), sum(ys) / len(ys)
        ext = max(max(xs) - min(xs), max(ys) - min(ys)); h = min(max(ext * 0.22, 40), 160); back = h * 1.6
        p = [cx - back * math.cos(math.radians(30)), cy - back * math.sin(math.radians(30)), h, 30, -28]
    b = w.get_blueprint_library().find('sensor.camera.rgb')
    for k, v in dict(image_size_x=1920, image_size_y=1080, fov=90, post_process_profile='Cinematic').items(): b.set_attribute(k, str(v))
    cam = w.spawn_actor(b, carla.Transform(carla.Location(*p[:3]), carla.Rotation(yaw=p[3], pitch=p[4]))); q = queue.Queue(); cam.listen(q.put)
    try:
        for _ in range(120):
            fr = w.tick(); im = q.get(timeout=120)
            while im.frame < fr: im = q.get(timeout=30)
        fn = os.path.join(out, name + ('_' + pose.replace(',', '_') if pose else '') + '.png'); im.save_to_disk(fn); print(fn, p)
    finally:
        cam.stop(); cam.destroy(); s.synchronous_mode = False; s.fixed_delta_seconds = None; w.apply_settings(s)
