"""Same frozen scene through every camera profile, and through weather presets.

python3 looks.py OUT --pose x,y,z,yaw[,pitch] --mode profiles|weather [--profile Cinematic]
"""
import argparse, os, queue, sys
import carla
sys.path.insert(0, os.path.dirname(__file__))
import scene
PROFILES = ["AutomotiveHDR", "AutomotiveHDRClean", "Cinematic", "GoPro", "Dashcam", "LegacyCCD", "LogHDR", "Smartphone", "WideHDR8MP"]
WEATHERS = ["ClearNoon", "CloudyNoon", "WetNoon", "MidRainyNoon", "HardRainNoon", "ClearSunset", "SoftRainSunset", "ClearNight", "HardRainNight"]

def shoot(world, t, profiles, frames=100, w=1600, h=900, fov=90):
    lib = world.get_blueprint_library(); cams, qs = [], []
    for prof in profiles:
        bp = lib.find("sensor.camera.rgb")
        for k, v in dict(image_size_x=w, image_size_y=h, fov=fov).items(): bp.set_attribute(k, str(v))
        if prof: bp.set_attribute("post_process_profile", prof)
        c = world.spawn_actor(bp, t); q = queue.Queue(); c.listen(q.put); cams.append(c); qs.append(q)
    try:
        last = [None] * len(cams)
        for _ in range(frames):
            world.tick()
            for i, q in enumerate(qs): last[i] = q.get(timeout=30)
        return last
    finally:
        for c in cams: c.stop(); c.destroy()

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("--pose", required=True)
    ap.add_argument("--mode", default="profiles"); ap.add_argument("--profile", default="Cinematic")
    ap.add_argument("--nveh", type=int, default=int(os.environ.get("NVEH", 25))); ap.add_argument("--nwalk", type=int, default=15)
    ap.add_argument("--only", nargs="+"); ap.add_argument("--weathers", nargs="+")
    a = ap.parse_args(); os.makedirs(a.out, exist_ok=True)
    p = [float(v) for v in a.pose.split(",")]
    t = carla.Transform(carla.Location(*p[:3]), carla.Rotation(yaw=p[3], pitch=p[4] if len(p) > 4 else 0))
    client, world = scene.connect(); tm = client.get_trafficmanager(8000)
    world.get_spectator().set_transform(t)
    base = world.get_weather()
    for k, v in dict(cloudiness=15, precipitation=0, precipitation_deposits=0, wetness=0, fog_density=0,
                     sun_altitude_angle=40, sun_azimuth_angle=230).items(): setattr(base, k, v)
    world.set_weather(base)
    scene.set_sync(world, tm, True)
    hero = []
    if os.environ.get("HERO"):   # World Partition maps (Town15) stream around the hero vehicle only
        hbp = world.get_blueprint_library().find("vehicle.lincoln.mkz"); hbp.set_attribute("role_name", "hero")
        f = t.get_forward_vector()
        h = world.try_spawn_actor(hbp, carla.Transform(t.location - f * 12 + carla.Location(z=-1.5), carla.Rotation(yaw=t.rotation.yaw)))
        if h: h.set_simulate_physics(False); hero.append(h)
        for _ in range(400): world.tick()   # 20 s of streaming
    tm, veh, walk, ctrl = scene.populate(client, world, t.location, a.nveh, a.nwalk)
    veh += hero
    try:
        for _ in range(120): world.tick()
        scene.freeze(veh, ctrl, walk)
        cars, peds = scene.stage_front(world, t); veh += cars; walk += peds
        for _ in range(30): world.tick()
        if a.mode == "profiles":
            items = a.only or PROFILES
            for i in range(0, len(items), 3):
                for prof, im in zip(items[i:i + 3], shoot(world, t, items[i:i + 3])):
                    im.save_to_disk(os.path.join(a.out, prof + ".png")); print(prof, flush=True)
        elif a.mode == "matrix":
            for wname in (a.weathers or WEATHERS):
                wp = getattr(carla.WeatherParameters, wname); world.set_weather(wp)
                L = carla.VehicleLightState
                ls = L(L.Position | L.LowBeam) if wp.sun_altitude_angle < 0 else L.NONE
                for v in veh: v.set_light_state(ls)
                for _ in range(200): world.tick()
                os.makedirs(os.path.join(a.out, wname), exist_ok=True)
                items = a.only or PROFILES
                for i in range(0, len(items), 3):
                    for prof, im in zip(items[i:i + 3], shoot(world, t, items[i:i + 3], frames=80)):
                        im.save_to_disk(os.path.join(a.out, wname, prof + ".png"))
                print(wname, flush=True)
            world.set_weather(base)
        else:
            for wname in (a.only or WEATHERS):
                wp = getattr(carla.WeatherParameters, wname)
                world.set_weather(wp)
                L = carla.VehicleLightState
                ls = L(L.Position | L.LowBeam) if wp.sun_altitude_angle < 0 else L.NONE
                for v in veh: v.set_light_state(ls)
                for _ in range(200): world.tick()   # 10 s: lighting jump, wetness, eye adaptation
                im = shoot(world, t, [a.profile], frames=60)[0]
                im.save_to_disk(os.path.join(a.out, wname + ".png")); print(wname, flush=True)
            world.set_weather(base)
    finally:
        scene.cleanup(client, world, tm, [veh, walk, ctrl])

if __name__ == "__main__":
    main()
