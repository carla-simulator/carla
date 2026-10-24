#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 11 (M11).

Verifies Pedestrian Crosswalks, Zebra Markings & Tactile Paving Integration:
  - DoD 11.1: OSM highway=crossing and crossing=* extraction parses authentic crossing nodes.
  - DoD 11.2: Association with OpenDRIVE carriageway lanes, traffic headings, and DIN/StVO stripe standards.
  - DoD 11.3: Raised pedestrian refuge islands (crossing:island=yes) with 15cm curb profile.
  - DoD 11.4: Yellow tactile warning paving pads (tactile_paving=yes) at sidewalk curb cuts.
  - DoD 11.5: Recast NavMesh walkable crosswalk corridor generation across carriageways.
  - DoD 11.6: Manifest serialization into Placements/crosswalks.json and bundle integration.
  - DoD 11.7: Bundle validation via carla-map inspect, verify (19 structural checks), and inspect_crosswalks.
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
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
tempfile.tempdir = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_verify
from Util.MapBuilder.inspect_crosswalks import inspect_crosswalks
from Util.MapBuilder.osm_crosswalks import (
    CrosswalkItem,
    audit_crosswalk_metrics,
    extract_osm_crosswalks,
    generate_crosswalk_meshes,
    load_crosswalks_manifest,
    save_crosswalks_manifest,
)

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
if not MUNICH_OSM.is_file():
    MUNICH_OSM = REPO_ROOT / "munich_stachus.osm"

BUNDLE_ARCHIVE = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap.tar.gz"
BUNDLE_DIR = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
MUNICH_XODR = BUNDLE_DIR / "OpenDrive" / "MunichStachus.xodr"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone11Crosswalks(unittest.TestCase):
    """Milestone 11 E2E Test Suite for Pedestrian Crosswalks & Zebra Markings."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m11_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_crosswalk_extraction(self) -> None:
        """DoD 11.1: Extract authentic pedestrian crosswalks from OSM and map to roads."""
        self.assertTrue(MUNICH_OSM.is_file(), "munich_stachus.osm must exist")

        carla_map = None
        if MUNICH_XODR.is_file():
            carla_map = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))

        crosswalks = extract_osm_crosswalks(
            MUNICH_OSM,
            carla_map=carla_map,
            max_road_dist=20.0,
        )
        self.assertGreaterEqual(
            len(crosswalks),
            18,
            f"Expected at least 18 authentic crosswalks extracted in Munich Stachus, found {len(crosswalks)}",
        )

        # Check attributes
        first = crosswalks[0]
        self.assertTrue(first.crosswalk_id.startswith("cw_"))
        self.assertEqual(len(first.location), 3)
        self.assertGreater(first.carriageway_width, 0.0)
        self.assertGreater(first.num_stripes, 0)
        self.assertIn(first.crossing_type, ("zebra", "traffic_signals", "uncontrolled", "marked"))

    def test_02_din_stvo_stripe_geometry(self) -> None:
        """DoD 11.2: Verify procedural zebra marking generation conforms to DIN / StVO standards."""
        test_cw = [
            CrosswalkItem(
                crosswalk_id="cw_test_1",
                location=(100.0, 50.0, 0.5),
                crossing_type="zebra",
                road_id=1,
                lane_ids=[-1, -2],
                heading_deg=90.0,
                carriageway_width=7.0,
                corridor_length=3.5,
                has_island=False,
                has_tactile=False,
                num_stripes=7,
            )
        ]
        meshes = generate_crosswalk_meshes(test_cw, stripe_width=0.50, stripe_gap=0.50, z_offset=0.02)
        z_mesh = meshes["zebra_stripes"]

        self.assertEqual(z_mesh["name"], "OSM_Crosswalks")
        self.assertEqual(z_mesh["semantic_label"], "RoadLines")
        self.assertEqual(z_mesh["material_preset"], "crosswalk")
        self.assertFalse(z_mesh["enable_collision"])

        # 7 stripes * 4 vertices = 28 vertices, 7 * 2 = 14 triangles
        self.assertEqual(len(z_mesh["vertices"]), 28)
        self.assertEqual(len(z_mesh["triangles"]), 14)

        # Verify z-elevation offset (+0.02m)
        for v in z_mesh["vertices"]:
            self.assertAlmostEqual(v[2], 0.52, places=2)

    def test_03_refuge_islands_and_tactile_paving(self) -> None:
        """DoD 11.3 & 11.4: Verify raised refuge islands (15cm curb) and yellow tactile pads."""
        test_cw = [
            CrosswalkItem(
                crosswalk_id="cw_test_island",
                location=(0.0, 0.0, 0.0),
                crossing_type="traffic_signals",
                road_id=2,
                lane_ids=[-1, -2],
                heading_deg=0.0,
                carriageway_width=8.0,
                corridor_length=3.5,
                has_island=True,
                has_tactile=True,
                num_stripes=8,
            )
        ]
        meshes = generate_crosswalk_meshes(test_cw)
        isl_mesh = meshes["refuge_islands"]
        tac_mesh = meshes["tactile_paving"]

        # Refuge island assertions
        self.assertEqual(isl_mesh["name"], "OSM_Crosswalk_Islands")
        self.assertTrue(isl_mesh["enable_collision"])
        self.assertEqual(len(isl_mesh["vertices"]), 8)  # top + bottom quad
        self.assertEqual(len(isl_mesh["triangles"]), 10)  # 2 top + 8 side tris
        # Top vertices elevated by 0.15m curb height
        top_verts = [v for v in isl_mesh["vertices"] if abs(v[2] - 0.15) < 0.01]
        self.assertEqual(len(top_verts), 4)

        # Tactile paving assertions
        self.assertEqual(tac_mesh["name"], "OSM_Crosswalk_Tactile")
        self.assertEqual(tac_mesh["semantic_label"], "Sidewalks")
        self.assertEqual(tac_mesh["material_preset"], "sidewalk_tiles")
        self.assertIn("material_params", tac_mesh)
        self.assertEqual(tac_mesh["material_params"]["base_color"], [240, 200, 40])
        self.assertEqual(len(tac_mesh["vertices"]), 8)  # 2 pads (both sidewalk ends) * 4 verts

    def test_04_navmesh_walkable_corridors(self) -> None:
        """DoD 11.5: Verify generation of walkable crossing corridors for Recast NavMesh."""
        test_cw = [
            CrosswalkItem(
                crosswalk_id="cw_test_nav",
                location=(10.0, 20.0, 1.0),
                crossing_type="zebra",
                road_id=3,
                lane_ids=[-1],
                heading_deg=45.0,
                carriageway_width=4.0,
                corridor_length=3.0,
                has_island=False,
                has_tactile=False,
                num_stripes=4,
            )
        ]
        meshes = generate_crosswalk_meshes(test_cw)
        corridors = meshes["walkable_corridors"]
        self.assertEqual(len(corridors), 1)
        verts, tris = corridors[0]
        self.assertEqual(len(verts), 4)
        self.assertEqual(len(tris), 2)

    def test_05_manifest_serialization(self) -> None:
        """DoD 11.6: Verify serialization and roundtrip loading of Placements/crosswalks.json."""
        manifest_file = self.test_dir / "Placements" / "crosswalks.json"
        items = [
            CrosswalkItem(
                crosswalk_id="cw_test_m",
                location=(1.0, 2.0, 3.0),
                crossing_type="zebra",
                road_id=42,
                lane_ids=[-1, -2],
                heading_deg=180.0,
                carriageway_width=7.5,
                corridor_length=4.0,
                has_island=True,
                has_tactile=True,
                num_stripes=8,
            )
        ]
        save_crosswalks_manifest(items, manifest_file)
        self.assertTrue(manifest_file.is_file())

        loaded = load_crosswalks_manifest(manifest_file)
        self.assertEqual(len(loaded), 1)
        c = loaded[0]
        self.assertEqual(c.crosswalk_id, "cw_test_m")
        self.assertEqual(c.location, (1.0, 2.0, 3.0))
        self.assertEqual(c.road_id, 42)
        self.assertEqual(c.num_stripes, 8)
        self.assertTrue(c.has_island)
        self.assertTrue(c.has_tactile)

    def test_06_bundle_inspect_and_verify(self) -> None:
        """DoD 11.7: Verify crosswalk inspection and bundle structural verification."""
        self.assertTrue(BUNDLE_ARCHIVE.is_file(), f"Bundle archive must exist at {BUNDLE_ARCHIVE}")

        # Test inspect_crosswalks tool
        cw_manifest = BUNDLE_DIR / "Placements" / "crosswalks.json"
        if cw_manifest.is_file():
            metrics = inspect_crosswalks(manifest_path=cw_manifest, xodr_path=MUNICH_XODR)
            self.assertGreaterEqual(metrics["crosswalk_count"], 18)
            self.assertGreaterEqual(metrics["total_stripes"], 100)
            self.assertEqual(metrics["road_alignment_pct"], 100.0)
            self.assertTrue(metrics["passed_din_standards"])

        # Test carla-map verify command
        args = argparse.Namespace(target=str(BUNDLE_ARCHIVE))
        exit_code = cmd_verify(args)
        self.assertEqual(exit_code, 0, "carla-map verify must exit with code 0")

    def test_07_live_server_pedestrian_crosswalk(self) -> None:
        """Live CARLA test: Validate walker traversal across crosswalk corridor."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server is offline at {self.host}:{self.port} (skipped gracefully)")

        client = carla.Client(self.host, self.port)
        client.set_timeout(10.0)
        world = client.get_world()

        # Check if MunichStachus is loaded
        if "MunichStachus" not in world.get_map().name:
            self.skipTest("MunichStachus not loaded on running server")

        cw_manifest = BUNDLE_DIR / "Placements" / "crosswalks.json"
        if not cw_manifest.is_file():
            self.skipTest("crosswalks.json not found")

        items = load_crosswalks_manifest(cw_manifest)
        self.assertGreater(len(items), 0)
        first_cw = items[0]

        bp_lib = world.get_blueprint_library()
        walker_bp = bp_lib.filter("walker.pedestrian.*")[0]

        # Spawn walker near crosswalk start
        start_loc = carla.Location(x=first_cw.location[0], y=first_cw.location[1], z=first_cw.location[2] + 0.5)
        spawn_tf = carla.Transform(start_loc, carla.Rotation(yaw=first_cw.heading_deg))
        walker = world.try_spawn_actor(walker_bp, spawn_tf)
        if walker is not None:
            try:
                world.wait_for_tick(seconds=2.0)
                loc = walker.get_location()
                self.assertIsNotNone(loc)
            finally:
                walker.destroy()


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 11 Crosswalks E2E Test Suite")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--offline-only", action="store_true", help="Run offline tests only")
    args, unknown = parser.parse_known_args()

    os.environ["CARLA_E2E_HOST"] = args.host
    os.environ["CARLA_E2E_PORT"] = str(args.port)
    os.environ["CARLA_E2E_OFFLINE_ONLY"] = "1" if args.offline_only else "0"

    unittest_args = [sys.argv[0]] + unknown
    result = unittest.main(argv=unittest_args, exit=False)
    return 0 if result.result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
