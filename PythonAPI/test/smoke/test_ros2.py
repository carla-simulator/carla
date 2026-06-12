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
        ProcessDataFromSemanticLidar — the three sensor publish paths not covered
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

    def test_ros2_map_publish_on_reload(self):
        """World reload re-publishes the latched OpenDRIVE map without crashing.

        Every episode start publishes the map as a latched std_msgs/String on
        rt/carla/map (NotifyBeginEpisode -> ROS2::ProcessDataFromMap). A crash
        in the transient_local publisher or in the OpenDRIVE retrieval would
        abort the reload or stop the world from ticking afterwards.
        """
        self.world = self.client.reload_world()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        for _ in range(5):
            self.world.tick()

    def test_ros2_enable_disable_cycle(self):
        """Enable → tick → disable → tick → re-enable → tick: no crash or state leak.

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

            # Cycle 2 — re-enable
            sensor.enable_for_ros()
            self.world.tick()
            self.assertTrue(sensor.is_enabled_for_ros())
            for _ in range(5):
                self.world.tick()

        finally:
            sensor.destroy()

    def test_ros2_hero_vehicle_topics_lifecycle(self):
        """Hero vehicle register -> publish -> destroy -> re-register cycle.

        A hero vehicle triggers RegisterVehicle(), which now also creates the
        odometry, vehicle_status and latched vehicle_info publishers plus the
        latched map -> odom transform on rt/tf_static, and every tick publishes
        odometry/status/TF from CarlaEngine::PublishROS2VehicleState. Destroying
        the hero and spawning a new one with the same ros_name re-creates the
        same topics, so this exercises the DDS writer cleanup in
        UnregisterVehicle and the latched re-publish on re-registration. A crash
        in any of those paths would stop the world from ticking.
        """
        bp_lib = self.world.get_blueprint_library()
        vehicle_bp = bp_lib.filter('vehicle.lincoln.mkz_2017')[0]
        vehicle_bp.set_attribute('role_name', 'hero')
        vehicle_bp.set_attribute('ros_name', 'hero')
        spawn_point = self.world.get_map().get_spawn_points()[0]

        for _ in range(2):
            vehicle = self.world.spawn_actor(vehicle_bp, spawn_point)
            try:
                vehicle.apply_control(carla.VehicleControl(throttle=0.5))
                for _ in range(20):
                    self.world.tick()
            finally:
                vehicle.destroy()
            # Liveness check after the vehicle publishers are destroyed.
            for _ in range(3):
                self.world.tick()

    def test_ros2_traffic_lights_lifecycle(self):
        """Traffic light status on-change publish and per-episode info latch.

        With --ros2 enabled every post-tick gathers all traffic lights
        (CarlaEngine::PublishROS2TrafficLights): the info list publishes
        latched once per episode and the status list republishes only when a
        light changes state. Forcing state transitions exercises the on-change
        path; reloading the world exercises the per-episode info re-latch. A
        crash in any of those paths would stop the world from ticking.
        """
        lights = self.world.get_actors().filter('traffic.traffic_light')
        self.assertTrue(len(lights) > 0,
                        'expected traffic lights in the default map')

        # Let the first post-tick gather and latch the info list.
        for _ in range(5):
            self.world.tick()

        # Force state transitions to exercise the on-change status publish.
        light = lights[0]
        light.freeze(True)
        try:
            for state in (carla.TrafficLightState.Red,
                          carla.TrafficLightState.Green,
                          carla.TrafficLightState.Off):
                light.set_state(state)
                for _ in range(3):
                    self.world.tick()
        finally:
            light.freeze(False)

        # A reload starts a new episode, which re-gathers and re-latches the
        # info list on its first post-tick.
        self.world = self.client.reload_world()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        for _ in range(5):
            self.world.tick()

    def test_ros2_multi_sensor_publish(self):
        """4 sensors + hero vehicle: 100-tick stress run then sequential teardown.

        Replicates the ros2_native.py -f stack.json workload that triggered a
        CycloneDDS heap corruption (double-free from ddsi_serdata_unref after
        dds_writecdr) and a segfault when multiple publishers share the same DDS
        type name (TFMessage).  The hero role_name triggers RegisterVehicle()
        which creates 2 CycloneDDS subscribers.  Each sensor gets an internal
        CarlaTransformPublisher, so 4 sensors == 4 TF publishers all using the
        same type name.

        100 ticks provide stronger assurance than 30 against heap corruption.
        Sequential teardown with a tick between each destroy verifies the server
        remains alive as each publisher and subscriber is cleaned up.
        """
        bp_lib = self.world.get_blueprint_library()

        vehicle_bp = bp_lib.filter('vehicle.lincoln.mkz_2017')[0]
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
