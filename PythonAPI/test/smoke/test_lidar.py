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
from enum import Enum, auto

class SensorType(Enum):
    LIDAR = auto()
    SEMLIDAR = auto()

class Sensor():
    def __init__(self, test, sensor_type, attributes):
        self.test = test
        self.world = test.world
        self.sensor_type = sensor_type
        self.error = None

        if self.sensor_type == SensorType.LIDAR:
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast")[0]
        elif self.sensor_type == SensorType.SEMLIDAR:
            self.bp_sensor = self.world.get_blueprint_library().filter("sensor.lidar.ray_cast_semantic")[0]
        else:
            self.error = "Unknown type of sensor"

        for key in attributes:
            self.bp_sensor.set_attribute(key, attributes[key])

        tranf = self.world.get_map().get_spawn_points()[0]
        self.sensor = self.world.spawn_actor(self.bp_sensor, tranf)
        self.sensor.listen(lambda sensor_data: self.callback(sensor_data))

    def destroy(self):
        self.sensor.destroy()

    def callback(self, sensor_data):
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
        elif self.sensor_type == SensorType.SEMLIDAR:
            data = np.frombuffer(sensor_data.raw_data, dtype=np.dtype([
                ('x', np.float32), ('y', np.float32), ('z', np.float32),
                ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
            points = np.array([data['x'], data['y'], data['z']]).T
            total_np_points = points.shape[0]
        else:
            self.error = "It should never reach this point"
            return

        if total_np_points != total_detect_points:
            self.error = "The number of points of the raw data does not match with the LidarMeasurament array"

        if total_channel_points != total_detect_points:
            self.error = "The sum of the points of all channels does not match with the LidarMeasurament array"

    def is_correct(self):
        return self.error is None

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
