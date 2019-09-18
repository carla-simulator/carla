#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Visualize actors in safe distance with bounding box.

from __future__ import print_function


# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================


import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


import carla

from client_bounding_boxes import ClientSideBoundingBoxes

import argparse
import logging
import random
import weakref

try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


class World(object):
    def __init__(self, carla_world, args):
        self.args = args
        self.world = carla_world
        self.actor_role_name = args.rolename
        self.map = self.world.get_map()
        self.player = None
        self.camera_manager = None
        self._actor_filter = args.filter
        self.restart()

    def restart(self):
        # Get a random blueprint.
        blueprint = random.choice(self.world.get_blueprint_library().filter(self._actor_filter))
        blueprint.set_attribute('role_name', self.actor_role_name)
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)
        if blueprint.has_attribute('is_invincible'):
            blueprint.set_attribute('is_invincible', 'true')
        # Spawn the player.
        if self.player is not None:
            spawn_point = self.player.get_transform()
            spawn_point.location.z += 2.0
            spawn_point.rotation.roll = 0.0
            spawn_point.rotation.pitch = 0.0
            self.destroy()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
        while self.player is None:
            spawn_points = self.map.get_spawn_points()
            spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)
        # Set up the sensors.
        self.safe_distance_sensor = SafeDistanceSensor(self.player, self.args)
        self.camera_manager = CameraManager(self.player, self.args)
        self.camera_manager.set_sensor()
        # Set autopilot
        self.player.set_autopilot(True)

    def tick(self):
        self.world.tick()

    def render(self, display):
        self.camera_manager.render(display)

    def destroy(self):
        actors = [
            self.safe_distance_sensor.sensor,
            self.camera_manager.sensor,
            self.player]
        for actor in actors:
            if actor is not None:
                actor.destroy()


# ==============================================================================
# -- SafeDistanceSensor --------------------------------------------------------
# ==============================================================================


class SafeDistanceSensor(object):
    def __init__(self, parent_actor, args):
        self.sensor = None
        self.attributes = {'safe_distance_front': str(args.front),
                           'safe_distance_back': str(args.back),
                           'safe_distance_lateral': str(args.lateral)}
        self.actors = []
        self._parent = parent_actor
        self.world = self._parent.get_world()
        bp = self.world.get_blueprint_library().find('sensor.other.safe_distance')
        for attribute, value in self.attributes.items():
            bp.set_attribute(attribute, value)
        self.sensor = self.world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda event: SafeDistanceSensor._on_safe_distance(weak_self, event))

    @staticmethod
    def _on_safe_distance(weak_self, event):
        self = weak_self()
        if not self:
            return
        self.actors = []
        for actor_id in event:
            actor = self.world.get_actor(actor_id)
            print('Actor too close: %s' % actor.type_id)
            self.actors.append(actor)


# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================


class CameraManager(object):
    def __init__(self, parent_actor, args):
        self.sensor = None
        self.surface = None
        self.width = args.width
        self.height = args.height
        self.fov = args.fov
        self._parent = parent_actor
        self.camera_transform = carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15))
        world = self._parent.get_world()
        self.camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
        self.camera_bp.set_attribute('image_size_x', str(self.width))
        self.camera_bp.set_attribute('image_size_y', str(self.height))
        self.camera_bp.set_attribute('fov', str(self.fov))

    def set_sensor(self):
        self.sensor = self._parent.get_world().spawn_actor(self.camera_bp, self.camera_transform, attach_to=self._parent)
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda image: CameraManager._parse_image(weak_self, image))

        calibration = np.identity(3)
        calibration[0, 2] = self.width / 2.0
        calibration[1, 2] = self.height / 2.0
        calibration[0, 0] = calibration[1, 1] = self.width / (2.0 * np.tan(self.fov * np.pi / 360.0))
        self.sensor.calibration = calibration

    def render(self, display):
        if self.surface is not None:
            display.blit(self.surface, (0, 0))

    @staticmethod
    def _parse_image(weak_self, image):
        self = weak_self()
        if not self:
            return
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))


# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    pygame.init()
    pygame.font.init()
    world = None

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        world = World(client.get_world(), args)

        clock = pygame.time.Clock()
        while True:
            clock.tick_busy_loop(20)
            world.render(display)
            bounding_boxes = ClientSideBoundingBoxes.get_bounding_boxes(world.safe_distance_sensor.actors,
                                                                        world.camera_manager.sensor)
            ClientSideBoundingBoxes.draw_bounding_boxes(display, bounding_boxes)
            pygame.display.flip()

    finally:

        if world is not None:
            world.destroy()

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-a', '--autopilot',
        action='store_true',
        help='enable autopilot')
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '--fov',
        metavar='FOV',
        default=90,
        help='FOV (default: 90)')
    argparser.add_argument(
        '--front',
        metavar='safe_distance_front',
        default=50,
        help='safe_distance_front (default: 50)')
    argparser.add_argument(
        '--back',
        metavar='safe_distance_back',
        default=20,
        help='safe_distance_back (default: 20)')
    argparser.add_argument(
        '--lateral',
        metavar='safe_distance_lateral',
        default=30,
        help='safe_distance_front (default: 30)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--rolename',
        metavar='NAME',
        default='hero',
        help='actor role name (default: "hero")')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    try:
        game_loop(args)
    finally:
        print('EXIT')

if __name__ == '__main__':
    main()
