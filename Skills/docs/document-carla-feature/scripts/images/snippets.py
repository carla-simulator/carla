"""Images for PythonAPI/docs/snipets: draw_box, TrafficLight.set_state, Map.get_waypoint."""
import os, sys, queue, math
import numpy as np, carla
from PIL import Image, ImageDraw, ImageFont
sys.path.insert(0, os.path.dirname(__file__)); import scene
OUT = sys.argv[1]; os.makedirs(OUT + "/tl", exist_ok=True)
MONO = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 17)
c, w = scene.connect(); tm = c.get_trafficmanager(8000); lib = w.get_blueprint_library(); m = w.get_map()
wp = w.get_weather()
for k, v in dict(cloudiness=15, precipitation=0, wetness=0, fog_density=0, sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(wp, k, v)
w.set_weather(wp); scene.set_sync(w, tm, True)
def cam(t, wd=1280, ht=720, fov=80, prof="Cinematic"):
    b = lib.find("sensor.camera.rgb")
    for k, v in dict(image_size_x=wd, image_size_y=ht, fov=fov, post_process_profile=prof).items(): b.set_attribute(k, str(v))
    s = w.spawn_actor(b, t); q = queue.Queue(); s.listen(q.put); return s, q
def grab(q, n):
    im = None
    for _ in range(n):
        fr = w.tick(); im = q.get(timeout=30)
        while im.frame < fr: im = q.get(timeout=5)
    return im
actors = []
try:
    # 1. draw_box around traffic lights, as the snippet does (box size from the snippet)
    t = carla.Transform(carla.Location(103.0, -2.0, 4.0), carla.Rotation(yaw=90.39, pitch=-10))
    s, q = cam(t); actors.append(s)
    debug = w.debug
    for a in w.get_actors().filter("traffic.traffic_light"):
        tr = a.get_transform()
        debug.draw_box(carla.BoundingBox(tr.location, carla.Vector3D(0.5, 0.5, 2)), tr.rotation, 0.1, carla.Color(255, 0, 0, 0), 4.0)
    grab(q, 50).save_to_disk(os.path.join(OUT, "carla.DebugHelper.draw_box.png")); s.stop(); s.destroy(); actors.remove(s)
    for _ in range(100): w.tick()   # let the debug boxes expire
    # 2. vehicle stopped at a red light, set_state(Green)
    tls = list(w.get_actors().filter("traffic.traffic_light"))
    tl = min(tls, key=lambda x: x.get_location().distance(carla.Location(102.9, 10, 0)))
    stop = tl.get_stop_waypoints()[0]
    vt = stop.previous(1.5)[0].transform; vt.location.z += 0.3
    vb = lib.find("vehicle.mini.cooper"); veh = w.spawn_actor(vb, vt); actors.append(veh)
    for x in tl.get_group_traffic_lights(): x.freeze(True)
    tl.set_state(carla.TrafficLightState.Red)
    f = vt.get_forward_vector(); r = vt.get_right_vector()
    cl = vt.location - f * 6.0 - r * 1.2 + carla.Location(z=2.6)
    heads = w.get_level_bbs(carla.CityObjectLabel.TrafficLight)
    fwd_heads = [h for h in heads if (h.location - vt.location).dot(f) > 5]
    look = min(fwd_heads, key=lambda h: h.location.distance(tl.get_location())).location
    cl = vt.location - f * 7.0 + carla.Location(z=2.4)
    yaw = math.degrees(math.atan2(look.y - cl.y, look.x - cl.x)); pit = math.degrees(math.atan2(look.z - cl.z, math.hypot(look.x - cl.x, look.y - cl.y)))
    s, q = cam(carla.Transform(cl, carla.Rotation(yaw=yaw, pitch=pit - 1)), 960, 540, 40); actors.append(s)
    grab(q, 40)
    n = 0
    for i in range(70):
        if i == 25:
            if veh.is_at_traffic_light() or True:
                tl.set_state(carla.TrafficLightState.Green)
        if i > 30: veh.apply_control(carla.VehicleControl(throttle=0.45))
        grab(q, 1).save_to_disk(os.path.join(OUT, "tl", "%04d.png" % n)); n += 1
    print("at_traffic_light", veh.is_at_traffic_light())
    s.stop(); s.destroy(); actors.remove(s)
    for x in tl.get_group_traffic_lights(): x.freeze(False)
    # 3. get_waypoint: vehicle on road + the snippet output
    vt2 = m.get_waypoint(carla.Location(102.9, -30, 0)).transform; vt2.location.z += 0.2
    veh.set_simulate_physics(False); veh.set_transform(vt2); w.tick(); w.tick(); f = vt2.get_forward_vector()
    cl = vt2.location - f * 7.0 + carla.Location(z=3.0)
    s, q = cam(carla.Transform(cl, carla.Rotation(yaw=vt2.rotation.yaw, pitch=-14)), 1280, 720, 75); actors.append(s)
    im = grab(q, 50); grab_path = os.path.join(OUT, "wp_base.png"); im.save_to_disk(grab_path)
    wpt = m.get_waypoint(veh.get_location(), project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk))
    lines = ["Current lane type: " + str(wpt.lane_type), "Current Lane change:  " + str(wpt.lane_change),
             "L lane marking type: " + str(wpt.left_lane_marking.type), "L lane marking change: " + str(wpt.left_lane_marking.lane_change),
             "R lane marking type: " + str(wpt.right_lane_marking.type), "R lane marking change: " + str(wpt.right_lane_marking.lane_change)]
    img = Image.open(grab_path).convert("RGB"); d = ImageDraw.Draw(img, "RGBA")
    d.rectangle((20, 20, 560, 40 + 26 * len(lines)), fill=(15, 18, 22, 210))
    for k, l in enumerate(lines): d.text((34, 30 + 26 * k), l, font=MONO, fill=(220, 235, 220))
    img.save(os.path.join(OUT, "carla.Map.get_waypoint.png")); print("\n".join(lines))
finally:
    for a in actors:
        try:
            if a.type_id.startswith("sensor"): a.stop()
        except Exception: pass
    scene.cleanup(c, w, tm, [actors, []])
