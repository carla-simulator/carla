# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Constant velocity initialization with native maps and inert actor RPCs."""

import os
import sys
from types import SimpleNamespace
import unittest

import carla

sys.path.insert(0, os.path.abspath(os.path.join(
    os.path.dirname(__file__), "..", "..", "carla")))

from agents.navigation.constant_velocity_agent import ConstantVelocityAgent


class _Actors(list):
    def filter(self, _pattern):
        return []


class TestConstantVelocityAgent(unittest.TestCase):
    @staticmethod
    def _agent(target_speed):
        xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
               '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
               '</geoReference></header>'
               '<road id="1" name="straight" length="100" junction="-1">'
               '<planView><geometry s="0" x="0" y="0" hdg="0" length="100">'
               '<line/></geometry></planView><lanes><laneSection s="0">'
               '<center><lane id="0" type="none"/></center><right>'
               '<lane id="-1" type="driving"><width sOffset="0" a="3.5" '
               'b="0" c="0" d="0"/></lane></right></laneSection></lanes>'
               '</road></OpenDRIVE>')
        native_map = carla.Map("constant-velocity-test", xml)
        sensor = SimpleNamespace(listen=lambda _callback: None, destroy=lambda: True)
        world = SimpleNamespace(
            get_map=lambda: native_map,
            get_actors=_Actors,
            get_blueprint_library=lambda: SimpleNamespace(find=lambda _name: object()),
            spawn_actor=lambda *_args, **_kwargs: sensor,
            get_snapshot=lambda: SimpleNamespace(timestamp=SimpleNamespace(elapsed_seconds=1.0)))
        commands = []
        vehicle = SimpleNamespace(
            get_world=lambda: world,
            get_location=lambda: native_map.get_waypoint_xodr(1, -1, 10).transform.location,
            get_control=carla.VehicleControl,
            get_velocity=carla.Vector3D,
            enable_constant_velocity=commands.append,
            disable_constant_velocity=lambda: None)
        agent = ConstantVelocityAgent(vehicle, target_speed, opt_dict={}, map_inst=native_map)
        return agent, commands

    def test_initial_velocity_uses_metres_per_second(self):
        for target_kmh in (0.0, 20.0, 36.0, 72.0):
            with self.subTest(target_kmh=target_kmh):
                agent, commands = self._agent(target_kmh)
                self.assertEqual(len(commands), 1)
                self.assertAlmostEqual(commands[0].x, target_kmh / 3.6, places=5)
                self.assertEqual(commands[0].y, 0.0)
                self.assertEqual(commands[0].z, 0.0)
                self.assertEqual(agent.get_local_planner()._target_speed, target_kmh)

    def test_restart_and_speed_changes_keep_the_initial_units(self):
        agent, commands = self._agent(36.0)
        agent.stop_constant_velocity()
        agent.restart_constant_velocity()
        self.assertEqual(commands[0], commands[-1])
        agent.set_target_speed(72.0)
        agent.restart_constant_velocity()
        self.assertAlmostEqual(commands[-1].x, 20.0)
        self.assertEqual(agent.get_local_planner()._target_speed, 72.0)


if __name__ == "__main__":
    unittest.main()
