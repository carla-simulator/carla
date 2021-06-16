# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
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
        if self.sensor_type == SensorType.LIDAR:
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

    def get_current_detection_points():
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
