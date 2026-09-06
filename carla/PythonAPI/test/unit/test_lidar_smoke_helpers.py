# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import ast
import os
import unittest

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))

LIDAR_SMOKE_TEST = os.path.join(
    REPO_ROOT, "PythonAPI", "test", "smoke", "test_lidar.py")


class TestLidarSmokeHelpers(unittest.TestCase):
    def test_get_current_detection_points_is_instance_method(self):
        with open(LIDAR_SMOKE_TEST) as f:
            module = ast.parse(f.read(), filename=LIDAR_SMOKE_TEST)

        sensor_class = next(
            node for node in module.body
            if isinstance(node, ast.ClassDef) and node.name == "Sensor")
        helper = next(
            node for node in sensor_class.body
            if isinstance(node, ast.FunctionDef)
            and node.name == "get_current_detection_points")

        self.assertEqual(
            [arg.arg for arg in helper.args.args],
            ["self"],
            "Sensor.get_current_detection_points must accept self")


if __name__ == "__main__":
    unittest.main()
