#!/usr/bin/env python

# Copyright (c) 2019-2021 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides a parser for scenario configuration files based on OpenSCENARIO
"""

from __future__ import print_function

from distutils.util import strtobool
import re
import copy
import datetime
import math
import operator

import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.weather_sim import Weather
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import (TrafficLightStateSetter,
                                                                      ActorTransformSetterToOSCPosition,
                                                                      RunScript,
                                                                      ChangeWeather,
                                                                      ChangeAutoPilot,
                                                                      ChangeRoadFriction,
                                                                      ChangeActorTargetSpeed,
                                                                      ChangeActorControl,
                                                                      ChangeActorWaypoints,
                                                                      ChangeActorLateralMotion,
                                                                      ChangeActorLaneOffset,
                                                                      SyncArrivalOSC,
                                                                      KeepLongitudinalGap,
                                                                      Idle,
                                                                      ChangeParameter)
# pylint: disable=unused-import
# For the following includes the pylint check is disabled, as these are accessed via globals()
from srunner.scenariomanager.scenarioatomics.atomic_criteria import (CollisionTest,
                                                                     MaxVelocityTest,
                                                                     DrivenDistanceTest,
                                                                     AverageVelocityTest,
                                                                     KeepLaneTest,
                                                                     ReachedRegionTest,
                                                                     OnSidewalkTest,
                                                                     WrongLaneTest,
                                                                     InRadiusRegionTest,
                                                                     InRouteTest,
                                                                     RouteCompletionTest,
                                                                     RunningRedLightTest,
                                                                     RunningStopTest,
                                                                     OffRoadTest,
                                                                     EndofRoadTest)
# pylint: enable=unused-import
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import (InTriggerDistanceToVehicle,
                                                                               InTriggerDistanceToOSCPosition,
                                                                               InTimeToArrivalToOSCPosition,
                                                                               InTimeToArrivalToVehicle,
                                                                               DriveDistance,
                                                                               StandStill,
                                                                               OSCStartEndCondition,
                                                                               TriggerAcceleration,
                                                                               RelativeVelocityToOtherActor,
                                                                               TimeOfDayComparison,
                                                                               TriggerVelocity,
                                                                               WaitForTrafficLightState,
                                                                               CheckParameter)
from srunner.scenariomanager.timer import TimeOut, SimulationTimeCondition
from srunner.tools.py_trees_port import oneshot_behavior
from srunner.tools.scenario_helper import get_offset_transform, get_troad_from_transform


def oneshot_with_check(variable_name, behaviour, name=None):
    """
    Check if the blackboard contains already variable_name and
    return a oneshot_behavior for behaviour.
    """
    blackboard = py_trees.blackboard.Blackboard()
    # check if the variable_name already exists in the blackboard
    if blackboard.get(variable_name) is not None:
        print("Warning: {} is already used before. Check your XOSC for duplicated names".format(variable_name))

    return oneshot_behavior(variable_name, behaviour, name)


class ParameterRef:
    """
    This class stores osc parameter reference in its original form.
    Returns the converted value whenever it is used.
    """

    def __init__(self, reference_text) -> None:
        # TODO: (for OSC1.1) add methods(lexer and math_interpreter) to
        #  recognize and interpret math expression from reference_text
        self.reference_text = str(reference_text)

    def is_literal(self) -> bool:
        """
        Returns: True when text is a literal/number
        """
        return self._is_matching(pattern=r"(-)?\d+(\.\d*)?")

    def is_parameter(self) -> bool:
        """
        Returns: True when text is a parameter
        """
        return self._is_matching(pattern=r"[$][A-Za-z_][\w]*")

    def _is_matching(self, pattern: str) -> bool:
        """
        Returns: True when pattern is matching with text
        """
        match = re.search(pattern, self.reference_text)
        if match is not None:
            matching_string = match.group()
            return matching_string == self.reference_text
        return False

    def get_interpreted_value(self):
        """
        Returns: interpreted value from reference_text
        """
        if self.is_literal():
            value = self.reference_text
        elif self.is_parameter():
            value = CarlaDataProvider.get_osc_global_param_value(self.reference_text)
            if value is None:
                raise Exception("Parameter '{}' is not defined".format(self.reference_text[1:]))
        else:
            value = None
        return value

    def __float__(self) -> float:
        value = self.get_interpreted_value()
        if value is not None:
            return float(value)
        else:
            raise Exception("could not convert '{}' to float".format(self.reference_text))

    def __int__(self) -> int:
        value = self.get_interpreted_value()
        if value is not None:
            return int(float(value))
        else:
            raise Exception("could not convert '{}' to int".format(self.reference_text))

    def __str__(self) -> str:
        value = self.get_interpreted_value()
        return str(value) if value is not None else self.reference_text

    def __repr__(self):
        value = self.get_interpreted_value()
        return value if value is not None else self.reference_text

    def __radd__(self, other) -> bool:
        return other + self.__float__()

    def __add__(self, other) -> bool:
        return other + self.__float__()

    def __rsub__(self, other) -> bool:
        return other - self.__float__()

    def __sub__(self, other) -> bool:
        return self.__float__() - other

    def __rmul__(self, other) -> bool:
        return other * self.__float__()

    def __mul__(self, other) -> bool:
        return other * self.__float__()

    def __truediv__(self, other) -> bool:
        return self.__float__() / other

    def __rtruediv__(self, other) -> bool:
        return other / self.__float__()

    def __eq__(self, other) -> bool:
        return other == self.__float__()

    def __ne__(self, other) -> bool:
        return other != self.__float__()

    def __ge__(self, other) -> bool:
        return self.__float__() >= other

    def __le__(self, other) -> bool:
        return self.__float__() <= other

    def __gt__(self, other) -> bool:
        return self.__float__() > other

    def __lt__(self, other) -> bool:
        return self.__float__() < other

    def __GE__(self, other) -> bool:  # pylint: disable=invalid-name
        return self.__float__() >= other

    def __LE__(self, other) -> bool:  # pylint: disable=invalid-name
        return self.__float__() <= other

    def __GT__(self, other) -> bool:  # pylint: disable=invalid-name
        return self.__float__() > other

    def __LT__(self, other) -> bool:  # pylint: disable=invalid-name
        return self.__float__() < other

    def __iadd__(self, other) -> bool:
        return self.__float__() + other

    def __isub__(self, other) -> bool:
        return self.__float__() - other

    def __abs__(self):
        return abs(self.__float__())


class OpenScenarioParser(object):

    """
    Pure static class providing conversions from OpenSCENARIO elements to ScenarioRunner elements
    """
    operators = {
        "greaterThan": operator.gt,
        "lessThan": operator.lt,
        "equalTo": operator.eq,
        "greaterOrEqual": operator.ge,
        "lessOrEqual": operator.le,
        "notEqualTo": operator.ne
    }

    actor_types = {
        "pedestrian": "walker",
        "vehicle": "vehicle",
        "miscellaneous": "miscellaneous"
    }

    tl_states = {
        "GREEN": carla.TrafficLightState.Green,
        "YELLOW": carla.TrafficLightState.Yellow,
        "RED": carla.TrafficLightState.Red,
        "OFF": carla.TrafficLightState.Off,
    }

    global_osc_parameters = {}
    use_carla_coordinate_system = False
    osc_filepath = None

    @staticmethod
    def get_traffic_light_from_osc_name(name):
        """
        Returns a carla.TrafficLight instance that matches the name given
        """
        traffic_light = None

        # Given by id
        if name.startswith("id="):
            tl_id = int(name[3:])
            for carla_tl in CarlaDataProvider.get_world().get_actors().filter('traffic.traffic_light'):
                if carla_tl.id == tl_id:
                    traffic_light = carla_tl
                    break
        # Given by position
        elif name.startswith("pos="):
            tl_pos = name[4:]
            pos = tl_pos.split(",")
            for carla_tl in CarlaDataProvider.get_world().get_actors().filter('traffic.traffic_light'):
                carla_tl_location = carla_tl.get_transform().location
                distance = carla_tl_location.distance(carla.Location(float(pos[0]),
                                                                     float(pos[1]),
                                                                     carla_tl_location.z))
                if distance < 2.0:
                    traffic_light = carla_tl
                    break

        if traffic_light is None:
            raise AttributeError("Unknown  traffic light {}".format(name))

        return traffic_light

    @staticmethod
    def set_osc_filepath(filepath):
        """
        Set path of OSC file. This is required if for example custom commands are provided with
        relative paths.
        """
        OpenScenarioParser.osc_filepath = filepath

    @staticmethod
    def set_use_carla_coordinate_system():
        """
        CARLA internally uses a left-hand coordinate system (Unreal), but OpenSCENARIO and OpenDRIVE
        are intended for right-hand coordinate system. Hence, we need to invert the coordinates, if
        the scenario does not use CARLA coordinates, but instead right-hand coordinates.
        """
        OpenScenarioParser.use_carla_coordinate_system = True

    @staticmethod
    def set_parameters(xml_tree, additional_parameter_dict=None):
        """
        Parse the xml_tree, and replace all parameter references
        with the actual values.

        Note: Parameter names must not start with "$", however when referencing a parameter the
              reference has to start with "$".
              https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_re_use_mechanisms

        Args:
            xml_tree: Containing all nodes that should be updated
            additional_parameter_dict (dictionary): Additional parameters as dict (key, value). Optional.

        returns:
            updated xml_tree, dictonary containing all parameters and their values
        """

        parameter_dict = {}
        parameters = xml_tree.find('ParameterDeclarations')

        if parameters is None and not parameter_dict:
            return xml_tree, parameter_dict

        if parameters is None:
            parameters = []

        for parameter in parameters:
            name = parameter.attrib.get('name')
            value = parameter.attrib.get('value')
            parameter_dict[name] = value

        # overwrite parameters in parameters_dict by additional_parameters_dict
        if additional_parameter_dict is not None:
            parameter_dict = dict(list(parameter_dict.items()) + list(additional_parameter_dict.items()))

        return xml_tree, parameter_dict

    @staticmethod
    def set_global_parameters(parameter_dict):
        """
        Set global_osc_parameter dictionary

        Args:
            parameter_dict (Dictionary): Input for global_osc_parameter
        """
        OpenScenarioParser.global_osc_parameters = parameter_dict
        CarlaDataProvider.update_osc_global_params(parameter_dict)

    @staticmethod
    def get_catalog_entry(catalogs, catalog_reference):
        """
        Get catalog entry referenced by catalog_reference included correct parameter settings

        Args:
            catalogs (Dictionary of dictionaries): List of all catalogs and their entries
            catalog_reference (XML ElementTree): Reference containing the exact catalog to be used

        returns:
            Catalog entry (XML ElementTree)
        """
        entry_name = str(ParameterRef(catalog_reference.attrib.get("entryName")))
        entry = catalogs[catalog_reference.attrib.get("catalogName")][entry_name]
        entry_copy = copy.deepcopy(entry)
        catalog_copy = copy.deepcopy(catalog_reference)
        entry = OpenScenarioParser.assign_catalog_parameters(entry_copy, catalog_copy)

        return entry

    @staticmethod
    def assign_catalog_parameters(entry_instance, catalog_reference):
        """
        Parse catalog_reference, and replace all parameter references
        in entry_instance by the values provided in catalog_reference.

        Not to be used from outside this class.

        Args:
            entry_instance (XML ElementTree): Entry to be updated
            catalog_reference (XML ElementTree): Reference containing the exact parameter values

        returns:
            updated entry_instance with updated parameter values
        """

        parameter_dict = {}
        for elem in entry_instance.iter():
            if elem.find('ParameterDeclarations') is not None:
                parameters = elem.find('ParameterDeclarations')
                for parameter in parameters:
                    name = parameter.attrib.get('name')
                    value = parameter.attrib.get('value')
                    parameter_dict[name] = value

        for parameter_assignments in catalog_reference.iter("ParameterAssignments"):
            for parameter_assignment in parameter_assignments.iter("ParameterAssignment"):
                parameter = parameter_assignment.attrib.get("parameterRef")
                value = parameter_assignment.attrib.get("value")
                parameter_dict[parameter] = value

        for node in entry_instance.iter():
            for key in node.attrib:
                for param in sorted(parameter_dict, key=len, reverse=True):
                    if "$" + param in node.attrib[key]:
                        node.attrib[key] = node.attrib[key].replace("$" + param, parameter_dict[param])

        OpenScenarioParser.set_parameters(entry_instance, OpenScenarioParser.global_osc_parameters)

        return entry_instance

    @staticmethod
    def get_friction_from_env_action(xml_tree, catalogs):
        """
        Extract the CARLA road friction coefficient from an OSC EnvironmentAction

        Args:
            xml_tree: Containing the EnvironmentAction,
                or the reference to the catalog it is defined in.
            catalogs: XML Catalogs that could contain the EnvironmentAction

        returns:
           friction (float)
        """

        if xml_tree.findall('.//EnvironmentAction'):
            node = xml_tree.findall('.//EnvironmentAction')[0]
            set_environment = next(node.iter("EnvironmentAction"))
        else:
            return 1.0

        if sum(1 for _ in set_environment.iter("Weather")) != 0:
            environment = set_environment.find("Environment")
        elif set_environment.find("CatalogReference") is not None:
            catalog_reference = set_environment.find("CatalogReference")
            environment = OpenScenarioParser.get_catalog_entry(catalogs, catalog_reference)

        friction = 1.0

        road_condition = environment.iter("RoadCondition")
        for condition in road_condition:
            friction = condition.attrib.get('frictionScaleFactor')

        return friction

    @staticmethod
    def get_weather_from_env_action(xml_tree, catalogs):
        """
        Extract the CARLA weather parameters from an OSC EnvironmentAction

        Args:
            xml_tree: Containing the EnvironmentAction,
                or the reference to the catalog it is defined in.
            catalogs: XML Catalogs that could contain the EnvironmentAction

        returns:
           Weather (srunner.scenariomanager.weather_sim.Weather)
        """

        if xml_tree.findall('.//EnvironmentAction'):
            node = xml_tree.findall('.//EnvironmentAction')[0]
            set_environment = next(node.iter("EnvironmentAction"))
        else:
            return Weather(carla.WeatherParameters())

        if sum(1 for _ in set_environment.iter("Weather")) != 0:
            environment = set_environment.find("Environment")
        elif set_environment.find("CatalogReference") is not None:
            catalog_reference = set_environment.find("CatalogReference")
            environment = OpenScenarioParser.get_catalog_entry(catalogs, catalog_reference)

        weather = environment.find("Weather")
        sun = weather.find("Sun")

        carla_weather = carla.WeatherParameters()
        carla_weather.sun_azimuth_angle = math.degrees(float(sun.attrib.get('azimuth', 0)))
        carla_weather.sun_altitude_angle = math.degrees(float(sun.attrib.get('elevation', 0)))
        carla_weather.cloudiness = 100 - float(sun.attrib.get('intensity', 0)) * 100
        fog = weather.find("Fog")
        carla_weather.fog_distance = float(fog.attrib.get('visualRange', 'inf'))
        if carla_weather.fog_distance < 1000:
            carla_weather.fog_density = 100
        carla_weather.precipitation = 0
        carla_weather.precipitation_deposits = 0
        carla_weather.wetness = 0
        carla_weather.wind_intensity = 30.0
        precepitation = weather.find("Precipitation")
        if precepitation.attrib.get('precipitationType') == "rain":
            carla_weather.precipitation = float(precepitation.attrib.get('intensity')) * 100
            carla_weather.precipitation_deposits = 100  # if it rains, make the road wet
            carla_weather.wetness = carla_weather.precipitation
        elif precepitation.attrib.get('type') == "snow":
            raise AttributeError("CARLA does not support snow precipitation")

        time_of_day = environment.find("TimeOfDay")
        weather_animation = strtobool(time_of_day.attrib.get("animation"))
        time = time_of_day.attrib.get("dateTime")
        dtime = datetime.datetime.strptime(time, "%Y-%m-%dT%H:%M:%S")

        return Weather(carla_weather, dtime, weather_animation)

    @staticmethod
    def get_controller(xml_tree, catalogs):
        """
        Extract the object controller from the OSC XML or a catalog

        Args:
            xml_tree: Containing the controller information,
                or the reference to the catalog it is defined in.
            catalogs: XML Catalogs that could contain the EnvironmentAction

        returns:
           module: Python module containing the controller implementation
           args: Dictonary with (key, value) parameters for the controller
        """

        assign_action = next(xml_tree.iter("AssignControllerAction"))

        properties = None
        if assign_action.find('Controller') is not None:
            properties = assign_action.find('Controller').find('Properties')
        elif assign_action.find("CatalogReference") is not None:
            catalog_reference = assign_action.find("CatalogReference")
            properties = OpenScenarioParser.get_catalog_entry(catalogs, catalog_reference).find('Properties')

        module = None
        args = {}
        for prop in properties:
            if prop.attrib.get('name') == "module":
                module = prop.attrib.get('value')
            else:
                args[prop.attrib.get('name')] = prop.attrib.get('value')

        override_action = xml_tree.find('OverrideControllerValueAction')
        for child in override_action:
            if strtobool(child.attrib.get('active')):
                raise NotImplementedError("Controller override actions are not yet supported")

        return module, args

    @staticmethod
    def get_route(xml_tree, catalogs):
        """
        Extract the route from the OSC XML or a catalog

        Args:
            xml_tree: Containing the route information,
                or the reference to the catalog it is defined in.
            catalogs: XML Catalogs that could contain the Route

        returns:
           waypoints: List of route waypoints = (waypoint, routing strategy)
                      where the strategy is a string indicating if the fastest/shortest/etc.
                      route should be used.
        """
        route = None

        if xml_tree.find('Route') is not None:
            route = xml_tree.find('Route')
        elif xml_tree.find('CatalogReference') is not None:
            catalog_reference = xml_tree.find("CatalogReference")
            route = OpenScenarioParser.get_catalog_entry(catalogs, catalog_reference)
        else:
            raise AttributeError("Unknown private FollowRoute action")

        waypoints = []

        if route is not None:
            for waypoint in route.iter('Waypoint'):
                position = waypoint.find('Position')
                routing_option = str(waypoint.attrib.get('routeStrategy'))
                waypoints.append((position, routing_option))
        else:
            raise AttributeError("No waypoints has been set")

        return waypoints

    @staticmethod
    def get_trajectory(xml_tree, catalogs):
        """
        Extract the trajectory from the OSC XML or a catalog

        Args:
            xml_tree: Containing the trajectory information,
                or the reference to the catalog it is defined in.
            catalogs: XML Catalogs that could contain the trajectory

        returns:
           waypoints: List of trajectory waypoints and times.
                      Only polylines are supported
        """
        trajectory = None
        waypoints = []
        times = []

        if xml_tree.find('Trajectory') is not None:
            trajectory = xml_tree.find('Trajectory')
        elif xml_tree.find('CatalogReference') is not None:
            catalog_reference = xml_tree.find("CatalogReference")
            trajectory = OpenScenarioParser.get_catalog_entry(catalogs, catalog_reference)
        else:
            raise AttributeError("Unknown private FollowTrajectory action")

        if trajectory is not None:
            shape = trajectory.find('Shape')
            if shape.find('Polyline') is not None:
                line = shape.find('Polyline')
                for vertex in line.iter('Vertex'):
                    times.append(float(vertex.get('time')))
                    waypoints.append(vertex.find('Position'))
            elif shape.find('Clothoid') is not None:
                raise AttributeError("Clothoid shapes are currently unsupported")
            elif shape.find('Nurbs') is not None:
                raise AttributeError("Nurbs shapes are currently unsupported")
            else:
                raise AttributeError("Requested shape {} isn't a valid shape".format(shape))
        else:
            raise AttributeError("No waypoints has been set")

        return waypoints, times

    @staticmethod
    def convert_position_to_transform(position, actor_list=None):
        """
        Convert an OpenScenario position into a CARLA transform

        Not supported: RoutePosition
        """

        if position.find('WorldPosition') is not None:
            world_pos = position.find('WorldPosition')
            x = float(ParameterRef(world_pos.attrib.get('x', 0)))
            y = float(ParameterRef(world_pos.attrib.get('y', 0)))
            z = float(ParameterRef(world_pos.attrib.get('z', 0)))
            yaw = math.degrees(float(ParameterRef(world_pos.attrib.get('h', 0))))
            pitch = math.degrees(float(ParameterRef(world_pos.attrib.get('p', 0))))
            roll = math.degrees(float(ParameterRef(world_pos.attrib.get('r', 0))))
            if not OpenScenarioParser.use_carla_coordinate_system:
                y = y * (-1.0)
                yaw = yaw * (-1.0)
            return carla.Transform(carla.Location(x=x, y=y, z=z), carla.Rotation(yaw=yaw, pitch=pitch, roll=roll))

        elif ((position.find('RelativeWorldPosition') is not None) or
              (position.find('RelativeObjectPosition') is not None) or
              (position.find('RelativeLanePosition') is not None) or
              (position.find('RelativeRoadPosition') is not None)):

            if position.find('RelativeWorldPosition') is not None:
                rel_pos = position.find('RelativeWorldPosition')
            if position.find('RelativeObjectPosition') is not None:
                rel_pos = position.find('RelativeObjectPosition')
            if position.find('RelativeLanePosition') is not None:
                rel_pos = position.find('RelativeLanePosition')
            if position.find('RelativeRoadPosition') is not None:
                rel_pos = position.find('RelativeRoadPosition')

            # get relative object and relative position
            obj = rel_pos.attrib.get('entityRef')
            obj_actor = None
            actor_transform = None

            if actor_list is not None:
                for actor in actor_list:
                    if actor.rolename == obj:
                        obj_actor = actor
                        actor_transform = actor.transform
            else:
                for actor in CarlaDataProvider.get_world().get_actors():
                    if 'role_name' in actor.attributes and actor.attributes['role_name'] == obj:
                        obj_actor = actor
                        actor_transform = obj_actor.get_transform()
                        break

            if obj_actor is None or actor_transform is None:
                raise AttributeError("Object '{}' provided as position reference is not known".format(obj))

            # calculate orientation h, p, r
            is_absolute = False
            dyaw = 0
            dpitch = 0
            droll = 0
            if rel_pos.find('Orientation') is not None:
                orientation = rel_pos.find('Orientation')
                is_absolute = (orientation.attrib.get('type') == "absolute")
                dyaw = math.degrees(float(ParameterRef(orientation.attrib.get('h', 0))))
                dpitch = math.degrees(float(ParameterRef(orientation.attrib.get('p', 0))))
                droll = math.degrees(float(ParameterRef(orientation.attrib.get('r', 0))))

            if not OpenScenarioParser.use_carla_coordinate_system:
                dyaw = dyaw * (-1.0)

            yaw = actor_transform.rotation.yaw
            pitch = actor_transform.rotation.pitch
            roll = actor_transform.rotation.roll

            if not is_absolute:
                yaw = yaw + dyaw
                pitch = pitch + dpitch
                roll = roll + droll
            else:
                yaw = dyaw
                pitch = dpitch
                roll = droll

            # calculate location x, y, z
            # dx, dy, dz
            if ((position.find('RelativeWorldPosition') is not None) or
                    (position.find('RelativeObjectPosition') is not None)):
                dx = float(ParameterRef(rel_pos.attrib.get('dx', 0)))
                dy = float(ParameterRef(rel_pos.attrib.get('dy', 0)))
                dz = float(ParameterRef(rel_pos.attrib.get('dz', 0)))

                if not OpenScenarioParser.use_carla_coordinate_system:
                    dy = dy * (-1.0)

                x = actor_transform.location.x + dx
                y = actor_transform.location.y + dy
                z = actor_transform.location.z + dz

                transform = carla.Transform(carla.Location(x=x, y=y, z=z),
                                            carla.Rotation(yaw=yaw, pitch=pitch, roll=roll))

            elif position.find('RelativeLanePosition') is not None:
                dlane = float(ParameterRef(rel_pos.attrib.get('dLane')))
                ds = float(ParameterRef(rel_pos.attrib.get('ds')))
                offset = float(ParameterRef(rel_pos.attrib.get('offset', 0.0)))

                carla_map = CarlaDataProvider.get_map()
                relative_waypoint = carla_map.get_waypoint(actor_transform.location)

                road_id, ref_lane_id, ref_s = relative_waypoint.road_id, relative_waypoint.lane_id, relative_waypoint.s
                target_lane_id = int(ref_lane_id + dlane)
                waypoint = CarlaDataProvider.get_map().get_waypoint_xodr(road_id, target_lane_id, ref_s)
                if waypoint is not None:
                    if ds < 0:
                        ds = (-1.0) * ds
                        waypoint = waypoint.previous(ds)[-1]
                    else:
                        waypoint = waypoint.next(ds)[-1]

                if waypoint is None:
                    raise AttributeError("RelativeLanePosition " +
                                         "'roadId={} with ds={} and lane_id={}' does not exist".format(road_id,
                                                                                                       ds,
                                                                                                       target_lane_id))

                transform = waypoint.transform
                transform.rotation.yaw = yaw
                transform.rotation.pitch = pitch
                transform.rotation.roll = roll

                # Adapt transform according to offset
                if abs(offset) == waypoint.lane_width:
                    # if the offset position is exactly on lane edge its difficult to find out which lane its on.
                    # so, when offset is on lane edge/corner adjust it to stay inside the lane
                    # if user wants to offset out of lane then make sure offset > waypoint.lane_width
                    if offset > 0:
                        offset = offset - 0.05
                    elif offset < 0:
                        offset = offset + 0.05
                transform = get_offset_transform(transform, offset)
            elif position.find('RelativeRoadPosition') is not None:
                ds = float(ParameterRef(rel_pos.attrib.get('ds')))
                dt = float(ParameterRef(rel_pos.attrib.get('dt', 0.0)))
                troad = get_troad_from_transform

                carla_map = CarlaDataProvider.get_map()
                relative_waypoint = carla_map.get_waypoint(actor_transform.location)

                road_id, ref_lane_id, ref_s = relative_waypoint.road_id, relative_waypoint.lane_id, relative_waypoint.s
                target_t, target_s = troad(relative_waypoint.transform) - troad(actor_transform) + dt, ref_s + ds
                waypoint = CarlaDataProvider.get_map().get_waypoint_xodr(road_id, ref_lane_id, target_s)
                if waypoint is None:
                    raise AttributeError("RelativeRoadPosition 'roadId={} with s={} and t={}' does not exist".format(
                        road_id, target_s, target_t))

                transform = waypoint.transform
                transform.rotation.yaw = yaw
                transform.rotation.pitch = pitch
                transform.rotation.roll = roll
                transform = get_offset_transform(transform, target_t)

            return transform

        elif position.find('RoadPosition') is not None:
            road_pos = position.find('RoadPosition')
            road_id = int(ParameterRef(road_pos.attrib.get('roadId', 0)))
            t = float(ParameterRef(road_pos.attrib.get('t', 0)))
            s = float(ParameterRef(road_pos.attrib.get('s', 0)))

            waypoint = CarlaDataProvider.get_map().get_waypoint_xodr(road_id, 0, s)
            if waypoint is None:
                raise AttributeError("RoadPosition 'roadId={} with s={} and t={}' does not exist".format(road_id, s, t))

            transform = waypoint.transform

            if road_pos.find('Orientation') is not None:
                orientation = road_pos.find('Orientation')
                dyaw = math.degrees(float(ParameterRef(orientation.attrib.get('h', 0))))
                dpitch = math.degrees(float(ParameterRef(orientation.attrib.get('p', 0))))
                droll = math.degrees(float(ParameterRef(orientation.attrib.get('r', 0))))

                if not OpenScenarioParser.use_carla_coordinate_system:
                    dyaw = dyaw * (-1.0)

                transform.rotation.yaw = transform.rotation.yaw + dyaw
                transform.rotation.pitch = transform.rotation.pitch + dpitch
                transform.rotation.roll = transform.rotation.roll + droll

            if not OpenScenarioParser.use_carla_coordinate_system:
                # multiply -1 because unlike offset t road is -ve for right side
                t = -1*t
            transform = get_offset_transform(transform, t)
            return transform

        elif position.find('LanePosition') is not None:
            lane_pos = position.find('LanePosition')
            road_id = int(ParameterRef(lane_pos.attrib.get('roadId', 0)))
            lane_id = int(ParameterRef(lane_pos.attrib.get('laneId', 0)))
            offset = float(ParameterRef(lane_pos.attrib.get('offset', 0)))
            s = float(ParameterRef(lane_pos.attrib.get('s', 0)))
            waypoint = CarlaDataProvider.get_map().get_waypoint_xodr(road_id, lane_id, s)
            if waypoint is None:
                raise AttributeError("LanePosition 'roadId={}, laneId={}, s={}, offset={}' does not exist".format(
                    road_id, lane_id, s, offset))

            transform = waypoint.transform
            if lane_pos.find('Orientation') is not None:
                orientation = lane_pos.find('Orientation')
                dyaw = math.degrees(float(ParameterRef(orientation.attrib.get('h', 0))))
                dpitch = math.degrees(float(ParameterRef(orientation.attrib.get('p', 0))))
                droll = math.degrees(float(ParameterRef(orientation.attrib.get('r', 0))))

                if not OpenScenarioParser.use_carla_coordinate_system:
                    dyaw = dyaw * (-1.0)

                transform.rotation.yaw = transform.rotation.yaw + dyaw
                transform.rotation.pitch = transform.rotation.pitch + dpitch
                transform.rotation.roll = transform.rotation.roll + droll

            if offset != 0:
                forward_vector = transform.rotation.get_forward_vector()
                orthogonal_vector = carla.Vector3D(x=-forward_vector.y, y=forward_vector.x, z=forward_vector.z)
                transform.location.x = transform.location.x + offset * orthogonal_vector.x
                transform.location.y = transform.location.y + offset * orthogonal_vector.y

            return transform
        elif position.find('RoutePosition') is not None:
            raise NotImplementedError("Route positions are not yet supported")
        else:
            raise AttributeError("Unknown position")

    @staticmethod
    def convert_condition_to_atomic(condition, actor_list):
        """
        Convert an OpenSCENARIO condition into a Behavior/Criterion atomic

        If there is a delay defined in the condition, then the condition is checked after the delay time
        passed by, e.g. <Condition name="" delay="5">.

        Note: Not all conditions are currently supported.
        """

        atomic = None
        delay_atomic = None
        condition_name = condition.attrib.get('name')

        if condition.attrib.get('delay') is not None and float(condition.attrib.get('delay')) != 0:
            delay = float(condition.attrib.get('delay'))
            delay_atomic = TimeOut(delay)

        if condition.find('ByEntityCondition') is not None:

            trigger_actor = None    # A-priori validation ensures that this will be not None
            triggered_actor = None

            for triggering_entities in condition.find('ByEntityCondition').iter('TriggeringEntities'):
                for entity in triggering_entities.iter('EntityRef'):
                    for actor in actor_list:
                        if actor is not None and entity.attrib.get('entityRef', None) == actor.attributes['role_name']:
                            trigger_actor = actor
                            break

            for entity_condition in condition.find('ByEntityCondition').iter('EntityCondition'):
                if entity_condition.find('EndOfRoadCondition') is not None:
                    end_road_condition = entity_condition.find('EndOfRoadCondition')
                    condition_duration = ParameterRef(end_road_condition.attrib.get('duration'))
                    atomic_cls = py_trees.meta.inverter(EndofRoadTest)
                    atomic = atomic_cls(trigger_actor, condition_duration, terminate_on_failure=True,
                                        name=condition_name)
                elif entity_condition.find('CollisionCondition') is not None:

                    collision_condition = entity_condition.find('CollisionCondition')

                    if collision_condition.find('EntityRef') is not None:
                        collision_entity = collision_condition.find('EntityRef')

                        for actor in actor_list:
                            if collision_entity.attrib.get('entityRef', None) == actor.attributes['role_name']:
                                triggered_actor = actor
                                break

                        if triggered_actor is None:
                            raise AttributeError("Cannot find actor '{}' for condition".format(
                                collision_condition.attrib.get('entityRef', None)))

                        atomic_cls = py_trees.meta.inverter(CollisionTest)
                        atomic = atomic_cls(trigger_actor, other_actor=triggered_actor, terminate_on_failure=True,
                                            name=condition_name)

                    elif collision_condition.find('ByType') is not None:
                        collision_type = collision_condition.find('ByType').attrib.get('type', None)

                        triggered_type = OpenScenarioParser.actor_types[collision_type]

                        atomic_cls = py_trees.meta.inverter(CollisionTest)
                        atomic = atomic_cls(trigger_actor, other_actor_type=triggered_type, terminate_on_failure=True,
                                            name=condition_name)

                    else:
                        atomic_cls = py_trees.meta.inverter(CollisionTest)
                        atomic = atomic_cls(trigger_actor, terminate_on_failure=True, name=condition_name)

                elif entity_condition.find('OffroadCondition') is not None:
                    off_condition = entity_condition.find('OffroadCondition')
                    condition_duration = ParameterRef(off_condition.attrib.get('duration'))
                    atomic_cls = py_trees.meta.inverter(OffRoadTest)
                    atomic = atomic_cls(trigger_actor, condition_duration, terminate_on_failure=True,
                                        name=condition_name)
                elif entity_condition.find('TimeHeadwayCondition') is not None:
                    headtime_condition = entity_condition.find('TimeHeadwayCondition')

                    condition_value = ParameterRef(headtime_condition.attrib.get('value'))

                    condition_rule = headtime_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]

                    condition_freespace = strtobool(headtime_condition.attrib.get('freespace', False))
                    condition_along_route = strtobool(headtime_condition.attrib.get('alongRoute', False))

                    for actor in actor_list:
                        if headtime_condition.attrib.get('entityRef', None) == actor.attributes['role_name']:
                            triggered_actor = actor
                            break
                    if triggered_actor is None:
                        raise AttributeError("Cannot find actor '{}' for condition".format(
                            headtime_condition.attrib.get('entityRef', None)))

                    atomic = InTimeToArrivalToVehicle(
                        trigger_actor, triggered_actor, condition_value, condition_freespace,
                        condition_along_route, condition_operator, condition_name
                    )

                elif entity_condition.find('TimeToCollisionCondition') is not None:
                    ttc_condition = entity_condition.find('TimeToCollisionCondition')

                    condition_rule = ttc_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]

                    condition_value = ParameterRef(ttc_condition.attrib.get('value'))
                    condition_target = ttc_condition.find('TimeToCollisionConditionTarget')
                    entity_ref_ = condition_target.find('EntityRef')

                    condition_freespace = strtobool(ttc_condition.attrib.get('freespace', False))
                    condition_along_route = strtobool(ttc_condition.attrib.get('alongRoute', False))

                    if condition_target.find('Position') is not None:
                        position = condition_target.find('Position')
                        atomic = InTimeToArrivalToOSCPosition(
                            trigger_actor, position, condition_value, condition_along_route, condition_operator)
                    else:
                        for actor in actor_list:
                            if entity_ref_.attrib.get('entityRef', None) == actor.attributes['role_name']:
                                triggered_actor = actor
                                break
                        if triggered_actor is None:
                            raise AttributeError("Cannot find actor '{}' for condition".format(
                                entity_ref_.attrib.get('entityRef', None)))

                        atomic = InTimeToArrivalToVehicle(
                            trigger_actor, triggered_actor, condition_value, condition_freespace,
                            condition_along_route, condition_operator, condition_name)
                elif entity_condition.find('AccelerationCondition') is not None:
                    accel_condition = entity_condition.find('AccelerationCondition')
                    condition_value = ParameterRef(accel_condition.attrib.get('value'))
                    condition_rule = accel_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]
                    atomic = TriggerAcceleration(
                        trigger_actor, condition_value, condition_operator, condition_name)
                elif entity_condition.find('StandStillCondition') is not None:
                    ss_condition = entity_condition.find('StandStillCondition')
                    duration = ParameterRef(ss_condition.attrib.get('duration'))
                    atomic = StandStill(trigger_actor, condition_name, duration)
                elif entity_condition.find('SpeedCondition') is not None:
                    spd_condition = entity_condition.find('SpeedCondition')
                    condition_value = ParameterRef(spd_condition.attrib.get('value'))
                    condition_rule = spd_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]

                    atomic = TriggerVelocity(
                        trigger_actor, condition_value, condition_operator, condition_name)
                elif entity_condition.find('RelativeSpeedCondition') is not None:
                    relspd_condition = entity_condition.find('RelativeSpeedCondition')
                    condition_value = ParameterRef(relspd_condition.attrib.get('value'))
                    condition_rule = relspd_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]

                    for actor in actor_list:
                        if relspd_condition.attrib.get('entityRef', None) == actor.attributes['role_name']:
                            triggered_actor = actor
                            break

                    if triggered_actor is None:
                        raise AttributeError("Cannot find actor '{}' for condition".format(
                            relspd_condition.attrib.get('entityRef', None)))

                    atomic = RelativeVelocityToOtherActor(
                        trigger_actor, triggered_actor, condition_value, condition_operator, condition_name)
                elif entity_condition.find('TraveledDistanceCondition') is not None:
                    distance_condition = entity_condition.find('TraveledDistanceCondition')
                    distance_value = ParameterRef(distance_condition.attrib.get('value'))
                    atomic = DriveDistance(trigger_actor, distance_value, name=condition_name)
                elif entity_condition.find('ReachPositionCondition') is not None:
                    rp_condition = entity_condition.find('ReachPositionCondition')
                    distance_value = ParameterRef(rp_condition.attrib.get('tolerance'))
                    position = rp_condition.find('Position')
                    atomic = InTriggerDistanceToOSCPosition(
                        trigger_actor, position, distance_value, name=condition_name)
                elif entity_condition.find('DistanceCondition') is not None:
                    distance_condition = entity_condition.find('DistanceCondition')

                    distance_value = ParameterRef(distance_condition.attrib.get('value'))

                    distance_rule = distance_condition.attrib.get('rule')
                    distance_operator = OpenScenarioParser.operators[distance_rule]

                    distance_freespace = strtobool(distance_condition.attrib.get('freespace', False))
                    if distance_freespace:
                        raise NotImplementedError(
                            "DistanceCondition: freespace attribute is currently not implemented")
                    distance_along_route = strtobool(distance_condition.attrib.get('alongRoute', False))

                    if distance_condition.find('Position') is not None:
                        position = distance_condition.find('Position')
                        atomic = InTriggerDistanceToOSCPosition(
                            trigger_actor, position, distance_value, distance_along_route,
                            distance_operator, name=condition_name)

                elif entity_condition.find('RelativeDistanceCondition') is not None:
                    distance_condition = entity_condition.find('RelativeDistanceCondition')
                    distance_value = ParameterRef(distance_condition.attrib.get('value'))

                    distance_freespace = distance_condition.attrib.get('freespace', "false") == "true"
                    rel_dis_type = distance_condition.attrib.get('relativeDistanceType')
                    for actor in actor_list:
                        if distance_condition.attrib.get('entityRef', None) == actor.attributes['role_name']:
                            triggered_actor = actor
                            break

                    if triggered_actor is None:
                        raise AttributeError("Cannot find actor '{}' for condition".format(
                            distance_condition.attrib.get('entityRef', None)))

                    condition_rule = distance_condition.attrib.get('rule')
                    condition_operator = OpenScenarioParser.operators[condition_rule]
                    atomic = InTriggerDistanceToVehicle(triggered_actor, trigger_actor, distance_value,
                                                        condition_operator, distance_type=rel_dis_type,
                                                        freespace=distance_freespace, name=condition_name)
        elif condition.find('ByValueCondition') is not None:
            value_condition = condition.find('ByValueCondition')
            if value_condition.find('ParameterCondition') is not None:
                parameter_condition = value_condition.find('ParameterCondition')
                arg_name = parameter_condition.attrib.get('parameterRef')
                value = ParameterRef(parameter_condition.attrib.get('value'))
                rule = parameter_condition.attrib.get('rule')
                if condition_name.startswith('criteria_'):
                    if str(value) != '':
                        arg_value = float(value)
                    else:
                        arg_value = 0
                    class_name = condition_name[9:]
                    if class_name in globals():
                        criterion_instance = globals()[class_name]
                    else:
                        raise AttributeError(
                            "The condition {} cannot be mapped to a criterion atomic".format(class_name))

                    atomic = py_trees.composites.Parallel("Evaluation Criteria for multiple ego vehicles")
                    for triggered_actor in actor_list:
                        if arg_name != '':
                            atomic.add_child(criterion_instance(triggered_actor, arg_value))
                        else:
                            atomic.add_child(criterion_instance(triggered_actor))
                else:
                    atomic = CheckParameter(arg_name, value, OpenScenarioParser.operators[rule], name=condition_name)
            elif value_condition.find('SimulationTimeCondition') is not None:
                simtime_condition = value_condition.find('SimulationTimeCondition')
                value = ParameterRef(simtime_condition.attrib.get('value'))
                rule = OpenScenarioParser.operators[simtime_condition.attrib.get('rule')]
                atomic = SimulationTimeCondition(value, comparison_operator=rule)
            elif value_condition.find('TimeOfDayCondition') is not None:
                tod_condition = value_condition.find('TimeOfDayCondition')
                condition_date = tod_condition.attrib.get('dateTime')
                condition_rule = tod_condition.attrib.get('rule')
                condition_operator = OpenScenarioParser.operators[condition_rule]
                atomic = TimeOfDayComparison(condition_date, condition_operator, condition_name)
            elif value_condition.find('StoryboardElementStateCondition') is not None:
                state_condition = value_condition.find('StoryboardElementStateCondition')
                element_name = state_condition.attrib.get('storyboardElementRef')
                element_type = state_condition.attrib.get('storyboardElementType')
                state = state_condition.attrib.get('state')
                if state == "startTransition":
                    atomic = OSCStartEndCondition(element_type, element_name, rule="START", name=state + "Condition")
                elif state in ["stopTransition", "endTransition", "completeState"]:
                    atomic = OSCStartEndCondition(element_type, element_name, rule="END", name=state + "Condition")
                else:
                    raise NotImplementedError(
                        "Only start, stop, endTransitions and completeState are currently supported")
            elif value_condition.find('UserDefinedValueCondition') is not None:
                raise NotImplementedError("ByValue UserDefinedValue conditions are not yet supported")
            elif value_condition.find('TrafficSignalCondition') is not None:
                tl_condition = value_condition.find('TrafficSignalCondition')

                name_condition = tl_condition.attrib.get('name')
                traffic_light = OpenScenarioParser.get_traffic_light_from_osc_name(name_condition)

                tl_state = tl_condition.attrib.get('state').upper()
                if tl_state not in OpenScenarioParser.tl_states:
                    raise KeyError("CARLA only supports Green, Red, Yellow or Off")
                state_condition = OpenScenarioParser.tl_states[tl_state]

                atomic = WaitForTrafficLightState(
                    traffic_light, state_condition, name=condition_name)
            elif value_condition.find('TrafficSignalControllerCondition') is not None:
                raise NotImplementedError("ByValue TrafficSignalController conditions are not yet supported")
            else:
                raise AttributeError("Unknown ByValue condition")

        else:
            raise AttributeError("Unknown condition")

        if delay_atomic is not None and atomic is not None:
            new_atomic = py_trees.composites.Sequence("delayed sequence")
            new_atomic.add_child(delay_atomic)
            new_atomic.add_child(atomic)
        else:
            new_atomic = atomic

        return new_atomic

    @staticmethod
    def convert_maneuver_to_atomic(action, actor, actor_list, catalogs):
        """
        Convert an OpenSCENARIO maneuver action into a Behavior atomic

        Note not all OpenSCENARIO actions are currently supported
        """
        maneuver_name = action.attrib.get('name', 'unknown')

        if action.find('GlobalAction') is not None:
            global_action = action.find('GlobalAction')
            if global_action.find('InfrastructureAction') is not None:
                infrastructure_action = global_action.find('InfrastructureAction').find('TrafficSignalAction')
                if infrastructure_action.find('TrafficSignalStateAction') is not None:
                    traffic_light_action = infrastructure_action.find('TrafficSignalStateAction')

                    name_condition = traffic_light_action.attrib.get('name')
                    traffic_light = OpenScenarioParser.get_traffic_light_from_osc_name(name_condition)

                    tl_state = traffic_light_action.attrib.get('state').upper()
                    if tl_state not in OpenScenarioParser.tl_states:
                        raise KeyError("CARLA only supports Green, Red, Yellow or Off")
                    traffic_light_state = OpenScenarioParser.tl_states[tl_state]

                    atomic = TrafficLightStateSetter(
                        traffic_light, traffic_light_state, name=maneuver_name + "_" + str(traffic_light.id))
                else:
                    raise NotImplementedError("TrafficLights can only be influenced via TrafficSignalStateAction")
            elif global_action.find('EnvironmentAction') is not None:
                weather_behavior = ChangeWeather(
                    OpenScenarioParser.get_weather_from_env_action(global_action, catalogs))
                friction_behavior = ChangeRoadFriction(
                    OpenScenarioParser.get_friction_from_env_action(global_action, catalogs))

                env_behavior = py_trees.composites.Parallel(
                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name=maneuver_name)

                env_behavior.add_child(
                    oneshot_with_check(variable_name=maneuver_name + ">WeatherUpdate", behaviour=weather_behavior))
                env_behavior.add_child(
                    oneshot_with_check(variable_name=maneuver_name + ">FrictionUpdate", behaviour=friction_behavior))

                return env_behavior
            elif global_action.find('ParameterAction') is not None:
                parameter_action = global_action.find('ParameterAction')
                parameter_ref = parameter_action.attrib.get('parameterRef')
                if parameter_action.find('ModifyAction') is not None:
                    action_rule = parameter_action.find('ModifyAction').find("Rule")
                    if action_rule.find("AddValue") is not None:
                        rule, value = '+', action_rule.find("AddValue").attrib.get('value')
                    else:
                        rule, value = '*', action_rule.find("MultiplyByValue").attrib.get('value')
                else:
                    rule, value = None, parameter_action.find('SetAction').attrib.get('value')
                atomic = ChangeParameter(parameter_ref, value=ParameterRef(value), rule=rule, name=maneuver_name)
            else:
                raise NotImplementedError("Global actions are not yet supported")
        elif action.find('UserDefinedAction') is not None:
            user_defined_action = action.find('UserDefinedAction')
            if user_defined_action.find('CustomCommandAction') is not None:
                command = user_defined_action.find('CustomCommandAction').attrib.get('type')
                atomic = RunScript(command, base_path=OpenScenarioParser.osc_filepath, name=maneuver_name)
        elif action.find('PrivateAction') is not None:
            private_action = action.find('PrivateAction')

            if private_action.find('LongitudinalAction') is not None:
                private_action = private_action.find('LongitudinalAction')

                if private_action.find('SpeedAction') is not None:
                    long_maneuver = private_action.find('SpeedAction')

                    # duration and distance
                    distance = float('inf')
                    duration = float('inf')
                    dimension = long_maneuver.find("SpeedActionDynamics").attrib.get('dynamicsDimension')
                    if dimension == "distance":
                        distance = ParameterRef(long_maneuver.find("SpeedActionDynamics").attrib.get(
                            'value', float("inf")))
                    else:
                        duration = ParameterRef(long_maneuver.find("SpeedActionDynamics").attrib.get(
                            'value', float("inf")))

                    # absolute velocity with given target speed
                    if long_maneuver.find("SpeedActionTarget").find("AbsoluteTargetSpeed") is not None:
                        target_speed = ParameterRef(long_maneuver.find("SpeedActionTarget").find(
                            "AbsoluteTargetSpeed").attrib.get('value', 0))
                        atomic = ChangeActorTargetSpeed(
                            actor, float(target_speed), distance=distance, duration=duration, name=maneuver_name)

                    # relative velocity to given actor
                    if long_maneuver.find("SpeedActionTarget").find("RelativeTargetSpeed") is not None:
                        relative_speed = long_maneuver.find("SpeedActionTarget").find("RelativeTargetSpeed")
                        obj = relative_speed.attrib.get('entityRef')
                        value = ParameterRef(relative_speed.attrib.get('value', 0))
                        value_type = relative_speed.attrib.get('speedTargetValueType')
                        continuous = bool(strtobool(relative_speed.attrib.get('continuous')))

                        for traffic_actor in actor_list:
                            if (traffic_actor is not None and 'role_name' in traffic_actor.attributes and
                                    traffic_actor.attributes['role_name'] == obj):
                                obj_actor = traffic_actor

                        atomic = ChangeActorTargetSpeed(actor,
                                                        target_speed=0.0,
                                                        relative_actor=obj_actor,
                                                        value=value,
                                                        value_type=value_type,
                                                        continuous=continuous,
                                                        distance=distance,
                                                        duration=duration,
                                                        name=maneuver_name)

                elif private_action.find('LongitudinalDistanceAction') is not None:
                    long_dist_action = private_action.find("LongitudinalDistanceAction")
                    obj = long_dist_action.attrib.get('entityRef')
                    for traffic_actor in actor_list:
                        if (traffic_actor is not None and 'role_name' in traffic_actor.attributes and
                                traffic_actor.attributes['role_name'] == obj):
                            obj_actor = traffic_actor

                    if "distance" in long_dist_action.attrib and "timeGap" not in long_dist_action.attrib:
                        gap_type, gap = 'distance', ParameterRef(long_dist_action.attrib.get('distance'))
                    elif "timeGap" in long_dist_action.attrib and "distance" not in long_dist_action.attrib:
                        raise NotImplementedError("LongitudinalDistanceAction: timeGap is not implemented")
                    else:
                        raise ValueError("LongitudinalDistanceAction: " +
                                         "Please specify any one attribute of [distance, timeGap]")

                    constraints = long_dist_action.find('DynamicConstraints')
                    max_speed = constraints.attrib.get('maxSpeed', None) if constraints is not None else None
                    continues = bool(strtobool(long_dist_action.attrib.get('continuous')))
                    freespace = bool(strtobool(long_dist_action.attrib.get('freespace')))
                    atomic = KeepLongitudinalGap(actor, reference_actor=obj_actor, gap=gap, gap_type=gap_type,
                                                 max_speed=max_speed, continues=continues, freespace=freespace,
                                                 name=maneuver_name)
                else:
                    raise AttributeError("Unknown longitudinal action")
            elif private_action.find('LateralAction') is not None:
                private_action = private_action.find('LateralAction')
                if private_action.find('LaneChangeAction') is not None:
                    # Note: LaneChangeActions are currently only supported for RelativeTargetLane
                    lat_maneuver = private_action.find('LaneChangeAction')
                    target_lane_rel = ParameterRef(lat_maneuver.find("LaneChangeTarget").find(
                        "RelativeTargetLane").attrib.get('value', 0))
                    direction = "left" if target_lane_rel > 0 else "right"
                    lane_changes = abs(target_lane_rel)
                    # duration and distance
                    distance = float('inf')
                    duration = float('inf')
                    dimension = lat_maneuver.find("LaneChangeActionDynamics").attrib.get('dynamicsDimension')
                    if dimension == "distance":
                        distance = ParameterRef(
                            lat_maneuver.find("LaneChangeActionDynamics").attrib.get('value', float("inf")))
                    else:
                        duration = ParameterRef(
                            lat_maneuver.find("LaneChangeActionDynamics").attrib.get('value', float("inf")))
                    atomic = ChangeActorLateralMotion(actor, direction=direction,
                                                      distance_lane_change=distance,
                                                      distance_other_lane=10,
                                                      lane_changes=lane_changes,
                                                      name=maneuver_name)
                elif private_action.find('LaneOffsetAction') is not None:
                    lat_maneuver = private_action.find('LaneOffsetAction')
                    continuous = bool(strtobool(lat_maneuver.attrib.get('continuous', "true")))
                    # Parsing of the different Dynamic shapes is missing

                    lane_target_offset = lat_maneuver.find('LaneOffsetTarget')
                    if lane_target_offset.find('AbsoluteTargetLaneOffset') is not None:
                        absolute_offset = ParameterRef(
                            lane_target_offset.find('AbsoluteTargetLaneOffset').attrib.get('value', 0))
                        atomic = ChangeActorLaneOffset(
                            actor, absolute_offset, continuous=continuous, name=maneuver_name)

                    elif lane_target_offset.find('RelativeTargetLaneOffset') is not None:
                        relative_target_offset = lane_target_offset.find('RelativeTargetLaneOffset')
                        relative_offset = ParameterRef(relative_target_offset.attrib.get('value', 0))

                        relative_actor = None
                        relative_actor_name = relative_target_offset.attrib.get('entityRef', None)
                        for _actor in actor_list:
                            if _actor is not None and 'role_name' in _actor.attributes:
                                if relative_actor_name == _actor.attributes['role_name']:
                                    relative_actor = _actor
                                    break
                        if relative_actor is None:
                            raise AttributeError("Cannot find actor '{}' for condition".format(relative_actor_name))

                        atomic = ChangeActorLaneOffset(actor, relative_offset, relative_actor,
                                                       continuous=continuous, name=maneuver_name)

                    else:
                        raise AttributeError("Unknown target offset")
                else:
                    raise AttributeError("Unknown lateral action")
            elif private_action.find('VisibilityAction') is not None:
                raise NotImplementedError("Visibility actions are not yet supported")
            elif private_action.find('SynchronizeAction') is not None:
                sync_action = private_action.find('SynchronizeAction')

                master_actor = None
                for actor_ins in actor_list:
                    if actor_ins is not None and 'role_name' in actor_ins.attributes:
                        if sync_action.attrib.get('masterEntityRef', None) == actor_ins.attributes['role_name']:
                            master_actor = actor_ins
                            break

                if master_actor is None:
                    raise AttributeError("Cannot find actor '{}' for condition".format(
                        sync_action.attrib.get('masterEntityRef', None)))

                master_position = OpenScenarioParser.convert_position_to_transform(
                    sync_action.find('TargetPositionMaster'))
                position = OpenScenarioParser.convert_position_to_transform(sync_action.find('TargetPosition'))

                if sync_action.find("FinalSpeed").find("AbsoluteSpeed") is not None:
                    final_speed = ParameterRef(sync_action.find("FinalSpeed").find(
                        "AbsoluteSpeed").attrib.get('value', 0))
                    atomic = SyncArrivalOSC(
                        actor, master_actor, position, master_position, final_speed, name=maneuver_name)

                # relative velocity to given actor
                elif sync_action.find("FinalSpeed").find("RelativeSpeedToMaster") is not None:
                    relative_speed = sync_action.find("FinalSpeed").find("RelativeSpeedToMaster")
                    final_speed = ParameterRef(relative_speed.attrib.get('value', 0))
                    relative_type = relative_speed.attrib.get('speedTargetValueType')
                    atomic = SyncArrivalOSC(
                        actor, master_actor, position, master_position, final_speed,
                        relative_to_master=True, relative_type=relative_type, name=maneuver_name)
                else:
                    raise AttributeError("Unknown speed action")
            elif private_action.find('ActivateControllerAction') is not None:
                private_action = private_action.find('ActivateControllerAction')
                activate = strtobool(private_action.attrib.get('longitudinal'))
                atomic = ChangeAutoPilot(actor, activate, name=maneuver_name)
            elif private_action.find('ControllerAction') is not None:
                controller_action = private_action.find('ControllerAction')
                module, args = OpenScenarioParser.get_controller(controller_action, catalogs)
                atomic = ChangeActorControl(actor, control_py_module=module, args=args,
                                            scenario_file_path=OpenScenarioParser.osc_filepath)
            elif private_action.find('TeleportAction') is not None:
                teleport_action = private_action.find('TeleportAction')
                position = teleport_action.find('Position')
                atomic = ActorTransformSetterToOSCPosition(actor, position, name=maneuver_name)
            elif private_action.find('RoutingAction') is not None:
                private_action = private_action.find('RoutingAction')
                if private_action.find('AssignRouteAction') is not None:
                    # @TODO: How to handle relative positions here? This might chance at runtime?!
                    route_action = private_action.find('AssignRouteAction')
                    waypoints = OpenScenarioParser.get_route(route_action, catalogs)
                    atomic = ChangeActorWaypoints(actor, waypoints=waypoints, name=maneuver_name)
                elif private_action.find('FollowTrajectoryAction') is not None:
                    trajectory_action = private_action.find('FollowTrajectoryAction')
                    waypoints, times = OpenScenarioParser.get_trajectory(trajectory_action, catalogs)
                    atomic = ChangeActorWaypoints(actor, waypoints=list(zip(waypoints, ['shortest'] * len(waypoints))),
                                                  times=times, name=maneuver_name)
                elif private_action.find('AcquirePositionAction') is not None:
                    route_action = private_action.find('AcquirePositionAction')
                    osc_position = route_action.find('Position')
                    waypoints = [(osc_position, 'fastest')]
                    atomic = ChangeActorWaypoints(actor, waypoints=waypoints, name=maneuver_name)
                else:
                    raise AttributeError("Unknown private routing action")
            else:
                raise AttributeError("Unknown private action")

        else:
            if list(action):
                raise AttributeError("Unknown action: {}".format(maneuver_name))
            else:
                return Idle(duration=0, name=maneuver_name)

        return atomic
