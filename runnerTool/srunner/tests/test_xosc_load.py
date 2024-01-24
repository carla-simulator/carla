#!/usr/bin/env python

# Copyright (c) 2021 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides some basic unit tests for the OpenSCENARIO feature of ScenarioRunner
"""

from unittest import TestCase
import glob
import carla
from srunner.scenarioconfigs.openscenario_configuration import OpenScenarioConfiguration
from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenarios.open_scenario import OpenScenario


class TestLoadingXOSC(TestCase):
    """
    Test class to load OpenSCENARIO files and test for exceptions
    """

    def test_all_xosc(self):
        """
        Load all examples OpenSCENARIO files
        """
        all_test_files = glob.glob('**/srunner/examples/*.xosc', recursive=True)

        for filename in all_test_files:
            client = carla.Client()
            config = OpenScenarioConfiguration(filename, client, {})
            self.assertTrue(config is not None)
            CarlaDataProvider.set_client(client)
            ego_vehicles = []
            for vehicle in config.ego_vehicles:
                ego_vehicles.append(CarlaDataProvider.request_new_actor(vehicle.model,
                                                                        vehicle.transform,
                                                                        vehicle.rolename,
                                                                        color=vehicle.color,
                                                                        actor_category=vehicle.category))

            scenario = OpenScenario(world=client.get_world(),
                                    ego_vehicles=ego_vehicles,
                                    config=config,
                                    config_file=filename,
                                    timeout=100000)
            self.assertTrue(scenario is not None)

            CarlaDataProvider.cleanup()
