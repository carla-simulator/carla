# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run production examples with real interpolation/buffers/settings and inert RPCs."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
from queue import Empty, Queue
import re
import sys
import tempfile
import threading
from types import SimpleNamespace as NS
import unittest
from unittest.mock import patch

import carla
import numpy as np

ROOT = Path(os.environ.get('CARLA_EXAMPLE_CONTRACT_TEST_ROOT', Path(__file__).resolve().parents[3]))


def example(name):
    """Load the actual example rather than copying its production logic."""
    spec = importlib.util.spec_from_file_location(name, ROOT / 'PythonAPI/examples' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@unittest.skipUnless(importlib.util.find_spec('scipy'), 'Optional SciPy is required for actual interpolation')
class TestCameraInterpolation(unittest.TestCase):
    """Exercise CLI endpoint selection and the real SciPy interpolation path."""

    def test_requested_host_and_port_reach_actual_client_boundary(self):
        """A valid nondefault server endpoint must not be silently discarded."""
        module = example('interpolate_camera')
        world = NS(debug=NS(draw_point=lambda *a, **k: None, draw_string=lambda *a, **k: None,
                            draw_arrow=lambda *a, **k: None), tick=lambda: None)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'camera.xml'
            path.write_text('<interpolations><interpolation id="route" order="1">'
                            '<point time="0" x="0"/><point time="1" x="1"/>'
                            '</interpolation></interpolations>')
            arguments = ['interpolate_camera.py', '--host', '203.0.113.42', '--port', '32123',
                         '--file', str(path), '--id', 'route', '--debug']
            with patch.object(module.carla, 'Client', return_value=NS(get_world=lambda: world)) as client, \
                    patch.object(sys, 'argv', arguments):
                module.main()
            client.assert_called_once_with('203.0.113.42', 32123)

    def test_off_grid_keypoints_share_one_constant_fps_timeline(self):
        """Twenty 0.05-second intervals must not stretch one second into 40 frames."""
        module = example('interpolate_camera')
        points = [[0.0, carla.Location(), carla.Rotation()]]
        points.extend([0.05, carla.Location(float(index), 0, 0), carla.Rotation()]
                      for index in range(1, 21))
        data = module.interpolate_data(points, 1)
        self.assertEqual(len(data), 30)
        np.testing.assert_allclose(np.diff([point.location.x for point in data]),
                                   20.0 / 30.0, atol=2e-6)

    def test_single_keypoint_has_one_static_pose(self):
        """A valid constant camera keyframe must not become an empty no-op."""
        module = example('interpolate_camera')
        point = [0.0, carla.Location(7, -8, 9), carla.Rotation(10, 20, 30)]
        data = module.interpolate_data([point], 1)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0].location, point[1])
        self.assertEqual(data[0].rotation, point[2])

    def test_ordinary_two_keypoint_control(self):
        """A whole-second 30 FPS interval keeps the existing frame cadence."""
        module = example('interpolate_camera')
        data = module.interpolate_data([[0, carla.Location(), carla.Rotation()],
                                        [1, carla.Location(30, 0, 0), carla.Rotation()]], 1)
        self.assertEqual(len(data), 30)
        np.testing.assert_allclose([point.location.x for point in data], np.arange(30), atol=2e-6)


class CallbackSensor:
    """A listener may receive a valid packet on another thread before listen returns."""

    def __init__(self, data, immediate):
        self.data = data
        self.immediate = immediate
        self.callback = None
        self.errors = []

    def emit(self):
        """Run the registered production callback on a real worker thread."""
        def receive():
            try:
                self.callback(self.data)
            except BaseException as error:  # pylint: disable=broad-exception-caught
                self.errors.append(error)
        worker = threading.Thread(target=receive)
        worker.start()
        worker.join(timeout=2)
        if worker.is_alive():
            raise AssertionError('sensor callback failed to finish')

    def listen(self, callback):
        """Register the real callback and optionally deliver its first packet immediately."""
        self.callback = callback
        if self.immediate:
            self.emit()

    @staticmethod
    def destroy():
        """Sensor actor destruction is an inert remote boundary."""


@unittest.skipUnless(importlib.util.find_spec('pygame'), 'Optional pygame is required to load the real example')
class TestEarlySensorCallbacks(unittest.TestCase):
    """Only RPC packet delivery is inert; buffer processing and timers remain real."""

    def create(self, sensor_type, immediate=True):
        """Construct the production SensorManager with a native transform and valid packet."""
        module = example('visualize_multiple_sensors')
        if sensor_type == 'RGBCamera':
            data = NS(width=32, height=24, raw_data=bytes(32 * 24 * 4), convert=lambda _: None)
        else:
            data = NS(raw_data=np.array([[0, 0, 0, 0.5]], dtype=np.float32).tobytes())
        sensor = CallbackSensor(data, immediate)
        blueprint = NS(set_attribute=lambda *a: None,
                       get_attribute=lambda _: NS(recommended_values=['0']))
        world = NS(get_blueprint_library=lambda: NS(find=lambda _: blueprint),
                   spawn_actor=lambda *a, **k: sensor)
        display = NS(get_display_size=lambda: [32, 24], render_enabled=lambda: False,
                     add_sensor=lambda _: None)
        manager = module.SensorManager(world, display, sensor_type, carla.Transform(), None,
                                       {'range': '100'}, [0, 0])
        self.addCleanup(manager.destroy)
        return manager, sensor

    def test_rgb_callback_is_ready_during_registration(self):
        """RGB delivery can race with the constructor immediately after listen."""
        manager, sensor = self.create('RGBCamera')
        self.assertEqual(sensor.errors, [])
        self.assertEqual(manager.tics_processing, 1)

    def test_lidar_callback_options_and_timer_are_ready(self):
        """The same construction contract includes LiDAR options and timing fields."""
        manager, sensor = self.create('LiDAR')
        self.assertEqual(sensor.errors, [])
        self.assertEqual(manager.tics_processing, 1)

    def test_deferred_callback_control(self):
        """Ordinary delivery after construction remains unchanged."""
        manager, sensor = self.create('RGBCamera', immediate=False)
        sensor.emit()
        self.assertEqual(sensor.errors, [])
        self.assertEqual(manager.tics_processing, 1)


class SyncSensor:
    """Track actual callback registration and remote actor cleanup."""

    def __init__(self, fail_listen=False):
        self.fail_listen = fail_listen
        self.callback = None
        self.destroyed = False

    def listen(self, callback):
        """Allow a legitimate RPC/stream-registration failure at this boundary."""
        if self.fail_listen:
            raise RuntimeError('listener boundary failed')
        self.callback = callback

    def destroy(self):
        """Record whether the production finally block owns this actor."""
        self.destroyed = True


class SyncWorld:
    """Native world settings with deterministic remote frame delivery."""

    def __init__(self, schedule=(), failure=None):
        self.schedule = schedule
        self.failure = failure
        self.sensors = []
        self.ticks = 0
        self.settings_reads = 0
        self.applied = []

    def get_settings(self):
        """Return value copies, matching the actual native settings contract."""
        self.settings_reads += 1
        if self.failure == 'settings':
            raise RuntimeError('settings boundary failed')
        return carla.WorldSettings(synchronous_mode=False, no_rendering_mode=True, fixed_delta_seconds=0.037)

    def apply_settings(self, settings):
        """Keep successful application and a restore failure distinguishable."""
        self.applied.append(settings)
        if self.failure == 'restore' and len(self.applied) == 2:
            raise RuntimeError('restore boundary failed')

    def get_blueprint_library(self):
        """A setup RPC may fail before any actor or sensor list exists."""
        if self.failure == 'blueprints':
            raise RuntimeError('blueprint boundary failed')
        return NS(find=lambda _: NS(set_attribute=lambda *a: None))

    def spawn_actor(self, *_args):
        """Actor allocation succeeds before a later stream registration can fail."""
        fail = self.failure == 'first-listen' or (self.failure == 'third-listen' and len(self.sensors) == 2)
        sensor = SyncSensor(fail)
        self.sensors.append(sensor)
        return sensor

    def tick(self):
        """Deliver queued sensor frame metadata, then end the real main loop."""
        self.ticks += 1
        if self.ticks > 1:
            raise KeyboardInterrupt
        for index, frame in self.schedule:
            self.sensors[index].callback(NS(frame=frame))

    @staticmethod
    def get_snapshot():
        """The current server frame is 10 for each focused example run."""
        return NS(frame=10)


class TestSensorSynchronizationContracts(unittest.TestCase):
    """Run the full actual example entry point and inspect its data/cleanup."""

    def invoke(self, world, error=KeyboardInterrupt):
        """Replace only client/world RPC boundaries, preserving queues and callbacks."""
        module = example('sensor_synchronization')
        output = io.StringIO()
        client = NS(set_timeout=lambda _: None, get_world=lambda: world)
        with patch.object(module.carla, 'Client', return_value=client), contextlib.redirect_stdout(output):
            with self.assertRaises(error):
                module.main()
        return re.findall(r'Frame: (\d+)   Sensor: (\w+)', output.getvalue())

    def assert_one_complete_frame(self, rows):
        """Each current frame must contain one packet from every named sensor."""
        self.assertEqual(len(rows), 5)
        self.assertEqual({frame for frame, _ in rows}, {'10'})
        self.assertEqual({name for _, name in rows}, {'camera01', 'lidar01', 'lidar02', 'radar01', 'radar02'})

    def test_stale_packet_does_not_satisfy_current_frame(self):
        """A delayed prior-frame camera packet must not replace the current packet."""
        world = SyncWorld([(0, 9), (1, 10), (2, 10), (3, 10), (4, 10), (0, 10)])
        self.assert_one_complete_frame(self.invoke(world))

    def test_duplicate_sensor_packet_does_not_hide_a_missing_sensor(self):
        """Counting duplicate camera data cannot stand in for the second radar."""
        world = SyncWorld([(0, 10), (0, 10), (1, 10), (2, 10), (3, 10), (4, 10)])
        self.assert_one_complete_frame(self.invoke(world))

    def test_current_packets_and_cleanup_control(self):
        """Ordinary matching delivery and native settings restoration remain intact."""
        world = SyncWorld([(index, 10) for index in range(5)])
        self.assert_one_complete_frame(self.invoke(world))
        self.assertTrue(all(sensor.destroyed for sensor in world.sensors))
        self.assertFalse(world.applied[-1].synchronous_mode)
        self.assertTrue(world.applied[-1].no_rendering_mode)
        self.assertEqual(world.applied[-1].fixed_delta_seconds, 0.037)

    def check_listen_failure(self, phase, expected_actors):
        """Every allocated actor must already be tracked when listen can fail."""
        world = SyncWorld(failure=phase)
        self.invoke(world, RuntimeError)
        self.assertEqual(len(world.sensors), expected_actors)
        self.assertTrue(all(sensor.destroyed for sensor in world.sensors))
        self.assertFalse(world.applied[-1].synchronous_mode)

    def test_first_listener_failure_destroys_its_allocated_sensor(self):
        """The first sensor is still owned even if its listener never starts."""
        self.check_listen_failure('first-listen', 1)

    def test_later_listener_failure_destroys_all_allocated_sensors(self):
        """Successful earlier sensors and the newest allocation are all tracked."""
        self.check_listen_failure('third-listen', 3)

    def test_early_blueprint_error_is_not_masked_by_cleanup(self):
        """The documented settings restore must work before any sensor exists."""
        world = SyncWorld(failure='blueprints')
        self.invoke(world, RuntimeError)
        self.assertFalse(world.applied[-1].synchronous_mode)

    def test_early_settings_error_is_not_masked_by_cleanup(self):
        """No restoration is attempted without a successfully read original value."""
        world = SyncWorld(failure='settings')
        self.invoke(world, RuntimeError)
        self.assertEqual(world.applied, [])


    def test_frame_collection_reports_a_missing_sensor(self):
        """Stale, future and unknown packets cannot satisfy a missing current sensor."""
        module = example('sensor_synchronization')
        sensor_queue = Queue()
        for packet in ((9, 'camera01'), (11, 'camera01'), (10, 'unknown')):
            sensor_queue.put(packet)
        with self.assertRaises(Empty):
            list(module.collect_sensor_frame(sensor_queue, 10, ['camera01'], timeout=0.01))

    def test_empty_expected_frame_needs_no_queue_data(self):
        """An empty expected sensor set completes immediately without reading the queue."""
        module = example('sensor_synchronization')
        self.assertEqual(list(module.collect_sensor_frame(Queue(), 10, [])), [])

    def test_restore_failure_still_destroys_allocated_sensors(self):
        """A failed settings RPC must not bypass actor cleanup."""
        world = SyncWorld([(index, 10) for index in range(5)], failure='restore')
        self.invoke(world, RuntimeError)
        self.assertTrue(all(sensor.destroyed for sensor in world.sensors))


if __name__ == '__main__':
    unittest.main()
