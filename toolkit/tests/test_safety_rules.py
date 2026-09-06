import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.safety_rules import CRITICAL_RULES, check_samples, classify_events, summarize  # noqa: E402


def sample(t, **kw):
    base = {"t": t, "frame": int(t * 20), "lat": 27.47, "lon": 89.63, "speed_mps": 10.0, "accel_x": 0.0,
            "accel_y": 0.0, "grade_pct": 0.0}
    base.update(kw)
    return base


class SafetyRuleTests(unittest.TestCase):
    def test_hard_brake_is_debounced(self):
        samples = [sample(i * 0.05, accel_x=-4.0) for i in range(40)]  # two seconds of braking
        violations = check_samples(samples, "truck")
        hard = [v for v in violations if v.rule_id == "SR-01"]
        self.assertEqual(len(hard), 1)
        self.assertEqual(hard[0].severity, "warning")

    def test_truck_downhill_overspeed_is_critical(self):
        samples = [sample(0.0, speed_mps=15.0, grade_pct=-9.0)]
        violations = check_samples(samples, "truck")
        self.assertTrue(any(v.rule_id == "SR-06" and v.severity == "critical" for v in violations))
        self.assertIn("SR-06", CRITICAL_RULES)

    def test_car_tolerates_same_descent(self):
        samples = [sample(0.0, speed_mps=15.0, grade_pct=-9.0)]
        self.assertFalse(any(v.rule_id == "SR-06" for v in check_samples(samples, "car")))

    def test_ttc_uses_closing_speed(self):
        close = sample(0.0, lead_distance_m=10.0, lead_rel_speed_mps=-8.0)   # 1.25 s
        far = sample(5.0, lead_distance_m=50.0, lead_rel_speed_mps=-8.0)     # 6.25 s
        violations = check_samples([close, far], "truck")
        self.assertEqual([v.rule_id for v in violations if v.rule_id == "SR-03"], ["SR-03"])

    def test_speeding_needs_limit(self):
        samples = [sample(0.0, speed_mps=20.0)]
        self.assertFalse(any(v.rule_id == "SR-02" for v in check_samples(samples, "truck")))
        self.assertTrue(any(v.rule_id == "SR-02" for v in check_samples(samples, "truck", speed_limit_kph=40)))

    def test_collision_events_are_classified_critical(self):
        events = [{"t": 1.0, "frame": 20, "event_class": "collision", "description": "hit static.prop.box01"}]
        classified = classify_events(events)
        self.assertEqual(classified[0].rule_id, "SR-05")
        self.assertEqual(classified[0].severity, "critical")
        summary = summarize(classified)
        self.assertEqual(summary["critical"], 1)


if __name__ == "__main__":
    unittest.main()
