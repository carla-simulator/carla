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

SOURCE_ROOT = Path(os.environ.get("CARLA_AGENT_OBSTACLE_TEST_ROOT", Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(SOURCE_ROOT / "PythonAPI/carla"))
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent  # pylint: disable=wrong-import-position
from agents.tools.misc import is_within_distance  # pylint: disable=wrong-import-position


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


class TestObstacleBoundaries(unittest.TestCase):
    """Separate angular detection boundaries from constant-velocity response."""

    def agent(self, obstacle_speed=None, offset=0.1):
        """Construct the complete real agent and its actual planners/PID."""
        world = World()
        vehicle = Vehicle(1, world, 10.0, 10.0)
        world.actors.append(vehicle)
        if obstacle_speed is not None:
            world.actors.append(Vehicle(2, world, 18.0, obstacle_speed, offset))
        agent = ConstantVelocityAgent(vehicle, target_speed=18, opt_dict={}, map_inst=world.native_map)
        self.addCleanup(agent.destroy_sensor)
        return agent, vehicle, world

    def test_documented_angle_endpoints_are_included(self):
        """The documented closed intervals include0,90 and180 degree rays."""
        for location, interval in ((carla.Location(5, 0, 0), [0, 90]),
                                   (carla.Location(0, 5, 0), [0, 90]),
                                   (carla.Location(-5, 0, 0), [90, 180])):
            with self.subTest(location=location, interval=interval):
                self.assertTrue(is_within_distance(carla.Transform(location), carla.Transform(), 10, interval))

    def test_outside_angle_distance_and_interior_controls(self):
        """Including boundaries keeps outside targets excluded."""
        reference = carla.Transform()
        self.assertFalse(is_within_distance(carla.Transform(carla.Location(-5, 0, 0)), reference, 10, [0, 90]))
        self.assertFalse(is_within_distance(carla.Transform(carla.Location(5, 1, 0)), reference, 10, [20, 40]))
        self.assertFalse(is_within_distance(carla.Transform(carla.Location(15, 1, 0)), reference, 10, [0, 90]))
        self.assertTrue(is_within_distance(carla.Transform(carla.Location(5, 1, 0)), reference, 10, [0, 90]))

    def test_actual_detector_sees_exactly_ahead_vehicle(self):
        """A real map/PID route and centered actor exercise BasicAgent detection."""
        agent, _, world = self.agent(obstacle_speed=2.0, offset=0.0)
        self.assertTrue(agent._vehicle_obstacle_detected(world.actors, 15)[0])  # pylint: disable=protected-access

    def check_hazard(self, obstacle_speed, expected):
        """Run real detection and the complete step; inspect the outgoing command."""
        agent, vehicle, world = self.agent(obstacle_speed)
        self.assertTrue(agent._vehicle_obstacle_detected(world.actors, 15)[0])  # pylint: disable=protected-access
        control = agent.run_step()
        self.assertIsInstance(control, carla.VehicleControl)
        self.assertAlmostEqual(vehicle.commands[-1].x, expected)

    def test_approaching_obstacle_does_not_command_reverse(self):
        """An approaching obstacle must slow this forward-only agent to zero."""
        self.check_hazard(-3.0, 0.0)

    def test_faster_lead_vehicle_does_not_raise_configured_speed(self):
        """A hazard response must not exceed the configured5m/s speed."""
        self.check_hazard(20.0, 5.0)

    def test_slower_lead_vehicle_control(self):
        """Ordinary2m/s car following remains unchanged."""
        self.check_hazard(2.0, 2.0)

    def test_stationary_obstacle_control(self):
        """A stationary actor still commands zero velocity."""
        self.check_hazard(0.0, 0.0)

    def test_clear_road_retains_configured_speed_control(self):
        """Without an obstacle the agent continues at its configured speed."""
        agent, vehicle, _ = self.agent()
        agent.run_step()
        self.assertAlmostEqual(vehicle.commands[-1].x, 5.0)


if __name__ == '__main__':
    unittest.main()
