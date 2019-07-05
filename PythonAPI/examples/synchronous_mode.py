#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

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

import carla

import logging
import random

try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

try:
    import queue
except ImportError:
    import Queue as queue


def draw_image(surface, image):
    array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
    array = np.reshape(array, (image.height, image.width, 4))
    array = array[:, :, :3]
    array = array[:, :, ::-1]
    image_surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
    surface.blit(image_surface, (0, 0))


def get_font():
    fonts = [x for x in pygame.font.get_fonts()]
    default_font = 'ubuntumono'
    font = default_font if default_font in fonts else fonts[0]
    font = pygame.font.match_font(font)
    return pygame.font.Font(font, 14)


def should_quit():
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            return True
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_ESCAPE:
                return True
    return False


def main():
    actor_list = []
    pygame.init()

    client = carla.Client('localhost', 2000)
    client.set_timeout(2.0)

    world = client.get_world()

    print('enabling synchronous mode.')
    settings = world.get_settings()
    settings.synchronous_mode = True
    world.apply_settings(settings)

    try:
        m = world.get_map()
        start_pose = random.choice(m.get_spawn_points())
        waypoint = m.get_waypoint(start_pose.location)

        blueprint_library = world.get_blueprint_library()

        vehicle = world.spawn_actor(
            random.choice(blueprint_library.filter('vehicle.*')),
            start_pose)
        actor_list.append(vehicle)
        vehicle.set_simulate_physics(False)

        camera = world.spawn_actor(
            blueprint_library.find('sensor.camera.rgb'),
            carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
            attach_to=vehicle)
        actor_list.append(camera)

        # Make sync queue for sensor data.
        image_queue = queue.Queue()
        camera.listen(image_queue.put)

        frame = None

        display = pygame.display.set_mode(
            (800, 600),
            pygame.HWSURFACE | pygame.DOUBLEBUF)
        font = get_font()

        clock = pygame.time.Clock()

        while True:
            if should_quit():
                return

            clock.tick()
            world.tick()
            ts = world.wait_for_tick().timestamp

            if frame is not None:
                if ts.frame != frame + 1:
                    logging.warning('frame skip!')

            frame = ts.frame

            while True:
                image = image_queue.get()
                if image.frame == ts.frame:
                    break
                logging.warning(
                    'wrong image time-stampstamp: frame=%d, image.frame=%d',
                    ts.frame,
                    image.frame)

            waypoint = random.choice(waypoint.next(2))
            vehicle.set_transform(waypoint.transform)

            draw_image(display, image)

            text_surface = font.render('% 5d FPS' % clock.get_fps(), True, (255, 255, 255))
            display.blit(text_surface, (8, 10))

            pygame.display.flip()

    finally:
        print('\ndisabling synchronous mode.')
        settings = world.get_settings()
        settings.synchronous_mode = False
        world.apply_settings(settings)

        print('destroying actors.')
        for actor in actor_list:
            actor.destroy()

        pygame.quit()
        print('done.')


if __name__ == '__main__':

    main()
