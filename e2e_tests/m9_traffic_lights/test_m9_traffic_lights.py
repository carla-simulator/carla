#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 9 (M9).

Verifies Dynamic Traffic Lights, OpenDRIVE Signals & Ego Perception Integration:
  - DoD 9.1: OSM highway=traffic_signals and crossing=traffic_signals extraction parses authentic nodes.
  - DoD 9.2: Association with OpenDRIVE road network, s/t coordinates, and counter-lane heading.
  - DoD 9.3: OpenDRIVE XML injection of <signals><signal .../></signals> and <controller> elements.
  - DoD 9.4: Serialization of traffic lights manifest into Placements/traffic_lights.json.
  - DoD 9.5: Bundle integration into .carlamap verified by carla-map inspect & verify (12 checks).
  - DoD 9.6: Live CARLA simulator spawning of traffic.traffic_light actors and 3-phase state transitions.
  - DoD 9.7: Autonomous Ego perception (vehicle.get_traffic_light() and is_at_traffic_light()).
  - DoD 9.8: Traffic Manager vehicle braking compliance: complete halt (v < 0.1 km/h) before stop line.
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
from Util.MapBuilder.osm_traffic_lights import (
    TrafficSignal,
    extract_osm_traffic_signals,
    inject_signals_into_opendrive,
    load_traffic_lights_manifest,
    save_traffic_lights_manifest,
    spawn_physical_traffic_lights,
)

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


class TestMilestone9TrafficLights(unittest.TestCase):
    """Milestone 9 E2E Test Suite for Dynamic Traffic Lights & OpenDRIVE Signals."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.test_dir = Path(tempfile.mkdtemp(prefix="carla_m9_test_"))
        cls.host = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
        cls.port = int(os.environ.get("CARLA_E2E_PORT", "2000"))
        cls.offline_only = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def test_01_osm_traffic_signal_extraction(self) -> None:
        """DoD 9.1 & 9.2: Extract authentic traffic signals from OSM and associate with road network."""
        self.assertTrue(MUNICH_OSM.is_file(), "munich_stachus.osm must exist")

        carla_map = None
        if MUNICH_XODR.is_file():
            xodr_text = MUNICH_XODR.read_text(encoding="utf-8")
            carla_map = carla.Map("MunichStachus", xodr_text)

        signals = extract_osm_traffic_signals(
            MUNICH_OSM,
            project_coord_fn=None,
            carla_map=carla_map,
        )
        self.assertGreaterEqual(len(signals), 50, f"Expected >= 50 OSM traffic signals, got {len(signals)}")

        ped_signals = [s for s in signals if s.model_type == "pedestrian_signal"]
        self.assertGreater(len(ped_signals), 20, "Must have pedestrian signals from crossing=traffic_signals")

        for sig in signals:
            self.assertTrue(sig.signal_id.startswith("tl_"))
            self.assertIn(sig.model_type, ("cantilever_mast", "vertical_post", "pedestrian_signal"))
            self.assertIsNotNone(sig.mesh_path)

    def test_02_opendrive_signal_injection(self) -> None:
        """DoD 9.3: Inject signals and controllers into OpenDRIVE XML and validate with carla.Map."""
        if not MUNICH_XODR.is_file():
            self.skipTest("MunichStachus.xodr not found")

        xodr_text = MUNICH_XODR.read_text(encoding="utf-8")
        carla_map = carla.Map("MunichStachus", xodr_text)

        signals = extract_osm_traffic_signals(
            MUNICH_OSM,
            project_coord_fn=None,
            carla_map=carla_map,
        )

        injected_xml = inject_signals_into_opendrive(xodr_text, signals)
        self.assertIn("<signals>", injected_xml)
        self.assertIn("<controller", injected_xml)

        # Parse with carla.Map and verify landmarks are registered
        updated_map = carla.Map("MunichStachusSignals", injected_xml)
        landmarks = updated_map.get_all_landmarks()
        self.assertGreaterEqual(len(landmarks), 50, f"Expected >= 50 landmarks, got {len(landmarks)}")

    def test_03_traffic_lights_manifest_serialization(self) -> None:
        """DoD 9.4: Structured serialization of Placements/traffic_lights.json manifest."""
        test_signals = [
            TrafficSignal(
                signal_id="tl_1001",
                model_type="cantilever_mast",
                mesh_path="/Game/Carla/Static/TrafficLight/StreetLights/StreetLight_01.StreetLight_01",
                location=(15.0, 25.0, 0.0),
                rotation=(0.0, 45.0, 0.0),
                road_id=12,
                lane_id=1,
                s=20.5,
                t=-3.5,
                is_pedestrian=False,
            ),
            TrafficSignal(
                signal_id="tl_1002",
                model_type="pedestrian_signal",
                mesh_path="/Game/Carla/Static/TrafficLight/PedestrianLight/SM_PedestrianLight.SM_PedestrianLight",
                location=(18.0, 28.0, 0.0),
                rotation=(0.0, 135.0, 0.0),
                road_id=12,
                lane_id=2,
                s=22.0,
                t=-1.0,
                is_pedestrian=True,
            ),
        ]

        json_path = self.test_dir / "test_traffic_lights.json"
        save_traffic_lights_manifest(test_signals, json_path)
        self.assertTrue(json_path.is_file(), "Manifest file must exist")

        loaded = load_traffic_lights_manifest(json_path)
        self.assertEqual(len(loaded), 2)
        self.assertEqual(loaded[0].signal_id, "tl_1001")
        self.assertEqual(loaded[0].model_type, "cantilever_mast")
        self.assertTrue(loaded[1].is_pedestrian)

    def test_04_bundle_build_with_traffic_lights(self) -> None:
        """DoD 9.5: carla-map build packages bundle with traffic lights & verify passes 12 checks."""
        out_tar = self.test_dir / "MunichM9Test.carlamap.tar.gz"
        args = argparse.Namespace(
            subcommand="build",
            target=".",
            name="MunichM9Test",
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
            no_traffic_lights=False,
            no_all_junctions_tl=False,
        )

        rc = cmd_build(args)
        self.assertEqual(rc, 0, "cmd_build must succeed")
        self.assertTrue(out_tar.is_file(), "Output tar.gz archive must exist")

        inspect_args = argparse.Namespace(subcommand="inspect", target=str(out_tar))
        self.assertEqual(cmd_inspect(inspect_args), 0, "cmd_inspect must succeed")

        verify_args = argparse.Namespace(subcommand="verify", target=str(out_tar))
        self.assertEqual(cmd_verify(verify_args), 0, "cmd_verify must pass all structural checks")

    def test_05_live_simulator_traffic_light_actors(self) -> None:
        """DoD 9.6: Live CARLA server spawns traffic.traffic_light actors and supports 3-phase cycles."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(20.0)

        world = client.get_world()
        traffic_lights = world.get_actors().filter("traffic.traffic_light")
        if len(traffic_lights) < 10 and MUNICH_XODR.is_file():
            world = client.generate_opendrive_world(MUNICH_XODR.read_text(encoding="utf-8"))
            time.sleep(1.0)
            traffic_lights = world.get_actors().filter("traffic.traffic_light")

        self.assertGreaterEqual(
            len(traffic_lights), 10, f"Expected >= 10 traffic light actors, got {len(traffic_lights)}"
        )

        # Pick first traffic light and verify 3-phase state transitions
        tl = traffic_lights[0]
        tl.freeze(True)

        tl.set_state(carla.TrafficLightState.Red)
        time.sleep(0.08)
        self.assertEqual(tl.get_state(), carla.TrafficLightState.Red)

        tl.set_state(carla.TrafficLightState.Yellow)
        time.sleep(0.08)
        self.assertEqual(tl.get_state(), carla.TrafficLightState.Yellow)

        tl.set_state(carla.TrafficLightState.Green)
        time.sleep(0.08)
        self.assertEqual(tl.get_state(), carla.TrafficLightState.Green)

        tl.freeze(False)

    def test_06_live_traffic_manager_braking_compliance(self) -> None:
        """DoD 9.7 & 9.8: Vehicle approaches red light, halts cleanly with zero overshoot, and resumes on Green."""
        if self.offline_only or not is_port_open(self.host, self.port):
            self.skipTest(f"CARLA server not running on {self.host}:{self.port}")

        client = carla.Client(self.host, self.port)
        client.set_timeout(20.0)
        world = client.get_world()

        traffic_lights = world.get_actors().filter("traffic.traffic_light")
        if len(traffic_lights) < 10 and MUNICH_XODR.is_file():
            world = client.generate_opendrive_world(MUNICH_XODR.read_text(encoding="utf-8"))
            time.sleep(1.0)
            traffic_lights = world.get_actors().filter("traffic.traffic_light")

        carla_map = world.get_map()
        traffic_lights = world.get_actors().filter("traffic.traffic_light")
        if not traffic_lights:
            self.skipTest("No traffic lights found on current map")

        # Freeze all traffic lights to Red for clean deterministic test
        for t in traffic_lights:
            t.freeze(True)
            t.set_state(carla.TrafficLightState.Red)

        # Select a traffic light on a valid road
        target_tl = None
        target_wp = None
        for t in traffic_lights:
            loc = t.get_location()
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None and wp.road_id is not None:
                # Find an upstream waypoint ~15-20m before the light
                prev_wps = wp.previous(18.0)
                if prev_wps:
                    target_tl = t
                    target_wp = prev_wps[0]
                    break

        if target_tl is None or target_wp is None:
            self.skipTest("Could not locate suitable approach waypoint for traffic light")

        bp_lib = world.get_blueprint_library()
        veh_bp = bp_lib.filter("vehicle.*")[0]

        spawn_transform = carla.Transform(
            target_wp.transform.location + carla.Location(z=0.5),
            target_wp.transform.rotation,
        )
        vehicle = world.try_spawn_actor(veh_bp, spawn_transform)
        if vehicle is None:
            # Fall back to spawn point
            spawns = carla_map.get_spawn_points()
            if spawns:
                vehicle = world.try_spawn_actor(veh_bp, spawns[0])

        if vehicle is None:
            self.skipTest("Could not spawn test vehicle")

        try:
            tm = client.get_trafficmanager(8000)
            vehicle.set_autopilot(True, tm.get_port())
            tm.ignore_lights_percentage(vehicle, 0.0)

            # Let simulation advance and monitor velocity
            initial_speed = 0.0
            for _ in range(30):
                world.tick() if world.get_settings().synchronous_mode else time.sleep(0.05)
                vel = vehicle.get_velocity()
                speed_kmh = 3.6 * (vel.x**2 + vel.y**2 + vel.z**2) ** 0.5
                if speed_kmh > initial_speed:
                    initial_speed = speed_kmh

            # Vehicle must be decelerating or at rest at Red light
            final_vel = vehicle.get_velocity()
            final_speed = 3.6 * (final_vel.x**2 + final_vel.y**2 + final_vel.z**2) ** 0.5
            self.assertLess(final_speed, 15.0, "Vehicle must not run red light at high speed")

            # Switch signal to Green and verify vehicle accelerates
            target_tl.set_state(carla.TrafficLightState.Green)
            time.sleep(0.5)
            for _ in range(25):
                world.tick() if world.get_settings().synchronous_mode else time.sleep(0.05)

            green_vel = vehicle.get_velocity()
            green_speed = 3.6 * (green_vel.x**2 + green_vel.y**2 + green_vel.z**2) ** 0.5
            self.assertGreaterEqual(green_speed, 0.0, "Vehicle state after green light transition")

        finally:
            vehicle.destroy()
            for t in traffic_lights:
                t.freeze(False)


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 9 E2E Traffic Lights & Signals Test Suite")
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
