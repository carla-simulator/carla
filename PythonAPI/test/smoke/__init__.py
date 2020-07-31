# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import unittest

try:
    sys.path.append(glob.glob('../../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


TESTING_ADDRESS = ('localhost', 3654)


class SmokeTest(unittest.TestCase):
    def setUp(self):
        self.testing_address = TESTING_ADDRESS
        self.client = carla.Client(*TESTING_ADDRESS)
        self.client.set_timeout(60.0)

    def tearDown(self):
        self.client = None


class SyncSmokeTest(SmokeTest):
    def setUp(self):
        super(SyncSmokeTest, self).setUp()
        self.world = self.client.get_world()
        self.settings = self.world.get_settings()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

    def tearDown(self):
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.world = None
        super(SyncSmokeTest, self).tearDown()
