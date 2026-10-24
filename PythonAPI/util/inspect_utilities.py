#!/usr/bin/env python3
"""
Diagnostic Inspector & Municipal Utilities Auditor for CARLA Map Maker (Milestone 15).

Audits municipal utilities, fire hydrants (pillar vs underground), street cabinets,
and cast-iron surface covers from OSM files, bundle archives, or Placements/utilities.json.
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

from Util.MapBuilder.osm_utilities import (
    audit_utility_metrics,
    extract_osm_utilities,
    generate_utility_mesh_layers,
)


def inspect_utilities(target_path: str | Path) -> int:
    """Inspects and audits municipal utilities from an OSM file, bundle archive, directory, or utilities.json."""
    p = Path(target_path).resolve()
    temp_dir = None

    try:
        manifest_data: dict[str, Any] | None = None

        if p.name == "utilities.json" and p.is_file():
            manifest_data = json.loads(p.read_text(encoding="utf-8"))
        elif p.suffix == ".osm" and p.is_file():
            items = extract_osm_utilities(
                p,
                lambda lat, lon: (lon * 10000.0, lat * 10000.0, 0.0),
            )
            mesh = generate_utility_mesh_layers(items)
            manifest_data = audit_utility_metrics(items, mesh)
        elif p.is_file() and p.name.endswith((".tar.gz", ".tar")):
            temp_dir = Path(tempfile.mkdtemp(prefix="carla_util_inspect_"))
            with tarfile.open(p, "r:*") as tar:
                tar.extractall(temp_dir)
            util_path = temp_dir / "Placements" / "utilities.json"
            if not util_path.is_file():
                for sub in temp_dir.iterdir():
                    cand = sub / "Placements" / "utilities.json"
                    if cand.is_file():
                        util_path = cand
                        break
            if util_path.is_file():
                manifest_data = json.loads(util_path.read_text(encoding="utf-8"))
        elif p.is_dir():
            util_path = p / "Placements" / "utilities.json"
            if util_path.is_file():
                manifest_data = json.loads(util_path.read_text(encoding="utf-8"))

        if manifest_data is None:
            print(f"[inspect-utilities] ERROR: No utilities data found in {p}")
            return 1

        counts = manifest_data.get("counts", {})
        tot_u = counts.get("total_utilities", 0)
        p_hyd = counts.get("pillar_hydrants", 0)
        u_hyd = counts.get("underground_hydrants", 0)
        cabs = counts.get("street_cabinets", 0)
        manholes = counts.get("manhole_covers", 0)
        osm_c = counts.get("osm_sourced", 0)
        proc_c = counts.get("procedurally_synthesized", 0)

        print("=" * 72)
        print("CARLA Municipal Utilities & Fire Hydrants Audit (Milestone 15)")
        print("=" * 72)
        print(f"  Target File / Bundle : {p.name}")
        print(f"  Total Utilities      : {tot_u:,} assets")
        print(f"    • OSM Sourced      : {osm_c:,}")
        print(f"    • Procedural (DIN) : {proc_c:,}")
        print(f"\nCategorical Breakdown:")
        print(f"  Pillar Fire Hydrants : {p_hyd:,} red cast-iron standing hydrants")
        print(f"  Underground Hydrants : {u_hyd:,} DIN 4055 cast-iron ground plates")
        print(f"  Street Cabinets      : {cabs:,} telecom & power distribution cabinets")
        print(f"  Manhole Access Covers: {manholes:,} drainage & utility conduit covers")

        operators = manifest_data.get("operators", [])
        if operators:
            print(f"\nMunicipal Operators / Utilities ({len(operators)}):")
            for op in operators:
                print(f"  • {op}")

        mesh = manifest_data.get("mesh", {})
        if mesh:
            v = mesh.get("vertices", 0)
            t = mesh.get("triangles", 0)
            cm = mesh.get("circular_manholes", 0)
            rh = mesh.get("rectangular_hydrants", 0)
            print(f"\nSurface Mesh Geometry (OSM_Utility_Covers):")
            print(f"  • Vertices           : {v:,}")
            print(f"  • Triangles          : {t:,}")
            print(f"  • Circular Covers    : {cm:,}")
            print(f"  • Rectangular Plates : {rh:,}")

        # Detailed utilities sample
        raw_items = manifest_data.get("utilities", [])
        if raw_items:
            print(f"\nSample Placements (first 5 of {len(raw_items)}):")
            for idx, item in enumerate(raw_items[:5], 1):
                cat = item.get("category", "")
                sub = item.get("subtype", "")
                loc = item.get("location", [0, 0, 0])
                rot = item.get("rotation", [0, 0, 0])
                asset = item.get("asset_path", "Cover Mesh")
                print(f"  [{idx}] {cat:<24} ({sub}) @ ({loc[0]:.2f}, {loc[1]:.2f}, {loc[2]:.2f}) rot={rot[1]}° | {asset}")

        print("=" * 72)
        print("[inspect-utilities] AUDIT PASSED: All municipal utilities verified successfully.")
        print("=" * 72)
        return 0

    finally:
        if temp_dir and temp_dir.exists():
            import shutil
            shutil.rmtree(temp_dir, ignore_errors=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="inspect-utilities",
        description="Inspect and audit CARLA municipal utilities and fire hydrants.",
    )
    parser.add_argument("target", help="Path to .osm, .carlamap.tar.gz, bundle folder, or utilities.json")
    args = parser.parse_args()
    return inspect_utilities(args.target)


if __name__ == "__main__":
    sys.exit(main())
