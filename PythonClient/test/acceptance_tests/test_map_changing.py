"""
For now we use the unit tests for this part
Basically the difference between this folder and the other, the unit tests folder
is that the acceptance tests is carla based

TODO: Change the name

"""

import unittest
import random
import time

from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings

from carla.util import print_over_same_line

from carla.tcp import TCPConnectionError

from carla.driving_benchmark.experiment_suites.basic_experiment_suite import BasicExperimentSuite

from carla.driving_benchmark.experiment_suites.corl_2017 import CoRL2017


def print_measurements(measurements):
    number_of_agents = len(measurements.non_player_agents)
    player_measurements = measurements.player_measurements
    message = 'Vehicle at ({pos_x:.1f}, {pos_y:.1f}), '
    message += '{speed:.0f} km/h, '
    message += 'Collision: {{vehicles={col_cars:.0f}, pedestrians={col_ped:.0f}, other={col_other:.0f}}}, '
    message += '{other_lane:.0f}% other lane, {offroad:.0f}% off-road, '
    message += '({agents_num:d} non-player agents in the scene)'
    message = message.format(
        pos_x=player_measurements.transform.location.x,
        pos_y=player_measurements.transform.location.y,
        speed=player_measurements.forward_speed * 3.6, # m/s -> km/h
        col_cars=player_measurements.collision_vehicles,
        col_ped=player_measurements.collision_pedestrians,
        col_other=player_measurements.collision_other,
        other_lane=100 * player_measurements.intersection_otherlane,
        offroad=100 * player_measurements.intersection_offroad,
        agents_num=number_of_agents)
    print_over_same_line(message)



class testMapChanging(unittest.TestCase):




    def test_properties(self):
        # Here we will run 3 episodes with 300 frames each.


        with make_carla_client('127.0.0.1', 2000) as client:
            print('CarlaClient connected')

            # Run Eight episodes
            for episode in range(0, 8):
                # Start a new episode.


                # We take different map on each episode
                if episode % 2 == 0:
                    map_name = "Town01"
                else:
                    map_name = "Town02"

                settings = CarlaSettings()
                settings.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    MapName=map_name,
                    NumberOfVehicles=20,
                    NumberOfPedestrians=40,
                    WeatherId=random.choice([1, 3, 7, 8, 14]),
                    QualityLevel='Epic')
                settings.randomize_seeds()


                scene = client.load_settings(settings)



                print('Starting new episode at %r...' % scene.map_name)
                client.start_episode(0)
                time.sleep(1)
                # Check if the map loaded in the settings is the map received as loaded.
                self.assertEqual(scene.map_name, map_name)