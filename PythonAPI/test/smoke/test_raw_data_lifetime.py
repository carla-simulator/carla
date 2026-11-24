# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import numpy as np
from queue import Queue

WIDTH = 320
HEIGHT = 240
TIMEOUT = 10.0


class TestRawDataLifetime(SyncSmokeTest):
    """An array made from raw_data stays valid after its sensor data is dropped."""

    def spawn_sensors(self):
        library = self.world.get_blueprint_library()
        spawn_point = self.world.get_map().get_spawn_points()[0]
        transform = carla.Transform(spawn_point.location + carla.Location(z=1.6), spawn_point.rotation)
        self.sensors = {}
        self.queues = {}
        for sensor_id in ('sensor.camera.rgb', 'sensor.lidar.ray_cast', 'sensor.camera.optical_flow'):
            bp = library.find(sensor_id)
            if bp.has_attribute('image_size_x'):
                bp.set_attribute('image_size_x', str(WIDTH))
                bp.set_attribute('image_size_y', str(HEIGHT))
            sensor = self.world.spawn_actor(bp, transform)
            queue = Queue()
            sensor.listen(queue.put)
            self.sensors[sensor_id] = sensor
            self.queues[sensor_id] = queue

    def destroy_sensors(self):
        for sensor in self.sensors.values():
            sensor.stop()
            sensor.destroy()
        self.sensors = {}
        self.queues = {}

    def next_data(self, sensor_id):
        frame = self.world.tick()
        while True:
            data = self.queues[sensor_id].get(timeout=TIMEOUT)
            if data.frame >= frame:
                return data

    def test_color_coded_flow_temporary(self):
        print("TestRawDataLifetime.test_color_coded_flow_temporary")
        self.spawn_sensors()
        try:
            flow = self.next_data('sensor.camera.optical_flow')
            held = flow.get_color_coded_flow()
            ref = np.frombuffer(held.raw_data, np.uint8).copy()
            del held
            # The image from get_color_coded_flow() is dropped right after raw_data.
            view = np.frombuffer(flow.get_color_coded_flow().raw_data, np.uint8)
            # Allocate blocks of the same size to reuse the memory if it was freed.
            junk = [np.full(ref.size, 0xAA, np.uint8) for _ in range(4)]
            self.assertEqual(view.size, WIDTH * HEIGHT * 4)
            self.assertTrue(np.array_equal(view, ref), "color-coded flow array changed after its image was dropped")
            del junk
        finally:
            self.destroy_sensors()

    def test_array_outlives_sensor_data(self):
        print("TestRawDataLifetime.test_array_outlives_sensor_data")
        self.spawn_sensors()
        try:
            arrays = {}
            for sensor_id in self.sensors:
                data = self.next_data(sensor_id)
                array = np.frombuffer(data.raw_data, np.uint8)
                self.assertGreater(array.size, 0, "%s sent no data" % sensor_id)
                arrays[sensor_id] = (array, array.copy())
                del data
            for _ in range(10):
                self.world.tick()
            for sensor_id, (array, expected) in arrays.items():
                self.assertTrue(np.array_equal(array, expected), "%s array changed after its data was dropped" % sensor_id)
        finally:
            self.destroy_sensors()
