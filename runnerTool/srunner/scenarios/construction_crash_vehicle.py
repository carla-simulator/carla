#!/usr/bin/env python
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Object crash without prior vehicle action scenario:
The scenario realizes the user controlled ego vehicle
moving along the road and encountering a construction setup.
"""

from __future__ import print_function

import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import ActorDestroy
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import DriveDistance
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import Idle
from srunner.tools.scenario_helper import get_location_in_distance_from_wp
from srunner.scenarios.object_crash_vehicle import StationaryObjectCrossing


class ConstructionSetupCrossing(StationaryObjectCrossing):

    """
    This class holds everything required for a construction scenario
    The ego vehicle is passing through a road and encounters
    a stationary rectangular construction cones setup and traffic warning.

    This is a single ego vehicle scenario
    """

    def __init__(
            self,
            world,
            ego_vehicles,
            config,
            randomize=False,
            debug_mode=False,
            criteria_enable=True,
            timeout=60):
        """
        Setup all relevant parameters and create scenario
        """
        super(
            ConstructionSetupCrossing,
            self).__init__(
            world,
            ego_vehicles=ego_vehicles,
            config=config,
            randomize=randomize,
            debug_mode=debug_mode,
            criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        _start_distance = 40
        lane_width = self._reference_waypoint.lane_width
        location, _ = get_location_in_distance_from_wp(
            self._reference_waypoint, _start_distance)
        waypoint = self._wmap.get_waypoint(location)
        self._create_construction_setup(waypoint.transform, lane_width)

    def create_cones_side(
            self,
            start_transform,
            forward_vector,
            z_inc=0,
            cone_length=0,
            cone_offset=0):
        """
        Creates One Side of the Cones
        """
        _dist = 0
        while _dist < (cone_length * cone_offset):
            # Move forward
            _dist += cone_offset
            forward_dist = carla.Vector3D(0, 0, 0) + forward_vector * _dist

            location = start_transform.location + forward_dist
            location.z += z_inc
            transform = carla.Transform(location, start_transform.rotation)

            cone = CarlaDataProvider.request_new_actor(
                'static.prop.constructioncone', transform)
            cone.set_simulate_physics(True)
            self.other_actors.append(cone)

    def _create_construction_setup(self, start_transform, lane_width):
        """
        Create Construction Setup
        """

        _initial_offset = {'cones': {'yaw': 180, 'k': lane_width / 2.0},
                           'warning_sign': {'yaw': 180, 'k': 5, 'z': 0},
                           'debris': {'yaw': 0, 'k': 2, 'z': 1}}
        _prop_names = {'warning_sign': 'static.prop.trafficwarning',
                       'debris': 'static.prop.dirtdebris02'}

        _perp_angle = 90
        _setup = {'lengths': [0, 6, 3], 'offsets': [0, 2, 1]}
        _z_increment = 0.1

        ############################# Traffic Warning and Debris ##############
        for key, value in _initial_offset.items():
            if key == 'cones':
                continue
            transform = carla.Transform(
                start_transform.location,
                start_transform.rotation)
            transform.rotation.yaw += value['yaw']
            transform.location += value['k'] * \
                transform.rotation.get_forward_vector()
            transform.location.z += value['z']
            transform.rotation.yaw += _perp_angle
            static = CarlaDataProvider.request_new_actor(
                _prop_names[key], transform)
            static.set_simulate_physics(True)
            self.other_actors.append(static)

        ############################# Cones ###################################
        side_transform = carla.Transform(
            start_transform.location,
            start_transform.rotation)
        side_transform.rotation.yaw += _perp_angle
        side_transform.location -= _initial_offset['cones']['k'] * \
            side_transform.rotation.get_forward_vector()
        side_transform.rotation.yaw += _initial_offset['cones']['yaw']

        for i in range(len(_setup['lengths'])):
            self.create_cones_side(
                side_transform,
                forward_vector=side_transform.rotation.get_forward_vector(),
                z_inc=_z_increment,
                cone_length=_setup['lengths'][i],
                cone_offset=_setup['offsets'][i])
            side_transform.location += side_transform.get_forward_vector() * \
                _setup['lengths'][i] * _setup['offsets'][i]
            side_transform.rotation.yaw += _perp_angle

    def _create_behavior(self):
        """
        Only behavior here is to wait
        """
        # leaf nodes
        actor_stand = Idle(15)

        end_condition = DriveDistance(
            self.ego_vehicles[0],
            self._ego_vehicle_distance_driven)

        # non leaf nodes
        scenario_sequence = py_trees.composites.Sequence()

        # building tree
        scenario_sequence.add_child(actor_stand)

        for i, _ in enumerate(self.other_actors):
            scenario_sequence.add_child(ActorDestroy(self.other_actors[i]))
        scenario_sequence.add_child(end_condition)

        return scenario_sequence
