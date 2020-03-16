#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module is responsible for the management of the sumo simulation. """

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import collections
import enum
import logging

import carla  # pylint: disable=import-error
import sumolib  # pylint: disable=import-error
import traci  # pylint: disable=import-error

from .constants import INVALID_ACTOR_ID

# ==================================================================================================
# -- sumo definitions ------------------------------------------------------------------------------
# ==================================================================================================


# https://sumo.dlr.de/docs/TraCI/Vehicle_Signalling.html
class SumoVehSignal(object):
    """
    SumoVehSignal contains the different sumo vehicle signals.
    """
    BLINKER_RIGHT = 1 << 0
    BLINKER_LEFT = 1 << 1
    BLINKER_EMERGENCY = 1 << 2
    BRAKELIGHT = 1 << 3
    FRONTLIGHT = 1 << 4
    FOGLIGHT = 1 << 5
    HIGHBEAM = 1 << 6
    BACKDRIVE = 1 << 7
    WIPER = 1 << 8
    DOOR_OPEN_LEFT = 1 << 9
    DOOR_OPEN_RIGHT = 1 << 10
    EMERGENCY_BLUE = 1 << 11
    EMERGENCY_RED = 1 << 12
    EMERGENCY_YELLOW = 1 << 13


# https://sumo.dlr.de/docs/Definition_of_Vehicles,_Vehicle_Types,_and_Routes.html#abstract_vehicle_class
class SumoActorClass(enum.Enum):
    """
    SumoActorClass enumerates the different sumo actor classes.
    """
    IGNORING = "ignoring"
    PRIVATE = "private"
    EMERGENCY = "emergency"
    AUTHORITY = "authority"
    ARMY = "army"
    VIP = "vip"
    PEDESTRIAN = "pedestrian"
    PASSENGER = "passenger"
    HOV = "hov"
    TAXI = "taxi"
    BUS = "bus"
    COACH = "coach"
    DELIVERY = "delivery"
    TRUCK = "truck"
    TRAILER = "trailer"
    MOTORCYCLE = "motorcycle"
    MOPED = "moped"
    BICYCLE = "bicycle"
    EVEHICLE = "evehicle"
    TRAM = "tram"
    RAIL_URBAN = "rail_urban"
    RAIL = "rail"
    RAIL_ELECTRIC = "rail_electric"
    RAIL_FAST = "rail_fast"
    SHIP = "ship"
    CUSTOM1 = "custom1"
    CUSTOM2 = "custom2"


SumoActor = collections.namedtuple(
    'SumoActor', 'type_id vclass transform signals extent color')

# ==============================================================================
# -- sumo simulation -----------------------------------------------------------
# ==============================================================================


class SumoSimulation(object):
    """
    SumoSimulation is responsible for the management of the sumo simulation.
    """

    def __init__(self, args):
        self.args = args
        host = args.sumo_host
        port = args.sumo_port

        if args.sumo_gui is True:
            sumo_binary = sumolib.checkBinary('sumo-gui')
        else:
            sumo_binary = sumolib.checkBinary('sumo')

        if args.sumo_host is None or args.sumo_port is None:
            logging.info('Starting new sumo server...')
            traci.start([
                sumo_binary,
                "-c", args.sumo_cfg_file,
                '--step-length', str(args.step_length),
                '--lateral-resolution', '0.25',
                '--collision.check-junctions'
            ])

            if args.sumo_gui:
                logging.info('Remember to press the play button to start the simulation')
        else:
            logging.info('Connection to sumo server. Host: %s Port: %s', host, port)
            traci.init(host=host, port=port)

        # Structures to keep track of the spawned and destroyed vehicles at each time step.
        self.spawned_actors = set()
        self.destroyed_actors = set()

        # Creating a random route to be able to spawn carla actors.
        traci.route.add("carla_route", [traci.edge.getIDList()[0]])

        # Variable to asign an id to new added actors.
        self._sequential_id = 0

    @staticmethod
    def subscribe(actor_id):
        """
        Subscribe the given actor to the following variables:

            * Type.
            * Vehicle class.
            * Color.
            * Length, Width, Height.
            * Position3D (i.e., x, y, z).
            * Angle, Slope.
            * Speed.
            * Lateral speed.
            * Signals.
        """
        traci.vehicle.subscribe(actor_id, [
            traci.constants.VAR_TYPE, traci.constants.VAR_VEHICLECLASS,
            traci.constants.VAR_COLOR, traci.constants.VAR_LENGTH,
            traci.constants.VAR_WIDTH, traci.constants.VAR_HEIGHT,
            traci.constants.VAR_POSITION3D, traci.constants.VAR_ANGLE,
            traci.constants.VAR_SLOPE, traci.constants.VAR_SPEED,
            traci.constants.VAR_SPEED_LAT, traci.constants.VAR_SIGNALS
        ])

    @staticmethod
    def unsubscribe(actor_id):
        """
        Unsubscribe the given actor from receiving updated information each step.
        """
        traci.vehicle.unsubscribe(actor_id)

    @staticmethod
    def get_net_offset():
        """
        Accessor for sumo net offset.
        """
        offset = traci.simulation.convertGeo(0, 0)
        return (-offset[0], -offset[1])

    @staticmethod
    def get_step_length():
        """
        Accessor for sumo simulation step length.
        """
        return traci.simulation.getDeltaT()

    @staticmethod
    def get_actor(actor_id):
        """
        Accessor for sumo actor.
        """
        results = traci.vehicle.getSubscriptionResults(actor_id)

        type_id = results[traci.constants.VAR_TYPE]
        vclass = SumoActorClass(results[traci.constants.VAR_VEHICLECLASS])
        color = results[traci.constants.VAR_COLOR]

        length = results[traci.constants.VAR_LENGTH]
        width = results[traci.constants.VAR_WIDTH]
        height = results[traci.constants.VAR_HEIGHT]

        location = list(results[traci.constants.VAR_POSITION3D])
        rotation = [
            results[traci.constants.VAR_SLOPE],
            results[traci.constants.VAR_ANGLE], 0.0
        ]
        transform = carla.Transform(
            carla.Location(location[0], location[1], location[2]),
            carla.Rotation(rotation[0], rotation[1], rotation[2])
        )

        signals = results[traci.constants.VAR_SIGNALS]
        extent = carla.Vector3D(length / 2.0, width / 2.0, height / 2.0)

        return SumoActor(type_id, vclass, transform, signals, extent, color)

    def spawn_actor(self, type_id, attrs=None):
        """
        Spawns a new actor.

            :param type_id: vtype to be spawned.
            :param attrs: dictionary with additional attributes for this specific actor.
            :return: actor id if the actor is successfully spawned. Otherwise, INVALID_ACTOR_ID.
        """
        actor_id = 'carla' + str(self._sequential_id)
        try:
            traci.vehicle.add(actor_id, 'carla_route', typeID=type_id)
        except traci.exceptions.TraCIException as error:
            logging.error('Spawn sumo actor failed: %s', error)
            return INVALID_ACTOR_ID

        if attrs is not None:
            if self.args.sync_vehicle_color and 'color' in attrs:
                color = attrs['color'].split(',')
                traci.vehicle.setColor(actor_id, color)

        self._sequential_id += 1

        return actor_id

    @staticmethod
    def destroy_actor(actor_id):
        """
        Destroys the given actor.
        """
        traci.vehicle.remove(actor_id)

    def synchronize_vehicle(self, vehicle_id, transform, signals=None):
        """
        Updates vehicle state.

            :param vehicle_id: id of the actor to be updated.
            :param transform: new vehicle transform (i.e., position and rotation).
            :param signals: new vehicle signals.
            :return: True if successfully updated. Otherwise, False.
        """
        loc_x, loc_y = transform.location.x, transform.location.y
        yaw = transform.rotation.yaw

        traci.vehicle.moveToXY(vehicle_id, "", 0, loc_x, loc_y, angle=yaw, keepRoute=2)
        if signals is not None and self.args.sync_vehicle_lights:
            traci.vehicle.setSignals(vehicle_id, signals)
        return True

    def tick(self):
        """
        Tick to sumo simulation.
        """
        traci.simulationStep()

        # Update data structures for the current frame.
        self.spawned_actors = set(traci.simulation.getDepartedIDList())
        self.destroyed_actors = set(traci.simulation.getArrivedIDList())

    @staticmethod
    def close():
        """
        Closes traci client.
        """
        traci.close()
