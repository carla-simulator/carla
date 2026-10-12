# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import time
import numpy as np
from queue import Queue
from queue import Empty

WIDTH = 320
HEIGHT = 240
NUM_FRAMES = 20
STEP_M = 0.5
# Packed DVSEvent layout (x, y, t, pol), 13 bytes per event.
DVS_DTYPE = np.dtype([('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool_)])


class TestDVSCamera(SyncSmokeTest):
    def tearDown(self):
        # Restore the settings from SyncSmokeTest.setUp and reload a map that ships with the
        # package. The base SmokeTest.tearDown loads Town03_Opt, which may not be present.
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        # workaround: give time to the engine to clean memory after loading
        time.sleep(5)
        self.world = None
        self.client = None

    def run_dvs(self, attributes):
        """Moves a DVS camera along the road and returns the received event arrays.

        The DVS only sends frames that have events, so the camera moves every tick.
        """
        bp = self.world.get_blueprint_library().find('sensor.camera.dvs')
        bp.set_attribute('image_size_x', str(WIDTH))
        bp.set_attribute('image_size_y', str(HEIGHT))
        for key, value in attributes.items():
            bp.set_attribute(key, value)

        carla_map = self.world.get_map()
        wp = carla_map.get_waypoint(carla_map.get_spawn_points()[0].location)
        def transform(waypoint):
            return carla.Transform(waypoint.transform.location + carla.Location(z=1.6), waypoint.transform.rotation)

        sensor = self.world.spawn_actor(bp, transform(wp))
        data_queue = Queue()
        sensor.listen(data_queue.put)
        try:
            for _ in range(NUM_FRAMES):
                nxt = wp.next(STEP_M)
                if nxt:
                    wp = nxt[0]
                sensor.set_transform(transform(wp))
                self.world.tick()
            # Let the last frames arrive.
            time.sleep(2.0)
        finally:
            sensor.stop()
            sensor.destroy()

        frames = []
        while True:
            try:
                frames.append(data_queue.get_nowait())
            except Empty:
                break
        return frames

    def check_frames(self, frames, delta):
        self.assertGreater(len(frames), NUM_FRAMES // 2, "DVS sent events on too few frames")
        positive = negative = 0
        for data in frames:
            self.assertEqual(data.width, WIDTH)
            self.assertEqual(data.height, HEIGHT)
            events = np.frombuffer(data.raw_data, dtype=DVS_DTYPE)
            self.assertEqual(len(events), len(data))
            self.assertGreater(len(events), 0)
            self.assertTrue(np.all(events['x'] < WIDTH), "event x outside the image")
            self.assertTrue(np.all(events['y'] < HEIGHT), "event y outside the image")
            self.assertTrue(np.all(np.diff(events['t']) >= 0), "events are not sorted by timestamp")
            # Events fall between the previous frame and this one (1 ms tolerance).
            frame_ns = data.timestamp * 1e9
            self.assertLessEqual(events['t'].max(), frame_ns + 1e6)
            self.assertGreaterEqual(events['t'].min(), frame_ns - delta * 1e9 - 1e6)
            # The Python accessors agree with the raw buffer.
            first = data[0]
            self.assertEqual((first.x, first.y, first.t, first.pol),
                             (events['x'][0], events['y'][0], events['t'][0], events['pol'][0]))
            positive += int(np.count_nonzero(events['pol']))
            negative += int(len(events) - np.count_nonzero(events['pol']))
        self.assertGreater(positive, 0, "no positive events")
        self.assertGreater(negative, 0, "no negative events")

    def test_dvs_log_events(self):
        print("TestDVSCamera.test_dvs_log_events")
        frames = self.run_dvs({'use_log': 'true'})
        self.check_frames(frames, self.world.get_settings().fixed_delta_seconds)

    def test_dvs_linear_events(self):
        print("TestDVSCamera.test_dvs_linear_events")
        # The default 0.3 threshold is meant for log intensity, linear values are 0-255.
        frames = self.run_dvs({'use_log': 'false', 'positive_threshold': '20', 'negative_threshold': '20'})
        self.check_frames(frames, self.world.get_settings().fixed_delta_seconds)

    def test_dvs_threshold_reduces_events(self):
        print("TestDVSCamera.test_dvs_threshold_reduces_events")
        low = sum(len(data) for data in self.run_dvs({'positive_threshold': '0.3', 'negative_threshold': '0.3'}))
        high = sum(len(data) for data in self.run_dvs({'positive_threshold': '0.9', 'negative_threshold': '0.9'}))
        self.assertGreater(low, 0)
        self.assertLess(high, low, "a higher contrast threshold should produce fewer events")
