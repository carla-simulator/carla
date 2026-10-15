"""Third-person chase camera on an autopilot vehicle (what tuto_G_pygame renders in its window)."""
import os, sys, queue
import carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
c, w = scene.connect(); tm = c.get_trafficmanager(8000); lib = w.get_blueprint_library()
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
sp = w.get_map().get_spawn_points()[int(os.environ.get("SP", 96))]
ego = w.spawn_actor(lib.find("vehicle.nissan.patrol"), sp)
tm_, veh, walk, ctrl = scene.populate(c, w, sp.location, 50, 40, radius=160, exclude=8)
ego.set_autopilot(True, tm.get_port())
b = lib.find("sensor.camera.rgb")
for k, v in dict(image_size_x=960, image_size_y=540, fov=90, post_process_profile="Cinematic").items(): b.set_attribute(k, str(v))
cam = w.spawn_actor(b, carla.Transform(carla.Location(x=-7.5, z=3.0), carla.Rotation(pitch=-4)), attach_to=ego,
                    attachment_type=carla.AttachmentType.SpringArmGhost)
q = queue.Queue(); cam.listen(q.put)
try:
    n = 0
    for i in range(80 + 180):
        fr = w.tick(); im = q.get(timeout=30)
        while im.frame < fr: im = q.get(timeout=5)
        if i >= 80: im.save_to_disk(os.path.join(OUT, "%04d.png" % n)); n += 1
finally:
    cam.stop(); scene.cleanup(c, w, tm, [[cam, ego], veh, walk, ctrl])
