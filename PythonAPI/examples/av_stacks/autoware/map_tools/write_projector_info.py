#!/usr/bin/env python3
"""Emit Autoware's map_projector_info.yaml.

Default is ``projector_type: Local`` -- the correct choice for CARLA towns,
whose lanelet2 nodes carry metric local_x/local_y coordinates rather than a
real geodetic reference. MGRS / LocalCartesianUTM options are exposed for
future digital-twin maps that do have a geodetic anchor.
"""

from __future__ import annotations

import argparse
import os
import sys

VALID_TYPES = ("Local", "MGRS", "LocalCartesianUTM", "TransverseMercator")


def build_yaml(projector_type="Local", mgrs_grid=None, map_origin=None,
               vertical_datum=None, scale_factor=None):
    if projector_type not in VALID_TYPES:
        raise ValueError(f"projector_type must be one of {VALID_TYPES}, got {projector_type!r}")
    lines = [
        "# Autoware map projector definition (consumed by map_projection_loader).",
        f"projector_type: {projector_type}",
    ]
    if projector_type == "MGRS":
        if not mgrs_grid:
            raise ValueError("projector_type MGRS requires --mgrs-grid (e.g. 54SUE)")
        lines.append(f"mgrs_grid: {mgrs_grid}")
        lines.append(f"vertical_datum: {vertical_datum or 'WGS84'}")
    elif projector_type in ("LocalCartesianUTM", "TransverseMercator"):
        if map_origin is None:
            raise ValueError(f"projector_type {projector_type} requires --map-origin LAT LON ALT")
        lat, lon, alt = map_origin
        lines.append(f"vertical_datum: {vertical_datum or 'WGS84'}")
        lines.append("map_origin:")
        lines.append(f"  latitude: {lat}")
        lines.append(f"  longitude: {lon}")
        lines.append(f"  altitude: {alt}")
        if projector_type == "TransverseMercator" and scale_factor is not None:
            lines.append(f"scale_factor: {scale_factor}")
    return "\n".join(lines) + "\n"


def write_projector_info(path, **kwargs):
    text = build_yaml(**kwargs)
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    return path


def parse_args(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--out", default="map_projector_info.yaml",
                   help="output yaml path (default: %(default)s)")
    p.add_argument("--projector-type", default="Local", choices=VALID_TYPES,
                   help="Autoware projector type (default: %(default)s)")
    p.add_argument("--mgrs-grid", help="MGRS 100 km grid code, e.g. 54SUE (MGRS only)")
    p.add_argument("--map-origin", nargs=3, type=float, metavar=("LAT", "LON", "ALT"),
                   help="geodetic origin (LocalCartesianUTM / TransverseMercator)")
    p.add_argument("--vertical-datum", help="vertical datum (default WGS84 where applicable)")
    p.add_argument("--scale-factor", type=float, help="TransverseMercator scale factor")
    return p.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        write_projector_info(
            args.out,
            projector_type=args.projector_type,
            mgrs_grid=args.mgrs_grid,
            map_origin=tuple(args.map_origin) if args.map_origin else None,
            vertical_datum=args.vertical_datum,
            scale_factor=args.scale_factor,
        )
    except ValueError as exc:
        sys.stderr.write(f"ERROR: {exc}\n")
        return 2
    print(f"Wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
