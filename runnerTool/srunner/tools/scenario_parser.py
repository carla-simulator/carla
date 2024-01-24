#!/usr/bin/env python

# Copyright (c) 2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides access to a scenario configuration parser
"""

import glob
import os
import xml.etree.ElementTree as ET

from srunner.scenarioconfigs.scenario_configuration import ScenarioConfiguration, ActorConfigurationData
from srunner.scenarioconfigs.route_scenario_configuration import RouteConfiguration


class ScenarioConfigurationParser(object):

    """
    Pure static class providing access to parser methods for scenario configuration files (*.xml)
    """

    @staticmethod
    def parse_scenario_configuration(scenario_name, config_file_name):
        """
        Parse all scenario configuration files at srunner/examples and the additional
        config files, providing a list of ScenarioConfigurations @return

        If scenario_name starts with "group:" all scenarios that
        have that type are parsed and returned. Otherwise only the
        scenario that matches the scenario_name is parsed and returned.
        """

        list_of_config_files = glob.glob("{}/srunner/examples/*.xml".format(os.getenv('SCENARIO_RUNNER_ROOT', "./")))

        if config_file_name != '':
            list_of_config_files.append(config_file_name)

        single_scenario_only = True
        if scenario_name.startswith("group:"):
            single_scenario_only = False
            scenario_name = scenario_name[6:]

        scenario_configurations = []

        for file_name in list_of_config_files:
            tree = ET.parse(file_name)

            for scenario in tree.iter("scenario"):

                scenario_config_name = scenario.attrib.get('name', None)
                scenario_config_type = scenario.attrib.get('type', None)

                if single_scenario_only:
                    # Check the scenario is the correct one
                    if scenario_config_name != scenario_name:
                        continue
                else:
                    # Check the scenario is of the correct type
                    if scenario_config_type != scenario_name:
                        continue

                new_config = ScenarioConfiguration()
                new_config.town = scenario.attrib.get('town', None)
                new_config.name = scenario_config_name
                new_config.type = scenario_config_type
                new_config.other_actors = []
                new_config.ego_vehicles = []
                new_config.trigger_points = []

                for weather in scenario.iter("weather"):
                    new_config.weather.cloudiness = float(weather.attrib.get("cloudiness", 0))
                    new_config.weather.precipitation = float(weather.attrib.get("precipitation", 0))
                    new_config.weather.precipitation_deposits = float(weather.attrib.get("precipitation_deposits", 0))
                    new_config.weather.wind_intensity = float(weather.attrib.get("wind_intensity", 0.35))
                    new_config.weather.sun_azimuth_angle = float(weather.attrib.get("sun_azimuth_angle", 0.0))
                    new_config.weather.sun_altitude_angle = float(weather.attrib.get("sun_altitude_angle", 15.0))
                    new_config.weather.fog_density = float(weather.attrib.get("fog_density", 0.0))
                    new_config.weather.fog_distance = float(weather.attrib.get("fog_distance", 0.0))
                    new_config.weather.wetness = float(weather.attrib.get("wetness", 0.0))

                for ego_vehicle in scenario.iter("ego_vehicle"):

                    new_config.ego_vehicles.append(ActorConfigurationData.parse_from_node(ego_vehicle, 'hero'))
                    new_config.trigger_points.append(new_config.ego_vehicles[-1].transform)

                for route in scenario.iter("route"):
                    route_conf = RouteConfiguration()
                    route_conf.parse_xml(route)
                    new_config.route = route_conf

                for other_actor in scenario.iter("other_actor"):
                    new_config.other_actors.append(ActorConfigurationData.parse_from_node(other_actor, 'scenario'))

                scenario_configurations.append(new_config)

        return scenario_configurations

    @staticmethod
    def get_list_of_scenarios(config_file_name):
        """
        Parse *all* config files and provide a list with all scenarios @return
        """

        list_of_config_files = glob.glob("{}/srunner/examples/*.xml".format(os.getenv('SCENARIO_RUNNER_ROOT', "./")))
        list_of_config_files += glob.glob("{}/srunner/examples/*.xosc".format(os.getenv('SCENARIO_RUNNER_ROOT', "./")))

        if config_file_name != '':
            list_of_config_files.append(config_file_name)

        scenarios = []
        for file_name in list_of_config_files:
            if ".xosc" in file_name:
                tree = ET.parse(file_name)
                scenarios.append("{} (OpenSCENARIO)".format(tree.find("FileHeader").attrib.get('description', None)))
            else:
                tree = ET.parse(file_name)
                for scenario in tree.iter("scenario"):
                    scenarios.append(scenario.attrib.get('name', None))

        return scenarios
