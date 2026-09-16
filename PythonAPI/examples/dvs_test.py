#!/usr/bin/env python3
"""Quick DVS sensor smoke test against a running CARLA UE5 server."""
import argparse
import time

import numpy as np
import carla


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=3000)
    ap.add_argument("--frames", type=int, default=40)
    ap.add_argument("--out", default="dvs_test_out")
    args = ap.parse_args()

    client = carla.Client("localhost", args.port)
    client.set_timeout(10.0)
    world = client.get_world()
    print("server version:", client.get_server_version())

    bp_lib = world.get_blueprint_library()
    dvs_bps = [bp.id for bp in bp_lib.filter("*dvs*")]
    print("DVS blueprints:", dvs_bps)
    if not dvs_bps:
        print("FAIL: sensor.camera.dvs not in blueprint library")
        return 1

    dvs_bp = bp_lib.find("sensor.camera.dvs")
    for attr in dvs_bp:
        print("  attr:", attr.id, "=", attr.recommended_values)
    dvs_bp.set_attribute("image_size_x", "640")
    dvs_bp.set_attribute("image_size_y", "480")

    # spawn a vehicle to attach to, with autopilot for scene motion
    veh_bp = bp_lib.filter("vehicle.*")[0]
    spawn = world.get_map().get_spawn_points()[0]
    vehicle = world.try_spawn_actor(veh_bp, spawn)
    if vehicle is None:
        print("FAIL: could not spawn vehicle")
        return 1
    vehicle.set_autopilot(True)

    cam_tf = carla.Transform(carla.Location(x=1.5, z=2.0))
    dvs = world.spawn_actor(dvs_bp, cam_tf, attach_to=vehicle)
    print("spawned:", dvs.type_id, dvs.id)

    frames = []

    def on_data(data):
        arr = np.frombuffer(
            data.raw_data,
            dtype=np.dtype([
                ("x", np.uint16), ("y", np.uint16),
                ("t", np.int64), ("pol", np.bool_)]))
        frames.append((data.frame, data.timestamp, arr.copy(),
                       data.width, data.height))

    dvs.listen(on_data)

    deadline = time.time() + 30.0
    while len(frames) < args.frames and time.time() < deadline:
        time.sleep(0.2)

    dvs.stop()
    dvs.destroy()
    vehicle.destroy()

    if not frames:
        print("FAIL: no DVS callbacks received in 30 s")
        return 1

    total_events = sum(len(f[2]) for f in frames)
    print(f"received {len(frames)} event packets, {total_events} events total")
    for f in frames[:5]:
        print(f"  frame={f[0]} t={f[1]:.3f} events={len(f[2])}")

    # sanity checks on the densest packet
    frame, ts, ev, w, h = max(frames, key=lambda f: len(f[2]))
    print(f"densest packet: frame={frame} events={len(ev)} size={w}x{h}")
    assert ev["x"].max() < w and ev["y"].max() < h, "event coords out of bounds"
    assert (np.diff(ev["t"]) >= 0).all(), "timestamps not sorted"
    pos = int(ev["pol"].sum())
    print(f"polarity split: +{pos} / -{len(ev) - pos}")

    # save visualization of densest packet
    img = np.zeros((h, w, 3), dtype=np.uint8)
    img[ev["y"], ev["x"], ev["pol"].astype(int) * 2] = 255
    try:
        from PIL import Image
        Image.fromarray(img).save(args.out + ".png")
        print("saved", args.out + ".png")
    except ImportError:
        np.save(args.out + ".npy", img)
        print("saved", args.out + ".npy")

    print("PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
