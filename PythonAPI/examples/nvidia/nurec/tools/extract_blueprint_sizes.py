#!/usr/bin/env python
# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Blueprint Size Extraction Tool

Connects to a CARLA server, measures every vehicle and walker blueprint via
BlueprintLibrary's live spawn-probe, and writes the results to the legacy
blueprint_sizes_*.json files. The NuRec integration itself probes the live
server automatically; these JSONs only serve as an offline fallback.
"""
import argparse
import json
import os
import sys

import carla

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from blueprint_library import BlueprintLibrary


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--output-dir',
        default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        help='Directory to write blueprint_sizes_*.json (default: the nurec module dir)')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)
    world = client.get_world()
    print(f"Connected to CARLA {client.get_server_version()}")

    vehicles, walkers = BlueprintLibrary._probe_world(world)

    vehicle_path = os.path.join(args.output_dir, "blueprint_sizes_vehicle.json")
    walker_path = os.path.join(args.output_dir, "blueprint_sizes_walker.json")
    with open(vehicle_path, "w") as f:
        json.dump(vehicles, f, indent=1)
    with open(walker_path, "w") as f:
        json.dump(walkers, f, indent=1)

    print(f"Wrote {len(vehicles)} vehicles to {vehicle_path}")
    print(f"Wrote {len(walkers)} walkers to {walker_path}")


if __name__ == "__main__":
    main()
