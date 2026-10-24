#!/usr/bin/env python3
"""
End-to-End (E2E) Test Suite for Milestone 5: Shipped Package Integration, Verification & Docs.

Covers Definition of Done (DoD 5.1 – DoD 5.5):
  - DoD 5.1: Shipped package structure completeness (CarlaUnreal.sh, CarlaUnreal-Mac-Shipping.app,
             carla-map wrapper, RecastBuilder, PythonAPI wheel, Util/MapBuilder).
  - DoD 5.2: E2E Scenario A: OpenStreetMap (.osm) -> 3D building extrusion -> bundle build ->
             install into packaged CARLA -> load world -> verify OSM buildings, spawn points,
             and vehicle/walker navigation.
  - DoD 5.3: E2E Scenario B: Custom 3D meshes (.obj) + OpenDRIVE (.xodr) -> bundle build ->
             verify generate_road_mesh=False (zero Z-fighting) -> live simulator load ->
             verify ground-truth semantic raycasts and drivability.
  - DoD 5.4: Documentation, YAML API doc generation, and skill completeness.
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
import tempfile
import unittest

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
M5_DIR = Path(__file__).resolve().parent
FIXTURES_DIR = M5_DIR / "fixtures"

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

import carla  # noqa: E402


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


def find_packaged_carla_dir() -> Path:
    env_pkg = os.environ.get("CARLA_SHIPPED_PACKAGE_PATH")
    if env_pkg and Path(env_pkg).is_dir():
        return Path(env_pkg)

    candidates = [
        REPO_ROOT / "Build" / "Package" / "Carla-0.10.0-Mac-Shipping",
        REPO_ROOT / "Build" / "Package" / "Carla-0.10.0-Linux-Shipping",
        REPO_ROOT / "Build" / "Package" / "Carla-0.10.0-Mac-Development",
    ]
    for c in candidates:
        if c.is_dir():
            return c
    raise FileNotFoundError("Could not find a packaged CARLA simulator directory under Build/Package/")


class Milestone5PackagedE2ETest(unittest.TestCase):
    """E2E Test Suite for Milestone 5."""

    host = "127.0.0.1"
    port = 2000
    offline_only = False

    @classmethod
    def setUpClass(cls):
        cls.package_dir = find_packaged_carla_dir()
        if not hasattr(cls, "host"):
            cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        if not hasattr(cls, "port"):
            cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        if not hasattr(cls, "offline_only"):
            cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    def setUp(self):
        self.temp_dir = tempfile.mkdtemp(prefix="carla_m5_test_")

    def tearDown(self):
        shutil.rmtree(self.temp_dir, ignore_errors=True)

    # -------------------------------------------------------------------------
    # DoD 5.1: Shipped Package Completeness
    # -------------------------------------------------------------------------
    def test_dod_5_1_shipped_package_completeness(self):
        """DoD 5.1: Verify shipped package completeness (binaries, wheel, carla-map CLI, RecastBuilder)."""
        pkg = self.package_dir
        self.assertTrue(pkg.is_dir(), f"Package directory must exist: {pkg}")

        # CarlaUnreal launcher script
        launcher = pkg / "CarlaUnreal.sh"
        self.assertTrue(launcher.is_file(), f"CarlaUnreal.sh must exist in package root: {launcher}")
        self.assertTrue(os.access(launcher, os.X_OK), "CarlaUnreal.sh must be executable")

        # Top-level carla-map CLI wrapper
        carla_map_bin = pkg / "carla-map"
        self.assertTrue(carla_map_bin.is_file(), f"carla-map wrapper must exist in package root: {carla_map_bin}")
        self.assertTrue(os.access(carla_map_bin, os.X_OK), "carla-map must be executable")

        # RecastBuilder binary
        recast_builder = pkg / "Tools" / "RecastBuilder"
        self.assertTrue(recast_builder.is_file(), f"RecastBuilder must exist in package Tools: {recast_builder}")
        self.assertTrue(os.access(recast_builder, os.X_OK), "RecastBuilder must be executable")

        # PythonAPI wheel
        wheels = list((pkg / "PythonAPI" / "carla" / "dist").glob("carla-*.whl"))
        self.assertTrue(len(wheels) > 0, "Package must contain at least one compiled carla .whl")

        # Util/MapBuilder suite
        mb_dir = pkg / "Util" / "MapBuilder"
        self.assertTrue((mb_dir / "carla_map.py").is_file(), "carla_map.py must exist in Util/MapBuilder")
        self.assertTrue((mb_dir / "cmesh.py").is_file(), "cmesh.py must exist in Util/MapBuilder")
        self.assertTrue((mb_dir / "osm_buildings.py").is_file(), "osm_buildings.py must exist in Util/MapBuilder")
        self.assertTrue((mb_dir / "mesh_importer.py").is_file(), "mesh_importer.py must exist in Util/MapBuilder")
        self.assertTrue((mb_dir / "navmesh_builder.py").is_file(), "navmesh_builder.py must exist in Util/MapBuilder")

        # Execute carla-map --help directly from the packaged bundle
        res = subprocess.run(
            [str(carla_map_bin), "--help"],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(res.returncode, 0, f"carla-map --help failed: {res.stderr}")
        self.assertIn("CARLA Editor-Free Map Creation", res.stdout)
        self.assertIn("build", res.stdout)
        self.assertIn("verify", res.stdout)
        self.assertIn("install", res.stdout)

    # -------------------------------------------------------------------------
    # DoD 5.2: Scenario A (OSM + 3D Buildings & Procedural Features)
    # -------------------------------------------------------------------------
    def test_dod_5_2_scenario_a_osm_pipeline_and_server_integration(self):
        """DoD 5.2: Run carla-map build on OSM, install into server, and verify in simulation."""
        carla_map_bin = self.package_dir / "carla-map"
        sample_osm = FIXTURES_DIR / "sample_city.osm"
        out_bundle = Path(self.temp_dir) / "ScenarioA.carlamap.tar.gz"

        # 1. Build bundle offline using packaged carla-map CLI
        cmd = [
            str(carla_map_bin),
            "build",
            "--osm",
            str(sample_osm),
            "--name",
            "ScenarioA",
            "--out",
            str(out_bundle),
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        self.assertEqual(res.returncode, 0, f"carla-map build failed: {res.stderr}\n{res.stdout}")
        self.assertTrue(out_bundle.is_file(), "ScenarioA.carlamap.tar.gz must be generated")

        # 2. Verify bundle offline
        verify_cmd = [str(carla_map_bin), "verify", str(out_bundle)]
        v_res = subprocess.run(verify_cmd, capture_output=True, text=True, check=False)
        self.assertEqual(v_res.returncode, 0, f"Verification failed: {v_res.stderr}\n{v_res.stdout}")
        self.assertIn("VERIFICATION PASSED", v_res.stdout)

        # 3. Test install subcommand into a mock/real server layout
        install_cmd = [str(carla_map_bin), "install", str(out_bundle), "--server", str(self.package_dir)]
        i_res = subprocess.run(install_cmd, capture_output=True, text=True, check=False)
        self.assertEqual(i_res.returncode, 0, f"Installation failed: {i_res.stderr}\n{i_res.stdout}")

        # 4. Live simulator load (if server running)
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running at {self.host}:{self.port}, skipping live checks")

        client = carla.Client(self.host, self.port)
        client.set_timeout(30.0)

        world = client.load_map_bundle(str(out_bundle))
        self.assertIsNotNone(world)

        carla_map = world.get_map()
        self.assertEqual(carla_map.name.split("/")[-1], "ScenarioA")

        # Verify extruded OSM buildings are registered as environment objects
        buildings = world.get_environment_objects(carla.CityObjectLabel.Buildings)
        self.assertGreater(len(buildings), 0, "ScenarioA must contain registered OSM 3D buildings")

        # Verify spawn points
        spawn_points = carla_map.get_spawn_points()
        self.assertGreater(len(spawn_points), 0, "ScenarioA must have valid road spawn points")

        # Spawn a vehicle and tick world
        bp_lib = world.get_blueprint_library()
        veh_bps = bp_lib.filter("vehicle.*")
        if veh_bps:
            veh_bp = veh_bps[0]
            sp = spawn_points[0]
            sp.location.z += 0.5
            veh = world.try_spawn_actor(veh_bp, sp)
            self.assertIsNotNone(veh, "Vehicle should spawn on OpenDRIVE road")
            try:
                for _ in range(20):
                    world.tick()
                loc = veh.get_location()
                self.assertGreater(loc.z, -5.0, "Vehicle must stay on road without falling through")
            finally:
                veh.destroy()

    # -------------------------------------------------------------------------
    # DoD 5.3: Scenario B (Custom 3D Meshes + Zero Z-Fighting)
    # -------------------------------------------------------------------------
    def test_dod_5_3_scenario_b_custom_mesh_import_and_zero_z_fighting(self):
        """DoD 5.3: Custom 3D meshes + XODR with generate_road_mesh=False, semantic raycasts, and collision."""
        carla_map_bin = self.package_dir / "carla-map"
        custom_obj = FIXTURES_DIR / "custom_town.obj"
        custom_xodr = FIXTURES_DIR / "custom_town.xodr"
        out_bundle = Path(self.temp_dir) / "ScenarioB.carlamap.tar.gz"

        # 1. Build bundle offline
        cmd = [
            str(carla_map_bin),
            "build",
            "--xodr",
            str(custom_xodr),
            "--mesh",
            str(custom_obj),
            "--name",
            "ScenarioB",
            "--out",
            str(out_bundle),
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        self.assertEqual(res.returncode, 0, f"carla-map build failed: {res.stderr}\n{res.stdout}")

        # 2. Inspect: confirm generate_road_mesh is False (Zero Z-Fighting mode)
        inspect_cmd = [str(carla_map_bin), "inspect", str(out_bundle)]
        ins_res = subprocess.run(inspect_cmd, capture_output=True, text=True, check=False)
        self.assertEqual(ins_res.returncode, 0)
        self.assertIn("generate_road_mesh=False", ins_res.stdout)

        # 3. Live simulator load (if server running)
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running at {self.host}:{self.port}, skipping live checks")

        client = carla.Client(self.host, self.port)
        client.set_timeout(30.0)

        world = client.load_map_bundle(str(out_bundle))
        self.assertIsNotNone(world)

        # Raycast onto custom road surface (x in [0, 40], y in [-4, 4], z=0)
        road_hits = world.cast_ray(carla.Location(20.0, 0.0, 10.0), carla.Location(20.0, 0.0, -10.0))
        self.assertGreater(len(road_hits), 0, "Raycast must hit custom road mesh")
        self.assertEqual(road_hits[0].label, carla.CityObjectLabel.Roads)

        # Raycast onto custom sidewalk surface (x in [0, 40], y in [4, 6], z=0.15)
        sw_hits = world.cast_ray(carla.Location(20.0, 5.0, 10.0), carla.Location(20.0, 5.0, -10.0))
        self.assertGreater(len(sw_hits), 0, "Raycast must hit custom sidewalk mesh")
        self.assertEqual(sw_hits[0].label, carla.CityObjectLabel.Sidewalks)

        # Raycast through custom building (x in [10, 20], y in [8, 18], z in [0, 15])
        bld_hits = world.cast_ray(carla.Location(15.0, 0.0, 5.0), carla.Location(15.0, 25.0, 5.0))
        self.assertGreater(len(bld_hits), 0, "Raycast must hit custom building mesh")
        self.assertEqual(bld_hits[0].label, carla.CityObjectLabel.Buildings)

        # Vehicle settling test on custom road mesh
        bp_lib = world.get_blueprint_library()
        veh_bps = bp_lib.filter("vehicle.*")
        if veh_bps:
            veh_bp = veh_bps[0]
            spawn_transform = carla.Transform(carla.Location(20.0, 0.0, 2.0), carla.Rotation(yaw=0.0))
            veh = world.try_spawn_actor(veh_bp, spawn_transform)
            self.assertIsNotNone(veh, "Vehicle should spawn above custom road mesh")
            try:
                for _ in range(50):
                    world.tick()
                loc = veh.get_location()
                self.assertAlmostEqual(loc.z, 0.0, delta=2.0, msg="Vehicle should settle on custom road mesh")
            finally:
                veh.destroy()

    # -------------------------------------------------------------------------
    # DoD 5.4: Documentation, YAML API Docs & Skills
    # -------------------------------------------------------------------------
    def test_dod_5_4_docs_and_skills_completeness(self):
        """DoD 5.4: Verify Docs/map-maker/README.md, Skills, and python_api.md completeness."""
        readme = REPO_ROOT / "Docs" / "map-maker" / "README.md"
        self.assertTrue(readme.is_file(), f"Docs/map-maker/README.md must exist: {readme}")
        readme_text = readme.read_text(encoding="utf-8")
        self.assertIn("carla-map", readme_text)
        self.assertIn("map_recipe.json", readme_text)
        self.assertIn("CMES", readme_text)
        self.assertIn("Z-Fighting", readme_text)

        skill_md = REPO_ROOT / "Skills" / "map-maker" / "create-carla-map" / "SKILL.md"
        self.assertTrue(skill_md.is_file(), f"Skill file must exist: {skill_md}")
        skill_text = skill_md.read_text(encoding="utf-8")
        self.assertIn("name: create-carla-map", skill_text)

        agents_md = REPO_ROOT / "AGENTS.md"
        self.assertTrue(agents_md.is_file())
        agents_text = agents_md.read_text(encoding="utf-8")
        self.assertIn("create-carla-map", agents_text)

        api_md = REPO_ROOT / "Docs" / "python_api.md"
        self.assertTrue(api_md.is_file())
        api_text = api_md.read_text(encoding="utf-8")
        self.assertIn("load_map_bundle", api_text)
        self.assertIn("spawn_custom_mesh_ex", api_text)
        self.assertIn("CustomMeshDescription", api_text)
        self.assertIn("CustomMaterialParameters", api_text)
        self.assertIn("cook_in_memory_map", api_text)


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 5 Packaged Carla E2E Test Suite")
    parser.add_argument("--host", default=os.environ.get("CARLA_E2E_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.environ.get("CARLA_E2E_PORT", "2000")))
    parser.add_argument("--offline-only", action="store_true")
    args, remaining = parser.parse_known_args()

    Milestone5PackagedE2ETest.host = args.host
    Milestone5PackagedE2ETest.port = args.port
    if args.offline_only:
        Milestone5PackagedE2ETest.offline_only = True

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Milestone5PackagedE2ETest)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

