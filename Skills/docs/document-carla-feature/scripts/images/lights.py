"""Vehicle light states, front and rear 3/4 views at night (Cinematic)."""
import os, sys, queue, math
import numpy as np, carla
sys.path.insert(0, os.path.dirname(__file__)); import scene
OUT = sys.argv[1]; MODEL = sys.argv[2] if len(sys.argv) > 2 else "vehicle.lincoln.mkz"
os.makedirs(OUT, exist_ok=True)
c, w = scene.connect(); tm = c.get_trafficmanager(8000); lib = w.get_blueprint_library()
old = w.get_weather(); w.set_weather(carla.WeatherParameters.ClearNight); scene.set_sync(w, tm, True)
sp = w.get_map().get_spawn_points()[int(os.environ.get("SP", 40))]
bp = lib.find(MODEL)
if bp.has_attribute("color"): bp.set_attribute("color", "200,200,205")
v = w.spawn_actor(bp, sp)
L = carla.VehicleLightState
states = {"front": [("NONE", L.NONE), ("Position", L.Position), ("LowBeam", L.Position | L.LowBeam), ("HighBeam", L.Position | L.LowBeam | L.HighBeam)],
          "rear": [("Position", L.Position), ("Brake", L.Position | L.Brake), ("Reverse", L.Position | L.Reverse), ("LeftBlinker", L.Position | L.LeftBlinker)]}
def cam_at(view):
    yaw = sp.rotation.yaw; ang = math.radians(yaw + (35 if view == "front" else 180 + 35))
    loc = sp.location + carla.Location(7.0 * math.cos(ang), 7.0 * math.sin(ang), 1.6)
    look = math.degrees(math.atan2(sp.location.y - loc.y, sp.location.x - loc.x))
    return carla.Transform(loc, carla.Rotation(yaw=look, pitch=-8))
try:
    for _ in range(100): w.tick()
    for view, lst in states.items():
        cb = lib.find("sensor.camera.rgb")
        for k, val in dict(image_size_x=1280, image_size_y=720, fov=60, post_process_profile="Cinematic").items(): cb.set_attribute(k, str(val))
        cam = w.spawn_actor(cb, cam_at(view)); q = queue.Queue(); cam.listen(q.put)
        for name, st in lst:
            v.set_light_state(L(st))
            best, bl = None, -1
            for i in range(70):
                w.tick(); im = q.get(timeout=30)
                if i >= 40:
                    a = np.frombuffer(im.raw_data, np.uint8).reshape(720, 1280, 4)[:, :, :3]
                    lum = a.astype(np.float32).mean()
                    if lum > bl: best, bl = im, lum
            best.save_to_disk(os.path.join(OUT, f"{view}_{name}.png")); print(view, name, round(bl, 1), flush=True)
        cam.stop(); cam.destroy()
finally:
    v.destroy(); scene.set_sync(w, tm, False); w.set_weather(old)
