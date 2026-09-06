import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.evaluation import evaluate_perception, merge_reports  # noqa: E402
from bhutan_sim.library import generate_library  # noqa: E402


GT = [
    {"frame": 1, "objects": [{"cls": "vehicle", "x": 20.0, "y": 0.0}, {"cls": "pedestrian", "x": 12.0, "y": 3.0}, {"cls": "animal", "x": 50.0, "y": -1.0}]},
    {"frame": 2, "objects": [{"cls": "vehicle", "x": 19.0, "y": 0.0}, {"cls": "pedestrian", "x": 11.0, "y": 3.0}]},
]
DET = [
    {"frame": 1, "detections": [{"cls": "car", "x": 20.5, "y": 0.2, "score": 0.9}, {"cls": "person", "x": 12.3, "y": 2.8, "score": 0.8}]},
    {"frame": 2, "detections": [{"cls": "car", "x": 19.2, "y": 0.1, "score": 0.9}, {"cls": "car", "x": 40.0, "y": 0.0, "score": 0.6}, {"cls": "person", "x": 11.0, "y": 3.0, "score": 0.1}]},
]


class EvaluationTests(unittest.TestCase):
    def test_counts_and_aliases(self):
        report = evaluate_perception(GT, DET, "m", "1", "run-1", conditions={"weather": "fog", "lighting": "night", "route_class": "curve_flat"})
        self.assertEqual(report.overall.tp, 3)
        self.assertEqual(report.overall.fp, 1)       # the phantom car at 40 m
        self.assertEqual(report.overall.fn, 2)       # the far animal and the low-score pedestrian
        self.assertEqual(report.by_class["vehicle"].tp, 2)
        self.assertEqual(report.by_class["animal"].fn, 1)
        self.assertEqual(report.by_condition["weather"]["fog"].tp, 3)
        self.assertAlmostEqual(report.overall.precision, 0.75)
        self.assertAlmostEqual(report.overall.recall, 0.6)

    def test_failure_clusters_ranked(self):
        report = evaluate_perception(GT, DET, "m", "1", "run-1")
        clusters = report.failure_clusters
        self.assertEqual(len(clusters), 2)
        self.assertEqual({c["cls"] for c in clusters}, {"animal", "pedestrian"})
        self.assertEqual(clusters[0]["missed"], 1)

    def test_scenario_conditions_and_reproducibility(self):
        scenario = generate_library()[0].to_dict()
        report = evaluate_perception(GT, DET, "m", "1", "run-1", scenario=scenario).to_dict()
        self.assertEqual(report["conditions"]["lighting"], scenario["lighting_class"])
        self.assertEqual(report["inputs"]["scenario"], scenario["content_hash"])
        self.assertTrue(report["reproducible"])
        again = evaluate_perception(GT, DET, "m", "1", "run-1", scenario=scenario).to_dict()
        self.assertEqual(report["evaluation_id"], again["evaluation_id"])

    def test_merge(self):
        a = evaluate_perception(GT, DET, "m", "1", "run-1", conditions={"weather": "clear"})
        b = evaluate_perception(GT, DET, "m", "1", "run-2", conditions={"weather": "fog"})
        merged = merge_reports([a, b])
        self.assertEqual(merged["overall"]["tp"], 6)
        self.assertIn("fog", merged["by_condition"]["weather"])


if __name__ == "__main__":
    unittest.main()
