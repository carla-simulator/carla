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

def wait_for_frame(test, data_queue, frame, timeout=10.0):
    """Returns the queued (frame, ...) entry for the given frame, failing the test on a timeout."""
    while True:
        try:
            data = data_queue.get(True, timeout)
        except Empty:
            test.fail("No sensor data for frame %d within %.0f s." % (frame, timeout))
        if data[0] == frame:
            return data


def try_spawn_at_free_spawn_point(test, blueprint):
    """Spawns the blueprint at the first free spawn point, failing the test if none is free."""
    for spawn_point in test.world.get_map().get_spawn_points():
        actor = test.world.try_spawn_actor(blueprint, spawn_point)
        if actor is not None:
            return actor, spawn_point
    test.fail("Could not spawn %s at any spawn point." % blueprint.id)


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
                frames.append(wait_for_frame(self, data_queue, frame)[1])
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


class TestLidarFixedSeedOutput(SyncSmokeTest):
    """Each LiDAR type must produce identical per-channel counts and point bytes for the same seed and scene."""

    FRAMES = 10
    # Noise and drop-off enabled where supported, so the random sequence is covered too.
    BASE = {'channels': '32', 'range': '50', 'rotation_frequency': '20'}
    NOISE = {'noise_stddev': '0.02', 'dropoff_general_rate': '0.2', 'noise_seed': '4242'}
    LIDARS = [
        ('sensor.lidar.ray_cast', dict(BASE, points_per_second='100000', **NOISE)),
        ('sensor.lidar.ray_cast_semantic', dict(BASE, points_per_second='100000')),
        ('sensor.lidar.hss_lidar', dict(BASE, horizontal_resolution='0.2', **NOISE)),
    ]

    def record(self, bp_id, attributes):
        bp = self.world.get_blueprint_library().find(bp_id)
        for key, value in attributes.items():
            bp.set_attribute(key, value)
        tranf = self.world.get_map().get_spawn_points()[0]
        tranf.location.z += 3
        lidar = self.world.spawn_actor(bp, tranf)

        data_queue = Queue()
        lidar.listen(lambda data: data_queue.put(
            (data.frame, [data.get_point_count(c) for c in range(data.channels)], bytes(data.raw_data))))
        frames = []
        try:
            for _ in range(self.FRAMES):
                frame = self.world.tick()
                frames.append(wait_for_frame(self, data_queue, frame)[1:])
        finally:
            lidar.stop()
            lidar.destroy()
            self.world.tick()
        return frames

    def test_repeated_runs_match(self):
        print("TestLidarFixedSeedOutput.test_repeated_runs_match")
        for bp_id, attributes in self.LIDARS:
            first = self.record(bp_id, attributes)
            second = self.record(bp_id, attributes)
            self.assertEqual(len(first), len(second))
            for idx, ((counts_a, raw_a), (counts_b, raw_b)) in enumerate(zip(first, second)):
                self.assertTrue(sum(counts_a) > 0, "%s frame %d has no points." % (bp_id, idx))
                self.assertEqual(counts_a, counts_b, "%s frame %d: per-channel counts differ." % (bp_id, idx))
                self.assertEqual(raw_a, raw_b, "%s frame %d: point data differs." % (bp_id, idx))


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


class TestSemanticTags(SyncSmokeTest):
    """Semantic tags read from tagged components by the semantic LiDAR."""

    SEMANTIC_POINT = np.dtype([('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('cos', 'f4'),
                               ('object_idx', 'u4'), ('object_tag', 'u4')])

    def test_semantic_lidar_vehicle_tags(self):
        print("TestSemanticTags.test_semantic_lidar_vehicle_tags")
        bp_lib = self.world.get_blueprint_library()
        vehicle, spawn_point = try_spawn_at_free_spawn_point(self, bp_lib.find('vehicle.lincoln.mkz'))
        # Behind the vehicle, looking at it.
        behind = spawn_point.transform(carla.Location(x=-8.0, z=1.5))
        lidar_location = carla.Location(x=behind.x, y=behind.y, z=behind.z)
        bp = bp_lib.find('sensor.lidar.ray_cast_semantic')
        for key, value in {'channels': '32', 'range': '50', 'points_per_second': '200000',
                           'rotation_frequency': '20'}.items():
            bp.set_attribute(key, value)
        lidar = self.world.spawn_actor(bp, carla.Transform(lidar_location, spawn_point.rotation))

        data_queue = Queue()
        lidar.listen(lambda data: data_queue.put((data.frame, bytes(data.raw_data))))
        try:
            # Let the vehicle settle before reading the last frame.
            for _ in range(10):
                frame = self.world.tick()
            points = np.frombuffer(wait_for_frame(self, data_queue, frame)[1], dtype=self.SEMANTIC_POINT)
        finally:
            lidar.stop()
            lidar.destroy()
            vehicle.destroy()
            self.world.tick()

        valid_tags = {int(label) for label in carla.CityObjectLabel.values.values()}
        unknown_tags = set(np.unique(points['object_tag']).tolist()) - valid_tags
        self.assertEqual(len(unknown_tags), 0, "Unknown semantic tags: %s" % sorted(unknown_tags))

        vehicle_tags = points['object_tag'][points['object_idx'] == vehicle.id]
        self.assertTrue(len(vehicle_tags) > 0, "The semantic LiDAR did not hit the vehicle.")
        self.assertTrue(np.all(vehicle_tags == int(carla.CityObjectLabel.Car)),
                        "Vehicle points have tags %s, expected Car." % sorted(set(vehicle_tags.tolist())))
