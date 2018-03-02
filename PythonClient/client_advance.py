#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Welcome to CARLA Simulator.

If this was executed in Manual mode then
use 'ARROWS' or W, A, S, D keys for control.

    W            : throttle
    S            : brake
    AD           : steer
    Q            : toggle reverse
    Space        : hand-brake

    R            : restart level
    P            : toggle autopilot
    Esc          : Exit Game


STARTING in a moment...
"""

# ******************************************    Libraries to be imported    ****************************************** #
from __future__ import print_function

import argparse
import logging
import random
import sys
import time
import os

try:
    import pygame
    from pygame.locals import K_DOWN
    from pygame.locals import K_LEFT
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SPACE
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_w
    from pygame.locals import K_p
    from pygame.locals import K_ESCAPE
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

try:
    import scipy.misc
except ImportError:
    raise RuntimeError('cannot import scipy, make sure numpy package is installed')

try:
    from carla import image_converter
    from carla import sensor
    from carla.settings import CarlaSettings
    from carla.planner.map import CarlaMap
    from carla.tcp import TCPConnectionError
    from carla.util import print_over_same_line
    from carla.client import make_carla_client, VehicleControl
except ImportError:
    raise RuntimeError('cannot import carla, make sure carla libraries folder is present in the client folder')

# ******************************************        Global Variables        ****************************************** #
WIN_WDT = 640
WIN_HGT = 480
MINI_WIN_WDT = 320
MINI_WIN_HGT = 240
MAX_FRAMES = 7500
WEATHER_PRESETS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]


# ******************************************    Class Declaration Start     ****************************************** #
class MakeCarlaSettings(object):
    """
    Make a CarlaSettings object with the settings we need.
    """

    def __init__(self):
        # Create a CarlaSettings object. This object is a wrapper around the CarlaSettings.ini file.
        # Here we set the configuration we want for the new episode.
        self._settings = CarlaSettings()

        self._settings.set(
            SynchronousMode=True,
            SendNonPlayerAgentsInfo=True,
            NumberOfVehicles=30,
            NumberOfPedestrians=20,
            WeatherId=random.choice(WEATHER_PRESETS))

        self._settings.randomize_seeds()

        # We want to add a couple of cameras to the player vehicle.
        # We will collect the images produced by these cameras every frame.
        self._camera = []

        # The default camera captures RGB images of the scene.
        self._camera.append(sensor.Camera('CameraRGB', PostProcessing='SceneFinal'))
        self._camera[0].set_image_size(WIN_WDT, WIN_HGT)
        self._camera[0].set_position(100, 0, 130)
        self._camera[0].set_rotation(10.0, 0.0, 0.0)
        self._settings.add_sensor(self._camera[0])

        # Add another camera producing ground-truth depth.
        self._camera.append(sensor.Camera('CameraDepth', PostProcessing='Depth'))
        self._camera[1].set_image_size(WIN_WDT, WIN_HGT)
        self._camera[1].set_position(100, 0, 130)
        self._camera[1].set_rotation(10.0, 0.0, 0.0)
        self._settings.add_sensor(self._camera[1])

        # Add another camera producing SemanticSegmentation.
        self._camera.append(sensor.Camera('CameraSemSeg', PostProcessing='SemanticSegmentation'))
        self._camera[2].set_image_size(WIN_WDT, WIN_HGT)
        self._camera[2].set_position(100, 0, 130)
        self._camera[2].set_rotation(10.0, 0.0, 0.0)
        self._settings.add_sensor(self._camera[2])

        self._num_of_cameras = len(self._camera)

    # ******************************        Class Method Declaration        ****************************************** #
    def get_carla_settings(self):

        return self._settings

    # ******************************        Class Method Declaration        ****************************************** #
    def get_num_of_cameras(self):

        return self._num_of_cameras

    # ******************************        Class Method Declaration        ****************************************** #
    def get_camera_names(self):

        camera_names = []
        for i in range(self._num_of_cameras):
            camera_names.append(self._camera[i].CameraName)

        return camera_names

    # ******************************        Class Method Declaration        ****************************************** #
    def get_post_processing(self):

        post_processing = []
        for i in range(self._num_of_cameras):
            post_processing.append(self._camera[i].PostProcessing)

        return post_processing

    # ******************************        Class Method Declaration        ****************************************** #
    def get_camera_resolution(self):

        camera_resolution = []
        for i in range(self._num_of_cameras):
            camera_resolution.append([self._camera[i].ImageSizeX, self._camera[i].ImageSizeY])

        return camera_resolution


# ******************************************    Class Declaration End       ****************************************** #


# ******************************************    Class Declaration Start     ****************************************** #
class Timer(object):
    """
    Create a Timer class which handles all the time related operations in the program.
    """

    def __init__(self):
        # Class Initialization

        self.step = 0
        self._lap_step = 0
        self._lap_time = time.time()

    # ******************************        Class Method Declaration        ****************************************** #
    def tick(self):
        self.step += 1

    # ******************************        Class Method Declaration        ****************************************** #
    def step(self):
        return self.step

    # ******************************        Class Method Declaration        ****************************************** #
    def lap(self):
        self._lap_step = self.step
        self._lap_time = time.time()

    # ******************************        Class Method Declaration        ****************************************** #
    def ticks_per_second(self):
        return float(self.step - self._lap_step) / self.elapsed_seconds_since_lap()

    # ******************************        Class Method Declaration        ****************************************** #
    def elapsed_seconds_since_lap(self):
        return time.time() - self._lap_time


# ******************************************    Class Declaration End       ****************************************** #


# ******************************************    Class Declaration Start     ****************************************** #
# noinspection SpellCheckingInspection,SpellCheckingInspection
class CarlaSim(object):
    """
    Create a CarlaSim class that contains everything related to the simulation of the car
    """

    def __init__(self, carla_client, autopilot_on, render_on, save_images_to_disk, city_name=None):
        # Class Initialization

        self._client = carla_client
        self._autopilot_on = autopilot_on
        self._render_on = render_on
        self._save_images_to_disk = save_images_to_disk
        self._city_name = city_name

        self._timer = None
        self._display = None
        self._settings = MakeCarlaSettings()

        self._map_view = None
        self._position = None
        self._agent_positions = None
        self._control = VehicleControl()
        self._control.reverse = False

        self._episode = 0

        self._measure = {'Name': "Annotations", 'Address': "", 'Measurements': None}

        # camera data is stored in a list of dictionaries.
        self._camera = []

        for i in range(self._settings.get_num_of_cameras()):
            self._camera.append({'Name': self._settings.get_camera_names()[i],
                                 'PostProcessing': self._settings.get_post_processing()[i],
                                 'Address': "",
                                 'Image': None,
                                 'Image_Array': np.empty(self._settings.get_camera_resolution()[i])})

        self._map = CarlaMap(city_name, 16.43, 50.0) if city_name is not None else None
        self._map_shape = self._map.map_image.shape if city_name is not None else None

        if self._settings.get_num_of_cameras() is 1:
            self._map_view = self._map.get_map(WIN_HGT) if city_name is not None else None
        elif 1 < self._settings.get_num_of_cameras() < 4:
            self._map_view = self._map.get_map(WIN_HGT + MINI_WIN_HGT) if city_name is not None else None

        self._map_view_array = None

    # ******************************        Class Method Declaration        ****************************************** #
    def execute(self):
        """
        Launch the PyGame.
        """

        pygame.init()

        self.initialize_game()

        try:
            while True:

                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        return

                # Process the sensor readings
                self.on_loop()

                # Save the images to disk if requested
                if self._save_images_to_disk:
                    # Save the measurements to the disk
                    self.write_measurements_to_disk()

                    # Save the images to the disk
                    self.write_images_to_disk()

                # Render the sensor readings to the screen if requested
                if self._render_on:
                    # Render the sensor readings
                    self.on_render()

                if self._timer.step >= MAX_FRAMES:
                    # If the required number of frames were generated, start a new episode
                    self.on_new_episode()

        finally:
            pygame.quit()

    # ******************************        Class Method Declaration        ****************************************** #
    def initialize_game(self):
        """
        Initialize the displays used to see camera data and map in the simulation
        """

        # If rendering was requested by the user only then create the display windows
        if self._render_on or not self._autopilot_on:

            # If city name was explicitly passed by the user in the argument then we need to display the map of the city
            # along with the camera renders.
            if self._city_name is not None:
                if self._settings.get_num_of_cameras() is 1:
                    # Create a display to accommodate the camera renders and the town map
                    self._display = pygame.display.set_mode((
                        WIN_WDT + int((WIN_HGT / float(self._map.map_image.shape[0])) *
                                      self._map.map_image.shape[1]), WIN_HGT),
                        pygame.HWSURFACE | pygame.DOUBLEBUF)

                elif 1 < self._settings.get_num_of_cameras() < 4:
                    # Create a display to accommodate the camera renders and the town map
                    self._display = pygame.display.set_mode((
                        WIN_WDT + int(((WIN_HGT + MINI_WIN_HGT) / float(self._map.map_image.shape[0])) *
                                      self._map.map_image.shape[1]), WIN_HGT + MINI_WIN_HGT),
                        pygame.HWSURFACE | pygame.DOUBLEBUF)

            # Else we just need to display the camera renders
            else:
                if self._settings.get_num_of_cameras() is 1:
                    # Create a display to accommodate the camera renders
                    self._display = pygame.display.set_mode((WIN_WDT, WIN_HGT),
                                                            pygame.HWSURFACE | pygame.DOUBLEBUF)

                elif 1 < self._settings.get_num_of_cameras() < 4:
                    # Create a display to accommodate the camera renders
                    self._display = pygame.display.set_mode((WIN_WDT, WIN_HGT + MINI_WIN_HGT),
                                                            pygame.HWSURFACE | pygame.DOUBLEBUF)

        logging.debug('PyGame Started Successfully')

        # After creating the displays start the Episode
        self.on_new_episode()

    # ******************************        Class Method Declaration        ****************************************** #
    def on_new_episode(self):
        """
        Start a new episode in simulation
        """
        # Mark time at the start of the Episode    
        start_time = time.ctime(time.time())

        self._episode += 1

        self._measure['Address'] = os.getenv("HOME") + '/Sims_CARLA/Ep' + str(self._episode) + '_'
        self._measure['Address'] += start_time + '/' + self._measure['Name'] + '/'

        for i in range(self._settings.get_num_of_cameras()):
            self._camera[i]['Address'] = os.getenv("HOME") + '/Sims_CARLA/Ep' + str(self._episode) + '_'
            self._camera[i]['Address'] += start_time + '/' + self._camera[i]['Name'] + '/'

        # Now we load these settings into the server. The server replies with a scene description containing available
        # start spots for the player. Here we can provide a CarlaSettings object or a CarlaSettings.ini file as string.

        # Load the settings from CarlaSettings object
        scene = self._client.load_settings(self._settings.get_carla_settings())

        # Choose one player start point at random.
        number_of_player_starts = len(scene.player_start_spots)
        player_start = np.random.randint(number_of_player_starts)

        # Notify the server that we want to start the episode at the player_start index.
        # This function blocks the program until the server is ready to start the episode.
        print('Starting a new episode...')
        self._client.start_episode(player_start)

        # Initialize the Timer used for Step Count
        self._timer = Timer()

    # ******************************        Class Method Declaration        ****************************************** #
    def on_loop(self):
        """
        Process the sensor readings in the simulation and generate control for every frame
        """
        try:
            if pygame.key.get_pressed()[K_ESCAPE]:
                raise KeyboardInterrupt
        except KeyboardInterrupt:
            print('\nEsc key was pressed. Exiting the game ...')
            exit(1)

        # Increase the Step Value on processing of every frame
        self._timer.tick()

        # Read the data produced by the server this frame
        self._measure['Measurements'], sensor_data = self._client.read_data()

        for i in range(self._settings.get_num_of_cameras()):
            # Link the camera images to the appropriate images of the CarlaSim class
            try:
                self._camera[i]['Image'] = sensor_data[self._camera[i]['Name']]
            except Warning:
                pass

        # Print measurements every second.
        if self._timer.elapsed_seconds_since_lap() > 1.0:

            # If city_name was explicitly mentioned by the user as an argument then we print the car position on map and
            # orientation of the road car is in along with the player measurements
            if self._city_name is not None:

                # Function to get car position on map.
                map_position = self._map.convert_to_pixel([
                    self._measure['Measurements'].player_measurements.transform.location.x,
                    self._measure['Measurements'].player_measurements.transform.location.y,
                    self._measure['Measurements'].player_measurements.transform.location.z])

                # Function to get orientation of the road car is in.
                lane_orientation = self._map.get_lane_orientation([
                    self._measure['Measurements'].player_measurements.transform.location.x,
                    self._measure['Measurements'].player_measurements.transform.location.y,
                    self._measure['Measurements'].player_measurements.transform.location.z])

                self.print_player_measurements_map(map_position, lane_orientation)

            # Else we just print the player measurements
            else:
                self.print_player_measurements()

            self._timer.lap()

        # Now we have to send the instructions to control the vehicle. If we are in synchronous mode the server will
        # pause the simulation until we send this control.
        if not self._autopilot_on:

            # Generate the Control command to be sent to the server by reading the input from the user
            self.get_keyboard_control(pygame.key.get_pressed())

            # If input is available by the user, over-ride the autopilot
            if pygame.key.get_pressed()[K_p]:
                self._autopilot_on = True

        else:

            # Along with measurements, the server also sends the control that the in-game autopilot would do this frame.
            # We enable autopilot by sending back this control to the server. Also, here we add some noise to the steer.
            self._control = self._measure['Measurements'].player_measurements.autopilot_control
            steer_noise = random.uniform(-1, 1) / (
                    (self._measure['Measurements'].player_measurements.forward_speed ** 2) + 20)
            self._control.steer += steer_noise

            # If input is available by the user, over-ride the autopilot
            if pygame.key.get_pressed()[K_p]:
                self._autopilot_on = False

            # # Generate the reset command to be sent to the server if it was obtained from the user
            # control_reset = self.get_keyboard_control(pygame.key.get_pressed())
            # if control_reset is None:
            #     control = control_reset

        # Plot position on the map as well.

        # Set the player position
        if self._city_name is not None:
            self._position = self._map.convert_to_pixel([
                self._measure['Measurements'].player_measurements.transform.location.x,
                self._measure['Measurements'].player_measurements.transform.location.y,
                self._measure['Measurements'].player_measurements.transform.location.z])

            self._agent_positions = self._measure['Measurements'].non_player_agents

        # If control is 'None' i.e. the value for restart level, we start a new episode
        if self._control is None:
            self.on_new_episode()

        # Else we send the control to server
        else:
            self._client.send_control(self._control)

        # Process images for user only if rendering is on or saving to disk is on
        if self._save_images_to_disk or self._render_on:

            for i in range(self._settings.get_num_of_cameras()):

                if self._camera[i]['Image'] is not None:

                    if self._camera[i]['PostProcessing'] is "SceneFinal":
                        self._camera[i]['Image_Array'] = image_converter.to_rgb_array(
                            self._camera[i]['Image']).copy(order='C')

                    elif self._camera[i]['PostProcessing'] is "Depth":
                        self._camera[i]['Image_Array'] = image_converter.depth_to_logarithmic_grayscale(
                            self._camera[i]['Image']).copy(order='C')

                    elif self._camera[i]['PostProcessing'] is "SemanticSegmentation":
                        self._camera[i]['Image_Array'] = image_converter.labels_to_cityscapes_palette(
                            self._camera[i]['Image']).copy(order='C')

    # ******************************        Class Method Declaration        ****************************************** #
    def get_keyboard_control(self, keys):
        """
        Return a VehicleControl message based on the pressed keys. Return None if a new episode was requested.
        """

        if keys[K_r]:
            return None
        self._control = VehicleControl()
        if keys[K_LEFT] or keys[K_a]:
            self._control.steer = -1.0
        if keys[K_RIGHT] or keys[K_d]:
            self._control.steer = 1.0
        if keys[K_UP] or keys[K_w]:
            self._control.throttle = 1.0
        if keys[K_DOWN] or keys[K_s]:
            self._control.brake = 1.0
        if keys[K_SPACE]:
            self._control.hand_brake = True
        if keys[K_q]:
            self._control.reverse = not self._control.reverse
        if keys[K_p]:
            self._autopilot_on = True

    # ******************************        Class Method Declaration        ****************************************** #
    # noinspection SpellCheckingInspection
    def print_player_measurements_map(self, map_position, lane_orientation):
        """
        Prints the car position on map and orientation of the road car is in along with the player measurements
        """

        message = 'Step {step} ({fps:.1f} FPS): '
        message += 'Map Position ({map_x:.1f},{map_y:.1f}) Lane Orientation ({ori_x:.1f},{ori_y:.1f})  '
        message += '{speed:.2f} km/h, '
        message += '{other_lane:.0f}% other lane, {offroad:.0f}% off-road'
        message = message.format(
            map_x=map_position[0],
            map_y=map_position[1],
            ori_x=lane_orientation[0],
            ori_y=lane_orientation[1],
            step=self._timer.step,
            fps=self._timer.ticks_per_second(),
            speed=self._measure['Measurements'].player_measurements.forward_speed,
            other_lane=100 * self._measure['Measurements'].player_measurements.intersection_otherlane,
            offroad=100 * self._measure['Measurements'].player_measurements.intersection_offroad)
        print_over_same_line(message)

    # ******************************        Class Method Declaration        ****************************************** #
    def print_player_measurements(self):
        """
        Prints the player measurements
        """

        message = 'Step {step} ({fps:.1f} FPS): '
        message += '{speed:.2f} km/h, '
        message += '{other_lane:.0f}% other lane, {offroad:.0f}% off-road'
        message = message.format(
            step=self._timer.step,
            fps=self._timer.ticks_per_second(),
            speed=self._measure['Measurements'].player_measurements.forward_speed,
            other_lane=100 * self._measure['Measurements'].player_measurements.intersection_otherlane,
            offroad=100 * self._measure['Measurements'].player_measurements.intersection_offroad)
        print_over_same_line(message)

    # ******************************        Class Method Declaration        ****************************************** #
    def on_render(self):
        """
        Renders the images generated from the camera and the town map every frame
        """

        if self._settings.get_num_of_cameras() is 1:
            if self._camera[0]['Image'] is not None:
                surface = pygame.surfarray.make_surface(self._camera[0]['Image_Array'].swapaxes(0, 1))
                self._display.blit(surface, (0, 0))

            # If the city_name was passed in as an argument, a map_view is also rendered
            if self._map_view is not None:
                self._map_view_array = self._map_view
                self._map_view_array = self._map_view_array[:, :, :3]
                new_win_wid = (float(WIN_HGT) / float(self._map_shape[0])) * float(
                    self._map_shape[1])
                surface = pygame.surfarray.make_surface(self._map_view_array.swapaxes(0, 1))

                w_pos = int(self._position[0] * (float(WIN_HGT) / float(self._map_shape[0])))
                h_pos = int(self._position[1] * (new_win_wid / float(self._map_shape[1])))

                pygame.draw.circle(surface, [255, 0, 0, 255], (w_pos, h_pos), 6, 0)
                for agent in self._agent_positions:
                    if agent.HasField('vehicle'):
                        agent_position = self._map.get_position_on_map([
                            agent.vehicle.transform.location.x,
                            agent.vehicle.transform.location.y,
                            agent.vehicle.transform.location.z])
                        w_pos = int(agent_position[0] * (float(WIN_HGT) / float(self._map_shape[0])))
                        h_pos = int(agent_position[1] * (new_win_wid / float(self._map_shape[1])))
                        pygame.draw.circle(surface, [255, 0, 255, 255], (w_pos, h_pos), 4, 0)

                self._display.blit(surface, (WIN_WDT, 0))

        # Only if the main image was processed, render it.
        elif 1 < self._settings.get_num_of_cameras() < 4:
            if self._camera[0]['Image'] is not None:
                surface = pygame.surfarray.make_surface(self._camera[0]['Image_Array'].swapaxes(0, 1))
                self._display.blit(surface, (0, MINI_WIN_HGT))

            if self._camera[1]['Image'] is not None:
                self._camera[1]['Image_Array'] = scipy.misc.imresize(self._camera[1]['Image_Array'], 50)
                surface = pygame.surfarray.make_surface(self._camera[1]['Image_Array'].swapaxes(0, 1))
                self._display.blit(surface, (0, 0))

            if self._camera[2]['Image'] is not None:
                self._camera[2]['Image_Array'] = scipy.misc.imresize(self._camera[2]['Image_Array'], 50)
                surface = pygame.surfarray.make_surface(self._camera[2]['Image_Array'].swapaxes(0, 1))
                self._display.blit(surface, (MINI_WIN_WDT, 0))

            # If the city_name was passed in as an argument, a map_view is also rendered
            if self._map_view is not None:
                self._map_view_array = self._map_view
                self._map_view_array = self._map_view_array[:, :, :3]
                new_win_wid = (float(WIN_HGT + MINI_WIN_HGT) / float(self._map_shape[0])) * float(self._map_shape[1])
                surface = pygame.surfarray.make_surface(self._map_view_array.swapaxes(0, 1))

                w_pos = int(self._position[0] * (float(WIN_HGT + MINI_WIN_HGT) / float(self._map_shape[0])))
                h_pos = int(self._position[1] * (new_win_wid / float(self._map_shape[1])))

                pygame.draw.circle(surface, [255, 0, 0, 255], (w_pos, h_pos), 6, 0)
                for agent in self._agent_positions:
                    if agent.HasField('vehicle'):
                        agent_position = self._map.convert_to_pixel([
                            agent.vehicle.transform.location.x,
                            agent.vehicle.transform.location.y,
                            agent.vehicle.transform.location.z])
                        w_pos = int(agent_position[0] * (float(WIN_HGT + MINI_WIN_HGT) / float(self._map_shape[0])))
                        h_pos = int(agent_position[1] * (new_win_wid / float(self._map_shape[1])))
                        pygame.draw.circle(surface, [255, 0, 255, 255], (w_pos, h_pos), 4, 0)

                self._display.blit(surface, (WIN_WDT, 0))

        pygame.display.flip()

    # ******************************        Class Method Declaration        ****************************************** #
    def write_measurements_to_disk(self):
        """
        Writes the player measurements to the disk
        """

        if self._measure is not None:

            if not os.path.exists(self._measure['Address']):
                os.makedirs(self._measure['Address'])

            message = 'Game Timestamp: \t' + str(self._measure['Measurements'].game_timestamp) + '\n'
            message += 'Platform Timestamp: \t' + str(self._measure['Measurements'].platform_timestamp) + '\n\n'
            message += 'Car Location: \n' + str(self._measure['Measurements'].
                                                player_measurements.transform.location) + '\n'
            message += 'Car Rotation: \n' + str(self._measure['Measurements'].
                                                player_measurements.transform.rotation) + '\n\n'
            message += 'Car Acceleration: \n' + str(
                self._measure['Measurements'].player_measurements.acceleration) + '\n'
            message += 'Car Forward Speed: \t' + str(
                self._measure['Measurements'].player_measurements.forward_speed) + '\n\n'
            message += 'Collisions with Vehicles: \t' + str(
                self._measure['Measurements'].player_measurements.collision_vehicles) + '\n'
            message += 'Collisions with Pedestrians: \t' + str(
                self._measure['Measurements'].player_measurements.collision_pedestrians) + '\n'
            message += 'Collisions with Others: \t' + str(
                self._measure['Measurements'].player_measurements.collision_other) + '\n\n'
            message += 'Intersection with other lane: \t' + str(
                100 * self._measure['Measurements'].player_measurements.intersection_otherlane) + '\n'
            message += 'Intersection with off road: \t' + str(
                100 * self._measure['Measurements'].player_measurements.intersection_offroad) + '\n\n'
            message += 'Steer control: \t' + str(self._control.steer) + '\n'
            message += 'Throttle control: \t' + str(self._control.throttle) + '\n'
            message += 'Brake control: \t' + str(self._control.brake) + '\n'
            message += 'Hand Brake control: \t' + str(self._control.hand_brake) + '\n'
            message += 'Reverse control: \t' + str(self._control.reverse) + '\n'
            message += '\n******************************************************************\n\n'
            message += 'Number of Non Player Agents:\t' + str(
                len(self._measure['Measurements'].non_player_agents)) + '\n'
            message += 'Non Player Agents:\n' + str(self._measure['Measurements'].non_player_agents) + '\n'

            with open(self._measure['Address'] + 'measurement' + str(self._timer.step) + '.txt', 'w') as the_file:
                the_file.write(message)

    # ******************************        Class Method Declaration        ****************************************** #
    def write_images_to_disk(self):
        """
        Writes the camera images to the disk
        """
        for i in range(self._settings.get_num_of_cameras()):

            if self._camera[i]['Image'] is not None:

                if not os.path.exists(self._camera[i]['Address']):
                    os.makedirs(self._camera[i]['Address'])

                address = self._camera[i]['Address'] + 'image' + str(self._timer.step) + '.png'
                scipy.misc.imsave(address, self._camera[i]['Image_Array'])


# ******************************************    Class Declaration End       ****************************************** #


# ******************************************        Main Program Start      ****************************************** #
def main():
    """
    The main of the Program
    """

    arg_parser = argparse.ArgumentParser(description='CARLA Client for Manual / Autopilot control: '
                                                     'This is a slightly more complicated code than client_example'
                                                     'or manual_control. If you are finding trouble understanding this'
                                                     'code, try referring to them first.',
                                         usage='./client_advance.py [options ...]'
                                               '\n\tExample: ./client_advance.py -r -m=Town01'
                                               '\n\tExample: ./client_advance.py -v -a -r -i'
                                               '\n\t\t\t\t\tetc..')

    arg_parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='Enable printing of the debug information during program execution. It is recommended to enable this'
             'if the code does not run for some reason, before trying to edit the source code.')

    arg_parser.add_argument(
        '--host',
        metavar='HOST_IP',
        default='localhost',
        help='IP of the host server. CARLA Client is connected to the CARLA Server using the TCP/IP protocol.'
             'mention the IP address of the machine where CARLA Server is run. (default value: localhost)')

    arg_parser.add_argument(
        '-p', '--port',
        metavar='PORT_NUM',
        default=2000,
        type=int,
        help='TCP port to listen to for connecting to the CARLA Server (default value: 2000)')

    arg_parser.add_argument(
        '-a', '--autopilot',
        action='store_true',
        help='Enable the autopilot mode. In this mode the autopilot commands are written to the CARLA Server.'
             'Autopilot mode can be toggled while the CARLA session is running by pressing the key \'M\'')

    arg_parser.add_argument(
        '-r', '--render',
        action='store_true',
        help='Enable rendering the camera onto the screen. It is recommended to enable this during the manual mode.')

    arg_parser.add_argument(
        '-i', '--images-to-disk',
        action='store_true',
        help='Save the sensor images to disk. The images will be saved in their respective folders in the parent'
             'folder: $HOME/Sims_CARLA/Episode<episode_num> <time_stamp>/')

    arg_parser.add_argument(
        '-m', '--map-name',
        metavar='MAP_NAME',
        default=None,
        help='Enable plotting of the map of the current city and the player position onto the screen.'
             '(needs to match active map in server, options: Town01 or Town02)')

    args = arg_parser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO

    # noinspection SpellCheckingInspection
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    while True:
        try:

            with make_carla_client(args.host, args.port) as client:

                # Create a CarlaSim object
                game = CarlaSim(client,
                                autopilot_on=args.autopilot,
                                render_on=args.render,
                                save_images_to_disk=args.images_to_disk,
                                city_name=args.map_name)

                # Execute the simulation
                game.execute()
                break

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)
        except Exception as exception:
            logging.exception(exception)
            sys.exit(1)


if __name__ == '__main__':
    # Defines the function that will run first

    try:
        main()
    except KeyboardInterrupt:
        print('\nCarla Client interrupted by user. Bye!')
