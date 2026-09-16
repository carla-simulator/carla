# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
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

    No ROS2 subscriber is needed: the tests exercise the full server-side publish
    path (PublisherImpl -> IPublisherMiddleware -> GenericCdrPubSubType::serialize
    for FastDDS, or the CycloneDDS sertype path) without verifying that data
    arrives at an external ROS2 node.

    The tests are middleware-agnostic: the server selects the DDS middleware at
    launch via --rmw=<fastdds|cyclonedds>, so the same suite validates whichever
    middleware the server was started with. Run it once per middleware.

    Requires a server launched with --ros2. The base tearDown is overridden to
    reload Town10HD_Opt because Town03 is not present in the packaged build.
    """

    def tearDown(self):
        # Restore the synchronous world settings applied by SyncSmokeTest.setUp,
        # then reload a map that ships with the packaged build. The base
        # SmokeTest.tearDown loads Town03, which is absent from the package, so
        # this override does not call super().tearDown().
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        # workaround: give time to the engine to clean memory after loading
        time.sleep(5)
        self.world = None
        self.client = None

    def test_ros2_api(self):
        """enable_for_ros/disable_for_ros/is_enabled_for_ros work correctly."""
        bp_lib = self.world.get_blueprint_library()
        camera_bp = bp_lib.filter('sensor.camera.rgb')[0]
        camera_bp.set_attribute('ros_name', 'test_camera_api')

        spawn_point = self.world.get_map().get_spawn_points()[0]
        sensor = self.world.spawn_actor(camera_bp, spawn_point)
        try:
            # A server launched with --ros2 defaults ROS2TopicVisibility to true,
            # so a freshly spawned sensor may already be enabled for ROS. Drive the
            # toggle from a known state rather than assuming the initial value.
            sensor.disable_for_ros()
            self.world.tick()
            self.assertFalse(sensor.is_enabled_for_ros(),
                             'ROS2 should be disabled after disable_for_ros()')

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

        vehicle_bp = self.world.get_blueprint_library().filter('vehicle.lincoln.mkz')[0]
        spawn_point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(vehicle_bp, spawn_point)

        camera_offset = carla.Transform(carla.Location(x=1.5, z=2.4))
        lidar_offset = carla.Transform(carla.Location(x=0.0, z=2.8))

        camera = None
        lidar = None
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
            if camera is not None:
                camera.destroy()
            if lidar is not None:
                lidar.destroy()
            vehicle.destroy()

    def test_ros2_additional_sensors(self):
        """Radar, DVS camera, and semantic LiDAR publish paths.

        Exercises ProcessDataFromRadar, ProcessDataFromDVS, and
        ProcessDataFromSemanticLidar, the three sensor publish paths not covered
        by test_ros2_sensor_publish or test_ros2_multi_sensor_publish.
        """
        bp_lib = self.world.get_blueprint_library()
        spawn_point = self.world.get_map().get_spawn_points()[0]

        radar_bp = bp_lib.filter('sensor.other.radar')[0]
        radar_bp.set_attribute('ros_name', 'test_radar')

        dvs_bp = bp_lib.filter('sensor.camera.dvs')[0]
        dvs_bp.set_attribute('ros_name', 'test_dvs')

        sem_lidar_bp = bp_lib.filter('sensor.lidar.ray_cast_semantic')[0]
        sem_lidar_bp.set_attribute('ros_name', 'test_sem_lidar')

        radar = self.world.spawn_actor(radar_bp, spawn_point)
        dvs = self.world.spawn_actor(dvs_bp, spawn_point)
        sem_lidar = self.world.spawn_actor(sem_lidar_bp, spawn_point)

        try:
            radar.enable_for_ros()
            dvs.enable_for_ros()
            sem_lidar.enable_for_ros()

            self.world.tick()

            self.assertTrue(radar.is_enabled_for_ros())
            self.assertTrue(dvs.is_enabled_for_ros())
            self.assertTrue(sem_lidar.is_enabled_for_ros())

            for _ in range(19):
                self.world.tick()

        finally:
            radar.destroy()
            dvs.destroy()
            sem_lidar.destroy()

    def test_ros2_enable_disable_cycle(self):
        """Enable -> tick -> disable -> tick -> re-enable -> tick: no crash or state leak.

        Verifies that calling enable_for_ros() and disable_for_ros() multiple
        times on the same sensor does not leave the DDS publisher in a bad state.
        """
        bp_lib = self.world.get_blueprint_library()
        camera_bp = bp_lib.filter('sensor.camera.rgb')[0]
        camera_bp.set_attribute('ros_name', 'test_cycle')
        spawn_point = self.world.get_map().get_spawn_points()[0]
        sensor = self.world.spawn_actor(camera_bp, spawn_point)

        try:
            # Cycle 1
            sensor.enable_for_ros()
            self.world.tick()
            self.assertTrue(sensor.is_enabled_for_ros())
            for _ in range(5):
                self.world.tick()

            sensor.disable_for_ros()
            self.world.tick()
            self.assertFalse(sensor.is_enabled_for_ros())
            for _ in range(5):
                self.world.tick()

            # Cycle 2: re-enable
            sensor.enable_for_ros()
            self.world.tick()
            self.assertTrue(sensor.is_enabled_for_ros())
            for _ in range(5):
                self.world.tick()

        finally:
            sensor.destroy()

    def test_ros2_multi_sensor_publish(self):
        """4 sensors + hero vehicle: 100-tick stress run then sequential teardown.

        Replicates the ros2_native.py -f stack.json workload that stresses the
        DDS publish path. The hero role_name triggers RegisterVehicle(), which
        creates subscribers. Each sensor gets an internal transform publisher, so
        4 sensors means 4 TF publishers all using the same DDS type name.

        100 ticks provide stronger assurance than 30 against heap corruption.
        Sequential teardown with a tick between each destroy verifies the server
        remains alive as each publisher and subscriber is cleaned up.
        """
        bp_lib = self.world.get_blueprint_library()

        vehicle_bp = bp_lib.filter('vehicle.lincoln.mkz')[0]
        vehicle_bp.set_attribute('role_name', 'hero')
        vehicle_bp.set_attribute('ros_name', 'hero')
        spawn_point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(vehicle_bp, spawn_point)

        sensor_configs = [
            ('sensor.camera.rgb',     carla.Transform(carla.Location(x=1.5, z=2.4))),
            ('sensor.lidar.ray_cast', carla.Transform(carla.Location(x=0.0, z=2.8))),
            ('sensor.other.gnss',     carla.Transform(carla.Location(x=1.0, z=2.0))),
            ('sensor.other.imu',      carla.Transform(carla.Location(x=2.0, z=2.0))),
        ]

        sensors = []
        try:
            for bp_name, offset in sensor_configs:
                bp = bp_lib.filter(bp_name)[0]
                bp.set_attribute('ros_name', bp_name.split('.')[-1])
                sensor = self.world.spawn_actor(bp, offset, attach_to=vehicle)
                sensor.enable_for_ros()
                sensors.append(sensor)

            # Publish phase: 100 ticks to stress-test DDS publishing.
            for _ in range(100):
                self.world.tick()

            # Teardown phase: destroy sensors one by one.
            # world.tick() after each destroy verifies the server is still alive.
            for sensor in sensors:
                sensor.destroy()
                self.world.tick()
            sensors = []

            # Final liveness check after all sensors and subscribers are gone.
            self.world.tick()

        finally:
            for sensor in sensors:
                sensor.destroy()
            vehicle.destroy()
