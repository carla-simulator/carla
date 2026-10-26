# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual navigation/detection/PID execution with native maps and inert actor RPCs."""

# The detector is intentionally tested through its existing private entry point.
# pylint: disable=protected-access

import fnmatch
import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest

import carla

SOURCE_ROOT = Path(os.environ.get("CARLA_AGENT_FOOTPRINT_TEST_ROOT", Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(SOURCE_ROOT / "PythonAPI/carla"))
from agents.navigation.basic_agent import BasicAgent  # pylint: disable=wrong-import-position
from agents.navigation.behavior_agent import BehaviorAgent  # pylint: disable=wrong-import-position
from agents.navigation.local_planner import RoadOption  # pylint: disable=wrong-import-position


class Actors(list):
    """Remote actor-list boundary with the real filter semantics."""

    def filter(self, pattern):
        """Select actor types without replacing detection or geometry."""
        return Actors(actor for actor in self if fnmatch.fnmatchcase(actor.type_id, pattern))


class Vehicle:
    """Actor RPC boundary; all transforms, velocities and boxes are native values."""
    type_id = 'vehicle.test'

    def __init__(self, actor_id, world, longitudinal, speed, offset=0.0, extent=(1, .5, .5)):
        # The inert actor boundary exposes the independent geometry/velocity inputs.
        # pylint: disable=too-many-arguments,too-many-positional-arguments
        self.extent = extent
        self.current_control = carla.VehicleControl()
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
        return carla.BoundingBox(carla.Location(), carla.Vector3D(*self.extent))

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
        return self.current_control

    def get_speed_limit(self):
        """Provide an ordinary positive map speed limit in km/h."""
        return 50.0

    def enable_constant_velocity(self, velocity):
        """Record the real agent's outgoing native velocity command."""
        self.commands.append(velocity)


class World:
    """Real native lane geometry with inert world/sensor RPC endpoints."""

    def __init__(self, two_lanes=False):
        xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
               '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
               '</geoReference></header><road id="1" name="straight" length="100" junction="-1" rule="RHT">'
               '<planView><geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry></planView>'
               '<lanes><laneSection s="0"><center><lane id="0" type="none"/></center><right>'
               '<lane id="-1" type="driving" level="false"><width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
               '</lane></right></laneSection></lanes></road></OpenDRIVE>')
        if two_lanes:
            xml = xml.replace('</right>', '<lane id="-2" type="driving" level="false">'
                              '<width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
                              '</lane></right>')
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


class TestObstacleFootprintsAndResponse(unittest.TestCase):
    """Real maps, actor geometry, complete planners/PID and actual agent decisions."""

    @staticmethod
    def agent(behavior=False, use_boxes=False, two_lanes=False, offset=None, plan=(14, 18, 22, 26, 30)):
        """Create a real agent while replacing only remote actor/world endpoints."""
        world = World(two_lanes)
        ego = Vehicle(1, world, 10, 0)
        world.actors.append(ego)
        options = {'use_bbs_detection': use_boxes}
        if offset is not None:
            options['offset'] = offset
        agent_class = BehaviorAgent if behavior else BasicAgent
        agent = agent_class(ego, opt_dict=options, map_inst=world.native_map)
        agent.set_global_plan([(world.native_map.get_waypoint_xodr(1, -1, station), RoadOption.LANEFOLLOW)
                               for station in plan])
        return agent, world, ego

    def test_corridor_contains_small_footprints_between_route_samples(self):
        """Connecting opposite sides in a zigzag must not cut holes into the lane strip."""
        for offset in (-.4, .4):
            with self.subTest(offset=offset):
                agent, world, _ = self.agent(use_boxes=True)
                target = Vehicle(2, world, 12, 0, offset=offset, extent=(.2, .1, .5))
                self.assertTrue(agent._vehicle_obstacle_detected([target], 30)[0])

    def test_corridor_excludes_nonintersecting_side_actors(self):
        """A corrected perimeter must not expand detection beyond the planned lane strip."""
        agent, world, _ = self.agent(use_boxes=True)
        target = Vehicle(2, world, 12, 0, offset=2., extent=(.2, .1, .5))
        self.assertFalse(agent._vehicle_obstacle_detected([target], 30)[0])

    def test_projected_native_box_retains_its_interior(self):
        """A short route already overlaps a large native box despite missing its diagonals."""
        agent, world, _ = self.agent(use_boxes=True, plan=(12,))
        target = Vehicle(2, world, 10, 0, offset=.9, extent=(3, .5, .5))
        self.assertTrue(agent._vehicle_obstacle_detected([target], 30)[0])

    def test_rotated_and_pitched_native_boxes_remain_detectable(self):
        """Project the full 3D native box rather than assuming one axis-aligned face."""
        for yaw, pitch, roll in ((0, 0, 0), (45, 0, 0), (90, 20, 0), (15, 10, 25)):
            with self.subTest(rotation=(yaw, pitch, roll)):
                agent, world, _ = self.agent(use_boxes=True)
                target = Vehicle(2, world, 14, 0, extent=(1., .5, .5))
                target.transform.rotation = carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
                self.assertTrue(agent._vehicle_obstacle_detected([target], 30)[0])

    def test_nearest_hazard_is_independent_of_actor_list_order(self):
        """A farther moving actor cannot hide the stationary car requiring an emergency stop."""
        for far_first in (False, True):
            with self.subTest(far_first=far_first):
                agent, world, _ = self.agent(behavior=True)
                near = Vehicle(2, world, 16, 0, offset=.1)
                far = Vehicle(3, world, 22, 10, offset=.1)
                world.actors.extend([far, near] if far_first else [near, far])
                detected = agent._vehicle_obstacle_detected(world.actors, 30)
                self.assertEqual(detected[1].id, near.id)
                control = agent.run_step()
                self.assertAlmostEqual(control.brake, .6)
                self.assertEqual(control.throttle, 0)

    def test_farther_truck_with_closer_boundary_has_priority(self):
        """Rank by BehaviorAgent's consumed clearance while retaining centre-distance reporting."""
        for truck_first in (False, True):
            with self.subTest(truck_first=truck_first):
                agent, world, _ = self.agent(behavior=True)
                car = Vehicle(2, world, 19, 10, offset=.1)
                truck = Vehicle(3, world, 22, 0, offset=.1, extent=(8, 1, 1.5))
                world.actors.extend([truck, car] if truck_first else [car, truck])
                self.assertEqual(agent._vehicle_obstacle_detected(world.actors, 30)[1].id, truck.id)
                control = agent.run_step()
                self.assertAlmostEqual(control.brake, .6)
                self.assertEqual(control.throttle, 0)

    def test_irrelevant_closer_actor_does_not_hide_route_hazard(self):
        """Range/angle/lane rejection still runs before selecting a blocking actor."""
        agent, world, _ = self.agent(two_lanes=True)
        other_lane = Vehicle(2, world, 12, 0)
        other_lane.transform = world.native_map.get_waypoint_xodr(1, -2, 12).transform
        lead = Vehicle(3, world, 18, 0, offset=.1)
        self.assertEqual(agent._vehicle_obstacle_detected([other_lane, lead], 30)[1].id, lead.id)

    def test_bbox_detection_uses_the_same_clearance_priority(self):
        """The junction/bounding-box path must select the same relevant blocker."""
        agent, world, _ = self.agent(use_boxes=True)
        near = Vehicle(2, world, 16, 0, offset=.1)
        far = Vehicle(3, world, 22, 0, offset=.1)
        self.assertEqual(agent._vehicle_obstacle_detected([far, near], 30)[1].id, near.id)

    def test_runtime_offset_matches_a_fresh_shifted_planner(self):
        """Changing the lane offset must move both the PID target and obstacle corridor."""
        for constructor in (False, True):
            with self.subTest(constructor=constructor):
                agent, world, _ = self.agent(two_lanes=True, offset=3.5 if constructor else None)
                target = Vehicle(2, world, 14, 0)
                target.transform = world.native_map.get_waypoint_xodr(1, -2, 14).transform
                if not constructor:
                    agent.set_offset(3.5)
                self.assertTrue(agent._vehicle_obstacle_detected([target], 30)[0])
                agent.set_offset(0)
                self.assertFalse(agent._vehicle_obstacle_detected([target], 30)[0])

    def test_behavior_emergency_stop_retains_current_turning_steer(self):
        """A close hazard must not straighten the wheels while braking through a turn."""
        for steering in (-.4, 0, .4):
            with self.subTest(steering=steering):
                agent, world, ego = self.agent(behavior=True)
                ego.current_control = carla.VehicleControl(steer=steering)
                world.actors.append(Vehicle(2, world, 16, 0, offset=.1))
                control = agent.run_step()
                self.assertAlmostEqual(control.steer, steering)
                self.assertAlmostEqual(control.brake, .6)
                self.assertEqual(control.throttle, 0)

    def test_clear_road_control_keeps_ordinary_pid_acceleration(self):
        """None of the geometry/response changes introduces a false emergency stop."""
        agent, _, _ = self.agent(behavior=True)
        control = agent.run_step()
        self.assertGreater(control.throttle, 0)
        self.assertEqual(control.brake, 0)


if __name__ == '__main__':
    unittest.main()
