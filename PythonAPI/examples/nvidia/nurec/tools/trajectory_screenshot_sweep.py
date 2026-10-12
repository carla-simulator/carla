#!/usr/bin/env python3
"""Drive a camera along the NUREC ego trajectory over the xodr-generated map
and save a screenshot every N seconds of trajectory time.

No nurec rendering involved: we only need the rig trajectory (from the .usdz)
and the OpenDRIVE file. Output: numbered PNGs (chase view; optional top-down)
plus a manifest.json mapping each shot to trajectory time and map coordinates,
so bad-looking areas can be traced back to xodr roads.

Usage:
  python3 trajectory_screenshot_sweep.py <map.xodr> <scenario.usdz> <outdir>
      [--interval 5.0] [--width 0.6] [--topdown] [--host 127.0.0.1] [--port 2000]
"""
import argparse
import json
import math
import os
import queue
import sys
import zipfile

import numpy as np
import carla

NUREC_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, NUREC_DIR)
from projection_functions import get_t_rig_enu_from_ecef  # noqa: E402

_Y_MIRROR = np.diag([1.0, -1.0, 1.0, 1.0])


def load_ego_trajectory(usdz_file):
    """Return (poses Nx4x4 in nurec world frame, timestamps_s N, T_world_base 4x4)."""
    with zipfile.ZipFile(usdz_file, "r") as z:
        names = [n for n in z.namelist() if n.endswith("rig_trajectories.json")]
        with z.open(names[0]) as f:
            rig = json.load(f)
    traj = rig["rig_trajectories"][0]
    poses = np.array(traj["T_rig_worlds"], dtype=np.float64)
    ts = np.array(traj["T_rig_world_timestamps_us"], dtype=np.float64) * 1e-6
    t_world_base = np.array(rig["T_world_base"], dtype=np.float64)
    return poses, ts - ts[0], t_world_base


def to_carla(mat):
    """nurec RH pose -> CARLA location + yaw (deg)."""
    m = _Y_MIRROR @ mat @ _Y_MIRROR
    fwd = m[:3, 0]
    yaw = math.degrees(math.atan2(fwd[1], fwd[0]))
    return carla.Location(x=m[0, 3], y=m[1, 3], z=m[2, 3]), yaw


def make_camera(world, w=1280, h=720, fov=90):
    bp = world.get_blueprint_library().find("sensor.camera.rgb")
    bp.set_attribute("image_size_x", str(w))
    bp.set_attribute("image_size_y", str(h))
    bp.set_attribute("fov", str(fov))
    cam = world.spawn_actor(bp, carla.Transform(carla.Location(z=100)))
    q = queue.Queue()
    cam.listen(q.put)
    return cam, q


def grab(world, q, min_frame):
    """Tick until we get an image captured at/after min_frame."""
    for _ in range(10):
        frame = world.tick()
        try:
            while True:
                img = q.get(timeout=2.0)
                if img.frame >= min_frame:
                    return img
        except queue.Empty:
            continue
    raise RuntimeError("no frame received")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("xodr")
    ap.add_argument("usdz")
    ap.add_argument("outdir")
    ap.add_argument("--interval", type=float, default=5.0)
    ap.add_argument("--width", type=float, default=0.6, help="additional_width")
    ap.add_argument("--topdown", action="store_true", help="also save a top-down shot per sample")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    xodr_text = open(args.xodr).read()

    client = carla.Client(args.host, args.port)
    client.set_timeout(300.0)
    print("generating world from OpenDRIVE (additional_width=%.2f)..." % args.width)
    params = carla.OpendriveGenerationParameters(
        vertex_distance=2.0,
        max_road_length=500.0,
        wall_height=0.0,
        additional_width=args.width,
        smooth_junctions=True,
        enable_mesh_visibility=True,
    )
    world = client.generate_opendrive_world(xodr_text, params)

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # bright noon light so surface defects are visible
    weather = world.get_weather()
    weather.sun_altitude_angle = 70.0
    weather.cloudiness = 10.0
    weather.fog_density = 0.0
    world.set_weather(weather)

    poses, ts, t_world_base = load_ego_trajectory(args.usdz)
    t_scenario_carla = get_t_rig_enu_from_ecef(t_world_base, xodr_text)
    print("trajectory: %d poses, %.1f s" % (len(poses), ts[-1]))

    # sample indices every --interval seconds of trajectory time
    sample_idx = []
    next_t = 0.0
    for i, t in enumerate(ts):
        if t >= next_t:
            sample_idx.append(i)
            next_t = t + args.interval

    cam, q = make_camera(world)
    world.tick()

    manifest = []
    try:
        for n, i in enumerate(sample_idx):
            loc, yaw = to_carla(t_scenario_carla @ poses[i])
            yr = math.radians(yaw)
            f = carla.Location(x=math.cos(yr), y=math.sin(yr))
            # chase view: 9 m behind, 4.5 m up, pitched down
            cam_loc = carla.Location(
                x=loc.x - 9.0 * f.x, y=loc.y - 9.0 * f.y, z=loc.z + 4.5)
            cam.set_transform(carla.Transform(
                cam_loc, carla.Rotation(pitch=-16.0, yaw=yaw)))
            frame = world.tick()
            img = grab(world, q, frame + 1)
            name = "shot_%03d_t%05.1fs.png" % (n, ts[i])
            img.save_to_disk(os.path.join(args.outdir, name))

            entry = {"index": n, "t_s": round(float(ts[i]), 2),
                     "x": round(loc.x, 2), "y": round(loc.y, 2), "z": round(loc.z, 2),
                     "yaw": round(yaw, 1), "file": name}
            wp = world.get_map().get_waypoint(loc, project_to_road=True)
            if wp is not None:
                entry["road_id"] = wp.road_id
                entry["lane_id"] = wp.lane_id
                entry["is_junction"] = wp.is_junction
            if args.topdown:
                cam.set_transform(carla.Transform(
                    carla.Location(x=loc.x, y=loc.y, z=loc.z + 45.0),
                    carla.Rotation(pitch=-90.0, yaw=yaw)))
                frame = world.tick()
                img = grab(world, q, frame + 1)
                tname = "shot_%03d_t%05.1fs_top.png" % (n, ts[i])
                img.save_to_disk(os.path.join(args.outdir, tname))
                entry["top_file"] = tname
            manifest.append(entry)
            print("  [%d/%d] t=%.1fs (%.1f, %.1f) road=%s%s" % (
                n + 1, len(sample_idx), ts[i], loc.x, loc.y,
                entry.get("road_id"), " JUNCTION" if entry.get("is_junction") else ""))
    finally:
        cam.stop()
        cam.destroy()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = 0.0
        world.apply_settings(settings)

    with open(os.path.join(args.outdir, "manifest.json"), "w") as f:
        json.dump(manifest, f, indent=1)
    print("saved %d samples -> %s" % (len(manifest), args.outdir))


if __name__ == "__main__":
    main()
