# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# NOT part of smoke_test_list.txt: every other smoke test shares one already
# running, normally rendering server (see PythonAPI/test/smoke/__init__.py,
# TESTING_ADDRESS). This module needs a server started with -nullrhi and no
# Multi-GPU secondary connected, so it must be run on its own against a
# separately launched server, e.g.:
#
#   ./CarlaUnreal.sh -nullrhi -RenderOffScreen -carla-rpc-port=3654
#   python -m nose2 -v smoke.test_nullrhi_primary

from . import SyncSmokeTest

import carla
import time

class TestNullRhiPrimary(SyncSmokeTest):
    def tearDown(self):
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            self.world.tick()
        self.settings = None
        self.client.load_world('Town10HD_Opt')
        time.sleep(5)
        self.world = None
        self.client = None

    def test_camera_listen_refused_without_secondary(self):
        bp_camera = self.world.get_blueprint_library().find('sensor.camera.rgb')
        camera = self.world.spawn_actor(bp_camera, carla.Transform())
        try:
            with self.assertRaisesRegex(RuntimeError, r'-nullrhi'):
                camera.listen(lambda image: None)
        finally:
            camera.destroy()

    def test_cpu_sensor_still_delivers(self):
        bp_imu = self.world.get_blueprint_library().find('sensor.other.imu')
        imu = self.world.spawn_actor(bp_imu, carla.Transform())

        received = []
        imu.listen(lambda data: received.append(data))

        for _i in range(0, 20):
            self.world.tick()

        imu.destroy()

        self.assertGreater(len(received), 0, "IMU sensor produced no data on a -nullrhi primary")

    def test_cpu_sensors_deliver_without_gpu(self):
        """LiDAR, semantic LiDAR, radar and GNSS deliver on a -nullrhi server.

        They are ray casts and physics queries, not scene captures, so they need
        no GPU (Docs/core_rendering.md, "Running without a GPU"). The IMU is
        covered above."""
        library = self.world.get_blueprint_library()
        spawn_point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(library.filter('vehicle.*')[0], spawn_point)
        sensors = []
        counts = {}
        try:
            for sensor_id in ('sensor.lidar.ray_cast', 'sensor.lidar.ray_cast_semantic',
                              'sensor.other.radar', 'sensor.other.gnss'):
                sensor = self.world.spawn_actor(
                    library.find(sensor_id), carla.Transform(carla.Location(z=2.0)), attach_to=vehicle)
                sensors.append(sensor)
                counts[sensor_id] = 0
                sensor.listen(lambda data, key=sensor_id: counts.__setitem__(key, counts[key] + 1))
            for _ in range(40):
                self.world.tick()
            time.sleep(1.0)
            for sensor_id, count in counts.items():
                self.assertGreater(count, 0, f'{sensor_id} delivered nothing on a -nullrhi server')
        finally:
            for sensor in sensors:
                sensor.stop()
                sensor.destroy()
            vehicle.destroy()
