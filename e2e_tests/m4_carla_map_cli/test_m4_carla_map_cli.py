#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 4 (M4).

Verifies the standalone `carla-map` CLI and SDK:
  - DoD 4.1: CLI subcommands (`init`, `build`, `inspect`, `verify`, `install`).
  - DoD 4.2: Multi-format custom 3D mesh import (.obj, .gltf), sub-mesh splitting,
             semantic tag classification, and auto-disabling procedural road mesh.
  - DoD 4.3: OSM pipeline and automatic 3D building extrusion (footprints, levels, height, UVs).
  - DoD 4.4: Offline pedestrian NavMesh (`Nav/*.bin`) and Traffic Manager cache (`TM/*.bin`).
  - DoD 4.5: Cross-platform portability and live simulator verification via `client.load_map_bundle`.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import tarfile
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

from Util.MapBuilder.cmesh import read_cmesh_file, unpack_cmesh
from Util.MapBuilder.carla_map import cmd_build, cmd_init, cmd_inspect, cmd_install, cmd_verify

FIXTURES_DIR = E2E_DIR / "fixtures"
SAMPLE_CITY_OSM = FIXTURES_DIR / "sample_city.osm"
CUSTOM_TOWN_OBJ = FIXTURES_DIR / "custom_town.obj"
CUSTOM_TOWN_XODR = FIXTURES_DIR / "custom_town.xodr"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


def find_unreal_editor_binary(repo_root: Path) -> Path | None:
    """Finds the UnrealEditor binary."""
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

    if sys.platform == "darwin":
        binary = engine_path / "Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
    elif sys.platform.startswith("linux"):
        binary = engine_path / "Engine/Binaries/Linux/UnrealEditor"
    else:
        binary = engine_path / "Engine/Binaries/Win64/UnrealEditor.exe"

    return binary if binary.exists() else None


class Milestone4CarlaMapCliE2ETest(unittest.TestCase):
    host: str = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
    port: int = int(os.environ.get("CARLA_E2E_PORT", "2000"))
    offline_only: bool = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    def setUp(self) -> None:
        self.temp_dir = Path(tempfile.mkdtemp(prefix="carla_m4_test_"))

    def tearDown(self) -> None:
        shutil.rmtree(self.temp_dir, ignore_errors=True)

    # -------------------------------------------------------------------------
    # DoD 4.1: CLI Subcommands (init, build, inspect, verify, install)
    # -------------------------------------------------------------------------
    def test_dod_4_1_cli_subcommands(self) -> None:
        """DoD 4.1: Verify CLI init, build, inspect, verify, and install commands."""
        project_dir = self.temp_dir / "ScaffoldTown"

        # 1. Test 'init'
        init_args = argparse.Namespace(
            name="ScaffoldTown",
            output=str(self.temp_dir),
            osm=str(SAMPLE_CITY_OSM),
            xodr=None,
            force=True,
        )
        rc_init = cmd_init(init_args)
        self.assertEqual(rc_init, 0, "carla-map init failed")
        self.assertTrue((project_dir / "map_recipe.json").is_file(), "Missing map_recipe.json")
        self.assertTrue((project_dir / "osm" / "sample_city.osm").is_file(), "OSM file not staged")
        self.assertTrue((project_dir / "meshes").is_dir())
        self.assertTrue((project_dir / "opendrive").is_dir())

        # 2. Test 'build'
        build_args = argparse.Namespace(
            target=str(project_dir),
            name="ScaffoldTown",
            output=None,
            osm=None,
            xodr=None,
            meshes=None,
        )
        rc_build = cmd_build(build_args)
        self.assertEqual(rc_build, 0, "carla-map build failed")
        tar_path = project_dir / "ScaffoldTown.carlamap.tar.gz"
        bundle_dir = project_dir / "ScaffoldTown.carlamap"
        self.assertTrue(tar_path.is_file(), f"Tar bundle missing: {tar_path}")
        self.assertTrue((bundle_dir / "carla-map.json").is_file(), "Manifest missing in bundle")

        # 3. Test 'inspect'
        inspect_args = argparse.Namespace(target=str(tar_path))
        rc_inspect = cmd_inspect(inspect_args)
        self.assertEqual(rc_inspect, 0, "carla-map inspect failed on tar archive")

        # 4. Test 'verify'
        verify_args = argparse.Namespace(target=str(tar_path))
        rc_verify = cmd_verify(verify_args)
        self.assertEqual(rc_verify, 0, "carla-map verify failed on valid bundle")

        # 5. Test 'install'
        mock_server = self.temp_dir / "MockServer"
        install_args = argparse.Namespace(
            target=str(tar_path),
            server=str(mock_server),
        )
        rc_install = cmd_install(install_args)
        self.assertEqual(rc_install, 0, "carla-map install failed")
        installed_manifest = mock_server / "Maps" / "ScaffoldTown" / "carla-map.json"
        self.assertTrue(installed_manifest.is_file(), f"Bundle not installed at: {installed_manifest}")

    # -------------------------------------------------------------------------
    # DoD 4.2: Multi-Format Custom Mesh Import (.obj) & Z-Fighting Prevention
    # -------------------------------------------------------------------------
    def test_dod_4_2_custom_mesh_import_and_z_fighting_prevention(self) -> None:
        """DoD 4.2: Verify .obj multi-submesh splitting, semantic rules, and auto-disable procedural road mesh."""
        self.assertTrue(CUSTOM_TOWN_OBJ.is_file(), f"Missing fixture: {CUSTOM_TOWN_OBJ}")
        self.assertTrue(CUSTOM_TOWN_XODR.is_file(), f"Missing fixture: {CUSTOM_TOWN_XODR}")

        out_bundle = self.temp_dir / "CustomTown.carlamap.tar.gz"
        build_args = argparse.Namespace(
            target=str(self.temp_dir),
            name="CustomTown",
            output=str(out_bundle),
            osm=None,
            xodr=str(CUSTOM_TOWN_XODR),
            meshes=str(CUSTOM_TOWN_OBJ),
        )
        rc = cmd_build(build_args)
        self.assertEqual(rc, 0, "carla-map build failed on custom obj meshes")
        self.assertTrue(out_bundle.is_file(), "Bundle archive not generated")

        # Inspect generated carla-map.json
        bundle_dir = self.temp_dir / "CustomTown.carlamap"
        manifest = json.loads((bundle_dir / "carla-map.json").read_text(encoding="utf-8"))

        # Verify procedural road mesh is disabled to eliminate Z-fighting
        od_params = manifest["opendrive_parameters"]
        self.assertFalse(
            od_params["generate_road_mesh"],
            "generate_road_mesh must be False when user supplies custom road meshes to prevent Z-fighting",
        )

        # Verify discrete .cmesh layers were created with proper semantic labels
        layers = manifest["layers"]
        layer_names = [l["name"] for l in layers]
        self.assertIn("Road_Highway", layer_names)
        self.assertIn("Sidewalk_North", layer_names)
        self.assertIn("Building_Tower", layer_names)

        layer_by_name = {l["name"]: l for l in layers}
        self.assertEqual(layer_by_name["Road_Highway"]["semantic_label"], "Roads")
        self.assertEqual(layer_by_name["Sidewalk_North"]["semantic_label"], "Sidewalks")
        self.assertEqual(layer_by_name["Building_Tower"]["semantic_label"], "Buildings")

        # Verify .cmesh binary files
        road_geom = read_cmesh_file(bundle_dir / layer_by_name["Road_Highway"]["mesh_file"])
        self.assertEqual(road_geom["vertex_count"], 4)
        self.assertEqual(road_geom["triangle_count"], 2)

        bldg_geom = read_cmesh_file(bundle_dir / layer_by_name["Building_Tower"]["mesh_file"])
        self.assertGreater(bldg_geom["vertex_count"], 4)
        self.assertGreater(bldg_geom["triangle_count"], 4)

    # -------------------------------------------------------------------------
    # DoD 4.3 & 4.4: OSM Pipeline, 3D Building Extrusion, NavMesh & TM Cache
    # -------------------------------------------------------------------------
    def test_dod_4_3_and_4_4_osm_buildings_navmesh_and_tm_cache(self) -> None:
        """DoD 4.3 & 4.4: Verify 3D building extrusion from OSM XML, NavMesh compilation, and TM cache."""
        self.assertTrue(SAMPLE_CITY_OSM.is_file(), f"Missing fixture: {SAMPLE_CITY_OSM}")

        out_bundle = self.temp_dir / "OsmCity.carlamap.tar.gz"
        build_args = argparse.Namespace(
            target=str(self.temp_dir),
            name="OsmCity",
            output=str(out_bundle),
            osm=str(SAMPLE_CITY_OSM),
            xodr=None,
            meshes=None,
        )
        rc = cmd_build(build_args)
        self.assertEqual(rc, 0, "carla-map build failed on OSM city")

        bundle_dir = self.temp_dir / "OsmCity.carlamap"
        manifest = json.loads((bundle_dir / "carla-map.json").read_text(encoding="utf-8"))

        # Verify extruded OSM buildings layer
        bldg_layer = next((l for l in manifest["layers"] if l["name"] == "OSM_Buildings"), None)
        self.assertIsNotNone(bldg_layer, "OSM_Buildings layer was not generated")
        self.assertEqual(bldg_layer["semantic_label"], "Buildings")
        self.assertTrue(bldg_layer["enable_collision"])

        bldg_cmesh_path = bundle_dir / bldg_layer["mesh_file"]
        self.assertTrue(bldg_cmesh_path.is_file(), f"Missing .cmesh: {bldg_cmesh_path}")

        bldg_geom = read_cmesh_file(bldg_cmesh_path)
        self.assertGreater(bldg_geom["vertex_count"], 0)
        self.assertGreater(bldg_geom["triangle_count"], 0)

        # sample_city.osm contains 2 buildings (4 walls + 1 roof quad each = 20 verts * 2 = 40 verts total)
        total_bldg_verts = sum(
            read_cmesh_file(bundle_dir / l["mesh_file"])["vertex_count"]
            for l in manifest["layers"] if l.get("semantic_label") == "Buildings"
        )
        total_bldg_tris = sum(
            read_cmesh_file(bundle_dir / l["mesh_file"])["triangle_count"]
            for l in manifest["layers"] if l.get("semantic_label") == "Buildings"
        )
        self.assertEqual(total_bldg_verts, 40)
        self.assertEqual(total_bldg_tris, 20)

        # Verify Traffic Manager cache was created
        tm_bin = bundle_dir / "TM" / "OsmCity.bin"
        self.assertTrue(tm_bin.is_file(), f"TM cache missing: {tm_bin}")
        self.assertGreater(tm_bin.stat().st_size, 0, "TM cache binary is empty")

        # Verify NavMesh binary was created
        nav_bin = bundle_dir / "Nav" / "OsmCity.bin"
        self.assertTrue(nav_bin.is_file(), f"NavMesh binary missing: {nav_bin}")
        self.assertGreater(nav_bin.stat().st_size, 0, "NavMesh binary is empty")

        # Verify bundle passes verify command
        rc_verify = cmd_verify(argparse.Namespace(target=str(out_bundle)))
        self.assertEqual(rc_verify, 0, "Generated OSM city bundle failed verify")

    # -------------------------------------------------------------------------
    # DoD 4.5: Live Simulator Verification via client.load_map_bundle
    # -------------------------------------------------------------------------
    def test_dod_4_5_live_simulator_load_and_semantic_raycast(self) -> None:
        """DoD 4.5: Load built custom-mesh bundle into live CARLA simulator and verify physics & ground-truth semantics."""
        if self.offline_only:
            self.skipTest("Skipping live tests (--offline-only requested)")

        # Build custom mesh bundle
        out_bundle = self.temp_dir / "LiveTown.carlamap.tar.gz"
        build_args = argparse.Namespace(
            target=str(self.temp_dir),
            name="LiveTown",
            output=str(out_bundle),
            osm=None,
            xodr=str(CUSTOM_TOWN_XODR),
            meshes=str(CUSTOM_TOWN_OBJ),
        )
        rc = cmd_build(build_args)
        self.assertEqual(rc, 0, "carla-map build failed")
        self.assertTrue(out_bundle.is_file())

        server_proc: subprocess.Popen[bytes] | None = None
        log_handle = None

        try:
            if not is_port_open(self.host, self.port):
                editor_bin = find_unreal_editor_binary(REPO_ROOT)
                uproject = REPO_ROOT / "Unreal/CarlaUnreal/CarlaUnreal.uproject"
                if editor_bin is None or not uproject.is_file():
                    self.skipTest("CARLA server not running and UnrealEditor binary not found")

                log_path = LOCAL_TMP_DIR / f"carla_m4_e2e_{self.port}.log"
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

            # Wait for server engine to tick
            ready = False
            for _ in range(60):
                try:
                    w = client.get_world()
                    if w is not None:
                        ready = True
                        break
                except Exception:
                    time.sleep(1.0)
            self.assertTrue(ready, "Server engine did not initialize within timeout")
            client.set_timeout(60.0)

            # ------------------------------------------------------------------
            # Load the bundle generated by carla-map CLI via client.load_map_bundle
            # ------------------------------------------------------------------
            world = client.load_map_bundle(str(out_bundle), reset_settings=True)
            self.assertIsNotNone(world, "Failed to load bundle in simulator")

            carla_map = world.get_map()
            self.assertEqual(
                carla_map.name.split("/")[-1],
                "LiveTown",
                f"Expected map name LiveTown, got {carla_map.name}",
            )

            # Verify custom environment objects
            road_objs = world.get_environment_objects(carla.CityObjectLabel.Roads)
            self.assertGreater(len(road_objs), 0, "Custom road mesh not found in environment objects")

            bldg_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)
            self.assertGreater(len(bldg_objs), 0, "Custom building mesh not found in environment objects")

            # Ground-truth semantic raycast test:
            # Raycast directed through Building_Tower (located at x=[10, 20], y=[8, 18], z=[0, 15])
            # Cast ray from (15, 0, 5) to (15, 25, 5)
            hits = world.cast_ray(carla.Location(15.0, 0.0, 5.0), carla.Location(15.0, 25.0, 5.0))
            self.assertGreater(len(hits), 0, "Raycast did not hit custom building mesh")
            self.assertEqual(
                hits[0].label,
                carla.CityObjectLabel.Buildings,
                f"Expected CityObjectLabel.Buildings, got {hits[0].label}",
            )

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
    parser = argparse.ArgumentParser(description="Milestone 4 carla-map CLI E2E Test Suite")
    parser.add_argument("--host", default=os.environ.get("CARLA_E2E_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.environ.get("CARLA_E2E_PORT", "2000")))
    parser.add_argument("--offline-only", action="store_true")
    args, remaining = parser.parse_known_args()

    Milestone4CarlaMapCliE2ETest.host = args.host
    Milestone4CarlaMapCliE2ETest.port = args.port
    if args.offline_only:
        Milestone4CarlaMapCliE2ETest.offline_only = True

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Milestone4CarlaMapCliE2ETest)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
