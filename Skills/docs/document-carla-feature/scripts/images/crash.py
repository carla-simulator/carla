"""Side impact at the sp8 junction, recorded and filmed from a chase camera (Cinematic)."""
import os, sys, queue, math
import numpy as np, carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
c, w = scene.connect(); tm = c.get_trafficmanager(8000); lib = w.get_blueprint_library(); m = w.get_map()
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
bstart = m.get_waypoint(carla.Location(102.9, -24.0, 0)).transform; bstart.location.z += 0.3
# crossing lane: waypoint 30 m ahead of the striker, then the lane crossing it
ahead = m.get_waypoint(bstart.location).next(34)[0]
cands = [x for x in m.generate_waypoints(2.0) if x.transform.location.distance(ahead.transform.location) < 8 and
         abs(((x.transform.rotation.yaw - bstart.rotation.yaw + 180) % 360) - 180) > 70 and not x.is_junction]
cross = min(cands, key=lambda x: x.transform.location.distance(ahead.transform.location)) if cands else ahead
A = cross.transform; A.location.z += 0.3
victim = w.spawn_actor(lib.find("vehicle.nissan.patrol"), A)
striker_bp = lib.find("vehicle.lincoln.mkz"); striker_bp.set_attribute("color", "220,30,30")
striker = w.spawn_actor(striker_bp, bstart)
col = w.spawn_actor(lib.find("sensor.other.collision"), carla.Transform(), attach_to=striker)
hits = []; col.listen(lambda e: hits.append((e.frame, e.other_actor.type_id)))
cb = lib.find("sensor.camera.rgb")
for k, v in dict(image_size_x=960, image_size_y=540, fov=75, post_process_profile="Cinematic").items(): cb.set_attribute(k, str(v))
cam = w.spawn_actor(cb, carla.Transform()); q = queue.Queue(); cam.listen(q.put)
c.start_recorder(os.path.join(OUT, "crash.log"), True)
try:
    for i in range(30): w.tick(); q.get(timeout=30)
    n = 0
    for i in range(110):
        if i < 50: striker.apply_control(carla.VehicleControl(throttle=0.85))
        else: striker.apply_control(carla.VehicleControl(throttle=0.0, brake=0.3))
        s = striker.get_transform(); f = s.get_forward_vector(); r = s.get_right_vector()
        cl = s.location - f * 7.5 + r * 4.5 + carla.Location(z=3.0)
        tgt = s.location + f * 6.0
        yaw = math.degrees(math.atan2(tgt.y - cl.y, tgt.x - cl.x)); pitch = math.degrees(math.atan2(tgt.z - cl.z, math.hypot(tgt.x - cl.x, tgt.y - cl.y)))
        cam.set_transform(carla.Transform(cl, carla.Rotation(yaw=yaw, pitch=pitch)))
        fr = w.tick(); im = q.get(timeout=30)
        while im.frame < fr: im = q.get(timeout=5)
        im.save_to_disk(os.path.join(OUT, "frames", "%04d.png" % n)); n += 1
finally:
    c.stop_recorder()
    print("hits", hits[:3], "frames", n)
    print(c.show_recorder_collisions(os.path.join(OUT, "crash.log"), "v", "v"))
    cam.stop(); col.stop(); scene.cleanup(c, w, tm, [[cam, col], [striker, victim], []])
