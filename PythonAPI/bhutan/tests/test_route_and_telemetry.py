import json
import math
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.route import profile, segment_route  # noqa: E402
from bhutan_sim.taxonomy import Taxonomy  # noqa: E402
from bhutan_sim.telemetry import Event, RunManifest, Sample, TelemetryLogger, load_run  # noqa: E402


def hairpin_trace():
    """A 300 m straight at 2 %, then a 25 m radius half-turn climbing at about 10 %."""
    pts = []
    lat0, lon0 = 27.4728, 89.6390
    m_per_deg_lat = 111320.0
    m_per_deg_lon = 111320.0 * math.cos(math.radians(lat0))
    for i in range(0, 300, 3):
        pts.append({"lat": lat0 + i / m_per_deg_lat, "lon": lon0, "alt": 2300.0 + i * 0.02})
    for k in range(0, 60):
        ang = math.pi * k / 60.0
        x = 25.0 * math.sin(ang)
        y = 300.0 + 25.0 * (1 - math.cos(ang))
        pts.append({"lat": lat0 + y / m_per_deg_lat, "lon": lon0 + x / m_per_deg_lon, "alt": 2306.0 + k * 0.13})
    return pts


class RouteTests(unittest.TestCase):
    def test_segments_detect_curvature_and_grade(self):
        segments = segment_route(hairpin_trace())
        self.assertGreaterEqual(len(segments), 3)
        self.assertTrue(segments[0].route_class.startswith("straight"))
        self.assertGreater(segments[-1].curvature, 0.03)
        self.assertGreater(segments[-1].grade_pct, 6.0)

    def test_profile_recommends_families(self):
        result = profile(hairpin_trace(), Taxonomy.load())
        families = [r["family"] for r in result["recommended_families"]]
        self.assertTrue(families)
        self.assertIn("grade_climb", families)


class TelemetryTests(unittest.TestCase):
    def test_logger_writes_manifest_and_hashes(self):
        with tempfile.TemporaryDirectory() as tmp:
            manifest = RunManifest(run_id="run-x", source="sim", vehicle_class="truck", route_id="r1", started_at=1.0)
            logger = TelemetryLogger(tmp, manifest)
            logger.sample(Sample(t=1.0, frame=1, lat=27.0, lon=89.0))
            logger.event(Event(t=1.0, frame=1, event_class="hard_brake", severity="warning"))
            logger.close()
            run = load_run(tmp)
            self.assertEqual(run["manifest"]["sample_count"], 1)
            self.assertTrue(run["manifest"]["streams"]["events"])
            self.assertTrue(run["manifest"]["files"]["telemetry.jsonl"].startswith("sha256:"))
            self.assertEqual(run["events"][0]["event_class"], "hard_brake")

    def test_event_validation(self):
        with self.assertRaises(ValueError):
            Event(t=0, frame=0, event_class="not_a_class")
        with self.assertRaises(ValueError):
            Event(t=0, frame=0, event_class="collision", severity="huge")


if __name__ == "__main__":
    unittest.main()
