import cv2
import skvideo.io
import numpy as np
import gym

# ==============================================================================
# -- CarlaReward ---------------------------------------------------------------------
# ==============================================================================

class CarlaReward():
    '''
        Reward used for training in the original CARLA paper (http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf)

        From the paper:
        "The reward is a weighted sum of five terms: distance traveled towards the goal d in km, speed v in
        km/h, collision damage c, intersection with the sidewalk s (between 0 and 1), and intersection with
        the opposite lane o (between 0 and 1)"

    '''

    def __init__(self):
        # Reward is calculated based on differences between timesteps, so need to
        # save the current state for the next reward.
        self.state = None


    def get_reward(self, measurements, target, action, env_state):

        # Distance towards goal (in km)
        d_x = measurements.player_measurements.transform.location.x
        d_y = measurements.player_measurements.transform.location.y
        d_z = measurements.player_measurements.transform.location.z
        player_location = np.array([d_x, d_y, d_z])
        goal_location = np.array([target.location.x,
                                  target.location.y,
                                  target.location.z])
        d = np.linalg.norm(player_location - goal_location) / 1000

        # Speed
        v = measurements.player_measurements.forward_speed * 3.6
        # Collision damage
        c_v = measurements.player_measurements.collision_vehicles
        c_p = measurements.player_measurements.collision_pedestrians
        c_o = measurements.player_measurements.collision_other
        c = c_v + c_p + c_o

        # Intersection with sidewalk
        s = measurements.player_measurements.intersection_offroad

        # Intersection with opposite lane
        o = measurements.player_measurements.intersection_otherlane

        # Compute reward
        r = 0
        if self.state is not None:
            r += 1000 * (self.state['d'] - d)
            r += 0.05 * (v - self.state['v'])
            r -= 0.00002 * (c - self.state['c'])
            r -= -0.1 * float(s > 0.001)
            r -= 2 * (o - self.state['o'])

        # Update state
        new_state = {'d': d, 'v': v, 'c': c, 's': s, 'o': o,
                     'd_x': d_x, 'd_y': d_y, 'd_z': d_z,
                     'c_v': c_v, 'c_p': c_p, 'c_o': c_o}
        self.state = new_state

        return r


    def reset_reward(self):

        self.state = None

# ==============================================================================
# -- CarlaEnv ---------------------------------------------------------------------
# ==============================================================================

class CarlaEnv(object):
    '''
        An OpenAI Gym Environment for CARLA.
    '''
    def __init__(self, 
                world,
                env_id,
                time_out,
                reward_calculator,
                rel_coord_system=False):
        self.world = world
        self.last_measurements = None
        self.last_distance_to_goal = None
        self.id = env_id
        self.done = False
        self.steps = 0
        self.num_episodes = 0
        self.rel_coord_system = rel_coord_system
        self._time_out = time_out # TODO: this was retrieved from the APIs
        self._failure_timeout = False
        self.reward_calculator = reward_calculator
        self._failure_collision = False

    def _get_distance_to_goal(self, measurements, target):

        current_x = measurements.player_measurements.transform.location.x
        current_y = measurements.player_measurements.transform.location.y
        distance_to_goal = np.linalg.norm(np.array([current_x, current_y]) - \
                            np.array([target.location.x, target.location.y]))
        return distance_to_goal

    def _obs_convert(self, measurements, sensor_data, target, env_id):

        player_measurements = measurements.player_measurements

        if self.rel_coord_system:
            target_rel_x, target_rel_y = self.get_relative_location_target(
                                        player_measurements.transform.location.x,
                                        player_measurements.transform.location.y,
                                        player_measurements.transform.rotation.yaw,
                                        target.location.x,
                                        target.location.y,)
            target_rel_norm = np.linalg.norm(np.array([target_rel_x, target_rel_y]))
            target_rel_x_unit = target_rel_x / target_rel_norm
            target_rel_y_unit = target_rel_y / target_rel_norm

            v = np.array([
                player_measurements.forward_speed,
                player_measurements.acceleration.x,
                player_measurements.acceleration.y,
                # NOTE: omit directions
                #float(np.isclose(directions, self.REACH_GOAL)),
                #float(np.isclose(directions, self.GO_STRAIGHT)),
                #float(np.isclose(directions, self.TURN_RIGHT)),
                #float(np.isclose(directions, self.TURN_LEFT)),
                #float(np.isclose(directions, self.LANE_FOLLOW)),
                target_rel_x,
                target_rel_y,
                target_rel_x_unit,
                target_rel_y_unit
            ])

            world_pos = np.array([
                player_measurements.transform.location.x,
                player_measurements.transform.location.y,
                player_measurements.transform.rotation.yaw
            ])

        else:
            v = np.array([
                player_measurements.transform.location.x,
                player_measurements.transform.location.y,
                player_measurements.transform.rotation.yaw,
                player_measurements.forward_speed,
                player_measurements.acceleration.x,
                player_measurements.acceleration.y,
                # NOTE: omit directions
                # float(np.isclose(directions, self.REACH_GOAL)),
                # float(np.isclose(directions, self.GO_STRAIGHT)),
                # float(np.isclose(directions, self.TURN_RIGHT)),
                # float(np.isclose(directions, self.TURN_LEFT)),
                # float(np.isclose(directions, self.LANE_FOLLOW)),
                player_measurements.collision_vehicles,
                player_measurements.collision_pedestrians,
                player_measurements.collision_other,
                player_measurements.intersection_otherlane,
                player_measurements.intersection_offroad,
                target.location.x,
                target.location.y,
                target.rotation.yaw
            ])

        try:
            img = cv2.resize(sensor_data['CameraRGB'].data, (self.h, self.w)) / 255.0
        except:
            raise CameraException(env_id)
        img = np.transpose(img, (2, 0, 1))

        if self.rel_coord_system:
            return {'img': img, 'v': v, 'world_pos': world_pos}
        else:
            return {'img': img, 'v': v}

    def _raster_frame(self, sensor_data, measurements, obs):

        frame = sensor_data['CameraRGB'].data.copy()
        cv2.putText(frame, text='Episode number: {:,}'.format(self.num_episodes-1),
                org=(50, 50), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                color=[0, 0, 0], thickness=2)
        cv2.putText(frame, text='Environment steps: {:,}'.format(self.steps),
                org=(50, 80), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                color=[0, 0, 0], thickness=2)

        cv2.putText(frame, text='Speed: {:.02f}'.format( measurements.player_measurements.forward_speed * 3.6),
                    org=(50, 140), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                    color=[0, 0, 0], thickness=2)
        cv2.putText(frame, text='rel_x: {:.02f}, rel_y: {:.02f}'.format(obs['v'][-2].item(), obs['v'][-1].item()),
                    org=(50, 170), fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.0,
                    color=[0, 0, 0], thickness=2)
        self.video_writer.writeFrame(frame)

    @staticmethod
    def _is_collision(measurements):

        c = 0
        c += measurements.player_measurements.collision_vehicles
        c += measurements.player_measurements.collision_pedestrians
        c += measurements.player_measurements.collision_other

        sidewalk_intersection = measurements.player_measurements.intersection_offroad

        otherlane_intersection = measurements.player_measurements.intersection_otherlane

        return (c > 1e-9) or (sidewalk_intersection > 0.01) or (otherlane_intersection > 0.9), c

    def _new_episode(self):
        ???
    
    def step(self, action):
        # TODO: operate on World
        if self.done:
            raise ValueError('self.done should always be False when calling step')

        while True:

            try:
                # Send Control: TODO: configure self._control
                control = ??? "see action converter"
                self.world.player.apply_control(control)

                # TODO: measurement incomplete
                measurements, sensor_data = ???
                self.last_measurements = measurements
                current_timestamp = measurements.game_timestamp
                distance_to_goal = self._get_distance_to_goal(measurements, self._target) # TODO: self._target is set in new_episode
                self.last_distance_to_goal = distance_to_goal
                # omit directions (closely coupled to deprecated APIs)
                # TODO: obs incomplete
                obs = self._obs_convert(measurements, sensor_data, self._target, self.id)
                # NOTE: video_writer set in new_episode
                if self.video_writer is not None and self.steps % 2 == 0:
                    self._raster_frame(sensor_data, measurements, obs) # direction omitted

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
        # NOTE: _initial_timestamp set in reset
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
        reward = self.reward_calculator.get_reward(measurements, self._target, control, env_state)

        # Additional information
        info = {'carla-reward': reward}

        self.steps += 1

        return obs, reward, self.done, info

    def reset(self):
        
        # Loop forever due to TCPConnectionErrors
        while True:
            try:
                self.reward_calculator.reset_reward()
                self.done = False
                if self.video_writer is not None:
                    try:
                        self.video_writer.close()
                    except Exception as e:
                        self.logger.debug('Error when closing video writer in reset')
                        self.logger.error(e)
                    self.video_writer = None
                # TODO: maybe add back benchmark if possible?
                # if self.benchmark:
                #     end_indicator = self._new_episode_benchmark()
                #     if end_indicator is False:
                #         return False
                # else:
                #     self._new_episode()
                self._new_episode()
                # Hack: Try sleeping so that the server is ready. Reduces the number of TCPErrors
                time.sleep(4)
                # measurements, sensor_data = self._client.read_data()
                # self._client.send_control(VehicleControl())
                measurements, sensor_data = ??? # TODO: get measurements
                self._initial_timestamp = measurements.game_timestamp
                self.last_measurements = measurements
                self.last_distance_to_goal = self._get_distance_to_goal(measurements, self._target)
                # direction omitted
                obs = self._obs_convert(measurements, sensor_data, self._target, self.id)
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
        try:
            self.video_writer.close()
        except Exception as e:
            self.logger.debug('Error when closing video writer in disconnect')
            self.logger.error(e)
        self.video_writer = None
