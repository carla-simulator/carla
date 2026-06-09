#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Top-down view of a Hybrid Solid-State LiDAR point cloud.

The HSS LiDAR (`sensor.lidar.hss_lidar`) is mounted forward-facing on a
vehicle. Each sweep is drawn as a bird's-eye scatter in a PyGame window: the
ego vehicle sits at the bottom centre and the forward 120-degree wedge fans
out upward. Points are coloured by distance: near is red, far is blue.
"""

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

import numpy as np
import pygame


WINDOW = 900
MARGIN = 40
LIDAR_RANGE = 100.0
LIDAR_TRANSFORM = carla.Transform(carla.Location(x=0.0, z=2.4))


def depth_colormap(depth, max_range):
    """Map a (N,) distance array to (N,3) uint8 RGB: near=red, far=blue."""
    t = np.clip(depth / max_range, 0.0, 1.0)
    r = np.clip(1.5 - 2.0 * t, 0.0, 1.0)
    g = np.clip(1.5 - 2.0 * np.abs(t - 0.5), 0.0, 1.0)
    b = np.clip(2.0 * t - 0.5, 0.0, 1.0)
    return (np.stack([r, g, b], axis=1) * 255).astype(np.uint8)


def top_down_pixels(measurement):
    """A LiDAR sweep -> (u, v, colors) for a bird's-eye scatter.

    LiDAR frame: x forward, y right. The ego sits at the bottom centre, x
    points up the screen, y points right.
    """
    data = np.frombuffer(measurement.raw_data, dtype=np.float32)
    points = data.reshape((-1, 4))  # x, y, z, intensity
    x, y = points[:, 0], points[:, 1]

    scale = (WINDOW - 2 * MARGIN) / LIDAR_RANGE
    u = (WINDOW / 2 + y * scale).astype(np.int32)
    v = (WINDOW - MARGIN - x * scale).astype(np.int32)

    inside = (u >= 0) & (u < WINDOW) & (v >= 0) & (v < WINDOW)
    colors = depth_colormap(np.hypot(x, y), LIDAR_RANGE)
    return u[inside], v[inside], colors[inside]


def main():
    client = carla.Client('localhost', 2000)
    client.set_timeout(15.0)
    world = client.get_world()
    bp_lib = world.get_blueprint_library()

    actors = []
    pygame.init()
    display = pygame.display.set_mode((WINDOW, WINDOW))
    pygame.display.set_caption('HSS LiDAR - top-down point cloud')
    font = pygame.font.SysFont('mono', 18)
    clock = pygame.time.Clock()

    latest = {'points': None}

    try:
        vehicle_bp = bp_lib.filter('vehicle.*')[0]
        spawn = world.get_map().get_spawn_points()[0]
        vehicle = world.spawn_actor(vehicle_bp, spawn)
        vehicle.set_autopilot(True)
        actors.append(vehicle)

        lidar_bp = bp_lib.find('sensor.lidar.hss_lidar')
        lidar_bp.set_attribute('range', str(int(LIDAR_RANGE)))
        lidar_bp.set_attribute('channels', '128')
        lidar_bp.set_attribute('horizontal_resolution', '0.1')
        lidar = world.spawn_actor(lidar_bp, LIDAR_TRANSFORM, attach_to=vehicle)
        actors.append(lidar)
        lidar.listen(lambda m: latest.__setitem__('points', m))

        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif (event.type == pygame.KEYUP and
                      event.key in (pygame.K_ESCAPE, pygame.K_q)):
                    running = False

            display.fill((0, 0, 0))
            count = 0
            if latest['points'] is not None:
                u, v, colors = top_down_pixels(latest['points'])
                count = len(u)
                frame = np.zeros((WINDOW, WINDOW, 3), dtype=np.uint8)
                frame[u, v] = colors  # surfarray axes: [x, y]
                display.blit(pygame.surfarray.make_surface(frame), (0, 0))

            pygame.draw.circle(display, (255, 255, 255),
                               (WINDOW // 2, WINDOW - MARGIN), 5)
            hud = font.render('HSS LiDAR points: %d   (ESC/Q to quit)' % count,
                              True, (255, 255, 255), (0, 0, 0))
            display.blit(hud, (8, 8))
            pygame.display.flip()
            clock.tick(30)

    finally:
        for actor in actors:
            try:
                actor.destroy()
            except RuntimeError:
                pass
        pygame.quit()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user.')
