"""
Example CARLA Reinforcement Learning environment

This file contains an example RL environment compliant with the OpenAI-Gym API.
The reward is the sum of speeds over some predefined number of timesteps,
the available actions consist of controlling the throttle, brakes, steering
and reverse mode. The state is a tuple containing the image from an RGB camera
and various measurements such as forward speed, acceleration and collision information.
"""

from __future__ import print_function

import argparse
import numpy as np

import gym

from carla.client import VehicleControl, CarlaClient
from carla.settings import CarlaSettings
from carla import sensor
from carla import image_converter


def make_carla_settings(window_width, window_height):
    """Make a CarlaSettings object with the settings we need."""
    settings = CarlaSettings()
    settings.set(
        SynchronousMode=True,
        SendNonPlayerAgentsInfo=False,
        NumberOfVehicles=15,
        NumberOfPedestrians=30,
        WeatherId=1)
    settings.randomize_seeds()
    camera0 = sensor.Camera('CameraRGB')
    camera0.set_image_size(window_width, window_height)
    camera0.set_position(200, 0, 140)
    camera0.set_rotation(0.0, 0.0, 0.0)
    settings.add_sensor(camera0)
    return settings


class Carla(object):
    """
    Example CARLA RL environment
    """

    def __init__(self, hostname='localhost', port=2000, max_timesteps=100,
                 image_width=800, image_height=600):
        self.host = hostname
        self.port = port
        self._main_image = None
        self._client = None
        self.image_width = image_width
        self.image_height = image_height
        self.reward_range = (-np.inf, np.inf)
        self.metadata = {}
        self.max_timesteps = max_timesteps
        self.spec = gym.envs.registration.EnvSpec('Carla-v0')
        self._steps_made = 0
        self.number_of_player_starts = None

        self._create_action_space()
        self._create_observation_space()

    def _create_action_space(self):
        self.action_space = gym.spaces.Box(low=0, high=1, shape=(5,))

    def _create_observation_space(self):
        observation_subspaces = [
            gym.spaces.Box(0, 255, [self.image_height, self.image_width, 3]),
            gym.spaces.Box(-np.inf, np.inf, (7,))
        ]
        self.observation_space = gym.spaces.Tuple(observation_subspaces)

    @staticmethod
    def _compute_reward(state):
        reward = state[1][0]
        return reward

    def _is_done(self):
        return self._steps_made > self.max_timesteps

    @staticmethod
    def _action_to_control(action):
        control = VehicleControl()
        control.steer = (action[0] * 2) - 1
        control.throttle = action[1]
        control.brake = action[2]
        control.hand_brake = action[3] > 0.5
        control.reverse = action[4] > 0.5
        return control

    @staticmethod
    def _get_numeric_measurements(measurements):
        pm = measurements.player_measurements
        result = [
            pm.forward_speed,
            pm.acceleration,
            pm.intersection_offroad,
            pm.intersection_otherlane,
            pm.collision_vehicles,
            pm.collision_pedestrians,
            pm.collision_other]

        return np.array(result)

    def _get_current_state(self):
        measurements, sensor_data = self._client.read_data()

        numeric_measurements = self._get_numeric_measurements(measurements)

        main_image = sensor_data['CameraRGB']
        main_image = image_converter.to_rgb_array(main_image)
        state = (main_image, numeric_measurements)
        return state

    def step(self, action):
        """
        Make a step in the environment
        """
        self._steps_made += 1
        control = self._action_to_control(action)
        self._client.send_control(control)

        state = self._get_current_state()
        reward = self._compute_reward(state)
        done = self._is_done()

        info = {}
        return state, reward, done, info

    def reset(self):
        """
        Reset to the initial state
        """
        self._steps_made = 0

        if self._client is None:
            self._client = CarlaClient(self.host, self.port)
            self._client.connect()
            settings = make_carla_settings(self.image_width, self.image_height)
            scene = self._client.load_settings(settings)
            self.number_of_player_starts = len(scene.player_start_spots)

        player_start = np.random.randint(self.number_of_player_starts)
        print('Starting new episode...')
        self._client.start_episode(player_start)

        state = self._get_current_state()
        print('env reset done')
        return state


class VideoRecorder(object):
    """
    Helper class for writing videos of for the tests
    """
    def __init__(self, image_width=800, image_height=600):
        import cv2
        self.cv2 = cv2
        self.image_height = image_height
        self.image_width = image_width
        self.video = None

    def __enter__(self):
        video_name = 'video.avi'
        fourcc = self.cv2.VideoWriter_fourcc(*'XVID')
        self.video = self.cv2.VideoWriter(video_name, fourcc, fps=20.0,
                                          frameSize=(self.image_width, self.image_height))
        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self.video.release()

    @staticmethod
    def _rgb_to_bgr(image):
        return image[...,::-1]

    def record(self, state):
        """
        Push a frame to the video writer from the current environment state
        """
        (main_image, measurements) = state
        image = self._rgb_to_bgr(main_image)
        self.video.write(image)
        print(measurements)


def main():
    argparser = argparse.ArgumentParser(
        description='Example CARLA Reinforcement-Learning environment test.')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    args = argparser.parse_args()

    print(__doc__)
    env = Carla(args.host, args.port)
    env.reset()

    with VideoRecorder() as video_recorder:
        for _ in range(400):
            action = env.action_space.sample()
            state, _, _, _ = env.step(action)
            video_recorder.record(state)


if __name__ == '__main__':
    main()
