"""pie_shot.py SHOTS.json OUTDIR: one CARLA RGB capture per shot, for material and scene examples.

SHOTS.json is a list of {"name", "cam": [x, y, z], "at": [x, y, z], "fov": 70, "weather": {field: value}, "wait": 4}
in metres (CARLA coordinates). Each shot applies its weather fields on top of the current weather, points a 1920x1080
Cinematic camera from `cam` at `at`, waits `wait` seconds (auto exposure, wetness) and saves OUTDIR/<name>.png.
Works against an editor PIE session (LevelEditorSubsystem.editor_request_begin_play) or a -game server.
"""
import carla, json, math, sys, time, queue
shots = json.load(open(sys.argv[1])); out = sys.argv[2]
c = carla.Client('127.0.0.1', 2000); c.set_timeout(60); w = c.get_world()
bp = w.get_blueprint_library().find('sensor.camera.rgb')
for k, v in dict(image_size_x=1920, image_size_y=1080, post_process_profile='Cinematic').items():
    if bp.has_attribute(k): bp.set_attribute(k, str(v))
cam = None
try:
    for s in shots:
        wp = w.get_weather()
        for k, v in s.get('weather', {}).items(): setattr(wp, k, v)
        w.set_weather(wp)
        f = carla.Location(*s['cam']); t = carla.Location(*s['at'])
        d = t - f
        rot = carla.Rotation(pitch=math.degrees(math.atan2(d.z, math.hypot(d.x, d.y))), yaw=math.degrees(math.atan2(d.y, d.x)))
        bp.set_attribute('fov', str(s.get('fov', 70)))
        if cam: cam.stop(); cam.destroy()
        cam = w.spawn_actor(bp, carla.Transform(f, rot))
        q = queue.Queue(); cam.listen(q.put)
        time.sleep(s.get('wait', 4))
        while not q.empty(): q.get()
        q.get(timeout=30).save_to_disk('%s/%s.png' % (out, s['name'])); print('shot', s['name'])
finally:
    if cam: cam.stop(); cam.destroy()
