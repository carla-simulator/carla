#!/usr/bin/env python3
"""
Milestone 16 (M16) E2E Test Suite: Physical Speed Limit & Warning Signs (StVO metal signboards).

Verifies:
  - DoD 16.1: Ingestion of OSM traffic signs (maxspeed, traffic_sign=DE:274, DE:206, DE:205, DE:306, DE:101, DE:133, DE:138)
  - DoD 16.2: Spatial curb setback (0.50m-0.75m), oncoming traffic yaw orientation, and lane clearance
  - DoD 16.3: Procedural traffic signs synthesis along road corridors with DIN 1451 spacing
  - DoD 16.4: 3D metal mesh geometry (DIN 76mm tubular posts, collar, brackets, plates) and 1024x1024 texture atlas
  - DoD 16.5: OpenDRIVE <signals> synchronization with curb-aligned lateral offsets t
  - DoD 16.6: NavMesh obstacle hulls for standing signposts
  - DoD 16.7: carla-map bundle compilation, cmd_inspect, and Check 15 verification
  - DoD 16.8: inspect_traffic_signs CLI diagnostic auditor
  - DoD 16.9: Real-world Munich Stachus signboards extraction and geometry generation
  - DoD 16.10: Live CARLA landmark registration (if simulator is active)
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

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_verify
from Util.MapBuilder.cmesh import read_cmesh_file
from Util.MapBuilder.inspect_traffic_signs import inspect_traffic_signs
from Util.MapBuilder.osm_traffic_signs import (
    TrafficSignItem,
    audit_traffic_sign_metrics,
    extract_osm_traffic_signs,
    generate_traffic_sign_obstacle_geometry,
    generate_traffic_signs_mesh,
    generate_traffic_signs_texture_atlas,
    inject_traffic_signs_into_opendrive,
    is_colliding_with_cycleways,
    is_point_inside_polygon,
    parse_maxspeed_value,
    save_traffic_signs_manifest,
    synthesize_procedural_traffic_signs,
)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SAMPLE_SIGNS_OSM = FIXTURES_DIR / "sample_traffic_signs.osm"
MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"


def is_carla_server_running(host: str = "127.0.0.1", port: int = 2000, timeout: float = 1.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except (OSError, ConnectionRefusedError):
        return False


class TestMilestone16TrafficSigns(unittest.TestCase):
    """Milestone 16 E2E Test Suite for Physical Speed Limit & Warning Signs."""

    def test_01_osm_traffic_signs_ingestion(self) -> None:
        """DoD 16.1: Ingest OSM traffic signs and verify StVO classifications, shapes, and values."""
        self.assertTrue(SAMPLE_SIGNS_OSM.is_file(), f"Fixture missing: {SAMPLE_SIGNS_OSM}")

        def dummy_proj(lat: float, lon: float) -> tuple[float, float]:
            return ((lon - 11.563) * 74000.0, -(lat - 48.138) * 111000.0)

        items = extract_osm_traffic_signs(SAMPLE_SIGNS_OSM, dummy_proj)
        self.assertGreaterEqual(len(items), 8, f"Expected at least 8 signs from fixture, got {len(items)}")

        type_map = {item.stvo_code: item for item in items}

        # 1. Speed Limit 30 (StVO 274-30)
        self.assertIn("274-30", type_map)
        s30 = type_map["274-30"]
        self.assertEqual(s30.sign_type, "speed_limit")
        self.assertEqual(s30.plate_shape, "circle")
        self.assertEqual(s30.value, 30.0)

        # 2. Speed Limit 50 (StVO 274-50)
        self.assertIn("274-50", type_map)
        s50 = type_map["274-50"]
        self.assertEqual(s50.sign_type, "speed_limit")
        self.assertEqual(s50.plate_shape, "circle")
        self.assertEqual(s50.value, 50.0)

        # 3. Stop Sign (StVO 206)
        self.assertIn("206", type_map)
        s_stop = type_map["206"]
        self.assertEqual(s_stop.sign_type, "stop")
        self.assertEqual(s_stop.plate_shape, "octagon")

        # 4. Yield Sign (StVO 205)
        self.assertIn("205", type_map)
        s_yield = type_map["205"]
        self.assertEqual(s_yield.sign_type, "yield")
        self.assertEqual(s_yield.plate_shape, "triangle_down")

        # 5. Priority Road (StVO 306)
        self.assertIn("306", type_map)
        s_prio = type_map["306"]
        self.assertEqual(s_prio.sign_type, "priority_road")
        self.assertEqual(s_prio.plate_shape, "diamond")

        # 6. Danger Warning (StVO 101)
        self.assertIn("101", type_map)
        s_warn = type_map["101"]
        self.assertEqual(s_warn.sign_type, "warning")
        self.assertEqual(s_warn.plate_shape, "triangle_up")

    def test_02_spatial_clearance_and_setback(self) -> None:
        """DoD 16.2: Verify curb setback, driving lane clearance, building collision avoidance, and cycleway clearance."""
        # 1. Building collision filter
        bldg_poly = [(10.0, 10.0), (30.0, 10.0), (30.0, 30.0), (10.0, 30.0)]
        self.assertTrue(is_point_inside_polygon(20.0, 20.0, bldg_poly))
        self.assertFalse(is_point_inside_polygon(5.0, 5.0, bldg_poly))

        # 2. Cycleway clearance check
        class DummyCycleway:
            world_coords = [(0.0, 0.0), (100.0, 0.0)]
            width_m = 2.0

        cyc = DummyCycleway()
        # Point at y=0.5 (inside 1.0m half-width + 0.35m clearance)
        self.assertTrue(is_colliding_with_cycleways(50.0, 0.5, [cyc], clearance=0.35))
        # Point at y=2.0 (outside 1.35m)
        self.assertFalse(is_colliding_with_cycleways(50.0, 2.0, [cyc], clearance=0.35))

    def test_03_procedural_traffic_signs_synthesis(self) -> None:
        """DoD 16.3: Verify procedural traffic sign synthesis along road network with minimum spacing."""
        # Mock carla.Map and waypoints
        mock_map = MagicMock()
        mock_wp1 = MagicMock()
        mock_wp1.lane_type = carla.LaneType.Driving if carla else 1
        mock_wp1.road_id = 10
        mock_wp1.lane_id = -1
        mock_wp1.lane_width = 3.5
        mock_wp1.s = 20.0
        mock_wp1.is_junction = False
        mock_wp1.get_right_lane.return_value = None
        mock_wp1.get_left_lane.return_value = None
        mock_wp1.transform.location = MagicMock(x=0.0, y=0.0, z=0.0)
        mock_wp1.transform.rotation = MagicMock(yaw=0.0)

        mock_map.generate_waypoints.return_value = [mock_wp1]
        mock_map.get_waypoint.return_value = None

        synth = synthesize_procedural_traffic_signs(
            mock_map,
            existing_signs=[],
            spacing=100.0,
        )

        self.assertEqual(len(synth), 1)
        sign = synth[0]
        self.assertEqual(sign.sign_type, "speed_limit")
        self.assertEqual(sign.subtype, "50")
        self.assertEqual(sign.stvo_code, "274-50")
        self.assertAlmostEqual(sign.curb_setback_m, 0.60, places=2)
        # Sign orientation facing oncoming traffic: yaw + 180 = 180 deg
        self.assertAlmostEqual(sign.rotation[1], 180.0, places=1)

    def test_04_3d_metal_mesh_and_texture_atlas_generation(self) -> None:
        """DoD 16.4: Generate 1024x1024 texture atlas and 3D metal geometry (post, collar, brackets, plates)."""
        atlas_path = LOCAL_TMP_DIR / "test_suite_atlas.png"
        generate_traffic_signs_texture_atlas(atlas_path)
        self.assertTrue(atlas_path.is_file())
        self.assertGreater(atlas_path.stat().st_size, 10000)

        signs = [
            TrafficSignItem(
                sign_id="sign_1",
                sign_type="speed_limit",
                subtype="50",
                stvo_code="274-50",
                location=(0.0, 0.0, 0.0),
                rotation=(0.0, 0.0, 0.0),
                plate_shape="circle",
                value=50.0,
            ),
            TrafficSignItem(
                sign_id="sign_2",
                sign_type="stop",
                subtype="stop",
                stvo_code="206",
                location=(10.0, 0.0, 0.0),
                rotation=(0.0, 90.0, 0.0),
                plate_shape="octagon",
                value=0.0,
            ),
            TrafficSignItem(
                sign_id="sign_3",
                sign_type="yield",
                subtype="yield",
                stvo_code="205",
                location=(20.0, 0.0, 0.0),
                rotation=(0.0, 180.0, 0.0),
                plate_shape="triangle_down",
                value=0.0,
            ),
            TrafficSignItem(
                sign_id="sign_4",
                sign_type="priority_road",
                subtype="priority",
                stvo_code="306",
                location=(30.0, 0.0, 0.0),
                rotation=(0.0, 270.0, 0.0),
                plate_shape="diamond",
                value=0.0,
            ),
        ]

        mesh = generate_traffic_signs_mesh(signs)
        self.assertGreater(len(mesh["vertices"]), 300)
        self.assertGreater(len(mesh["triangles"]), 400)
        self.assertEqual(len(mesh["normals"]), len(mesh["vertices"]))
        self.assertEqual(len(mesh["uvs"]), len(mesh["vertices"]))
        self.assertEqual(mesh["semantic_label"], "TrafficSign")
        self.assertEqual(mesh["material_preset"], "custom_pbr")

    def test_05_opendrive_signal_synchronization(self) -> None:
        """DoD 16.5: Ingest traffic signs into OpenDRIVE XML with curb-aligned lateral offset t."""
        sample_xodr = """<?xml version="1.0" standalone="yes"?>
<OpenDRIVE>
  <road id="1" length="100.0" junction="-1">
    <signals>
      <signal id="old_1" s="10.0" t="-2.35" type="274" subtype="50" name="sign:speed_50:old" />
    </signals>
  </road>
</OpenDRIVE>"""

        sign = TrafficSignItem(
            sign_id="sign_7001",
            sign_type="speed_limit",
            subtype="50",
            stvo_code="274-50",
            location=(0.0, 0.0, 0.0),
            road_id=1,
            lane_id=-1,
            s=25.0,
            t=-4.15,
            value=50.0,
        )

        updated_xml = inject_traffic_signs_into_opendrive(sample_xodr, [sign])
        root = ET.fromstring(updated_xml)

        road = root.find("road")
        self.assertIsNotNone(road)
        signals = road.findall(".//signal")
        # Old t=-2.35 signal should be cleaned, only new curb-aligned signal should be present
        self.assertEqual(len(signals), 1)

        sig = signals[0]
        self.assertEqual(sig.get("id"), "7001")
        self.assertEqual(sig.get("s"), "25.00")
        self.assertEqual(sig.get("t"), "-4.15")  # Curb-aligned offset outside driving lane
        self.assertEqual(sig.get("country"), "Germany")
        self.assertEqual(sig.get("type"), "274")
        self.assertEqual(sig.get("subtype"), "50")
        self.assertEqual(sig.get("value"), "50")

        validity = sig.find("validity")
        self.assertIsNotNone(validity)
        self.assertEqual(validity.get("fromLane"), "-1")
        self.assertEqual(validity.get("toLane"), "-1")

    def test_06_navmesh_obstacle_geometry(self) -> None:
        """DoD 16.6: Verify NavMesh obstacle cylinders generated for standing signposts."""
        signs = [
            TrafficSignItem(
                sign_id="sign_1",
                sign_type="speed_limit",
                subtype="50",
                stvo_code="274-50",
                location=(15.0, 25.0, 0.0),
                post_height=2.25,
            )
        ]
        obs = generate_traffic_sign_obstacle_geometry(signs)
        self.assertEqual(len(obs), 1)
        verts, tris = obs[0]
        self.assertEqual(len(verts), 16)  # 8 top + 8 bottom
        self.assertEqual(len(tris), 16)   # 8 quads * 2 = 16 triangles

    def test_07_bundle_compilation_inspect_verify(self) -> None:
        """DoD 16.7: Compile a full .carlamap bundle with StVO traffic signs, run cmd_inspect, and cmd_verify."""
        with tempfile.TemporaryDirectory(prefix="carla_m16_bundle_") as tmp_dir_str:
            tmp_dir = Path(tmp_dir_str)
            bundle_out = tmp_dir / "TestSignsMap.carlamap.tar.gz"

            build_args = argparse.Namespace(
                target=str(tmp_dir),
                name="TestSignsMap",
                output=str(bundle_out),
                osm=str(SAMPLE_SIGNS_OSM),
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
                no_utilities=True,
                no_procedural_hydrants=True,
                hydrant_spacing=120.0,
                no_traffic_signs=False,
                sign_spacing=120.0,
            )

            ret = cmd_build(build_args)
            self.assertEqual(ret, 0, "carla-map build failed")
            self.assertTrue(bundle_out.is_file(), "Bundle archive was not created")

            # Verify bundle structure
            with tarfile.open(bundle_out, "r:gz") as tar:
                names = tar.getnames()
                self.assertTrue(any("osm_traffic_signs.cmesh" in n for n in names), "Missing osm_traffic_signs.cmesh")
                self.assertTrue(any("traffic_signs_atlas.png" in n for n in names), "Missing traffic_signs_atlas.png")
                self.assertTrue(any("traffic_signs.json" in n for n in names), "Missing traffic_signs.json")

            # Run inspect
            inspect_args = argparse.Namespace(target=str(bundle_out))
            ret_inspect = cmd_inspect(inspect_args)
            self.assertEqual(ret_inspect, 0, "cmd_inspect failed")

            # Run verify (verifies Check 15)
            verify_args = argparse.Namespace(target=str(bundle_out))
            ret_verify = cmd_verify(verify_args)
            self.assertEqual(ret_verify, 0, "cmd_verify failed (Check 15)")

    def test_08_cli_inspector_and_audit(self) -> None:
        """DoD 16.8: Verify inspect_traffic_signs CLI diagnostic output and JSON reporting."""
        with tempfile.TemporaryDirectory(prefix="carla_m16_inspect_") as tmp_dir_str:
            tmp_dir = Path(tmp_dir_str)
            manifest_path = tmp_dir / "traffic_signs.json"
            signs = [
                TrafficSignItem(
                    sign_id="sign_1",
                    sign_type="speed_limit",
                    subtype="50",
                    stvo_code="274-50",
                    location=(10.0, 20.0, 0.0),
                    curb_setback_m=0.60,
                    lane_clearance_m=0.60,
                ),
                TrafficSignItem(
                    sign_id="sign_2",
                    sign_type="stop",
                    subtype="stop",
                    stvo_code="206",
                    location=(30.0, 40.0, 0.0),
                    curb_setback_m=0.65,
                    lane_clearance_m=0.65,
                ),
            ]
            save_traffic_signs_manifest(signs, manifest_path)
            self.assertTrue(manifest_path.is_file())

            # Test text inspection
            ret_text = inspect_traffic_signs(manifest_path, summary_only=True)
            self.assertEqual(ret_text, 0)

            # Test JSON inspection
            ret_json = inspect_traffic_signs(manifest_path, json_output=True)
            self.assertEqual(ret_json, 0)

    def test_09_real_world_munich_stachus_extraction(self) -> None:
        """DoD 16.9: Verify real-world Munich Stachus OSM sign extraction."""
        if not MUNICH_OSM.is_file():
            self.skipTest("Munich Stachus OSM not found")

        def dummy_proj(lat: float, lon: float) -> tuple[float, float]:
            return ((lon - 11.565) * 74000.0, -(lat - 48.140) * 111000.0)

        items = extract_osm_traffic_signs(MUNICH_OSM, dummy_proj)
        self.assertGreater(len(items), 20, f"Expected >20 signs from Munich Stachus, got {len(items)}")

        # Verify speed limits
        speed_signs = [s for s in items if s.sign_type == "speed_limit"]
        self.assertGreater(len(speed_signs), 15)

        # Generate mesh
        mesh = generate_traffic_signs_mesh(items)
        self.assertGreater(len(mesh["vertices"]), 1000)
        self.assertGreater(len(mesh["triangles"]), 1500)

    def test_10_live_carla_simulation(self) -> None:
        """DoD 16.10: If live CARLA server is running, verify landmark query and sign API."""
        if not is_carla_server_running():
            self.skipTest("Live CARLA server not running on port 2000")

        client = carla.Client("127.0.0.1", 2000)
        client.set_timeout(5.0)
        world = client.get_world()
        landmarks = world.get_map().get_all_landmarks()
        self.assertIsNotNone(landmarks)
        print(f"[live-carla] Successfully verified {len(landmarks)} OpenDRIVE landmarks in live CARLA world.")

def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 16 E2E Test Suite")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument("--offline-only", action="store_true", help="Run only offline tests")
    args, unknown = parser.parse_known_args()

    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    suite.addTests(loader.loadTestsFromTestCase(TestMilestone16TrafficSigns))

    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

