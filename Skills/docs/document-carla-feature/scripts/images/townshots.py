"""Town stills with traffic: townshots.py OUTDIR W H name=x,y,z,yaw,pitch[,fov] ... (Cinematic, sunny afternoon)."""
import sys, os, queue, carla
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import scene
out, W, H = sys.argv[1], sys.argv[2], sys.argv[3]; os.makedirs(out, exist_ok=True)
shots = [(a.split('=')[0], [float(v) for v in a.split('=')[1].split(',')]) for a in sys.argv[4:]]
c, w = scene.connect()
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, c.get_trafficmanager(8000), True)
tm, veh, wal, ctl = scene.populate(c, w, carla.Location(20, 60, 0), n_veh=70, n_walk=60, radius=400, seed=11)
for _ in range(200): w.tick()
lib = w.get_blueprint_library()
try:
    for name, p in shots:
        b = lib.find('sensor.camera.rgb')
        for k, v in dict(image_size_x=W, image_size_y=H, fov=p[5] if len(p) > 5 else 90, post_process_profile='Cinematic').items(): b.set_attribute(k, str(v))
        cam = w.spawn_actor(b, carla.Transform(carla.Location(*p[:3]), carla.Rotation(yaw=p[3], pitch=p[4]))); q = queue.Queue(); cam.listen(q.put)
        for _ in range(80):
            fr = w.tick(); im = q.get(timeout=60)
            while im.frame < fr: im = q.get(timeout=10)
        im.save_to_disk(os.path.join(out, name + '.png')); print(name); cam.stop(); cam.destroy()
finally:
    scene.cleanup(c, w, tm, [veh, wal, ctl])
