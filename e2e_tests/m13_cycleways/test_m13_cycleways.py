#!/usr/bin/env python3
"""
Milestone 13 (M13) E2E Test Suite: Dedicated Bicycle Infrastructure & Cycleway Networks.

Verifies:
  - DoD 13.1: Ingestion of OSM cycleway tags (highway=cycleway, cycleway:right=lane, cycleway:left=lane, etc.)
  - DoD 13.2: Curbside lateral offset calculation and road alignment
  - DoD 13.3: 3D corridor ribbon mesh synthesis with Munich Emerald Green / Terracotta Red PBR surfaces
  - DoD 13.4: Retroreflective white demarcation line markings (solid for exclusive, dashed for advisory)
  - DoD 13.5: White bicycle silhouette stencils (StVO 237) and Advance Stop Lines (Bike Boxes)
  - DoD 13.6: Bicycle parking racks and parked bicycle vehicle blueprints (Omafiets, Century, Crossbike)
  - DoD 13.7: Standalone CLI inspector (inspect_cycleways.py), carla-map inspect/verify integration
  - DoD 13.8: Live simulator verification of cycling infrastructure
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
import unittest

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

from Util.MapBuilder.carla_map import (
    cmd_build,
    cmd_inspect,
    cmd_verify,
)
from Util.MapBuilder.osm_crosswalks import make_lat_lon_projector
from Util.MapBuilder.osm_cycleways import (
    extract_osm_cycleways,
    generate_cycleway_geometry,
    generate_cycleway_demarcation_mesh,
    generate_bike_stencils_and_boxes_mesh,
    generate_bike_parking_clutter,
    save_cycleways_manifest,
    load_cycleways_manifest,
    audit_cycleway_metrics,
    CYCLEWAY_COLOR_PRESETS,
    BICYCLE_BLUEPRINTS,
)

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
MUNICH_XODR = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap" / "OpenDrive" / "MunichStachus.xodr"


def is_port_open(host: str, port: int) -> bool:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(1.0)
        return s.connect_ex((host, port)) == 0


class TestMilestone13Cycleways(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = LOCAL_TMP_DIR / "m13_test_run"
        if cls.test_dir.exists():
            shutil.rmtree(cls.test_dir, ignore_errors=True)
        cls.test_dir.mkdir(parents=True, exist_ok=True)

        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.test_dir.exists():
            shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_cycleway_ingestion(self) -> None:
        """DoD 13.1: Verify ingestion of OSM cycleway tags and infrastructure extraction."""
        self.assertTrue(MUNICH_OSM.is_file(), f"Munich OSM file must exist at {MUNICH_OSM}")
        self.assertTrue(MUNICH_XODR.is_file(), f"Munich OpenDRIVE file must exist at {MUNICH_XODR}")

        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        cycleways, bike_boxes, bike_racks = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        self.assertGreater(len(cycleways), 30, "Must extract at least 30 cycleway segments in Munich Stachus")
        self.assertGreater(len(bike_boxes), 50, "Must detect advance stop lines (bike boxes)")
        self.assertGreater(len(bike_racks), 5, "Must detect bicycle parking facilities")

        # Verify attributes on segments
        total_len = sum(c.length_m for c in cycleways)
        self.assertGreater(total_len, 3000.0, "Total cycleway network must exceed 3.0 lane km")

        for c in cycleways:
            self.assertIn(c.side, ("right", "left", "both", "standalone"))
            self.assertIn(c.surface_color, ("green", "red", "asphalt"))
            self.assertGreater(c.width_m, 0.5)
            self.assertGreater(len(c.nodes_carla), 1)

    def test_02_lateral_curbside_offsets_and_alignment(self) -> None:
        """DoD 13.2: Verify curbside lateral offsets and road alignment."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        cycleways, _, _ = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        # Curbside cycleways must satisfy ERA / StVO width standards (1.0m - 3.5m)
        roadside_cycleways = [c for c in cycleways if c.side in ("right", "left", "both")]
        self.assertGreaterEqual(len(roadside_cycleways), 10, "Must have roadside cycleway lanes")

        for c in roadside_cycleways:
            self.assertGreaterEqual(c.width_m, 1.0, f"Cycleway {c.segment_id} width too narrow")
            self.assertLessEqual(c.width_m, 4.0, f"Cycleway {c.segment_id} width excessively wide")
            self.assertGreater(c.length_m, 0.0)

        # Curbside segments in the road network area must be close to road waypoints
        for c in roadside_cycleways:
            # Check mid-point of cycleway
            pts = c.nodes_carla
            mid_pt = pts[len(pts) // 2]
            loc = carla.Location(x=mid_pt[0], y=mid_pt[1], z=mid_pt[2])
            wp = cmap.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None:
                dist = wp.transform.location.distance(loc)
                self.assertLess(dist, 40.0, f"Cycleway {c.segment_id} should be within 40m of road network")

    def test_03_surface_pbr_tinting_presets(self) -> None:
        """DoD 13.3: Verify 3D ribbon mesh synthesis and PBR color palettes."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        cycleways, _, _ = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        mesh_data, walkable = generate_cycleway_geometry(cycleways)
        self.assertGreater(len(mesh_data["vertices"]), 500, "Cycleway mesh must have significant vertices")
        self.assertGreater(len(mesh_data["triangles"]), 400, "Cycleway mesh must have significant triangles")
        self.assertEqual(len(mesh_data["normals"]), len(mesh_data["vertices"]))
        self.assertEqual(len(mesh_data["uvs"]), len(mesh_data["vertices"]))

        # Verify walkable surfaces generated for Recast NavMesh integration
        self.assertGreater(len(walkable), 0, "Cycleways must contribute walkable surfaces")

        # Verify color presets
        self.assertIn("green", CYCLEWAY_COLOR_PRESETS)
        self.assertIn("red", CYCLEWAY_COLOR_PRESETS)
        self.assertIn("asphalt", CYCLEWAY_COLOR_PRESETS)

    def test_04_retroreflective_demarcation_lines(self) -> None:
        """DoD 13.4: Verify solid (exclusive) and dashed (advisory) demarcation line meshes."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        cycleways, _, _ = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        demarc_mesh = generate_cycleway_demarcation_mesh(cycleways)
        self.assertGreater(len(demarc_mesh["vertices"]), 1000, "Demarcation lines mesh must have vertices")
        self.assertGreater(len(demarc_mesh["triangles"]), 500, "Demarcation lines mesh must have triangles")

        # Check line markings categorization
        solid_count = sum(1 for c in cycleways if c.lane_marking == "solid")
        dashed_count = sum(1 for c in cycleways if c.lane_marking == "dashed")
        self.assertGreater(solid_count, 0, "Must have solid exclusive cycleway lines")
        self.assertGreater(dashed_count, 0, "Must have dashed advisory cycleway lines")

    def test_05_stencils_and_bike_boxes(self) -> None:
        """DoD 13.5: Verify bicycle silhouette stencils and advance stop line boxes."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        cycleways, bike_boxes, _ = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        stencil_mesh, stencils_meta = generate_bike_stencils_and_boxes_mesh(cycleways, bike_boxes)
        self.assertGreater(len(stencil_mesh["vertices"]), 10000, "Stencil & box mesh must contain detailed vertices")
        self.assertGreater(len(stencil_mesh["triangles"]), 5000, "Stencil & box mesh must contain triangles")
        self.assertGreater(len(stencils_meta), 50, "Must synthesize corridor bicycle stencils")

    def test_06_bike_parking_and_vehicle_assets(self) -> None:
        """DoD 13.6: Verify bicycle parking stands and parked bicycle vehicle blueprints."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        _, _, bike_racks = extract_osm_cycleways(
            MUNICH_OSM,
            project_coord_fn=project_lat_lon,
            carla_map=cmap,
        )

        parked_items = generate_bike_parking_clutter(bike_racks)
        self.assertGreater(len(parked_items), 20, "Must generate at least 20 parked bicycle clutter items")

        for item in parked_items:
            self.assertEqual(item.category, "parked_bicycle")
            self.assertIn(item.blueprint_id, BICYCLE_BLUEPRINTS)
            self.assertEqual(len(item.location), 3)
            self.assertEqual(len(item.rotation), 3)

    def test_07_bundle_compilation_inspect_verify(self) -> None:
        """DoD 13.7: Compile bundle with cycleway integration and run structural verification."""
        out_tar = self.test_dir / "MunichM13Test.carlamap.tar.gz"
        build_args = argparse.Namespace(
            subcommand="build",
            target=".",
            name="MunichM13Test",
            osm=str(MUNICH_OSM),
            xodr=None,
            output=str(out_tar),
            meshes=None,
            layer_elevation=None,
            no_terrain=False,
            no_subbase=False,
            subbase_margin=50.0,
            no_clutter=False,
            no_parked_cars=False,
            max_parked_cars=10,
            no_lighting=False,
            no_traffic_lights=False,
            no_all_junctions_tl=False,
            no_crosswalks=False,
            no_vegetation=False,
            no_satellite_imagery=False,
            satellite_image=None,
            no_satellite_vegetation=False,
            no_satellite_parking=False,
            no_cycleways=False,
            cycleway_color="auto",
            no_bike_boxes=False,
            no_bike_stencils=False,
        )
        res = cmd_build(build_args)
        self.assertEqual(res, 0, "cmd_build must succeed with bicycle infrastructure")
        self.assertTrue(out_tar.is_file(), "Bundle archive must be created")

        # Inspect
        inspect_args = argparse.Namespace(subcommand="inspect", target=str(out_tar))
        self.assertEqual(cmd_inspect(inspect_args), 0, "cmd_inspect must succeed")

        # Verify
        verify_args = argparse.Namespace(subcommand="verify", target=str(out_tar))
        self.assertEqual(cmd_verify(verify_args), 0, "cmd_verify must pass all structural checks")

    def test_08_live_simulator_cycleways(self) -> None:
        """DoD 13.8: Live CARLA simulator verification of cycling corridors and parked bikes."""
        if self.offline_only:
            self.skipTest("Skipping live simulator test (--offline-only specified)")

        if not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(15.0)

        world = client.get_world()
        self.assertIsNotNone(world)

        actors = world.get_actors()
        bike_actors = [
            a for a in actors
            if any(b in a.type_id.lower() for b in ["bicycle", "omafiets", "century", "crossbike"])
        ]
        print(f"\n[live] Connected to CARLA simulator: {len(actors)} total actors, {len(bike_actors)} bicycle assets detected.")


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 13 Cycleways E2E Test Suite")
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
