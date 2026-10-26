"""Exercise real skill commands, native value types and sensor buffer layouts.

Only the remote World/Actor calls are inert: no live simulator is required.
The PNG regression uses the actual optional OpenCV encoder and filesystem.
"""
import contextlib
import fnmatch
import importlib.util
import io
import os
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace as NS
import unittest
from unittest.mock import patch

import carla
import numpy as np

ROOT = Path(os.environ.get("CARLA_SKILL_TEST_ROOT", Path(__file__).resolve().parents[3]))


def load_skill(group, filename):
    path = ROOT / "Skills/python-api" / group / "scripts" / filename
    spec = importlib.util.spec_from_file_location(group.replace("-", "_"), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class Actors(list):
    def filter(self, pattern):
        return Actors(a for a in self if fnmatch.fnmatchcase(a.type_id, pattern))

    def find(self, actor_id):
        return next((a for a in self if a.id == actor_id), None)


class Sensor:
    id = 17
    type_id = "sensor.other.imu"
    parent = None
    attributes = {}

    def __init__(self):
        self.destroyed = self.stopped = 0
        self.listen_error = self.stop_error = self.ros_error = None

    def destroy(self):
        self.destroyed += 1
        return True

    def enable_for_ros(self):
        if self.ros_error:
            raise self.ros_error

    def listen(self, callback):
        if self.listen_error:
            raise self.listen_error

    def stop(self):
        self.stopped += 1
        if self.stop_error:
            raise self.stop_error


class World:
    def __init__(self, sensor=None, synchronous=False):
        self.sensor = sensor or Sensor()
        self.settings = carla.WorldSettings(
            synchronous_mode=synchronous, no_rendering_mode=True,
            fixed_delta_seconds=0.037)
        self.applied = []
        self.waited = self.ticked = 0
        self.tick_error = None

    def get_settings(self):
        return carla.WorldSettings(
            synchronous_mode=self.settings.synchronous_mode,
            no_rendering_mode=self.settings.no_rendering_mode,
            fixed_delta_seconds=self.settings.fixed_delta_seconds)

    def apply_settings(self, settings):
        self.settings = settings
        self.applied.append(settings.synchronous_mode)

    def get_actors(self, ids=None):
        return Actors([self.sensor] if ids is None or self.sensor.id in ids else [])

    def get_blueprint_library(self):
        return NS(find=lambda name: NS(id=name))

    def spawn_actor(self, blueprint, transform, **kwargs):
        assert isinstance(transform, carla.Transform)
        return self.sensor

    def wait_for_tick(self, *args):
        self.waited += 1
        raise KeyboardInterrupt

    def tick(self):
        self.ticked += 1
        if self.tick_error:
            raise self.tick_error

    def set_pedestrians_seed(self, seed):
        if seed == 99:
            raise RuntimeError("seed RPC failed")

    def set_pedestrians_cross_factor(self, factor):
        pass

    def get_random_location_from_navigation(self):
        return None


class TestSensorHolder(unittest.TestCase):
    def setUp(self):
        self.module = load_skill("create-sensor", "sensors.py")
        self.world = World()
        self.client = NS(get_world=lambda: self.world)
        self.args = NS(type="other.imu", attr=[], ros_name=None,
                       ros_frame_id=None, no_ros_tf=False, parent_id=None,
                       attach_to=None, x=1.5, y=0, z=2.4, pitch=0, yaw=0,
                       roll=0, attachment="Rigid", ros=False, hold=True)

    def invoke(self):
        with patch.object(self.module, "_client", return_value=self.client):
            self.module.cmd_spawn(self.args)

    def test_hold_waits_and_destroys_on_interrupt(self):
        self.invoke()
        self.assertEqual(self.world.waited, 1)
        self.assertEqual(self.world.sensor.destroyed, 1)

    def test_hold_destroys_after_ros_setup_failure(self):
        self.args.ros = True
        self.world.sensor.ros_error = RuntimeError("ROS unavailable")
        with self.assertRaisesRegex(RuntimeError, "ROS unavailable"):
            self.invoke()
        self.assertEqual(self.world.sensor.destroyed, 1)

    def test_unheld_sensor_persists_control(self):
        self.args.hold = False
        self.invoke()
        self.assertEqual(self.world.waited, 0)
        self.assertEqual(self.world.sensor.destroyed, 0)


class Controller:
    type_id = "controller.ai.walker"

    def __init__(self, actor_id):
        self.id = actor_id
        self.calls = []

    def start(self):
        self.calls.append("start")

    def go_to_location(self, location):
        self.calls.append("destination")

    def set_max_speed(self, speed):
        self.calls.append("speed")


class TestWalkerOwnership(unittest.TestCase):
    def setUp(self):
        self.module = load_skill("spawn-walkers", "walkers.py")
        self.args = NS(verify_frames=1, retries=1, speed_min=1, speed_max=2)
        self.owned, self.other = Controller(1), Controller(2)
        self.walker = NS(id=3, type_id="walker.pedestrian.0001",
                         get_location=lambda: carla.Location())
        actors = Actors([self.owned, self.other, self.walker])
        self.world = NS(
            get_actors=lambda ids=None: Actors(
                a for a in actors if ids is None or a.id in ids),
            get_random_location_from_navigation=lambda: carla.Location(3, 4, 0))

    def test_retry_only_controls_this_runs_fresh_handles(self):
        stale = Controller(1)
        with patch.object(self.module, "_advance"):
            self.module._ensure_walking(self.world, [stale], [3], self.args)
        self.assertEqual(self.owned.calls, ["start", "destination", "speed"])
        self.assertEqual(self.other.calls, [])
        self.assertEqual(stale.calls, [])

    def test_empty_owned_set_never_retargets_other_crowds(self):
        with patch.object(self.module, "_advance"):
            self.module._ensure_walking(self.world, [], [3], self.args)
        self.assertEqual(self.other.calls, [])


class TestWalkerSetupClock(unittest.TestCase):
    def invoke(self, seed=None, synchronous=False):
        module = load_skill("spawn-walkers", "walkers.py")
        world = World(synchronous=synchronous)
        # Keep the real native WorldSettings and clock functions; only the
        # remote blueprint lookup is inert (no body will be spawned here).
        world.get_blueprint_library = lambda: NS(filter=lambda _: [], find=lambda _: None)
        if synchronous:
            world.wait_for_tick = lambda: None
        args = NS(delta=0.05, no_sync=False, seed=seed, cross_factor=0.1,
                  count=1, hold=True, keep=False)
        with patch.object(module, "_client", return_value=NS(get_world=lambda: world)):
            with self.assertRaises((SystemExit, RuntimeError)):
                module.cmd_spawn(args)
        return world

    def test_empty_navigation_restores_claimed_clock(self):
        world = self.invoke()
        self.assertFalse(world.settings.synchronous_mode)
        self.assertEqual(world.settings.fixed_delta_seconds, 0.037)
        self.assertTrue(world.settings.no_rendering_mode)

    def test_seed_rpc_failure_restores_claimed_clock(self):
        self.assertFalse(self.invoke(seed=99).settings.synchronous_mode)

    def test_foreign_clock_is_untouched_control(self):
        world = self.invoke(synchronous=True)
        self.assertTrue(world.settings.synchronous_mode)
        self.assertEqual(world.applied, [])

class TestSensorFormats(unittest.TestCase):
    def setUp(self):
        self.module = load_skill("read-sensor", "read_sensor.py")

    def test_semantic_single_detection_is_one_pixel(self):
        data = NS(raw_data=struct.pack("<ffffII", 10, 20, 0, 1, 1, 4))
        image = self.module._render_frame(data, "sensor.lidar.ray_cast_semantic", None)
        self.assertEqual(np.count_nonzero(image[:, :, 0]), 1)
        np.testing.assert_array_equal(image[420, 360], [255, 255, 255])

    def test_semantic_metadata_never_becomes_extra_points(self):
        data = NS(raw_data=b"".join(struct.pack("<ffffII", x, y, 0, 1, 3, 4)
                                  for x, y in ((10, 20), (-10, -20))))
        image = self.module._render_frame(data, "sensor.lidar.ray_cast_semantic", None)
        self.assertEqual(np.count_nonzero(image[:, :, 0]), 2)

    def test_plain_and_hss_lidar_four_word_layout_control(self):
        data = NS(raw_data=struct.pack("<ffff", 10, 20, 0, 0.5))
        for sensor in ("sensor.lidar.ray_cast", "sensor.lidar.hss_lidar"):
            with self.subTest(sensor=sensor):
                image = self.module._render_frame(data, sensor, None)
                self.assertEqual(np.count_nonzero(image[:, :, 0]), 1)


class TestPngFailure(unittest.TestCase):
    def test_real_encoder_failure_is_reported(self):
        try:
            import cv2
        except ImportError:
            self.skipTest("Optional OpenCV encoder is not installed")
        module = load_skill("read-sensor", "read_sensor.py")
        data = NS(width=1, height=1, raw_data=bytes([1, 2, 3, 255]), convert=lambda _: None)
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaises(OSError):
                module._write_png(data, str(Path(tmp) / "missing" / "frame.png"), None)
            path = str(Path(tmp) / "frame.png")
            module._write_png(data, path, None)
            np.testing.assert_array_equal(cv2.imread(path), [[[1, 2, 3]]])


class TestCaptureCleanup(unittest.TestCase):
    def invoke(self, phase):
        module = load_skill("read-sensor", "read_sensor.py")
        world = World()
        sensor = world.sensor
        if phase == "listen":
            sensor.listen_error = RuntimeError("listen failed")
        elif phase == "warmup":
            world.tick_error = RuntimeError("warmup failed")
        elif phase == "stop":
            sensor.stop_error = RuntimeError("stop failed")
        with tempfile.TemporaryDirectory() as tmp:
            args = NS(id=sensor.id, type=None, attached_to=None, out=tmp,
                      delta=0.05, no_sync=False, warmup=1, frame_timeout=0.001,
                      frames=1, seconds=1, queue_max=2)
            files = []
            def tracked_open(*args, **kwargs):
                handle = open(*args, **kwargs)
                files.append(handle)
                return handle
            with patch.object(module, "_client", return_value=NS(get_world=lambda: world)):
                with patch.object(module, "open", side_effect=tracked_open, create=True):
                    with self.assertRaisesRegex(RuntimeError, phase + " failed"):
                        module.cmd_save(args)
            try:
                self.assertEqual(len(files), 1)
                self.assertTrue(files[0].closed)
            finally:
                # Close leaked baseline handles so fixture cleanup cannot mask
                # the actual assertion with a Windows sharing violation.
                for handle in files:
                    handle.close()
        self.assertFalse(world.settings.synchronous_mode)
        self.assertEqual(world.settings.fixed_delta_seconds, 0.037)
        self.assertFalse(module._OWNS_CLOCK)

    def test_listen_failure_restores_clock(self):
        self.invoke("listen")

    def test_warmup_failure_restores_clock(self):
        self.invoke("warmup")

    def test_stop_failure_still_restores_clock(self):
        self.invoke("stop")


if __name__ == "__main__":
    with contextlib.redirect_stdout(io.StringIO()):
        unittest.main()
