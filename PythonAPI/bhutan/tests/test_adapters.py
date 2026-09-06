import json
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.adapters.common import derive_motion, write_run_dir  # noqa: E402
from bhutan_sim.adapters.gpx import read_gpx, write_gpx  # noqa: E402
from bhutan_sim.adapters.traccar import position_to_point, read_traccar  # noqa: E402
from bhutan_sim.telemetry import RunManifest, load_run  # noqa: E402

try:
    from bhutan_sim.adapters.mcap_io import HAVE_MCAP, read_mcap, write_mcap
except ImportError:
    HAVE_MCAP = False


class DeriveMotionTests(unittest.TestCase):
    def test_derives_speed_heading_and_grade_from_fixes(self):
        points = [
            {"t": 0.0, "lat": 27.0, "lon": 89.0, "alt": 1000.0},
            {"t": 10.0, "lat": 27.0009, "lon": 89.0, "alt": 1005.0},
        ]
        samples = derive_motion(points)
        self.assertEqual(len(samples), 2)
        self.assertAlmostEqual(samples[1]["speed_mps"], 10.0, delta=0.2)
        self.assertAlmostEqual(samples[1]["heading_deg"], 0.0, delta=1e-6)
        self.assertAlmostEqual(samples[1]["grade_pct"], 5.0, delta=0.2)
        self.assertEqual(samples[0]["source"], "vehicle")

    def test_reported_speed_and_heading_are_kept(self):
        points = [{"t": 0.0, "lat": 1.0, "lon": 1.0, "speed_mps": 5.0, "heading_deg": 90.0}]
        samples = derive_motion(points)
        self.assertEqual(samples[0]["speed_mps"], 5.0)
        self.assertEqual(samples[0]["heading_deg"], 90.0)


class GpxTests(unittest.TestCase):
    def test_round_trips_through_gpx(self):
        samples = derive_motion([
            {"t": 1700000000.0, "lat": 27.47, "lon": 89.63, "alt": 2400.0},
            {"t": 1700000010.0, "lat": 27.4705, "lon": 89.6302, "alt": 2402.0},
        ])
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "track.gpx")
            write_gpx(samples, path, name="test track")
            with open(path, encoding="utf-8") as handle:
                content = handle.read()
            self.assertIn("<trk>", content)
            self.assertIn("test track", content)
            back = read_gpx(path)
            self.assertEqual(len(back), 2)
            self.assertAlmostEqual(back[0]["lat"], 27.47, places=4)
            self.assertAlmostEqual(back[1]["speed_mps"], samples[1]["speed_mps"], delta=0.5)


class TraccarTests(unittest.TestCase):
    def test_position_to_point_handles_forward_envelope_and_knots(self):
        point = position_to_point({
            "device": {"id": 7},
            "position": {"deviceId": 7, "fixTime": "2026-09-05T10:00:00.000+00:00", "latitude": 27.47, "longitude": 89.63,
                        "altitude": 2400.0, "speed": 10.0, "course": 90.0, "attributes": {}},
        })
        self.assertIsNotNone(point)
        self.assertEqual(point["device_id"], "7")
        self.assertAlmostEqual(point["speed_mps"], 10.0 * 0.514444, places=4)

    def test_read_traccar_filters_by_device_and_derives_motion(self):
        positions = [
            {"deviceId": 1, "fixTime": "2026-09-05T10:00:00Z", "latitude": 27.0, "longitude": 89.0},
            {"deviceId": 1, "fixTime": "2026-09-05T10:00:10Z", "latitude": 27.0009, "longitude": 89.0},
            {"deviceId": 2, "fixTime": "2026-09-05T10:00:05Z", "latitude": 10.0, "longitude": 10.0},
        ]
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "positions.json")
            with open(path, "w", encoding="utf-8") as handle:
                json.dump(positions, handle)
            samples = read_traccar(path, device_id=1)
            self.assertEqual(len(samples), 2)
            self.assertAlmostEqual(samples[1]["speed_mps"], 10.0, delta=0.2)


@unittest.skipUnless(HAVE_MCAP, "mcap package not installed")
class McapTests(unittest.TestCase):
    def test_round_trips_samples_and_events_through_mcap(self):
        samples = derive_motion([
            {"t": 1000.0, "lat": 27.47, "lon": 89.63, "alt": 2400.0},
            {"t": 1000.5, "lat": 27.4701, "lon": 89.6301, "alt": 2401.0},
        ])
        events = [{"t": 1000.2, "frame": 1, "event_class": "hard_brake", "severity": "warning", "rule_id": "SR-01", "description": "brake"}]
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "run.mcap")
            write_mcap(path, samples, events, metadata={"run_id": "demo"})
            self.assertTrue(os.path.exists(path))
            back = read_mcap(path)
            self.assertEqual(len(back), 2)
            self.assertAlmostEqual(float(back[0]["lat"]), 27.47, places=4)


class WriteRunDirTests(unittest.TestCase):
    def test_writes_a_loadable_run_directory(self):
        samples = derive_motion([
            {"t": 500.0, "lat": 27.0, "lon": 89.0},
            {"t": 501.0, "lat": 27.0001, "lon": 89.0},
        ])
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = os.path.join(tmp, "run-1")
            manifest = RunManifest(run_id="run-1", source="vehicle", vehicle_class="truck", route_id="r1")
            result = write_run_dir(run_dir, manifest, samples)
            self.assertEqual(result["samples"], 2)
            loaded = load_run(run_dir)
            self.assertEqual(loaded["manifest"]["run_id"], "run-1")
            self.assertEqual(len(loaded["samples"]), 2)
            self.assertTrue(loaded["manifest"]["streams"]["gnss"])


if __name__ == "__main__":
    unittest.main()
