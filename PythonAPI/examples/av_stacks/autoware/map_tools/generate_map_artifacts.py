#!/usr/bin/env python3
"""Driver: produce the full Autoware map directory for a CARLA town.

Output layout (what ``ros2 launch autoware_launch e2e_simulator.launch.xml
map_path:=<out>`` expects):

    <out>/
      pointcloud_map.pcd       ground-truth lidar sweep, voxelized
      lanelet2_map.osm         OpenDRIVE -> lanelet2 (+ traffic-light reg. elements)
      map_projector_info.yaml  projector_type: Local

Requires a RUNNING CARLA server (never launches one). If the server has a
different town loaded, it is switched with client.load_world() unless
--no-load is given. Honours the single-ticker rule: pass --tick to let the
pointcloud sweep tick a synchronous world.
"""

from __future__ import annotations

import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_lanelet2_map  # noqa: E402
import generate_pointcloud_map  # noqa: E402
from carla_common import connect  # noqa: E402
from pcd_io import write_pcd  # noqa: E402
from write_projector_info import write_projector_info  # noqa: E402


def parse_args(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--town", required=True, help="town to map, e.g. Town10HD_Opt")
    p.add_argument("--out", required=True, help="output map directory")
    p.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: %(default)s)")
    p.add_argument("--port", type=int, default=2000, help="CARLA RPC port (default: %(default)s)")
    p.add_argument("--no-load", action="store_true",
                   help="never call client.load_world(); fail instead if the wrong town is loaded")
    p.add_argument("--skip-pointcloud", action="store_true", help="skip pointcloud_map.pcd")
    p.add_argument("--skip-lanelet2", action="store_true", help="skip lanelet2_map.osm")
    p.add_argument("--keep-xodr", action="store_true",
                   help="also keep the fetched OpenDRIVE as <out>/map.xodr")
    # Pointcloud passthroughs (same defaults as generate_pointcloud_map.py).
    p.add_argument("--resolution", type=float, default=0.2)
    p.add_argument("--max-points", type=int, default=60_000_000)
    p.add_argument("--capture-spacing", type=float, default=10.0)
    p.add_argument("--waypoint-step", type=float, default=2.0)
    p.add_argument("--tick", action="store_true",
                   help="in a synchronous world, act as the SINGLE ticking client")
    return p.parse_args(argv)


def ensure_town(client, world, town, allow_load):
    current = world.get_map().name  # e.g. "Carla/Maps/Town10HD_Opt"
    if current.split("/")[-1] == town:
        return world
    if not allow_load:
        sys.stderr.write(
            f"ERROR: server has '{current}' loaded but --town {town} was requested, "
            "and --no-load forbids switching. Load the town yourself or drop --no-load.\n"
        )
        sys.exit(2)
    print(f"Switching server world '{current}' -> '{town}'...", flush=True)
    try:
        world = client.load_world(town)
    except RuntimeError as exc:
        sys.stderr.write(
            f"ERROR: client.load_world({town!r}) failed: {exc}\n"
            "Check `client.get_available_maps()` for valid names.\n"
        )
        sys.exit(2)
    time.sleep(3.0)  # let the new episode settle
    return world


def main(argv=None):
    args = parse_args(argv)
    out_dir = os.path.abspath(args.out)
    os.makedirs(out_dir, exist_ok=True)
    client, world = connect(args.host, args.port, timeout=60.0)
    world = ensure_town(client, world, args.town, allow_load=not args.no_load)

    osm_path = os.path.join(out_dir, "lanelet2_map.osm")
    pcd_path = os.path.join(out_dir, "pointcloud_map.pcd")
    yaml_path = os.path.join(out_dir, "map_projector_info.yaml")

    # 1. lanelet2 first: it is fast and fails early if the converter venv is missing.
    if args.skip_lanelet2:
        print("Skipping lanelet2_map.osm (--skip-lanelet2).", flush=True)
    else:
        ll_args = generate_lanelet2_map.parse_args([
            "--host", args.host, "--port", str(args.port), "--out", osm_path,
        ] + (["--keep-xodr", os.path.join(out_dir, "map.xodr")] if args.keep_xodr else []))
        generate_lanelet2_map.generate(ll_args, world=world)

    # 2. pointcloud sweep (the slow part).
    if args.skip_pointcloud:
        print("Skipping pointcloud_map.pcd (--skip-pointcloud).", flush=True)
    else:
        pc_args = generate_pointcloud_map.parse_args([
            "--host", args.host, "--port", str(args.port), "--out", pcd_path,
            "--resolution", str(args.resolution),
            "--max-points", str(args.max_points),
            "--capture-spacing", str(args.capture_spacing),
            "--waypoint-step", str(args.waypoint_step),
        ] + (["--tick"] if args.tick else []))
        try:
            cloud = generate_pointcloud_map.build_pointcloud(world, pc_args)
        except RuntimeError as exc:
            sys.stderr.write(f"ERROR: {exc}\n")
            return 1
        write_pcd(pcd_path, cloud)
        print(f"Wrote {cloud.shape[0]:,} points -> {pcd_path}", flush=True)

    # 3. projector info.
    write_projector_info(yaml_path, projector_type="Local")

    print("\nAutoware map directory ready:")
    for name in ("pointcloud_map.pcd", "lanelet2_map.osm", "map_projector_info.yaml"):
        path = os.path.join(out_dir, name)
        status = f"{os.path.getsize(path):,} bytes" if os.path.isfile(path) else "SKIPPED"
        print(f"  {name:26s} {status}")
    print(f"\nUse with: ros2 launch autoware_launch e2e_simulator.launch.xml "
          f"map_path:={out_dir} vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit")
    return 0


if __name__ == "__main__":
    sys.exit(main())
