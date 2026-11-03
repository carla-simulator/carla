"""One still from a pose: still.py OUT.png x,y,z,yaw,pitch [profile] [fov]"""
import sys, os, queue, carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
out, pose = sys.argv[1], [float(v) for v in sys.argv[2].split(",")]
prof = sys.argv[3] if len(sys.argv) > 3 else "Cinematic"; fov = sys.argv[4] if len(sys.argv) > 4 else "90"
c, w = scene.connect(); tm = c.get_trafficmanager(8000)
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
b = w.get_blueprint_library().find("sensor.camera.rgb")
for k, v in dict(image_size_x=1920, image_size_y=1080, fov=fov, post_process_profile=prof).items(): b.set_attribute(k, str(v))
cam = w.spawn_actor(b, carla.Transform(carla.Location(*pose[:3]), carla.Rotation(yaw=pose[3], pitch=pose[4]))); q = queue.Queue(); cam.listen(q.put)
try:
    for _ in range(150):
        fr = w.tick(); im = q.get(timeout=60)
    im.save_to_disk(out); print(out)
finally:
    cam.stop(); cam.destroy(); scene.set_sync(w, tm, False)
