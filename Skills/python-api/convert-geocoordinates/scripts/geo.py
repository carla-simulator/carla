#!/usr/bin/env python3
"""Convert between CARLA locations and geographic coordinates on a running server.

Commands:
    info                                   the map's georeference and projection
    to-geo   --at X,Y,Z | --actor ID | --file CSV     CARLA location -> lat/lon/alt
    to-world --at LAT,LON[,ALT] | --file CSV          lat/lon/alt -> CARLA location

--proj '+proj=tmerc +lat_0=... +ellps=WGS84' converts through that projection instead of the map's.
--snap (to-world) also prints the nearest driving lane, to tell a point on the road from one off the map.
Every conversion is checked by converting back; the residual is printed in metres.
Connection from env.sh (CARLA_HOST, CARLA_PORT, CARLA_TIMEOUT).
"""
from __future__ import annotations

import argparse
import csv
import math
import os
import re
import sys

import carla  # provided by the active interpreter; check_env.sh verifies this

# Semi-major axis (m) and inverse flattening, as Docs/core_geocoordinates.md lists them.
ELLIPSOIDS = {
    "wgs84": (6378137.0, 298.257223563), "grs80": (6378137.0, 298.257222101),
    "intl": (6378388.0, 297.0), "bessel": (6377397.155, 299.1528128),
    "clrk66": (6378206.4, 294.9786982138), "airy": (6377563.396, 299.3249646),
    "wgs72": (6378135.0, 298.26), "wgs66": (6378145.0, 298.25),
    "sphere": (6370997.0, math.inf),
}


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"), int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


def _ellipsoid(p: dict) -> carla.GeoEllipsoid:
    name = (p.get("ellps") or p.get("datum") or "").lower()
    a, f_inv = ELLIPSOIDS.get(name, (6378137.0, math.inf) if "a" in p else ELLIPSOIDS["wgs84"])
    if "a" in p:
        a = float(p["a"])
    if "b" in p:
        b = float(p["b"])
        f_inv = math.inf if a == b else a / (a - b)
    if "f" in p:
        f = float(p["f"])
        f_inv = math.inf if f == 0 else 1.0 / f
    if "rf" in p:
        f_inv = float(p["rf"])
    return carla.GeoEllipsoid(a=a, f_inv=f_inv)


def parse_proj(text: str):
    """Build the carla projection object a PROJ string names, with CARLA's own defaults."""
    p = {}
    for tok in text.replace("<![CDATA[", "").replace("]]>", "").split():
        if tok.startswith("+"):
            k, _, v = tok[1:].partition("=")
            p[k] = v
    kind, ell = p.get("proj", "tmerc"), _ellipsoid(p)
    g = lambda k, d=0.0: float(p.get(k, d))
    if kind == "tmerc":
        return carla.GeoProjectionTM(lat_0=g("lat_0"), lon_0=g("lon_0"), k=g("k", 1.0), x_0=g("x_0"), y_0=g("y_0"), ellps=ell)
    if kind == "utm":
        return carla.GeoProjectionUTM(zone=int(p.get("zone", 31)), north="south" not in p, ellps=ell)
    if kind == "merc":
        return carla.GeoProjectionWebMerc(ellps=ell)
    if kind == "lcc":
        return carla.GeoProjectionLCC2SP(lat_0=g("lat_0"), lat_1=g("lat_1"), lat_2=g("lat_2"), lon_0=g("lon_0"), x_0=g("x_0"), y_0=g("y_0"), ellps=ell)
    raise SystemExit(f"+proj={kind} is not one CARLA implements (tmerc, utm, merc, lcc)")


def _describe(obj) -> str:
    fields = [n for n in dir(obj) if not n.startswith("_") and not callable(getattr(obj, n))]
    return ", ".join(f"{n}={getattr(obj, n)}" for n in fields)


def _proj_args(proj):
    return (proj,) if proj is not None else ()


def _points(args, n_min, n_max):
    if args.at:
        rows = [args.at.split(",")]
    else:
        with open(args.file, newline="") as fh:
            rows = [r for r in csv.reader(fh) if r and not r[0].lstrip().startswith("#")]
    out = []
    for r in rows:
        try:
            vals = [float(v) for v in r]
        except ValueError:
            continue  # a header row
        if not n_min <= len(vals) <= n_max:
            raise SystemExit(f"expected {n_min}-{n_max} numbers per point, got {r}")
        out.append(vals + [0.0] * (n_max - len(vals)))
    if not out:
        raise SystemExit("no points to convert")
    return out


def cmd_info(args):
    m = _client().get_world().get_map()
    ref, proj = m.get_georeference(), m.get_geoprojection()
    print(f"map            {m.name}")
    print(f"georeference   lat={ref.latitude:.8f} lon={ref.longitude:.8f} alt={ref.altitude:.3f}")
    print(f"projection     {type(proj).__name__}")
    print(f"parameters     {_describe(proj)}")
    if hasattr(proj, "ellps"):
        print(f"ellipsoid      {_describe(proj.ellps)}")
    # The projection objects do not expose the map's ellipsoid, so show the header it was parsed from.
    hit = re.search(r"<geoReference>\s*(?:<!\[CDATA\[)?(.*?)(?:\]\]>)?\s*</geoReference>", m.to_opendrive(), re.S)
    print(f"geoReference   {hit.group(1).strip() if hit else '(none: CARLA falls back to tmerc at 0,0 on WGS84)'}")


def cmd_to_geo(args):
    world = _client().get_world()
    m, proj = world.get_map(), parse_proj(args.proj) if args.proj else None
    if args.actor is not None:
        a = world.get_actors().find(args.actor)
        if a is None:
            raise SystemExit(f"no actor id {args.actor}")
        l = a.get_transform().location
        pts = [[l.x, l.y, l.z]]
    else:
        pts = _points(args, 3, 3)
    print("x,y,z,latitude,longitude,altitude,roundtrip_m")
    for x, y, z in pts:
        loc = carla.Location(x, y, z)
        geo = m.transform_to_geolocation(loc, *_proj_args(proj))
        back = m.geolocation_to_transform(geo, *_proj_args(proj))
        print(f"{x:.3f},{y:.3f},{z:.3f},{geo.latitude:.9f},{geo.longitude:.9f},{geo.altitude:.3f},{back.distance(loc):.4f}")


def cmd_to_world(args):
    m = _client().get_world().get_map()
    proj = parse_proj(args.proj) if args.proj else None
    head = "latitude,longitude,altitude,x,y,z,roundtrip_m"
    print(head + (",road_id,lane_id,lane_offset_m" if args.snap else ""))
    for lat, lon, alt in _points(args, 2, 3):
        geo = carla.GeoLocation(latitude=lat, longitude=lon, altitude=alt)
        loc = m.geolocation_to_transform(geo, *_proj_args(proj))
        back = m.transform_to_geolocation(loc, *_proj_args(proj))
        err = m.geolocation_to_transform(back, *_proj_args(proj)).distance(loc)
        line = f"{lat:.9f},{lon:.9f},{alt:.3f},{loc.x:.3f},{loc.y:.3f},{loc.z:.3f},{err:.4f}"
        if args.snap:
            wp = m.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            off = wp.transform.location.distance(carla.Location(loc.x, loc.y, wp.transform.location.z)) if wp else float("nan")
            line += f",{wp.road_id},{wp.lane_id},{off:.2f}" if wp else ",,,"
            if wp is None or off > args.max_offset:
                print(f"[geo] {lat},{lon} is {off:.0f} m from the nearest driving lane: outside the map, or the wrong projection", file=sys.stderr)
        print(line)


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("info", help="the map's georeference and projection").set_defaults(func=cmd_info)
    for name, func, at_help in (("to-geo", cmd_to_geo, "X,Y,Z"), ("to-world", cmd_to_world, "LAT,LON[,ALT]")):
        s = sub.add_parser(name)
        src = s.add_mutually_exclusive_group(required=True)
        src.add_argument("--at", help=at_help)
        src.add_argument("--file", help=f"CSV, one {at_help} per row; a header row is skipped")
        if name == "to-geo":
            src.add_argument("--actor", type=int, help="convert this actor's current location")
        else:
            s.add_argument("--snap", action="store_true", help="also report the nearest driving lane")
            s.add_argument("--max-offset", type=float, default=50.0, help="--snap warns beyond this distance, m")
        s.add_argument("--proj", help="PROJ string to use instead of the map's projection")
        s.set_defaults(func=func)
    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    sys.exit(main())
