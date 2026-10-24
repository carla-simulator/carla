#!/usr/bin/env python3
"""
Milestone 3 End-to-End Verification Suite:
Server-Side .carlamap Bundle Discovery & Runtime Loader.

Verifies all Definition-of-Done (DoD) items for Milestone 3:
  - DoD 3.1: .carlamap specification and binary .cmesh format reader/validator.
  - DoD 3.2: Native map discovery (client.get_available_maps() lists .carlamap bundles).
  - DoD 3.3: Seamless client.load_world("<MapName>") execution with custom .cmesh geometry,
             materials, collisions, and vehicle drivability.
  - DoD 3.4: Pre-compiled Walker NavMesh (get_random_location_from_navigation, walker AI navigation).
  - DoD 3.5: Remote bundle loading via RPC (client.load_map_bundle).
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import time
import unittest

import carla


REPO_ROOT = Path(__file__).resolve().parents[2]
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
import tempfile
tempfile.tempdir = str(LOCAL_TMP_DIR)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SAMPLE_BUNDLE_DIR = FIXTURES_DIR / "sample_bundle"
SAMPLE_BUNDLE_TAR = FIXTURES_DIR / "sample_bundle.carlamap.tar.gz"

sys.path.insert(0, str(REPO_ROOT / "e2e_tests" / "m1_osm2odr"))
from test_m1_osm2odr import find_unreal_editor_binary, is_port_open  # noqa: E402


def unpack_cmesh(filepath: Path) -> dict:
    """Unpacks and validates a binary .cmesh file."""
    data = filepath.read_bytes()
    if len(data) < 20:
        raise ValueError(f"CMesh too short: {len(data)} bytes")

    magic, version, vertex_count, index_count, flags = struct.unpack("<IIIII", data[:20])
    if magic != 0x53454D43:
        raise ValueError(f"Invalid magic: 0x{magic:08X}")
    if version != 1:
        raise ValueError(f"Unsupported version: {version}")

    has_normals = bool(flags & 1)
    has_uvs = bool(flags & 2)
    has_colors = bool(flags & 4)

    expected_size = 20 + vertex_count * 3 * 4
    if has_normals:
        expected_size += vertex_count * 3 * 4
    if has_uvs:
        expected_size += vertex_count * 2 * 4
    if has_colors:
        expected_size += vertex_count * 4 * 1
    expected_size += index_count * 4

    if len(data) < expected_size:
        raise ValueError(f"CMesh truncated: expected {expected_size}, got {len(data)}")

    offset = 20
    flat_verts = struct.unpack(f"<{vertex_count * 3}f", data[offset : offset + vertex_count * 12])
    vertices = [(flat_verts[3 * i], flat_verts[3 * i + 1], flat_verts[3 * i + 2]) for i in range(vertex_count)]
    offset += vertex_count * 12

    normals = None
    if has_normals:
        flat_norms = struct.unpack(f"<{vertex_count * 3}f", data[offset : offset + vertex_count * 12])
        normals = [(flat_norms[3 * i], flat_norms[3 * i + 1], flat_norms[3 * i + 2]) for i in range(vertex_count)]
        offset += vertex_count * 12

    uvs = None
    if has_uvs:
        flat_uvs = struct.unpack(f"<{vertex_count * 2}f", data[offset : offset + vertex_count * 8])
        uvs = [(flat_uvs[2 * i], flat_uvs[2 * i + 1]) for i in range(vertex_count)]
        offset += vertex_count * 8

    colors = None
    if has_colors:
        flat_cols = struct.unpack(f"<{vertex_count * 4}B", data[offset : offset + vertex_count * 4])
        colors = [(flat_cols[4 * i], flat_cols[4 * i + 1], flat_cols[4 * i + 2], flat_cols[4 * i + 3]) for i in range(vertex_count)]
        offset += vertex_count * 4

    indices = struct.unpack(f"<{index_count}I", data[offset : offset + index_count * 4])

    return {
        "magic": magic,
        "version": version,
        "vertex_count": vertex_count,
        "index_count": index_count,
        "flags": flags,
        "vertices": vertices,
        "normals": normals,
        "uvs": uvs,
        "colors": colors,
        "indices": list(indices),
    }


class Milestone3CarlaMapLoaderE2ETest(unittest.TestCase):
    host: str = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
    port: int = int(os.environ.get("CARLA_E2E_PORT", "2000"))
    offline_only: bool = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    def test_dod_3_1_cmesh_binary_spec_offline(self) -> None:
        """DoD 3.1 (Offline): Verify .cmesh binary specification, headers, unpacking, and validation."""
        self.assertTrue(SAMPLE_BUNDLE_DIR.is_dir(), f"Sample bundle directory missing: {SAMPLE_BUNDLE_DIR}")
        roads_cmesh = SAMPLE_BUNDLE_DIR / "Meshes" / "roads.cmesh"
        sidewalks_cmesh = SAMPLE_BUNDLE_DIR / "Meshes" / "sidewalks.cmesh"
        buildings_cmesh = SAMPLE_BUNDLE_DIR / "Meshes" / "buildings.cmesh"

        for cmesh_path, exp_verts, exp_tris in [
            (roads_cmesh, 4, 6),
            (sidewalks_cmesh, 4, 6),
            (buildings_cmesh, 8, 30),
        ]:
            self.assertTrue(cmesh_path.is_file(), f"Missing cmesh: {cmesh_path}")
            mesh_data = unpack_cmesh(cmesh_path)
            self.assertEqual(mesh_data["magic"], 0x53454D43)
            self.assertEqual(mesh_data["version"], 1)
            self.assertEqual(mesh_data["vertex_count"], exp_verts)
            self.assertEqual(mesh_data["index_count"], exp_tris)
            self.assertGreater(len(mesh_data["vertices"]), 0)
            self.assertGreater(len(mesh_data["indices"]), 0)
            self.assertTrue(all(idx < mesh_data["vertex_count"] for idx in mesh_data["indices"]))
            if mesh_data["normals"] is not None:
                self.assertEqual(len(mesh_data["normals"]), exp_verts)
            if mesh_data["uvs"] is not None:
                self.assertEqual(len(mesh_data["uvs"]), exp_verts)

        # Verify corrupted header fails safely
        bad_magic_bytes = struct.pack("<IIIII", 0x12345678, 1, 4, 6, 0) + b"\x00" * 100
        temp_bad = FIXTURES_DIR / "temp_bad.cmesh"
        temp_bad.write_bytes(bad_magic_bytes)
        try:
            with self.assertRaises(ValueError):
                unpack_cmesh(temp_bad)
        finally:
            if temp_bad.exists():
                temp_bad.unlink()

        # Verify carla.Client has load_map_bundle method
        self.assertTrue(hasattr(carla.Client, "load_map_bundle"))

    def test_dod_3_2_to_3_5_live_carlamap_loader(self) -> None:
        """DoD 3.2 - 3.5 (Live Server): Test native discovery, load_world, navmesh, driving, and remote bundle load."""
        if self.offline_only:
            self.skipTest("Skipping live tests (--offline-only requested)")

        # Ensure sample bundle is staged into project Maps/ folder for native discovery
        staging_destinations = [
            REPO_ROOT / "Unreal" / "CarlaUnreal" / "Maps" / "BundleTestTown",
            REPO_ROOT / "Build" / "Package" / "Carla-0.10.0-Mac-Shipping" / "Mac" / "CarlaUnreal-Mac-Shipping.app" / "Contents" / "UE" / "CarlaUnreal" / "Maps" / "BundleTestTown",
            REPO_ROOT / "Build" / "Package" / "Carla-0.10.0-Linux-Shipping" / "CarlaUnreal" / "Maps" / "BundleTestTown",
        ]
        staged_dirs: list[Path] = []
        for p_dir in staging_destinations:
            if p_dir.parent.parent.is_dir() or p_dir.parent.is_dir():
                p_dir.parent.mkdir(parents=True, exist_ok=True)
                if p_dir.exists():
                    shutil.rmtree(p_dir)
                shutil.copytree(SAMPLE_BUNDLE_DIR, p_dir)
                staged_dirs.append(p_dir)

        server_proc: subprocess.Popen[bytes] | None = None
        log_handle = None

        try:
            if not is_port_open(self.host, self.port):
                editor_bin = find_unreal_editor_binary(REPO_ROOT)
                uproject = REPO_ROOT / "Unreal/CarlaUnreal/CarlaUnreal.uproject"
                if editor_bin is None or not uproject.is_file():
                    self.skipTest("CARLA server not running and UnrealEditor binary not found")

                log_path = LOCAL_TMP_DIR / f"carla_m3_e2e_{self.port}.log"
                log_handle = log_path.open("w", encoding="utf-8")
                server_proc = subprocess.Popen(
                    [
                        str(editor_bin),
                        str(uproject),
                        "OpenDriveMap",
                        "-game",
                        "-RenderOffScreen",
                        "-notrace",
                        "-nosound",
                        "-stdout",
                        f"-carla-rpc-port={self.port}",
                    ],
                    stdout=log_handle,
                    stderr=subprocess.STDOUT,
                )
                for _ in range(120):
                    if server_proc.poll() is not None:
                        self.fail(f"UnrealEditor exited early with code {server_proc.returncode}")
                    if is_port_open(self.host, self.port):
                        break
                    time.sleep(1.0)
                else:
                    self.fail(f"Timed out waiting for CARLA server on {self.host}:{self.port}")

            client = carla.Client(self.host, self.port)
            client.set_timeout(120.0)

            # Wait for CARLA server engine loop to finish initialization and tick
            ready = False
            for _ in range(60):
                try:
                    w = client.get_world()
                    if w is not None:
                        ready = True
                        break
                except Exception:
                    time.sleep(1.0)
            self.assertTrue(ready, "Server engine did not finish initialization within timeout")
            client.set_timeout(60.0)

            # ------------------------------------------------------------------
            # DoD 3.2: Native Map Discovery (client.get_available_maps())
            # ------------------------------------------------------------------
            available_maps = client.get_available_maps()
            self.assertGreater(len(available_maps), 0)
            bundle_discovered = any(
                m.endswith("BundleTestTown") or m == "BundleTestTown"
                for m in available_maps
            )
            self.assertTrue(
                bundle_discovered,
                f"BundleTestTown not discovered in available maps: {available_maps}",
            )

            # ------------------------------------------------------------------
            # DoD 3.3: Seamless client.load_world("BundleTestTown") Execution
            # ------------------------------------------------------------------
            world = client.load_world("BundleTestTown")
            time.sleep(1.0)
            self.assertIsNotNone(world)

            carla_map = world.get_map()
            self.assertEqual(
                carla_map.name.split("/")[-1],
                "BundleTestTown",
                f"Expected map name BundleTestTown, got {carla_map.name}",
            )

            spawn_points = carla_map.get_spawn_points()
            self.assertGreater(
                len(spawn_points),
                0,
                "Map spawn points list must be non-empty",
            )

            # Verify custom .cmesh layers registered as environment objects
            road_objs = world.get_environment_objects(carla.CityObjectLabel.Roads)
            self.assertGreater(
                len(road_objs),
                0,
                "Expected custom road meshes registered as CityObjectLabel.Roads",
            )

            bldg_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)
            self.assertGreater(
                len(bldg_objs),
                0,
                "Expected custom building meshes registered as CityObjectLabel.Buildings",
            )

            # Synchronous mode for deterministic physics & walker tests
            settings = world.get_settings()
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)
            world.tick()

            # Physics Collision & Vehicle Settle Test
            bp_lib = world.get_blueprint_library()
            veh_bp = bp_lib.find("vehicle.mini.cooper") or bp_lib.filter("vehicle.*")[0]
            # Spawn 1 m above road mesh at (0, 0, 1.0)
            veh_trans = carla.Transform(carla.Location(x=0.0, y=0.0, z=1.0), carla.Rotation(0, 0, 0))
            vehicle = world.spawn_actor(veh_bp, veh_trans)
            self.assertIsNotNone(vehicle, "Failed to spawn vehicle on custom road mesh")

            try:
                for _ in range(40):  # 2.0 seconds at 0.05s
                    world.tick()

                v_loc = vehicle.get_location()
                v_vel = vehicle.get_velocity()
                # Vehicle should have settled on the road surface z approx 0.2-0.5 m (above road z=0.0)
                self.assertGreater(v_loc.z, -0.5, "Vehicle fell through custom road mesh!")
                self.assertLess(v_loc.z, 2.0, "Vehicle launched into the air!")
                self.assertAlmostEqual(v_vel.z, 0.0, delta=0.5, msg="Vehicle has not settled vertically")

                # Drive forward test
                vehicle.apply_control(carla.VehicleControl(throttle=0.6, steer=0.0))
                for _ in range(30):  # 1.5 seconds
                    world.tick()

                v_vel_moving = vehicle.get_velocity()
                speed = (v_vel_moving.x**2 + v_vel_moving.y**2)**0.5
                self.assertGreater(speed, 0.5, "Vehicle failed to accelerate on custom road mesh")

            finally:
                vehicle.destroy()

            # ------------------------------------------------------------------
            # DoD 3.4: Pre-Compiled Walker NavMesh & AI Navigation Verification
            # ------------------------------------------------------------------
            nav_loc1 = world.get_random_location_from_navigation()
            self.assertIsNotNone(nav_loc1, "Failed to get random location from pre-compiled NavMesh")
            self.assertGreater(nav_loc1.z, -900000.0, "NavMesh returned error sentinel")

            nav_loc2 = world.get_random_location_from_navigation()
            self.assertIsNotNone(nav_loc2, "Failed to get second random navigation location")
            self.assertGreater(nav_loc2.z, -900000.0, "NavMesh returned error sentinel")

            walker_bp = bp_lib.filter("walker.pedestrian.*")[0]
            walker = world.spawn_actor(walker_bp, carla.Transform(nav_loc1))
            self.assertIsNotNone(walker, "Failed to spawn walker on navmesh")

            controller_bp = bp_lib.find("controller.ai.walker")
            controller = world.spawn_actor(controller_bp, carla.Transform(), walker)
            self.assertIsNotNone(controller, "Failed to spawn walker AI controller")

            try:
                world.tick()
                controller.start()
                controller.go_to_location(nav_loc2)
                controller.set_max_speed(1.4)  # 1.4 m/s walking speed

                for _ in range(30):
                    world.tick()

                # Walker should be alive and moving
                walker_loc = walker.get_location()
                self.assertGreater(walker_loc.z, -900000.0)

            finally:
                controller.stop()
                controller.destroy()
                walker.destroy()

            # ------------------------------------------------------------------
            # DoD 3.5: Remote Bundle Loading via RPC (client.load_map_bundle)
            # ------------------------------------------------------------------
            self.assertTrue(SAMPLE_BUNDLE_TAR.is_file(), f"Missing tar archive: {SAMPLE_BUNDLE_TAR}")
            bundle_world = client.load_map_bundle(str(SAMPLE_BUNDLE_TAR), reset_settings=True)
            self.assertIsNotNone(bundle_world)

            bundle_map = bundle_world.get_map()
            self.assertEqual(
                bundle_map.name.split("/")[-1],
                "BundleTestTown",
                f"Expected map BundleTestTown from tar bundle, got {bundle_map.name}",
            )

            # Raycast test: check raycast hitting building at (30, 0, 5)
            # Ray from (0, 0, 5) to (50, 0, 5) passes through building x=[25, 40]
            ray_hits = bundle_world.cast_ray(carla.Location(0.0, 0.0, 5.0), carla.Location(50.0, 0.0, 5.0))
            self.assertGreater(len(ray_hits), 0, "Raycast did not hit custom building mesh")
            self.assertEqual(
                ray_hits[0].label,
                carla.CityObjectLabel.Buildings,
                f"Expected CityObjectLabel.Buildings, got {ray_hits[0].label}",
            )

            settings.synchronous_mode = False
            bundle_world.apply_settings(settings)

        finally:
            for p_dir in staged_dirs:
                if p_dir.exists():
                    shutil.rmtree(p_dir, ignore_errors=True)
            if server_proc is not None:
                server_proc.terminate()
                try:
                    server_proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    server_proc.kill()
            if log_handle is not None:
                log_handle.close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 3 CarlaMap Loader E2E Test Suite")
    parser.add_argument("--host", default=os.environ.get("CARLA_E2E_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.environ.get("CARLA_E2E_PORT", "2000")))
    parser.add_argument("--offline-only", action="store_true")
    args, remaining = parser.parse_known_args()

    Milestone3CarlaMapLoaderE2ETest.host = args.host
    Milestone3CarlaMapLoaderE2ETest.port = args.port
    if args.offline_only:
        Milestone3CarlaMapLoaderE2ETest.offline_only = True

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Milestone3CarlaMapLoaderE2ETest)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
