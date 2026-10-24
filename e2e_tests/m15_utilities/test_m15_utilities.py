#!/usr/bin/env python3
"""
Milestone 15 (M15) E2E Test Suite: Municipal Utilities & Fire Hydrants.

Verifies:
  - DoD 15.1: Ingestion of OSM utility tags (emergency=fire_hydrant, man_made=street_cabinet, man_made=manhole, utility=*) & underground filtering
  - DoD 15.2: 3D Pillar fire hydrants (SM_FireHdrant) & street cabinets (SM_LightBox1, SM_MailBox)
  - DoD 15.3: Cast-iron ground cover geometry (circular manholes & rectangular DIN 4055 underground hydrant plates)
  - DoD 15.4: Procedural fire hydrant synthesis along corridors (DIN 14461 standard: 100m-150m spacing)
  - DoD 15.5: NavMesh obstacle hulls for standing utilities and walkable surface for flush covers
  - DoD 15.6: Bundle compilation, inspection, and check 14 verification (carla-map build / inspect / verify)
  - DoD 15.7: Real-world Munich Stachus utilities extraction and geometry generation
"""

from __future__ import annotations

import argparse
import io
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
from unittest.mock import MagicMock

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
from Util.MapBuilder.osm_utilities import (
    ASSET_PILLAR_HYDRANT,
    ASSET_POSTAL_CABINET,
    ASSET_STREET_CABINET,
    UtilityItem,
    audit_utility_metrics,
    classify_utility_node,
    extract_osm_utilities,
    generate_utility_hism_placements,
    generate_utility_mesh_layers,
    generate_utility_obstacle_geometry,
    point_in_polygon,
    point_near_cycleway,
    save_utilities_manifest,
    synthesize_procedural_fire_hydrants,
)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SAMPLE_UTILITIES_OSM = FIXTURES_DIR / "sample_utilities.osm"
MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"


def is_carla_server_running(host: str = "127.0.0.1", port: int = 2000, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except (OSError, ConnectionRefusedError):
        return False


class TestMilestone15Utilities(unittest.TestCase):
    """Milestone 15 E2E Test Suite for Municipal Utilities & Fire Hydrants."""

    def test_01_osm_utility_ingestion_and_underground_filtering(self) -> None:
        """DoD 15.1: Ingest OSM utility tags and verify underground (level < 0) and indoor features are excluded."""
        self.assertTrue(SAMPLE_UTILITIES_OSM.is_file(), f"Fixture missing: {SAMPLE_UTILITIES_OSM}")

        def dummy_proj(lat: float, lon: float) -> tuple[float, float, float]:
            return ((lon - 11.565) * 74000.0, -(lat - 48.140) * 111000.0, 0.0)

        items = extract_osm_utilities(SAMPLE_UTILITIES_OSM, dummy_proj)
        self.assertEqual(len(items), 7, f"Expected 7 above-ground utility items, got {len(items)}")

        # Verify underground node 5001 (level=-2) and indoor node 5002 are filtered out
        osm_ids = [item.osm_id for item in items]
        self.assertNotIn(5001, osm_ids, "Underground hydrant (level=-2) must be excluded")
        self.assertNotIn(5002, osm_ids, "Indoor station cabinet (indoor=yes) must be excluded")

        item_map = {item.osm_id: item for item in items}

        # 1. Authentic Pillar Fire Hydrant
        p_hyd = item_map[2001]
        self.assertEqual(p_hyd.category, "fire_hydrant_pillar")
        self.assertEqual(p_hyd.subtype, "pillar")
        self.assertEqual(p_hyd.asset_path, ASSET_PILLAR_HYDRANT)
        self.assertEqual(p_hyd.diameter_mm, 100)
        self.assertEqual(p_hyd.operator, "Feuerwehr München")

        # 2. Underground Fire Hydrant
        u_hyd = item_map[2002]
        self.assertEqual(u_hyd.category, "fire_hydrant_underground")
        self.assertEqual(u_hyd.subtype, "underground")
        self.assertEqual(u_hyd.diameter_mm, 150)
        self.assertEqual(u_hyd.operator, "SWM")

        # 3. Telecom Cabinet
        tc_cab = item_map[3001]
        self.assertEqual(tc_cab.category, "street_cabinet")
        self.assertEqual(tc_cab.subtype, "telecom")
        self.assertEqual(tc_cab.asset_path, ASSET_STREET_CABINET)

        # 4. Postal Cabinet
        post_cab = item_map[3003]
        self.assertEqual(post_cab.category, "street_cabinet")
        self.assertEqual(post_cab.subtype, "postal")
        self.assertEqual(post_cab.asset_path, ASSET_POSTAL_CABINET)

        # 5. Manhole Storm Drain
        drain_mh = item_map[4001]
        self.assertEqual(drain_mh.category, "manhole_cover")
        self.assertEqual(drain_mh.subtype, "drain")

    def test_02_3d_standing_assets_and_hism_placements(self) -> None:
        """DoD 15.2 & 15.3: Verify HISM placement formatting and asset paths for pillar hydrants and cabinets."""
        items = [
            UtilityItem(
                category="fire_hydrant_pillar",
                subtype="pillar",
                location=(10.0, 20.0, 0.15),
                rotation=(0.0, 90.0, 0.0),
                scale=(1.0, 1.0, 1.0),
                asset_path=ASSET_PILLAR_HYDRANT,
            ),
            UtilityItem(
                category="street_cabinet",
                subtype="telecom",
                location=(15.0, 25.0, 0.15),
                rotation=(0.0, 180.0, 0.0),
                scale=(1.0, 1.0, 1.0),
                asset_path=ASSET_STREET_CABINET,
            ),
            UtilityItem(
                category="manhole_cover",
                subtype="drain",
                location=(12.0, 22.0, 0.0),
                asset_path=None,  # Belongs to surface cover mesh, not HISM
            ),
        ]

        hism = generate_utility_hism_placements(items)
        self.assertEqual(len(hism), 2, "Expected 2 distinct asset HISM groups")

        assets = {group["asset_path"]: group["transforms"] for group in hism}
        self.assertIn(ASSET_PILLAR_HYDRANT, assets)
        self.assertIn(ASSET_STREET_CABINET, assets)

        # Check pillar hydrant transform
        hyd_t = assets[ASSET_PILLAR_HYDRANT][0]
        self.assertEqual(hyd_t["location"], [10.0, 20.0, 0.15])
        self.assertEqual(hyd_t["rotation"], [0.0, 90.0, 0.0])
        self.assertEqual(hyd_t["scale"], [1.0, 1.0, 1.0])

    def test_03_cast_iron_surface_cover_mesh_geometry(self) -> None:
        """DoD 15.4: Verify watertight 3D geometry generation for circular manholes and rectangular DIN 4055 hydrants."""
        items = [
            UtilityItem(
                category="manhole_cover",
                subtype="sewer",
                location=(0.0, 0.0, 0.0),
                rotation=(0.0, 0.0, 0.0),
                diameter_mm=600,
            ),
            UtilityItem(
                category="fire_hydrant_underground",
                subtype="underground",
                location=(5.0, 5.0, 0.0),
                rotation=(0.0, 45.0, 0.0),
                diameter_mm=150,
            ),
        ]

        mesh = generate_utility_mesh_layers(items)
        self.assertEqual(mesh["num_circular_manholes"], 1)
        self.assertEqual(mesh["num_rectangular_hydrants"], 1)

        verts = mesh["vertices"]
        tris = mesh["triangles"]
        normals = mesh["normals"]

        self.assertGreater(len(verts), 30, "Expected vertices for circular manhole and rectangular plate")
        self.assertGreater(len(tris), 30, "Expected triangles forming watertight discs and skirts")
        self.assertEqual(len(verts), len(normals), "Every vertex must have a matching normal vector")

        # Verify elevation sits flush (+0.012m - +0.015m) above base surface to avoid z-fighting
        top_verts_z = [v[2] for v in verts if v[2] > 0.01]
        self.assertTrue(len(top_verts_z) > 0, "Ground plates must have slightly elevated top surface")
        for z in top_verts_z:
            self.assertAlmostEqual(z, 0.015, delta=0.005)

    def test_04_procedural_hydrant_synthesis(self) -> None:
        """DoD 15.5: Synthesize procedural pillar hydrants along corridors lacking fire protection (DIN 14461)."""
        if carla is None:
            self.skipTest("carla Python module required for procedural waypoint testing")

        # Mock carla map with 2 driving waypoints spaced 150m apart
        mock_map = MagicMock()
        mock_wp1 = MagicMock()
        mock_wp1.lane_type = carla.LaneType.Driving
        mock_wp1.lane_width = 3.5
        mock_wp1.transform.location = carla.Location(x=0.0, y=0.0, z=0.0)
        mock_wp1.transform.rotation = carla.Rotation(yaw=0.0)
        mock_wp1.get_right_lane.return_value = None

        mock_wp2 = MagicMock()
        mock_wp2.lane_type = carla.LaneType.Driving
        mock_wp2.lane_width = 3.5
        mock_wp2.transform.location = carla.Location(x=150.0, y=0.0, z=0.0)
        mock_wp2.transform.rotation = carla.Rotation(yaw=0.0)
        mock_wp2.get_right_lane.return_value = None

        mock_map.generate_waypoints.return_value = [mock_wp1, mock_wp2]
        mock_map.get_waypoint.return_value = None

        synth = synthesize_procedural_fire_hydrants(mock_map, existing_hydrants=[], spacing=100.0)
        self.assertEqual(len(synth), 2)
        for h in synth:
            self.assertEqual(h.category, "fire_hydrant_pillar")
            self.assertEqual(h.source, "procedural")
            self.assertEqual(h.asset_path, ASSET_PILLAR_HYDRANT)
            # Must sit to the right of lane heading
            self.assertGreater(h.location[1], 0.0)

    def test_05_navmesh_obstacles_and_spatial_clearances(self) -> None:
        """DoD 15.6: Verify 3D bounding box obstacle generation for Recast NavMesh and polygon clearances."""
        items = [
            UtilityItem(
                category="fire_hydrant_pillar",
                subtype="pillar",
                location=(10.0, 10.0, 0.0),
                rotation=(0.0, 0.0, 0.0),
                asset_path=ASSET_PILLAR_HYDRANT,
            ),
            UtilityItem(
                category="street_cabinet",
                subtype="telecom",
                location=(20.0, 20.0, 0.0),
                rotation=(0.0, 0.0, 0.0),
                asset_path=ASSET_STREET_CABINET,
            ),
            UtilityItem(
                category="manhole_cover",
                subtype="drain",
                location=(5.0, 5.0, 0.0),
            ),
        ]

        obs = generate_utility_obstacle_geometry(items)
        # Only pillar hydrants and cabinets are obstacle hulls (manhole covers are flush and walkable)
        self.assertEqual(len(obs), 2, "Only standing utilities must generate NavMesh obstacles")
        for verts, tris in obs:
            self.assertEqual(len(verts), 8, "Bounding box must have 8 vertices")
            self.assertEqual(len(tris), 12, "Bounding box must have 12 triangles")

        # Test building polygon containment
        square_bldg = [(0.0, 0.0), (10.0, 0.0), (10.0, 10.0), (0.0, 10.0)]
        self.assertTrue(point_in_polygon(5.0, 5.0, square_bldg))
        self.assertFalse(point_in_polygon(15.0, 5.0, square_bldg))

        # Test cycleway corridor proximity
        mock_cycleway = MagicMock()
        mock_cycleway.nodes = [(0.0, 0.0), (50.0, 0.0)]
        mock_cycleway.width_m = 2.0
        self.assertTrue(point_near_cycleway(25.0, 0.5, [mock_cycleway], clearance=0.3))
        self.assertFalse(point_near_cycleway(25.0, 3.0, [mock_cycleway], clearance=0.3))

    def test_06_bundle_compilation_inspect_and_verify(self) -> None:
        """DoD 15.7: Compile bundle with municipal utilities and verify via inspect and verify (Check 14)."""
        temp_bundle_dir = Path(tempfile.mkdtemp(prefix="carla_m15_test_"))
        tar_output = temp_bundle_dir / "MunichUtilitiesTest.carlamap.tar.gz"

        try:
            build_args = argparse.Namespace(
                target=str(temp_bundle_dir),
                name="MunichUtilitiesTest",
                output=str(tar_output),
                osm=str(SAMPLE_UTILITIES_OSM),
                xodr=None,
                meshes=None,
                layer_elevation=None,
                no_terrain=True,
                no_subbase=True,
                subbase_margin=50.0,
                no_clutter=True,
                no_parked_cars=True,
                max_parked_cars=0,
                no_lighting=True,
                no_traffic_lights=True,
                no_all_junctions_tl=True,
                no_crosswalks=True,
                no_vegetation=True,
                no_satellite_imagery=True,
                satellite_image=None,
                no_satellite_vegetation=True,
                no_satellite_parking=True,
                no_cycleways=True,
                cycleway_color="red",
                no_bike_boxes=True,
                no_bike_stencils=True,
                no_barriers=True,
                no_curbs=True,
                curb_material=None,
                no_utilities=False,
                no_procedural_hydrants=False,
                hydrant_spacing=100.0,
            )

            rc_build = cmd_build(build_args)
            self.assertEqual(rc_build, 0, "carla-map build failed on utilities test fixture")
            self.assertTrue(tar_output.is_file(), f"Bundle archive not created: {tar_output}")

            # Verify bundle contents
            with tarfile.open(tar_output, "r:gz") as tar:
                names = tar.getnames()
                self.assertTrue(any("carla-map.json" in n for n in names))
                self.assertTrue(any("Placements/utilities.json" in n for n in names))
                self.assertTrue(any("Meshes/osm_utility_covers.cmesh" in n for n in names))

            # Inspect
            inspect_args = argparse.Namespace(target=str(tar_output))
            rc_inspect = cmd_inspect(inspect_args)
            self.assertEqual(rc_inspect, 0, "carla-map inspect failed on utilities test bundle")

            # Verify (Check 14)
            verify_args = argparse.Namespace(target=str(tar_output))
            rc_verify = cmd_verify(verify_args)
            self.assertEqual(rc_verify, 0, "carla-map verify failed on check 14")

        finally:
            shutil.rmtree(temp_bundle_dir, ignore_errors=True)

    def test_07_munich_stachus_real_world_utilities(self) -> None:
        """DoD 15.8: Extract and verify real-world utilities from Munich Stachus OSM dataset."""
        if not MUNICH_OSM.is_file():
            self.skipTest(f"Munich OSM dataset not found at {MUNICH_OSM}")

        def dummy_proj(lat: float, lon: float) -> tuple[float, float, float]:
            return ((lon - 11.567) * 74000.0, -(lat - 48.140) * 111000.0, 0.0)

        items = extract_osm_utilities(MUNICH_OSM, dummy_proj)
        self.assertGreaterEqual(len(items), 50, "Expected at least 50 utility features in Munich Stachus")

        metrics = audit_utility_metrics(items)
        counts = metrics["counts"]
        self.assertGreater(counts["pillar_hydrants"], 0, "Expected authentic pillar hydrants in Munich")
        self.assertGreater(counts["underground_hydrants"], 20, "Expected abundant DIN underground hydrants")
        self.assertGreater(counts["street_cabinets"], 10, "Expected street distribution cabinets")
        self.assertGreater(counts["manhole_covers"], 15, "Expected manhole covers")

        # Operators
        operators = metrics["operators"]
        self.assertTrue(any("swm" in op.lower() or "stadtwerke" in op.lower() for op in operators))
        self.assertTrue(any("telekom" in op.lower() for op in operators))


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 15 E2E Test Suite")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument("--offline-only", action="store_true", help="Run only offline tests")
    args, unknown = parser.parse_known_args()

    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    suite.addTests(loader.loadTestsFromTestCase(TestMilestone15Utilities))

    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

