#!/usr/bin/env python3
"""
OSM Pedestrian Crosswalks & Zebra Markings Inspector.

Inspects crosswalk manifests and map bundle crosswalk geometry to verify:
  1. Total crosswalks and zebra stripes adhere to DIN / StVO geometric standards.
  2. Road carriageway alignment and lane projection across multi-lane roads.
  3. Pedestrian refuge islands (crossing:island=yes) and tactile paving (tactile_paving=yes).
  4. Recast NavMesh walkable corridor coverage connecting sidewalks across carriageways.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import sys
from typing import Any

import carla

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))
sys.path.insert(0, str(REPO_ROOT / "PythonAPI" / "util"))

from osm_crosswalks import (
    CrosswalkItem,
    audit_crosswalk_metrics,
    extract_osm_crosswalks,
    generate_crosswalk_meshes,
    load_crosswalks_manifest,
)


def inspect_crosswalks(
    manifest_path: Path | str | None = None,
    osm_path: Path | str | None = None,
    xodr_path: Path | str | None = None,
    max_road_dist: float = 20.0,
) -> dict[str, Any]:
    """Audits crosswalk items and returns compliance metrics."""
    carla_map = None
    if xodr_path:
        xp = Path(xodr_path)
        if xp.is_file():
            carla_map = carla.Map(xp.stem, xp.read_text(encoding="utf-8"))

    items: list[CrosswalkItem] = []
    if manifest_path and Path(manifest_path).is_file():
        items = load_crosswalks_manifest(manifest_path)
    elif osm_path and Path(osm_path).is_file():
        items = extract_osm_crosswalks(osm_path, carla_map=carla_map, max_road_dist=max_road_dist)

    if not items:
        return {
            "crosswalk_count": 0,
            "total_stripes": 0,
            "island_count": 0,
            "tactile_count": 0,
            "road_alignment_pct": 0.0,
            "passed_din_standards": False,
            "crosswalks": [],
        }

    # Run geometric audit
    metrics = audit_crosswalk_metrics(items, carla_map=carla_map, max_road_dist=max_road_dist)

    island_count = sum(1 for c in items if c.has_island)
    tactile_count = sum(1 for c in items if c.has_tactile)
    types_breakdown: dict[str, int] = {}
    for c in items:
        types_breakdown[c.crossing_type] = types_breakdown.get(c.crossing_type, 0) + 1

    # Verify mesh generation
    mesh_layers = generate_crosswalk_meshes(items)
    z_mesh = mesh_layers["zebra_stripes"]
    isl_mesh = mesh_layers["refuge_islands"]
    tac_mesh = mesh_layers["tactile_paving"]
    walk_mesh = mesh_layers["walkable_corridors"]

    res = {
        **metrics,
        "island_count": island_count,
        "tactile_count": tactile_count,
        "crossing_types": types_breakdown,
        "mesh_stats": {
            "zebra_vertices": len(z_mesh["vertices"]),
            "zebra_triangles": len(z_mesh["triangles"]),
            "island_vertices": len(isl_mesh["vertices"]),
            "island_triangles": len(isl_mesh["triangles"]),
            "tactile_vertices": len(tac_mesh["vertices"]),
            "tactile_triangles": len(tac_mesh["triangles"]),
            "walkable_corridors_count": len(walk_mesh),
        },
        "sample_crosswalks": [
            {
                "id": c.crosswalk_id,
                "type": c.crossing_type,
                "road_id": c.road_id,
                "stripes": c.num_stripes,
                "width": round(c.carriageway_width, 2),
                "has_island": c.has_island,
                "has_tactile": c.has_tactile,
            }
            for c in items[:10]
        ],
    }
    return res


def main() -> None:
    parser = argparse.ArgumentParser(description="Inspect CARLA Crosswalk Markings & Metrics")
    parser.add_argument("--manifest", type=Path, help="Path to Placements/crosswalks.json")
    parser.add_argument("--osm", type=Path, help="Path to input .osm file")
    parser.add_argument("--xodr", type=Path, help="Path to OpenDRIVE .xodr file")
    parser.add_argument("--json", type=Path, help="Write metrics report to JSON file")
    args = parser.parse_args()

    results = inspect_crosswalks(
        manifest_path=args.manifest,
        osm_path=args.osm,
        xodr_path=args.xodr,
    )

    print("\n" + "=" * 60)
    print("  CARLA Pedestrian Crosswalk & Zebra Marking Inspection")
    print("=" * 60)
    print(f"  Crosswalks Count:      {results['crosswalk_count']}")
    print(f"  Total Zebra Stripes:   {results['total_stripes']}")
    print(f"  Refuge Islands:        {results['island_count']}")
    print(f"  Tactile Warning Pads:  {results['tactile_count']}")
    print(f"  Road Alignment Rate:   {results['road_alignment_pct']}%")
    print(f"  DIN / StVO Compliant:  {'YES (PASS)' if results['passed_din_standards'] else 'NO (FAIL)'}")
    print("\n  Mesh Generation Stats:")
    ms = results.get("mesh_stats", {})
    print(f"    Zebra Stripes:       {ms.get('zebra_vertices', 0)} vertices, {ms.get('zebra_triangles', 0)} triangles")
    print(f"    Refuge Islands:      {ms.get('island_vertices', 0)} vertices, {ms.get('island_triangles', 0)} triangles")
    print(f"    Tactile Paving:      {ms.get('tactile_vertices', 0)} vertices, {ms.get('tactile_triangles', 0)} triangles")
    print(f"    Walkable Corridors:  {ms.get('walkable_corridors_count', 0)}")

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(results, indent=2), encoding="utf-8")
        print(f"\nSaved metrics report to: {args.json}")


if __name__ == "__main__":
    main()
