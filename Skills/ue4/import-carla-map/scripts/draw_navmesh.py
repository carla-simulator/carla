#!/usr/bin/env python3
"""Draw a CARLA pedestrian navmesh in the server viewport, colour-coded by area.

UE4's navmesh view shows nothing for CARLA (see references/maps.md), so this
decodes Nav/<map>.bin and renders its polygons with world.debug.draw_line. The
file is parsed locally, so the server need not have it loaded -- a .bin can be
inspected before it is installed into Content/.

    python3 draw_navmesh.py --package MyTown --spectator --loop

Needs a rendering server (run_server.sh WINDOW=1, or a packaged build); debug
lines do not appear under -nullrhi, and expire unless --loop redraws them.
"""
import argparse
import importlib.util
import os
import sys
import time

import carla

_HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "navmesh_to_obj", os.path.join(_HERE, "navmesh_to_obj.py"))
_nav = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_nav)

# Recast area id -> (name, RGB). Ids are CARLA_AREA_* in
# LibCarla/source/carla/nav/Navigation.h.
AREA_STYLE = {
    0: ("block", (200, 40, 40)),
    1: ("sidewalk", (40, 220, 90)),
    2: ("crosswalk", (250, 220, 40)),
    3: ("road", (60, 140, 255)),
    4: ("grass", (30, 120, 30)),
}


def main():
    ap = argparse.ArgumentParser()
    _nav.add_navmesh_args(ap)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--life-time", type=float, default=60.0,
                    help="seconds each line stays visible (default 60)")
    ap.add_argument("--z-offset", type=float, default=0.15,
                    help="metres to lift lines off the surface, avoiding z-fighting")
    ap.add_argument("--thickness", type=float, default=0.06)
    ap.add_argument("--spectator", action="store_true",
                    help="move the spectator camera to look at the navmesh")
    ap.add_argument("--height", type=float, default=120.0,
                    help="spectator height above the navmesh centre (metres)")
    ap.add_argument("--loop", action="store_true",
                    help="keep redrawing until interrupted")
    ap.add_argument("--areas", help="comma-separated subset, e.g. sidewalk,crosswalk")
    args = ap.parse_args()

    navmesh = _nav.resolve_navmesh(args)
    nav = _nav.parse(navmesh)
    tiles = nav["tiles"]
    if not tiles:
        sys.exit("navmesh has no tiles: %s" % navmesh)

    wanted = None
    if args.areas:
        wanted = {a.strip().lower() for a in args.areas.split(",")}

    # Recast space is (x, up, z) in metres; CARLA is (x, y, z) with z up.
    # get_xodr_crosswalks.py writes 'v x z y' from a carla.Location, so the
    # inverse is carla(x, y, z) = recast(x, z, up).
    segments = []
    cx = cy = 0.0
    npt = 0
    for t in tiles:
        v = t["verts"]

        def loc(i):
            return carla.Location(x=v[i * 3], y=v[i * 3 + 2],
                                  z=v[i * 3 + 1] + args.z_offset)

        for idx, area in t["polys"]:
            name, rgb = AREA_STYLE.get(area, ("area%d" % area, (255, 255, 255)))
            if wanted and name not in wanted:
                continue
            pts = [loc(i) for i in idx]
            for k in range(len(pts)):
                segments.append((pts[k], pts[(k + 1) % len(pts)], rgb))
            for p in pts:
                cx += p.x
                cy += p.y
                npt += 1

    if not segments:
        sys.exit("nothing to draw (check --areas)")
    cx /= npt
    cy /= npt

    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)
    world = client.get_world()
    debug = world.debug
    print("map: %s" % world.get_map().name)
    print("drawing %d edges, centre x=%.1f y=%.1f" % (len(segments), cx, cy))

    if args.spectator:
        spec = world.get_spectator()
        spec.set_transform(carla.Transform(
            carla.Location(x=cx, y=cy, z=args.height),
            carla.Rotation(pitch=-70.0)))
        print("spectator moved to (%.1f, %.1f, %.1f)" % (cx, cy, args.height))

    def draw():
        for a, b, rgb in segments:
            debug.draw_line(a, b, thickness=args.thickness,
                            color=carla.Color(r=rgb[0], g=rgb[1], b=rgb[2]),
                            life_time=args.life_time)

    draw()
    counts = {}
    for t in tiles:
        for _, area in t["polys"]:
            n = AREA_STYLE.get(area, ("area%d" % area, None))[0]
            counts[n] = counts.get(n, 0) + 1
    print("polygons by area: %s" % ", ".join("%s=%d" % kv for kv in sorted(counts.items())))
    print("legend: road=blue sidewalk=green crosswalk=yellow grass=dkgreen block=red")

    if args.loop:
        print("redrawing every %.0fs -- Ctrl-C to stop" % (args.life_time * 0.8))
        try:
            while True:
                time.sleep(args.life_time * 0.8)
                draw()
        except KeyboardInterrupt:
            print("\nstopped")


if __name__ == "__main__":
    main()
