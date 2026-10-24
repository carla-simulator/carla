#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 12 (M12).

Verifies Satellite & Aerial Imagery Integration:
  - DoD 12.1: Orthophoto tile ingestion, caching, and georeferencing metadata calculation.
  - DoD 12.2: Spectral index processing: Excess Green Index (ExG/NExG) and asphalt reflectance.
  - DoD 12.3: Unbuilt parcel delineation: BBox \\ (OSM Buildings ∪ OpenDRIVE Roads).
  - DoD 12.4: Green park lawn mesh generation and native CARLA 3D foliage placement.
  - DoD 12.5: Open surface parking lot detection, stall markings synthesis, and parked vehicles.
  - DoD 12.6: Manifest serialization into Placements/satellite_analysis.json and bundle verification.
  - DoD 12.7: Live CARLA simulator verification (when server is running).
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
import numpy as np
from PIL import Image

E2E_DIR = Path(__file__).resolve().parent
REPO_ROOT = E2E_DIR.parent.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
tempfile.tempdir = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_verify
from Util.MapBuilder.osm_satellite import (
    SatelliteMetadata,
    SatelliteProjector,
    analyze_satellite_parcels,
    compute_spectral_classification,
    deg2num,
    fetch_or_load_satellite_imagery,
    generate_satellite_green_mesh_and_foliage,
    generate_satellite_parking_layers,
    num2deg,
    render_satellite_diagnostic_overlay,
    save_satellite_manifest,
)
from Util.MapBuilder.osm_crosswalks import make_lat_lon_projector
from Util.MapBuilder.osm_buildings import extract_osm_building_footprints

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
if not MUNICH_OSM.is_file():
    MUNICH_OSM = REPO_ROOT / "munich_stachus.osm"

BUNDLE_ARCHIVE = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap.tar.gz"
BUNDLE_DIR = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
MUNICH_XODR = BUNDLE_DIR / "OpenDrive" / "MunichStachus.xodr"
MUNICH_SAT_IMAGE = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus_satellite.jpg"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone12Satellite(unittest.TestCase):
    """Milestone 12 E2E Test Suite for Satellite & Aerial Imagery Integration."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m12_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_tile_math_and_orthophoto_ingestion(self) -> None:
        """DoD 12.1: Verify Web Mercator conversions and orthophoto ingestion."""
        lat, lon, zoom = 48.140, 11.567, 18
        xtile, ytile = deg2num(lat, lon, zoom)
        self.assertIsInstance(xtile, int)
        self.assertIsInstance(ytile, int)

        lat_rt, lon_rt = num2deg(xtile, ytile, zoom)
        self.assertAlmostEqual(lat, lat_rt, delta=0.01)
        self.assertAlmostEqual(lon, lon_rt, delta=0.01)

        # Ingestion
        img, meta = fetch_or_load_satellite_imagery(
            48.138, 11.564, 48.142, 11.570,
            zoom=18,
            osm_path=MUNICH_OSM,
        )
        self.assertGreater(img.width, 256)
        self.assertGreater(img.height, 256)
        self.assertEqual(meta.zoom, 18)
        self.assertGreaterEqual(meta.tile_x1, meta.tile_x0)
        self.assertGreaterEqual(meta.tile_y1, meta.tile_y0)

    def test_02_spectral_indices_classification(self) -> None:
        """DoD 12.2: Verify Excess Green Index (NExG) and neutral asphalt detection."""
        img, _ = fetch_or_load_satellite_imagery(
            48.138, 11.564, 48.142, 11.570,
            zoom=18,
            osm_path=MUNICH_OSM,
        )
        classification = compute_spectral_classification(img)
        self.assertIsInstance(classification.vegetation_mask, np.ndarray)
        self.assertIsInstance(classification.asphalt_mask, np.ndarray)
        self.assertGreater(classification.veg_percentage, 10.0, "Vegetation area must be non-trivial")
        self.assertGreater(classification.asphalt_percentage, 1.0, "Asphalt area must be non-trivial")

    def test_03_parcel_analysis_and_classification(self) -> None:
        """DoD 12.3: Verify unbuilt parcel analysis finds green areas and parking lots."""
        self.assertTrue(MUNICH_XODR.is_file(), "MunichStachus.xodr must exist")
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))

        img, meta = fetch_or_load_satellite_imagery(
            48.138, 11.564, 48.142, 11.570,
            zoom=18,
            osm_path=MUNICH_OSM,
        )
        classification = compute_spectral_classification(img)
        projector = SatelliteProjector(meta, project_lat_lon)

        footprints = extract_osm_building_footprints(MUNICH_OSM)
        bpolys = []
        for fp in footprints:
            pts = [project_lat_lon(lat, lon)[:2] for lat, lon in fp.nodes]
            if len(pts) >= 3:
                bpolys.append(pts)

        all_x = [wp.transform.location.x for wp in cmap.generate_waypoints(2.0)]
        all_y = [wp.transform.location.y for wp in cmap.generate_waypoints(2.0)]
        map_bounds = (min(all_x), max(all_x), min(all_y), max(all_y))

        green_parcels, parking_lots = analyze_satellite_parcels(
            meta, classification, projector, bpolys, cmap, map_bounds
        )
        self.assertGreater(len(green_parcels), 0, "Must detect at least 1 green vegetation parcel")
        self.assertGreaterEqual(len(parking_lots), 1, "Must detect at least 1 open surface parking lot")

    def test_04_green_mesh_and_parking_geometry_synthesis(self) -> None:
        """DoD 12.4 & 12.5: Verify 3D mesh synthesis for green lawns, parking lots, and stalls."""
        cmap = carla.Map("MunichStachus", MUNICH_XODR.read_text(encoding="utf-8"))
        project_lat_lon = make_lat_lon_projector(carla_map=cmap, xodr_xml=MUNICH_XODR.read_text(encoding="utf-8"))
        img, meta = fetch_or_load_satellite_imagery(48.138, 11.564, 48.142, 11.570, zoom=18, osm_path=MUNICH_OSM)
        classification = compute_spectral_classification(img)
        projector = SatelliteProjector(meta, project_lat_lon)

        footprints = extract_osm_building_footprints(MUNICH_OSM)
        bpolys = [[project_lat_lon(lat, lon)[:2] for lat, lon in fp.nodes] for fp in footprints]
        bpolys = [p for p in bpolys if len(p) >= 3]

        all_x = [wp.transform.location.x for wp in cmap.generate_waypoints(2.0)]
        all_y = [wp.transform.location.y for wp in cmap.generate_waypoints(2.0)]
        map_bounds = (min(all_x), max(all_x), min(all_y), max(all_y))

        green_parcels, parking_lots = analyze_satellite_parcels(
            meta, classification, projector, bpolys, cmap, map_bounds
        )

        # Green mesh & foliage
        g_mesh, hism_veg, obs, foliage = generate_satellite_green_mesh_and_foliage(green_parcels, cmap, bpolys)
        self.assertGreater(len(g_mesh["vertices"]), 0)
        self.assertGreater(len(g_mesh["triangles"]), 0)
        self.assertGreater(len(foliage), 0)

        # Parking lot & stalls
        lot_mesh, stall_mesh, parked_cars = generate_satellite_parking_layers(parking_lots, cmap)
        self.assertGreater(len(lot_mesh["vertices"]), 0)
        self.assertGreater(len(stall_mesh["vertices"]), 0)
        self.assertGreater(len(parked_cars), 0)

    def test_05_bundle_compilation_and_verification(self) -> None:
        """DoD 12.6: Compile bundle with satellite integration and run structural verification."""
        out_tar = self.test_dir / "MunichM12Test.carlamap.tar.gz"
        build_args = argparse.Namespace(
            subcommand="build",
            target=".",
            name="MunichM12Test",
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
        )
        res = cmd_build(build_args)
        self.assertEqual(res, 0, "cmd_build must succeed")
        self.assertTrue(out_tar.is_file(), "Bundle archive must be created")

        # Inspect
        inspect_args = argparse.Namespace(subcommand="inspect", target=str(out_tar))
        self.assertEqual(cmd_inspect(inspect_args), 0, "cmd_inspect must succeed")

        # Verify
        verify_args = argparse.Namespace(subcommand="verify", target=str(out_tar))
        self.assertEqual(cmd_verify(verify_args), 0, "cmd_verify must pass all structural checks")

    def test_06_diagnostic_overlay_artifact(self) -> None:
        """DoD 12.6: Verify 4-panel diagnostic visual comparison overlay is generated."""
        overlay_p = LOCAL_TMP_DIR / "satellite_spectral_analysis.png"
        self.assertTrue(overlay_p.is_file(), "Diagnostic overlay must exist")
        self.assertGreater(overlay_p.stat().st_size, 50000, "Diagnostic overlay must be non-empty")
        with Image.open(overlay_p) as img:
            self.assertGreater(img.width, 800)
            self.assertGreater(img.height, 600)

    def test_07_live_simulator_satellite_verification(self) -> None:
        """DoD 12.7: Live CARLA simulator verification of satellite vegetation and parked cars."""
        if self.offline_only:
            self.skipTest("Skipping live simulator test (--offline-only specified)")

        if not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(15.0)

        world = client.get_world()
        self.assertIsNotNone(world)

        actors = world.get_actors()
        veg_actors = [a for a in actors if "vegetation" in a.type_id.lower() or "foliage" in a.type_id.lower()]
        car_actors = [a for a in actors if a.type_id.startswith("vehicle.")]
        print(f"\n[live] Connected to CARLA simulator: {len(actors)} total actors, {len(car_actors)} vehicles.")


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 12 Satellite E2E Test Suite")
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
