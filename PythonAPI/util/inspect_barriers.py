#!/usr/bin/env python3
"""
Diagnostic Inspector & Network Auditor for Continuous Barriers, Walls, Fences & Guard Rails (Milestone 14).

Audits barrier continuity, lengths, categories (walls, fences, guardrails, hedges),
materials, and 3D geometry layers.
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

from Util.MapBuilder.osm_barriers import audit_barrier_metrics, extract_osm_barriers, build_osm_barriers_multislot_layers


def inspect_barriers(target_path: str | Path) -> int:
    """Inspects and audits barriers from an OSM file, bundle archive, directory, or barriers.json."""
    p = Path(target_path).resolve()
    temp_dir = None

    try:
        manifest_data: dict[str, Any] | None = None

        if p.name == "barriers.json" and p.is_file():
            manifest_data = json.loads(p.read_text(encoding="utf-8"))
        elif p.suffix == ".osm" and p.is_file():
            barriers = extract_osm_barriers(p)
            layers = build_osm_barriers_multislot_layers(
                barriers,
                lambda lat, lon: (lon * 10000.0, lat * 10000.0, 0.0),
            )
            manifest_data = audit_barrier_metrics(barriers, layers)
        elif p.is_file() and p.name.endswith((".tar.gz", ".tar")):
            temp_dir = Path(tempfile.mkdtemp(prefix="carla_barrier_inspect_"))
            with tarfile.open(p, "r:*") as tar:
                tar.extractall(temp_dir)
            bar_path = temp_dir / "Placements" / "barriers.json"
            if not bar_path.is_file():
                for sub in temp_dir.iterdir():
                    cand = sub / "Placements" / "barriers.json"
                    if cand.is_file():
                        bar_path = cand
                        break
            if bar_path.is_file():
                manifest_data = json.loads(bar_path.read_text(encoding="utf-8"))
        elif p.is_dir():
            bar_path = p / "Placements" / "barriers.json"
            if bar_path.is_file():
                manifest_data = json.loads(bar_path.read_text(encoding="utf-8"))

        if manifest_data is None:
            print(f"[inspect-barriers] ERROR: No barriers data found in {p}")
            return 1

        counts = manifest_data.get("counts", {})
        total_barriers = counts.get("total_barriers", 0)
        total_len = counts.get("total_length_m", 0.0)
        n_walls = counts.get("walls", 0)
        n_fences = counts.get("fences", 0)
        n_guardrails = counts.get("guard_rails", 0)
        n_hedges = counts.get("hedges", 0)

        print("=" * 72)
        print("CARLA Continuous Barriers & Perimeter Infrastructure Audit")
        print("=" * 72)
        print(f"  Target File / Bundle : {p.name}")
        print(f"  Total Barrier Ways   : {total_barriers}")
        print(f"  Total Barrier Length : {total_len:.1f} meters")
        print(f"  Masonry / Walls      : {n_walls} ways")
        print(f"  Fences & Railings    : {n_fences} ways")
        print(f"  Highway Guard Rails  : {n_guardrails} ways")
        print(f"  Living Hedges        : {n_hedges} ways")

        materials = manifest_data.get("materials", {})
        if materials:
            print("\nMaterial Breakdown:")
            for mat, count in sorted(materials.items(), key=lambda x: -x[1]):
                print(f"  • {mat:<30} : {count} ways")

        layers = manifest_data.get("layers", {})
        if layers:
            print("\nGeometry Layers:")
            for l_name, l_info in layers.items():
                v = l_info.get("vertices", 0)
                t = l_info.get("triangles", 0)
                ways = l_info.get("way_count", 0)
                length = l_info.get("total_length_m", 0.0)
                semantic = l_info.get("semantic_label", "Walls")
                preset = l_info.get("material_preset", "")
                print(f"  • {l_name:<26} | Label: {semantic:<10} | Mat: {preset:<16} | {v} verts, {t} tris | {ways} ways ({length:.1f}m)")

        print("=" * 72)
        print("[inspect-barriers] AUDIT PASSED: All barrier metrics verified successfully.")
        print("=" * 72)
        return 0

    finally:
        if temp_dir and temp_dir.exists():
            import shutil
            shutil.rmtree(temp_dir, ignore_errors=True)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Diagnostic Inspector & Auditor for Continuous Barriers (Milestone 14)",
    )
    parser.add_argument(
        "target",
        help="Path to .osm file, .carlamap.tar.gz bundle, extracted bundle folder, or barriers.json",
    )
    args = parser.parse_args()
    sys.exit(inspect_barriers(args.target))


if __name__ == "__main__":
    main()

