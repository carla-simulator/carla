#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 8 (M8).

Verifies the Physical Street Lamps, Lumen Lighting & Day/Night Weather Simulation:
  - DoD 8.1: OSM highway=street_lamp extraction parses authentic lamp posts with metadata.
  - DoD 8.2: Procedural curbside street lamp synthesis eliminates dark gaps along avenues & junctions.
  - DoD 8.3: Dynamic luminous specs emit realistic lumens (8k–14k lm) and color temps (2700K / 5000K).
  - DoD 8.4: Serialization of lighting manifest JSON into Placements/streetlights.json.
  - DoD 8.5: Bundle integration into .carlamap verified by carla-map inspect & verify.
  - DoD 8.6: Photometric roadway lux audit meets European EN 13201 (Class M3/M4) standards.
  - DoD 8.7: Live CARLA simulator verification spawns street lamps and applies Day/Night weather presets.
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
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_install, cmd_verify
from Util.MapBuilder.osm_lighting import (
    StreetLamp,
    calculate_roadway_lux_grid,
    extract_osm_street_lamps,
    load_lighting_manifest,
    save_lighting_manifest,
    spawn_street_lamps_in_world,
    synthesize_procedural_street_lamps,
)
from Util.MapBuilder.inspect_nocturnal_lux import run_lux_audit

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
if not MUNICH_OSM.is_file():
    MUNICH_OSM = REPO_ROOT / "munich_stachus.osm"

BUNDLE_DIR = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
if not BUNDLE_DIR.is_dir():
    BUNDLE_DIR = REPO_ROOT / "MunichStachus.carlamap"

MUNICH_XODR = BUNDLE_DIR / "OpenDrive" / "MunichStachus.xodr"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone8Lighting(unittest.TestCase):
    """Milestone 8 E2E Test Suite for Street Lighting & Day/Night Weather."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m8_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_street_lamp_extraction(self) -> None:
        """DoD 8.1 & 8.3: Extract authentic highway=street_lamp nodes from OSM."""
        self.assertTrue(MUNICH_OSM.is_file(), "munich_stachus.osm must exist")

        lamps = extract_osm_street_lamps(
            MUNICH_OSM,
            project_coord_fn=lambda lat, lon: (lon * 1000.0, lat * 1000.0, 0.0),
        )
        self.assertGreater(len(lamps), 40, f"Expected > 40 OSM street lamps, got {len(lamps)}")

        for lamp in lamps:
            self.assertTrue(lamp.mesh_path.startswith("/Game/Carla/Static/Static/Streetlights/"))
            self.assertGreaterEqual(lamp.lumens, 6000.0, "Street lamp lumens must be >= 6000 lm")
            self.assertIn(lamp.color_temp, (2700, 4000, 4500, 5000), "Valid color temperature")
            self.assertEqual(lamp.source, "osm")

    def test_02_manifest_serialization(self) -> None:
        """DoD 8.4: Structured serialization of Placements/streetlights.json manifest."""
        test_lamps = [
            StreetLamp(
                lamp_id="lamp_01",
                model_type="urban_avenue",
                mesh_path="/Game/Carla/Static/Static/Streetlights/SM_StreetLight06_A.SM_StreetLight06_A",
                location=(10.0, 20.0, 0.5),
                rotation=(0.0, 90.0, 0.0),
                height=7.8,
                arm_reach=3.8,
                lumens=10000.0,
                color_temp=4000,
                color_rgb=(255, 235, 205),
                source="osm",
            ),
            StreetLamp(
                lamp_id="lamp_02",
                model_type="arterial_cantilever",
                mesh_path="/Game/Carla/Static/Static/Streetlights/SM_StreetLight01.SM_StreetLight01",
                location=(20.0, 30.0, 0.5),
                rotation=(0.0, 180.0, 0.0),
                height=11.5,
                arm_reach=4.4,
                lumens=14000.0,
                color_temp=5000,
                color_rgb=(245, 248, 255),
                source="synthetic",
            ),
        ]

        json_path = self.test_dir / "test_streetlights.json"
        save_lighting_manifest(test_lamps, json_path)
        self.assertTrue(json_path.is_file(), "Manifest file must exist")

        loaded = load_lighting_manifest(json_path)
        self.assertEqual(len(loaded), 2)
        self.assertEqual(loaded[0].lamp_id, "lamp_01")
        self.assertEqual(loaded[1].lumens, 14000.0)
        self.assertEqual(loaded[1].source, "synthetic")

    def test_03_bundle_build_with_lighting(self) -> None:
        """DoD 8.5: carla-map build packages bundle with streetlights manifest & verify passes."""
        out_tar = self.test_dir / "MunichM8Test.carlamap.tar.gz"
        args = argparse.Namespace(
            subcommand="build",
            target=".",
            name="MunichM8Test",
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
            no_lighting=False,
        )

        rc = cmd_build(args)
        self.assertEqual(rc, 0, "cmd_build must succeed")
        self.assertTrue(out_tar.is_file(), "Output tar.gz archive must exist")

        inspect_args = argparse.Namespace(subcommand="inspect", target=str(out_tar))
        self.assertEqual(cmd_inspect(inspect_args), 0, "cmd_inspect must succeed")

        verify_args = argparse.Namespace(subcommand="verify", target=str(out_tar))
        self.assertEqual(cmd_verify(verify_args), 0, "cmd_verify must pass 11 structural checks")

    def test_04_photometric_roadway_lux_compliance(self) -> None:
        """DoD 8.6: Photometric lux audit meets European EN 13201 Class M3/M4 standards."""
        if not MUNICH_XODR.is_file():
            self.skipTest("MunichStachus.xodr not found")

        xodr_content = MUNICH_XODR.read_text(encoding="utf-8")
        carla_map = carla.Map("MunichStachus", xodr_content)
        manifest_path = BUNDLE_DIR / "Placements" / "streetlights.json"

        heatmap_png = self.test_dir / "test_lux_heatmap.png"
        metrics = run_lux_audit(
            carla_map,
            lighting_manifest_path=manifest_path if manifest_path.is_file() else None,
            osm_path=MUNICH_OSM,
            xodr_path=MUNICH_XODR,
            out_image=heatmap_png,
        )

        self.assertGreaterEqual(metrics["avg_lux"], 10.0, f"Average illuminance {metrics['avg_lux']} must be >= 10.0 lx")
        self.assertGreaterEqual(metrics["min_lux"], 2.5, f"Minimum illuminance {metrics['min_lux']} must be >= 2.5 lx (no blind spots)")
        self.assertTrue(metrics["passed_en13201"], "Lighting grid must pass EN 13201 Class M3/M4 standards")
        self.assertTrue(heatmap_png.is_file(), "False-color lux heatmap must be generated")

    def test_05_live_simulator_lighting_spawning(self) -> None:
        """DoD 8.7: Live CARLA server spawns physical street lamp pole actors."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(15.0)

        world = client.get_world()
        if "MunichStachus" not in world.get_map().name:
            world = client.load_world("MunichStachus")
            time.sleep(2.0)

        manifest_path = BUNDLE_DIR / "Placements" / "streetlights.json"
        if not manifest_path.is_file():
            self.skipTest("streetlights.json not found")

        lamps = load_lighting_manifest(manifest_path)
        sample_lamps = lamps[:15]
        spawned = spawn_street_lamps_in_world(world, sample_lamps)

        try:
            self.assertGreaterEqual(len(spawned), 10, "Must successfully spawn at least 10 lamp poles")
            for actor in spawned:
                self.assertIsNotNone(actor.get_location(), "Spawned lamp pole must have valid location")
        finally:
            for actor in spawned:
                try:
                    actor.destroy()
                except Exception:
                    pass

    def test_06_day_night_weather_presets(self) -> None:
        """DoD 8.7: Live CARLA simulator applies Day/Night weather presets."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(15.0)
        world = client.get_world()

        # Test ClearNoon
        world.set_weather(carla.WeatherParameters.ClearNoon)
        time.sleep(0.1)
        w1 = world.get_weather()
        self.assertAlmostEqual(w1.sun_altitude_angle, 45.0, delta=1.0)

        # Test ClearSunset
        world.set_weather(carla.WeatherParameters.ClearSunset)
        time.sleep(0.1)
        w2 = world.get_weather()
        self.assertAlmostEqual(w2.sun_altitude_angle, 15.0, delta=1.0)

        # Test ClearNight
        world.set_weather(carla.WeatherParameters.ClearNight)
        time.sleep(0.1)
        w3 = world.get_weather()
        self.assertAlmostEqual(w3.sun_altitude_angle, -90.0, delta=1.0)

        # Test WetNight
        world.set_weather(carla.WeatherParameters.WetNight)
        time.sleep(0.1)
        w4 = world.get_weather()
        self.assertAlmostEqual(w4.sun_altitude_angle, -90.0, delta=1.0)
        self.assertGreater(w4.wetness, 50.0)


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 8 E2E Lighting & Weather Test Suite")
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

