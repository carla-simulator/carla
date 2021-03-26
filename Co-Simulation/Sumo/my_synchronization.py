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
    CAMsHandler,
    CPM,
    CPMsHandler,
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
)
##### End: My code. #####

# ==================================================================================================
# -- synchronization_loop --------------------------------------------------------------------------
# ==================================================================================================

##### Begin: My code. #####
def location(actor, distance):
    x = actor.get_transform().location.x + distance * math.cos(math.radians(actor.get_transform().rotation.yaw))
    y = actor.get_transform().location.y + distance * math.sin(math.radians(actor.get_transform().rotation.yaw))

    return Location(x, y)

def speed(actor, abs_speed):
    x = abs_speed * math.cos(math.radians(actor.get_transform().rotation.yaw))
    y = abs_speed * math.sin(math.radians(actor.get_transform().rotation.yaw))

    return Speed(x, y)

class VehicleData:
    def __init__(self, time, actor):
        self.data = []
        self.tick(time, actor)

    def latest(self):
        if len(self.data) <= 0:
            return None
        else:
            return self.data[-1]

    def tick(self, time, actor):
        al = actor.get_transform().location

        if len(self.data) <= 0:
            self.data.append(self.formatted_data(time, Location(al.x, al.y), Speed(0, 0)))
        else:
            dT = time - self.data[-1]["time"]
            dX = al.x - self.data[-1]["location"].x
            dY = al.y - self.data[-1]["location"].y

            self.data.append(self.formatted_data(time, Location(al.x, al.y), Speed(dX / dT, dY / dT)))

    def formatted_data(self, time, location, speed):
        return {"time": time, "location": location, "speed": speed}


class CAV:
    TARGET_ROAD_WIDTH = Constants.TARGET_ROAD_WIDTH
    TARGET_ROAD_LENGTH = Constants.TARGET_ROAD_LENGTH
    VEHICLE_WIDTH = Constants.VEHICLE_WIDTH
    VEHICLE_LENGTH = Constants.VEHICLE_LENGTH
    SENSOR_TICK = Constants.SENSOR_TICK


    def __init__(self, sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization):
        global DATA_SERVER_HOST
        global DATA_SERVER_PORT
        global DATA_DIR

        self.sim_synchronization = sim_synchronization
        self.carla = carla_sim
        self.sumo = sumo_sim

        self.carla_actor = self.carla.get_actor(carla_actor_id)
        self.carla_actor_id = carla_actor_id
        self.sumo_actor_id = sumo_actor_id
        self.init_time = carla_init_time

        self.sensors = []

        self.sensor_data_handler = SensorDataHandler()
        self.perceived_objects_handler = PerceivedObjectsHandler()

        self.CAMs_handler = CAMsHandler(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)
        self.CPMs_handler = CPMsHandler(DATA_SERVER_HOST, DATA_SERVER_PORT, DATA_DIR)

        self.sim_synchronization.carlaid2vehicle_data[self.carla_actor_id] = VehicleData(self.sumo_elapsed_seconds(), self.carla_actor)
        self.load_sensors()

    def attach_bp(self, bp, transform):
        return self.carla.world.spawn_actor(bp, transform, attach_to=self.carla_actor)

    def sensor_bp(self, sensor_name="", attributes={}):
        bp = self.carla.world.get_blueprint_library().find(sensor_name)

        for k, v in attributes.items():
            bp.set_attribute(k, v)

        return bp

    def load_sensors(self):
        pass

    def new_perceived_objects_with_pseudonym(self):
        new_perceived_objects = self.sensor_data_handler.perceived_objects(self.sumo_elapsed_seconds(), Constants.VALID_TIME_DELTA)

        return [self.perceived_objects_handler.save(new_perceived_object) for new_perceived_object in new_perceived_objects]


    def rads(self, sensor_dist, target_rad, center_rad=0):
        sensor_num = self.sensor_num(sensor_dist, target_rad)

        return [float(target_rad) / sensor_num * i - float(target_rad * (sensor_num - 1) / sensor_num / 2.0 + center_rad) for i in range(0, sensor_num)]

    def receive_sensor_data(self, data):
        pass

    def sensor_num(self, sensor_dist, target_rad):
        return int(2 * math.pi * float(sensor_dist) * (target_rad / 360.0) / float(CAV.VEHICLE_WIDTH)) + 1

    def sumo_elapsed_seconds(self):
        return self.carla.world.get_snapshot().timestamp.elapsed_seconds - self.init_time

    def tick(self):
        start = time.time()
        t1 = start
        t2 = start
        t3 = start

        # ----- update vehicle data -----
        self.sim_synchronization.carlaid2vehicle_data[self.carla_actor_id].tick(self.sumo_elapsed_seconds(), self.carla_actor)

        # ----- update perceived_objects -----
        t1 = time.time()

        # p1 = Process(target=self.CPMs_handler.receive, args=[self.sumo_actor_id])
        # p1.start()
        # p1.join()
        #
        # p2 = Process(target=self.CPMs_handler.send, args=[self.sumo_actor_id, [CPM(
        #     self.sumo_elapsed_seconds(),
        #     self.ITS_PDU_Header(),
        #     self.Management_Container(),
        #     self.Station_Data_Container(),
        #     self.Sensor_Information_Container(),
        #     self.Perceived_Object_Container()
        # )]])
        # p2.start()
        # p2.join()
        # self.CPMs_handler.receive(self.sumo_actor_id)
        t2 = time.time()

        perceived_object_container = self.Perceived_Object_Container(self.new_perceived_objects_with_pseudonym())
        if 1 <= len(perceived_object_container):
            self.CPMs_handler.send(self.sumo_actor_id, CPM(
                self.sumo_elapsed_seconds(),
                self.ITS_PDU_Header(),
                self.Management_Container(),
                self.Station_Data_Container(),
                self.Sensor_Information_Container(),
                perceived_object_container
            ))

        t3 = time.time()
        print(f"Delay: {t3 - t2}, {t2 - t1}, {t1 - start}")

    def new_CPM(self):
        pass

    def ITS_PDU_Header(self):
        return {"tmp": "tmp"}

    def Management_Container(self):
        return {"tmp": "tmp"}

    def Station_Data_Container(self):
        location = self.carla_actor.get_transform().location
        speed = self.carla_actor.get_velocity()

        data = {
            "location" : [location.x, location.y, location.z],
            "speed" : [speed.x, speed.y, speed.z]
        }

        return data

    def Sensor_Information_Container(self):
        return ["360_sensor", "forward_sensor", "backward_sensor"]

    def Perceived_Object_Container(self, new_perceived_objects_with_pseudonym):
        detected_objects_for_new_CPM = []

        for the_latest_detected_object in new_perceived_objects_with_pseudonym:
            is_already_sent, delta_t, delta_s, delta_p = self.CPMs_handler.is_already_sent(the_latest_detected_object)

            if is_already_sent:
                # ETSI Standard
                if Constants.CPM_DELTA_T < delta_t or Constants.CPM_DELTA_S < delta_s or Constants.CPM_DELTA_P < delta_p:
                    detected_objects_for_new_CPM.append(the_latest_detected_object)

            else:
                detected_objects_for_new_CPM.append(the_latest_detected_object)

        return [obj.dict_format() for obj in detected_objects_for_new_CPM]



class CAVWithObstacleSensors(CAV):
    def __init__(self, sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization):
        super().__init__(sumo_actor_id, carla_actor_id, carla_sim, sumo_sim, carla_init_time, sim_synchronization)
        self.sensor_data_handler = ObstacleSensorDataHandler()


    def load_sensors(self):
        # 360 sensor
        rads_360 = self.rads(CAV.TARGET_ROAD_WIDTH, 360.0)
        rads_front = self.rads(CAV.TARGET_ROAD_LENGTH, 2 * math.degrees(math.atan(float(CAV.TARGET_ROAD_WIDTH) / float(CAV.TARGET_ROAD_LENGTH))))
        rads_back = self.rads(CAV.TARGET_ROAD_LENGTH, 2 * math.degrees(math.atan(float(CAV.TARGET_ROAD_WIDTH) / float(CAV.TARGET_ROAD_LENGTH))), 180)

        for z_rad in rads_360:
            sensor = self.attach_bp(self.sensor_bp('sensor.other.obstacle', {'distance': str(CAV.TARGET_ROAD_WIDTH), 'only_dynamics': 'True', 'sensor_tick': str(CAV.SENSOR_TICK)}), carla.Transform(carla.Location(x=0.0, z=1.7), carla.Rotation(yaw=z_rad)))
            sensor.listen(lambda data: self.receive_sensor_data(data))
            self.sensors.append(sensor)

        for z_rad in rads_front:
            sensor = self.attach_bp(self.sensor_bp('sensor.other.obstacle', {'distance': str(CAV.TARGET_ROAD_LENGTH), 'only_dynamics': 'True', 'sensor_tick': str(CAV.SENSOR_TICK)}), carla.Transform(carla.Location(x=0.0, z=1.7), carla.Rotation(yaw=z_rad)))
            sensor.listen(lambda data: self.receive_sensor_data(data))
            self.sensors.append(sensor)

        for z_rad in rads_back:
            sensor = self.attach_bp(self.sensor_bp('sensor.other.obstacle', {'distance': str(CAV.TARGET_ROAD_LENGTH), 'only_dynamics': 'True', 'sensor_tick': str(CAV.SENSOR_TICK)}), carla.Transform(carla.Location(x=0.0, z=1.7), carla.Rotation(yaw=z_rad)))
            sensor.listen(lambda data: self.receive_sensor_data(data))
            self.sensors.append(sensor)

    def receive_sensor_data(self, data):
        if "static" in data.actor.type_id:
            return

        self.sensor_data_handler.save(ObstacleSensorData(
            data,
            self.sumo_elapsed_seconds(),
            location(data.actor, data.distance),
            self.sim_synchronization.carlaid2vehicle_data[data.other_actor.id].latest()["speed"]
        ))
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
        self.cavs = []
        self.sumoid2sensors = {}
        self.carlaid2vehicle_data = {}
        self.current_time = 0
        self.init_time = self.carla.world.get_snapshot().timestamp.elapsed_seconds

    def sumoid_from_carlaid(self, carlaid):
        print(carlaid)
        print(self.carla2sumo_ids)
        print(self.sumo2carla_ids)
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
                self.cavs.append(CAVWithObstacleSensors(sumo_actor_id, carla_actor_id, self.carla, self.sumo, self.init_time, self))
                ##### End: My code #####

                if carla_actor_id != INVALID_ACTOR_ID:
                    self.sumo2carla_ids[sumo_actor_id] = carla_actor_id


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
        ##### Start: My code. #####
        for cav in self.cavs:
            cav.tick()

        procs = []
        # for cav in self.cavs:
        #     procs.append(Process(target=cav.tick, args=()))
        #     procs[-1].start()

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


        for p in procs:
            p.join()

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
    # global DATA_DIR

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
    argparser.add_argument('--carla_veins_data_server_host', default=DATA_SERVER_HOST)
    argparser.add_argument('--carla_veins_data_server_port', default=DATA_SERVER_PORT)
    argparser.add_argument('--carla_veins_data_dir', default=DATA_DIR)
    ###### End: My codes #####

    arguments = argparser.parse_args()

    ###### Begin: My codes. #####
    DATA_SERVER_HOST = arguments.carla_veins_data_server_host
    DATA_SERVER_PORT = arguments.carla_veins_data_server_port
    DATA_DIR = arguments.carla_veins_data_dir
    ###### End: My codes #####

    if arguments.sync_vehicle_all is True:
        arguments.sync_vehicle_lights = True
        arguments.sync_vehicle_color = True

    if arguments.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    synchronization_loop(arguments)
