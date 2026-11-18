"""Run the tuto_G_traffic_manager set_path example and record what the vehicles actually drive.
tm_paths.py OUT.json R1S R1(comma) R2S R2(comma) [seconds]"""
import json, random, sys, os
import carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
out, r1s, r1, r2s, r2 = sys.argv[1], int(sys.argv[2]), [int(x) for x in sys.argv[3].split(",")], int(sys.argv[4]), [int(x) for x in sys.argv[5].split(",")]
secs = float(sys.argv[6]) if len(sys.argv) > 6 else 90
c, w = scene.connect(); tm = c.get_trafficmanager(8000); scene.set_sync(w, tm, True)
random.seed(1); lib = w.get_blueprint_library()
bps = [b for b in lib.filter("vehicle.*") if int(b.get_attribute("number_of_wheels")) == 4 and "mine" not in b.id]
sp = w.get_map().get_spawn_points()
routes = {1: (sp[r1s], [sp[i].location for i in r1]), 2: (sp[r2s], [sp[i].location for i in r2])}
cars, tracks, alt, counter = [], {}, False, 20
cam = None
if os.environ.get("CAM"):   # x,y,z,yaw,pitch;outdir;first_step
    import queue
    pose, camdir, first = os.environ["CAM"].split(";"); pv = [float(x) for x in pose.split(",")]; first = int(first)
    os.makedirs(camdir, exist_ok=True)
    cb = lib.find("sensor.camera.rgb")
    for k, v in dict(image_size_x=960, image_size_y=540, fov=80, post_process_profile="Cinematic").items(): cb.set_attribute(k, str(v))
    cam = w.spawn_actor(cb, carla.Transform(carla.Location(*pv[:3]), carla.Rotation(yaw=pv[3], pitch=pv[4])))
    cq = queue.Queue(); cam.listen(cq.put)
try:
    for step in range(int(secs / 0.05)):
        fr = w.tick()
        if cam is not None:
            im = cq.get(timeout=30)
            while im.frame < fr: im = cq.get(timeout=5)
            if step >= first and step % 2 == 0:
                if os.environ.get("OVERLAY"):
                    import numpy as np, math
                    from PIL import Image, ImageDraw
                    a = np.frombuffer(im.raw_data, np.uint8).reshape(540, 960, 4)[:, :, 2::-1]
                    img = Image.fromarray(a.copy()); d = ImageDraw.Draw(img, "RGBA")
                    w2c = np.array(cam.get_transform().get_inverse_matrix()); f = 960 / (2 * math.tan(math.radians(40)))
                    for v in cars:
                        l = v.get_location(); q = w2c @ np.array([l.x, l.y, l.z, 1.0])
                        x, y = 480 + f * q[1] / q[0], 270 - f * q[2] / q[0]
                        col = (217, 83, 79) if tracks[v.id]["route"] == 1 else (41, 128, 185)
                        d.ellipse((x - 9, y - 9, x + 9, y + 9), outline=col + (255,), width=4)
                    for k, (lab, col) in enumerate((("route 1", (217, 83, 79)), ("route 2", (41, 128, 185)))):
                        d.rectangle((8, 8 + k * 26, 120, 30 + k * 26), fill=(0, 0, 0, 150)); d.ellipse((14, 12 + k * 26, 28, 26 + k * 26), outline=col, width=3)
                        d.text((36, 11 + k * 26), lab, fill="white")
                    img.save(os.path.join(camdir, "%05d.png" % step))
                else:
                    im.save_to_disk(os.path.join(camdir, "%05d.png" % step))
        if counter == 20 and len(cars) < 40:
            rid = 1 if alt else 2
            v = w.try_spawn_actor(random.choice(bps), routes[rid][0])
            if v:
                v.set_autopilot(True, tm.get_port())
                tm.random_left_lanechange_percentage(v, 0); tm.random_right_lanechange_percentage(v, 0)
                tm.auto_lane_change(v, False); tm.set_path(v, routes[rid][1])
                cars.append(v); tracks[v.id] = {"route": rid, "xy": []}
                alt = not alt
            counter -= 1
        elif counter > 0: counter -= 1
        else: counter = 20
        if step % 4 == 0:
            for v in cars:
                l = v.get_location(); tracks[v.id]["xy"].append((round(l.x, 1), round(l.y, 1)))
finally:
    json.dump({"tracks": tracks, "r1": [r1s] + r1, "r2": [r2s] + r2}, open(out, "w"))
    if cam is not None: cam.stop(); cars.append(cam)
    scene.cleanup(c, w, tm, [cars, []])
print(len(cars), "vehicles")
