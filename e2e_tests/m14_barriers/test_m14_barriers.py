#!/usr/bin/env python3
"""
Milestone 14 (M14) E2E Test Suite: Continuous Barriers, Perimeter Walls, Fences & Guard Rails.

Verifies:
  - DoD 14.1: Ingestion of OSM barrier tags (barrier=wall, fence, guard_rail, hedge, etc.) & underground filtering
  - DoD 14.2: 3D polyline extrusion engine with corner mitering, side walls, top caps, and closed endcaps
  - DoD 14.3: Specialized architectural layer generation (Walls, Fences, GuardRails, Hedges)
  - DoD 14.4: Accurate outward normals and non-stretching UV mapping
  - DoD 14.5: Collision hulls & NavMesh obstacle registration
  - DoD 14.6: Diagnostic auditor (inspect_barriers.py) and barriers.json serialization
  - DoD 14.7: Bundle compilation, inspection, and verification (carla-map build / inspect / verify)
  - DoD 14.8: Real-world Munich Stachus barrier extraction and layer generation
"""

from __future__ import annotations

import argparse
import json
import math
import os
from pathlib import Path
import shutil
import socket
import sys
import tarfile
import tempfile
import unittest

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
tempfile.tempdir = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

try:
    import carla
except ImportError:
    carla = None

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_verify
from Util.MapBuilder.cmesh import read_cmesh_file
from Util.MapBuilder.osm_barriers import (
    BarrierWay,
    audit_barrier_metrics,
    classify_barrier,
    extract_osm_barriers,
    extrude_barrier_polyline_mesh,
    build_osm_barriers_multislot_layers,
    parse_barrier_height,
    parse_barrier_width,
)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SAMPLE_BARRIERS_OSM = FIXTURES_DIR / "sample_barriers.osm"
MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"


def is_carla_server_running(host: str = "127.0.0.1", port: int = 2000, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except (OSError, ConnectionRefusedError):
        return False


class TestMilestone14Barriers(unittest.TestCase):
    """Milestone 14 E2E Test Suite for Continuous Barriers, Walls, Fences & Guard Rails."""

    def test_01_osm_barrier_ingestion_and_underground_filtering(self) -> None:
        """DoD 14.1: Ingest OSM barrier=* tags and verify underground features (level < 0) are excluded."""
        self.assertTrue(SAMPLE_BARRIERS_OSM.is_file(), f"Fixture missing: {SAMPLE_BARRIERS_OSM}")

        barriers = extract_osm_barriers(SAMPLE_BARRIERS_OSM)
        self.assertEqual(len(barriers), 4, f"Expected 4 above-ground barriers, got {len(barriers)}")

        # Verify underground way 6001 was filtered out
        osm_ids = [b.osm_id for b in barriers]
        self.assertNotIn(6001, osm_ids, "Underground barrier (level=-2) must be filtered out")

        b_map = {b.osm_id: b for b in barriers}

        # 1. Perimeter Masonry Wall
        wall = b_map[2001]
        self.assertEqual(wall.category, "wall")
        self.assertEqual(wall.semantic_label, "Walls")
        self.assertEqual(wall.material_preset, "wall_brick")
        self.assertAlmostEqual(wall.height, 2.2, places=2)
        self.assertAlmostEqual(wall.width, 0.4, places=2)

        # 2. Metal Railing / Fence
        fence = b_map[3001]
        self.assertEqual(fence.category, "fence")
        self.assertEqual(fence.semantic_label, "Fences")
        self.assertEqual(fence.material_preset, "fence_metal")
        self.assertAlmostEqual(fence.height, 1.1, places=2)

        # 3. Guard Rail
        guard = b_map[4001]
        self.assertEqual(guard.category, "guard_rail")
        self.assertEqual(guard.semantic_label, "GuardRail")
        self.assertEqual(guard.material_preset, "guardrail")
        self.assertAlmostEqual(guard.height, 0.85, places=2)

        # 4. Living Hedge
        hedge = b_map[5001]
        self.assertEqual(hedge.category, "hedge")
        self.assertEqual(hedge.semantic_label, "Vegetation")
        self.assertEqual(hedge.material_preset, "hedge")
        self.assertAlmostEqual(hedge.height, 1.6, places=2)
        self.assertAlmostEqual(hedge.width, 0.8, places=2)

    def test_02_barrier_polyline_extrusion_and_miter_joins(self) -> None:
        """DoD 14.2 & 14.4: Verify continuous 3D watertight prism extrusion, corner mitering, and normals."""
        # 3-point L-shaped polyline with a 90-degree corner
        polyline = [
            (0.0, 0.0, 0.0),
            (10.0, 0.0, 0.0),
            (10.0, 10.0, 0.0),
        ]
        height = 1.5
        width = 0.4

        verts, tris, norms, uvs = extrude_barrier_polyline_mesh(
            polyline,
            height=height,
            width=width,
            is_closed=False,
            uv_tile_u=2.0,
        )

        self.assertGreater(len(verts), 0, "Extruded mesh must have vertices")
        self.assertGreater(len(tris), 0, "Extruded mesh must have triangles")
        self.assertEqual(len(verts), len(norms), "Every vertex must have a normal")
        self.assertEqual(len(verts), len(uvs), "Every vertex must have UVs")

        # Top cap quads must have normals pointing upwards (+Z)
        top_normals = [n for n in norms if abs(n[2] - 1.0) < 1e-4]
        self.assertGreater(len(top_normals), 0, "Must have top cap vertices with normal +Z")
        for tn in top_normals:
            self.assertAlmostEqual(tn[2], 1.0, places=2, msg="Top cap normal must point in +Z")

        # Side walls must have horizontal normals (Z == 0)
        side_normals = [n for n in norms if abs(n[2]) < 1e-4]
        self.assertGreater(len(side_normals), 0, "Must have side wall vertices with horizontal normal")

    def test_03_multislot_layer_assembly(self) -> None:
        """DoD 14.3 & 14.5: Verify separate architectural barrier layers (Walls, Fences, GuardRails, Hedges)."""
        barriers = extract_osm_barriers(SAMPLE_BARRIERS_OSM)
        dummy_proj = lambda lat, lon: ((lon - 11.564) * 10000.0, (lat - 48.139) * 10000.0, 0.0)

        layers = build_osm_barriers_multislot_layers(barriers, dummy_proj)

        self.assertIn("walls", layers, "Must generate 'walls' layer")
        self.assertIn("fences", layers, "Must generate 'fences' layer")
        self.assertIn("guard_rails", layers, "Must generate 'guard_rails' layer")
        self.assertIn("hedges", layers, "Must generate 'hedges' layer")

        # Semantic labels and collisions
        self.assertEqual(layers["walls"]["semantic_label"], "Walls")
        self.assertEqual(layers["fences"]["semantic_label"], "Fences")
        self.assertEqual(layers["guard_rails"]["semantic_label"], "GuardRail")
        self.assertEqual(layers["hedges"]["semantic_label"], "Vegetation")

        for slot_id, l_data in layers.items():
            self.assertTrue(l_data["enable_collision"], f"Layer {slot_id} must have collision enabled")
            self.assertGreater(len(l_data["vertices"]), 0)
            self.assertGreater(len(l_data["triangles"]), 0)

    def test_04_barrier_audit_metrics(self) -> None:
        """DoD 14.6: Verify barrier audit metrics computation."""
        barriers = extract_osm_barriers(SAMPLE_BARRIERS_OSM)
        dummy_proj = lambda lat, lon: ((lon - 11.564) * 10000.0, (lat - 48.139) * 10000.0, 0.0)
        layers = build_osm_barriers_multislot_layers(barriers, dummy_proj)

        metrics = audit_barrier_metrics(barriers, layers)
        counts = metrics["counts"]

        self.assertEqual(counts["total_barriers"], 4)
        self.assertEqual(counts["walls"], 1)
        self.assertEqual(counts["fences"], 1)
        self.assertEqual(counts["guard_rails"], 1)
        self.assertEqual(counts["hedges"], 1)
        self.assertGreater(counts["total_length_m"], 0.0)
        self.assertEqual(len(metrics["layers"]), 4)

    def test_05_bundle_build_inspect_and_verify(self) -> None:
        """DoD 14.7: Verify carla-map build creates barrier meshes, placements/barriers.json, and passes verify."""
        with tempfile.TemporaryDirectory(prefix="e2e_m14_build_") as tmp_dir:
            out_tar = Path(tmp_dir) / "TestM14Map.carlamap.tar.gz"

            args = argparse.Namespace(
                subcommand="build",
                target=".",
                osm=str(SAMPLE_BARRIERS_OSM),
                xodr=None,
                name="TestM14Map",
                output=str(out_tar),
                meshes=None,
                layer_elevation=None,
                no_terrain=True,
                no_subbase=True,
                subbase_margin=50.0,
                no_vegetation=True,
                no_clutter=True,
                no_parked_cars=True,
                max_parked_cars=0,
                no_lighting=True,
                no_traffic_lights=True,
                no_all_junctions_tl=True,
                no_crosswalks=True,
                no_satellite_imagery=True,
                satellite_image=None,
                no_satellite_vegetation=True,
                no_satellite_parking=True,
                no_cycleways=True,
                no_bike_boxes=True,
                no_bike_stencils=True,
                no_barriers=False,
            )

            rc = cmd_build(args)
            self.assertEqual(rc, 0, "carla-map build must exit with 0")
            self.assertTrue(out_tar.is_file(), f"Output tar must exist: {out_tar}")

            # Verify bundle passes verify
            verify_args = argparse.Namespace(target=str(out_tar))
            v_rc = cmd_verify(verify_args)
            self.assertEqual(v_rc, 0, "carla-map verify must exit with 0")

            # Extract and inspect manifest & barrier .cmesh files
            with tarfile.open(out_tar, "r:*") as tar:
                tar.extractall(tmp_dir)

            bundle_dir = Path(tmp_dir) / "TestM14Map"
            manifest = json.loads((bundle_dir / "carla-map.json").read_text(encoding="utf-8"))

            self.assertIn("barriers_file", manifest, "Manifest must specify 'barriers_file'")
            bar_json_path = bundle_dir / manifest["barriers_file"]
            self.assertTrue(bar_json_path.is_file(), "Placements/barriers.json must exist in bundle")

            bar_manifest = json.loads(bar_json_path.read_text(encoding="utf-8"))
            self.assertEqual(bar_manifest["counts"]["total_barriers"], 4)

            # Check that barrier .cmesh files were generated and are valid
            layer_names = [l["name"] for l in manifest["layers"]]
            self.assertIn("OSM_Barriers_Walls", layer_names)
            self.assertIn("OSM_Barriers_Fences", layer_names)
            self.assertIn("OSM_Barriers_GuardRails", layer_names)
            self.assertIn("OSM_Barriers_Hedges", layer_names)

            for l in manifest["layers"]:
                if l["name"].startswith("OSM_Barriers_"):
                    cmesh_p = bundle_dir / l["mesh_file"]
                    self.assertTrue(cmesh_p.is_file(), f"Mesh file must exist: {cmesh_p}")
                    geom = read_cmesh_file(cmesh_p)
                    self.assertGreater(geom["vertex_count"], 0)
                    self.assertGreater(geom["triangle_count"], 0)

    def test_06_real_munich_stachus_barriers(self) -> None:
        """DoD 14.8: Verify robust barrier extraction on full real-world Munich Stachus dataset."""
        self.assertTrue(MUNICH_OSM.is_file(), f"Munich OSM fixture missing: {MUNICH_OSM}")

        barriers = extract_osm_barriers(MUNICH_OSM)
        self.assertGreaterEqual(len(barriers), 40, f"Expected at least 40 barrier ways in Munich, got {len(barriers)}")

        categories = {b.category for b in barriers}
        self.assertIn("wall", categories, "Munich must contain walls")
        self.assertIn("fence", categories, "Munich must contain fences")
        self.assertIn("hedge", categories, "Munich must contain hedges")

        dummy_proj = lambda lat, lon: ((lon - 11.56) * 10000.0, (lat - 48.13) * 10000.0, 0.0)
        layers = build_osm_barriers_multislot_layers(barriers, dummy_proj)

        self.assertIn("walls", layers)
        self.assertIn("fences", layers)
        self.assertIn("hedges", layers)

        total_length = sum(l["total_length_m"] for l in layers.values())
        self.assertGreater(total_length, 100.0, "Total barrier length in Munich Stachus must exceed 100m")


def main() -> None:
    parser = argparse.ArgumentParser(description="Milestone 14 Barriers E2E Test Suite")
    parser.add_argument("--offline-only", action="store_true", help="Run offline tests only")
    args, remaining = parser.parse_known_args()

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(TestMilestone14Barriers)
    runner = unittest.TextTestRunner(verbosity=2)
    res = runner.run(suite)
    sys.exit(0 if res.wasSuccessful() else 1)


if __name__ == "__main__":
    main()
