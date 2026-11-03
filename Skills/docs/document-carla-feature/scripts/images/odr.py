"""generate_opendrive_world from an .xodr and shoot it. odr.py XODR OUTDIR [smooth 0/1] [pose;pose...]"""
import sys, os, queue, time, math, carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
xodr, out = sys.argv[1], sys.argv[2]; os.makedirs(out, exist_ok=True)
smooth = bool(int(sys.argv[3])) if len(sys.argv) > 3 else True
c = carla.Client("localhost", 2000); c.set_timeout(600)
p = carla.OpendriveGenerationParameters(vertex_distance=2.0, max_road_length=50.0, wall_height=1.0,
        additional_width=0.6, smooth_junctions=smooth, enable_mesh_visibility=True)
w = c.generate_opendrive_world(open(xodr).read(), p)
time.sleep(20); w = c.get_world()
m = w.get_map(); wps = m.generate_waypoints(10.0)
xs = [x.transform.location.x for x in wps]; ys = [x.transform.location.y for x in wps]; zs = [x.transform.location.z for x in wps]
print("bounds", min(xs), max(xs), min(ys), max(ys), min(zs), max(zs), "junctions", len(m.get_topology()))
tm = c.get_trafficmanager(8000)
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
b = w.get_blueprint_library().find("sensor.camera.rgb")
for k, v in dict(image_size_x=1920, image_size_y=1080, fov=80, post_process_profile="Cinematic").items(): b.set_attribute(k, str(v))
poses = [[float(v) for v in s.split(",")] for s in sys.argv[4].split(";")] if len(sys.argv) > 4 else \
    [[(min(xs) + max(xs)) / 2 - 0.45 * (max(xs) - min(xs)), (min(ys) + max(ys)) / 2, max(zs) + 0.35 * (max(xs) - min(xs)), 0, -35]]
try:
    for i, pz in enumerate(poses):
        cam = w.spawn_actor(b, carla.Transform(carla.Location(*pz[:3]), carla.Rotation(yaw=pz[3], pitch=pz[4]))); q = queue.Queue(); cam.listen(q.put)
        for _ in range(150): fr = w.tick(); im = q.get(timeout=60)
        im.save_to_disk(os.path.join(out, "shot%d_smooth%d.png" % (i, smooth))); cam.stop(); cam.destroy()
    jn = sorted({x.get_junction().id: x.get_junction().bounding_box.location for x in wps if x.is_junction}.items())
    import json; json.dump([(k, v.x, v.y, v.z) for k, v in jn], open(os.path.join(out, "junctions.json"), "w"))
finally:
    scene.set_sync(w, tm, False)
