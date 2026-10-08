# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest
from . import SmokeTest

import carla
import time
import math
import numpy as np
from enum import Enum
from queue import Queue
from queue import Empty

class SensorType(Enum):
    LIDAR = 1
    SEMLIDAR = 2
    HSSLIDAR = 3

class Sensor():
    def __init__(self, test, sensor_type, attributes, sensor_name = None, sensor_queue = None):
        self.test = test
        self.world = test.world
        self.sensor_type = sensor_type
        self.error = None
        self.name = sensor_name
        self.queue = sensor_queue
        self.curr_det_pts = 0

        if self.sensor_type == SensorType.LIDAR:
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast")[0]
        elif self.sensor_type == SensorType.SEMLIDAR:
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast_semantic")[0]
        elif self.sensor_type == SensorType.HSSLIDAR:
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.hss_lidar")[0]
        else:
            self.error = "Unknown type of sensor"

        for key in attributes:
            self.bp_sensor.set_attribute(key, attributes[key])

        tranf = self.world.get_map().get_spawn_points()[0]
        tranf.location.z += 3
        self.sensor = self.world.spawn_actor(self.bp_sensor, tranf)
        self.sensor.listen(lambda sensor_data: self.callback(sensor_data, self.name, self.queue))

    def destroy(self):
        self.sensor.destroy()

    def callback(self, sensor_data, sensor_name=None, queue=None):
        # Compute the total sum of points adding all channels
        total_channel_points = 0
        for i in range(0, sensor_data.channels):
            total_channel_points += sensor_data.get_point_count(i)

        # Total points iterating in the LidarMeasurement
        total_detect_points = 0
        for _detection in sensor_data:
            total_detect_points += 1

        # Point cloud used with numpy from the raw data
        if self.sensor_type in (SensorType.LIDAR, SensorType.HSSLIDAR):
            points = np.frombuffer(sensor_data.raw_data, dtype=np.dtype('f4'))
            points = np.reshape(points, (int(points.shape[0] / 4), 4))
            total_np_points = points.shape[0]
            self.curr_det_pts = total_np_points
        elif self.sensor_type == SensorType.SEMLIDAR:
            data = np.frombuffer(sensor_data.raw_data, dtype=np.dtype([
                ('x', np.float32), ('y', np.float32), ('z', np.float32),
                ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
            points = np.array([data['x'], data['y'], data['z']]).T
            total_np_points = points.shape[0]
            self.curr_det_pts = total_np_points
        else:
            self.error = "It should never reach this point"
            return

        if total_np_points != total_detect_points:
            self.error = "The number of points of the raw data does not match with the LidarMeasurament array"

        if total_channel_points != total_detect_points:
            self.error = "The sum of the points of all channels does not match with the LidarMeasurament array"

        # Add option to synchronization queue
        if queue is not None:
            queue.put((sensor_data.frame, sensor_name, self.curr_det_pts))

    def is_correct(self):
        return self.error is None

    def get_current_detection_points(self):
        return self.curr_det_pts

class TestSyncLidar(SyncSmokeTest):
    def test_lidar_point_count(self):
        print("TestSyncLidar.test_lidar_point_count")
        sensors = []

        att_l00={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '20'}
        att_l01={'channels' : '64', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '5'}
        att_l02={'channels' : '64', 'dropoff_intensity_limit': '1.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.LIDAR, att_l00))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l01))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l02))

        for _ in range(0, 10):
            self.world.tick()
        time.sleep(0.5)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


    def test_semlidar_point_count(self):
        print("TestSyncLidar.test_semlidar_point_count")
        sensors = []

        att_s00 = {'channels' : '64', 'range' : '100', 'points_per_second': '100000',
          'rotation_frequency': '20'}
        att_s01 = {'channels' : '32', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s00))
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s01))

        for _ in range(0, 10):
            self.world.tick()
        time.sleep(0.5)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


    def test_hsslidar_point_count(self):
        print("TestSyncLidar.test_hsslidar_point_count")
        sensors = []

        att_h00 = {'channels' : '64', 'range' : '100', 'horizontal_resolution': '0.2',
          'rotation_frequency': '20'}
        att_h01 = {'channels' : '32', 'range' : '200', 'horizontal_resolution': '0.1',
          'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.HSSLIDAR, att_h00))
        sensors.append(Sensor(self, SensorType.HSSLIDAR, att_h01))

        for _ in range(0, 10):
            self.world.tick()
        time.sleep(0.5)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


class TestASyncLidar(SmokeTest):
    def test_lidar_point_count(self):
        print("TestASyncLidar.test_lidar_point_count")
        sensors = []

        att_l00={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '20'}
        att_l01={'channels' : '64', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '5'}
        att_l02={'channels' : '64', 'dropoff_intensity_limit': '1.0', 'dropoff_general_rate': '0.0',
          'range' : '50', 'points_per_second': '100000', 'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.LIDAR, att_l00))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l01))
        sensors.append(Sensor(self, SensorType.LIDAR, att_l02))

        time.sleep(3.0)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)


    def test_semlidar_point_count(self):
        print("TestASyncLidar.test_semlidar_point_count")
        sensors = []

        att_s00 = {'channels' : '64', 'range' : '100', 'points_per_second': '100000',
          'rotation_frequency': '20'}
        att_s01 = {'channels' : '32', 'range' : '200', 'points_per_second': '500000',
          'rotation_frequency': '50'}

        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s00))
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_s01))

        time.sleep(3.0)

        for sensor in sensors:
            sensor.destroy()

        for sensor in sensors:
            if not sensor.is_correct():
                self.fail(sensor.error)

class TestCompareLidars(SyncSmokeTest):
    def test_lidar_comparison(self):
        print("TestCompareLidars.test_lidar_comparison")
        sensors = []

        att_sem_lidar={'channels' : '64', 'range' : '200', 'points_per_second': '500000'}
        att_lidar_nod={'channels' : '64', 'dropoff_intensity_limit': '0.0', 'dropoff_general_rate': '0.0',
          'range' : '200', 'points_per_second': '500000'}
        att_lidar_def={'channels' : '64', 'range' : '200', 'points_per_second': '500000'}

        sensor_queue = Queue()
        sensors.append(Sensor(self, SensorType.SEMLIDAR, att_sem_lidar, "SemLidar", sensor_queue))
        sensors.append(Sensor(self, SensorType.LIDAR, att_lidar_nod, "LidarNoD", sensor_queue))
        sensors.append(Sensor(self, SensorType.LIDAR, att_lidar_def, "LidarDef", sensor_queue))

        for _ in range(0, 15):
            self.world.tick()

            data_sem_lidar = None
            data_lidar_nod = None
            data_lidar_def = None
            for _ in range(len(sensors)):
                data = sensor_queue.get(True, 10.0)
                if data[1] == "SemLidar":
                    data_sem_lidar = data
                elif data[1] == "LidarNoD":
                    data_lidar_nod = data
                elif data[1] == "LidarDef":
                    data_lidar_def = data
                else:
                    self.fail("It should never reach this point")

            # Check that frame number are correct
            self.assertEqual(data_sem_lidar[0], data_lidar_nod[0], "The frame numbers of LiDAR and SemLiDAR do not match.")
            self.assertEqual(data_sem_lidar[0], data_lidar_def[0], "The frame numbers of LiDAR and SemLiDAR do not match.")

            # The detections of the semantic lidar and the Lidar with no dropoff should have the same point count always
            self.assertEqual(data_sem_lidar[2], data_lidar_nod[2], "The point count of the detections of this frame of LiDAR(No dropoff) and SemLiDAR do not match.")

            # Default lidar should drop a minimum of 45% of the points so we check that but with a high tolerance to account for 'rare' cases
            if data_lidar_def[2] > 0.75 * data_sem_lidar[2]:
                self.fail("The point count of the default lidar should be much less than the Semantic Lidar point count.")

        time.sleep(1)
        for sensor in sensors:
            sensor.destroy()


class TestLidarBatchConsistency(SyncSmokeTest):
    """A LiDAR must produce the same data alone and when simulated in parallel with other LiDARs."""

    FRAMES = 10

    def record(self, attributes, extra_lidars=0):
        bp_lib = self.world.get_blueprint_library()
        tranf = self.world.get_map().get_spawn_points()[0]
        tranf.location.z += 3

        bp = bp_lib.find("sensor.lidar.ray_cast")
        for key in attributes:
            bp.set_attribute(key, attributes[key])
        lidar = self.world.spawn_actor(bp, tranf)
        others = [self.world.spawn_actor(bp_lib.find("sensor.lidar.ray_cast"), tranf) for _ in range(extra_lidars)]

        data_queue = Queue()
        lidar.listen(lambda data: data_queue.put((data.frame, bytes(data.raw_data))))
        for other in others:
            other.listen(lambda data: None)

        frames = []
        try:
            for _ in range(self.FRAMES):
                frame = self.world.tick()
                while True:
                    data = data_queue.get(True, 10.0)
                    if data[0] == frame:
                        frames.append(data[1])
                        break
        finally:
            for sensor in [lidar] + others:
                sensor.stop()
                sensor.destroy()
            self.world.tick()
        return frames

    def test_single_and_batch_match(self):
        print("TestLidarBatchConsistency.test_single_and_batch_match")
        # Noise and dropoff enabled so the random sequence is covered as well.
        attributes = {'channels' : '32', 'range' : '50', 'points_per_second': '100000',
          'rotation_frequency': '20', 'noise_stddev': '0.02', 'noise_seed': '4242'}

        single = self.record(attributes)
        batch = self.record(attributes, extra_lidars=2)

        self.assertEqual(len(single), len(batch))
        for idx, (a, b) in enumerate(zip(single, batch)):
            self.assertTrue(len(a) > 0, "Frame %d has no points." % idx)
            self.assertEqual(a, b, "Frame %d differs between the single and the batched LiDAR." % idx)

    def test_noise_seed(self):
        print("TestLidarBatchConsistency.test_noise_seed")
        attributes = {'channels' : '32', 'range' : '50', 'points_per_second': '100000',
          'rotation_frequency': '20', 'noise_stddev': '0.02'}

        seed_1 = self.record(dict(attributes, noise_seed='1'))
        seed_1_again = self.record(dict(attributes, noise_seed='1'))
        seed_2 = self.record(dict(attributes, noise_seed='2'))

        self.assertEqual(len(seed_1), len(seed_2))
        for idx, (a, b, c) in enumerate(zip(seed_1, seed_1_again, seed_2)):
            self.assertTrue(len(a) > 0, "Frame %d has no points." % idx)
            self.assertTrue(a == b, "Frame %d differs between two runs with the same noise_seed." % idx)
        self.assertTrue(any(a != c for a, c in zip(seed_1, seed_2)), "Different noise_seed values produced identical data.")


class TestLidarSensorTick(SyncSmokeTest):
    """LiDARs must honor sensor_tick when they are simulated by the LiDAR subsystem."""

    def test_lidar_sensor_tick(self):
        print("TestLidarSensorTick.test_lidar_sensor_tick")
        bp_lib = self.world.get_blueprint_library()
        sensor_tick = 1.0
        num_ticks = 50

        counts = {}
        sensors = []
        for bp_id in ["sensor.lidar.ray_cast", "sensor.lidar.ray_cast_semantic", "sensor.lidar.hss_lidar"]:
            bp = bp_lib.find(bp_id)
            bp.set_attribute("sensor_tick", str(sensor_tick))
            if bp.has_attribute("points_per_second"):
                bp.set_attribute("points_per_second", "10000")
            sensor = self.world.spawn_actor(bp, carla.Transform())
            counts[bp_id] = 0
            sensor.listen(lambda data, bp_id=bp_id: counts.__setitem__(bp_id, counts[bp_id] + 1))
            sensors.append(sensor)

        for _ in range(num_ticks):
            self.world.tick()
        time.sleep(1.0)

        for sensor in sensors:
            sensor.stop()
            sensor.destroy()

        dt = self.world.get_settings().fixed_delta_seconds
        expected = int(math.ceil(num_ticks * dt / sensor_tick))
        for bp_id, count in counts.items():
            self.assertEqual(count, expected, "%s does not match tick count" % bp_id)
