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
