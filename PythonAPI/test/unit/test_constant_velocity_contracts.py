# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual navigation/detection/PID execution with native maps and inert actor RPCs."""

import fnmatch
import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest

import carla

SOURCE_ROOT = Path(os.environ.get("CARLA_CV_CONTRACT_TEST_ROOT", Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(SOURCE_ROOT / "PythonAPI/carla"))
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent  # pylint: disable=wrong-import-position
from agents.navigation.local_planner import RoadOption  # pylint: disable=wrong-import-position


class Actors(list):
    """Remote actor-list boundary with the real filter semantics."""

    def filter(self, pattern):
        """Select actor types without replacing detection or geometry."""
        return Actors(actor for actor in self if fnmatch.fnmatchcase(actor.type_id, pattern))


class Vehicle:
    """Actor RPC boundary; all transforms, velocities and boxes are native values."""
    type_id = 'vehicle.test'

    def __init__(self, actor_id, world, longitudinal, speed, offset=0.0):
        self.id = actor_id
        self.world = world
        self.transform = world.native_map.get_waypoint_xodr(1, -1, longitudinal).transform
        self.transform.location.y += offset
        self.velocity = carla.Vector3D(speed, 0, 0)
        self.commands = []
        if speed < 0:
            self.transform.rotation.yaw += 180

    @property
    def bounding_box(self):
        """Real Actor bindings return a copy of the local box."""
        return carla.BoundingBox(carla.Location(), carla.Vector3D(1, 0.5, 0.5))

    def get_transform(self):
        """Return a native value copy as the real RPC does."""
        return carla.Transform(self.transform.location, self.transform.rotation)

    def get_location(self):
        """Return a native location value."""
        return self.get_transform().location

    def get_world(self):
        """Return the remote-world boundary."""
        return self.world

    def get_velocity(self):
        """Return actual native vector data for speed projection and PID."""
        return self.velocity

    def get_control(self):
        """Return a native neutral control for PID initialization."""
        return carla.VehicleControl()

    def get_speed_limit(self):
        """Provide an ordinary positive map speed limit in km/h."""
        return 50.0

    def disable_constant_velocity(self):
        """Keep the remote motor-disable endpoint inert."""

    def enable_constant_velocity(self, velocity):
        """Record the real agent's outgoing native velocity command."""
        self.commands.append(velocity)


class World:
    """Real native lane geometry with inert world/sensor RPC endpoints."""

    def __init__(self):
        xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
               '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
               '</geoReference></header><road id="1" name="straight" length="100" junction="-1" rule="RHT">'
               '<planView><geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry></planView>'
               '<lanes><laneSection s="0"><center><lane id="0" type="none"/></center><right>'
               '<lane id="-1" type="driving" level="false"><width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
               '</lane></right></laneSection></lanes></road></OpenDRIVE>')
        self.native_map = carla.Map('straight-obstacle', xml)
        self.actors = Actors()

    def get_map(self):
        """Return the actual parsed OpenDrive map."""
        return self.native_map

    @staticmethod
    def get_snapshot():
        """Provide a valid simulated time for stop/restart bookkeeping."""
        return SimpleNamespace(timestamp=SimpleNamespace(elapsed_seconds=1.0))

    def get_actors(self):
        """Return actor handles without changing the actual agent detector."""
        return self.actors

    @staticmethod
    def get_blueprint_library():
        """Only the collision sensor spawn crosses this inert boundary."""
        return SimpleNamespace(find=lambda name: SimpleNamespace(id=name))

    @staticmethod
    def spawn_actor(*_args, **_kwargs):
        """Keep collision callback registration inert; these tests do not collide."""
        return SimpleNamespace(listen=lambda callback: None, destroy=lambda: True)


class TestConstantVelocityContracts(unittest.TestCase):
    """Check motor output against actual planner lifecycle and speed targets."""

    def agent(self, options=None, speed_limit=10.0):
        """Construct the real complete agent without replacing either planner."""
        world = World()
        vehicle = Vehicle(1, world, 10.0, 5.0)
        vehicle.get_speed_limit = lambda: speed_limit
        world.actors.append(vehicle)
        agent = ConstantVelocityAgent(vehicle, target_speed=18, opt_dict=options or {}, map_inst=world.native_map)
        self.addCleanup(agent.destroy_sensor)
        return agent, vehicle, world

    def drive_plan_to_end(self, agent, vehicle, world):
        """Advance inert RPC positions along real native waypoints, without faking planner state."""
        plan = [world.native_map.get_waypoint_xodr(1, -1, value) for value in (20.0, 30.0, 40.0)]
        agent.set_global_plan([(point, RoadOption.LANEFOLLOW) for point in plan])
        for point in plan:
            vehicle.transform = point.transform
            control = agent.run_step()
        self.assertTrue(agent.done())
        self.assertEqual(control.brake, 1.0)

    def test_completed_route_stops_forced_motion(self):
        """The final planner brake must not be overwritten by forward velocity."""
        agent, vehicle, world = self.agent()
        self.drive_plan_to_end(agent, vehicle, world)
        self.assertEqual(vehicle.commands[-1].x, 0.0)

    def test_explicitly_cleared_route_stops_forced_motion(self):
        """An empty valid plan has the same terminal-stop contract."""
        agent, vehicle, _ = self.agent()
        agent.set_global_plan([])
        self.assertTrue(agent.done())
        control = agent.run_step()
        self.assertEqual(control.brake, 1.0)
        self.assertEqual(vehicle.commands[-1].x, 0.0)

    def test_new_route_resumes_after_terminal_stop(self):
        """Stopping a completed plan must not permanently disable the agent."""
        agent, vehicle, world = self.agent()
        self.drive_plan_to_end(agent, vehicle, world)
        self.assertEqual(vehicle.commands[-1].x, 0.0)
        next_point = world.native_map.get_waypoint_xodr(1, -1, 60.0)
        agent.set_global_plan([(next_point, RoadOption.LANEFOLLOW)])
        agent.run_step()
        self.assertFalse(agent.done())
        self.assertEqual(vehicle.commands[-1].x, 5.0)

    def test_runtime_speed_limits_reach_the_forced_motor(self):
        """Dynamic road limits must control forced speed as well as wheel PID."""
        agent, vehicle, _ = self.agent()
        agent.follow_speed_limits(True)
        for limit in (10.0, 20.0, 7.0):
            vehicle.get_speed_limit = lambda limit=limit: limit
            agent.run_step()
            self.assertAlmostEqual(vehicle.commands[-1].x, limit / 3.6, places=6)

    def test_constructor_follow_limit_option_reaches_motor_on_step(self):
        """Existing LocalPlanner options have the same dynamic effect."""
        agent, vehicle, _ = self.agent({'follow_speed_limits': True})
        self.assertAlmostEqual(vehicle.commands[-1].x, 10.0 / 3.6, places=6)
        agent.run_step()
        self.assertAlmostEqual(vehicle.commands[-1].x, 10.0 / 3.6, places=6)

    def test_restart_uses_the_current_enabled_speed_limit(self):
        """Manual restart must not briefly reapply an older road limit."""
        agent, vehicle, _ = self.agent({'follow_speed_limits': True})
        agent.stop_constant_velocity()
        vehicle.get_speed_limit = lambda: 20.0
        agent.restart_constant_velocity()
        self.assertAlmostEqual(vehicle.commands[-1].x, 20.0 / 3.6, places=6)

    def test_turning_limits_off_restores_explicit_target_control(self):
        """A later explicit target remains effective when automatic limits are off."""
        agent, vehicle, _ = self.agent()
        agent.follow_speed_limits(True)
        agent.run_step()
        self.assertAlmostEqual(vehicle.commands[-1].x, 10.0 / 3.6, places=6)
        agent.follow_speed_limits(False)
        agent.set_target_speed(36.0)
        agent.run_step()
        self.assertAlmostEqual(vehicle.commands[-1].x, 10.0)

    def test_ordinary_nonterminal_fixed_speed_control(self):
        """A regular route without dynamic limits retains the fixed5m/s target."""
        agent, vehicle, _ = self.agent()
        agent.run_step()
        self.assertFalse(agent.done())
        self.assertEqual(vehicle.commands[-1].x, 5.0)

    def test_disabled_limit_option_keeps_explicit_target_control(self):
        """Reading a lower speed limit must not affect an opt-out driver."""
        agent, vehicle, _ = self.agent({'follow_speed_limits': False})
        agent.run_step()
        self.assertEqual(vehicle.commands[-1].x, 5.0)


if __name__ == '__main__':
    unittest.main()
