# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual example execution with native maps/weather and genuine image output."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
import struct
import sys
import tempfile
from types import SimpleNamespace as NS
import unittest
from unittest.mock import patch

import carla

ROOT = Path(os.environ.get('CARLA_EXAMPLE_TIMING_TEST_ROOT', Path(__file__).resolve().parents[3]))
EVENT = struct.pack('<HHq?', 1, 2, 123, True)


def example(name):
    """Load the real source under test, preserving all production control flow."""
    spec = importlib.util.spec_from_file_location(name, ROOT / 'PythonAPI/examples' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestWeatherWarmupClock(unittest.TestCase):
    """The documented real-second warmup is independent of the simulated cycle clock."""

    def blend(self, speed, simulated_seconds, wall_seconds):
        """Execute the actual native weather blend across two simulator snapshots."""
        module = example('dynamic_day_night_weather_cycle')
        received = []
        world = NS(get_weather=lambda: carla.WeatherParameters(precipitation=50), set_weather=received.append)
        cycle = module.DayNightWeatherCycle(world, speed=speed, warmup_seconds=5)
        now = [100.]
        with patch.object(module.time, 'time', side_effect=lambda: now[0]), \
                patch.object(module.time, 'monotonic', side_effect=lambda: now[0]):
            cycle._on_tick(NS(timestamp=NS(elapsed_seconds=0.)))  # pylint: disable=protected-access
            now[0] += wall_seconds
            cycle._on_tick(NS(timestamp=NS(elapsed_seconds=simulated_seconds)))  # pylint: disable=protected-access
        return cycle, received[-1]

    def test_cycle_speed_does_not_shorten_real_warmup(self):
        """Speed two changes cycle phase, while one real second blends only one fifth."""
        cycle, weather = self.blend(2, 1, 1)
        self.assertEqual(cycle.elapsed, 2)
        self.assertAlmostEqual(weather.precipitation, 40)

    def test_fast_simulation_does_not_skip_warmup(self):
        """Ten simulated seconds in one real second must not cause an immediate snap."""
        cycle, weather = self.blend(1, 10, 1)
        self.assertEqual(cycle.elapsed, 10)
        self.assertAlmostEqual(weather.precipitation, 40)

    def test_slow_simulation_still_completes_real_warmup(self):
        """The blend finishes after five real seconds even if simulation advanced only one."""
        _, weather = self.blend(1, 1, 5)
        self.assertAlmostEqual(weather.precipitation, 0)

    def test_matching_simulation_and_wall_clock_control(self):
        """Ordinary one-to-one timing retains the same native weather interpolation."""
        _, weather = self.blend(1, 1, 1)
        self.assertAlmostEqual(weather.precipitation, 40)


class Blueprint:
    """Only the remote sensor blueprint boundary is inert."""
    id = 'sensor.camera.dvs'

    def __iter__(self):
        return iter(())

    @staticmethod
    def set_attribute(*_args):
        """Blueprint configuration is a remote boundary."""


class Actor:
    """Deliver valid serialized packets to the production callback."""
    id = 2
    type_id = 'sensor.camera.dvs'

    def __init__(self, packets=()):
        self.packets = packets
        self.destroyed = False

    @staticmethod
    def set_autopilot(_active):
        """Vehicle behavior is outside this smoke-script contract."""

    def listen(self, callback):
        """Keep the actual structured NumPy decoding and callback accumulation."""
        for frame, raw in enumerate(self.packets, 10):
            callback(NS(raw_data=raw, frame=frame, timestamp=.5, width=640, height=480))

    @staticmethod
    def stop():
        """The remote listener stop is inert."""

    def destroy(self):
        """Record actual entry-point cleanup ownership."""
        self.destroyed = True


@unittest.skipUnless(importlib.util.find_spec('PIL'), 'Pillow is required for genuine PNG output checks')
class TestDvsSmokePackets(unittest.TestCase):
    """Execute real packet decoding, validation and image encoding, replacing only RPCs."""

    def invoke(self, packets, requested):
        """Run the actual CLI with a deterministic 30-second timeout boundary."""
        module = example('dvs_test')
        sensor, vehicle = Actor(packets), Actor()
        blueprint = Blueprint()
        world = NS(get_blueprint_library=lambda: NS(filter=lambda _: [blueprint], find=lambda _: blueprint),
                   get_map=lambda: NS(get_spawn_points=lambda: [carla.Transform()]),
                   try_spawn_actor=lambda *_args: vehicle, spawn_actor=lambda *_args, **_kwargs: sensor)
        client = NS(set_timeout=lambda _: None, get_world=lambda: world,
                    get_server_version=lambda: 'inert-RPC-boundary')
        temporary = tempfile.TemporaryDirectory()  # pylint: disable=consider-using-with
        self.addCleanup(temporary.cleanup)
        image = Path(temporary.name) / 'image.png'
        arguments = ['dvs_test.py', '--frames', str(requested), '--out', str(image.with_suffix(''))]
        output = io.StringIO()
        with patch.object(module.carla, 'Client', return_value=client), patch.object(sys, 'argv', arguments), \
                patch.object(module.time, 'time', side_effect=[100., 131.]), contextlib.redirect_stdout(output):
            status = module.main()
        self.assertTrue(sensor.destroyed and vehicle.destroyed)
        return status, image, output.getvalue()

    def test_valid_empty_packet_saves_a_blank_frame(self):
        """Static-scene DVS callbacks legitimately contain no events."""
        from PIL import Image  # pylint: disable=import-outside-toplevel
        status, path, _ = self.invoke([b''], 1)
        self.assertEqual(status, 0)
        with Image.open(path) as image:
            self.assertEqual(image.size, (640, 480))
            self.assertIsNone(image.getbbox())

    def test_partial_timeout_does_not_report_pass(self):
        """One valid packet cannot satisfy a request for two or three frames."""
        for requested in (2, 3):
            with self.subTest(requested=requested):
                status, _, output = self.invoke([EVENT], requested)
                self.assertEqual(status, 1)
                self.assertNotIn('PASS', output)

    def test_complete_stream_retains_actual_event_colors(self):
        """A genuine 13-byte native event layout still encodes the expected pixel."""
        from PIL import Image  # pylint: disable=import-outside-toplevel
        status, path, output = self.invoke([b'', EVENT], 2)
        self.assertEqual(status, 0)
        self.assertIn('PASS', output)
        with Image.open(path) as image:
            self.assertEqual(image.getpixel((1, 2)), (0, 0, 255))

    def test_absent_callbacks_remain_a_failure_control(self):
        """No callback is distinct from a valid empty event callback."""
        status, _, output = self.invoke([], 1)
        self.assertEqual(status, 1)
        self.assertIn('no DVS callbacks', output)


class TestLaneExplorerTerminal(unittest.TestCase):
    """Drive the real terminal/ordinary path selection using native OpenDRIVE maps."""

    @staticmethod
    def world(length, lane_id, rule):
        """Generate a valid single driving lane with an ordinary terminal boundary."""
        side = 'left' if lane_id > 0 else 'right'
        xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
               '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
               '</geoReference></header>'
               f'<road id="1" length="{length}" junction="-1" rule="{rule}">'
               f'<planView><geometry s="0" x="0" y="0" hdg="0" length="{length}"><line/>'
               '</geometry></planView><lanes><laneSection s="0"><center><lane id="0" type="none"/>'
               f'</center><{side}><lane id="{lane_id}" type="driving">'
               '<width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
               f'</lane></{side}></laneSection></lanes></road></OpenDRIVE>')
        native_map = carla.Map('terminal-explorer', xml)
        drawings = []
        debug = NS(draw_line=lambda *a, **k: drawings.append((a, k)), draw_point=lambda *a, **k: None,
                   draw_arrow=lambda *a, **k: None)
        return NS(get_map=lambda: native_map, debug=debug), native_map, drawings

    @staticmethod
    def invoke(world, location, interrupt=False):
        """Leave map traversal/drawing native and stop an ordinary loop after its first step."""
        module = example('lane_explorer')
        client = NS(set_timeout=lambda _: None, get_world=lambda: world)
        arguments = ['lane_explorer.py', '-x', str(location.x), '-y', str(location.y), '-z', str(location.z)]
        with patch.object(module.carla, 'Client', return_value=client), patch.object(sys, 'argv', arguments), \
                patch.object(module.time, 'sleep', side_effect=KeyboardInterrupt if interrupt else None):
            module.main()

    def test_valid_terminal_lane_finishes_for_both_directions_and_rules(self):
        """A native empty next-list at a real road end is a normal completion condition."""
        for rule in ('RHT', 'LHT'):
            for lane_id in (-1, 1):
                with self.subTest(rule=rule, lane_id=lane_id):
                    world, native_map, drawings = self.world(2, lane_id, rule)
                    waypoint = native_map.get_waypoint_xodr(1, lane_id, 1)
                    self.assertEqual(waypoint.next(4), [])
                    self.invoke(world, waypoint.transform.location)
                    self.assertEqual(drawings, [])

    def test_ordinary_successor_is_still_followed_control(self):
        """A valid next waypoint continues through the ordinary draw path."""
        world, native_map, drawings = self.world(20, -1, 'RHT')
        with self.assertRaises(KeyboardInterrupt):
            self.invoke(world, native_map.get_waypoint_xodr(1, -1, 1).transform.location, interrupt=True)
        self.assertEqual(len(drawings), 1)


if __name__ == '__main__':
    unittest.main()
