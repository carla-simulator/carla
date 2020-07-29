import time
import os
import cv2
import skvideo.io
import numpy as np

import rewards

import experiment_suites
import carla.driving_benchmark.experiment_suites as experiment_suites_benchmark
from carla.client import VehicleControl
from carla.planner.planner import Planner
from carla.settings import CarlaSettings
from carla.client import CarlaClient
from carla.tcp import TCPConnectionError
from observation_utils import CameraException
import gym

from carla_logger import get_carla_logger
# TODO: Remove this before open-sourcing environment

class CarlaEnv(object):
    '''
        An OpenAI Gym Environment for CARLA.
    '''

    def __init__(self,
                 obs_converter,
                 action_converter,
                 env_id,
                 random_seed=0,
                 exp_suite_name='TrainingSuite',
                 reward_class_name='RewardCarla',
                 host='127.0.0.1',
                 port=2000,
                 city_name='Town01',
                 subset=None,
                 video_every=100,
                 video_dir='./video/',
                 distance_for_success=2.0,
                 benchmark=False):

        self.logger = get_carla_logger()
        self.logger.info('Environment {} running in port {}'.format(env_id, port))
        self.host, self.port = host, port
        self.id = env_id
        self._obs_converter = obs_converter
        self.observation_space = obs_converter.get_observation_space()
        self._action_converter = action_converter
        self.action_space = self._action_converter.get_action_space()
        if benchmark:
            self._experiment_suite = getattr(experiment_suites_benchmark, exp_suite_name)(city_name)
        else:
            self._experiment_suite = getattr(experiment_suites, exp_suite_name)(city_name, subset)
        self._reward = getattr(rewards, reward_class_name)()
        self._experiments = self._experiment_suite.get_experiments()
        self.subset = subset
        self._make_carla_client(host, port)
        self._distance_for_success = distance_for_success
        self._planner = Planner(city_name)
        self.done = False
        self.last_obs = None
        self.last_distance_to_goal = None
        self.last_direction = None
        self.last_measurements = None
        np.random.seed(random_seed)
        self.video_every = video_every
        self.video_dir = video_dir
        self.video_writer = None
        self._success = False
        self._failure_timeout = False
        self._failure_collision = False
        self.benchmark = benchmark
        self.benchmark_index = [0, 0, 0]
        try:
            if not os.path.isdir(self.video_dir):
                os.makedirs(self.video_dir)
        except OSError:
            pass
        self.steps = 0
        self.num_episodes = 0

    def get_control(action):
        return self._action_converter.action_to_control(action, self.last_measurements)

    def step(self, measurements, sensor_data):

        if self.done:
            raise ValueError('self.done should always be False when calling step')

        while True:

            try:
                # Send control
                # control = self._action_converter.action_to_control(action, self.last_measurements)
                # self._client.send_control(control)

                # Gather the observations (including measurements, sensor and directions)
                # measurements, sensor_data = self._client.read_data()
                self.last_measurements = measurements
                current_timestamp = measurements.game_timestamp
                distance_to_goal = self._get_distance_to_goal(measurements, self._target)
                self.last_distance_to_goal = distance_to_goal
                directions = self._get_directions(measurements.player_measurements.transform,
                                                self._target)
                self.last_direction = directions
                obs = self._obs_converter.convert(measurements, sensor_data, directions, self._target, self.id)

                if self.video_writer is not None and self.steps % 2 == 0:
                    self._raster_frame(sensor_data, measurements, directions, obs)

                self.last_obs = obs

            except CameraException:
                self.logger.debug('Camera Exception in step()')
                obs = self.last_obs
                distance_to_goal = self.last_distance_to_goal
                current_timestamp = self.last_measurements.game_timestamp

            except TCPConnectionError as e:
                self.logger.debug('TCPConnectionError inside step(): {}'.format(e))
                self.done = True
                return self.last_obs, 0.0, True, {'carla-reward': 0.0}

            break

        # Check if terminal state
        timeout = (current_timestamp - self._initial_timestamp) > (self._time_out * 1000)
        collision, _ = self._is_collision(measurements)
        success = distance_to_goal < self._distance_for_success
        if timeout:
            self.logger.debug('Timeout')
            self._failure_timeout = True
        if collision:
            self.logger.debug('Collision')
            self._failure_collision = True
        if success:
            self.logger.debug('Success')
        self.done = timeout or collision or success


        # Get the reward
        env_state = {'timeout': timeout, 'collision': collision, 'success': success}
        reward = self._reward.get_reward(measurements, self._target, self.last_direction, control, env_state)

        # Additional information
        info = {'carla-reward': reward}

        self.steps += 1

        return obs, reward, self.done, info


    def reset(self):

        # Loop forever due to TCPConnectionErrors
        while True:
            try:
                self._reward.reset_reward()
                self.done = False
                if self.video_writer is not None:
                    try:
                        self.video_writer.close()
                    except Exception as e:
                        self.logger.debug('Error when closing video writer in reset')
                        self.logger.error(e)
                    self.video_writer = None
                if self.benchmark:
                    end_indicator = self._new_episode_benchmark()
                    if end_indicator is False:
                        return False
                else:
                    self._new_episode()
                # Hack: Try sleeping so that the server is ready. Reduces the number of TCPErrors
                time.sleep(4)
                # measurements, sensor_data = self._client.read_data()
                self._client.send_control(VehicleControl())
                measurements, sensor_data = self._client.read_data()
                self._initial_timestamp = measurements.game_timestamp
                self.last_measurements = measurements
                self.last_distance_to_goal = self._get_distance_to_goal(measurements, self._target)
                directions = self._get_directions(measurements.player_measurements.transform, self._target)
                self.last_direction = directions
                obs = self._obs_converter.convert(measurements, sensor_data, directions, self._target, self.id)
                self.last_obs = obs
                self.done = False
                self._success = False
                self._failure_timeout = False
                self._failure_collision = False
                return obs

            except CameraException:
                self.logger.debug('Camera Exception in reset()')
                continue

            except TCPConnectionError as e:
                self.logger.debug('TCPConnectionError in reset()')
                self.logger.error(e)
                # Disconnect and reconnect
                self.disconnect()
                time.sleep(5)
                self._make_carla_client(self.host, self.port)


    def disconnect(self):

        if self.video_writer is not None:
            try:
                self.video_writer.close()
            except Exception as e:
                self.logger.debug('Error when closing video writer in disconnect')
                self.logger.error(e)
            self.video_writer = None

        self._client.disconnect()


    def _raster_frame(self, sensor_data, measurements, directions, obs):

        frame = sensor_data['CameraRGB'].data.copy()
        cv2.putText(frame, text='Episode number: {:,}'.format(self.num_episodes-1),
                org=(50, 50), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                color=[0, 0, 0], thickness=2)
        cv2.putText(frame, text='Environment steps: {:,}'.format(self.steps),
                org=(50, 80), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                color=[0, 0, 0], thickness=2)

        REACH_GOAL = 0.0
        GO_STRAIGHT = 5.0
        TURN_RIGHT = 4.0
        TURN_LEFT = 3.0
        LANE_FOLLOW = 2.0
        if np.isclose(directions, REACH_GOAL):
            dir_str = 'REACH GOAL'
        elif np.isclose(directions, GO_STRAIGHT):
            dir_str = 'GO STRAIGHT'
        elif np.isclose(directions, TURN_RIGHT):
            dir_str = 'TURN RIGHT'
        elif np.isclose(directions, TURN_LEFT):
            dir_str = 'TURN LEFT'
        elif np.isclose(directions, LANE_FOLLOW):
            dir_str = 'LANE FOLLOW'
        else:
            raise ValueError(directions)
        cv2.putText(frame, text='Direction: {}'.format(dir_str),
                    org=(50, 110), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                    color=[0, 0, 0], thickness=2)
        cv2.putText(frame, text='Speed: {:.02f}'.format( measurements.player_measurements.forward_speed * 3.6),
                    org=(50, 140), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                    color=[0, 0, 0], thickness=2)
        cv2.putText(frame, text='rel_x: {:.02f}, rel_y: {:.02f}'.format(obs['v'][-2].item(), obs['v'][-1].item()),
                    org=(50, 170), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                    color=[0, 0, 0], thickness=2)
        self.video_writer.writeFrame(frame)


    def _get_distance_to_goal(self, measurements, target):

        current_x = measurements.player_measurements.transform.location.x
        current_y = measurements.player_measurements.transform.location.y
        distance_to_goal = np.linalg.norm(np.array([current_x, current_y]) - \
                            np.array([target.location.x, target.location.y]))
        return distance_to_goal


    def _new_episode(self):
        experiment_idx = np.random.randint(0, len(self._experiments))
        experiment = self._experiments[experiment_idx]
        exp_settings = experiment.conditions
        exp_settings.set(QualityLevel='Low')
        positions = self._client.load_settings(exp_settings).player_start_spots
        idx_pose = np.random.randint(0, len(experiment.poses))
        pose = experiment.poses[idx_pose]
        self.logger.info('Env {} gets experiment {} with pose {}'.format(self.id, experiment_idx, idx_pose))
        start_index = pose[0]
        end_index = pose[1]
        self._client.start_episode(start_index)
        self._time_out = self._experiment_suite.calculate_time_out(
                        self._get_shortest_path(positions[start_index], positions[end_index]))
        self._target = positions[end_index]
        self._episode_name = str(experiment.Conditions.WeatherId) + '_' \
                            + str(experiment.task) + '_' + str(start_index) \
                            + '_' + str(end_index)


        if ((self.num_episodes % self.video_every) == 0) and (self.id == 0):
            video_path = os.path.join(self.video_dir, '{:08d}_'.format(self.num_episodes) + self._episode_name + '.mp4')
            self.logger.info('Writing video at {}'.format(video_path))
            self.video_writer = skvideo.io.FFmpegWriter(video_path, inputdict={'-r': '30'}, outputdict={'-r': '30'})
        else:
            self.video_writer = None

        self.num_episodes += 1

    def _new_episode_benchmark(self):
        experiment_idx_past = self.benchmark_index[0]
        pose_idx_past = self.benchmark_index[1]
        repetition_idx_past = self.benchmark_index[2]

        experiment_past = self._experiments[experiment_idx_past]
        poses_past = experiment_past.poses[0:]
        repetition_past = experiment_past.repetitions

        if repetition_idx_past == repetition_past:
            if pose_idx_past == len(poses_past) - 1:
                if experiment_idx_past == len(self._experiments) - 1:
                    return False
                else:
                    experiment = self._experiments[experiment_idx_past + 1]
                    pose = experiment.poses[0:][0]
                    self.benchmark_index = [experiment_idx_past + 1, 0, 1]
            else:
                experiment = experiment_past
                pose = poses_past[pose_idx_past + 1]
                self.benchmark_index = [experiment_idx_past, pose_idx_past + 1, 1]
        else:
            experiment = experiment_past
            pose = poses_past[pose_idx_past]
            self.benchmark_index = [experiment_idx_past, pose_idx_past, repetition_idx_past + 1]
        exp_settings = experiment.Conditions
        exp_settings.set(QualityLevel='Low')
        positions = self._client.load_settings(exp_settings).player_start_spots
        start_index = pose[0]
        end_index = pose[1]
        self._client.start_episode(start_index)
        self._time_out = self._experiment_suite.calculate_time_out(
                        self._get_shortest_path(positions[start_index], positions[end_index]))
        self._target = positions[end_index]
        self._episode_name = str(experiment.Conditions.WeatherId) + '_' \
                            + str(experiment.task) + '_' + str(start_index) \
                            + '_' + str(end_index)
        if ((self.num_episodes % self.video_every) == 0) and (self.id == 0):
            video_path = os.path.join(self.video_dir, '{:08d}_'.format(self.num_episodes) + self._episode_name + '.mp4')
            self.logger.info('Writing video at {}'.format(video_path))
            self.video_writer = skvideo.io.FFmpegWriter(video_path, inputdict={'-r': '30'}, outputdict={'-r': '30'})
        else:
            self.video_writer = None

        self.num_episodes += 1


    def _get_directions(self, current_point, end_point):

        directions = self._planner.get_next_command(
            (current_point.location.x,
             current_point.location.y, 0.22),
            (current_point.orientation.x,
             current_point.orientation.y,
             current_point.orientation.z),
            (end_point.location.x, end_point.location.y, 0.22),
            (end_point.orientation.x, end_point.orientation.y, end_point.orientation.z))
        return directions


    def _get_shortest_path(self, start_point, end_point):

        return self._planner.get_shortest_path_distance(
            [   start_point.location.x, start_point.location.y, 0.22], [
                start_point.orientation.x, start_point.orientation.y, 0.22], [
                end_point.location.x, end_point.location.y, end_point.location.z], [
                end_point.orientation.x, end_point.orientation.y, end_point.orientation.z])


    @staticmethod
    def _is_collision(measurements):

        c = 0
        c += measurements.player_measurements.collision_vehicles
        c += measurements.player_measurements.collision_pedestrians
        c += measurements.player_measurements.collision_other

        sidewalk_intersection = measurements.player_measurements.intersection_offroad

        otherlane_intersection = measurements.player_measurements.intersection_otherlane

        return (c > 1e-9) or (sidewalk_intersection > 0.01) or (otherlane_intersection > 0.9), c


    def _make_carla_client(self, host, port):

        while True:
            try:
                self.logger.info("Trying to make client on port {}".format(port))
                self._client = CarlaClient(host, port, timeout=100)
                self._client.connect()
                self._client.load_settings(CarlaSettings(QualityLevel='Low'))
                self._client.start_episode(0)
                self.logger.info("Successfully made client on port {}".format(port))
                break
            except TCPConnectionError as error:
                self.logger.debug('Got TCPConnectionError..sleeping for 1')
                self.logger.error(error)
                time.sleep(1)
