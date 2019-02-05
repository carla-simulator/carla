import argparse
import copy
import importlib
import json
import logging
import random
import threading
import os
import sys
sys.path.append('{}/PythonAPI'.format(os.getcwd()))

import numpy as np
import carla
from carla import ColorConverter as cc

from server_manager import *


data_buffer_lock = threading.Lock()
class CallBack(object):
    def __init__(self, tag, obj):
        self._tag = tag
        self._obj = obj

    def __call__(self, image):
        self._parse_image_cb(image, self._tag)

    def _parse_image_cb(self, image, tag):
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = copy.deepcopy(array)

        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]

        data_buffer_lock.acquire()
        self._obj.data_buffers[self._tag] = array
        data_buffer_lock.release()

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
        self._sensors_setup = agent.sensors_setup()
        self._carla_server = None
        self._carla_client = None
        self._world = None
        self._vehicle = None
        self._sensors_list = []
        self._sensors = []
        self._routes = []

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

    def reset(self, map_id, track_id=Track.SENSORS, port=2000, human_flag=False):
        # instantiate a CARLA server manager
        self._carla_server = ServerManager({'CARLA_SERVER': self._args.server_path})
        self._carla_server.reset(map_id, track_id, port, human_flag)
        self._carla_server.wait_until_ready()

        # initialize client
        self._carla_client = carla.Client('127.0.0.1', self._args.port)
        self._carla_client.set_timeout(4.0)

        self._world = self._carla_client.get_world()

    def reset_vehicle(self, spawn_point):
        # Spawn the vehicle.
        if self._vehicle is not None:
            self._vehicle.destroy()

        blueprint = self._world.get_blueprint_library().find('vehicle.lincoln.mkz2017')
        blueprint.set_attribute('role_name', 'hero')
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
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

            elif item[0].startswith('sensor.lidar'):
                bp.set_attribute('range', '5000')

            # create sensor
            sensor = self._world.spawn_actor(bp,
                                             carla.Transform(
                                                 carla.Location(x=item[1]['x'], y=item[1]['y'], z=item[1]['z']),
                                                 carla.Rotation(pitch=item[1]['pitch'], roll=item[1]['roll'], yaw=item[1]['yaw'])
                                             ),
                                             self._vehicle)
            # setup callback
            sensor.listen(CallBack(item[2], self._agent))
            self._sensors_list.append(sensor)

    def wait_for_tick(self):
        return self._world.wait_for_tick(10.0)

    def apply_control(self, control):
        self._vehicle.apply_control(control)

import pdb
def run_evaluation(args):
    """

    :param args:
    :return:
    """

    # first we instantiate the Agent
    module_name = os.path.basename(args.agent).split('.')[0]
    module_spec = importlib.util.spec_from_file_location(module_name,
                                                         args.agent)
    foo = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(foo)
    agent_instance = getattr(foo, foo.__name__)()

    # configure simulation
    scenario_manager = ScenarioSetup(args, agent_instance)

    for route in scenario_manager.get_routes():
        scenario_manager.reset(route.map, args.track, args.port)
        scenario_manager.reset_vehicle(route.start)

        # main loop for the current scenario
        while True:
            # as soon as the server is ready continue!
            if not scenario_manager.wait_for_tick():
                continue

            action = agent_instance.run_step()
            scenario_manager.apply_control(action)

def main():
    argparser = argparse.ArgumentParser(description='CARLA automatic evaluation script')

    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--server_path', help='Absolute path to CARLA server binary', required=True)
    argparser.add_argument("-a", "--agent", type=str, help="Path to Agent's py file to evaluate", required=True)
    argparser.add_argument("-r", "--routes", help="Path to routes file", required=True)
    argparser.add_argument("-t", "--track", type=str, choices=["Sensors",
                                                               "NoRendering"],
                           help="Select competition track",
                           default="Sensors")
    args = argparser.parse_args()

    if args.track is 'Sensors':
        args.track = Track.SENSORS
    else:
        args.track = Track.NO_RENDERING

    # CARLA Evaluation protocol
    run_evaluation(args)


if __name__ == '__main__':
    main()
