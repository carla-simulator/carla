#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
""" This module is responsible for the management of the ptv-vissim simulation. """

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import enum
import logging
import math
import os

import carla  # pylint: disable=import-error
from ctypes import *

from . import constants

# ==================================================================================================
# -- vissim definitions ----------------------------------------------------------------------------
# ==================================================================================================


class Simulator_Veh_Data(Structure):
    """
    Structure to hold the data sent to vissim about the status of the simulator vehicles (i.e.,
    carla vehicles).
    """
    _fields_ = [
        ('Position_X', c_double),  # front center of the vehicle in m
        ('Position_Y', c_double),  # front center of the vehicle in m
        ('Position_Z', c_double),  # front center of the vehicle in m
        ('Orient_Heading', c_double),  # in radians, eastbound = zero, northbound = +Pi/2 */
        ('Orient_Pitch', c_double),  # in radians, uphill = positive
        ('Speed', c_double),  # in m/s
        ('ControlledByVissim', c_bool),  # affects next time step
        ('RoutingDecisionNo', c_long),  # used once if ControlledByVissim changed from false to true
        ('RouteNo', c_long)  # used once if ControlledByVissim changed from false to true
    ]


class VISSIM_Veh_Data(Structure):
    """
    Structure to hold the data received from vissim about the status of the traffic vehicles (i.e.,
    vissim vehicles).
    """
    _fields_ = [
        ('VehicleID', c_long),
        ('VehicleType', c_long),  # vehicle type number from Vissim
        ('ModelFileName', c_char * constants.NAME_MAX_LENGTH),  # .v3d
        ('color', c_long),  # RGB
        ('Position_X', c_double),  # front center of the vehicle in m
        ('Position_Y', c_double),  # front center of the vehicle in m
        ('Position_Z', c_double),  # front center of the vehicle in m
        ('Orient_Heading', c_double),  # in radians, eastbound = zero, northbound = +Pi/2 */
        ('Orient_Pitch', c_double),  # in radians, uphill = positive
        ('Speed', c_double),  # in m/s
        ('LeadingVehicleID', c_long),  # relevant vehicle in front
        ('TrailingVehicleID', c_long),  # next vehicle back on the same lane
        ('LinkID', c_long),  # Vissim link attribute “Number”
        ('LinkName', c_char * constants.NAME_MAX_LENGTH),  # empty if “Name” not set in Vissim
        ('LinkCoordinate', c_double),  # in m
        ('LaneIndex', c_int),  # 0 = rightmost
        ('TurningIndicator', c_int),  # 1 = left, 0 = none, -1 = right
        ('PreviousIndex', c_long),  # for interpolation: index in the array in the previous Vissim time step, < 0 = new in the visibility area
        ('NumUDAs', c_long),  # the number of UDA values in the following array
        ('UDA', c_double * constants.MAX_UDA)  # the first MAX_UDA user-defined numeric vehicle attributes
    ]


class VissimLightState(enum.Enum):
    """
    VissimLightState contains the different vissim indicator states.
    """
    LEFT = 1
    NONE = 0
    RIGHT = -1


class VissimVehicle(object):
    """
    VissimVehicle holds the data relative to traffic vehicles in vissim.
    """
    def __init__(self,
                 vehicle_id,
                 type_id,
                 model_filename,
                 color,
                 location,
                 rotation,
                 velocity,
                 lights_state=VissimLightState.NONE):
        # Static parameters.
        self.id = vehicle_id
        self.type = type_id
        self.model_filename = model_filename
        self.color = color

        # Dynamic attributes.
        loc = carla.Location(location[0], location[1], location[2])
        rot = carla.Rotation(math.degrees(rotation[0]), math.degrees(rotation[1]),
                             math.degrees(rotation[2]))
        self._transform = carla.Transform(loc, rot)
        self._velocity = carla.Vector3D(
            velocity * math.cos(math.radians(rot.yaw)) * math.cos(math.radians(rot.pitch)),
            velocity * math.sin(math.radians(rot.yaw)) * math.cos(math.radians(rot.pitch)),
            velocity * math.sin(math.radians(rot.pitch)))
        self._lights_state = lights_state

    def get_velocity(self):
        """
        Returns the vehicle's velocity.
        """
        return self._velocity

    def get_transform(self):
        """
        Returns carla transform.
        """
        return self._transform


# ==================================================================================================
# -- vissim simulation -----------------------------------------------------------------------------
# ==================================================================================================


class PTVVissimSimulation(object):
    """
    PTVVissimSimulation is responsible for the management of the vissim simulation.
    """
    def __init__(self, args):
        # Maximum number of simulator vehicles to be tracked by the driving simulator interface.
        self._max_simulator_vehicles = args.simulator_vehicles

        # Loading driving simulator proxy library.
        logging.info('Loading DrivingSimulatorProxy library...')
        self.ds_proxy = cdll.LoadLibrary('DrivingSimulatorProxy.dll')

        # Connection to vissim simulator.
        logging.info('Establishing a connection with a GUI version of PTV-Vissim')
        result = self.ds_proxy.VISSIM_Connect(args.vissim_version,
                                              os.path.abspath(args.vissim_network),
                                              int(1. / args.step_length),
                                              c_double(constants.VISSIM_VISIBILITY_RADIUS),
                                              c_ushort(constants.VISSIM_MAX_SIMULATOR_VEH),
                                              c_ushort(constants.VISSIM_MAX_SIMULATOR_PED),
                                              c_ushort(constants.VISSIM_MAX_SIMULATOR_DET),
                                              c_ushort(constants.VISSIM_MAX_VISSIM_VEH),
                                              c_ushort(constants.VISSIM_MAX_VISSIM_PED),
                                              c_ushort(constants.VISSIM_MAX_VISSIM_SIGGRP))

        if not result:
            raise RuntimeError('There was an error when establishing a connection with PTV-Vissim')

        # Structures to keep track of the simulation state at each time step.
        self._vissim_vehicles = {}  # vissim_actor_id: VissimVehicle (only vissim traffic)
        self._simulator_vehicles = {}  # vissim_actor_id: Simulator_Veh_Data

        self.spawned_vehicles = set()
        self.destroyed_vehicles = set()

    def _get_next_actor_id(self):
        """
        Returns an available actor id. Otherwise, returns INVALID_ACTOR_ID.
        """
        all_ids = set(range(1, self._max_simulator_vehicles + 1))
        used_ids = set(self._simulator_vehicles.keys())
        available_ids = all_ids - used_ids
        if len(available_ids):
            return available_ids.pop()
        else:
            return constants.INVALID_ACTOR_ID

    def get_actor(self, actor_id):
        """
        Accessor for vissim actor.
        """
        return self._vissim_vehicles[actor_id]

    def spawn_actor(self, transform):
        """
        Spawns a new actor.

        Warning: When the maximum number of simulator vehicles being tracked at the same time is
        reached, no new vehicles are spawned.
        """
        # Checks number of simulator vehicles currently being tracked.
        if (len(self._simulator_vehicles) < self._max_simulator_vehicles):
            actor_id = self._get_next_actor_id()
            self._simulator_vehicles[actor_id] = Simulator_Veh_Data(
                transform.location.x, transform.location.y, transform.location.z,
                math.radians(transform.rotation.yaw), math.radians(transform.rotation.pitch), 0.0,
                False, 0, 0)
            return actor_id
        else:
            logging.warning(
                'Maximum number of simulator vehicles reached. No vehicle will be spawned.')
            return constants.INVALID_ACTOR_ID

    def destroy_actor(self, actor_id):
        """
        Destroys the given actor.

            :param actor_id: id of the vehicle to be destroyed.
            :return: True if successfully destroyed. Otherwise, False.
        """
        if actor_id in self._simulator_vehicles:
            del self._simulator_vehicles[actor_id]
            return True
        return False

    def synchronize_vehicle(self, vehicle_id, transform, velocity):
        """
        Updates vehicle state.

            :param int vehicle_id: id of the vehicle to be updated.
            :param carla.Transform transform: new vehicle transform (i.e., position and rotation).
            :param carla.Vector3D velocity: new vehicle velocity.
            :return: True if successfully updated. Otherwise, False.
        """
        self._simulator_vehicles[vehicle_id] = Simulator_Veh_Data(
            transform.location.x, transform.location.y, transform.location.z,
            math.radians(transform.rotation.yaw), math.radians(transform.rotation.pitch),
            math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2), False, 0, 0)
        return True

    def _get_simulator_veh_data(self):
        """
        Returns list of Simulator_Veh_Data structures ready to be sent to the driving simulator
        interface.
        """
        data = []
        for i in range(1, self._max_simulator_vehicles + 1):
            if i in self._simulator_vehicles:
                data.append(self._simulator_vehicles[i])
            else:
                # Invalid Simulator_Veh_Data to set the position of simulator vehicles in vissim
                # that are not yet in carla.
                data.append(
                    Simulator_Veh_Data(float('inf'), float('inf'), float('inf'), 0.0, 0.0, 0.0,
                                       False, 0, 0))
        return data

    def tick(self):
        """
        Tick to vissim simulation.
        """
        # Updating simulator vehicles data.
        arr = (Simulator_Veh_Data * self._max_simulator_vehicles)(*self._get_simulator_veh_data())
        self.ds_proxy.VISSIM_SetDriverVehicles(self._max_simulator_vehicles, byref(arr))

        # Retrieving vissim traffic data.
        num_vehicles = c_int(0)
        traffic_data = POINTER(VISSIM_Veh_Data)()
        self.ds_proxy.VISSIM_GetTrafficVehicles(byref(num_vehicles), byref(traffic_data))

        vehicles = {}
        for i in range(num_vehicles.value):
            vehicle_data = traffic_data[i]

            vehicles[vehicle_data.VehicleID] = VissimVehicle(
                vehicle_data.VehicleID, vehicle_data.VehicleType, vehicle_data.ModelFileName,
                vehicle_data.color,
                [vehicle_data.Position_X, vehicle_data.Position_Y, vehicle_data.Position_Z],
                [vehicle_data.Orient_Pitch, vehicle_data.Orient_Heading, 0.0], vehicle_data.Speed,
                vehicle_data.TurningIndicator)

        # Update data structures for the current time step.
        active_vehicles = set(self._vissim_vehicles.keys())
        current_vehicles = set(vehicles.keys())

        self.spawned_vehicles = current_vehicles.difference(active_vehicles)
        self.destroyed_vehicles = active_vehicles.difference(current_vehicles)

        self._vissim_vehicles = vehicles

    def close(self):
        self.ds_proxy.VISSIM_Disconnect()
