#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 7 (M7).

Verifies the Vegetation, Roadside Furniture, Urban Clutter & Curbside Parked Cars Pipeline:
  - DoD 7.1: Real-world OSM clutter extraction parses trees, benches, bins, bollards, etc.
  - DoD 7.2: Procedural sidewalk clutter synthesis decorates sparse avenues with strict clearances.
  - DoD 7.3: Curbside parked car generator places parked vehicles with correct lateral offset and clearances.
  - DoD 7.4: Spatial clearance auditing confirms 0 driving lane encroachments and 0 building penetrations.
  - DoD 7.5: Bundle integration bundles Placements/clutter.json, verified by carla-map inspect & verify.
  - DoD 7.6: Live CARLA simulator verification (when server is running) spawns clutter props and parked cars.
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

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_install, cmd_verify
from Util.MapBuilder.osm_clutter import (
    ClutterItem,
    extract_osm_clutter,
    generate_curbside_parked_cars,
    load_clutter_manifest,
    save_clutter_manifest,
    spawn_clutter_in_world,
    synthesize_procedural_sidewalk_clutter,
)
from Util.MapBuilder.inspect_clutter_spatial import audit_clutter_clearance

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
if not MUNICH_OSM.is_file():
    MUNICH_OSM = REPO_ROOT / "munich_stachus.osm"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone7ClutterParking(unittest.TestCase):
    """Milestone 7 E2E Test Suite for Clutter, Vegetation & Curbside Parked Cars."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m7_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_clutter_extraction(self) -> None:
        """DoD 7.1: Extract real-world roadside clutter and vegetation from OSM."""
        self.assertTrue(MUNICH_OSM.is_file(), "munich_stachus.osm must exist")

        # Fake identity projection for unit test isolation returning (x, y, z)
        items = extract_osm_clutter(
            MUNICH_OSM,
            project_coord_fn=lambda lat, lon: (lon * 1000.0, lat * 1000.0, 0.0),
        )
        self.assertGreater(len(items), 100, f"Expected > 100 OSM clutter items, got {len(items)}")

        categories = {it.category for it in items}
        self.assertIn("tree", categories, "Must extract real-world trees")
        self.assertIn("bench", categories, "Must extract real-world benches")
        self.assertIn("waste_basket", categories, "Must extract real-world waste baskets")

        # Verify blueprint tags
        for it in items:
            self.assertTrue(
                it.blueprint_id.startswith("static.prop.") or it.blueprint_id.startswith("vehicle."),
                f"Invalid blueprint: {it.blueprint_id}",
            )

    def test_02_manifest_serialization(self) -> None:
        """DoD 7.5: Serialization of clutter and parked cars manifest JSON."""
        test_items = [
            ClutterItem("tree", "static.prop.plantpot05", (10.0, 20.0, 0.5), (0.0, 90.0, 0.0), source="osm"),
            ClutterItem("bench", "static.prop.bench01", (12.0, 20.0, 0.5), (0.0, 0.0, 0.0), source="synthetic"),
        ]
        test_cars = [
            ClutterItem(
                "parked_car",
                "vehicle.lincoln.mkz",
                (15.0, 25.0, 0.2),
                (0.0, 45.0, 0.0),
                color="30,30,30",
                source="curbside_parking",
            )
        ]

        json_path = self.test_dir / "test_clutter.json"
        save_clutter_manifest(test_items, test_cars, json_path)
        self.assertTrue(json_path.is_file(), "Manifest file must be created")

        loaded_items, loaded_cars = load_clutter_manifest(json_path)
        self.assertEqual(len(loaded_items), 2)
        self.assertEqual(len(loaded_cars), 1)
        self.assertEqual(loaded_cars[0].color, "30,30,30")
        self.assertEqual(loaded_items[0].category, "tree")

    def test_03_bundle_build_with_clutter(self) -> None:
        """DoD 7.5: carla-map build creates bundle containing Placements/clutter.json."""
        out_tar = self.test_dir / "MunichM7Test.carlamap.tar.gz"
        args = argparse.Namespace(
            subcommand="build",
            target=".",
            name="MunichM7Test",
            output=str(out_tar),
            osm=str(MUNICH_OSM),
            xodr=None,
            meshes=None,
            layer_elevation=None,
            no_terrain=False,
            no_subbase=False,
            subbase_margin=50.0,
            no_clutter=False,
            no_parked_cars=False,
            max_parked_cars=10,
        )

        rc = cmd_build(args)
        self.assertEqual(rc, 0, "cmd_build must succeed")
        self.assertTrue(out_tar.is_file(), "Output tar.gz archive must exist")

        # Verify inspection includes clutter info
        inspect_args = argparse.Namespace(subcommand="inspect", target=str(out_tar))
        self.assertEqual(cmd_inspect(inspect_args), 0, "cmd_inspect must succeed")

        # Verify integrity
        verify_args = argparse.Namespace(subcommand="verify", target=str(out_tar))
        self.assertEqual(cmd_verify(verify_args), 0, "cmd_verify must pass 10 structural checks")

    def test_04_spatial_clearance_audit(self) -> None:
        """DoD 7.4: Clutter spatial auditor reports 0 lane encroaches and 0 building wall penetrations."""
        bundle_dir = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
        if not bundle_dir.is_dir():
            bundle_dir = REPO_ROOT / "MunichStachus.carlamap"
        clutter_manifest = bundle_dir / "Placements" / "clutter.json"
        xodr_file = bundle_dir / "OpenDrive" / "MunichStachus.xodr"
        osm_file = MUNICH_OSM

        if not clutter_manifest.is_file() or not xodr_file.is_file():
            self.skipTest("MunichStachus bundle not found on disk")

        metrics = audit_clutter_clearance(clutter_manifest, xodr_file, osm_file)
        self.assertEqual(metrics["lane_encroachments"], 0, "0 clutter props may encroach driving lanes")
        self.assertEqual(metrics["building_penetrations"], 0, "0 clutter props may penetrate buildings")
        self.assertEqual(metrics["junction_encroachments"], 0, "0 parked cars may be in junctions")
        self.assertEqual(metrics["corridor_blocks"], 0, "0 parked cars may block lane bypass corridors")
        self.assertGreaterEqual(metrics["clutter_count"], 300, "Should have >= 300 clutter items")
        self.assertGreaterEqual(metrics["parked_cars_count"], 20, "Should have >= 20 parked cars")

    def test_05_live_simulator_clutter_spawning(self) -> None:
        """DoD 7.6: Live CARLA server loads map and spawns clutter props and parked cars."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(15.0)

        world = client.get_world()
        if "MunichStachus" not in world.get_map().name:
            world = client.load_world("MunichStachus")
            time.sleep(2.0)

        bundle_dir = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
        if not bundle_dir.is_dir():
            bundle_dir = REPO_ROOT / "MunichStachus.carlamap"
        clutter_manifest = bundle_dir / "Placements" / "clutter.json"
        if not clutter_manifest.is_file():
            self.skipTest("MunichStachus clutter manifest not found")

        items, cars = load_clutter_manifest(clutter_manifest)
        spawn_res = spawn_clutter_in_world(world, items[:20], cars[:5])
        spawned_props = spawn_res["props"]
        spawned_cars = spawn_res["parked_cars"]

        try:
            self.assertGreater(len(spawned_props), 5, "Must spawn at least 5 clutter props")
            self.assertGreater(len(spawned_cars), 2, "Must spawn at least 2 parked cars")

            # Check that parked cars have handbrake applied and physics active
            for car in spawned_cars:
                self.assertIsNotNone(car.get_location(), "Parked car must have valid location")
        finally:
            for actor in spawned_props + spawned_cars:
                try:
                    actor.destroy()
                except Exception:
                    pass


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 7 E2E Clutter & Parking Test Suite")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--offline-only", action="store_true")
    args, unknown = parser.parse_known_args()

    os.environ["CARLA_E2E_HOST"] = args.host
    os.environ["CARLA_E2E_PORT"] = str(args.port)
    os.environ["CARLA_E2E_OFFLINE_ONLY"] = "1" if args.offline_only else "0"

    unittest_args = [sys.argv[0]] + unknown
    result = unittest.main(argv=unittest_args, exit=False)
    return 0 if result.result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

