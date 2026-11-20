# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the traffic script's seed handoff without creating a client."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest import mock

import carla
import numpy


SOURCE_ROOT = Path(os.environ.get(
    "CARLA_REGRESSION_SOURCE_ROOT", str(Path(__file__).resolve().parents[3])))
SPEC = importlib.util.spec_from_file_location(
    "traffic_seed_regression", SOURCE_ROOT / "PythonAPI/examples/generate_traffic.py")
TRAFFIC = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(TRAFFIC)


class _Finished(Exception):
    """Stop the example after setup and before its unbounded tick loop."""


class TestGenerateTrafficSeeding(unittest.TestCase):
    """Keep the actual parser and NumPy generator at inert RPC boundaries."""

    def run_script(self, arguments):
        """Run the complete example with zero actors and a bounded endpoint."""
        world = mock.Mock()
        world.get_settings.side_effect = carla.WorldSettings
        world.get_blueprint_library.return_value.filter.return_value = [SimpleNamespace(id="unused")]
        world.get_map.return_value.get_spawn_points.return_value = []
        world.get_actors.return_value = []
        client = mock.Mock()
        client.get_world.return_value = world
        client.apply_batch_sync.return_value = []
        manager = client.get_trafficmanager.return_value
        manager.get_port.return_value = 8000
        manager.global_percentage_speed_difference.side_effect = _Finished
        argv = ["generate_traffic.py", "-n", "0", "-w", "0", *arguments]
        with mock.patch.object(TRAFFIC.carla, "Client", return_value=client) as constructor, \
                mock.patch.object(TRAFFIC.time, "sleep"), \
                mock.patch.object(TRAFFIC.time, "time", return_value=123), \
                mock.patch.object(sys, "argv", argv), \
                contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaises(_Finished):
                TRAFFIC.main()
        constructor.assert_called_once_with("127.0.0.1", 2000)
        self.assertEqual(client.apply_batch_sync.call_count, 3)
        return world, manager

    def setUp(self):
        """Preserve the process NumPy generator across regression methods."""
        self.random_state = numpy.random.get_state()

    def tearDown(self):
        """Restore the process NumPy generator after each method."""
        numpy.random.set_state(self.random_state)

    def test_omitted_pedestrian_seed_preserves_vehicle_seed(self):
        """Without --seedw, pedestrian seeding leaves the vehicle seed in place."""
        world, manager = self.run_script(["--seed", "10"])
        world.set_pedestrians_seed.assert_not_called()
        manager.set_random_device_seed.assert_called_once_with(10)
        expected = numpy.random.RandomState(10).get_state()
        actual = numpy.random.get_state()
        numpy.testing.assert_array_equal(actual[1], expected[1])
        self.assertEqual(actual[2:], expected[2:])

    def test_explicit_zero_seed_reaches_pedestrian_boundary(self):
        """An explicit zero reaches the same native API boundary."""
        world, _ = self.run_script(["--seed", "10", "--seedw", "0"])
        world.set_pedestrians_seed.assert_called_once_with(0)

    def test_zero_seed_resets_the_actual_numpy_generator(self):
        """Pedestrian blueprint choices use the requested NumPy seed."""
        self.run_script(["--seed", "10", "--seedw", "0"])
        expected = numpy.random.RandomState(0).get_state()
        actual = numpy.random.get_state()
        numpy.testing.assert_array_equal(actual[1], expected[1])
        self.assertEqual(actual[2:], expected[2:])

    def test_positive_pedestrian_seed_remains_supported(self):
        """Positive seeds preserve the existing handoff and NumPy state."""
        world, _ = self.run_script(["--seed", "10", "--seedw", "7"])
        world.set_pedestrians_seed.assert_called_once_with(7)
        expected = numpy.random.RandomState(7).get_state()
        actual = numpy.random.get_state()
        numpy.testing.assert_array_equal(actual[1], expected[1])
        self.assertEqual(actual[2:], expected[2:])

    def test_vehicle_zero_seed_remains_supported(self):
        """The existing vehicle seed zero behavior is retained."""
        _, manager = self.run_script(["--seed", "0", "--seedw", "7"])
        manager.set_random_device_seed.assert_called_once_with(0)


if __name__ == "__main__":
    unittest.main()
