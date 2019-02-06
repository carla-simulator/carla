from enum import Enum
import json
import math
import random
import time

import numpy as np
import carla

from agents.navigation.local_planner import compute_connection, RoadOption
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.global_route_planner_dao import GlobalRoutePlannerDAO
from agents.tools.misc import vector
from challenge.server_manager import ServerManager, Track
from challenge.data_provider import CallBack, DataProvider

import pdb

class TerminationState(Enum):
    SUCCESS = 1
    FAIL_CRASH = 2
    FAIL_TIMEOUT = 3
    NOT_TERMINATED = 4


class Route(object):
    def __init__(self):
        self.id = None
        self.map = None
        self.threshold = None
        self.start = carla.Transform()
        self.end = carla.Transform()


class ScenarioSetup(object):
    def __init__(self, args, agent):
        self._args = args
        self._agent = agent
        self._agent_state = TerminationState.NOT_TERMINATED
        self._sensors_setup = agent.sensors_setup()
        self._carla_server = None
        self._carla_client = None
        self._world = None
        self._map = None
        self._vehicle = None
        self._sensors_list = []
        self._sensors = []
        self._routes = []
        self._current_route = None
        self._hop_resolution = 2.0
        self._frame_number = 0
        self._simulation_time = 0
        self._start_time_scenario = 0
        self._timeout_scenario = 1000
        self._elapsed_time = 0
        self._max_distance_target = 5.0 # meters

        self._load_routes()

    def _load_routes(self):
        routes_file = self._args.routes
        with open(routes_file) as json_file:
            data = json.load(json_file)

            for route_id in data:
                route = Route()
                route.id = route_id
                route.map = data[route_id]['map']
                route.timeout = data[route_id]['timeout']

                route.start.location.x = data[route_id]['start']['pos']['x']
                route.start.location.y = data[route_id]['start']['pos']['y']
                route.start.location.z = data[route_id]['start']['pos']['z']

                route.start.rotation.pitch = data[route_id]['start'][
                    'ori']['pitch']
                route.start.rotation.roll = data[route_id]['start'][
                    'ori']['roll']
                route.start.rotation.yaw = data[route_id]['start'][
                    'ori']['yaw']

                route.end.location.x = data[route_id]['end']['pos']['x']
                route.end.location.y = data[route_id]['end']['pos']['y']
                route.end.location.z = data[route_id]['end']['pos']['z']

                route.end.rotation.pitch = data[route_id]['end'][
                    'ori']['pitch']
                route.end.rotation.roll = data[route_id]['end'][
                    'ori']['roll']
                route.end.rotation.yaw = data[route_id]['end'][
                    'ori']['yaw']

                self._routes.append(route)

    def get_routes(self):
        return self._routes

    def reset(self, map_id, track_id=Track.SENSORS, port=2000,
              human_flag=False):
        # instantiate a CARLA server manager
        self._carla_server = ServerManager(
            {'CARLA_SERVER': self._args.server_path})
        self._carla_server.reset(map_id, track_id, port, human_flag)
        self._carla_server.wait_until_ready()

        # initialize client
        self._carla_client = carla.Client('127.0.0.1', self._args.port)
        self._carla_client.set_timeout(4.0)

        self._world = self._carla_client.get_world()
        self._world.on_tick(self._on_world_tick)
        self._map = self._world.get_map()

    def reset_vehicle(self, spawn_point):
        # Spawn the vehicle.
        if self._vehicle is not None:
            self._vehicle.destroy()

        blueprint = self._world.get_blueprint_library().find(
            'vehicle.lincoln.mkz2017')
        blueprint.set_attribute('role_name', 'hero')
        if blueprint.has_attribute('color'):
            color = random.choice(
                blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)

        self._vehicle = self._world.spawn_actor(blueprint, spawn_point)

        # setup sensors
        bp_library = self._world.get_blueprint_library()
        for item in self._sensors_setup:
            bp = bp_library.find(item[0])
            if item[0].startswith('sensor.camera'):
                bp.set_attribute('image_size_x', str(item[1]['width']))
                bp.set_attribute('image_size_y', str(item[1]['height']))
                bp.set_attribute('fov', str(item[1]['fov']))

                sensor_location = carla.Location(x=item[1]['x'],
                                                 y=item[1]['y'],
                                                 z=item[1]['z'])
                sensor_rotation = carla.Rotation(pitch=item[1]['pitch'],
                                                 roll=item[1]['roll'],
                                                 yaw=item[1]['yaw'])

            elif item[0].startswith('sensor.lidar'):
                bp.set_attribute('range', '5000')

                sensor_location = carla.Location(x=item[1]['x'],
                                                 y=item[1]['y'],
                                                 z=item[1]['z'])
                sensor_rotation = carla.Rotation(pitch=item[1]['pitch'],
                                                 roll=item[1]['roll'],
                                                 yaw=item[1]['yaw'])
            elif item[0].startswith('sensor.other.gnss'):
                sensor_location = carla.Location(x=item[1]['x'],
                                                 y=item[1]['y'],
                                                 z=item[1]['z'])
                sensor_rotation = carla.Rotation()

            # create sensor
            sensor_transform = carla.Transform(sensor_location, sensor_rotation)
            sensor = self._world.spawn_actor(bp, sensor_transform,
                                             self._vehicle)
            # setup callback
            sensor.listen(CallBack(item[2], sensor, self._agent.data_provider))
            self._sensors_list.append(sensor)

        # check that all sensors have initialized their data structure
        while not self._agent.all_sensors_ready():
            time.sleep(0.1)

    def assign_global_plan(self, route):
        start_waypoint = self._map.get_waypoint(route.start.location)
        end_waypoint = self._map.get_waypoint(route.end.location)
        solution = []

        # Setting up global router
        dao = GlobalRoutePlannerDAO(self._world.get_map())
        grp = GlobalRoutePlanner(dao)
        grp.setup()

        # Obtain route plan
        x1 = start_waypoint.transform.location.x
        y1 = start_waypoint.transform.location.y
        x2 = end_waypoint.transform.location.x
        y2 = end_waypoint.transform.location.y
        route = grp.plan_route((x1, y1), (x2, y2))

        current_waypoint = start_waypoint
        route.append(RoadOption.VOID)
        for action in route:

            #   Generate waypoints to next junction
            wp_choice = current_waypoint.next(self._hop_resolution)
            while len(wp_choice) == 1:
                current_waypoint = wp_choice[0]
                solution.append((current_waypoint, RoadOption.LANEFOLLOW))
                wp_choice = current_waypoint.next(self._hop_resolution)
                #   Stop at destination
                if current_waypoint.transform.location.distance(
                        end_waypoint.transform.location) < self._hop_resolution: break
            if action == RoadOption.VOID: break

            #   Select appropriate path at the junction
            if len(wp_choice) > 1:

                # Current heading vector
                current_transform = current_waypoint.transform
                current_location = current_transform.location
                projected_location = current_location + \
                                     carla.Location(
                                         x=math.cos(math.radians(
                                             current_transform.rotation.yaw)),
                                         y=math.sin(math.radians(
                                             current_transform.rotation.yaw)))
                v_current = vector(current_location, projected_location)

                direction = 0
                if action == RoadOption.LEFT:
                    direction = 1
                elif action == RoadOption.RIGHT:
                    direction = -1
                elif action == RoadOption.STRAIGHT:
                    direction = 0
                select_criteria = float('inf')

                #   Choose correct path
                for wp_select in wp_choice:
                    v_select = vector(
                        current_location, wp_select.transform.location)
                    cross = float('inf')
                    if direction == 0:
                        cross = abs(np.cross(v_current, v_select)[-1])
                    else:
                        cross = direction * np.cross(v_current, v_select)[-1]
                    if cross < select_criteria:
                        select_criteria = cross
                        current_waypoint = wp_select

                # Generate all waypoints within the junction
                #   along selected path
                solution.append((current_waypoint, action))
                current_waypoint = current_waypoint.next(self._hop_resolution)[
                    0]
                while current_waypoint.is_intersection:
                    solution.append((current_waypoint, action))
                    current_waypoint = \
                        current_waypoint.next(self._hop_resolution)[0]

        # send plan to agent
        self._waypoints_plan, self._topological_plan = zip(*solution)
        self._agent.set_global_plan(self._topological_plan, self._waypoints_plan)

    def _on_world_tick(self, timestamp):
        self._frame_number = timestamp.frame_count
        self._simulation_time = timestamp.elapsed_seconds

    def start_scenario(self, route):
        self._current_route = route
        self._start_time_scenario = self._simulation_time
        self._timeout_scenario = route.timeout

    def stop_scenario(self):
        # destroy objects
        pass

    def is_scenario_over(self):
        # scenario over for timeout
        self._elapsed_time = self._simulation_time - self._start_time_scenario
        if  self._elapsed_time > self._timeout_scenario:
            self._agent_state = TerminationState.FAIL_TIMEOUT
            return True

        elif self._is_agent_at_goal():
            self._agent_state = TerminationState.SUCCESS
            return True

        self._agent_state = TerminationState.NOT_TERMINATED
        return False

    def _is_agent_at_goal(self):
        current_waypoint =  self._map.get_waypoint(self._vehicle.get_location())
        end_waypoint = self._map.get_waypoint(self._current_route.end.location)

        if current_waypoint.road_id != end_waypoint.road_id:
            return False
        if current_waypoint.lane_id != end_waypoint.lane_id:
            return False

        if current_waypoint.transform.location.distance(
                end_waypoint.transform.location) >= self._max_distance_target:
            return False

        return True

    def report_statistics(self):
        print("==Route: {}== [Status: {}] [Time(s): {}]".format(
            self._current_route.id, self._agent_state, self._elapsed_time))

    def wait_for_tick(self):
        return self._world.wait_for_tick(10.0)

    def apply_control(self, control):
        self._vehicle.apply_control(control)