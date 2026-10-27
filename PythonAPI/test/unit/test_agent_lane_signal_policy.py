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

SOURCE_ROOT = Path(os.environ.get("CARLA_AGENT_POLICY_TEST_ROOT", Path(__file__).resolve().parents[3]))
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
        # The inert actor boundary exposes independent geometry and
        # velocity inputs.
        # pylint: disable=too-many-arguments,too-many-positional-arguments
        self.extent = extent
        self.current_control = carla.VehicleControl()
        self.id = actor_id
        self.world = world
        self.transform = world.native_map.get_waypoint_xodr(1, -1, longitudinal).transform
        self.transform.location.y += offset
        self.velocity = carla.Vector3D(speed, 0, 0)
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


class PolicyWorld(World):
    """Native multi-lane map and explicit ASAM traffic-signal validities."""

    def __init__(self, rule='RHT', curved=False):
        super().__init__()
        lanes = '<center><lane id="0" type="none"/></center>'
        for side, sign in (('left', 1), ('right', -1)):
            lanes += '<' + side + '>'
            for index in (1, 2, 3):
                lanes += (f'<lane id="{sign * index}" type="driving">'
                          '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane>')
            lanes += '</' + side + '>'
        xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
               '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
               '</geoReference></header>')
        for road_id, x in ((1, 0), (2, 200)):
            geometry = '<arc curvature="0.01"/>' if curved and road_id == 1 else '<line/>'
            xml += (f'<road id="{road_id}" length="100" junction="-1" rule="{rule}">'
                    f'<planView><geometry s="0" x="{x}" y="0" hdg="0" length="100">{geometry}'
                    f'</geometry></planView><lanes><laneSection s="0">{lanes}</laneSection></lanes>')
            if road_id == 1:
                xml += '<signals>'
                for identifier, first, last in (('side', -2, -2), ('own', -1, -1), ('multi', -2, -1)):
                    xml += (f'<signal s="13" t="-5.25" id="{identifier}" name="signal" dynamic="yes" '
                            'orientation="+" zOffset="3" country="DEU" type="1000001" '
                            f'subtype="-1" value="-1"><validity fromLane="{first}" toLane="{last}"/>'
                            '</signal>')
                xml += '</signals>'
            xml += '</road>'
        self.native_map = carla.Map('policy-contracts', xml + '</OpenDRIVE>')


def vehicle(identifier, world, lane_id=-1, station=10, extent=(1, .5, .5)):
    """Locate an inert RPC actor using an actual native lane transform."""
    result = Vehicle(identifier, world, station, 0., extent=extent)
    result.transform = world.native_map.get_waypoint_xodr(1, lane_id, station).transform
    return result


def make_plan(agent, world):
    """Use the actual local planner and native waypoints for ordinary control steps."""
    agent.set_global_plan([(world.native_map.get_waypoint_xodr(1, -1, station), RoadOption.LANEFOLLOW)
                           for station in (14, 18, 22, 26, 30)])


class Light:
    """Remote traffic-light handle with actual native signal validity metadata."""
    type_id = 'traffic.traffic_light'

    def __init__(self, world, identifier, lane_id):
        self.id = {'side': 101, 'own': 102, 'multi': 103, 'unmapped': 104}[identifier]
        self.state = carla.TrafficLightState.Red
        self.transform = world.native_map.get_waypoint_xodr(1, lane_id, 13).transform
        self.trigger_volume = carla.BoundingBox(carla.Location(y=.1), carla.Vector3D(.5, .5, 1))
        self.landmarks = world.native_map.get_all_landmarks_from_id(identifier)
        self.native_map = world.native_map

    def get_transform(self):
        """Return value copies, matching the actual Actor binding."""
        return carla.Transform(self.transform.location, self.transform.rotation)

    def get_affected_lane_waypoints(self):
        """Remote metadata boundary following TrafficSign's documented lane ranges."""
        result = []
        for landmark in self.landmarks:
            for first, last in landmark.get_lane_validities():
                result.extend(self.native_map.get_waypoint_xodr(landmark.road_id, lane, landmark.s)
                              for lane in range(min(first, last), max(first, last) + 1) if lane)
        return result


class TestAgentLaneAndSignalPolicy(unittest.TestCase):
    """Execute real agents on real maps with only remote state delivery replaced."""

    @staticmethod
    def pedestrians(ignored=False, long_vehicle=False, station=16):
        """Construct a full behavior agent and a pedestrian within its actual road lane."""
        world = PolicyWorld()
        ego = vehicle(1, world, extent=(8, 1, 1.5) if long_vehicle else (1, .5, .5))
        walker = vehicle(2, world, station=station, extent=(.3, .3, .9))
        walker.type_id = 'walker.pedestrian.test'
        walker.transform.location.y += .1
        world.actors[:] = [ego, walker]
        agent = BehaviorAgent(ego, map_inst=world.native_map)
        make_plan(agent, world)
        agent.ignore_vehicles(ignored)
        return agent, world, ego, walker

    def test_adjacent_hazards_follow_each_native_road_rule(self):
        """Both lane signs and both driver-relative sides work on RHT and LHT."""
        for rule in ('RHT', 'LHT'):
            for lane_id in (-2, 2):
                world = PolicyWorld(rule)
                ego = vehicle(1, world, lane_id, 50)
                world.actors.append(ego)
                agent = BasicAgent(ego, map_inst=world.native_map)
                waypoint = world.native_map.get_waypoint_xodr(1, lane_id, 50)
                station = 56 if waypoint.transform.get_forward_vector().x > 0 else 44
                for offset, side in ((1, waypoint.get_right_lane()), (-1, waypoint.get_left_lane())):
                    with self.subTest(rule=rule, lane_id=lane_id, offset=offset):
                        target = vehicle(2, world, side.lane_id, station)
                        self.assertTrue(agent._vehicle_obstacle_detected(
                            [target], 20, up_angle_th=180, lane_offset=offset)[0])

    def test_the_other_adjacent_side_is_not_selected(self):
        """Correcting LHT must not mistake a left-side actor for a right-side actor."""
        for rule in ('RHT', 'LHT'):
            for lane_id in (-2, 2):
                with self.subTest(rule=rule, lane_id=lane_id):
                    world = PolicyWorld(rule)
                    ego = vehicle(1, world, lane_id, 50)
                    world.actors.append(ego)
                    agent = BasicAgent(ego, map_inst=world.native_map)
                    waypoint = world.native_map.get_waypoint_xodr(1, lane_id, 50)
                    station = 56 if waypoint.transform.get_forward_vector().x > 0 else 44
                    target = vehicle(2, world, waypoint.get_left_lane().lane_id, station)
                    self.assertFalse(agent._vehicle_obstacle_detected(
                        [target], 20, up_angle_th=180, lane_offset=1)[0])

    def test_ignoring_vehicles_preserves_pedestrian_braking(self):
        """The vehicle-only policy must not silently disable pedestrian avoidance."""
        for ignored in (False, True):
            with self.subTest(ignored=ignored):
                agent, _, _, _ = self.pedestrians(ignored=ignored)
                control = agent.run_step()
                self.assertAlmostEqual(control.brake, .6)
                self.assertEqual(control.throttle, 0)

    def test_vehicle_ignoring_still_applies_to_vehicles_control(self):
        """A vehicle blocker is ignored while the same policy is enabled."""
        agent, _, _, walker = self.pedestrians(ignored=True)
        walker.type_id = 'vehicle.test'
        control = agent.run_step()
        self.assertEqual(control.brake, 0)
        self.assertGreater(control.throttle, 0)

    def test_long_vehicle_sees_walker_inside_configured_detection_range(self):
        """A walker 12 m from the centre is only 3.7 m beyond a long truck's front."""
        agent, world, ego, _ = self.pedestrians(long_vehicle=True, station=22)
        agent._update_information()
        self.assertTrue(agent.pedestrian_avoid_manager(world.native_map.get_waypoint(ego.get_location()))[0])
        control = agent.run_step()
        self.assertAlmostEqual(control.brake, .6)
        self.assertEqual(control.throttle, 0)

    def test_far_walkers_do_not_create_emergency_stops_control(self):
        """The existing configured range and braking clearance still govern behavior."""
        for station in (22, 40):
            with self.subTest(station=station):
                agent, _, _, _ = self.pedestrians(station=station)
                control = agent.run_step()
                self.assertEqual(control.brake, 0)
                self.assertGreater(control.throttle, 0)

    @staticmethod
    def signals(identifier='own', lane_id=-1):
        """Prime native map/metadata and an ordinary agent on lane -1."""
        world = PolicyWorld(curved=True)
        ego = vehicle(1, world)
        light = Light(world, identifier, lane_id)
        world.actors[:] = [ego, light]
        agent = BasicAgent(ego, map_inst=world.native_map)
        return agent, world, ego, light

    def test_only_signal_affected_lanes_are_blocked(self):
        """A red signal assigned only to lane -2 must not stop lane -1."""
        agent, _, _, light = self.signals('side', -2)
        self.assertEqual(light.landmarks[0].get_lane_validities(), [(-2, -2)])
        self.assertFalse(agent._affected_by_traffic_light([light], 20)[0])

    def test_multilane_signal_retains_each_native_valid_lane_control(self):
        """Filtering by trigger-centre lane alone would incorrectly drop this valid red."""
        agent, _, _, light = self.signals('multi', -2)
        self.assertEqual(light.landmarks[0].get_lane_validities(), [(-2, -1)])
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])

    def test_cached_red_is_revalidated_after_a_road_change(self):
        """A prior road's red light cannot keep controlling an actor on another road."""
        agent, world, ego, light = self.signals()
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])
        ego.transform = world.native_map.get_waypoint_xodr(2, -1, 10).transform
        self.assertFalse(agent._affected_by_traffic_light([light], 20)[0])

    def test_cached_red_is_revalidated_after_passing_the_signal(self):
        """An unchanged red state alone cannot override a signal now behind the vehicle."""
        agent, world, ego, light = self.signals()
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])
        ego.transform = world.native_map.get_waypoint_xodr(1, -1, 20).transform
        self.assertFalse(agent._affected_by_traffic_light([light], 20)[0])

    def test_stopped_vehicle_and_green_transition_controls(self):
        """Ordinary repeated red checks and a later green state keep their behavior."""
        agent, _, _, light = self.signals()
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])
        light.state = carla.TrafficLightState.Green
        self.assertFalse(agent._affected_by_traffic_light([light], 20)[0])

    def test_unmapped_signal_retains_legacy_trigger_fallback(self):
        """Maps without a matching landmark retain the previous trigger-based behavior."""
        agent, _, _, light = self.signals('unmapped', -1)
        self.assertEqual(light.get_affected_lane_waypoints(), [])
        self.assertTrue(agent._affected_by_traffic_light([light], 20)[0])


if __name__ == '__main__':
    unittest.main()
