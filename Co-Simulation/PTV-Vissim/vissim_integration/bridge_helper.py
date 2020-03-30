#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
""" This module provides a helper for the co-simulation between vissim and carla. """

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import logging
import math
import random

import carla  # pylint: disable=import-error

# ==================================================================================================
# -- Bridge helper (VISSIM <=> CARLA) --------------------------------------------------------------
# ==================================================================================================


class BridgeHelper(object):
    """
    BridgeHelper provides methos to ease the co-simulation between vissim and carla.
    """

    blueprint_library = []
    vtypes = {}

    @staticmethod
    def get_carla_transform(in_vissim_transform, extent=None):
        """
        Returns carla transform based on vissim transform.
        """
        in_location = in_vissim_transform.location
        in_rotation = in_vissim_transform.rotation

        # From front-center-bumper to center (vissim reference system).
        if extent is not None:
            out_location = (in_location.x - math.cos(math.radians(in_rotation.yaw)) * extent.x,
                            in_location.y - math.sin(math.radians(in_rotation.yaw)) * extent.x,
                            in_location.z - math.sin(math.radians(in_rotation.pitch)) * extent.x)
        else:
            out_location = (in_location.x, in_location.y, in_location.z)
        out_rotation = (in_rotation.pitch, in_rotation.yaw, in_rotation.roll)

        # Transform to carla reference system (left-handed system).
        out_transform = carla.Transform(
            carla.Location(out_location[0], -out_location[1], out_location[2]),
            carla.Rotation(out_rotation[0], -out_rotation[1], out_rotation[2]))

        return out_transform

    @staticmethod
    def get_vissim_transform(in_carla_transform, extent=None):
        """
        Returns vissim transform based on carla transform.
        """
        in_location = in_carla_transform.location
        in_rotation = in_carla_transform.rotation

        # From center to front-center-bumper (carla reference system).
        if extent is not None:
            yaw = -1 * in_rotation.yaw
            pitch = in_rotation.pitch
            out_location = (in_location.x + math.cos(math.radians(yaw)) * extent.x,
                            in_location.y - math.sin(math.radians(yaw)) * extent.x,
                            in_location.z - math.sin(math.radians(pitch)) * extent.x)
        else:
            out_location = (in_location.x, in_location.y, in_location.z)
        out_rotation = (in_rotation.pitch, in_rotation.yaw, in_rotation.roll)

        # Transform to vissim reference system (right-handed system).
        out_transform = carla.Transform(
            carla.Location(out_location[0], -out_location[1], out_location[2]),
            carla.Rotation(out_rotation[0], -out_rotation[1], out_rotation[2]))

        return out_transform

    @staticmethod
    def _flip_y(in_vector):
        """
        Flips y coordinate of the given vector.
        """
        return carla.Vector3D(in_vector.x, -in_vector.y, in_vector.z)

    @staticmethod
    def get_carla_velocity(in_vissim_velocity):
        """
        Returns carla velocity based on vissim velocity.
        """
        return BridgeHelper._flip_y(in_vissim_velocity)

    @staticmethod
    def get_vissim_velocity(in_carla_velocity):
        """
        Returns vissim velocity based on carla velocity.
        """
        return BridgeHelper._flip_y(in_carla_velocity)

    @staticmethod
    def _get_recommended_carla_blueprint(vissim_actor):
        """
        Returns an appropriate blueprint based on the given vissim actor.
        """
        blueprint = BridgeHelper.blueprint_library.filter('vehicle.seat.leon')[0]
        color = random.choice(blueprint.get_attribute('color').recommended_values)
        blueprint.set_attribute('color', color)
        return blueprint

    @staticmethod
    def get_carla_blueprint(vissim_actor):
        """
        Returns an appropriate blueprint based on the received vissim actor.
        """
        type_id = str(vissim_actor.type)

        if type_id in BridgeHelper.vtypes:
            candidates = BridgeHelper.vtypes[type_id]
            if candidates:
                blueprint_id = random.choice(candidates)
            else:
                logging.error(
                    'vissim type %s not supported. No vehicle will be spawned in carla', type_id)
                return None

            blueprint = BridgeHelper.blueprint_library.filter(blueprint_id)
            if not blueprint:
                logging.error(
                    'carla blueprint %s unknown. No vehicle will be spawned', blueprint_id)
                return None
            blueprint = blueprint[0]

            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)

            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)

            blueprint.set_attribute('role_name', 'vissim_driver')
            return blueprint

        else:
            logging.error(
                'vissim type %s unknown. No vehicle will be spawned in carla', type_id)
            return None
