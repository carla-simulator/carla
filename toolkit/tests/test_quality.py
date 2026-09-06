import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.quality import assess_run, events_per_100km, route_distance_km  # noqa: E402


def trace(seconds, rate=20.0, start_lat=27.4728, start_lon=89.6390, speed=10.0, drop_every=None):
    samples = []
    n = int(seconds * rate)
    for i in range(n):
        if drop_every and i % drop_every == 0:
            continue
        t = i / rate
        samples.append({"t": t, "frame": i, "lat": start_lat + (speed * t) / 111320.0, "lon": start_lon,
                        "alt": 2300.0, "speed_mps": speed, "heading_deg": 0.0, "accel_x": 0.0, "accel_y": 0.0})
    return samples


class QualityTests(unittest.TestCase):
    def test_clean_run_passes_all_gates(self):
        samples = trace(90)
        report = assess_run("run-1", samples, [{"event_class": "hard_brake"}], expected_rate_hz=20.0, privacy_status="not_required")
        self.assertEqual(len(report.segments), 3)
        self.assertEqual(report.acceptance_rate, 1.0)
        self.assertTrue(report.replay_complete)
        self.assertTrue(report.passed)
        self.assertAlmostEqual(report.distance_km, 0.9, places=2)

    def test_gap_fails_drift_gate(self):
        samples = trace(60)
        samples = [s for s in samples if not (10.0 <= s["t"] < 12.0)]  # two-second hole
        report = assess_run("run-2", samples, [], expected_rate_hz=20.0, privacy_status="not_required")
        first = report.segments[0]
        self.assertFalse(first.passed)
        self.assertIn("Q2_timestamp_drift", [g.id for g in first.gates if not g.passed])
        self.assertEqual(report.acceptance_rate, 0.5)

    def test_video_without_redaction_is_an_issue(self):
        report = assess_run("run-3", trace(30), [], streams={"video": True}, privacy_status="unprocessed")
        self.assertIn("video present but redaction not run", report.issues)
        self.assertFalse(report.passed)

    def test_empty_run(self):
        report = assess_run("run-4", [], [])
        self.assertEqual(report.sample_count, 0)
        self.assertFalse(report.passed)

    def test_events_per_100km(self):
        events = [{"event_class": "hard_brake"}] * 3 + [{"event_class": "vru_close_pass"}]
        rates = events_per_100km(events, 12.5)
        self.assertEqual(rates["hard_brake"], 24.0)
        self.assertEqual(rates["vru_close_pass"], 8.0)
        self.assertEqual(route_distance_km([]), 0.0)


if __name__ == "__main__":
    unittest.main()
