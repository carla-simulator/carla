import argparse
import copy
import importlib
import logging
import random
import threahing

import numpy as np

import carla
from carla import ColorConverter as cc

from server_manager import *

data_buffer_lock = threading.Lock()
class CallBack():
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

    def reset(self):
        # instantiate a CARLA server manager
        self._carla_server = ServerManager({'CARLA_SERVER': self._args.server_path})
        self._carla_server.reset(map_id='Town04', track_id=Track.SENSORS, port=2000, human_flag=False)
        self._carla_server.wait_until_ready()

        # initialize client
        self._carla_client = carla.Client(self._args.host, self._args.port)
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

        self._vehicle = self.world.spawn_actor(blueprint, spawn_point)

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

import pdb
def run_evaluation(args):
    """

    :param args:
    :return:
    """

    # first we instantiate the Agent
    agent_module = importlib.import_module(args.agent)
    agent_instance = agent_module()

    pdb.set_trace()
    # configure simulation
    scenario_manager = ScenarioSetup(args, agent_instance)
    scenario_manager.reset()
    scenario_manager.reset_vehicle()





def main():
    argparser = argparse.ArgumentParser(description='CARLA automatic evaluation script')

    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--server_path', help='Absolute path to CARLA server binary', required=True)
    argparser.add_argument("-a", "--agent", type=str, help="Path to Agent's py file to evaluate", required=True)
    args = argparser.parse_args()

    # CARLA Evaluation protocol
    run_evaluation(args)


if __name__ == '__main__':
    main()
