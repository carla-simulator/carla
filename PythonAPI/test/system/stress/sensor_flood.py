#!/usr/bin/env python3
"""Many high-resolution cameras and lidars in synchronous mode: does every sensor deliver every tick?"""

import queue

import carla

import _common as C


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--cameras", type=int, default=16)
    ap.add_argument("--lidars", type=int, default=4)
    ap.add_argument("--ticks", type=int, default=300)
    args = ap.parse_args()
    c = C.client(args)
    world = c.get_world()
    lib = world.get_blueprint_library()
    sps = world.get_map().get_spawn_points()
    C.set_sync(world, True)
    sensors, queues = [], []
    try:
        cam = lib.find("sensor.camera.rgb")
        cam.set_attribute("image_size_x", "1920")
        cam.set_attribute("image_size_y", "1080")
        lid = lib.find("sensor.lidar.ray_cast")
        lid.set_attribute("channels", "64")
        lid.set_attribute("points_per_second", "1300000")
        lid.set_attribute("rotation_frequency", "20")
        lid.set_attribute("range", "100")
        for i in range(args.cameras + args.lidars):
            sp = sps[i % len(sps)]
            tf = carla.Transform(sp.location + carla.Location(z=3), sp.rotation)
            s = world.spawn_actor(cam if i < args.cameras else lid, tf)
            q = queue.Queue()
            s.listen(q.put)
            sensors.append(s)
            queues.append(q)
        world.tick()
        missing, late, ticks = 0, 0, []
        for _ in range(args.ticks):
            t = C.now()
            frame = world.tick(30.0)
            for q in queues:
                try:
                    while True:
                        d = q.get(timeout=10.0)
                        if d.frame >= frame:
                            late += d.frame > frame
                            break
                except queue.Empty:
                    missing += 1
            ticks.append(C.now() - t)
    finally:
        for s in sensors:
            s.stop()
        C.destroy_all(c, ("sensor.*",))
        C.set_sync(world, False)
    C.result(sensors=len(sensors), ticks=len(ticks), missing=missing, out_of_order=late,
             tick_mean_s=sum(ticks) / len(ticks), tick_p99_s=C.percentile(ticks, 0.99))


if __name__ == "__main__":
    main()
