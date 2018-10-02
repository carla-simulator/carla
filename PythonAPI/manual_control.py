#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Keyboard controlling for CARLA. Please refer to client_example.py for a simpler
# and more documented example.

"""
Welcome to CARLA manual control.

Use ARROWS or WASD keys for control.

    W            : throttle
    S            : brake
    AD           : steer
    Q            : toggle reverse
    Space        : hand-brake
    P            : toggle autopilot

STARTING in a moment...
"""

###############################################################################

from __future__ import print_function

import sys
import os

if os.name == "posix":
    sys.path.append(
        'PythonAPI/carla-0.9.0-py%d.%d-linux-x86_64.egg' % (sys.version_info.major,
                                                            sys.version_info.minor))
elif os.name == "nt":
    sys.path.append(
        'PythonAPI/carla-0.9.0-py%d.%d-win-amd64.egg' % (sys.version_info.major,
                                                        sys.version_info.minor))
else:
    raise NotImplementedError

import carla

import argparse
import logging
import random
import time

###############################################################################

try:
    import pygame
    from pygame.locals import K_DOWN
    from pygame.locals import K_LEFT
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SPACE
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_p
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_w
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

###############################################################################

WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600
START_POSITION = carla.Transform(carla.Location(x=180.0, y=199.0, z=40.0))
CAMERA_POSITION = carla.Transform(carla.Location(x=0.5, z=1.40))

###############################################################################

class CarlaGame(object):
    def __init__(self, args):
        self._client = carla.Client(args.host, args.port)
        self._client.set_timeout(2000)

        self._display = None
        self._surface = None

        self._camera = None
        self._vehicle = None

        self._is_on_reverse = False
        self._autopilot_enabled = args.autopilot

        self.keyTime = {}
        self.keyTimeout = 0.3

        self.steeringTime = 2.0
        self.steeringTimeLeft = 0.0
        self.steeringTimeRight = 0.0

    def lerp(self, t, v0, v1):
        if t > 1.0: t = 1.0
        return (1.0 - t) * v0 + t * v1

    def resetVehicle(self):
        if self._camera is not None:
            self._camera.destroy()
            self._camera = None

        if self._vehicle is not None:
            self._vehicle.destroy()
            self._vehicle = None

        world = self._client.get_world()
        blueprint = random.choice(world.get_blueprint_library().filter('tt'))
        cam_blueprint = world.get_blueprint_library().find('sensor.camera')

        self._vehicle = world.spawn_actor(blueprint, START_POSITION)
        self._camera = world.spawn_actor(cam_blueprint, CAMERA_POSITION, attach_to=self._vehicle)

        self._camera.listen(self._parse_image)

    def execute(self):
        pygame.init()

        try:
            self._display = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.HWSURFACE | pygame.DOUBLEBUF)
            logging.debug('pygame started')

            self.resetVehicle()
            self._vehicle.set_autopilot(self._autopilot_enabled)

            while True:
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        return

                self._on_loop()
                self._on_render()
        finally:
            if self._camera is not None:
                self._camera.destroy()
                self._camera = None

            if self._vehicle is not None:
                self._vehicle.destroy()
                self._vehicle = None

            pygame.quit()

    def _parse_image(self, image):
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        self._surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

    def _on_loop(self):
        autopilot = self._autopilot_enabled
        control = self._get_keyboard_control(pygame.key.get_pressed())

        if autopilot != self._autopilot_enabled:
            self._vehicle.set_autopilot(autopilot)
            self._autopilot_enabled = autopilot

        if not self._autopilot_enabled:
            self._vehicle.apply_control(control)

    def _get_keyboard_control(self, keys):
        control = carla.VehicleControl()

        if keys[K_LEFT] or keys[K_a]:
            if self.steeringTimeLeft < 0.00001: self.steeringTimeLeft = time.time() + self.steeringTime
            t = 1.0 - (self.steeringTimeLeft - time.time()) / self.steeringTime
            control.steer = -(0.2 + self.lerp(t, 0.0, 0.5))
        else:
            self.steeringTimeLeft = 0.0

        if keys[K_RIGHT] or keys[K_d]:
            if self.steeringTimeRight < 0.00001: self.steeringTimeRight = time.time() + self.steeringTime
            t = 1.0 - (self.steeringTimeRight - time.time()) / self.steeringTime
            control.steer = +(0.2 + self.lerp(t, 0.0, 0.5))
        else:
            self.steeringTimeRight = 0.0

        if keys[K_UP] or keys[K_w]:
            control.throttle = 1.0

        if keys[K_DOWN] or keys[K_s]:
            control.brake = 1.0

        if keys[K_SPACE]:
            control.hand_brake = True

        # Interval key press
        # These keys can only be pressed each X seconds

        if keys[K_q] and not 'K_q' in self.keyTime:
            self._is_on_reverse = not self._is_on_reverse
            self.keyTime.update(K_q = time.time() + self.keyTimeout)

        if keys[K_p] and not 'K_p' in self.keyTime:
            self._autopilot_enabled = not self._autopilot_enabled
            self.keyTime.update(K_p = time.time() + self.keyTimeout)

        # Allow keys to be pressed again
        # Delete key from the pressed key dictionary

        for key, value in self.keyTime.items():
            if value - time.time() <= 0.0:
                self.keyTime.pop(key)
                break

        control.reverse = self._is_on_reverse
        return control

    def _on_render(self):
        if self._surface is not None:
            self._display.blit(self._surface, (0, 0))
        pygame.display.flip()


###############################################################################

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
        help='IP of the host server (default: localhost)')

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

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)
    print(__doc__)

    while True:
        try:
            game = CarlaGame(args)
            game.execute()
            break

        except Exception as error:
            logging.error(error)
            time.sleep(1)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
