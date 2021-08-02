#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
""" This module provides a helper for the co-simulation between sumo and carla ."""

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import json
import logging
import math
import os
import random

import carla  # pylint: disable=import-error
import traci  # pylint: disable=import-error

from .sumo_simulation import SumoSignalState, SumoVehSignal

# ==================================================================================================
# -- Bridge helper (SUMO <=> CARLA) ----------------------------------------------------------------
# ==================================================================================================


class BridgeHelper(object):
    """
    BridgeHelper provides methos to ease the co-simulation between sumo and carla.
    """

    blueprint_library = []
    offset = (0, 0)

    _vtypes_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "data",
                                "vtypes.json")
    with open(_vtypes_path) as f:
        _VTYPES = json.load(f)['carla_blueprints']

    @staticmethod
    def get_carla_transform(in_sumo_transform, extent):
        """
        Returns carla transform based on sumo transform.
        """
        offset = BridgeHelper.offset
        in_location = in_sumo_transform.location
        in_rotation = in_sumo_transform.rotation

        # From front-center-bumper to center (sumo reference system).
        # (http://sumo.sourceforge.net/userdoc/Purgatory/Vehicle_Values.html#angle)
        yaw = -1 * in_rotation.yaw + 90
        pitch = in_rotation.pitch
        out_location = (in_location.x - math.cos(math.radians(yaw)) * extent.x,
                        in_location.y - math.sin(math.radians(yaw)) * extent.x,
                        in_location.z - math.sin(math.radians(pitch)) * extent.x)
        out_rotation = (in_rotation.pitch, in_rotation.yaw, in_rotation.roll)

        # Applying offset sumo-carla net.
        out_location = (out_location[0] - offset[0], out_location[1] - offset[1], out_location[2])

        # Transform to carla reference system (left-handed system).
        out_transform = carla.Transform(
            carla.Location(out_location[0], -out_location[1], out_location[2]),
            carla.Rotation(out_rotation[0], out_rotation[1] - 90, out_rotation[2]))

        return out_transform

    @staticmethod
    def get_sumo_transform(in_carla_transform, extent):
        """
        Returns sumo transform based on carla transform.
        """
        offset = BridgeHelper.offset
        in_location = in_carla_transform.location
        in_rotation = in_carla_transform.rotation

        # From center to front-center-bumper (carla reference system).
        yaw = -1 * in_rotation.yaw
        pitch = in_rotation.pitch
        out_location = (in_location.x + math.cos(math.radians(yaw)) * extent.x,
                        in_location.y - math.sin(math.radians(yaw)) * extent.x,
                        in_location.z - math.sin(math.radians(pitch)) * extent.x)
        out_rotation = (in_rotation.pitch, in_rotation.yaw, in_rotation.roll)

        # Applying offset carla-sumo net
        out_location = (out_location[0] + offset[0], out_location[1] - offset[1], out_location[2])

        # Transform to sumo reference system.
        out_transform = carla.Transform(
            carla.Location(out_location[0], -out_location[1], out_location[2]),
            carla.Rotation(out_rotation[0], out_rotation[1] + 90, out_rotation[2]))

        return out_transform

    @staticmethod
    def _get_recommended_carla_blueprint(sumo_actor):
        """
        Returns an appropriate blueprint based on the given sumo actor.
        """
        vclass = sumo_actor.vclass.value

        blueprints = []
        for blueprint in BridgeHelper.blueprint_library:
            if blueprint.id in BridgeHelper._VTYPES and \
               BridgeHelper._VTYPES[blueprint.id]['vClass'] == vclass:
                blueprints.append(blueprint)

        if not blueprints:
            return None

        return random.choice(blueprints)

    @staticmethod
    def get_carla_blueprint(sumo_actor, sync_color=False):
        """
        Returns an appropriate blueprint based on the received sumo actor.
        """
        blueprint_library = BridgeHelper.blueprint_library
        type_id = sumo_actor.type_id

        if type_id in [bp.id for bp in blueprint_library]:
            blueprint = blueprint_library.filter(type_id)[0]
            logging.debug('[BridgeHelper] sumo vtype %s found in carla blueprints', type_id)
        else:
            blueprint = BridgeHelper._get_recommended_carla_blueprint(sumo_actor)
            if blueprint is not None:
                logging.warning(
                    'sumo vtype %s not found in carla. The following blueprint will be used: %s',
                    type_id, blueprint.id)
            else:
                logging.error('sumo vtype %s not supported. No vehicle will be spawned in carla',
                              type_id)
                return None

        if blueprint.has_attribute('color'):
            if sync_color:
                color = "{},{},{}".format(sumo_actor.color[0], sumo_actor.color[1],
                                          sumo_actor.color[2])
            else:
                color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)

        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)

        blueprint.set_attribute('role_name', 'sumo_driver')

        logging.debug(
            '''[BridgeHelper] sumo vtype %s will be spawned in carla with the following attributes:
            \tblueprint: %s
            \tcolor: %s''', type_id, blueprint.id,
            sumo_actor.color if blueprint.has_attribute('color') else (-1, -1, -1))

        return blueprint

    @staticmethod
    def _create_sumo_vtype(carla_actor):
        """
        Creates an appropriate vtype based on the given carla_actor.
        """
        type_id = carla_actor.type_id
        attrs = carla_actor.attributes
        extent = carla_actor.bounding_box.extent

        if int(attrs['number_of_wheels']) == 2:
            traci.vehicletype.copy('DEFAULT_BIKETYPE', type_id)
        else:
            traci.vehicletype.copy('DEFAULT_VEHTYPE', type_id)

        if type_id in BridgeHelper._VTYPES:
            if 'vClass' in BridgeHelper._VTYPES[type_id]:
                _class = BridgeHelper._VTYPES[type_id]['vClass']
                traci.vehicletype.setVehicleClass(type_id, _class)

            if 'guiShape' in BridgeHelper._VTYPES[type_id]:
                shape = BridgeHelper._VTYPES[type_id]['guiShape']
                traci.vehicletype.setShapeClass(type_id, shape)

        if 'color' in attrs:
            color = attrs['color'].split(',')
            traci.vehicletype.setColor(type_id, color)

        traci.vehicletype.setLength(type_id, 2.0 * extent.x)
        traci.vehicletype.setWidth(type_id, 2.0 * extent.y)
        traci.vehicletype.setHeight(type_id, 2.0 * extent.z)

        logging.debug(
            '''[BridgeHelper] blueprint %s not found in sumo vtypes
            \tdefault vtype: %s
            \tvtype: %s
            \tclass: %s
            \tshape: %s
            \tcolor: %s
            \tlenght: %s
            \twidth: %s
            \theight: %s''', type_id,
            'DEFAULT_BIKETYPE' if int(attrs['number_of_wheels']) == 2 else 'DEFAULT_VEHTYPE',
            type_id, traci.vehicletype.getVehicleClass(type_id),
            traci.vehicletype.getShapeClass(type_id), traci.vehicletype.getColor(type_id),
            traci.vehicletype.getLength(type_id), traci.vehicletype.getWidth(type_id),
            traci.vehicletype.getHeight(type_id))

        return type_id

    @staticmethod
    def get_sumo_vtype(carla_actor):
        """
        Returns an appropriate vtype based on the type id and attributes.
        """
        type_id = carla_actor.type_id

        if not type_id.startswith('vehicle'):
            logging.error(
                '[BridgeHelper] Blueprint %s not supported. No vehicle will be spawned in sumo',
                type_id)
            return None

        if type_id in traci.vehicletype.getIDList():
            logging.debug('[BridgeHelper] blueprint %s found in sumo vtypes', type_id)
            return type_id
        return BridgeHelper._create_sumo_vtype(carla_actor)

    @staticmethod
    def get_carla_lights_state(current_carla_lights, sumo_lights):
        """
        Returns carla vehicle light state based on sumo signals.
        """
        current_lights = current_carla_lights

        # Blinker right / emergency.
        if (any([
                bool(sumo_lights & SumoVehSignal.BLINKER_RIGHT),
                bool(sumo_lights & SumoVehSignal.BLINKER_EMERGENCY)
        ]) != bool(current_lights & carla.VehicleLightState.RightBlinker)):
            current_lights ^= carla.VehicleLightState.RightBlinker

        # Blinker left / emergency.
        if (any([
                bool(sumo_lights & SumoVehSignal.BLINKER_LEFT),
                bool(sumo_lights & SumoVehSignal.BLINKER_EMERGENCY)
        ]) != bool(current_lights & carla.VehicleLightState.LeftBlinker)):
            current_lights ^= carla.VehicleLightState.LeftBlinker

        # Break.
        if (bool(sumo_lights & SumoVehSignal.BRAKELIGHT) !=
                bool(current_lights & carla.VehicleLightState.Brake)):
            current_lights ^= carla.VehicleLightState.Brake

        # Front (low beam).
        if (bool(sumo_lights & SumoVehSignal.FRONTLIGHT) !=
                bool(current_lights & carla.VehicleLightState.LowBeam)):
            current_lights ^= carla.VehicleLightState.LowBeam

        # Fog.
        if (bool(sumo_lights & SumoVehSignal.FOGLIGHT) !=
                bool(current_lights & carla.VehicleLightState.Fog)):
            current_lights ^= carla.VehicleLightState.Fog

        # High beam.
        if (bool(sumo_lights & SumoVehSignal.HIGHBEAM) !=
                bool(current_lights & carla.VehicleLightState.HighBeam)):
            current_lights ^= carla.VehicleLightState.HighBeam

        # Backdrive (reverse).
        if (bool(sumo_lights & SumoVehSignal.BACKDRIVE) !=
                bool(current_lights & carla.VehicleLightState.Reverse)):
            current_lights ^= carla.VehicleLightState.Reverse

        # Door open left/right.
        if (any([
                bool(sumo_lights & SumoVehSignal.DOOR_OPEN_LEFT),
                bool(sumo_lights & SumoVehSignal.DOOR_OPEN_RIGHT)
        ]) != bool(current_lights & carla.VehicleLightState.Position)):
            current_lights ^= carla.VehicleLightState.Position

        return current_lights

    @staticmethod
    def get_sumo_lights_state(current_sumo_lights, carla_lights):
        """
        Returns sumo signals based on carla vehicle light state.
        """
        current_lights = current_sumo_lights

        # Blinker right.
        if (bool(carla_lights & carla.VehicleLightState.RightBlinker) !=
                bool(current_lights & SumoVehSignal.BLINKER_RIGHT)):
            current_lights ^= SumoVehSignal.BLINKER_RIGHT

        # Blinker left.
        if (bool(carla_lights & carla.VehicleLightState.LeftBlinker) !=
                bool(current_lights & SumoVehSignal.BLINKER_LEFT)):
            current_lights ^= SumoVehSignal.BLINKER_LEFT

        # Emergency.
        if (all([
                bool(carla_lights & carla.VehicleLightState.RightBlinker),
                bool(carla_lights & carla.VehicleLightState.LeftBlinker)
        ]) != (current_lights & SumoVehSignal.BLINKER_EMERGENCY)):
            current_lights ^= SumoVehSignal.BLINKER_EMERGENCY

        # Break.
        if (bool(carla_lights & carla.VehicleLightState.Brake) !=
                bool(current_lights & SumoVehSignal.BRAKELIGHT)):
            current_lights ^= SumoVehSignal.BRAKELIGHT

        # Front (low beam)
        if (bool(carla_lights & carla.VehicleLightState.LowBeam) !=
                bool(current_lights & SumoVehSignal.FRONTLIGHT)):
            current_lights ^= SumoVehSignal.FRONTLIGHT

        # Fog light.
        if (bool(carla_lights & carla.VehicleLightState.Fog) !=
                bool(current_lights & SumoVehSignal.FOGLIGHT)):
            current_lights ^= SumoVehSignal.FOGLIGHT

        # High beam ligth.
        if (bool(carla_lights & carla.VehicleLightState.HighBeam) !=
                bool(current_lights & SumoVehSignal.HIGHBEAM)):
            current_lights ^= SumoVehSignal.HIGHBEAM

        # Backdrive (reverse)
        if (bool(carla_lights & carla.VehicleLightState.Reverse) !=
                bool(current_lights & SumoVehSignal.BACKDRIVE)):
            current_lights ^= SumoVehSignal.BACKDRIVE

        return current_lights

    @staticmethod
    def get_carla_traffic_light_state(sumo_tl_state):
        """
        Returns carla traffic light state based on sumo traffic light state.
        """
        if sumo_tl_state == SumoSignalState.RED or sumo_tl_state == SumoSignalState.RED_YELLOW:
            return carla.TrafficLightState.Red

        elif sumo_tl_state == SumoSignalState.YELLOW:
            return carla.TrafficLightState.Yellow

        elif sumo_tl_state == SumoSignalState.GREEN or \
             sumo_tl_state == SumoSignalState.GREEN_WITHOUT_PRIORITY:
            return carla.TrafficLightState.Green

        elif sumo_tl_state == SumoSignalState.OFF:
            return carla.TrafficLightState.Off

        else:  # SumoSignalState.GREEN_RIGHT_TURN and SumoSignalState.OFF_BLINKING
            return carla.TrafficLightState.Unknown

    @staticmethod
    def get_sumo_traffic_light_state(carla_tl_state):
        """
        Returns sumo traffic light state based on carla traffic light state.
        """
        if carla_tl_state == carla.TrafficLightState.Red:
            return SumoSignalState.RED

        elif carla_tl_state == carla.TrafficLightState.Yellow:
            return SumoSignalState.YELLOW

        elif carla_tl_state == carla.TrafficLightState.Green:
            return SumoSignalState.GREEN

        else:  # carla.TrafficLightState.Off and carla.TrafficLightState.Unknown
            return SumoSignalState.OFF
