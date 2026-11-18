"""Stills on a World Partition map (Town15 by default, MAP=... to change) with a hero vehicle parked under each
camera, so the map streams the area. Without the hero a -game server renders only the cells around the
spectator start and the shot comes out empty.

    wp_shots.py OUTDIR "name=x,y,z,yaw,pitch;hx,hy,hz" ...   (camera pose ; hero location, Cinematic, 400 ticks)"""
import sys, os, queue, time, carla
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
shots = [(a.split('=')[0], [float(v) for v in a.split('=')[1].split(';')[0].split(',')], [float(v) for v in a.split(';')[1].split(',')]) for a in sys.argv[2:]]
c = carla.Client('localhost', 2000); c.set_timeout(900); w = c.get_world()
MAP = os.environ.get('MAP', 'Town15')
if not w.get_map().name.endswith(MAP): w = c.load_world(MAP); time.sleep(10)
s = w.get_settings(); s.synchronous_mode = True; s.fixed_delta_seconds = 0.05; w.apply_settings(s)
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp)
lib = w.get_blueprint_library(); sps = w.get_map().get_spawn_points()
hbp = lib.find('vehicle.lincoln.mkz'); hbp.set_attribute('role_name', 'hero')
try:
    for name, p, hp in shots:
        loc = carla.Location(*p[:3])
        sp = carla.Transform(carla.Location(*hp[:3]), carla.Rotation(yaw=p[3]))
        h = w.try_spawn_actor(hbp, sp)
        if h: h.set_simulate_physics(False)
        else: print('hero spawn failed')
        b = lib.find('sensor.camera.rgb')
        for k, v in dict(image_size_x=1920, image_size_y=1080, fov=90, post_process_profile='Cinematic').items(): b.set_attribute(k, str(v))
        cam = w.spawn_actor(b, carla.Transform(loc, carla.Rotation(yaw=p[3], pitch=p[4]))); q = queue.Queue(); cam.listen(q.put)
        for _ in range(400):
            fr = w.tick(); im = q.get(timeout=120)
            while im.frame < fr: im = q.get(timeout=30)
        im.save_to_disk(os.path.join(out, name + '.png')); print(name, 'hero at', sp.location)
        cam.stop(); cam.destroy()
        if h: h.destroy()
        w.tick()
finally:
    s.synchronous_mode = False; s.fixed_delta_seconds = None; w.apply_settings(s)
