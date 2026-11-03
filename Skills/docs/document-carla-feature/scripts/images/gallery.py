"""Sensor gallery from one static viewpoint (Town10HD_Opt).

python3 gallery.py OUT --pose x,y,z,yaw[,pitch] [--weather k=v,...]
Writes OUT/<sensor>.png (+ lidar/radar .npy) all of the same frozen scene.
"""
import argparse, os, queue, sys, time, json
import numpy as np, carla
sys.path.insert(0, os.path.dirname(__file__))
import scene

W, H, FOV = 1920, 1080, 90
CAMS = [("rgb", "sensor.camera.rgb", None),
        ("depth_raw", "sensor.camera.depth", carla.ColorConverter.Raw),
        ("depth_log", "sensor.camera.depth", carla.ColorConverter.LogarithmicDepth),
        ("semantic", "sensor.camera.semantic_segmentation", carla.ColorConverter.CityScapesPalette),
        ("instance", "sensor.camera.instance_segmentation", carla.ColorConverter.Raw),
        ("normals", "sensor.camera.normals", carla.ColorConverter.Raw)]

def capture(world, specs, t, frames):
    lib = world.get_blueprint_library(); sensors = []; qs = []
    for name, bpid, conv in specs:
        bp = lib.find(bpid)
        if bpid.startswith("sensor.camera"):
            bp.set_attribute("image_size_x", str(W)); bp.set_attribute("image_size_y", str(H)); bp.set_attribute("fov", str(FOV))
        elif bpid == "sensor.lidar.ray_cast" or bpid == "sensor.lidar.ray_cast_semantic":
            for k, v in dict(channels=64, range=80, points_per_second=2400000, rotation_frequency=20,
                             upper_fov=12, lower_fov=-30).items():
                bp.set_attribute(k, str(v))
            if bpid == "sensor.lidar.ray_cast":
                bp.set_attribute("dropoff_general_rate", "0.0"); bp.set_attribute("noise_stddev", "0.0")
        elif bpid == "sensor.other.radar":
            for k, v in dict(horizontal_fov=60, vertical_fov=12, range=80, points_per_second=12000).items():
                bp.set_attribute(k, str(v))
        s = world.spawn_actor(bp, t); q = queue.Queue(); s.listen(q.put); sensors.append(s); qs.append(q)
    try:
        last = [None] * len(sensors)
        for _ in range(frames):
            world.tick()
            for i, q in enumerate(qs):
                last[i] = q.get(timeout=30)
        return last
    finally:
        for s in sensors:
            s.stop(); s.destroy()

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("out"); ap.add_argument("--pose", required=True)
    ap.add_argument("--weather", default="cloudiness=15,precipitation=0,precipitation_deposits=0,wetness=0,fog_density=0,sun_altitude_angle=40,sun_azimuth_angle=230")
    ap.add_argument("--nveh", type=int, default=45); ap.add_argument("--nwalk", type=int, default=40)
    ap.add_argument("--warm", type=int, default=260)
    a = ap.parse_args(); os.makedirs(a.out, exist_ok=True)
    p = [float(v) for v in a.pose.split(",")]
    t = carla.Transform(carla.Location(p[0], p[1], p[2]), carla.Rotation(yaw=p[3], pitch=p[4] if len(p) > 4 else 0))
    client, world = scene.connect()
    wp = world.get_weather()
    for kv in a.weather.split(","):
        k, v = kv.split("="); setattr(wp, k, float(v))
    world.set_weather(wp)
    world.get_spectator().set_transform(t)
    tm = client.get_trafficmanager(8000)
    scene.set_sync(world, tm, True)
    tm, veh, walk, ctrl = scene.populate(client, world, t.location, a.nveh, a.nwalk)
    print("spawned", len(veh), "vehicles", len(walk), "walkers", flush=True)
    try:
        for _ in range(a.warm):
            world.tick()
        scene.freeze(veh, ctrl, walk)
        cars, peds = scene.stage_front(world, t)
        veh += cars; walk += peds
        print("staged", len(cars), "cars", len(peds), "walkers", flush=True)
        for _ in range(40):
            world.tick()
        # cameras in batches of 3 (each Lumen scene capture costs ~1.4 GB VRAM)
        for i in range(0, len(CAMS), 3):
            group = CAMS[i:i + 3]
            res = capture(world, group, t, 90)
            for (name, bpid, conv), im in zip(group, res):
                if conv is None: im.save_to_disk(os.path.join(a.out, name + ".png"))
                else: im.save_to_disk(os.path.join(a.out, name + ".png"), conv)
                print(name, im.frame, flush=True)
        res = capture(world, [("lidar", "sensor.lidar.ray_cast", None), ("semlidar", "sensor.lidar.ray_cast_semantic", None),
                              ("radar", "sensor.other.radar", None)], t, 6)
        lid, sem, rad = res
        np.save(os.path.join(a.out, "lidar.npy"), np.frombuffer(lid.raw_data, dtype=np.float32).reshape(-1, 4))
        sd = np.frombuffer(sem.raw_data, dtype=np.dtype([("x", "f4"), ("y", "f4"), ("z", "f4"), ("cos", "f4"), ("idx", "u4"), ("tag", "u4")]))
        np.save(os.path.join(a.out, "semlidar.npy"), sd)
        np.save(os.path.join(a.out, "radar.npy"), np.frombuffer(rad.raw_data, dtype=np.float32).reshape(-1, 4))
        json.dump({"pose": p, "fov": FOV, "w": W, "h": H, "weather": str(world.get_weather())}, open(os.path.join(a.out, "meta.json"), "w"), indent=1)
        print("lidar", lid.frame, "pts", len(lid), "radar", len(rad), flush=True)
    finally:
        scene.cleanup(client, world, tm, [veh, walk, ctrl])

if __name__ == "__main__":
    main()
