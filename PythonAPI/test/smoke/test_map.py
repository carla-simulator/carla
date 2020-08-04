# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random

from . import SmokeTest


class TestMap(SmokeTest):
    def test_reload_world(self):
        print("TestMap.test_reload_world")
        map_name = self.client.get_world().get_map().name
        world = self.client.reload_world()
        self.assertEqual(map_name, world.get_map().name)

    def test_load_all_maps(self):
        print("TestMap.test_load_all_maps")
        map_names = list(self.client.get_available_maps())
        random.shuffle(map_names)
        for map_name in map_names:
            if map_name != '/Game/Carla/Maps/BaseMap/BaseMap':
                world = self.client.load_world(map_name)
                m = world.get_map()
                self.assertEqual(map_name.split('/')[-1], m.name)
                self._check_map(m)

    def _check_map(self, m):
        for spawn_point in m.get_spawn_points():
            waypoint = m.get_waypoint(spawn_point.location, project_to_road=False)
            self.assertIsNotNone(waypoint)
        topology = m.get_topology()
        self.assertGreater(len(topology), 0)
        waypoints = list(m.generate_waypoints(2))
        self.assertGreater(len(waypoints), 0)
        random.shuffle(waypoints)
        for waypoint in waypoints[:200]:
            for _ in range(0, 20):
                self.assertGreaterEqual(waypoint.lane_width, 0.0)
                _ = waypoint.get_right_lane()
                _ = waypoint.get_left_lane()
                next_waypoints = waypoint.next(4)
                if not next_waypoints:
                    break
                waypoint = random.choice(next_waypoints)
        _ = m.transform_to_geolocation(carla.Location())
        self.assertTrue(str(m.to_opendrive()))
