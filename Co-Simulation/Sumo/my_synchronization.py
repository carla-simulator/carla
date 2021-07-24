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
import math
from collections import deque
from copy import deepcopy
from multiprocessing import Process
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
DATA_DIR = "./../Veins/carla-veins-data/"
CAM_MESSAGE_STANDARD = "etsi"
CPM_MESSAGE_STANDARD = "etsi"
SENSOR_TYPE = "360"
TIME_TO_START = 0
TIME_STEP = 0.05
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


##### Begin: My code. #####
from util.classes.constants import (
    Constants,
)
from util.classes.messages import (
    CAM,
    CAMsHandlerWithNoSend,
    CAMsHandlerWithEtsi,
    CAMsHandlerWithInterval,
    CPM,
    CPMsHandlerWithNoSend,
    CPMsHandlerWithEtsi,
    CPMsHandlerWithInterval,
)
from util.classes.perceived_objects import (
    PerceivedObject,
    PerceivedObjectsHandler,
)
from util.classes.sensor_data import (
    SensorData,
    SensorDataHandler,
    ObstacleSensorData,
    ObstacleSensorDataHandler,
)
from util.classes.utils import (
    Location,
    Speed,
    VehicleData,
    location,
    speed,
)
##### End: My code. #####

# ==================================================================================================
# -- synchronization_loop --------------------------------------------------------------------------
# ==================================================================================================

##### Begin: My code. #####
class CAV:
    def __init__(self, sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization):
        global DATA_SERVER_HOST
        global DATA_SERVER_PORT
        global DATA_DIR

        self.sim_synchronization = sim_synchronization
        self.carla = carla_sim
        self.sumo = sumo_sim

        # print(carla_actor_id)
        self.carla_actor = self.carla.get_actor(carla_actor_id)
        self.carla_actor_id = carla_actor_id
        self.sumo_actor_id = sumo_actor_id
        self.init_time = carla_init_time

        self.sensors = []

        self.sensor_data_handler = SensorDataHandler()
        self.perceived_objects_handler = PerceivedObjectsHandler()

        self.CAMs_handler = CAMsHandlerWithNoSend(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        self.CPMs_handler = CPMsHandlerWithNoSend(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)

        self.sim_synchronization.carlaid2vehicle_data[self.carla_actor_id] = VehicleData(self.sumo_elapsed_seconds(), self.carla_actor)
        self.load_sensors()

    def attach_bp(self, bp, transform):
        return self.carla.world.spawn_actor(bp, transform, attach_to=self.carla_actor)

    def sensor_bp(self, sensor_name="", attributes={}):
        bp = self.carla.world.get_blueprint_library().find(sensor_name)

        for k, v in attributes.items():
            bp.set_attribute(k, v)

        return bp

    def latest_vehicle_data(self):
        return self.sim_synchronization.carlaid2vehicle_data[self.carla_actor_id].latest()

    def load_sensors(self):
        pass

    def location(self):
        d = self.latest_vehicle_data()

        return d["location"]

    def new_perceived_objects_with_pseudonym(self):
        new_perceived_objects = self.sensor_data_handler.perceived_objects(self.sumo_elapsed_seconds(), Constants.VALID_TIME_DELTA)

        return [self.perceived_objects_handler.save(new_perceived_object) for new_perceived_object in new_perceived_objects]


    def rads(self, sensor_dist, target_rad, center_rad=0):
        sensor_num = self.sensor_num_in_range(sensor_dist, target_rad)

        return [float(target_rad) / sensor_num * i - float(target_rad * (sensor_num - 1) / sensor_num / 2.0 + center_rad) for i in range(0, sensor_num)]

    def receive_sensor_data(self, data):
        pass

    def sensor_num_in_range(self, sensor_dist, target_rad, target_width=Constants.VEHICLE_WIDTH):
        # This sensor numbers captur the object with at least vehicle width.

        return int(2 * math.pi * float(sensor_dist) * (target_rad / 360.0) / float(target_width)) + 1

    def speed(self):
        d = self.latest_vehicle_data()

        return d["speed"]

    def sumo_elapsed_seconds(self):
        return self.carla.world.get_snapshot().timestamp.elapsed_seconds - self.init_time

    def tick(self):
        # ----- update vehicle data -----
        self.sim_synchronization.carlaid2vehicle_data[self.carla_actor_id].tick(self.sumo_elapsed_seconds(), self.carla_actor)

        # ----- update perceived_objects -----
        self.CPMs_handler.receive(self.sumo_actor_id)


        # ----- send CPM -----
        perceived_object_container = self.CPMs_handler.new_perceived_object_container(self.new_perceived_objects_with_pseudonym())
        if 1 <= len(perceived_object_container):
            self.CPMs_handler.send(self.sumo_actor_id, CPM(
                    timestamp=self.sumo_elapsed_seconds(),
                    ITS_PDU_Header=self.__tmp_data(),
                    Management_Container=self.__tmp_data(),
                    Station_Data_Container=self.__Station_Data_Container(),
                    Sensor_Information_Container=self.__Sensor_Information_Container(),
                    Perceived_Object_Container=perceived_object_container
                ))

        # ----- send CAM -----
        # print(f"sumo_id: {self.sumo_actor_id}")
        if self.CAMs_handler.is_generate(self.sumo_elapsed_seconds(), self.location(), self.speed(), self.yaw()):
            self.CAMs_handler.send(self.sumo_actor_id, CAM(
                timestamp=self.sumo_elapsed_seconds(),
                ITS_PDU_Header=self.__tmp_data(),
                Basic_Container=self.__tmp_data(),
                HF_Container=self.__Station_Data_Container(),
                LF_Container=self.__tmp_data(),
                Special_Vehicle_Container=self.__tmp_data()
            ))

    def yaw(self):
        d = self.latest_vehicle_data()

        return d["yaw"]

    def new_CPM(self):
        pass

    def ITS_PDU_Header(self):
        return {"tmp": "tmp"}

    def Management_Container(self):
        return {"tmp": "tmp"}

    def __tmp_data(self):
        return {"tmp": "tmp"}

    def __Station_Data_Container(self):
        location = self.location()
        speed = self.speed()
        yaw = self.yaw()

        data = {
            "sumo_id": self.sumo_actor_id,
            "location" : [location.x, location.y],
            "speed" : [speed.x, speed.y, speed.z],
            "yaw" : yaw
        }

        return data

    def __Sensor_Information_Container(self):
        """
        Now, the container is only used to calculate CPM size.
        Since we only use the number of sensors, we simply return list with the number of the sensors.
        """
        return list(range(0, self.sensor_num()))

    def sensor_num(self):
        return len(self.sensors)


class CAVWithObstacleSensors(CAV):
    def __init__(self, sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization):
        super().__init__(sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization)
        self.sensor_data_handler = ObstacleSensorDataHandler()


    def load_sensors(self):
        self.load_obstacle_sensors(
            ranges =[Constants.SENSOR_RANGE_360,    Constants.SENSOR_RANGE_FRONT,   Constants.SENSOR_RANGE_BACK],
            degrees=[Constants.SENSOR_DEGREE_360,   Constants.SENSOR_DEGREE_FRONT,  Constants.SENSOR_DEGREE_BACK],
            phases =[0,                             0,                              180]
        )


    def load_obstacle_sensors(self, ranges=[], degrees=[], phases=[]):
        for i in range(0, min([len(ranges), len(degrees), len(phases)])):
            sensor_range = ranges[i]
            sensor_degree = degrees[i]
            sensor_phase = phases[i]
            rads = self.rads(sensor_range, sensor_degree, sensor_phase)

            for z_rad in rads:
                sensor = self.attach_bp(self.sensor_bp('sensor.other.obstacle', {'distance': str(sensor_range), 'only_dynamics': 'True', 'sensor_tick': str(Constants.SENSOR_TICK)}), carla.Transform(carla.Location(x=0.0, z=1.7), carla.Rotation(yaw=z_rad)))
                sensor.listen(lambda data: self.receive_sensor_data(data))
                self.sensors.append(sensor)


    def receive_sensor_data(self, data):
        if "static" in data.actor.type_id:
            return

        try:
            self.sensor_data_handler.save(ObstacleSensorData(
                data,
                self.sumo_elapsed_seconds(),
                location(data.actor, data.distance),
                self.sim_synchronization.carlaid2vehicle_data[data.other_actor.id].latest()["speed"]
            ))
        except KeyError as e:
            # Since the key is not in carlaid2vehicle_data, the object is not vehicle, so we ignore the key.
            pass


    def sensor_num(self):
        return 3

class CAVWith360ObstacleSensors(CAVWithObstacleSensors):
    RANGES =    [150]
    DEGREES =   [360]
    PHASES =    [0]
    # Cite from:
    # https://www.etsi.org/deliver/etsi_tr/103500_103599/103562/02.01.01_60/tr_103562v020101p.pdf

    def load_sensors(self):
        self.load_obstacle_sensors(
            ranges= CAVWith360ObstacleSensors.RANGES,
            degrees=CAVWith360ObstacleSensors.DEGREES,
            phases= CAVWith360ObstacleSensors.PHASES
        )


    def sensor_num(self):
        return 1


class CAVWithForwardObstacleSensors(CAVWithObstacleSensors):
    RANGES =    [65,    150]
    DEGREES =   [40*2,  5*2]
    PHASES =    [0,     0]
    # Cite from:
    # https://www.etsi.org/deliver/etsi_tr/103500_103599/103562/02.01.01_60/tr_103562v020101p.pdf

    def load_sensors(self):
        self.load_obstacle_sensors(
            ranges= CAVWithForwardObstacleSensors.RANGES,
            degrees=CAVWithForwardObstacleSensors.DEGREES,
            phases= CAVWithForwardObstacleSensors.PHASES
        )


    def sensor_num(self):
        return 2

##### End: My code. #####


class SimulationSynchronization(object):
    """
    SimulationSynchronization class is responsible for the synchronization of sumo and carla
    simulations.
    """


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
        self.sumoid2cav = {}
        self.carlaid2vehicle_data = {}
        self.init_time = self.carla.world.get_snapshot().timestamp.elapsed_seconds


    def sumoid_from_carlaid(self, carlaid):
        if carlaid in self.carla2sumo_ids.keys():
            return self.carla2sumo_ids[carlaid]
        else:
            for k, v in self.sumo2carla_ids.items():
                if v == carlaid:
                    return k
                else:
                    continue

        return None


    def sumo_elapsed_seconds(self):
        return self.carla.world.get_snapshot().timestamp.elapsed_seconds - self.init_time

    def cav_tick(self):
        for cav in self.sumoid2cav.values():
            cav.tick()

        return 0
    ##### End: My code #####


    def tick(self):
        """
        Tick to simulation synchronization
        """
        # -----------------
        # sumo-->carla sync
        # -----------------
        if self.sumo_elapsed_seconds() < TIME_TO_START:
            self.sumo.tick_at(TIME_TO_START)

            # To obtain acurate time from sumo_elapsed_seconds, correct init time.
            # self.sumo_elapsed_seconds - self.init_time = TIME_TO_START - TIME_STEP
            # At the nest step of carla, the time becomes TIME_TO_START
            self.init_time = self.init_time - TIME_TO_START + TIME_STEP
        else:
            self.sumo.tick()

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


                if carla_actor_id != INVALID_ACTOR_ID:
                    ##### Begin: My code #####
                    # ----- load sensors -----
                    self.sumoid2cav[sumo_actor_id] = self.get_cav_by_sensor_type(sumo_actor_id, carla_actor_id, SENSOR_TYPE)

                    # ----- load message standard -----
                    self.sumoid2cav[sumo_actor_id].CAMs_handler = self.get_cams_handler_by_standard(CAM_MESSAGE_STANDARD)
                    self.sumoid2cav[sumo_actor_id].CPMs_handler = self.get_cpms_handler_by_standard(CPM_MESSAGE_STANDARD)
                    ##### End: My code #####

                    self.sumo2carla_ids[sumo_actor_id] = carla_actor_id


            else:
                self.sumo.unsubscribe(sumo_actor_id)

        # Destroying sumo arrived actors in carla.
        for sumo_actor_id in self.sumo.destroyed_actors:
            if sumo_actor_id in self.sumo2carla_ids:
                self.carla.destroy_actor(self.sumo2carla_ids.pop(sumo_actor_id))

                ##### Begin: My code #####
                del self.sumoid2cav[sumo_actor_id]
                ##### End: My code #####



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

        ##### Start: My code. #####
        for cav in self.sumoid2cav.values():
            cav.tick()
        ##### End: My code. #####

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


    # My Code Begin
    def get_cav_by_sensor_type(self, sumo_actor_id, carla_actor_id, sensor_type):
        if sensor_type == "init":
            return CAVWithObstacleSensors(sumo_actor_id, carla_actor_id, self.carla, self.sumo, self.init_time, self)
        elif sensor_type == "360":
            return CAVWith360ObstacleSensors(sumo_actor_id, carla_actor_id, self.carla, self.sumo, self.init_time, self)
        elif sensor_type == "forward":
            return CAVWithForwardObstacleSensors(sumo_actor_id, carla_actor_id, self.carla, self.sumo, self.init_time, self)
        else:
            raise Exception(f"No such sensor: {sensor_type}")

    def get_cams_handler_by_standard(self, std):
        if std == "etsi":
            return CAMsHandlerWithEtsi(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        elif std == "periodic":
            return CAMsHandlerWithInterval(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        elif std == "no":
            return CAMsHandlerWithNoSend(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        else:
            raise Exception(f"No such CAM standard: {std}")

    def get_cpms_handler_by_standard(self, std):
        if std == "etsi":
            return CPMsHandlerWithEtsi(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        elif std == "periodic":
            return CPMsHandlerWithInterval(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        elif std == "no":
            return CPMsHandlerWithNoSend(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        else:
            raise Exception(f"No such CAM standard: {std}")
    # My Code End


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
    argparser.add_argument('--carla_veins_data_server_host',                                            default=DATA_SERVER_HOST)
    argparser.add_argument('--carla_veins_data_server_port',                                            default=DATA_SERVER_PORT)
    argparser.add_argument('--carla_veins_data_dir',                                                    default=DATA_DIR)
    argparser.add_argument('--sensor_type',             type=str,   choices=["init", "360", "forward"], default=SENSOR_TYPE)
    argparser.add_argument('--cam_message_standard',    type=str,   choices=["etsi", "periodic", "no"], default=CAM_MESSAGE_STANDARD)
    argparser.add_argument('--cpm_message_standard',    type=str,   choices=["etsi", "periodic", "no"], default=CPM_MESSAGE_STANDARD)
    argparser.add_argument('--time_to_start',           type=float,                                     default=TIME_TO_START)
    ###### End: My codes #####

    arguments = argparser.parse_args()

    ###### Begin: My codes. #####
    DATA_SERVER_HOST = arguments.carla_veins_data_server_host
    DATA_SERVER_PORT = arguments.carla_veins_data_server_port
    DATA_DIR = arguments.carla_veins_data_dir
    TIME_TO_START = arguments.time_to_start
    TIME_STEP = arguments.step_length
    SENSOR_TYPE = arguments.sensor_type
    CAM_MESSAGE_STANDARD = arguments.cam_message_standard
    CPM_MESSAGE_STANDARD = arguments.cpm_message_standard
    ###### End: My codes #####

    if arguments.sync_vehicle_all is True:
        arguments.sync_vehicle_lights = True
        arguments.sync_vehicle_color = True

    if arguments.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    synchronization_loop(arguments)
