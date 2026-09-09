#!/usr/bin/env python3
"""Spawn and destroy a 1920x1080 rgb_fisheye N times on a live server and read the server's GPU
memory (nvidia-smi) between iterations. It must return to within ~200 MB of the post-first-spawn
baseline; before the EndPlay release fix each sensor left ~2-6 GB behind until garbage collection.

usage: fisheye_memory_test.py [--port 3000] [--iters 3] [--frames 10] [--face 2048]
Single client, synchronous mode, ticks before every read; does not load a world.
"""
import argparse, os, queue, subprocess, sys, time
import carla

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, default=3000)
ap.add_argument("--iters", type=int, default=3)
ap.add_argument("--frames", type=int, default=10)
ap.add_argument("--face", type=int, default=0, help="face_size attribute (0 = default, max(w,h))")
ap.add_argument("--settle-ticks", type=int, default=6)
ap.add_argument("--settle-seconds", type=float, default=1.0)
a = ap.parse_args()


def gpu_mb():
    out = subprocess.run(["nvidia-smi", "--query-compute-apps=pid,process_name,used_memory", "--format=csv,noheader,nounits"],
                         capture_output=True, text=True).stdout
    for line in out.strip().splitlines():
        pid, name, mem = [x.strip() for x in line.split(",")]
        if "UnrealEditor" in name or "CarlaUnreal" in name:
            return int(mem)
    return -1


client = carla.Client("localhost", a.port); client.set_timeout(60.0)
world = client.get_world()
settings = world.get_settings(); prev = (settings.synchronous_mode, settings.fixed_delta_seconds)
settings.synchronous_mode = True; settings.fixed_delta_seconds = 0.05; world.apply_settings(settings)
bpl = world.get_blueprint_library()
sp = world.get_map().get_spawn_points()[0]
tf = carla.Transform(sp.location + carla.Location(z=1.8), carla.Rotation(yaw=sp.rotation.yaw))
readings = []
try:
    world.tick(); base = gpu_mb(); print("baseline (no fisheye): %d MiB" % base, flush=True)
    for it in range(a.iters):
        bp = bpl.find("sensor.camera.rgb_fisheye")
        bp.set_attribute("image_size_x", "1920"); bp.set_attribute("image_size_y", "1080"); bp.set_attribute("fov", "150")
        if a.face > 0 and bp.has_attribute("face_size"): bp.set_attribute("face_size", str(a.face))
        q = queue.Queue(); cam = world.spawn_actor(bp, tf); cam.listen(q.put)
        for _ in range(a.frames):
            world.tick(); img = q.get(timeout=30)
        alive = gpu_mb()
        cam.stop(); cam.destroy()
        for _ in range(a.settle_ticks): world.tick()
        time.sleep(a.settle_seconds); world.tick()
        after = gpu_mb(); readings.append((alive, after))
        print("iter %d: with sensor %d MiB, after destroy %d MiB (delta vs baseline %+d)" % (it, alive, after, after - base), flush=True)
    print("RESULT baseline %d, after each destroy %s -> leak per iteration %s MiB" % (
        base, [r[1] for r in readings], [readings[i][1] - (base if i == 0 else readings[i - 1][1]) for i in range(len(readings))]), flush=True)
finally:
    settings.synchronous_mode, settings.fixed_delta_seconds = prev; world.apply_settings(settings)
    sys.stdout.flush(); os._exit(0)
