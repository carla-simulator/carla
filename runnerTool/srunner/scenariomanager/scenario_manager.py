#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides the ScenarioManager implementation.
It must not be modified and is for reference only!
"""

from __future__ import print_function
import sys
import time
import carla
import py_trees

from srunner.autoagents.agent_wrapper import AgentWrapper
from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.result_writer import ResultOutputProvider
from srunner.scenariomanager.timer import GameTime
from srunner.scenariomanager.watchdog import Watchdog

from srunner.scenariomanager.actorcontrols.visualizer import Visualizer


class ScenarioManager(object):

    """
    Basic scenario manager class. This class holds all functionality
    required to start, and analyze a scenario.

    The user must not modify this class.

    To use the ScenarioManager:
    1. Create an object via manager = ScenarioManager()
    2. Load a scenario via manager.load_scenario()
    3. Trigger the execution of the scenario manager.run_scenario()
       This function is designed to explicitly control start and end of
       the scenario execution
    4. Trigger a result evaluation with manager.analyze_scenario()
    5. If needed, cleanup with manager.stop_scenario()
    """

    def __init__(self, debug_mode=False, sync_mode=False, timeout=2.0, runnerTool_params = None):
        """
        Setups up the parameters, which will be filled at load_scenario()

        """
        self.scenario = None
        self.scenario_tree = None
        self.scenario_class = None
        self.ego_vehicles = None
        self.other_actors = None

        self._debug_mode = debug_mode
        self._agent = None
        self._sync_mode = sync_mode
        self._watchdog = None
        self._timeout = timeout

        self._running = False
        self._timestamp_last_run = 0.0
        self.scenario_duration_system = 0.0
        self.scenario_duration_game = 0.0
        self.start_system_time = None
        self.end_system_time = None

        self.runnerTool_cam = runnerTool_params["camera"]
        self.runnerTool_speed = runnerTool_params["speed"]

        self._visualizer = None


    def _reset(self):
        """
        Reset all parameters
        """
        self._running = False
        self._timestamp_last_run = 0.0
        self.scenario_duration_system = 0.0
        self.scenario_duration_game = 0.0
        self.start_system_time = None
        self.end_system_time = None
        GameTime.restart()

    def cleanup(self):
        """
        This function triggers a proper termination of a scenario
        """

        if self._watchdog is not None:
            self._watchdog.stop()
            self._watchdog = None

        if self.scenario is not None:
            self.scenario.terminate()

        if self._agent is not None:
            self._agent.cleanup()
            self._agent = None

        #runnerTool
        if self._visualizer is not None:
            self._visualizer.reset()

        CarlaDataProvider.cleanup()

    def load_scenario(self, scenario, agent=None):
        """
        Load a new scenario
        """
        self._reset()
        self._agent = AgentWrapper(agent) if agent else None
        if self._agent is not None:
            self._sync_mode = True
        self.scenario_class = scenario
        self.scenario = scenario.scenario
        self.scenario_tree = self.scenario.scenario_tree
        self.ego_vehicles = scenario.ego_vehicles
        self.other_actors = scenario.other_actors

        #runnerTool set speed and camera
        world = CarlaDataProvider.get_world()

        if self.runnerTool_speed != 100:
            self.set_speed(world)

        if self.runnerTool_cam is not None and self.runnerTool_speed == 100: 
            self._visualizer = Visualizer(self.ego_vehicles[0])

        self.get_camera(world)

        # To print the scenario tree uncomment the next line
        # py_trees.display.render_dot_tree(self.scenario_tree)

        if self._agent is not None:
            self._agent.setup_sensors(self.ego_vehicles[0], self._debug_mode)
    
    ####################runnerTool:
    def set_speed(self, world):
        ''' [RUNNERTOOL] changes scenario display speed'''
        print("Setting Scenario Speed to %.2fX" %(self.runnerTool_speed/100))
        settings = world.get_settings()
        if self.runnerTool_speed == 100:
            settings.fixed_delta_seconds = None
        else:
            settings.fixed_delta_seconds = (1.0 / (140/(self.runnerTool_speed/100)))
        world.apply_settings(settings)

    def get_camera(self,world):
        ''' [RUNNERTOOL] creates initial spectator position above ego vehicle'''
        spectator = world.get_spectator()
        target = world.get_actor(self.ego_vehicles[0].id).get_location()
        transform = carla.Transform(carla.Location(x = target.x, y = target.y, z = 60), carla.Rotation(pitch=270, yaw=0, roll=0))
        spectator.set_transform(transform) 
        time.sleep(1)

    def reset_camera(self, actor, spectator):
        ''' [RUNNERTOOL] fixes spectator to ego vehicle either in bird or ego perspective. 
            Removed with runnerTool v1.01 due to problems in comination with runnerTool_speed settings.
            Instead visualizer is used if runnerTool_speed is 100 i.e. not changed from default
        '''

        raise NotImplementedError

        target = actor.get_location()
        if self.runnerTool_cam == "bird":
            transform = carla.Transform(carla.Location(x = target.x, y = target.y, z = 60), carla.Rotation(pitch=270, yaw=0, roll=0))
        elif self.runnerTool_cam == "ego":
            transform = carla.Transform(carla.Location(x = target.x, y = target.y, z = 3), actor.get_transform().rotation)
        else:
            print("ERROR could not resolve camera setting")

        spectator.set_transform(transform) 
    ####################


    def run_scenario(self):
        """
        Trigger the start of the scenario and wait for it to finish/fail
        """
        print("ScenarioManager: Running scenario {}".format(self.scenario_tree.name))
        self.start_system_time = time.time()
        start_game_time = GameTime.get_time()

        self._watchdog = Watchdog(float(self._timeout))
        self._watchdog.start()
        self._running = True

        #k = 0
        while self._running:
            timestamp = None
            world = CarlaDataProvider.get_world()          
            if world:
                #if self._visualizer is None and self.runnerTool_cam is not None:
                    #self._visualizer.render()
                    #if k % 5000 == 0:
                        #self.reset_camera(world.get_actor(self.ego_vehicles[0].id),world.get_spectator())

                snapshot = world.get_snapshot()
                if snapshot:
                    timestamp = snapshot.timestamp
            if timestamp:
                self._tick_scenario(timestamp)
            #k+=1

        self.cleanup()

        self.end_system_time = time.time()
        end_game_time = GameTime.get_time()

        self.scenario_duration_system = self.end_system_time - \
            self.start_system_time
        self.scenario_duration_game = end_game_time - start_game_time

        if self.scenario_tree.status == py_trees.common.Status.FAILURE:
            print("ScenarioManager: Terminated due to failure")

    def _tick_scenario(self, timestamp):
        """
        Run next tick of scenario and the agent.
        If running synchornously, it also handles the ticking of the world.
        """

        if self._timestamp_last_run < timestamp.elapsed_seconds and self._running:
            self._timestamp_last_run = timestamp.elapsed_seconds

            self._watchdog.update()

            if self._debug_mode:
                print("\n--------- Tick ---------\n")

            # Update game time and actor information
            GameTime.on_carla_tick(timestamp)
            CarlaDataProvider.on_carla_tick()

            ##runnerTool
            if self._visualizer is not None:
                self._visualizer.render()
            ##
            if self._agent is not None:
                ego_action = self._agent()  # pylint: disable=not-callable

            if self._agent is not None:
                self.ego_vehicles[0].apply_control(ego_action)

            # Tick scenario
            self.scenario_tree.tick_once()

            if self._debug_mode:
                print("\n")
                py_trees.display.print_ascii_tree(self.scenario_tree, show_status=True)
                sys.stdout.flush()

            if self.scenario_tree.status != py_trees.common.Status.RUNNING:
                self._running = False

        if self._sync_mode and self._running and self._watchdog.get_status():
            CarlaDataProvider.get_world().tick()

    def get_running_status(self):
        """
        returns:
           bool:  False if watchdog exception occured, True otherwise
        """
        return self._watchdog.get_status()

    def stop_scenario(self):
        """
        This function is used by the overall signal handler to terminate the scenario execution
        """
        self._running = False

    def analyze_scenario(self, stdout, filename, junit, json):
        """
        This function is intended to be called from outside and provide
        the final statistics about the scenario (human-readable, in form of a junit
        report, etc.)
        """

        failure = False
        timeout = False
        result = "SUCCESS"

        if self.scenario.test_criteria is None:
            print("Nothing to analyze, this scenario has no criteria")
            return True

        for criterion in self.scenario.get_criteria():
            if (not criterion.optional and
                    criterion.test_status != "SUCCESS" and
                    criterion.test_status != "ACCEPTABLE"):
                failure = True
                result = "FAILURE"
            elif criterion.test_status == "ACCEPTABLE":
                result = "ACCEPTABLE"

        if self.scenario.timeout_node.timeout and not failure:
            timeout = True
            result = "TIMEOUT"

        output = ResultOutputProvider(self, result, stdout, filename, junit, json)
        output.write()

        return failure or timeout
