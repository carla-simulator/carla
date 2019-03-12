#!/usr/bin/env python

# Copyright (c) 2019 Aptiv
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
An example of client-side bounding boxes with basic car controls.

Controls:

    W            : throttle
    S            : brake
    AD           : steer
    Space        : hand-brake

    ESC          : quit
"""
import weakref
import random
import numpy as np

import pygame
from pygame.locals import K_ESCAPE
from pygame.locals import K_SPACE
from pygame.locals import K_a
from pygame.locals import K_d
from pygame.locals import K_s
from pygame.locals import K_w

import carla

view_width = 1920//2
view_height = 1080//2


class BasicSynchronousClient(object):
    def __init__(self):
        self.client = None
        self.world = None
        self.car = None

        self.display = None
        self.image = None

    def camera_blueprint(self):
        camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        camera_bp.set_attribute('image_size_x', str(view_width))
        camera_bp.set_attribute('image_size_y', str(view_height))
        camera_bp.set_attribute('fov', str(90))
        return camera_bp


    def set_synchronous_mode(self, synchronous_mode):
        settings = self.world.get_settings()
        settings.synchronous_mode = synchronous_mode
        self.world.apply_settings(settings)


    def control(self, car):
        keys = pygame.key.get_pressed()
        if keys[K_ESCAPE]:
            return True

        control = car.get_control()
        control.throttle = 0
        if keys[K_w]:
            control.throttle = 1
            control.reverse = False
        elif keys[K_s]:
            control.throttle = 1
            control.reverse = True
        if keys[K_a]:
            control.steer = max(-1., min(control.steer - 0.05,0))
        elif keys[K_d]:
            control.steer = min(1., max(control.steer + 0.05,0))
        else:
            control.steer = 0
        control.hand_brake = keys[K_SPACE]

        car.apply_control(control)

    @staticmethod
    def set_image(weak_self, img):
        self = weak_self()
        if self.capture:
            self.image = img
            self.capture = False

    def render(self, display):
        if(self.image is not None):
            array = np.frombuffer(self.image.raw_data, dtype=np.dtype("uint8"))
            array = np.reshape(array, (self.image.height, self.image.width, 4))
            array = array[:, :, :3]
            array = array[:, :, ::-1]
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
            display.blit(surface, (0, 0))
            pygame.display.flip()

    def game_loop(self):
        pygame.init()

        try:
            self.client = carla.Client('127.0.0.1', 2000)
            self.client.set_timeout(2.0)
            self.world = self.client.get_world()

            self.display = pygame.display.set_mode(
                (view_width, view_height),
                pygame.HWSURFACE | pygame.DOUBLEBUF)
            clock = pygame.time.Clock()

            car_bp = self.world.get_blueprint_library().filter('vehicle.*')[0]
            location = random.choice(self.world.get_map().get_spawn_points())
            self.car = self.world.spawn_actor(car_bp, location)

            camera_transform = carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15))
            camera = self.world.spawn_actor(self.camera_blueprint(), camera_transform, attach_to=self.car)
            weak_self = weakref.ref(self)
            camera.listen(lambda image: weak_self().set_image(weak_self, image))

            self.set_synchronous_mode(True)

            while True:
                pygame.event.pump()
                self.capture = True
                self.world.tick()
                clock.tick_busy_loop(60)

                self.render(self.display)
                if self.control(self.car):
                    return

        finally:
            self.set_synchronous_mode(False)
            camera.destroy()
            self.car.destroy()
            pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    try:
        client = BasicSynchronousClient()
        client.game_loop()
    finally:
        print('EXIT')


if __name__ == '__main__':
    main()
