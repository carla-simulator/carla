# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import time
from queue import Queue, Empty


class TestROS2(SyncSmokeTest):
    """Smoke tests for the native ROS2 DDS publishing path.

    Verifies that the enable_for_ros/disable_for_ros API works, that sensors
    with ROS2 enabled produce Python callbacks normally, and that the server
    does not crash when publishing large-payload sensor data (Image, PointCloud2)
    over DDS.

    No ROS2 subscriber is needed: the tests exercise the full server-side
    publish path (PublisherImpl -> FastDDSPublisherMiddleware ->
    GenericCdrPubSubType::serialize()) without verifying that data arrives at
    an external ROS2 node.
    """

    def test_ros2_api(self):
        """enable_for_ros/disable_for_ros/is_enabled_for_ros work correctly."""
        bp_lib = self.world.get_blueprint_library()
        camera_bp = bp_lib.filter('sensor.camera.rgb')[0]
        camera_bp.set_attribute('ros_name', 'test_camera_api')

        spawn_point = self.world.get_map().get_spawn_points()[0]
        sensor = self.world.spawn_actor(camera_bp, spawn_point)
        try:
            self.assertFalse(sensor.is_enabled_for_ros(),
                             'ROS2 should be disabled by default')

            sensor.enable_for_ros()
            # enable_for_ros() is an async RPC call. Tick once so the server
            # processes the command before querying the state.
            self.world.tick()
            self.assertTrue(sensor.is_enabled_for_ros(),
                            'ROS2 should be enabled after enable_for_ros()')

            for _ in range(5):
                self.world.tick()

            sensor.disable_for_ros()
            # Same reason: tick to flush the disable command before querying.
            self.world.tick()
            self.assertFalse(sensor.is_enabled_for_ros(),
                             'ROS2 should be disabled after disable_for_ros()')
        finally:
            sensor.destroy()

    def test_ros2_sensor_publish(self):
        """Camera and LiDAR publish over DDS without server crash.

        Spawns an RGB camera (produces large Image payloads) and a LiDAR
        (produces large PointCloud2 payloads), enables ROS2 on both, ticks
        20 frames, and verifies that Python sensor callbacks still fire. A
        server crash or DDS serialization failure would prevent the world
        from ticking correctly and cause a timeout here.
        """
        bp_lib = self.world.get_blueprint_library()

        camera_bp = bp_lib.filter('sensor.camera.rgb')[0]
        camera_bp.set_attribute('image_size_x', '800')
        camera_bp.set_attribute('image_size_y', '600')
        camera_bp.set_attribute('ros_name', 'test_camera')

        lidar_bp = bp_lib.filter('sensor.lidar.ray_cast')[0]
        lidar_bp.set_attribute('range', '50')
        lidar_bp.set_attribute('ros_name', 'test_lidar')

        vehicle_bp = self.world.get_blueprint_library().filter('vehicle.lincoln.mkz_2017')[0]
        spawn_point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(vehicle_bp, spawn_point)

        camera_offset = carla.Transform(carla.Location(x=1.5, z=2.4))
        lidar_offset = carla.Transform(carla.Location(x=0.0, z=2.8))

        camera = self.world.spawn_actor(camera_bp, camera_offset, attach_to=vehicle)
        lidar = self.world.spawn_actor(lidar_bp, lidar_offset, attach_to=vehicle)

        camera_queue = Queue()
        lidar_queue = Queue()

        camera.listen(lambda data: camera_queue.put(1))
        lidar.listen(lambda data: lidar_queue.put(1))

        try:
            camera.enable_for_ros()
            lidar.enable_for_ros()
            # enable_for_ros() is async. Tick once to flush before querying state.
            self.world.tick()

            self.assertTrue(camera.is_enabled_for_ros())
            self.assertTrue(lidar.is_enabled_for_ros())

            received_camera = 0
            received_lidar = 0
            for _ in range(19):  # 1 tick already consumed by flush above
                self.world.tick()
                try:
                    camera_queue.get(timeout=5.0)
                    received_camera += 1
                except Empty:
                    pass
                try:
                    lidar_queue.get(timeout=5.0)
                    received_lidar += 1
                except Empty:
                    pass

            self.assertGreaterEqual(received_camera, 5,
                'Camera callback should fire while ROS2 is enabled')
            self.assertGreaterEqual(received_lidar, 5,
                'LiDAR callback should fire while ROS2 is enabled')
        finally:
            camera.destroy()
            lidar.destroy()
            vehicle.destroy()
