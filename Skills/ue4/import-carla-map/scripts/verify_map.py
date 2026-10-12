#!/usr/bin/env python3
"""Verify an imported map on a RUNNING CARLA server (uncooked -game -nullrhi is
enough — a map needs no rendering to load and drive).

Boot the server directly on the imported base level, e.g.:
    bash ../run-carla-server/scripts/run_server.sh /Game/<pkg>/Maps/<Name>/<Name> 2000 &
then:
    python3 verify_map.py --map <Name> --port 2000        # geometry + roads
    python3 verify_map.py --map <Name> --port 2000 --nav  # also walker navmesh

Checks: the loaded world IS this map, the OpenDRIVE parsed into a road network
(spawn points exist and to_opendrive() is non-empty), and — with --nav — that a
pedestrian can be placed on the navigation mesh.
"""

from __future__ import annotations

import argparse
import sys
import time

import carla


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--map", required=True, help="the imported map name (e.g. MyTown)")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--nav", action="store_true", help="also smoke-test the pedestrian navmesh")
    args = ap.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    world = client.get_world()
    carla_map = world.get_map()

    ok = True

    # 1) The right map is loaded. map.name is like 'Game/<pkg>/Maps/<Name>/<Name>'.
    #    Compare the last component exactly — a substring test passes 'Town1'
    #    against a loaded 'Town10'.
    if carla_map.name.rsplit("/", 1)[-1] == args.map:
        print(f"PASS  loaded map is '{carla_map.name}'")
    else:
        print(f"FAIL  loaded map is '{carla_map.name}', expected one named '{args.map}' "
              f"— the server booted a different level")
        ok = False

    # 2) The OpenDRIVE parsed into a usable road network.
    spawns = carla_map.get_spawn_points()
    if spawns:
        print(f"PASS  {len(spawns)} road spawn points from OpenDRIVE")
    else:
        print("FAIL  no spawn points — the .xodr did not parse into a road network")
        ok = False

    if len(carla_map.to_opendrive()) > 0:
        print("PASS  OpenDRIVE present on the server")
    else:
        print("FAIL  empty OpenDRIVE on the server")
        ok = False

    # 3) A vehicle can spawn and sit on the road. Every spawn is wrapped in
    #    try/finally: an exception between spawn and destroy would otherwise
    #    leave the actor in a world the next check still runs against.
    bp_lib = world.get_blueprint_library()
    veh_bps = bp_lib.filter("vehicle.*")
    if spawns and not veh_bps:
        print("FAIL  no vehicle blueprints on this server — content is missing")
        ok = False
    elif spawns:
        veh_bp = veh_bps[0]
        vehicle = world.try_spawn_actor(veh_bp, spawns[0])
        if vehicle is not None:
            try:
                print(f"PASS  spawned {veh_bp.id} at a road spawn point")
            finally:
                vehicle.destroy()
        else:
            print("FAIL  could not spawn a vehicle at spawn point 0 (blocked/off-road geometry)")
            ok = False

    # 4) Optional: the pedestrian navmesh loaded.
    if args.nav:
        loc = world.get_random_location_from_navigation()
        walker_bps = bp_lib.filter("walker.pedestrian.*")
        if loc is not None and not walker_bps:
            print("FAIL  no walker blueprints on this server — content is missing")
            ok = False
        elif loc is not None:
            walker_bp = walker_bps[0]
            walker = world.try_spawn_actor(walker_bp, carla.Transform(loc))
            if walker is not None:
                try:
                    print(f"PASS  navmesh loaded — placed {walker_bp.id} on it")
                finally:
                    walker.destroy()
            else:
                print("WARN  navmesh returned a point but the walker did not spawn there")
        else:
            print("FAIL  no navigable location — Nav/<map>.bin missing or empty "
                  "(install_fbx2obj.sh, then build_navmesh.py --package <pkg>)")
            ok = False

    # destroy() is asynchronous; dropping the client immediately can leave the
    # actor registered on the server. One tick of the default 20 Hz fixed step
    # is enough for the destruction to be applied.
    time.sleep(0.2)
    print()
    print("verify: PASS" if ok else "verify: FAIL")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
