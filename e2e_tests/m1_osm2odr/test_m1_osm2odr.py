#!/usr/bin/env python3
"""
Milestone 1 End-to-End Verification Suite: OSM2ODR Build & Python Wheel Integration.

Verifies all Definition-of-Done (DoD) items for Milestone 1:
  - DoD 1.1: Static libraries (libosm2odr.a, libproj.a, libxerces-c.a, liblibsqlite3.a) built via CMake.
  - DoD 1.2: carla.Osm2Odr & carla.Osm2OdrSettings exposed in Python wheel with 100% static linkage.
  - DoD 1.3: Carla.Build.cs & CarlaTools.Build.cs compiled with WITH_OSM2ODR and linked against libosm2odr.a.
  - DoD 1.4: Offline .osm -> .xodr conversion and carla.Map waypoint/topology/landmark generation.
  - DoD 1.5: Live CARLA server generate_opendrive_world(xodr_str) and vehicle physics settlement test.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import platform
import socket
import subprocess
import sys
import tempfile
import time
import unittest
import xml.etree.ElementTree as ET

import carla


REPO_ROOT = Path(__file__).resolve().parents[2]
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
tempfile.tempdir = str(LOCAL_TMP_DIR)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SAMPLE_OSM_PATH = FIXTURES_DIR / "sample_network.osm"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


def find_unreal_editor_binary(repo_root: Path) -> Path | None:
    # Check CMakeCache.txt for CARLA_UNREAL_ENGINE_PATH first
    cmake_cache = repo_root / "Build" / "CMakeCache.txt"
    engine_path: Path | None = None
    if cmake_cache.is_file():
        for line in cmake_cache.read_text(encoding="utf-8", errors="ignore").splitlines():
            if line.startswith("CARLA_UNREAL_ENGINE_PATH:"):
                val = line.split("=", 1)[1].strip()
                if val:
                    engine_path = Path(val)
                    break
    if engine_path is None and os.environ.get("CARLA_UNREAL_ENGINE_PATH"):
        engine_path = Path(os.environ["CARLA_UNREAL_ENGINE_PATH"])

    if engine_path is None:
        candidate = repo_root.parent / "UnrealEngine5_carla"
        if candidate.is_dir():
            engine_path = candidate

    if engine_path is None:
        return None

    if platform.system() == "Darwin":
        binary = engine_path / "Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
    elif platform.system() == "Linux":
        binary = engine_path / "Engine/Binaries/Linux/UnrealEditor"
    else:
        binary = engine_path / "Engine/Binaries/Win64/UnrealEditor.exe"

    return binary if binary.exists() else None


class Milestone1Osm2OdrE2ETest(unittest.TestCase):
    host: str = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
    port: int = int(os.environ.get("CARLA_E2E_PORT", "2000"))
    offline_only: bool = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    def _convert_sample_osm(self) -> str:
        self.assertTrue(SAMPLE_OSM_PATH.is_file(), f"Missing fixture: {SAMPLE_OSM_PATH}")
        osm_xml = SAMPLE_OSM_PATH.read_text(encoding="utf-8")

        settings = carla.Osm2OdrSettings()
        settings.set_osm_way_types([
            "motorway",
            "motorway_link",
            "trunk",
            "trunk_link",
            "primary",
            "primary_link",
            "secondary",
            "secondary_link",
            "tertiary",
            "tertiary_link",
            "unclassified",
            "residential",
        ])
        settings.default_lane_width = 3.5
        settings.default_sidewalk_width = 3.0
        settings.elevation_layer_height = 4.0
        settings.generate_traffic_lights = True
        settings.center_map = True

        return carla.Osm2Odr.convert(osm_xml, settings)

    def test_dod_1_1_static_libraries_built(self) -> None:
        """DoD 1.1: Verify static archives for osm2odr, proj, xerces-c, and sqlite3 exist in Build/."""
        build_dir = REPO_ROOT / "Build"
        expected_archives = [
            build_dir / "_deps/osm2odr-build/src/libosm2odr.a",
            build_dir / "_deps/proj-build/lib/libproj.a",
            build_dir / "_deps/xercesc-build/src/libxerces-c.a",
            build_dir / "liblibsqlite3.a",
        ]
        for archive in expected_archives:
            self.assertTrue(archive.is_file(), f"Expected static archive not found: {archive}")
            self.assertGreater(archive.stat().st_size, 100_000, f"Static archive is unexpectedly small: {archive}")

    def test_dod_1_2_python_wheel_and_static_linkage(self) -> None:
        """DoD 1.2: Verify carla.Osm2Odr/Osm2OdrSettings bindings and static linkage (no external .dylib/.so)."""
        self.assertTrue(hasattr(carla, "Osm2Odr"), "carla.Osm2Odr is missing from the carla module")
        self.assertTrue(hasattr(carla, "Osm2OdrSettings"), "carla.Osm2OdrSettings is missing from the carla module")

        settings = carla.Osm2OdrSettings()
        settings.default_lane_width = 3.5
        settings.default_sidewalk_width = 3.0
        self.assertAlmostEqual(settings.default_lane_width, 3.5, places=3)
        self.assertAlmostEqual(settings.default_sidewalk_width, 3.0, places=3)
        self.assertIn("default_sidewalk_width=3", str(settings))

        so_path = Path(carla.__file__).resolve()
        self.assertTrue(so_path.is_file(), f"carla shared object not found: {so_path}")

        if platform.system() == "Darwin":
            otool_out = subprocess.check_output(["otool", "-L", str(so_path)], text=True)
            for forbidden in ("libosm2odr", "libproj", "libxerces", "libcurl", "libicu"):
                self.assertNotIn(
                    forbidden,
                    otool_out.lower(),
                    f"Found unexpected dynamic library dependency '{forbidden}' in otool -L:\n{otool_out}",
                )
        elif platform.system() == "Linux":
            ldd_out = subprocess.check_output(["ldd", str(so_path)], text=True)
            for forbidden in ("libosm2odr", "libproj", "libxerces"):
                self.assertNotIn(
                    forbidden,
                    ldd_out.lower(),
                    f"Found unexpected dynamic library dependency '{forbidden}' in ldd:\n{ldd_out}",
                )

    def test_dod_1_3_unreal_plugin_symbols(self) -> None:
        """DoD 1.3: Verify Carla.Build.cs / CarlaTools.Build.cs guards and linked ConvertOSMToOpenDRIVE symbol."""
        carla_build_cs = (
            REPO_ROOT / "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Carla.Build.cs"
        ).read_text(encoding="utf-8")
        carla_tools_build_cs = (
            REPO_ROOT / "Unreal/CarlaUnreal/Plugins/CarlaTools/Source/CarlaTools/CarlaTools.Build.cs"
        ).read_text(encoding="utf-8")

        self.assertIn('PrivateDefinitions.Add(definition);', carla_tools_build_cs)
        self.assertNotIn('PrivateDefinitions.Add(name);', carla_tools_build_cs)

        if platform.system() == "Darwin":
            dylib = REPO_ROOT / "Unreal/CarlaUnreal/Plugins/CarlaTools/Binaries/Mac/libUnrealEditor-CarlaTools.dylib"
            if dylib.is_file():
                nm_out = subprocess.check_output(["nm", str(dylib)], text=True)
                self.assertIn("ConvertOSMToOpenDRIVE", nm_out)
                self.assertIn("ConvertOSMInOpenDrive", nm_out)
        elif platform.system() == "Linux":
            so_file = REPO_ROOT / "Unreal/CarlaUnreal/Plugins/CarlaTools/Binaries/Linux/libUnrealEditor-CarlaTools.so"
            if so_file.is_file():
                nm_out = subprocess.check_output(["nm", "-D", str(so_file)], text=True)
                self.assertIn("ConvertOSMToOpenDRIVE", nm_out)

    def test_dod_1_4_offline_osm_to_xodr_and_map_topology(self) -> None:
        """DoD 1.4: Convert sample_network.osm -> .xodr and validate offline carla.Map waypoints & topology."""
        xodr_str = self._convert_sample_osm()
        self.assertGreater(len(xodr_str), 10_000, "Generated OpenDRIVE XML is unexpectedly short")

        root = ET.fromstring(xodr_str)
        self.assertEqual(root.tag, "OpenDRIVE")
        roads = root.findall("road")
        junctions = root.findall("junction")
        self.assertGreaterEqual(len(roads), 10, "Expected at least 10 <road> elements in converted OpenDRIVE")
        self.assertGreaterEqual(len(junctions), 2, "Expected at least 2 <junction> elements in converted OpenDRIVE")

        carla_map = carla.Map("E2E_TestOSM", xodr_str)
        waypoints = carla_map.generate_waypoints(2.0)
        topology = carla_map.get_topology()
        landmarks = carla_map.get_all_landmarks()

        self.assertGreater(len(waypoints), 100, "Expected > 100 waypoints at 2.0m spacing")
        self.assertGreater(len(topology), 10, "Expected > 10 topology segments")
        self.assertGreater(len(landmarks), 0, "Expected traffic light landmarks from signalized OSM node")

    def test_dod_1_5_live_simulator_opendrive_world_and_vehicle_physics(self) -> None:
        """DoD 1.5: Load converted OSM network in live CARLA server and verify vehicle physics on road mesh."""
        if self.offline_only:
            self.skipTest("Skipping live server test because --offline-only / CARLA_E2E_OFFLINE_ONLY=1 was set")

        server_proc: subprocess.Popen[bytes] | None = None
        log_handle = None

        try:
            if not is_port_open(self.host, self.port):
                editor_bin = find_unreal_editor_binary(REPO_ROOT)
                uproject = REPO_ROOT / "Unreal/CarlaUnreal/CarlaUnreal.uproject"
                self.assertIsNotNone(
                    editor_bin,
                    f"No CARLA server running on {self.host}:{self.port} and UnrealEditor binary not found",
                )
                log_path = LOCAL_TMP_DIR / f"carla_e2e_m1_{self.port}.log"
                log_handle = log_path.open("w", encoding="utf-8")
                server_proc = subprocess.Popen(
                    [
                        str(editor_bin),
                        str(uproject),
                        "OpenDriveMap",
                        "-game",
                        "-nullrhi",
                        "-notrace",
                        "-nosound",
                        "-stdout",
                        f"-carla-rpc-port={self.port}",
                    ],
                    stdout=log_handle,
                    stderr=subprocess.STDOUT,
                )

            client: carla.Client | None = None
            for _ in range(90):
                if server_proc is not None and server_proc.poll() is not None:
                    self.fail(f"CARLA server exited prematurely with code {server_proc.returncode}")
                if is_port_open(self.host, self.port):
                    try:
                        client = carla.Client(self.host, self.port)
                        client.set_timeout(30.0)
                        _ = client.get_server_version()
                        break
                    except RuntimeError:
                        client = None
                time.sleep(1.0)

            self.assertIsNotNone(client, f"Timed out waiting for CARLA server on {self.host}:{self.port}")
            assert client is not None

            xodr_str = self._convert_sample_osm()
            world = client.generate_opendrive_world(
                xodr_str,
                carla.OpendriveGenerationParameters(
                    vertex_distance=2.0,
                    max_road_length=50.0,
                    wall_height=0.0,
                    additional_width=0.6,
                    smooth_junctions=True,
                    enable_mesh_visibility=True,
                    enable_pedestrian_navigation=True,
                ),
            )
            time.sleep(1.0)
            world = client.get_world()
            sim_map = world.get_map()
            waypoints = sim_map.generate_waypoints(2.0)
            self.assertIn("OpenDriveMap", sim_map.name)
            self.assertGreater(len(waypoints), 100)

            original_settings = world.get_settings()
            vehicle = None
            try:
                sim_settings = world.get_settings()
                sim_settings.synchronous_mode = True
                sim_settings.fixed_delta_seconds = 0.05
                world.apply_settings(sim_settings)

                driving_wps = [
                    wp
                    for wp in waypoints
                    if not wp.is_junction and wp.lane_type == carla.LaneType.Driving
                ]
                self.assertGreater(len(driving_wps), 0, "No non-junction driving waypoints found")

                spawn_wp = driving_wps[len(driving_wps) // 2]
                spawn_tf = spawn_wp.transform
                spawn_tf.location.z += 0.5

                veh_bp = world.get_blueprint_library().filter("vehicle.*")[0]
                vehicle = world.spawn_actor(veh_bp, spawn_tf)

                for _ in range(40):
                    world.tick()

                final_loc = vehicle.get_location()
                self.assertGreater(
                    final_loc.z,
                    spawn_wp.transform.location.z - 0.5,
                    f"Vehicle fell through the generated road mesh (z={final_loc.z:.3f})",
                )
            finally:
                if vehicle is not None:
                    vehicle.destroy()
                world.apply_settings(original_settings)
        finally:
            if server_proc is not None:
                server_proc.terminate()
                try:
                    server_proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    server_proc.kill()
            if log_handle is not None:
                log_handle.close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Milestone 1 (OSM2ODR) E2E verification tests.")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=2000, help="CARLA server RPC port (default: 2000)")
    parser.add_argument(
        "--offline-only",
        action="store_true",
        help="Run only offline build/linkage/conversion tests (DoD 1.1 - 1.4), skipping live server test (DoD 1.5)",
    )
    args, remaining = parser.parse_known_args()

    Milestone1Osm2OdrE2ETest.host = args.host
    Milestone1Osm2OdrE2ETest.port = args.port
    Milestone1Osm2OdrE2ETest.offline_only = args.offline_only

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Milestone1Osm2OdrE2ETest)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

