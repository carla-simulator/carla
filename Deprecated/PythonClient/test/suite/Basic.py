# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import logging
import random

import suite

import carla

from carla.client import CarlaClient
from carla.sensor import Camera, Image
from carla.sensor import Lidar, LidarMeasurement
from carla.settings import CarlaSettings
from carla.util import make_connection


class _BasicTestBase(suite.CarlaServerTest):
    def run_carla_client(self, carla_settings, number_of_episodes, number_of_frames, use_autopilot_control=None):
        with make_connection(CarlaClient, self.args.host, self.args.port, timeout=15) as client:
            logging.info('CarlaClient connected, running %d episodes', number_of_episodes)
            for _ in range(0, number_of_episodes):
                carla_settings.randomize_seeds()
                carla_settings.randomize_weather()
                logging.debug('sending CarlaSettings:\n%s', carla_settings)
                logging.info('new episode requested')
                scene = client.load_settings(carla_settings)
                number_of_player_starts = len(scene.player_start_spots)
                player_start = random.randint(0, max(0, number_of_player_starts - 1))
                logging.info(
                    'start episode at %d/%d player start (%d frames)',
                    player_start,
                    number_of_player_starts,
                    number_of_frames)
                client.start_episode(player_start)
                if use_autopilot_control is None:
                    use_autopilot_control = (random.random() < 0.5)
                reverse = (random.random() < 0.2)
                for _ in range(0, number_of_frames):
                    logging.debug('reading measurements...')
                    measurements, sensor_data = client.read_data()
                    images = [x for x in sensor_data.values() if isinstance(x, Image)]
                    number_of_agents = len(measurements.non_player_agents)
                    logging.debug('received data of %d agents', number_of_agents)
                    logging.debug('received %d images', len(images))
                    if len(sensor_data) != len(carla_settings._sensors):
                        raise RuntimeError('received %d, expected %d' % (len(sensor_data), len(carla_settings._sensors)))
                    logging.debug('sending control...')
                    control = measurements.player_measurements.autopilot_control
                    if not use_autopilot_control:
                        control.steer = random.uniform(-1.0, 1.0)
                        control.throttle = 0.3
                        control.hand_brake = False
                        control.reverse = reverse
                    client.send_control(
                        steer=control.steer,
                        throttle=control.throttle,
                        brake=control.brake,
                        hand_brake=control.hand_brake,
                        reverse=control.reverse)


class UseCase(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_sensor(Camera('DefaultCamera'))
        self.run_carla_client(settings, 5, 200)


class NoCamera(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        self.run_carla_client(settings, 3, 200)


class TwoCameras(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_sensor(Camera('DefaultCamera'))
        camera2 = Camera('Camera2')
        camera2.set(PostProcessing='Depth', FOV=120)
        camera2.set_image_size(1924, 1028)
        settings.add_sensor(camera2)
        self.run_carla_client(settings, 3, 100)


class SynchronousMode(_BasicTestBase):
    def run(self):
        settings = CarlaSettings(SynchronousMode=True)
        settings.add_sensor(Camera('DefaultCamera'))
        self.run_carla_client(settings, 3, 200)


class GetAgentsInfo(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.set(
            SynchronousMode=True,
            SendNonPlayerAgentsInfo=True,
            NumberOfVehicles=60,
            NumberOfPedestrians=90)
        settings.add_sensor(Camera('DefaultCamera'))
        self.run_carla_client(settings, 3, 100)


class LongEpisode(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_sensor(Camera('DefaultCamera'))
        self.run_carla_client(settings, 1, 2000, use_autopilot_control=True)


class LidarTest(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_sensor(Lidar('DefaultLidar'))
        self.run_carla_client(settings, 3, 100)


class SpeedLowQuality(_BasicTestBase):
    def run(self):
        settings = CarlaSettings(QualityLevel='Low')
        settings.add_sensor(Lidar('DefaultLidar'))
        settings.add_sensor(Camera('DefaultCamera'))
        settings.add_sensor(Camera('DefaultDepth', PostProcessing='Depth'))
        settings.add_sensor(Camera('DefaultSemSeg', PostProcessing='SemanticSegmentation'))
        self.run_carla_client(settings, 3, 200)
