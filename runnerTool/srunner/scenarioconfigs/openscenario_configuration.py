#!/usr/bin/env python

# Copyright (c) 2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides the key configuration parameters for a scenario based on OpenSCENARIO
"""

import logging
import os
import xml.etree.ElementTree as ET

import xmlschema

import carla

# pylint: disable=line-too-long
from srunner.scenarioconfigs.scenario_configuration import ActorConfigurationData, ScenarioConfiguration
# pylint: enable=line-too-long
from srunner.scenariomanager.carla_data_provider import CarlaDataProvider  # workaround
from srunner.tools.openscenario_parser import OpenScenarioParser, ParameterRef


class OpenScenarioConfiguration(ScenarioConfiguration):

    """
    Limitations:
    - Only one Story + Init is supported per Storyboard
    """

    def __init__(self, filename, client, custom_params):

        self.xml_tree = ET.parse(filename)
        self.filename = filename
        self._custom_params = custom_params if custom_params is not None else {}

        self._validate_openscenario_configuration()
        self.client = client

        self.catalogs = {}

        self.other_actors = []
        self.ego_vehicles = []
        self.trigger_points = []
        self.weather = carla.WeatherParameters()

        self.storyboard = self.xml_tree.find("Storyboard")
        self.stories = self.storyboard.findall("Story")
        self.init = self.storyboard.find("Init")

        logging.basicConfig()
        self.logger = logging.getLogger("[SR:OpenScenarioConfiguration]")

        self._global_parameters = {}

        self._set_parameters()
        self._parse_openscenario_configuration()

    def _validate_openscenario_configuration(self):
        """
        Validate the given OpenSCENARIO config against the 1.0 XSD

        Note: This will throw if the config is not valid. But this is fine here.
        """
        xsd_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../openscenario/OpenSCENARIO.xsd")
        xsd = xmlschema.XMLSchema(xsd_file)
        xsd.validate(self.xml_tree)

    def _validate_openscenario_catalog_configuration(self, catalog_xml_tree):
        """
        Validate the given OpenSCENARIO catalog config against the 1.0 XSD

        Note: This will throw if the catalog config is not valid. But this is fine here.
        """
        xsd_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../openscenario/OpenSCENARIO.xsd")
        xsd = xmlschema.XMLSchema(xsd_file)
        xsd.validate(catalog_xml_tree)

    def _parse_openscenario_configuration(self):
        """
        Parse the given OpenSCENARIO config file, set and validate parameters
        """
        OpenScenarioParser.set_osc_filepath(os.path.dirname(self.filename))

        self._check_version()
        self._load_catalogs()
        self._set_scenario_name()
        self._set_carla_town()
        self._set_actor_information()

        self._validate_result()

    def _check_version(self):
        """
        Ensure correct OpenSCENARIO version is used
        """
        header = self.xml_tree.find("FileHeader")
        if not (header.attrib.get('revMajor') == "1" and header.attrib.get('revMinor') == "0"):
            raise AttributeError("Only OpenSCENARIO 1.0 is supported")

    def _load_catalogs(self):
        """
        Read Catalog xml files into dictionary for later use

        NOTE: Catalogs must have distinct names, even across different types
        """
        catalogs = self.xml_tree.find("CatalogLocations")
        if list(catalogs) is None:
            return

        catalog_types = ["Vehicle",
                         "Controller",
                         "Pedestrian",
                         "MiscObject",
                         "Environment",
                         "Maneuver",
                         "Trajectory",
                         "Route"]
        for catalog_type in catalog_types:
            catalog = catalogs.find(catalog_type + "Catalog")
            if catalog is None:
                continue

            catalog_path = catalog.find("Directory").attrib.get('path') + "/" + catalog_type + "Catalog.xosc"
            if not os.path.isabs(catalog_path) and "xosc" in self.filename:
                catalog_path = os.path.dirname(os.path.abspath(self.filename)) + "/" + catalog_path

            if not os.path.isfile(catalog_path):
                self.logger.warning(" The %s path for the %s Catalog is invalid", catalog_path, catalog_type)
            else:
                xml_tree = ET.parse(catalog_path)
                self._validate_openscenario_catalog_configuration(xml_tree)
                catalog = xml_tree.find("Catalog")
                catalog_name = catalog.attrib.get("name")
                self.catalogs[catalog_name] = {}
                for entry in catalog:
                    self.catalogs[catalog_name][entry.attrib.get("name")] = entry

    def _set_scenario_name(self):
        """
        Extract the scenario name from the OpenSCENARIO header information
        """
        header = self.xml_tree.find("FileHeader")
        self.name = header.attrib.get('description', 'Unknown')

        if self.name.startswith("CARLA:"):
            self.name = self.name[6:]
            OpenScenarioParser.set_use_carla_coordinate_system()

    def _set_carla_town(self):
        """
        Extract the CARLA town (level) from the RoadNetwork information from OpenSCENARIO

        Note: The specification allows multiple Logics elements within the RoadNetwork element.
              Hence, there can be multiple towns specified. We just use the _last_ one.
        """
        for logic in self.xml_tree.find("RoadNetwork").findall("LogicFile"):
            self.town = logic.attrib.get('filepath', None)

        if self.town is not None and ".xodr" in self.town:
            if not os.path.isabs(self.town):
                self.town = os.path.dirname(os.path.abspath(self.filename)) + "/" + self.town
            if not os.path.exists(self.town):
                raise AttributeError("The provided RoadNetwork '{}' does not exist".format(self.town))

        # workaround for relative positions during init
        world = self.client.get_world()
        wmap = None
        if world:
            world.get_settings()
            wmap = world.get_map()

        if world is None or (wmap is not None and wmap.name.split('/')[-1] != self.town):
            if ".xodr" in self.town:
                with open(self.town, 'r', encoding='utf-8') as od_file:
                    data = od_file.read()
                index = data.find('<OpenDRIVE>')
                data = data[index:]

                old_map = ""
                if wmap is not None:
                    old_map = wmap.to_opendrive()
                    index = old_map.find('<OpenDRIVE>')
                    old_map = old_map[index:]

                if data != old_map:
                    self.logger.warning(" Wrong OpenDRIVE map in use. Forcing reload of CARLA world")

                    vertex_distance = 2.0  # in meters
                    wall_height = 1.0      # in meters
                    extra_width = 0.6      # in meters
                    world = self.client.generate_opendrive_world(str(data),
                                                                 carla.OpendriveGenerationParameters(
                                                                 vertex_distance=vertex_distance,
                                                                 wall_height=wall_height,
                                                                 additional_width=extra_width,
                                                                 smooth_junctions=True,
                                                                 enable_mesh_visibility=True))
            else:
                self.logger.warning(" Wrong map in use. Forcing reload of CARLA world")
                self.client.load_world(self.town)
                world = self.client.get_world()

            CarlaDataProvider.set_world(world)
            if CarlaDataProvider.is_sync_mode():
                world.tick()
            else:
                world.wait_for_tick()
        else:
            CarlaDataProvider.set_world(world)

    def _set_parameters(self):
        """
        Parse the complete scenario definition file, and replace all parameter references
        with the actual values

        Set _global_parameters.
        """
        self.xml_tree, self._global_parameters = OpenScenarioParser.set_parameters(self.xml_tree, self._custom_params)

        for elem in self.xml_tree.iter():
            if elem.find('ParameterDeclarations') is not None:
                elem, _ = OpenScenarioParser.set_parameters(elem)

        OpenScenarioParser.set_global_parameters(self._global_parameters)

    def _set_actor_information(self):
        """
        Extract all actors and their corresponding specification

        NOTE: The rolename property has to be unique!
        """
        for entity in self.xml_tree.iter("Entities"):
            for obj in entity.iter("ScenarioObject"):
                rolename = obj.attrib.get('name', 'simulation')
                args = {}
                for prop in obj.iter("Property"):
                    key = prop.get('name')
                    value = prop.get('value')
                    args[key] = value

                for catalog_reference in obj.iter("CatalogReference"):
                    entry = OpenScenarioParser.get_catalog_entry(self.catalogs, catalog_reference)
                    if entry.tag == "Vehicle":
                        self._extract_vehicle_information(entry, rolename, entry, args)
                    elif entry.tag == "Pedestrian":
                        self._extract_pedestrian_information(entry, rolename, entry, args)
                    elif entry.tag == "MiscObject":
                        self._extract_misc_information(entry, rolename, entry, args)
                    else:
                        self.logger.debug(
                            " A CatalogReference specifies a reference that is not an Entity. Skipping...")

                for vehicle in obj.iter("Vehicle"):
                    self._extract_vehicle_information(obj, rolename, vehicle, args)

                for pedestrian in obj.iter("Pedestrian"):
                    self._extract_pedestrian_information(obj, rolename, pedestrian, args)

                for misc in obj.iter("MiscObject"):
                    self._extract_misc_information(obj, rolename, misc, args)

        # Set transform for all actors
        # This has to be done in a multi-stage loop to resolve relative position settings
        all_actor_transforms_set = False
        while not all_actor_transforms_set:
            all_actor_transforms_set = True
            for actor in self.other_actors + self.ego_vehicles:
                if actor.transform is None:
                    try:
                        actor.transform = self._get_actor_transform(actor.rolename)
                    except AttributeError as e:
                        if "Object '" in str(e):
                            ref_actor_rolename = str(e).split('\'')[1]
                            for ref_actor in self.other_actors + self.ego_vehicles:
                                if ref_actor.rolename == ref_actor_rolename:
                                    if ref_actor.transform is not None:
                                        raise e
                                    break
                        else:
                            raise e
                    if actor.transform is None:
                        all_actor_transforms_set = False

    def _extract_vehicle_information(self, obj, rolename, vehicle, args):
        """
        Helper function to _set_actor_information for getting vehicle information from XML tree
        """
        color = None
        model = vehicle.attrib.get('name', "vehicle.*")
        category = vehicle.attrib.get('vehicleCategory', "car")
        ego_vehicle = False
        for prop in obj.iter("Property"):
            if prop.get('name', '') == 'type':
                ego_vehicle = prop.get('value') == 'ego_vehicle'
            if prop.get('name', '') == 'color':
                color = prop.get('value')

        speed = self._get_actor_speed(rolename)
        new_actor = ActorConfigurationData(
            model, None, rolename, speed, color=color, category=category, args=args)

        if ego_vehicle:
            self.ego_vehicles.append(new_actor)
        else:
            self.other_actors.append(new_actor)

    def _extract_pedestrian_information(self, obj, rolename, pedestrian, args):
        """
        Helper function to _set_actor_information for getting pedestrian information from XML tree
        """
        model = pedestrian.attrib.get('model', "walker.*")

        speed = self._get_actor_speed(rolename)
        new_actor = ActorConfigurationData(model, None, rolename, speed, category="pedestrian", args=args)

        self.other_actors.append(new_actor)

    def _extract_misc_information(self, obj, rolename, misc, args):
        """
        Helper function to _set_actor_information for getting vehicle information from XML tree
        """
        category = misc.attrib.get('miscObjectCategory')
        if category == "barrier":
            model = "static.prop.streetbarrier"
        elif category == "guardRail":
            model = "static.prop.chainbarrier"
        else:
            model = misc.attrib.get('name')
        new_actor = ActorConfigurationData(model, None, rolename, category="misc", args=args)

        self.other_actors.append(new_actor)

    def _get_actor_transform(self, actor_name):
        """
        Get the initial actor transform provided by the Init section

        Note: - The OpenScenario specification allows multiple definitions. We use the _first_ one
              - The OpenScenario specification allows different ways of specifying a position.
                We currently support the specification with absolute world coordinates and the relative positions
                RelativeWorld, RelativeObject and RelativeLane
              - When using relative positions the relevant reference point (e.g. transform of another actor)
                should be defined before!
        """

        actor_transform = carla.Transform()

        actor_found = False

        for private_action in self.init.iter("Private"):
            if private_action.attrib.get('entityRef', None) == actor_name:
                if actor_found:
                    # pylint: disable=line-too-long
                    self.logger.warning(
                        " Warning: The actor '%s' was already assigned an initial position. Overwriting pose!", actor_name)
                    # pylint: enable=line-too-long
                actor_found = True
                for position in private_action.iter('Position'):
                    transform = OpenScenarioParser.convert_position_to_transform(
                        position, actor_list=self.other_actors + self.ego_vehicles)
                    if transform:
                        actor_transform = transform

        if not actor_found:
            # pylint: disable=line-too-long
            self.logger.warning(
                " Warning: The actor '%s' was not assigned an initial position. Using (0,0,0)", actor_name)
            # pylint: enable=line-too-long

        return actor_transform

    def _get_actor_speed(self, actor_name):
        """
        Get the initial actor speed provided by the Init section
        """
        actor_speed = 0
        actor_found = False

        for private_action in self.init.iter("Private"):
            if private_action.attrib.get('entityRef', None) == actor_name:
                if actor_found:
                    # pylint: disable=line-too-long
                    self.logger.warning(
                        " Warning: The actor '%s' was already assigned an initial speed. Overwriting inital speed!", actor_name)
                    # pylint: enable=line-too-long
                actor_found = True

                for longitudinal_action in private_action.iter('LongitudinalAction'):
                    for speed in longitudinal_action.iter('SpeedAction'):
                        for target in speed.iter('SpeedActionTarget'):
                            for absolute in target.iter('AbsoluteTargetSpeed'):
                                speed = float(ParameterRef(absolute.attrib.get('value', 0)))
                                if speed >= 0:
                                    actor_speed = speed
                                else:
                                    raise AttributeError(
                                        "Warning: Speed value of actor {} must be positive. Speed set to 0.".format(actor_name))  # pylint: disable=line-too-long
        return actor_speed

    def _validate_result(self):
        """
        Check that the current scenario configuration is valid
        """
        if not self.name:
            raise AttributeError("No scenario name found")

        if not self.town:
            raise AttributeError("CARLA level not defined")

        if not self.ego_vehicles:
            self.logger.warning(" No ego vehicles defined in scenario")
