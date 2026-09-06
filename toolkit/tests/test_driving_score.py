import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.driving_score import PENALTIES, compute_driving_score, infraction_kind  # noqa: E402


class DrivingScoreTests(unittest.TestCase):
    def test_clean_completed_run_scores_100(self):
        score = compute_driving_score([], 60.0, 60.0)
        self.assertEqual(score.driving_score, 100.0)
        self.assertEqual(score.route_completion, 100.0)
        self.assertEqual(score.infraction_penalty, 1.0)
        self.assertEqual(score.route_completion_basis, "duration_vs_planned")
        self.assertEqual(score.infractions, [])

    def test_penalties_multiply_and_collisions_classified_by_actor(self):
        events = [
            {"event_class": "collision", "severity": "critical", "description": "hit walker.pedestrian.0001"},
            {"event_class": "collision", "severity": "critical", "description": "vehicle.tesla.model3"},
            {"event_class": "collision", "severity": "critical", "data": {"source_event": "static.prop.rock"}},
            {"event_class": "hard_brake", "severity": "warning"},
            {"event_class": "sensor_dropout", "severity": "info"},
        ]
        score = compute_driving_score(events, 30.0, 60.0)
        expected = 0.5 * 0.6 * 0.65 * 0.97
        self.assertAlmostEqual(score.infraction_penalty, round(expected, 4))
        self.assertEqual(score.route_completion, 50.0)
        self.assertAlmostEqual(score.driving_score, round(100.0 * 0.5 * expected, 4))
        self.assertEqual(len(score.infractions), 4)
        self.assertEqual(score.infractions[0].kind, "collision_pedestrian")
        self.assertIsNone(infraction_kind({"event_class": "sensor_dropout"}))

    def test_real_world_run_without_a_plan_assumes_completion(self):
        score = compute_driving_score([{"event_class": "speeding"}], 120.0, None)
        self.assertEqual(score.route_completion_basis, "assumed_complete")
        self.assertAlmostEqual(score.driving_score, 100.0 * PENALTIES["speeding"])


if __name__ == "__main__":
    unittest.main()
