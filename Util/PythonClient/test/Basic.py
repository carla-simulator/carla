import test

import logging
import random

import carla

from carla.client import CarlaClient
from carla.settings import CarlaSettings, Camera
from carla.util import make_connection


class _BasicTestBase(test.CarlaServerTest):
    def run_carla_client(self, carla_settings, number_of_episodes, number_of_frames, control=None):
        with make_connection(CarlaClient, self.args.host, self.args.port, timeout=15) as client:
            logging.info('CarlaClient connected, running %d episodes', number_of_episodes)
            for _ in range(0, number_of_episodes):
                carla_settings.randomize_seeds()
                carla_settings.randomize_weather()
                logging.debug('sending CarlaSettings:\n%s', carla_settings)
                logging.info('new episode requested')
                scene = client.request_new_episode(carla_settings)
                number_of_player_starts = len(scene.player_start_spots)
                player_start = random.randint(0, max(0, number_of_player_starts - 1))
                logging.info(
                    'start episode at %d/%d player start (%d frames)',
                    player_start,
                    number_of_player_starts,
                    number_of_frames)
                client.start_episode(player_start)
                autopilot = (random.random() < 0.5)
                reverse = (random.random() < 0.2)
                for _ in range(0, number_of_frames):
                    logging.debug('reading measurements...')
                    measurements, images = client.read_measurements()
                    number_of_agents = len(measurements.non_player_agents)
                    expected_number_of_agents = carla_settings.get_number_of_agents()
                    logging.debug('received data of %d agents', number_of_agents)
                    logging.debug('received %d images', len(images))
                    if len(images) != len(carla_settings._cameras):
                        raise RuntimeError('received %d images, expected %d' % (len(images), len(carla_settings._cameras)))
                    logging.debug('sending control...')
                    if control is not None:
                        client.send_control(**control)
                    else:
                        client.send_control(
                            steer=random.uniform(-1.0, 1.0),
                            throttle=0.3,
                            reverse=reverse,
                            autopilot=autopilot)


class UseCase(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_camera(Camera('DefaultCamera'))
        self.run_carla_client(settings, 5, 200)


class NoCamera(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        self.run_carla_client(settings, 3, 200)


class TwoCameras(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_camera(Camera('DefaultCamera'))
        camera2 = Camera('Camera2')
        camera2.set(PostProcessing='Depth', CameraFOV=120)
        camera2.set_image_size(1924, 1028)
        settings.add_camera(camera2)
        self.run_carla_client(settings, 3, 100)


class SynchronousMode(_BasicTestBase):
    def run(self):
        settings = CarlaSettings(SynchronousMode=True)
        settings.add_camera(Camera('DefaultCamera'))
        self.run_carla_client(settings, 3, 200)


class GetAgentsInfo(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.set(
            SynchronousMode=True,
            SendNonPlayerAgentsInfo=True,
            NumberOfVehicles=60,
            NumberOfPedestrians=90)
        settings.add_camera(Camera('DefaultCamera'))
        self.run_carla_client(settings, 3, 100)


class LongEpisode(_BasicTestBase):
    def run(self):
        settings = CarlaSettings()
        settings.add_camera(Camera('DefaultCamera'))
        self.run_carla_client(settings, 1, 2000, {'autopilot': True})
