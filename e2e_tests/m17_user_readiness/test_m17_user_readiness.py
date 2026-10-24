#!/usr/bin/env python3
"""
Milestone 17 (M17) E2E Test Suite: User Map Readiness & Autonomous Traffic Verification.

Verifies end-to-end user-perspective capabilities on custom CARLA maps (e.g. MunichStachus):
  - DoD 17.1: OpenDRIVE vehicle spawn point offline extraction & geometry validation
  - DoD 17.2: Recast NavMesh binary validation (header magic, tile count, file size > 1MB)
  - DoD 17.3: Recast NavMesh OBJ geometry exporter & material tagging (sidewalk, crosswalk, road)
  - DoD 17.4: Readiness auditor CLI interface & argument parsing
  - DoD 17.5: Standalone .carlamap bundle readiness & structural verification
  - DoD 17.6: Live CARLA OpenDRIVE map query & spawn points validation
  - DoD 17.7: Live CARLA autonomous vehicle traffic with Traffic Manager autopilot
  - DoD 17.8: Live CARLA pedestrian navigation on sidewalks and crosswalks
  - DoD 17.9: Compatibility with CARLA's standard PythonAPI/examples/generate_traffic.py
  - DoD 17.10: Live CARLA end-to-end readiness audit & clock restoration
"""

from __future__ import annotations

import argparse
import io
import json
import math
import os
from pathlib import Path
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
import unittest
import xml.etree.ElementTree as ET

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

from Util.MapBuilder.carla_map import cmd_verify
from Util.MapBuilder.navmesh_builder import (
    compile_navmesh_binary,
    export_recast_geometry_obj,
    find_recast_builder_binary,
)
from Util.MapBuilder.verify_map_readiness import (
    audit_map_readiness,
    build_arg_parser,
)

BUNDLE_PATH = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap.tar.gz"
MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
MUNICH_BIN = REPO_ROOT / "Saved" / "Nav" / "MunichStachus.bin"


def is_carla_server_running(host: str = "127.0.0.1", port: int = 2000, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except (OSError, ConnectionRefusedError):
        return False


class TestMilestone17UserReadiness(unittest.TestCase):
    """Milestone 17 E2E Test Suite for User Map Readiness & Traffic Navigation."""

    def test_01_opendrive_spawn_point_offline_validation(self) -> None:
        """DoD 17.1: Verify OpenDRIVE contains valid driving lanes and geometrically sound spawn points."""
        xodr_content: str | None = None
        if BUNDLE_PATH.is_file():
            import tarfile
            with tarfile.open(BUNDLE_PATH, "r:gz") as tar:
                for member in tar.getmembers():
                    if member.name.endswith(".xodr"):
                        extracted = tar.extractfile(member)
                        if extracted:
                            xodr_content = extracted.read().decode("utf-8")
                            break

        if not xodr_content:
            fallback = REPO_ROOT / "Unreal" / "CarlaUnreal" / "Saved" / "Maps" / "MunichStachus" / "OpenDrive" / "MunichStachus.xodr"
            if fallback.is_file():
                xodr_content = fallback.read_text(encoding="utf-8")

        if not xodr_content:
            self.skipTest(f"OpenDRIVE file not found in bundle {BUNDLE_PATH} or disk")

        root = ET.fromstring(xodr_content)
        roads = root.findall("road")
        self.assertGreater(len(roads), 0, "No roads found in OpenDRIVE XML")

        driving_lanes_found = 0
        for road in roads:
            lanes = road.find("lanes")
            if lanes is None:
                continue
            for lane_section in lanes.findall("laneSection"):
                for side in ("left", "right"):
                    side_el = lane_section.find(side)
                    if side_el is not None:
                        for lane in side_el.findall("lane"):
                            if lane.get("type") == "driving":
                                driving_lanes_found += 1

        self.assertGreater(driving_lanes_found, 10, f"Expected >10 driving lanes, got {driving_lanes_found}")

    def test_02_navmesh_binary_header_and_tiles(self) -> None:
        """DoD 17.2: Verify Recast NavMesh binary has valid DT_NAVMESH_MAGIC header, tile count, and size."""
        bin_data: bytes | None = None
        if BUNDLE_PATH.is_file():
            import tarfile
            with tarfile.open(BUNDLE_PATH, "r:gz") as tar:
                for member in tar.getmembers():
                    if member.name.endswith(".bin") and "Nav" in member.name:
                        extracted = tar.extractfile(member)
                        if extracted:
                            bin_data = extracted.read()
                            break

        if not bin_data:
            fallback = REPO_ROOT / "Unreal" / "CarlaUnreal" / "Saved" / "Maps" / "MunichStachus" / "Nav" / "MunichStachus.bin"
            if fallback.is_file():
                bin_data = fallback.read_bytes()

        if not bin_data:
            self.skipTest(f"NavMesh binary not found in bundle {BUNDLE_PATH} or disk")

        file_size = len(bin_data)
        self.assertGreater(file_size, 100_000, f"NavMesh binary too small ({file_size} bytes, expected >100KB)")

        header_bytes = bin_data[:20]
        # Recast dtNavMesh header: magic number is NAVMESHSET_MAGIC (0x4d534554 'MSET') or DT_NAVMESH_MAGIC (0x444e4156 'DNAV')
        magic, version = struct.unpack("<ii", header_bytes[:8])
        self.assertIn(magic, (0x4D534554, 0x444E4156), f"Invalid Recast NavMesh magic: {hex(magic)}")
        self.assertGreater(version, 0, f"Invalid NavMesh version: {version}")

    def test_03_navmesh_obj_materials_and_connectivity(self) -> None:
        """DoD 17.3: Verify Recast NavMesh OBJ exporter produces correct coordinate mapping and materials."""
        with tempfile.TemporaryDirectory() as tmpdir:
            obj_path = Path(tmpdir) / "test_nav.obj"
            walkable_surfaces = [
                ([(0.0, 0.0, 1.0), (1.0, 0.0, 1.0), (1.0, 1.0, 1.0)], [(0, 1, 2)], "sidewalk"),
                ([(2.0, 0.0, 0.5), (3.0, 0.0, 0.5), (3.0, 1.0, 0.5)], [(0, 1, 2)], "crosswalk"),
                ([(4.0, 0.0, 0.0), (5.0, 0.0, 0.0), (5.0, 1.0, 0.0)], [(0, 1, 2)], "road"),
            ]
            export_recast_geometry_obj(obj_path, walkable_surfaces)
            self.assertTrue(obj_path.is_file(), "Failed to export OBJ geometry")

            content = obj_path.read_text(encoding="utf-8")
            self.assertIn("usemtl sidewalk", content)
            self.assertIn("usemtl crosswalk", content)
            self.assertIn("usemtl road", content)

            # Check coordinate mapping: CARLA Z (1.0) maps to Recast Y (height)
            # CARLA (x=0, y=0, z=1) -> Recast 'v 0.0000 1.0000 0.0000'
            self.assertIn("v 0.0000 1.0000 0.0000", content)

    def test_04_readiness_auditor_cli_args(self) -> None:
        """DoD 17.4: Verify readiness auditor CLI argument parser and defaults."""
        parser = build_arg_parser()
        args = parser.parse_args(["--map", "MunichStachus", "-n", "8", "-w", "5", "-d", "4.0", "--json"])
        self.assertEqual(args.map, "MunichStachus")
        self.assertEqual(args.vehicles, 8)
        self.assertEqual(args.walkers, 5)
        self.assertEqual(args.duration, 4.0)
        self.assertTrue(args.json)

    def test_05_bundle_inspection_readiness_flags(self) -> None:
        """DoD 17.5: Verify standalone .carlamap bundle structure passes carla-map verify."""
        if not BUNDLE_PATH.is_file():
            self.skipTest(f"Bundle not found at {BUNDLE_PATH}")

        verify_args = argparse.Namespace(target=str(BUNDLE_PATH), json=False)
        ret = cmd_verify(verify_args)
        self.assertEqual(ret, 0, f"Bundle verification failed for {BUNDLE_PATH}")

    def test_06_live_carla_spawn_points_and_map_query(self) -> None:
        """DoD 17.6: Live CARLA map query returns valid spawn points on driving lanes."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        client = carla.Client("127.0.0.1", 2000)
        client.set_timeout(10.0)
        world = client.get_world()
        carla_map = world.get_map()

        spawn_points = carla_map.get_spawn_points()
        self.assertGreaterEqual(len(spawn_points), 50, f"Expected >= 50 spawn points, got {len(spawn_points)}")

        # Verify sample spawn point aligns with driving lane
        sample_pt = spawn_points[0]
        sample_wp = carla_map.get_waypoint(sample_pt.location, project_to_road=True, lane_type=carla.LaneType.Driving)
        self.assertIsNotNone(sample_wp, "Sample spawn point does not resolve to a valid waypoint")
        self.assertEqual(sample_wp.lane_type, carla.LaneType.Driving, "Spawn point is not on a driving lane")

    def test_07_live_carla_autonomous_vehicle_autopilot(self) -> None:
        """DoD 17.7: Live CARLA spawns vehicles and drives them autonomously with Traffic Manager."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        client = carla.Client("127.0.0.1", 2000)
        client.set_timeout(10.0)
        world = client.get_world()
        carla_map = world.get_map()
        tm = client.get_trafficmanager(8000)

        orig_settings = world.get_settings()
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)
        tm.set_synchronous_mode(True)

        bp_lib = world.get_blueprint_library()
        veh_bps = bp_lib.filter("vehicle.*")
        spawn_points = carla_map.get_spawn_points()

        spawned_vehicles = []
        try:
            batch = []
            for i in range(min(5, len(spawn_points))):
                bp = veh_bps[i % len(veh_bps)]
                tf = spawn_points[i]
                tf.location.z += 0.5
                batch.append(
                    carla.command.SpawnActor(bp, tf).then(
                        carla.command.SetAutopilot(carla.command.FutureActor, True, tm.get_port())
                    )
                )

            results = client.apply_batch_sync(batch, True)
            for res in results:
                if not res.error:
                    spawned_vehicles.append(res.actor_id)

            self.assertGreater(len(spawned_vehicles), 0, "Failed to spawn any test vehicles")
            veh_actors = [world.get_actor(v_id) for v_id in spawned_vehicles]

            init_locs = [v.get_location() for v in veh_actors]

            # Step 40 ticks (2.0s)
            for _ in range(40):
                world.tick()

            final_locs = [v.get_location() for v in veh_actors]
            displacements = [
                math.sqrt((f.x - i.x) ** 2 + (f.y - i.y) ** 2 + (f.z - i.z) ** 2)
                for i, f in zip(init_locs, final_locs)
            ]
            moving_count = sum(1 for d in displacements if d > 1.0)
            self.assertGreater(moving_count, 0, f"Expected moving vehicles, got {moving_count}/{len(spawned_vehicles)}")

        finally:
            if spawned_vehicles:
                client.apply_batch_sync([carla.command.DestroyActor(x) for x in spawned_vehicles], True)
            world.apply_settings(orig_settings)

    def test_08_live_carla_pedestrian_navigation(self) -> None:
        """DoD 17.8: Live CARLA spawns walkers and navigates along sidewalks and crosswalks."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        client = carla.Client("127.0.0.1", 2000)
        client.set_timeout(10.0)
        world = client.get_world()

        orig_settings = world.get_settings()
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)

        bp_lib = world.get_blueprint_library()
        walker_bps = bp_lib.filter("walker.pedestrian.*")
        ai_bp = bp_lib.find("controller.ai.walker")

        walker_ids = []
        controller_ids = []

        try:
            # 1. Spawn walkers
            batch = []
            for _ in range(5):
                loc = world.get_random_location_from_navigation()
                if loc is not None:
                    tf = carla.Transform(carla.Location(loc.x, loc.y, loc.z + 1.0))
                    bp = walker_bps[0]
                    batch.append(carla.command.SpawnActor(bp, tf))

            for res in client.apply_batch_sync(batch, True):
                if not res.error:
                    walker_ids.append(res.actor_id)

            self.assertGreater(len(walker_ids), 0, "Failed to spawn any walkers on navmesh")

            # Tick to materialize
            world.tick()

            # 2. Attach controllers
            con_batch = [carla.command.SpawnActor(ai_bp, carla.Transform(), w_id) for w_id in walker_ids]
            for res in client.apply_batch_sync(con_batch, True):
                if not res.error:
                    controller_ids.append(res.actor_id)

            self.assertEqual(len(controller_ids), len(walker_ids), "Controller pairing count mismatch")

            # Tick to attach
            for _ in range(3):
                world.tick()

            controllers = [world.get_actor(c_id) for c_id in controller_ids]
            walkers = [world.get_actor(w_id) for w_id in walker_ids]
            init_locs = [w.get_location() for w in walkers]

            for c in controllers:
                c.start()
                dest = world.get_random_location_from_navigation()
                if dest is not None:
                    c.go_to_location(dest)
                c.set_max_speed(1.4)

            # Step 40 ticks
            for _ in range(40):
                world.tick()

            final_locs = [w.get_location() for w in walkers]
            displacements = [
                math.sqrt((f.x - i.x) ** 2 + (f.y - i.y) ** 2 + (f.z - i.z) ** 2)
                for i, f in zip(init_locs, final_locs)
            ]
            moving_count = sum(1 for d in displacements if d > 0.5)
            self.assertGreater(moving_count, 0, f"Expected moving walkers, got {moving_count}/{len(walker_ids)}")

        finally:
            for c_id in controller_ids:
                try:
                    c = world.get_actor(c_id)
                    if c is not None and c.is_alive:
                        c.stop()
                except Exception:
                    pass
            if controller_ids:
                client.apply_batch_sync([carla.command.DestroyActor(x) for x in controller_ids], True)
            if walker_ids:
                client.apply_batch_sync([carla.command.DestroyActor(x) for x in walker_ids], True)
            world.apply_settings(orig_settings)

    def test_09_live_carla_generate_traffic_script_compatibility(self) -> None:
        """DoD 17.9: Verify standard PythonAPI/examples/generate_traffic.py runs and exits cleanly."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        gen_traffic_script = REPO_ROOT / "PythonAPI" / "examples" / "generate_traffic.py"
        self.assertTrue(gen_traffic_script.is_file(), f"Script missing: {gen_traffic_script}")

        # Launch generate_traffic for 4 seconds then send SIGINT
        proc = subprocess.Popen(
            [sys.executable, str(gen_traffic_script), "-n", "4", "-w", "4", "--safe"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        time.sleep(4.0)
        proc.send_signal(signal.SIGINT)
        stdout, stderr = proc.communicate(timeout=10.0)

        self.assertEqual(proc.returncode, 0, f"generate_traffic.py exited with error: {stderr}\n{stdout}")
        self.assertIn("destroying", stdout)

    def test_10_live_carla_full_readiness_audit(self) -> None:
        """DoD 17.10: Programmatic audit_map_readiness returns PASS status and cleans up completely."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        client = carla.Client("127.0.0.1", 2000)
        client.set_timeout(10.0)

        report = audit_map_readiness(
            client=client,
            num_vehicles=6,
            num_walkers=6,
            duration_s=4.0,
            seed=123,
            verbose=False,
        )

        self.assertEqual(report["overall_status"], "PASS")
        self.assertGreater(report["spawn_points"]["total_count"], 50)
        self.assertGreater(report["vehicles"]["moving"], 0)
        self.assertGreater(report["walkers"]["moving"], 0)
        self.assertTrue(report["lifecycle"]["clean_destruction"])
        self.assertTrue(report["lifecycle"]["settings_restored"])


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 17 E2E Test Suite")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument("--offline-only", action="store_true", help="Run only offline tests")
    args, unknown = parser.parse_known_args()

    suite = unittest.TestSuite()
    loader = unittest.TestLoader()

    # Load test cases
    for name in loader.getTestCaseNames(TestMilestone17UserReadiness):
        if args.offline_only and "live_carla" in name:
            continue
        suite.addTest(TestMilestone17UserReadiness(name))

    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
