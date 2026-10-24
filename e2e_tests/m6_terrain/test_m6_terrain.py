#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 6 (M6).

Verifies the Urban Terrain, Landuse Ground Meshes & Sub-Base Floor Pipeline:
  - DoD 6.1: OSM landuse, leisure, natural, and pedestrian area polygons are parsed and classified.
  - DoD 6.2: 2D polygon triangulation handles simple and complex polygons with inner holes.
  - DoD 6.3: Perimeter downward foundation skirts prevent edge light leaks across elevation changes.
  - DoD 6.4: Continuous sub-base ground plane seals the entire map bounding box with zero void abysses.
  - DoD 6.5: Automated landuse surface coverage inspection confirms >= 99.9% coverage.
  - DoD 6.6: Material presets (grass, water, sidewalk_tiles, dirt) are correctly assigned to .cmesh layers.
  - DoD 6.7: Live CARLA simulator verification (when server is running) validating world load and active environment objects.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import sys
import tempfile
import time
import unittest

import carla

E2E_DIR = Path(__file__).resolve().parent
REPO_ROOT = E2E_DIR.parent.parent
sys.path.insert(0, str(REPO_ROOT))

from Util.MapBuilder.cmesh import read_cmesh_file
from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_install, cmd_verify
from Util.MapBuilder.osm_terrain import (
    build_osm_terrain_layers,
    classify_terrain_tags,
    extract_osm_terrain_polygons,
    generate_subbase_grid,
    triangulate_terrain_polygon,
)
from Util.MapBuilder.inspect_landuse_coverage import inspect_coverage

FIXTURES_DIR = E2E_DIR / "fixtures"
SAMPLE_TERRAIN_OSM = FIXTURES_DIR / "sample_terrain.osm"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone6Terrain(unittest.TestCase):
    """Milestone 6 E2E Test Suite for Terrain, Landuse and Sub-Base Ground Plane."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m6_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_terrain_polygon_extraction(self) -> None:
        """DoD 6.1: Parse and classify OSM landuse, leisure, natural, and pedestrian polygons."""
        self.assertTrue(SAMPLE_TERRAIN_OSM.is_file(), "sample_terrain.osm fixture must exist")
        polys = extract_osm_terrain_polygons(SAMPLE_TERRAIN_OSM)
        self.assertGreaterEqual(len(polys), 3, "Must extract at least 3 terrain features")

        categories = {p.category for p in polys}
        self.assertIn("grass", categories, "Must classify green space (Central Garden)")
        self.assertIn("water", categories, "Must classify water basin (Reflecting Pool)")
        self.assertIn("plaza", categories, "Must classify pedestrian plaza (Town Market Square)")

        # Verify tag classification logic
        self.assertEqual(classify_terrain_tags({"landuse": "grass"}), "grass")
        self.assertEqual(classify_terrain_tags({"natural": "water"}), "water")
        self.assertEqual(classify_terrain_tags({"place": "square"}), "plaza")
        self.assertEqual(classify_terrain_tags({"highway": "pedestrian", "area": "yes"}), "plaza")
        self.assertIsNone(classify_terrain_tags({"building": "residential"}), "Buildings must not be classified as terrain")

    def test_02_triangulation_and_foundation_skirts(self) -> None:
        """DoD 6.2 & 6.3: Polygon triangulation with holes and vertical perimeter foundation skirts."""
        outer = [(0.0, 0.0), (40.0, 0.0), (40.0, 40.0), (0.0, 40.0)]
        hole = [(15.0, 15.0), (25.0, 15.0), (25.0, 25.0), (15.0, 25.0)]

        verts, tris, norms, uvs = triangulate_terrain_polygon(
            outer,
            inner_holes=[hole],
            base_z=0.0,
            skirt_depth=0.8,
            uv_scale=10.0,
        )

        self.assertGreater(len(verts), 0, "Vertices must not be empty")
        self.assertGreater(len(tris), 0, "Triangles must not be empty")
        self.assertEqual(len(verts), len(norms), "Normals must match vertex count")
        self.assertEqual(len(verts), len(uvs), "UVs must match vertex count")

        # Verify vertical foundation skirts: at least some vertices must have z = -0.8
        skirt_bottom_verts = [v for v in verts if abs(v[2] - (-0.8)) < 1e-3]
        self.assertGreaterEqual(len(skirt_bottom_verts), 4, "Must generate skirt vertices extending down to -0.8m")

        # Verify outward horizontal normals on skirts (nz == 0.0)
        skirt_normals = [n for n in norms if abs(n[2]) < 1e-4]
        self.assertGreater(len(skirt_normals), 0, "Must generate horizontal outward normals for skirts")

    def test_03_subbase_grid_generation(self) -> None:
        """DoD 6.4: Continuous sub-base regular 2D grid covering bounding box + margin."""
        min_x, max_x = -100.0, 100.0
        min_y, max_y = -50.0, 50.0
        base_z = -0.10
        margin = 30.0

        verts, tris, norms, uvs = generate_subbase_grid(
            min_x=min_x,
            max_x=max_x,
            min_y=min_y,
            max_y=max_y,
            base_z=base_z,
            margin=margin,
            cell_size=20.0,
            uv_scale=10.0,
        )

        self.assertGreater(len(verts), 10, "Sub-base grid must have multiple vertices")
        self.assertGreater(len(tris), 10, "Sub-base grid must have multiple triangles")

        # Verify boundary coverage: min_x should be <= min_x - margin
        xs = [v[0] for v in verts]
        ys = [v[1] for v in verts]
        zs = [v[2] for v in verts]
        self.assertLessEqual(min(xs), min_x - margin + 1e-3)
        self.assertGreaterEqual(max(xs), max_x + margin - 1e-3)
        self.assertLessEqual(min(ys), min_y - margin + 1e-3)
        self.assertGreaterEqual(max(ys), max_y + margin - 1e-3)

        # All Z values should match base_z exactly
        for z in zs:
            self.assertAlmostEqual(z, base_z, places=3)

    def test_04_bundle_build_and_coverage_inspection(self) -> None:
        """DoD 6.5 & 6.6: Build map bundle with terrain and verify >= 99.9% coverage and 0 abysses."""
        bundle_out = self.test_dir / "TestTerrainMap.carlamap.tar.gz"

        class BuildArgs:
            target = "."
            name = "TestTerrainMap"
            output = str(bundle_out)
            osm = str(SAMPLE_TERRAIN_OSM)
            xodr = None
            meshes = None
            layer_elevation = 0.0
            no_terrain = False
            no_subbase = False
            subbase_margin = 40.0

        rc = cmd_build(BuildArgs())
        self.assertEqual(rc, 0, "carla-map build must succeed with terrain enabled")
        self.assertTrue(bundle_out.is_file(), "Bundle archive must be created")

        # Verify bundle integrity
        class VerifyArgs:
            target = str(bundle_out)

        self.assertEqual(cmd_verify(VerifyArgs()), 0, "carla-map verify must pass 100%")

        # Inspect coverage and verify DoD criteria
        report = inspect_coverage(bundle_out)
        self.assertTrue(report["subbase_present"], "Sub-base ground plane must be present")
        self.assertGreaterEqual(report["coverage_percentage"], 99.9, "Coverage must meet >= 99.9% threshold")
        self.assertLess(report["uncovered_void_area_m2"], 1.0, "Uncovered void area must be 0 m²")

        # Verify material presets and semantic labels
        layer_names = [l["name"] for l in report["layers"]]
        layer_mats = {l["name"]: l["material"] for l in report["layers"]}
        layer_labels = {l["name"]: l["label"] for l in report["layers"]}

        self.assertIn("OSM_Terrain_Grass", layer_names)
        self.assertIn("OSM_Terrain_Water", layer_names)
        self.assertIn("OSM_Terrain_Plaza", layer_names)
        self.assertIn("OSM_Terrain_Subbase", layer_names)

        self.assertEqual(layer_mats["OSM_Terrain_Grass"], "grass")
        self.assertEqual(layer_mats["OSM_Terrain_Water"], "water")
        self.assertEqual(layer_mats["OSM_Terrain_Plaza"], "sidewalk_tiles")
        self.assertEqual(layer_mats["OSM_Terrain_Subbase"], "dirt")

        self.assertEqual(layer_labels["OSM_Terrain_Grass"], "Vegetation")
        self.assertEqual(layer_labels["OSM_Terrain_Water"], "Water")
        self.assertEqual(layer_labels["OSM_Terrain_Plaza"], "Ground")
        self.assertEqual(layer_labels["OSM_Terrain_Subbase"], "Terrain")

    def test_05_live_simulator_terrain_and_semantics(self) -> None:
        """DoD 6.7: Load bundle in live CARLA simulator and query terrain environment objects."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"Live CARLA simulator not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(30.0)

        # Install bundle into simulator
        bundle_out = self.test_dir / "TestTerrainMap.carlamap.tar.gz"
        class InstallArgs:
            target = str(bundle_out)
            server = ""

        rc = cmd_install(InstallArgs())
        self.assertEqual(rc, 0, "carla-map install must succeed")

        # Load world
        world = client.load_world("TestTerrainMap")
        self.assertIsNotNone(world, "client.load_world must return valid world")

        # Query environment objects by semantic label
        terrain_objs = world.get_environment_objects(carla.CityObjectLabel.Terrain)
        veg_objs = world.get_environment_objects(carla.CityObjectLabel.Vegetation)
        water_objs = world.get_environment_objects(carla.CityObjectLabel.Water)
        building_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

        self.assertGreater(len(terrain_objs), 0, "Must have active Terrain environment objects")
        self.assertGreater(len(veg_objs), 0, "Must have active Vegetation environment objects")
        self.assertGreater(len(water_objs), 0, "Must have active Water environment objects")
        self.assertGreater(len(building_objs), 0, "Must have active Building environment objects")


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 6 Terrain E2E Test Suite")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA server host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA server port")
    parser.add_argument("--offline-only", action="store_true", help="Run only offline tests")
    args, unknown = parser.parse_known_args()

    os.environ["CARLA_E2E_HOST"] = args.host
    os.environ["CARLA_E2E_PORT"] = str(args.port)
    os.environ["CARLA_E2E_OFFLINE_ONLY"] = "1" if args.offline_only else "0"

    unittest_args = [sys.argv[0]] + unknown
    unittest.main(argv=unittest_args)
    return 0


if __name__ == "__main__":
    main()

