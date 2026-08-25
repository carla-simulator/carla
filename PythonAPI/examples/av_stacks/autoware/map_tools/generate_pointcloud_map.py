#!/usr/bin/env python3
"""Generate an Autoware pointcloud_map.pcd for the CURRENT town of a running
CARLA server by sweeping the waypoint graph with a ground-truth ray-cast lidar.

Method
------
1. ``world.get_map().generate_waypoints(step)`` covers the whole road network.
2. Capture poses are picked from those waypoints with a minimum spacing
   (``--capture-spacing``) so we don't scan the same spot repeatedly.
3. One noise-free lidar (all dropoffs 0, noise_stddev 0) is spawned once and
   teleported pose to pose; a few settle frames are discarded after each move.
4. Each measurement's points (sensor frame) are transformed to the CARLA
   world frame with the per-measurement ``data.transform`` matrix.
5. The accumulated cloud is voxel-downsampled (pure numpy) to ``--resolution``
   (default 0.2 m, the Autoware pointcloud map spec floor) and the y axis is
   flipped once at the end (CARLA left-handed -> Autoware right-handed map
   frame, consistent with the y-inverted lanelet2 convention).

Ticking: honours the single-ticker rule -- in a synchronous world this tool
NEVER ticks unless ``--tick`` is passed, in which case it becomes the single
ticking client. See carla_common.TickPolicy.
"""

from __future__ import annotations

import argparse
import math
import os
import queue
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from carla_common import TickPolicy, connect, import_carla  # noqa: E402
from pcd_io import voxel_downsample, write_pcd  # noqa: E402

# Downsample the raw accumulation buffer whenever it grows past this many
# points, to bound memory (~16 bytes/point => 5M points ~ 80 MB).
RAW_CHUNK_LIMIT = 5_000_000


def parse_args(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: %(default)s)")
    p.add_argument("--port", type=int, default=2000, help="CARLA RPC port (default: %(default)s)")
    p.add_argument("--out", default="pointcloud_map.pcd", help="output PCD path (default: %(default)s)")
    p.add_argument("--resolution", type=float, default=0.2,
                   help="voxel downsample resolution in meters (default: %(default)s)")
    p.add_argument("--max-points", type=int, default=60_000_000,
                   help="stop accumulating when the downsampled cloud exceeds this (default: %(default)s)")
    p.add_argument("--waypoint-step", type=float, default=2.0,
                   help="generate_waypoints spacing in meters (default: %(default)s)")
    p.add_argument("--capture-spacing", type=float, default=10.0,
                   help="minimum distance between lidar capture poses (default: %(default)s)")
    p.add_argument("--lidar-height", type=float, default=2.4,
                   help="lidar height above the waypoint in meters (default: %(default)s)")
    p.add_argument("--range", type=float, default=80.0, help="lidar range in meters (default: %(default)s)")
    p.add_argument("--channels", type=int, default=64, help="lidar channels (default: %(default)s)")
    p.add_argument("--points-per-second", type=int, default=600_000,
                   help="lidar points_per_second (default: %(default)s)")
    p.add_argument("--upper-fov", type=float, default=15.0)
    p.add_argument("--lower-fov", type=float, default=-35.0)
    p.add_argument("--scans-per-pose", type=int, default=3,
                   help="lidar measurements accumulated per capture pose (default: %(default)s)")
    p.add_argument("--settle-frames", type=int, default=2,
                   help="frames discarded after teleporting the sensor (default: %(default)s)")
    p.add_argument("--sensor-timeout", type=float, default=20.0,
                   help="seconds to wait for a lidar measurement before giving up (default: %(default)s)")
    p.add_argument("--tick", action="store_true",
                   help="in a synchronous world, make this tool the SINGLE ticking client; "
                        "without it a synchronous world must be ticked by another client")
    p.add_argument("--keep-carla-frame", action="store_true",
                   help="debug: skip the final y flip and write CARLA left-handed coordinates")
    return p.parse_args(argv)


def select_capture_poses(waypoints, spacing):
    """Greedy 2D-grid thinning of waypoint transforms to >= spacing apart."""
    seen = set()
    poses = []
    for wp in waypoints:
        loc = wp.transform.location
        key = (int(math.floor(loc.x / spacing)), int(math.floor(loc.y / spacing)))
        if key in seen:
            continue
        seen.add(key)
        poses.append(wp.transform)
    return poses


def make_lidar_blueprint(world, args):
    bp = world.get_blueprint_library().find("sensor.lidar.ray_cast")
    bp.set_attribute("range", str(args.range))
    bp.set_attribute("channels", str(args.channels))
    bp.set_attribute("points_per_second", str(args.points_per_second))
    bp.set_attribute("upper_fov", str(args.upper_fov))
    bp.set_attribute("lower_fov", str(args.lower_fov))
    # Ground truth: no stochastic dropout, no noise.
    bp.set_attribute("dropoff_general_rate", "0.0")
    bp.set_attribute("dropoff_intensity_limit", "0.0")
    bp.set_attribute("dropoff_zero_intensity", "0.0")
    bp.set_attribute("noise_stddev", "0.0")
    bp.set_attribute("sensor_tick", "0.0")
    settings = world.get_settings()
    if settings.synchronous_mode and settings.fixed_delta_seconds:
        # One tick == one full 360 deg rotation.
        bp.set_attribute("rotation_frequency", str(1.0 / settings.fixed_delta_seconds))
    else:
        bp.set_attribute("rotation_frequency", "10.0")
    return bp


def measurement_to_world_points(measurement):
    """Sensor-frame float32 (x,y,z,intensity) -> CARLA-world-frame Nx4."""
    raw = np.frombuffer(measurement.raw_data, dtype=np.float32)
    if raw.size == 0:
        return np.empty((0, 4), dtype=np.float32)
    pts = raw.reshape(-1, 4).copy()
    m = np.array(measurement.transform.get_matrix(), dtype=np.float64)
    world_xyz = pts[:, :3].astype(np.float64) @ m[:3, :3].T + m[:3, 3]
    pts[:, :3] = world_xyz.astype(np.float32)
    return pts


def drain_queue(q):
    while True:
        try:
            q.get_nowait()
        except queue.Empty:
            return


def build_pointcloud(world, args):
    carla = import_carla()
    policy = TickPolicy(world, allow_tick=args.tick)
    carla_map = world.get_map()
    print(f"Map: {carla_map.name}. Generating waypoint coverage (step {args.waypoint_step} m)...", flush=True)
    waypoints = carla_map.generate_waypoints(args.waypoint_step)
    poses = select_capture_poses(waypoints, args.capture_spacing)
    if not poses:
        raise RuntimeError("waypoint graph produced no capture poses -- does this town have roads?")
    print(f"{len(waypoints)} waypoints -> {len(poses)} capture poses "
          f"(spacing {args.capture_spacing} m)", flush=True)

    bp = make_lidar_blueprint(world, args)
    first = poses[0]
    spawn_tf = carla.Transform(
        carla.Location(first.location.x, first.location.y, first.location.z + args.lidar_height),
        carla.Rotation(),
    )
    sensor = world.spawn_actor(bp, spawn_tf)
    q = queue.Queue()
    sensor.listen(q.put)

    raw_chunks = []
    raw_count = 0
    cloud = np.empty((0, 4), dtype=np.float32)
    budget_hit = False
    try:
        for i, pose in enumerate(poses):
            tf = carla.Transform(
                carla.Location(pose.location.x, pose.location.y, pose.location.z + args.lidar_height),
                carla.Rotation(),
            )
            sensor.set_transform(tf)
            drain_queue(q)
            # Discard settle frames so the teleport has taken effect.
            for _ in range(args.settle_frames):
                policy.advance()
                try:
                    q.get(timeout=args.sensor_timeout)
                except queue.Empty:
                    raise RuntimeError(policy.timeout_hint())
            for _ in range(args.scans_per_pose):
                policy.advance()
                try:
                    meas = q.get(timeout=args.sensor_timeout)
                except queue.Empty:
                    raise RuntimeError(policy.timeout_hint())
                pts = measurement_to_world_points(meas)
                if pts.shape[0]:
                    raw_chunks.append(pts)
                    raw_count += pts.shape[0]
            if raw_count >= RAW_CHUNK_LIMIT:
                merged = np.vstack([cloud] + raw_chunks) if raw_chunks else cloud
                cloud = voxel_downsample(merged, args.resolution)
                raw_chunks, raw_count = [], 0
                if cloud.shape[0] > args.max_points:
                    budget_hit = True
            if (i + 1) % 10 == 0 or i == len(poses) - 1 or budget_hit:
                print(f"  pose {i + 1}/{len(poses)}  downsampled={cloud.shape[0]:,}  "
                      f"pending_raw={raw_count:,}", flush=True)
            if budget_hit:
                print(f"WARNING: --max-points ({args.max_points:,}) exceeded; stopping the sweep "
                      f"early with partial coverage.", flush=True)
                break
    finally:
        sensor.stop()
        sensor.destroy()

    merged = np.vstack([cloud] + raw_chunks) if raw_chunks else cloud
    cloud = voxel_downsample(merged, args.resolution)
    if cloud.shape[0] > args.max_points:
        print(f"WARNING: truncating final cloud {cloud.shape[0]:,} -> {args.max_points:,} points",
              flush=True)
        cloud = cloud[: args.max_points]
    if not args.keep_carla_frame:
        # CARLA left-handed -> Autoware right-handed map frame (see carla_common).
        cloud[:, 1] *= -1.0
    return cloud


def main(argv=None):
    args = parse_args(argv)
    _, world = connect(args.host, args.port)
    try:
        cloud = build_pointcloud(world, args)
    except RuntimeError as exc:
        sys.stderr.write(f"ERROR: {exc}\n")
        return 1
    out_dir = os.path.dirname(os.path.abspath(args.out))
    os.makedirs(out_dir, exist_ok=True)
    write_pcd(args.out, cloud)
    print(f"Wrote {cloud.shape[0]:,} points at {args.resolution} m resolution -> {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
