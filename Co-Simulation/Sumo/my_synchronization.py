#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""
Script to integrate CARLA and SUMO simulations
"""

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import argparse
import json
import logging
import time

# ==================================================================================================
# -- find carla module -----------------------------------------------------------------------------
# ==================================================================================================

import glob
import os
import socket
import sys

try:
    sys.path.append(
        glob.glob('../../PythonAPI/carla/dist/carla-*%d.%d-%s.egg' %
                  (sys.version_info.major, sys.version_info.minor,
                   'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

##### Begin: My code #####
DATA_SERVER_HOST = "localhost"
DATA_SERVER_PORT = 9998
##### End: My code #####

# ==================================================================================================
# -- find traci module -----------------------------------------------------------------------------
# ==================================================================================================

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

# ==================================================================================================
# -- sumo integration imports ----------------------------------------------------------------------
# ==================================================================================================

import carla  # pylint: disable=import-error
from sumo_integration.bridge_helper import BridgeHelper  # pylint: disable=wrong-import-position
from sumo_integration.carla_simulation import CarlaSimulation  # pylint: disable=wrong-import-position
from sumo_integration.constants import INVALID_ACTOR_ID  # pylint: disable=wrong-import-position
from sumo_integration.sumo_simulation import SumoSimulation  # pylint: disable=wrong-import-position


# ==================================================================================================
# -- synchronization_loop --------------------------------------------------------------------------
# ==================================================================================================

##### Begin: My code. #####
class CAV:
    def __init__(self, sumo_actor_id, carla_sim, sumo_sim):
        self.sumo_actor_id = sumo_actor_id
        self.carla = carla_sim
        self.sumo = sumo_sim

        self.sensor_data = []
        self.perceived_objects = []
        self.received_CPMs = []

        self.load_sensors()

    def load_sensors(self):
        pass

    def receive_CPMs(self):
        pass

    def receive_sensor_data(self, data):
        pass

    def send_CPMs(self):
        pass

    def tick(self):
        self.update_perceived_objects()
        self.receive_CPMs()
        self.send_CPMs()

    def update_perceived_objects(self):
        pass


class CAVWithObstacleSensor(CAV):
    def load_sensors(self):
        pass


    def update_perceived_objects(self):
        pass


class CPM:
    MAX_SIZE = 800

    def __init__(ITS_PDU_Header={}, Management_Container={}, Station_Data_Container={}, Sensor_Information_Container=[], Perceived_Object_Container=[], option={}):
        # Easy implementation of a Cooprative Perception Message (CPM).
        # If you want to use detail information of CPM, you should include the information in CPM.
        self.ITS_PDU_Header = ITS_PDU_Header
        self.Management_Container = Management_Container
        self.Station_Data_Container = Station_Data_Container
        self.Sensor_Information_Container = Sensor_Information_Container
        self.Perceived_Object_Container = Perceived_Object_Container

        # For convenience, we add optional data like payload size
        self.option = option
        self.update_option()

    def dict_format(self):
        message = {
            "ITS_PDU_Header": self.ITS_PDU_Header,
            "Management_Container": self.Management_Container,
            "Station_Data_Container": self.Station_Data_Container,
            "Sensor_Information_Container": self.Sensor_Information_Container,
            "Perceived_Object_Container": self.Perceived_Object_Container,
            "option": self.option
        }

        return json


    def update_option(self):
        """
        ITS_PDU_Header + Management_Container + Station_Data_Container = 121 (Byte)
        Sensor_Information_Container = 35 (Byte/data)
        Perceived_Object_Container = 35 (Byte/data)

        cite from:
        Thandavarayan, G., Sepulcre, M., & Gozalvez, J. (2020). Cooperative Perception for Connected and Automated Vehicles: Evaluation and Impact of Congestion Control. IEEE Access, 8, 197665–197683. https://doi.org/10.1109/access.2020.3035119
        """

        self.option["size"] = 121 + 35 * len(self.Sensor_Information_Container) + 35 * len(self.Perceived_Object_Container)
##### End: My code. #####


class SimulationSynchronization(object):
    """
    SimulationSynchronization class is responsible for the synchronization of sumo and carla
    simulations.
    """

    ##### Begin: My code #####
    def save_obstacle_data(self, data, sumo_actor_id, host, port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

            s.connect((str(host), int(port)))
            send_data = {
                "vehid": str(sumo_actor_id),
                "method": "save_obstacle_data",
                "args": {
                    "data": {
                        "obstacle_id": data.other_actor.id,
                        "distance": data.distance,
                    }
                }
            }

            s.sendall(bytes(json.dumps(send_data), "ascii"))
            recv_data = s.recv(1024)
            print(f"time: {self.current_time}, elapsed_seconds: {self.elapsed_seconds(data.timestamp)}, recver_sumo_id: {sumo_actor_id}, target_sumo_id: {self.carlaid2sumoid(data.other_actor.id)}, transform: {data.transform}, target_type: {data.other_actor.type_id}, distance: {data.distance}")


    def attach_to_actor(self, bp, actor):
        return self.carla.world.spawn_actor(bp, carla.Transform(carla.Location(x=0.0, z=1.7)), attach_to=actor)


    def elapsed_seconds(self, world_elapsed_seconds):
        return world_elapsed_seconds - self.init_time

    def carlaid2sumoid(self, carlaid):
        sumoid = None

        try:
            sumoid = ([v_sumoid for k_carlaid, v_sumoid in self.carla2sumo_ids.items() if str(k_carlaid) == str(carlaid)])[0]
        except Exception as e:
            pass

        if sumoid is not None:
            return sumoid

        try:
            sumoid = ([k_sumoid for k_sumoid, v_carlaid in self.sumo2carla_ids.items() if str(v_carlaid) == str(carlaid)])[0]
        except Exception as e:
            pass

        return sumoid
    ##### End: My code #####


    def __init__(self,
                 sumo_simulation,
                 carla_simulation,
                 tls_manager='none',
                 sync_vehicle_color=False,
                 sync_vehicle_lights=False):

        self.sumo = sumo_simulation
        self.carla = carla_simulation

        self.tls_manager = tls_manager
        self.sync_vehicle_color = sync_vehicle_color
        self.sync_vehicle_lights = sync_vehicle_lights

        if tls_manager == 'carla':
            self.sumo.switch_off_traffic_lights()
        elif tls_manager == 'sumo':
            self.carla.switch_off_traffic_lights()

        # Mapped actor ids.
        self.sumo2carla_ids = {}  # Contains only actors controlled by sumo.
        self.carla2sumo_ids = {}  # Contains only actors controlled by carla.

        BridgeHelper.blueprint_library = self.carla.world.get_blueprint_library()
        BridgeHelper.offset = self.sumo.get_net_offset()

        # Configuring carla simulation in sync mode.
        settings = self.carla.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = self.carla.step_length
        self.carla.world.apply_settings(settings)

        ##### Begin: My code #####
        self.sumoid2sensors = {}
        self.current_time = 0
        self.init_time = self.carla.world.get_snapshot().timestamp.elapsed_seconds
        ##### End: My code #####


    def tick(self):
        """
        Tick to simulation synchronization
        """
        # -----------------
        # sumo-->carla sync
        # -----------------
        self.sumo.tick()

        ##### Begin: My code #####
        self.current_time = self.sumo.current_time()
        ##### End: My code #####

        # Spawning new sumo actors in carla (i.e, not controlled by carla).
        sumo_spawned_actors = self.sumo.spawned_actors - set(self.carla2sumo_ids.values())
        for sumo_actor_id in sumo_spawned_actors:
            self.sumo.subscribe(sumo_actor_id)
            sumo_actor = self.sumo.get_actor(sumo_actor_id)

            carla_blueprint = BridgeHelper.get_carla_blueprint(sumo_actor, self.sync_vehicle_color)
            if carla_blueprint is not None:
                carla_transform = BridgeHelper.get_carla_transform(sumo_actor.transform,
                                                                   sumo_actor.extent)

                carla_actor_id = self.carla.spawn_actor(carla_blueprint, carla_transform)

                ##### Begin: My code #####
                global DATA_SERVER_HOST
                global DATA_SERVER_PORT

                # ----- attach sensors to new actors -----
                self.sumoid2sensors[sumo_actor_id] = {}
                bp = self.carla.world.get_blueprint_library().find('sensor.other.obstacle')
                bp.set_attribute('distance', '100')
                bp.set_attribute('only_dynamics', 'True')
                self.sumoid2sensors[sumo_actor_id]['sensor.other.obstacle'] = self.attach_to_actor(bp, self.carla.get_actor(carla_actor_id))
                self.sumoid2sensors[sumo_actor_id]['sensor.other.obstacle'].listen(lambda data: self.save_obstacle_data(
                    data,
                    sumo_actor_id,
                    DATA_SERVER_HOST,
                    DATA_SERVER_PORT)
                )

                if carla_actor_id != INVALID_ACTOR_ID:
                    self.sumo2carla_ids[sumo_actor_id] = carla_actor_id

                ##### End: My code #####

            else:
                self.sumo.unsubscribe(sumo_actor_id)

        # Destroying sumo arrived actors in carla.
        for sumo_actor_id in self.sumo.destroyed_actors:
            if sumo_actor_id in self.sumo2carla_ids:
                self.carla.destroy_actor(self.sumo2carla_ids.pop(sumo_actor_id))

        # Updating sumo actors in carla.
        for sumo_actor_id in self.sumo2carla_ids:
            carla_actor_id = self.sumo2carla_ids[sumo_actor_id]

            sumo_actor = self.sumo.get_actor(sumo_actor_id)
            carla_actor = self.carla.get_actor(carla_actor_id)

            carla_transform = BridgeHelper.get_carla_transform(sumo_actor.transform,
                                                               sumo_actor.extent)
            if self.sync_vehicle_lights:
                carla_lights = BridgeHelper.get_carla_lights_state(carla_actor.get_light_state(),
                                                                   sumo_actor.signals)
            else:
                carla_lights = None

            self.carla.synchronize_vehicle(carla_actor_id, carla_transform, carla_lights)

            # ----- listen sensors -----
            # for name, sensor in self.sumoid2sensors[sumo_actor_id].items():
            #     sensor.listen(lambda data: save_into_data_server(data))

        # Updates traffic lights in carla based on sumo information.
        if self.tls_manager == 'sumo':
            common_landmarks = self.sumo.traffic_light_ids & self.carla.traffic_light_ids
            for landmark_id in common_landmarks:
                sumo_tl_state = self.sumo.get_traffic_light_state(landmark_id)
                carla_tl_state = BridgeHelper.get_carla_traffic_light_state(sumo_tl_state)

                self.carla.synchronize_traffic_light(landmark_id, carla_tl_state)

        # -----------------
        # carla-->sumo sync
        # -----------------
        self.carla.tick()

        # Spawning new carla actors (not controlled by sumo)
        carla_spawned_actors = self.carla.spawned_actors - set(self.sumo2carla_ids.values())
        for carla_actor_id in carla_spawned_actors:
            carla_actor = self.carla.get_actor(carla_actor_id)

            type_id = BridgeHelper.get_sumo_vtype(carla_actor)
            color = carla_actor.attributes.get('color', None) if self.sync_vehicle_color else None
            if type_id is not None:
                sumo_actor_id = self.sumo.spawn_actor(type_id, color)
                if sumo_actor_id != INVALID_ACTOR_ID:
                    self.carla2sumo_ids[carla_actor_id] = sumo_actor_id
                    self.sumo.subscribe(sumo_actor_id)

        # Destroying required carla actors in sumo.
        for carla_actor_id in self.carla.destroyed_actors:
            if carla_actor_id in self.carla2sumo_ids:
                self.sumo.destroy_actor(self.carla2sumo_ids.pop(carla_actor_id))

        # Updating carla actors in sumo.
        for carla_actor_id in self.carla2sumo_ids:
            sumo_actor_id = self.carla2sumo_ids[carla_actor_id]

            carla_actor = self.carla.get_actor(carla_actor_id)
            sumo_actor = self.sumo.get_actor(sumo_actor_id)

            sumo_transform = BridgeHelper.get_sumo_transform(carla_actor.get_transform(),
                                                             carla_actor.bounding_box.extent)
            if self.sync_vehicle_lights:
                carla_lights = self.carla.get_actor_light_state(carla_actor_id)
                if carla_lights is not None:
                    sumo_lights = BridgeHelper.get_sumo_lights_state(sumo_actor.signals,
                                                                     carla_lights)
                else:
                    sumo_lights = None
            else:
                sumo_lights = None

            self.sumo.synchronize_vehicle(sumo_actor_id, sumo_transform, sumo_lights)

        # Updates traffic lights in sumo based on carla information.
        if self.tls_manager == 'carla':
            common_landmarks = self.sumo.traffic_light_ids & self.carla.traffic_light_ids
            for landmark_id in common_landmarks:
                carla_tl_state = self.carla.get_traffic_light_state(landmark_id)
                sumo_tl_state = BridgeHelper.get_sumo_traffic_light_state(carla_tl_state)

                # Updates all the sumo links related to this landmark.
                self.sumo.synchronize_traffic_light(landmark_id, sumo_tl_state)

    def close(self):
        """
        Cleans synchronization.
        """
        # Configuring carla simulation in async mode.
        settings = self.carla.world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        self.carla.world.apply_settings(settings)

        # Destroying synchronized actors.
        for carla_actor_id in self.sumo2carla_ids.values():
            self.carla.destroy_actor(carla_actor_id)

        for sumo_actor_id in self.carla2sumo_ids.values():
            self.sumo.destroy_actor(sumo_actor_id)

        # Closing sumo and carla client.
        self.carla.close()
        self.sumo.close()


def synchronization_loop(args):
    """
    Entry point for sumo-carla co-simulation.
    """
    sumo_simulation = SumoSimulation(args.sumo_cfg_file, args.step_length, args.sumo_host,
                                     args.sumo_port, args.sumo_gui, args.client_order)
    carla_simulation = CarlaSimulation(args.carla_host, args.carla_port, args.step_length)

    synchronization = SimulationSynchronization(sumo_simulation, carla_simulation, args.tls_manager,
                                                args.sync_vehicle_color, args.sync_vehicle_lights)
    try:
        while True:
            start = time.time()

            synchronization.tick()

            end = time.time()
            elapsed = end - start
            if elapsed < args.step_length:
                time.sleep(args.step_length - elapsed)

    except KeyboardInterrupt:
        logging.info('Cancelled by user.')

    finally:
        logging.info('Cleaning synchronization')

        synchronization.close()


if __name__ == '__main__':
    # global DATA_SERVER_HOST
    # global DATA_SERVER_PORT

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('sumo_cfg_file', type=str, help='sumo configuration file')
    argparser.add_argument('--carla-host',
                           metavar='H',
                           default='127.0.0.1',
                           help='IP of the carla host server (default: 127.0.0.1)')
    argparser.add_argument('--carla-port',
                           metavar='P',
                           default=2000,
                           type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--sumo-host',
                           metavar='H',
                           default=None,
                           help='IP of the sumo host server (default: 127.0.0.1)')
    argparser.add_argument('--sumo-port',
                           metavar='P',
                           default=None,
                           type=int,
                           help='TCP port to listen to (default: 8813)')
    argparser.add_argument('--sumo-gui', action='store_true', help='run the gui version of sumo')
    argparser.add_argument('--step-length',
                           default=0.05,
                           type=float,
                           help='set fixed delta seconds (default: 0.05s)')
    argparser.add_argument('--client-order',
                           metavar='TRACI_CLIENT_ORDER',
                           default=1,
                           type=int,
                           help='client order number for the co-simulation TraCI connection (default: 1)')
    argparser.add_argument('--sync-vehicle-lights',
                           action='store_true',
                           help='synchronize vehicle lights state (default: False)')
    argparser.add_argument('--sync-vehicle-color',
                           action='store_true',
                           help='synchronize vehicle color (default: False)')
    argparser.add_argument('--sync-vehicle-all',
                           action='store_true',
                           help='synchronize all vehicle properties (default: False)')
    argparser.add_argument('--tls-manager',
                           type=str,
                           choices=['none', 'sumo', 'carla'],
                           help="select traffic light manager (default: none)",
                           default='none')
    argparser.add_argument('--debug', action='store_true', help='enable debug messages')

    ###### Begin: My codes. #####
    argparser.add_argument('--carla_veins_data_server_host', default="localhost")
    argparser.add_argument('--carla_veins_data_server_port', default=9998)
    ###### End: My codes #####

    arguments = argparser.parse_args()

    ###### Begin: My codes. #####
    DATA_SERVER_HOST = arguments.carla_veins_data_server_host
    DATA_SERVER_PORT = arguments.carla_veins_data_server_port
    ###### End: My codes #####

    if arguments.sync_vehicle_all is True:
        arguments.sync_vehicle_lights = True
        arguments.sync_vehicle_color = True

    if arguments.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    synchronization_loop(arguments)
