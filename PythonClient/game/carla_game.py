import time
import logging

try:
    import pygame
    from pygame.locals import K_r
    from pygame.locals import K_p
    from pygame.locals import K_ESCAPE
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

from carla.planner.planner import sldist
from carla.planner.map import CarlaMap
from carla import image_converter
from carla.util import print_over_same_line


class Timer(object):
    def __init__(self):
        self.step = 0
        self._lap_step = 0
        self._lap_time = time.time()

    def tick(self):
        self.step += 1

    def lap(self):
        self._lap_step = self.step
        self._lap_time = time.time()

    def ticks_per_second(self):
        return float(self.step - self._lap_step) / self.elapsed_seconds_since_lap()

    def elapsed_seconds_since_lap(self):
        return time.time() - self._lap_time


class CarlaGame(object):
    """
        Class to plot a game screen and control a simple situation were the player has
        to reach some objectives.
    """

    def __init__(self, args, window_width, window_height, mini_window_width, mini_window_height):

        self._timer = None
        self._display = None
        self._main_image = None
        self._mini_view_image1 = None
        self._mini_view_image2 = None
        self._enable_autopilot = args.autopilot
        self._lidar_measurement = None
        self._map_view = None
        self._is_on_reverse = False
        self._display_map = args.map
        self._map_name = None
        self._map = None
        self._map_shape = None
        self._map_view = None
        self._goal_position = None

        self._window_width = window_width
        self._window_height = window_height
        self._mini_window_width = mini_window_width
        self._mini_window_height = mini_window_height

    def initialize_game(self, map_name):
        """
            Initialize the windows
        Args:
            map_name: The map that is going to be used ( If it was up to display)

        Returns:
            None

        """
        self._map_name = map_name
        if self._display_map:
            self._map = CarlaMap(map_name, 0.1643, 50.0)
            self._map_shape = self._map.map_image.shape
            self._map_view = self._map.get_map(self._window_height)

            extra_width = int(
                (self._window_height / float(self._map_shape[0])) * self._map_shape[1])
            self._display = pygame.display.set_mode(
                (self._window_width + extra_width, self._window_height),
                pygame.HWSURFACE | pygame.DOUBLEBUF)
        else:
            self._display = pygame.display.set_mode(
                (self._window_width, self._window_height),
                pygame.HWSURFACE | pygame.DOUBLEBUF)

        logging.debug('pygame started')

    def start_timer(self):
        self._timer = Timer()

    def set_objective(self, goal_position):
        """
            Set the player objective, the goal position.
            This will be rendered in the map in the future
            Only sets if map is enabled.
        Args:
            goal_position:

        Returns:

        """
        if self._display_map:
            goal_position = self._map.convert_to_pixel([
                goal_position.location.x, goal_position.location.y, goal_position.location.z])
            self._goal_position = goal_position

    def is_reset(self, player_position):
        """
            Check if player reach the goal or if reset button is pressed
        Args:
            player_position: The player position

        Returns:

        """
        if self._display_map:
            player_position = self._map.convert_to_pixel([
                player_position.x, player_position.y, player_position.z])

            if sldist(player_position, self._goal_position) < 7.0:
                return True

        keys = pygame.key.get_pressed()
        return keys[K_r]

    def is_running(self):
        """
            If esc is not pressed the game is running
        Returns:
            if esc was pressed
        """
        keys = pygame.key.get_pressed()
        return not keys[K_ESCAPE]

    def is_autopilot_enabled(self):
        keys = pygame.key.get_pressed()
        if keys[K_p]:
            self._enable_autopilot = not self._enable_autopilot

        return self._enable_autopilot

    def print_measurements(self, measurements):
        """
        Priting function: print all the measurements that are being received.

        """

        self._timer.tick()
        if self._timer.elapsed_seconds_since_lap() > 1.0:
            if self._display_map:
                # Function to get car position on map.
                map_position = self._map.convert_to_pixel([
                    measurements.player_measurements.transform.location.x,
                    measurements.player_measurements.transform.location.y,
                    measurements.player_measurements.transform.location.z])
                # Function to get orientation of the road car is in.
                lane_orientation = self._map.get_lane_orientation([
                    measurements.player_measurements.transform.location.x,
                    measurements.player_measurements.transform.location.y,
                    measurements.player_measurements.transform.location.z])

                self._print_player_measurements_map(
                    measurements.player_measurements,
                    map_position,
                    lane_orientation)
            else:
                self._print_player_measurements(measurements.player_measurements)

            self._timer.lap()

    def _print_player_measurements_map(
            self,
            player_measurements,
            map_position,
            lane_orientation):
        message = 'Step {step} ({fps:.1f} FPS): '
        message += 'Map Position ({map_x:.1f},{map_y:.1f}) '
        message += 'Lane Orientation ({ori_x:.1f},{ori_y:.1f}) '
        message += '{speed:.2f} km/h, '
        message += '{other_lane:.0f}% other lane, {offroad:.0f}% off-road'
        message = message.format(
            map_x=map_position[0],
            map_y=map_position[1],
            ori_x=lane_orientation[0],
            ori_y=lane_orientation[1],
            step=self._timer.step,
            fps=self._timer.ticks_per_second(),
            speed=player_measurements.forward_speed * 3.6,
            other_lane=100 * player_measurements.intersection_otherlane,
            offroad=100 * player_measurements.intersection_offroad)
        print_over_same_line(message)

    def _print_player_measurements(self, player_measurements):
        message = 'Step {step} ({fps:.1f} FPS): '
        message += '{speed:.2f} km/h, '
        message += '{other_lane:.0f}% other lane, {offroad:.0f}% off-road'
        message = message.format(
            step=self._timer.step,
            fps=self._timer.ticks_per_second(),
            speed=player_measurements.forward_speed * 3.6,
            other_lane=100 * player_measurements.intersection_otherlane,
            offroad=100 * player_measurements.intersection_offroad)
        print_over_same_line(message)

    def _draw_waypoints(self, surface, waypoints):
        """
            Draw the waypoints on the map surface.
        Args:
            surface:
            waypoints:

        Returns:

        """

        for waypoint in waypoints:
            new_window_width = \
                (float(self._window_height) / float(self._map_shape[0])) * \
                float(self._map_shape[1])

            w_pos = int(waypoint[0] * (float(self._window_height) / float(self._map_shape[0])))
            h_pos = int(waypoint[1] * (new_window_width / float(self._map_shape[1])))

            pygame.draw.circle(surface, [255, 0, 0, 255], (w_pos, h_pos), 3, 0)

    def _draw_goal_position(self, surface):
        """
            Draw the goal position on the map surface.
        Args:
            surface:

        Returns:

        """

        new_window_width = \
            (float(self._window_height) / float(self._map_shape[0])) * \
            float(self._map_shape[1])

        w_pos = int(
            self._goal_position[0] * (float(self._window_height) / float(self._map_shape[0])))
        h_pos = int(self._goal_position[1] * (new_window_width / float(self._map_shape[1])))

        pygame.draw.circle(surface, [0, 255, 0, 255], (w_pos, h_pos), 7, 0)

    def render(self, sensor_data, player_position=None, waypoints=None, agents_positions=None):
        """
        Main rendering function.
        Args:
            sensor_data: The sensor data, images you want to render on the window
            player_position: The current player position
            waypoints: The waypoints , next positions. If you want to plot then.
            agents_positions: All agent positions ( vehicles)

        Returns:
            None
        """

        gap_x = (self._window_width - 2 * self._mini_window_width) / 3
        mini_image_y = self._window_height - self._mini_window_height - gap_x

        main_image = sensor_data.get('CameraRGB', None)
        mini_view_image1 = sensor_data.get('CameraDepth', None)
        mini_view_image2 = sensor_data.get('CameraSemSeg', None)
        lidar_measurement = sensor_data.get('Lidar32', None)

        if main_image is not None:
            array = image_converter.to_rgb_array(main_image)
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
            self._display.blit(surface, (0, 0))

        if mini_view_image1 is not None:
            array = image_converter.depth_to_logarithmic_grayscale(mini_view_image1)
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
            self._display.blit(surface, (gap_x, mini_image_y))

        if mini_view_image2 is not None:
            array = image_converter.labels_to_cityscapes_palette(mini_view_image2)
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

            self._display.blit(
                surface, (2 * gap_x + self._mini_window_width, mini_image_y))

        # TODO: This LIdar visualization can be drastically improved
        if lidar_measurement is not None:
            lidar_data = np.array(lidar_measurement.data[:, :2])
            lidar_data *= 2.0
            lidar_data += 100.0
            lidar_data = np.fabs(lidar_data)
            lidar_data = lidar_data.astype(np.int32)
            lidar_data = np.reshape(lidar_data, (-1, 2))
            # draw lidar
            lidar_img_size = (200, 200, 3)
            lidar_img = np.zeros(lidar_img_size)
            lidar_img[tuple(lidar_data.T)] = (255, 255, 255)
            surface = pygame.surfarray.make_surface(lidar_img)
            self._display.blit(surface, (10, 10))

        # only if the map view setting is set we actually plot all the positions and waypoints
        if self._map_view is not None:
            player_position = self._map.convert_to_pixel([
                player_position.x,
                player_position.y,
                player_position.z])

            array = self._map_view
            array = array[:, :, :3]

            new_window_width = \
                (float(self._window_height) / float(self._map_shape[0])) * \
                float(self._map_shape[1])
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

            w_pos = int(
                player_position[0] * (float(self._window_height) / float(self._map_shape[0])))
            h_pos = int(player_position[1] * (new_window_width / float(self._map_shape[1])))

            if waypoints is not None:
                self._draw_waypoints(surface, waypoints)
            self._draw_goal_position(surface)
            pygame.draw.circle(surface, [255, 0, 0, 255], (w_pos, h_pos), 6, 0)
            for agent in agents_positions:
                if agent.HasField('vehicle'):
                    agent_position = self._map.convert_to_pixel([
                        agent.vehicle.transform.location.x,
                        agent.vehicle.transform.location.y,
                        agent.vehicle.transform.location.z])

                    w_pos = int(agent_position[0] * (
                            float(self._window_height) / float(self._map_shape[0])))
                    h_pos = int(agent_position[1] * (new_window_width / float(self._map_shape[1])))

                    pygame.draw.circle(surface, [255, 0, 255, 255], (w_pos, h_pos), 4, 0)

            self._display.blit(surface, (self._window_width, 0))

        pygame.display.flip()
