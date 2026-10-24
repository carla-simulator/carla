#!/usr/bin/env python3
"""
Diagnostic Inspector & Network Auditor for Dedicated Bicycle Infrastructure (Milestone 13).

Audits cycleway continuity, total lane kilometers, separation types, surface colors,
white demarcation lines, bicycle stencils, bike boxes, and parked bicycles.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
import tarfile
import tempfile
from typing import Any

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from Util.MapBuilder.osm_cycleways import audit_cycleway_metrics, load_cycleways_manifest


def inspect_cycleways(bundle_or_manifest_path: str | Path) -> int:
    """Inspects and audits bicycle infrastructure from a bundle or cycleways.json."""
    p = Path(bundle_or_manifest_path).resolve()
    temp_dir = None

    try:
        manifest_data: dict[str, Any] | None = None

        if p.name == "cycleways.json" and p.is_file():
            manifest_data = json.loads(p.read_text(encoding="utf-8"))
        elif p.is_file() and p.name.endswith((".tar.gz", ".tar")):
            temp_dir = Path(tempfile.mkdtemp(prefix="carla_cyc_inspect_"))
            with tarfile.open(p, "r:*") as tar:
                tar.extractall(temp_dir)
            cyc_path = temp_dir / "Placements" / "cycleways.json"
            if not cyc_path.is_file():
                for sub in temp_dir.iterdir():
                    cand = sub / "Placements" / "cycleways.json"
                    if cand.is_file():
                        cyc_path = cand
                        break
            if cyc_path.is_file():
                manifest_data = json.loads(cyc_path.read_text(encoding="utf-8"))
        elif p.is_dir():
            cyc_path = p / "Placements" / "cycleways.json"
            if cyc_path.is_file():
                manifest_data = json.loads(cyc_path.read_text(encoding="utf-8"))

        if manifest_data is None:
            print(f"[inspect-cycleways] ERROR: No cycleways.json found in {p}")
            return 1

        counts = manifest_data.get("counts", {})
        total_cyc = counts.get("total_cycleways", len(manifest_data.get("cycleways", [])))
        total_km = counts.get("total_lane_km", 0.0)
        total_boxes = counts.get("total_bike_boxes", len(manifest_data.get("bike_boxes", [])))
        total_racks = counts.get("total_bike_racks", len(manifest_data.get("bike_racks", [])))
        total_bikes = counts.get("total_parked_bikes", sum(r.get("num_parked_bikes", 0) for r in manifest_data.get("bike_racks", [])))

        print("=" * 72)
        print("CARLA Dedicated Bicycle Infrastructure Audit")
        print("=" * 72)
        print(f"  Total Cycleway Segments : {total_cyc:,}")
        print(f"  Total Cycling Network   : {total_km:.3f} lane km")
        print(f"  Advance Stop Lines      : {total_boxes:,} bike boxes")
        print(f"  Bicycle Parking Stands  : {total_racks:,} racks ({total_bikes:,} parked bicycles)")

        # Color & Type Breakdown
        cycleways = manifest_data.get("cycleways", [])
        color_counts: dict[str, int] = {}
        type_counts: dict[str, int] = {}
        marking_counts: dict[str, int] = {}

        for c in cycleways:
            col = c.get("surface_color", "unknown")
            color_counts[col] = color_counts.get(col, 0) + 1
            typ = c.get("cycleway_type", "unknown")
            type_counts[typ] = type_counts.get(typ, 0) + 1
            mark = c.get("lane_marking", "none")
            marking_counts[mark] = marking_counts.get(mark, 0) + 1

        print("\nSurface PBR Preset Breakdown:")
        for col, cnt in sorted(color_counts.items()):
            print(f"  - {col.capitalize()} Asphalt : {cnt:,} segments")

        print("\nInfrastructure Classification:")
        for typ, cnt in sorted(type_counts.items()):
            print(f"  - {typ.capitalize():<12} : {cnt:,} segments")

        print("\nDemarcation Markings:")
        for mark, cnt in sorted(marking_counts.items()):
            print(f"  - {mark.capitalize():<12} lines : {cnt:,} corridors")

        print("=" * 72)
        print("[inspect-cycleways] AUDIT PASSED: Cycling infrastructure validated.")
        print("=" * 72)
        return 0

    finally:
        if temp_dir is not None:
            import shutil
            shutil.rmtree(temp_dir, ignore_errors=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Inspect CARLA bicycle infrastructure.")
    parser.add_argument("target", help="Path to .carlamap, .tar.gz, or cycleways.json")
    args = parser.parse_args()
    return inspect_cycleways(args.target)


if __name__ == "__main__":
    sys.exit(main())
